#include <Elementary.h>
#include "elm_priv.h"

#define SMART_NAME "e_widget"
#define API_ENTRY Smart_Data *sd = evas_object_smart_data_get(obj); if ((!obj) || (!sd) || (evas_object_type_get(obj) && strcmp(evas_object_type_get(obj), SMART_NAME)))
#define INTERNAL_ENTRY Smart_Data *sd = evas_object_smart_data_get(obj); if (!sd) return;
typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{ 
   Evas_Object   *obj;
   Evas_Object   *parent_obj;
   Evas_Coord     x, y, w, h;
   Eina_List     *subobjs;
   Evas_Object   *resize_obj;
   Evas_Object   *hover_obj;
   void         (*del_func) (Evas_Object *obj);
   void         (*del_pre_func) (Evas_Object *obj);
   void         (*focus_func) (Evas_Object *obj);
   void         (*activate_func) (Evas_Object *obj);
   void         (*disable_func) (Evas_Object *obj);
   void         (*theme_func) (Evas_Object *obj);
   void         (*on_focus_func) (void *data, Evas_Object *obj);
   void          *on_focus_data;
   void         (*on_change_func) (void *data, Evas_Object *obj);
   void          *on_change_data;
   void         (*on_show_region_func) (void *data, Evas_Object *obj);
   void          *on_show_region_data;
   void          *data;
   Evas_Coord     rx, ry, rw, rh;
   unsigned char  can_focus : 1;
   unsigned char  child_can_focus : 1;
   unsigned char  focused : 1;
   unsigned char  disabled : 1;
}; 

/* local subsystem functions */
static void _smart_reconfigure(Smart_Data *sd);
static void _smart_add(Evas_Object *obj);
static void _smart_del(Evas_Object *obj);
static void _smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object *obj);
static void _smart_hide(Evas_Object *obj);
static void _smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object *obj, Evas_Object * clip);
static void _smart_clip_unset(Evas_Object *obj);
static void _smart_init(void);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

static void
_sub_obj_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Smart_Data *sd = data;
   if (obj == sd->resize_obj) sd->resize_obj = NULL;
   else if (obj == sd->hover_obj) sd->hover_obj = NULL;
   else sd->subobjs = eina_list_remove(sd->subobjs, obj);
}

static void
_sub_obj_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Smart_Data *sd = data;
   Evas_Object *op, *op2;

   elm_widget_focus_steal(sd->obj);
}

/* externally accessible functions */
EAPI Evas_Object *
elm_widget_add(Evas *evas)
{
   _smart_init();
   return evas_object_smart_add(evas, _e_smart);
}

EAPI void
elm_widget_del_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj))
{
   API_ENTRY return;
   sd->del_func = func;
}

EAPI void
elm_widget_del_pre_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj))
{
   API_ENTRY return;
   sd->del_pre_func = func;
}

EAPI void
elm_widget_focus_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj))
{
   API_ENTRY return;
   sd->focus_func = func;
}

EAPI void
elm_widget_activate_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj))
{
   API_ENTRY return;
   sd->activate_func = func;
}

EAPI void
elm_widget_disable_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj))
{
   API_ENTRY return;
   sd->disable_func = func;
}

EAPI void
elm_widget_theme_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj))
{
   API_ENTRY return;
   sd->theme_func = func;
}

EAPI void
elm_widget_theme(Evas_Object *obj)
{
   Eina_List *l;
   
   API_ENTRY return;
   if (sd->theme_func) sd->theme_func(obj);
   for (l = sd->subobjs; l; l = l->next) elm_widget_theme(l->data);
   if (sd->resize_obj) elm_widget_theme(sd->resize_obj);
   if (sd->hover_obj) elm_widget_theme(sd->hover_obj);
}

EAPI void
elm_widget_on_focus_hook_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data)
{
   API_ENTRY return;
   sd->on_focus_func = func;
   sd->on_focus_data = data;
}

EAPI void
elm_widget_on_change_hook_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data)
{
   API_ENTRY return;
   sd->on_change_func = func;
   sd->on_change_data = data;
}

EAPI void
elm_widget_on_show_region_hook_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data)
{
   API_ENTRY return;
   sd->on_show_region_func = func;
   sd->on_show_region_data = data;
}

EAPI void
elm_widget_data_set(Evas_Object *obj, void *data)
{
   API_ENTRY return;
   sd->data = data;
}

EAPI void *
elm_widget_data_get(Evas_Object *obj)
{
   API_ENTRY return NULL;
   return sd->data;
}

EAPI void
elm_widget_sub_object_add(Evas_Object *obj, Evas_Object *sobj)
{
   API_ENTRY return;
   sd->subobjs = eina_list_append(sd->subobjs, sobj);
   if (!sd->child_can_focus)
     {
	if (elm_widget_can_focus_get(sobj)) sd->child_can_focus = 1;
     }
   if (!strcmp(evas_object_type_get(sobj), SMART_NAME))
     {
	sd = evas_object_smart_data_get(sobj);
	if (sd)
	  {
	     if (sd->parent_obj) elm_widget_sub_object_del(sd->parent_obj, sobj);
	     sd->parent_obj = obj;
	  }
     }
   evas_object_smart_callback_call(obj, "sub-object-add", sobj);
}

EAPI void
elm_widget_sub_object_del(Evas_Object *obj, Evas_Object *sobj)
{
   API_ENTRY return;
   sd->subobjs = eina_list_remove(sd->subobjs, sobj);
   if (!sd->child_can_focus)
     {
	if (elm_widget_can_focus_get(sobj)) sd->child_can_focus = 0;
     }
   if (!strcmp(evas_object_type_get(sobj), SMART_NAME))
     {
	sd = evas_object_smart_data_get(sobj);
	if (sd) sd->parent_obj = NULL;
     }
   evas_object_smart_callback_call(obj, "sub-object-del", sobj);
}

EAPI void
elm_widget_resize_object_set(Evas_Object *obj, Evas_Object *sobj)
{
   API_ENTRY return;
   if (sd->resize_obj)
     {
	evas_object_event_callback_del(sd->resize_obj, EVAS_CALLBACK_DEL, _sub_obj_del);
	evas_object_event_callback_del(sd->resize_obj, EVAS_CALLBACK_MOUSE_DOWN, _sub_obj_mouse_down);
	evas_object_smart_member_del(sd->resize_obj);
     }
   sd->resize_obj = sobj;
   if (sd->resize_obj)
     {
	evas_object_clip_set(sobj, evas_object_clip_get(obj));
	evas_object_smart_member_add(sobj, obj);
	evas_object_event_callback_add(sobj, EVAS_CALLBACK_DEL, _sub_obj_del, sd);
	evas_object_event_callback_add(sobj, EVAS_CALLBACK_MOUSE_DOWN, _sub_obj_mouse_down, sd);
	_smart_reconfigure(sd);
	evas_object_smart_callback_call(obj, "sub-object-add", sobj);
     }
}

EAPI void
elm_widget_hover_object_set(Evas_Object *obj, Evas_Object *sobj)
{
   API_ENTRY return;
   if (sd->hover_obj)
     {
	evas_object_event_callback_del(sd->hover_obj, EVAS_CALLBACK_DEL, _sub_obj_del);
     }
   sd->hover_obj = sobj;
   if (sd->hover_obj)
     {
	evas_object_event_callback_add(sobj, EVAS_CALLBACK_DEL, _sub_obj_del, sd);
	_smart_reconfigure(sd);
     }
}

EAPI void
elm_widget_can_focus_set(Evas_Object *obj, int can_focus)
{
   API_ENTRY return;
   sd->can_focus = can_focus;
}

EAPI int
elm_widget_can_focus_get(Evas_Object *obj)
{
   API_ENTRY return 0;
   if (sd->can_focus) return 1;
   if (sd->child_can_focus) return 1;
   return 0;
}

EAPI int
elm_widget_focus_get(Evas_Object *obj)
{
   API_ENTRY return 0;
   return sd->focused;
}

EAPI Evas_Object *
elm_widget_focused_object_get(Evas_Object *obj)
{
   Eina_List *l;
   API_ENTRY return NULL;
   if (!sd->focused) return NULL;
   for (l = sd->subobjs; l; l = l->next)
     {  
	Evas_Object *fobj;
	
	fobj = elm_widget_focused_object_get(l->data);
	if (fobj) return fobj;
     }
   return obj;
}

EAPI int
elm_widget_focus_jump(Evas_Object *obj, int forward)
{
   API_ENTRY return 0;
   if (!elm_widget_can_focus_get(obj)) return 0;
       
   /* if it has a focus func its an end-point widget like a button */
   if (sd->focus_func)
     {
	if (!sd->focused) sd->focused = 1;
	else sd->focused = 0;
	sd->focus_func(obj);
	if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, obj);
	return sd->focused;
     }
   /* its some container */
   else
     {
	Eina_List *l;
	int focus_next;
	
	focus_next = 0;
	if (!sd->focused)
	  {
	     elm_widget_focus_set(obj, forward);
	     sd->focused = 1;
	     if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, obj);
	     return 1;
	  }
	else
	  {
	     if (forward)
	       {
		  for (l = sd->subobjs; l; l = l->next)
		    {
		       if (elm_widget_can_focus_get(l->data))
			 {
			    if ((focus_next) &&
				(!elm_widget_disabled_get(l->data)))
			      {
				 /* the previous focused item was unfocused - so focus
				  * the next one (that can be focused) */
				 if (elm_widget_focus_jump(l->data, forward)) return 1;
				 else break;
			      }
			    else
			      {
				 if (elm_widget_focus_get(l->data))
				   {
				      /* jump to the next focused item or focus this item */
				      if (elm_widget_focus_jump(l->data, forward)) return 1;
				      /* it returned 0 - it got to the last item and is past it */
				      focus_next = 1;
				   }
			      }
			 }
		    }
	       }
	     else
	       {
		  for (l = eina_list_last(sd->subobjs); l; l = l->prev)
		    {
		       if (elm_widget_can_focus_get(l->data))
			 {
			    if ((focus_next) &&
				(!elm_widget_disabled_get(l->data)))
			      {
				 /* the previous focused item was unfocused - so focus
				  * the next one (that can be focused) */
				 if (elm_widget_focus_jump(l->data, forward)) return 1;
				 else break;
			      }
			    else
			      {
				 if (elm_widget_focus_get(l->data))
				   {
				      /* jump to the next focused item or focus this item */
				      if (elm_widget_focus_jump(l->data, forward)) return 1;
				      /* it returned 0 - it got to the last item and is past it */
				      focus_next = 1;
				   }
			      }
			 }
		    }
	       }
	  }
     }
   /* no next item can be focused */
   sd->focused = 0;
   return 0;
}

EAPI void
elm_widget_focus_set(Evas_Object *obj, int first)
{
   API_ENTRY return;
   if (!sd->focused)
     {
	sd->focused = 1;
	if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, obj);
     }
   if (sd->focus_func)
     {
	sd->focus_func(obj);
	return;
     }
   else
     {
	Eina_List *l;
	     
	if (first)
	  {
	     for (l = sd->subobjs; l; l = l->next)
	       {
		  if ((elm_widget_can_focus_get(l->data)) &&
		      (!elm_widget_disabled_get(l->data)))
		    {
		       elm_widget_focus_set(l->data, first);
		       break;
		    }
	       }
	  }
	else
	  {
	     for (l = eina_list_last(sd->subobjs); l; l = l->prev)
	       {
		  if ((elm_widget_can_focus_get(l->data)) &&
		      (!elm_widget_disabled_get(l->data)))
		    {
		       elm_widget_focus_set(l->data, first);
		       break;
		    }
	       }
	  }
     }
}

EAPI Evas_Object *
elm_widget_parent_get(Evas_Object *obj)
{
   API_ENTRY return NULL;
   return sd->parent_obj;
}

EAPI void
elm_widget_focused_object_clear(Evas_Object *obj)
{
   Eina_List *l;
   API_ENTRY return;
   if (!sd->focused) return;
   sd->focused = 0;
   for (l = sd->subobjs; l; l = l->next)
     {  
	if (elm_widget_focus_get(l->data))
	  {
	     elm_widget_focused_object_clear(l->data);
	     break;
	  }
     }
   if (sd->focus_func) sd->focus_func(obj);
}

EAPI void
elm_widget_focus_steal(Evas_Object *obj)
{
   Evas_Object *parent, *o;
   API_ENTRY return;
   if (sd->focused) return;
   if (sd->disabled) return;
   parent = obj;
   for (;;)
     {
	o = elm_widget_parent_get(parent);
	if (!o) break;
	parent = o;
     }
   elm_widget_focused_object_clear(parent);
   parent = obj;
   for (;;)
     {
	sd = evas_object_smart_data_get(parent);
	sd->focused = 1;
	if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, parent);
	o = elm_widget_parent_get(parent);
	if (!o) break;
	parent = o;
     }
   sd = evas_object_smart_data_get(obj);
   if (sd->focus_func) sd->focus_func(obj);
   return;
}

EAPI void
elm_widget_activate(Evas_Object *obj)
{
   API_ENTRY return;
   elm_widget_change(obj);
   if (sd->activate_func) sd->activate_func(obj);
}

EAPI void
elm_widget_change(Evas_Object *obj)
{
   API_ENTRY return;
   elm_widget_change(elm_widget_parent_get(obj));
   if (sd->on_change_func) sd->on_change_func(sd->on_change_data, obj);
}

EAPI void
elm_widget_disabled_set(Evas_Object *obj, int disabled)
{
   API_ENTRY return;
   if (((sd->disabled) && (disabled)) ||
       ((!sd->disabled) && (!disabled))) return;
   sd->disabled = disabled;
   if (sd->focused)
     {
	Evas_Object *o, *parent;

	parent = obj;
        for (;;)
          {
	     o = elm_widget_parent_get(parent);
	     if (!o) break;
	     parent = o;
	  }
	elm_widget_focus_jump(parent, 1);
     }
   if (sd->disable_func) sd->disable_func(obj);
}

EAPI int
elm_widget_disabled_get(Evas_Object *obj)
{
   API_ENTRY return 0;
   return sd->disabled;
}

EAPI void
elm_widget_show_region_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   API_ENTRY return;
   if ((x == sd->rx) && (y == sd->ry) && (w == sd->rw) && (h == sd->rh)) return;
   sd->rx = x;
   sd->ry = y;
   sd->rw = w;
   sd->rh = h;
   if (sd->on_show_region_func) sd->on_show_region_func(sd->on_show_region_data, obj);
}

EAPI void
elm_widget_show_region_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   API_ENTRY return;
   if (x) *x = sd->rx;
   if (y) *y = sd->ry;
   if (w) *w = sd->rw;
   if (h) *h = sd->rh;
}

/* local subsystem functions */
static void
_smart_reconfigure(Smart_Data *sd)
{
   if (sd->resize_obj)
     {
	evas_object_move(sd->resize_obj, sd->x, sd->y);
	evas_object_resize(sd->resize_obj, sd->w, sd->h);
     }
   if (sd->hover_obj)
     {
	evas_object_move(sd->hover_obj, sd->x, sd->y);
	evas_object_resize(sd->hover_obj, sd->w, sd->h);
     }
}

static void
_smart_add(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = calloc(1, sizeof(Smart_Data));
   if (!sd) return;
   sd->obj = obj;
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   sd->can_focus = 1;
   evas_object_smart_data_set(obj, sd);
}

static void
_smart_del(Evas_Object *obj)
{
   Evas_Object *sobj;
   
   INTERNAL_ENTRY;
   if (sd->del_pre_func) sd->del_pre_func(obj);
   if (sd->resize_obj)
     {
	evas_object_event_callback_del(sd->resize_obj, EVAS_CALLBACK_DEL, _sub_obj_del);
	sd->resize_obj = NULL;
	evas_object_del(sd->resize_obj);
     }
   if (sd->hover_obj)
     {
	evas_object_event_callback_del(sd->hover_obj, EVAS_CALLBACK_DEL, _sub_obj_del);
	sd->hover_obj = NULL;
	evas_object_del(sd->hover_obj);
     }
   while (sd->subobjs)
     {
	sobj = sd->subobjs->data;
	sd->subobjs = eina_list_remove_list(sd->subobjs, sd->subobjs);
	evas_object_event_callback_del(sobj, EVAS_CALLBACK_DEL, _sub_obj_del);
	evas_object_del(sobj);
     }
   if (sd->del_func) sd->del_func(obj);
   free(sd);
}

static void
_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   INTERNAL_ENTRY;
   sd->x = x;
   sd->y = y;
   _smart_reconfigure(sd);
}

static void
_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   INTERNAL_ENTRY;
   sd->w = w;
   sd->h = h;
   _smart_reconfigure(sd);
}

static void
_smart_show(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_show(sd->resize_obj);
}

static void
_smart_hide(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_hide(sd->resize_obj);
}

static void
_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   INTERNAL_ENTRY;
   evas_object_color_set(sd->resize_obj, r, g, b, a);
}

static void
_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   INTERNAL_ENTRY;
   evas_object_clip_set(sd->resize_obj, clip);
}

static void
_smart_clip_unset(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_clip_unset(sd->resize_obj);
}  

/* never need to touch this */

static void
_smart_init(void)
{
   if (_e_smart) return;
     {
	static const Evas_Smart_Class sc =
	  {
	     SMART_NAME,
	       EVAS_SMART_CLASS_VERSION,
	       _smart_add,
	       _smart_del, 
	       _smart_move,
	       _smart_resize,
	       _smart_show,
	       _smart_hide,
	       _smart_color_set,
	       _smart_clip_set,
	       _smart_clip_unset,
	       NULL,
	       NULL,
	       NULL,
	       NULL
	  };
        _e_smart = evas_smart_class_new(&sc);
     }
}

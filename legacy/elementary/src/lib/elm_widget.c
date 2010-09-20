#include <Elementary.h>
#include "elm_priv.h"

static const char SMART_NAME[] = "elm_widget";

#define API_ENTRY \
   Smart_Data *sd = evas_object_smart_data_get(obj); \
   if ((!obj) || (!sd) || (!_elm_widget_is(obj)))
#define INTERNAL_ENTRY \
   Smart_Data *sd = evas_object_smart_data_get(obj); \
   if (!sd) return;

typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{
   Evas_Object   *obj;
   const char    *type;
   Evas_Object   *parent_obj;
   Evas_Coord     x, y, w, h;
   Eina_List     *subobjs;
   Evas_Object   *resize_obj;
   Evas_Object   *hover_obj;
   Eina_List     *tooltips;
   void         (*del_func) (Evas_Object *obj);
   void         (*del_pre_func) (Evas_Object *obj);
   void         (*focus_func) (Evas_Object *obj);
   void         (*activate_func) (Evas_Object *obj);
   void         (*disable_func) (Evas_Object *obj);
   void         (*theme_func) (Evas_Object *obj);
   void         (*signal_func) (Evas_Object *obj, const char *emission,
	                        const char *source);
   void         (*callback_add_func) (Evas_Object *obj, const char *emission,
	                        const char *source, void (*func) (void *data,
				   Evas_Object *o, const char *emission,
				   const char *source), void *data);
   void         *(*callback_del_func) (Evas_Object *obj, const char *emission,
	                          const char *source, void (*func) (void *data,
				     Evas_Object *o, const char *emission,
				     const char *source));
   void         (*changed_func) (Evas_Object *obj);
   void         (*on_focus_func) (void *data, Evas_Object *obj);
   void          *on_focus_data;
   void         (*on_change_func) (void *data, Evas_Object *obj);
   void          *on_change_data;
   void         (*on_show_region_func) (void *data, Evas_Object *obj);
   void          *on_show_region_data;
   void          *data;
   Evas_Coord     rx, ry, rw, rh;
   int            scroll_hold;
   int            scroll_freeze;
   double         scale;
   Elm_Theme     *theme;
   const char    *style;
   unsigned int   focus_order;
   
   int            child_drag_x_locked;
   int            child_drag_y_locked;
   Eina_Bool      drag_x_locked : 1;
   Eina_Bool      drag_y_locked : 1;
   
   Eina_Bool      can_focus : 1;
   Eina_Bool      child_can_focus : 1;
   Eina_Bool      focused : 1;
   Eina_Bool      highlight_ignore : 1;
   Eina_Bool      disabled : 1;
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
static void _smart_calculate(Evas_Object *obj);
static void _smart_init(void);

static void _if_focused_revert(Evas_Object *obj);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;
static Eina_List  *widtypes = NULL;

static unsigned int focus_order = 0;

// internal funcs
static inline Eina_Bool
_elm_widget_is(const Evas_Object *obj)
{
   const char *type = evas_object_type_get(obj);
   return type == SMART_NAME;
}

static void
_sub_obj_del(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Smart_Data *sd = data;

   if (obj == sd->resize_obj)
     sd->resize_obj = NULL;
   else if (obj == sd->hover_obj)
     sd->hover_obj = NULL;
   else
     sd->subobjs = eina_list_remove(sd->subobjs, obj);
   evas_object_smart_callback_call(sd->obj, "sub-object-del", obj);
}

static void
_sub_obj_mouse_down(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *o = obj;
   do 
     {
        if (_elm_widget_is(o)) break;
        o = evas_object_smart_parent_get(o);
     }
   while (o);
   if (!o) return;
   if (!elm_widget_can_focus_get(o)) return;
   elm_widget_focus_steal(o);
}

static void
_propagate_x_drag_lock(Evas_Object *obj, int dir)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   if (sd->parent_obj)
     {
        Smart_Data *sd2 = evas_object_smart_data_get(sd->parent_obj);
        if (sd2)
          {
             sd2->child_drag_x_locked += dir;
             _propagate_x_drag_lock(sd->parent_obj, dir);
          }
     }
}

static void
_propagate_y_drag_lock(Evas_Object *obj, int dir)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   if (sd->parent_obj)
     {
        Smart_Data *sd2 = evas_object_smart_data_get(sd->parent_obj);
        if (sd2)
          {
             sd2->child_drag_y_locked += dir;
             _propagate_y_drag_lock(sd->parent_obj, dir);
          }
     }
}

static void
_parent_focus(Evas_Object *obj)
{
   API_ENTRY return;
   Evas_Object *o = elm_widget_parent_get(obj);
   
   if (sd->focused) return;
   if (o) _parent_focus(o);
   focus_order++;
   sd->focus_order = focus_order;
   sd->focused = 1;
   if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, obj);
   if (sd->focus_func) sd->focus_func(obj);
}

// exposed util funcs to elm
void
_elm_widget_type_clear(void)
{
   const char **ptr;
   
   EINA_LIST_FREE(widtypes, ptr)
     {
        eina_stringshare_del(*ptr);
        *ptr = NULL;
     }
}

// exposed api for making widgets
EAPI void
elm_widget_type_register(const char **ptr)
{
   widtypes = eina_list_append(widtypes, (void *)ptr);
}

EAPI Eina_Bool
elm_widget_api_check(int ver)
{
   if (ver != ELM_INTERNAL_API_VERSION)
     {
        CRITICAL("Elementary widget api versions do not match");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

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
elm_widget_changed_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj))
{
   API_ENTRY return;
   sd->changed_func = func;
}

EAPI void
elm_widget_signal_emit_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj, const char *emission, const char *source))
{
   API_ENTRY return;
   sd->signal_func = func;
}

EAPI void
elm_widget_signal_callback_add_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj, const char *emission, const char *source, void (*func_cb) (void *data, Evas_Object *o, const char *emission, const char *source), void *data))
{
   API_ENTRY return;
   sd->callback_add_func = func;
}

EAPI void
elm_widget_signal_callback_del_hook_set(Evas_Object *obj, void *(*func) (Evas_Object *obj, const char *emission, const char *source, void (*func_cb) (void *data, Evas_Object *o, const char *emission, const char *source)))
{
   API_ENTRY return;
   sd->callback_del_func = func;
}

EAPI void
elm_widget_theme(Evas_Object *obj)
{
   const Eina_List *l;
   Evas_Object *child;
   Elm_Tooltip *tt;

   API_ENTRY return;
   EINA_LIST_FOREACH(sd->subobjs, l, child) elm_widget_theme(child);
   if (sd->resize_obj) elm_widget_theme(sd->resize_obj);
   if (sd->hover_obj) elm_widget_theme(sd->hover_obj);
   EINA_LIST_FOREACH(sd->tooltips, l, tt) elm_tooltip_theme(tt);
   if (sd->theme_func) sd->theme_func(obj);
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
elm_widget_data_get(const Evas_Object *obj)
{
   API_ENTRY return NULL;
   return sd->data;
}

EAPI void
elm_widget_sub_object_add(Evas_Object *obj, Evas_Object *sobj)
{
   API_ENTRY return;
   double scale, pscale = elm_widget_scale_get(sobj);
   Elm_Theme *th, *pth = elm_widget_theme_get(sobj);

   sd->subobjs = eina_list_append(sd->subobjs, sobj);
   if (!sd->child_can_focus)
     {
	if (elm_widget_can_focus_get(sobj)) sd->child_can_focus = 1;
     }
   if (_elm_widget_is(sobj))
     {
	Smart_Data *sd2 = evas_object_smart_data_get(sobj);
	if (sd2)
	  {
	     if (sd2->parent_obj)
                elm_widget_sub_object_del(sd2->parent_obj, sobj);
	     sd2->parent_obj = obj;
	  }
     }
   evas_object_data_set(sobj, "elm-parent", obj);
   evas_object_event_callback_add(sobj, EVAS_CALLBACK_DEL, _sub_obj_del, sd);
   evas_object_smart_callback_call(obj, "sub-object-add", sobj);
   scale = elm_widget_scale_get(sobj);
   th = elm_widget_theme_get(sobj);
   if ((scale != pscale) || (th != pth)) elm_widget_theme(sobj);
}

EAPI void
elm_widget_sub_object_del(Evas_Object *obj, Evas_Object *sobj)
{
   Evas_Object *sobj_parent;
   API_ENTRY return;
   if (!sobj) return;

   sobj_parent = evas_object_data_del(sobj, "elm-parent");
   if (sobj_parent != obj)
     {
	static int abort_on_warn = -1;
	ERR("removing sub object %p from parent %p, "
	    "but elm-parent is different %p!",
	    sobj, obj, sobj_parent);
	if (EINA_UNLIKELY(abort_on_warn == -1))
	  {
	     if (getenv("ELM_ERROR_ABORT")) abort_on_warn = 1;
	     else abort_on_warn = 0;
	  }
	if (abort_on_warn == 1) abort();
     }
   sd->subobjs = eina_list_remove(sd->subobjs, sobj);
   if (!sd->child_can_focus)
     {
	if (elm_widget_can_focus_get(sobj)) sd->child_can_focus = 0;
     }
   if (_elm_widget_is(sobj))
     {
	Smart_Data *sd2 = evas_object_smart_data_get(sobj);
	if (sd2) sd2->parent_obj = NULL;
     }
   evas_object_event_callback_del_full(sobj, EVAS_CALLBACK_DEL, 
                                       _sub_obj_del, sd);
   evas_object_smart_callback_call(obj, "sub-object-del", sobj);
}

EAPI void
elm_widget_resize_object_set(Evas_Object *obj, Evas_Object *sobj)
{
   API_ENTRY return;
   if (sd->resize_obj)
     {
	evas_object_data_del(sd->resize_obj, "elm-parent");
	if (_elm_widget_is(sd->resize_obj))
	  {
	     Smart_Data *sd2 = evas_object_smart_data_get(sd->resize_obj);
	     if (sd2) sd2->parent_obj = NULL;
	  }
	evas_object_event_callback_del_full(sd->resize_obj, EVAS_CALLBACK_DEL,
                                            _sub_obj_del, sd);
	evas_object_event_callback_del_full(sd->resize_obj, EVAS_CALLBACK_MOUSE_DOWN,
                                            _sub_obj_mouse_down, sd);
	evas_object_smart_member_del(sd->resize_obj);
     }
   sd->resize_obj = sobj;
   if (sd->resize_obj)
     {
	if (_elm_widget_is(sd->resize_obj))
	  {
	     Smart_Data *sd2 = evas_object_smart_data_get(sd->resize_obj);
	     if (sd2) sd2->parent_obj = obj;
	  }
	evas_object_clip_set(sobj, evas_object_clip_get(obj));
	evas_object_smart_member_add(sobj, obj);
	evas_object_event_callback_add(sobj, EVAS_CALLBACK_DEL,
                                       _sub_obj_del, sd);
	evas_object_event_callback_add(sobj, EVAS_CALLBACK_MOUSE_DOWN,
                                       _sub_obj_mouse_down, sd);
	_smart_reconfigure(sd);
	evas_object_data_set(sobj, "elm-parent", obj);
	evas_object_smart_callback_call(obj, "sub-object-add", sobj);
     }
}

EAPI void
elm_widget_hover_object_set(Evas_Object *obj, Evas_Object *sobj)
{
   API_ENTRY return;
   if (sd->hover_obj)
     {
	evas_object_event_callback_del_full(sd->hover_obj, EVAS_CALLBACK_DEL,
           _sub_obj_del, sd);
     }
   sd->hover_obj = sobj;
   if (sd->hover_obj)
     {
	evas_object_event_callback_add(sobj, EVAS_CALLBACK_DEL,
                                       _sub_obj_del, sd);
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
elm_widget_can_focus_get(const Evas_Object *obj)
{
   API_ENTRY return 0;
   if (sd->can_focus) return 1;
   if (sd->child_can_focus) return 1;
   return 0;
}

EAPI void
elm_widget_highlight_ignore_set(Evas_Object *obj, Eina_Bool ignore)
{
   API_ENTRY return;
   sd->highlight_ignore = !!ignore;
}

EAPI Eina_Bool
elm_widget_highlight_ignore_get(const Evas_Object *obj)
{
   API_ENTRY return EINA_FALSE;
   return sd->highlight_ignore;
}

EAPI int
elm_widget_focus_get(const Evas_Object *obj)
{
   API_ENTRY return 0;
   return sd->focused;
}

EAPI Evas_Object *
elm_widget_focused_object_get(const Evas_Object *obj)
{
   const Evas_Object *subobj;
   const Eina_List *l;
   API_ENTRY return NULL;

   if (!sd->focused) return NULL;
   EINA_LIST_FOREACH(sd->subobjs, l, subobj)
     {
	Evas_Object *fobj = elm_widget_focused_object_get(subobj);
	if (fobj) return fobj;
     }
   return (Evas_Object *)obj;
}

EAPI Evas_Object *
elm_widget_top_get(const Evas_Object *obj)
{
#if 1 // strict way  
   API_ENTRY return NULL;
   if (sd->parent_obj) return elm_widget_top_get(sd->parent_obj);
   return (Evas_Object *)obj;
#else // loose way
   Smart_Data *sd = evas_object_smart_data_get(obj);
   Evas_Object *par;
   
   if (!obj) return NULL;
   if ((sd) && _elm_widget_is(obj))
     {
        if ((sd->type) && (!strcmp(sd->type, "win"))) 
           return (Evas_Object *)obj;
        if (sd->parent_obj)
           return elm_widget_top_get(sd->parent_obj);
     }
   par = evas_object_smart_parent_get(obj);
   if (!par) return (Evas_Object *)obj;
   return elm_widget_top_get(par);
#endif   
}

EAPI Eina_Bool
elm_widget_is(const Evas_Object *obj)
{
   return _elm_widget_is(obj);
}

EAPI Evas_Object *
elm_widget_parent_widget_get(const Evas_Object *obj)
{
   Evas_Object *parent;

   if (_elm_widget_is(obj))
     {
	Smart_Data *sd = evas_object_smart_data_get(obj);
	if (!sd) return NULL;
	parent = sd->parent_obj;
     }
   else
     {
	parent = evas_object_data_get(obj, "elm-parent");
	if (!parent) parent = evas_object_smart_parent_get(obj);
     }

   while (parent)
     {
	Evas_Object *elm_parent;
	if (_elm_widget_is(parent)) break;
	elm_parent = evas_object_data_get(parent, "elm-parent");
        if (elm_parent) parent = elm_parent;
	else parent = evas_object_smart_parent_get(parent);
     }
   return parent;
}

EAPI int
elm_widget_focus_jump(Evas_Object *obj, int forward)
{
   API_ENTRY return 0;
   if (!elm_widget_can_focus_get(obj)) return 0;

   /* if it has a focus func its an end-point widget like a button */
   if (sd->focus_func)
     {
	if (!sd->focused)
          {
             focus_order++;
             sd->focus_order = focus_order;
             sd->focused = 1;
          }
	else sd->focused = 0;
	if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, obj);
	sd->focus_func(obj);
	return sd->focused;
     }
   /* its some container */
   else
     {
	int focus_next;
	int noloop = 0;
        
	focus_next = 0;
	if (!sd->focused)
	  {
	     elm_widget_focus_set(obj, forward);
	     return 1;
	  }
	else
	  {
	     if (forward)
	       {
		  if (elm_widget_can_focus_get(sd->resize_obj))
		    {
		       if ((focus_next) &&
			   (!elm_widget_disabled_get(sd->resize_obj)))
			 {
			    /* the previous focused item was unfocused - so focus
			     * the next one (that can be focused) */
			    if (elm_widget_focus_jump(sd->resize_obj, forward))
                               return 1;
			    else noloop = 1;
			 }
		       else
			 {
			    if (elm_widget_focus_get(sd->resize_obj))
			      {
				 /* jump to the next focused item or focus this item */
				 if (elm_widget_focus_jump(sd->resize_obj, forward))
                                    return 1;
				 /* it returned 0 - it got to the last item and is past it */
				 focus_next = 1;
			      }
			 }
		    }
		  if (!noloop)
		    {
		       const Eina_List *l;
		       Evas_Object *child;
		       EINA_LIST_FOREACH(sd->subobjs, l, child)
			 {
			    if (elm_widget_can_focus_get(child))
			      {
				 if ((focus_next) &&
				     (!elm_widget_disabled_get(child)))
				   {
				      /* the previous focused item was unfocused - so focus
				       * the next one (that can be focused) */
				      if (elm_widget_focus_jump(child, forward))
                                         return 1;
				      else break;
				   }
				 else
				   {
				      if (elm_widget_focus_get(child))
					{
					   /* jump to the next focused item or focus this item */
					   if (elm_widget_focus_jump(child, forward))
                                              return 1;
					   /* it returned 0 - it got to the last item and is past it */
					   focus_next = 1;
					}
				   }
			      }
			 }
		    }
	       }
	     else
	       {
		  const Eina_List *l;
		  Evas_Object *child;

		  EINA_LIST_REVERSE_FOREACH(sd->subobjs, l, child)
		    {
		       if (elm_widget_can_focus_get(child))
			 {
			    if ((focus_next) &&
				(!elm_widget_disabled_get(child)))
			      {
				 /* the previous focused item was unfocused - so focus
				  * the next one (that can be focused) */
				 if (elm_widget_focus_jump(child, forward))
                                    return 1;
				 else break;
			      }
			    else
			      {
				 if (elm_widget_focus_get(child))
				   {
				      /* jump to the next focused item or focus this item */
				      if (elm_widget_focus_jump(child, forward))
                                         return 1;
				      /* it returned 0 - it got to the last item and is past it */
				      focus_next = 1;
				   }
			      }
			 }
		    }
		  if (!l)
		    {
		       if (elm_widget_can_focus_get(sd->resize_obj))
			 {
			    if ((focus_next) &&
				(!elm_widget_disabled_get(sd->resize_obj)))
			      {
				 /* the previous focused item was unfocused - so focus
				  * the next one (that can be focused) */
				 if (elm_widget_focus_jump(sd->resize_obj, forward))
                                    return 1;
			      }
			    else
			      {
				 if (elm_widget_focus_get(sd->resize_obj))
				   {
				      /* jump to the next focused item or focus this item */
				      if (elm_widget_focus_jump(sd->resize_obj, forward))
                                         return 1;
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
   if (sd->focused)
     {
	sd->focused = 0;
	if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, obj);
     }
   return 0;
}

EAPI void
elm_widget_signal_emit(Evas_Object *obj, const char *emission, const char *source)
{
   API_ENTRY return;
   if (!sd->signal_func) return;
   sd->signal_func(obj, emission, source);
}

EAPI void
elm_widget_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source), void *data)
{
   API_ENTRY return;
   if (!sd->callback_add_func) return;
   sd->callback_add_func(obj, emission, source, func, data);
}

EAPI void *
elm_widget_signal_callback_del(Evas_Object *obj, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source))
{
   API_ENTRY return NULL;
   if (!sd->callback_del_func) return NULL;
   return sd->callback_del_func(obj, emission, source, func);
}

EAPI void
elm_widget_focus_set(Evas_Object *obj, int first)
{
   API_ENTRY return;
   if (!sd->focused)
     {
        focus_order++;
        sd->focus_order = focus_order;
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
	if (first)
	  {
	     if ((elm_widget_can_focus_get(sd->resize_obj)) &&
		 (!elm_widget_disabled_get(sd->resize_obj)))
	       {
		  elm_widget_focus_set(sd->resize_obj, first);
	       }
	     else
	       {
		  const Eina_List *l;
		  Evas_Object *child;
		  EINA_LIST_FOREACH(sd->subobjs, l, child)
		    {
		       if ((elm_widget_can_focus_get(child)) &&
			   (!elm_widget_disabled_get(child)))
			 {
			    elm_widget_focus_set(child, first);
			    break;
			 }
		    }
	       }
	  }
	else
	  {
	     const Eina_List *l;
	     Evas_Object *child;
	     EINA_LIST_REVERSE_FOREACH(sd->subobjs, l, child)
	       {
		  if ((elm_widget_can_focus_get(child)) &&
		      (!elm_widget_disabled_get(child)))
		    {
		       elm_widget_focus_set(child, first);
		       break;
		    }
	       }
	     if (!l)
	       {
		  if ((elm_widget_can_focus_get(sd->resize_obj)) &&
		      (!elm_widget_disabled_get(sd->resize_obj)))
		    {
		       elm_widget_focus_set(sd->resize_obj, first);
		    }
	       }
	  }
     }
}

EAPI Evas_Object *
elm_widget_parent_get(const Evas_Object *obj)
{
   API_ENTRY return NULL;
   return sd->parent_obj;
}

EAPI void
elm_widget_focused_object_clear(Evas_Object *obj)
{
   API_ENTRY return;
   if (!sd->focused) return;
   if (elm_widget_focus_get(sd->resize_obj))
      elm_widget_focused_object_clear(sd->resize_obj);
   else
     {
	const Eina_List *l;
	Evas_Object *child;
	EINA_LIST_FOREACH(sd->subobjs, l, child)
	  {
	     if (elm_widget_focus_get(child))
	       {
		  elm_widget_focused_object_clear(child);
		  break;
	       }
	  }
     }
   sd->focused = 0;
   if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, obj);
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
	sd = evas_object_smart_data_get(o);
	if (sd->focused) break;
	parent = o;
     }
   if (!elm_widget_parent_get(parent))
     elm_widget_focused_object_clear(parent);
   else
     {
	parent = elm_widget_parent_get(parent);
	sd = evas_object_smart_data_get(parent);
	if (elm_widget_focus_get(sd->resize_obj))
          {
             elm_widget_focused_object_clear(sd->resize_obj);
          }
	else
	  {
	     const Eina_List *l;
	     Evas_Object *child;
	     EINA_LIST_FOREACH(sd->subobjs, l, child)
	       {
		  if (elm_widget_focus_get(child))
		    {
		       elm_widget_focused_object_clear(child);
		       break;
		    }
	       }
	  }
     }
   _parent_focus(obj);
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

   if (sd->disabled == disabled) return;
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
elm_widget_disabled_get(const Evas_Object *obj)
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
   if (sd->on_show_region_func)
      sd->on_show_region_func(sd->on_show_region_data, obj);
}

EAPI void
elm_widget_show_region_get(const Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   API_ENTRY return;
   if (x) *x = sd->rx;
   if (y) *y = sd->ry;
   if (w) *w = sd->rw;
   if (h) *h = sd->rh;
}

EAPI void
elm_widget_scroll_hold_push(Evas_Object *obj)
{
   API_ENTRY return;
   sd->scroll_hold++;
   if (sd->scroll_hold == 1)
      evas_object_smart_callback_call(obj, "scroll-hold-on", obj);
   if (sd->parent_obj) elm_widget_scroll_hold_push(sd->parent_obj);
   // FIXME: on delete/reparent hold pop
}

EAPI void
elm_widget_scroll_hold_pop(Evas_Object *obj)
{
   API_ENTRY return;
   sd->scroll_hold--;
   if (sd->scroll_hold < 0) sd->scroll_hold = 0;
   if (sd->scroll_hold == 0)
      evas_object_smart_callback_call(obj, "scroll-hold-off", obj);
   if (sd->parent_obj) elm_widget_scroll_hold_pop(sd->parent_obj);
}

EAPI int
elm_widget_scroll_hold_get(const Evas_Object *obj)
{
   API_ENTRY return 0;
   return sd->scroll_hold;
}

EAPI void
elm_widget_scroll_freeze_push(Evas_Object *obj)
{
   API_ENTRY return;
   sd->scroll_freeze++;
   if (sd->scroll_freeze == 1)
      evas_object_smart_callback_call(obj, "scroll-freeze-on", obj);
   if (sd->parent_obj) elm_widget_scroll_freeze_push(sd->parent_obj);
   // FIXME: on delete/reparent freeze pop
}

EAPI void
elm_widget_scroll_freeze_pop(Evas_Object *obj)
{
   API_ENTRY return;
   sd->scroll_freeze--;
   if (sd->scroll_freeze < 0) sd->scroll_freeze = 0;
   if (sd->scroll_freeze == 0)
      evas_object_smart_callback_call(obj, "scroll-freeze-off", obj);
   if (sd->parent_obj) elm_widget_scroll_freeze_pop(sd->parent_obj);
}

EAPI int
elm_widget_scroll_freeze_get(const Evas_Object *obj)
{
   API_ENTRY return 0;
   return sd->scroll_freeze;
}

EAPI void
elm_widget_scale_set(Evas_Object *obj, double scale)
{
   API_ENTRY return;
   if (scale <= 0.0) scale = 0.0;
   if (sd->scale != scale)
     {
	sd->scale = scale;
	elm_widget_theme(obj);
     }
}

EAPI double
elm_widget_scale_get(const Evas_Object *obj)
{
   API_ENTRY return 1.0;
   // FIXME: save walking up the tree by storing/caching parent scale
   if (sd->scale == 0.0)
     {
	if (sd->parent_obj)
           return elm_widget_scale_get(sd->parent_obj);
	else
           return 1.0;
     }
   return sd->scale;
}

EAPI void
elm_widget_theme_set(Evas_Object *obj, Elm_Theme *th)
{
   API_ENTRY return;
   if (sd->theme != th)
     {
        if (sd->theme) elm_theme_free(sd->theme);
        sd->theme = th;
        if (th) th->ref++;
        elm_widget_theme(obj);
     }
}

EAPI Elm_Theme *
elm_widget_theme_get(const Evas_Object *obj)
{
   API_ENTRY return NULL;
   if (!sd->theme)
     {
        if (sd->parent_obj)
           return elm_widget_theme_get(sd->parent_obj);
        else
           return NULL;
     }
   return sd->theme;
}

EAPI void
elm_widget_style_set(Evas_Object *obj, const char *style)
{
   API_ENTRY return;
   
   if (eina_stringshare_replace(&sd->style, style))
      elm_widget_theme(obj);
}

EAPI const char *
elm_widget_style_get(const Evas_Object *obj)
{
   API_ENTRY return "";
   if (sd->style) return sd->style;
   return "default";
}

EAPI void
elm_widget_type_set(Evas_Object *obj, const char *type)
{
   API_ENTRY return;
   eina_stringshare_replace(&sd->type, type);
}

EAPI const char *
elm_widget_type_get(const Evas_Object *obj)
{
   API_ENTRY return "";
   if (sd->type) return sd->type;
   return "";
}

EAPI void
elm_widget_tooltip_add(Evas_Object *obj, Elm_Tooltip *tt)
{
   API_ENTRY return;
   sd->tooltips = eina_list_append(sd->tooltips, tt);
}

EAPI void
elm_widget_tooltip_del(Evas_Object *obj, Elm_Tooltip *tt)
{
   API_ENTRY return;
   sd->tooltips = eina_list_remove(sd->tooltips, tt);
}

EAPI void
elm_widget_drag_lock_x_set(Evas_Object *obj, Eina_Bool lock)
{
   API_ENTRY return;
   if (sd->drag_x_locked == lock) return;
   sd->drag_x_locked = lock;
   if (sd->drag_x_locked) _propagate_x_drag_lock(obj, 1);
   else _propagate_x_drag_lock(obj, -1);
}

EAPI void
elm_widget_drag_lock_y_set(Evas_Object *obj, Eina_Bool lock)
{
   API_ENTRY return;
   if (sd->drag_y_locked == lock) return;
   sd->drag_y_locked = lock;
   if (sd->drag_y_locked) _propagate_y_drag_lock(obj, 1);
   else _propagate_y_drag_lock(obj, -1);
}

EAPI Eina_Bool
elm_widget_drag_lock_x_get(const Evas_Object *obj)
{
   API_ENTRY return EINA_FALSE;
   return sd->drag_x_locked;
}

EAPI Eina_Bool
elm_widget_drag_lock_y_get(const Evas_Object *obj)
{
   API_ENTRY return EINA_FALSE;
   return sd->drag_y_locked;
}

EAPI int
elm_widget_drag_child_locked_x_get(const Evas_Object *obj)
{
   API_ENTRY return 0;
   return sd->child_drag_x_locked;
}

EAPI int
elm_widget_drag_child_locked_y_get(const Evas_Object *obj)
{
   API_ENTRY return 0;
   return sd->child_drag_y_locked;
}

EAPI int
elm_widget_theme_object_set(Evas_Object *obj, Evas_Object *edj, const char *wname, const char *welement, const char *wstyle)
{
   return _elm_theme_object_set(obj, edj, wname, welement, wstyle);
}

EAPI Eina_Bool
elm_widget_type_check(const Evas_Object *obj, const char *type)
{
   const char *provided, *expected = "(unknown)";
   static int abort_on_warn = -1;
   provided = elm_widget_type_get(obj);
   if (EINA_LIKELY(provided == type)) return EINA_TRUE;
   if (type) expected = type;
   if ((!provided) || (provided[0] == 0))
     {
        provided = evas_object_type_get(obj);
        if ((!provided) || (provided[0] == 0))
           provided = "(unknown)";
     }
   ERR("Passing Object: %p, of type: '%s' when expecting type: '%s'", obj, provided, expected);
   if (abort_on_warn == -1)
     {
        if (getenv("ELM_ERROR_ABORT")) abort_on_warn = 1;
        else abort_on_warn = 0;
     }
   if (abort_on_warn == 1) abort();
   return EINA_FALSE;
}

EAPI Eina_List *
elm_widget_stringlist_get(const char *str)
{
   Eina_List *list = NULL;
   const char *s, *b;
   if (!str) return NULL;
   for (b = s = str; 1; s++)
     {
	if ((*s == ' ') || (*s == 0))
	  {
	     char *t = malloc(s - b + 1);
	     if (t)
	       {
		  strncpy(t, b, s - b);
		  t[s - b] = 0;
		  list = eina_list_append(list, eina_stringshare_add(t));
		  free(t);
	       }
	     b = s + 1;
	  }
	if (*s == 0) break;
     }
   return list;
}

EAPI void
elm_widget_stringlist_free(Eina_List *list)
{
   const char *s;
   EINA_LIST_FREE(list, s) eina_stringshare_del(s);
}

/**
 * Allocate a new Elm_Widget_Item-derived structure.
 *
 * The goal of this structure is to provide common ground for actions
 * that a widget item have, such as the owner widget, callback to
 * notify deletion, data pointer and maybe more.
 *
 * @param widget the owner widget that holds this item, must be an elm_widget!
 * @param alloc_size any number greater than sizeof(Elm_Widget_Item) that will
 *        be used to allocate memory.
 *
 * @return allocated memory that is already zeroed out, or NULL on errors.
 *
 * @see elm_widget_item_new() convenience macro.
 * @see elm_widget_item_del() to release memory.
 */
EAPI Elm_Widget_Item *
_elm_widget_item_new(Evas_Object *widget, size_t alloc_size)
{
   Elm_Widget_Item *item;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(alloc_size < sizeof(Elm_Widget_Item), NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!_elm_widget_is(widget), NULL);

   item = calloc(1, alloc_size);
   EINA_SAFETY_ON_NULL_RETURN_VAL(item, NULL);

   EINA_MAGIC_SET(item, ELM_WIDGET_ITEM_MAGIC);
   item->widget = widget;
   return item;
}

/**
 * Releases widget item memory, calling back del_cb() if it exists.
 *
 * If there is a Elm_Widget_Item::del_cb, then it will be called prior
 * to memory release. Note that elm_widget_item_pre_notify_del() calls
 * this function and then unset it, thus being useful for 2 step
 * cleanup whenever the del_cb may use any of the data that must be
 * deleted from item.
 *
 * The Elm_Widget_Item::view will be deleted (evas_object_del()) if it
 * is presented!
 *
 * @param item a valid #Elm_Widget_Item to be deleted.
 * @see elm_widget_item_del() convenience macro.
 */
EAPI void
_elm_widget_item_del(Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);

   if (item->del_cb)
     item->del_cb((void *)item->data, item->widget, item);

   if (item->view)
     evas_object_del(item->view);

   EINA_MAGIC_SET(item, EINA_MAGIC_NONE);
   free(item);
}

/**
 * Notify object will be deleted without actually deleting it.
 *
 * This function will callback Elm_Widget_Item::del_cb if it is set
 * and then unset it so it is not called twice (ie: from
 * elm_widget_item_del()).
 *
 * @param item a valid #Elm_Widget_Item to be notified
 * @see elm_widget_item_pre_notify_del() convenience macro.
 */
EAPI void
_elm_widget_item_pre_notify_del(Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   if (!item->del_cb) return;
   item->del_cb((void *)item->data, item->widget, item);
   item->del_cb = NULL;
}

/**
 * Set the function to notify when item is being deleted.
 *
 * This function will complain if there was a callback set already,
 * however it will set the new one.
 *
 * The callback will be called from elm_widget_item_pre_notify_del()
 * or elm_widget_item_del() will be called with:
 *   - data: the Elm_Widget_Item::data value.
 *   - obj: the Elm_Widget_Item::widget evas object.
 *   - event_info: the item being deleted.
 *
 * @param item a valid #Elm_Widget_Item to be notified
 * @see elm_widget_item_del_cb_set() convenience macro.
 */
EAPI void
_elm_widget_item_del_cb_set(Elm_Widget_Item *item, Evas_Smart_Cb del_cb)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);

   if ((item->del_cb) && (item->del_cb != del_cb))
     WRN("You're replacing a previously set del_cb %p of item %p with %p",
         item->del_cb, item, del_cb);

   item->del_cb = del_cb;
}

/**
 * Set user-data in this item.
 *
 * User data may be used to identify this item or just store any
 * application data. It is automatically given as the first parameter
 * of the deletion notify callback.
 *
 * @param item a valid #Elm_Widget_Item to store data in.
 * @param data user data to store.
 * @see elm_widget_item_del_cb_set() convenience macro.
 */
EAPI void
_elm_widget_item_data_set(Elm_Widget_Item *item, const void *data)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   if ((item->data) && (item->data != data))
     DBG("Replacing item %p data %p with %p", item, item->data, data);
   item->data = data;
}

/**
 * Retrieves user-data of this item.
 *
 * @param item a valid #Elm_Widget_Item to get data from.
 * @see elm_widget_item_data_set()
 */
EAPI void *
_elm_widget_item_data_get(const Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   return (void *)item->data;
}

typedef struct _Elm_Widget_Item_Tooltip Elm_Widget_Item_Tooltip;

struct _Elm_Widget_Item_Tooltip
{
   Elm_Widget_Item             *item;
   Elm_Tooltip_Item_Content_Cb  func;
   Evas_Smart_Cb                del_cb;
   const void                  *data;
};

static Evas_Object *
_elm_widget_item_tooltip_label_create(void *data, Evas_Object *obj, void *item __UNUSED__)
{
   Evas_Object *label = elm_label_add(obj);
   if (!label)
     return NULL;
   elm_object_style_set(label, "tooltip");
   elm_label_label_set(label, data);
   return label;
}

static void
_elm_widget_item_tooltip_label_del_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   eina_stringshare_del(data);
}

/**
 * Set the text to be shown in the widget item.
 *
 * @param item Target item
 * @param text The text to set in the content
 *
 * Setup the text as tooltip to object. The item can have only one tooltip,
 * so any previous tooltip data is removed.
 *
 * @internal
 */
EAPI void
_elm_widget_item_tooltip_text_set(Elm_Widget_Item *item, const char *text)
{
   EINA_SAFETY_ON_NULL_RETURN(item);
   EINA_SAFETY_ON_NULL_RETURN(text);

   text = eina_stringshare_add(text);
   _elm_widget_item_tooltip_content_cb_set
     (item, _elm_widget_item_tooltip_label_create, text,
      _elm_widget_item_tooltip_label_del_cb);
}

static Evas_Object *
_elm_widget_item_tooltip_create(void *data, Evas_Object *obj)
{
   Elm_Widget_Item_Tooltip *wit = data;
   return wit->func((void *)wit->data, obj, wit->item);
}

static void
_elm_widget_item_tooltip_del_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Widget_Item_Tooltip *wit = data;
   if (wit->del_cb) wit->del_cb((void *)wit->data, obj, wit->item);
   free(wit);
}

/**
 * Set the content to be shown in the tooltip item
 *
 * Setup the tooltip to item. The item can have only one tooltip,
 * so any previous tooltip data is removed. @p func(with @p data) will
 * be called every time that need show the tooltip and it should
 * return a valid Evas_Object. This object is then managed fully by
 * tooltip system and is deleted when the tooltip is gone.
 *
 * @param item the widget item being attached a tooltip.
 * @param func the function used to create the tooltip contents.
 * @param data what to provide to @a func as callback data/context.
 * @param del_cb called when data is not needed anymore, either when
 *        another callback replaces @func, the tooltip is unset with
 *        elm_widget_item_tooltip_unset() or the owner @a item
 *        dies. This callback receives as the first parameter the
 *        given @a data, and @c event_info is the item.
 *
 * @internal
 */
EAPI void
_elm_widget_item_tooltip_content_cb_set(Elm_Widget_Item *item, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb)
{
   Elm_Widget_Item_Tooltip *wit;

   ELM_WIDGET_ITEM_CHECK_OR_GOTO(item, error);

   if (!func)
     {
        _elm_widget_item_tooltip_unset(item);
        return;
     }

   wit = ELM_NEW(Elm_Widget_Item_Tooltip);
   if (!wit) goto error;
   wit->item = item;
   wit->func = func;
   wit->data = data;
   wit->del_cb = del_cb;

   elm_object_sub_tooltip_content_cb_set
     (item->view, item->widget, _elm_widget_item_tooltip_create, wit,
      _elm_widget_item_tooltip_del_cb);

   return;

 error:
   if (del_cb) del_cb((void *)data, item->widget, item);
}

/**
 * Unset tooltip from item
 *
 * @param item widget item to remove previously set tooltip.
 *
 * Remove tooltip from item. The callback provided as del_cb to
 * elm_widget_item_tooltip_content_cb_set() will be called to notify
 * it is not used anymore.
 *
 * @see elm_widget_item_tooltip_content_cb_set()
 *
 * @internal
 */
EAPI void
_elm_widget_item_tooltip_unset(Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   elm_object_tooltip_unset(item->view);
}

/**
 * Sets a different style for this item tooltip.
 *
 * @note before you set a style you should define a tooltip with
 *       elm_widget_item_tooltip_content_cb_set() or
 *       elm_widget_item_tooltip_text_set()
 *
 * @param item widget item with tooltip already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @internal
 */
EAPI void
_elm_widget_item_tooltip_style_set(Elm_Widget_Item *item, const char *style)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   elm_object_tooltip_style_set(item->view, style);
}

/**
 * Get the style for this item tooltip.
 *
 * @param item widget item with tooltip already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a tooltip set, then NULL is returned.
 *
 * @internal
 */
EAPI const char *
_elm_widget_item_tooltip_style_get(const Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item, NULL);
   return elm_object_tooltip_style_get(item->view);
}

EAPI void
_elm_widget_item_cursor_set(Elm_Widget_Item *item, const char *cursor)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   elm_object_cursor_set(item->view, cursor);
}

EAPI void
_elm_widget_item_cursor_unset(Elm_Widget_Item *item)
{
   ELM_WIDGET_ITEM_CHECK_OR_RETURN(item);
   elm_object_cursor_unset(item->view);
}

// smart object funcs
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
   sd->x = sd->y = sd->w = sd->h = 0;
   sd->can_focus = 1;
   evas_object_smart_data_set(obj, sd);
}

static Evas_Object *
_newest_focus_order_get(Evas_Object *obj, unsigned int *newest_focus_order)
{
   const Eina_List *l;
   Evas_Object *child, *ret, *best;
   
   API_ENTRY return NULL;
   if (!evas_object_visible_get(obj)) return NULL;
   best = NULL;
   if (*newest_focus_order < sd->focus_order)
     {
        *newest_focus_order = sd->focus_order;
        best = obj;
     }
   EINA_LIST_FOREACH(sd->subobjs, l, child)
     {
        ret = _newest_focus_order_get(child, newest_focus_order);
        if (!ret) continue;
        best = ret;
     }
   return best;
}

static void
_if_focused_revert(Evas_Object *obj)
{
   Evas_Object *top;
   Evas_Object *newest = NULL;
   unsigned int newest_focus_order = 0;
   
   INTERNAL_ENTRY;
   
   if (!sd->focused) return;
   if (!sd->parent_obj) return;

   top = elm_widget_top_get(sd->parent_obj);
   if (top)
     {
        newest = _newest_focus_order_get(top, &newest_focus_order);
        if (newest)
          {
             elm_object_unfocus(newest);
             elm_object_focus(newest);
          }
     }
}

static void
_smart_del(Evas_Object *obj)
{
   Evas_Object *sobj;

   INTERNAL_ENTRY;
   if (sd->del_pre_func) sd->del_pre_func(obj);
   if (sd->resize_obj)
     {
	sobj = sd->resize_obj;
	sd->resize_obj = NULL;
	evas_object_event_callback_del_full(sobj, EVAS_CALLBACK_DEL, _sub_obj_del, sd);
	evas_object_smart_callback_call(sd->obj, "sub-object-del", sobj);
	evas_object_del(sobj);
     }
   if (sd->hover_obj)
     {
	sobj = sd->hover_obj;
	sd->hover_obj = NULL;
	evas_object_event_callback_del_full(sobj, EVAS_CALLBACK_DEL, _sub_obj_del, sd);
	evas_object_smart_callback_call(sd->obj, "sub-object-del", sobj);
	evas_object_del(sobj);
     }
   EINA_LIST_FREE(sd->subobjs, sobj)
     {
	evas_object_event_callback_del_full(sobj, EVAS_CALLBACK_DEL, _sub_obj_del, sd);
	evas_object_smart_callback_call(sd->obj, "sub-object-del", sobj);
	evas_object_del(sobj);
     }
   eina_list_free(sd->tooltips); /* should be empty anyway */
   if (sd->del_func) sd->del_func(obj);
   if (sd->style) eina_stringshare_del(sd->style);
   if (sd->type) eina_stringshare_del(sd->type);
   if (sd->theme) elm_theme_free(sd->theme);
   _if_focused_revert(obj);
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
   Eina_List *list, *l;
   Evas_Object *o;
   INTERNAL_ENTRY;
   list = evas_object_smart_members_get(obj);
   EINA_LIST_FOREACH(list, l, o)
    {
        if (evas_object_data_get(o, "_elm_leaveme")) continue;
        evas_object_show(o);
    }
}

static void
_smart_hide(Evas_Object *obj)
{
   Eina_List *list, *l;
   Evas_Object *o;
   INTERNAL_ENTRY;
   list = evas_object_smart_members_get(obj);
   EINA_LIST_FOREACH(list, l, o)
     {
        if (evas_object_data_get(o, "_elm_leaveme")) continue;
        evas_object_hide(o);
     }
   _if_focused_revert(obj);
}

static void
_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   Eina_List *list, *l;
   Evas_Object *o;
   INTERNAL_ENTRY;
   list = evas_object_smart_members_get(obj);
   EINA_LIST_FOREACH(list, l, o)
     {
        if (evas_object_data_get(o, "_elm_leaveme")) continue;
        evas_object_color_set(o, r, g, b, a);
     }
}

static void
_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   Eina_List *list, *l;
   Evas_Object *o;
   INTERNAL_ENTRY;
   list = evas_object_smart_members_get(obj);
   EINA_LIST_FOREACH(list, l, o)
     {
        if (evas_object_data_get(o, "_elm_leaveme")) continue;
        evas_object_clip_set(o, clip);
     }
}

static void
_smart_clip_unset(Evas_Object *obj)
{
   Eina_List *list, *l;
   Evas_Object *o;
   INTERNAL_ENTRY;
//   evas_object_clip_unset(sd->resize_obj);
//   return;
   list = evas_object_smart_members_get(obj);
   EINA_LIST_FOREACH(list, l, o)
     {
        if (evas_object_data_get(o, "_elm_leaveme")) continue;
        evas_object_clip_unset(o);
     }
}

static void
_smart_calculate(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   if (sd->changed_func) sd->changed_func(obj);
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
             _smart_calculate,
             NULL,
             NULL,
             NULL,
             NULL,
             NULL,
             NULL
	  };
	_e_smart = evas_smart_class_new(&sc);
     }
}

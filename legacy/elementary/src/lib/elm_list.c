#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup List List
 *
 * A list is a very simple type of list widget.  For more robust
 * lists, @ref Genlist should probably be used.
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scr, *box, *self;
   Eina_List *items, *selected, *to_delete;
   Elm_List_Mode mode;
   Evas_Coord minw[2], minh[2];
   int walking;
   Eina_Bool fix_pending : 1;
   Eina_Bool on_hold : 1;
   Eina_Bool multi : 1;
   Eina_Bool always_select : 1;
   Eina_Bool longpressed : 1;
   Eina_Bool wasselected : 1;
};

struct _Elm_List_Item
{
   Eina_List *node;
   Evas_Object *obj, *base;
   const char *label;
   Evas_Object *icon, *end;
   Evas_Smart_Cb func;
   Evas_Smart_Cb del_cb;
   const void *data;
   Ecore_Timer *long_timer;
   Eina_Bool deleted : 1;
   Eina_Bool even : 1;
   Eina_Bool is_even : 1;
   Eina_Bool fixed : 1;
   Eina_Bool selected : 1;
   Eina_Bool hilighted : 1;
   Eina_Bool dummy_icon : 1;
   Eina_Bool dummy_end : 1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _on_focus_hook(void *data, Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _fix_items(Evas_Object *obj);
static void _mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _mouse_up(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _mouse_move(void *data, Evas *evas, Evas_Object *obj, void *event_info);

#define ELM_LIST_ITEM_CHECK_DELETED_RETURN(it, ...)			\
  if (!it)								\
    {									\
       fprintf(stderr, "ERROR: %s:%d:%s() "#it" is NULL.\n",		\
	       __FILE__, __LINE__, __FUNCTION__);			\
       return __VA_ARGS__;						\
    }								 	\
  else if (it->deleted)						\
    {									\
       fprintf(stderr, "ERROR: %s:%d:%s() "#it" has been DELETED.\n",	\
	       __FILE__, __LINE__, __FUNCTION__);			\
       return __VA_ARGS__;						\
    }



static inline void
_elm_list_item_call_del_cb(Elm_List_Item *it)
{
   if (it->del_cb) it->del_cb((void *)it->data, it->obj, it);
}

static inline void
_elm_list_item_free(Elm_List_Item *it)
{
   evas_object_event_callback_del_full
     (it->base, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, it);
   evas_object_event_callback_del_full
     (it->base, EVAS_CALLBACK_MOUSE_UP, _mouse_up, it);
   evas_object_event_callback_del_full
     (it->base, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move, it);

   if (it->icon)
     evas_object_event_callback_del_full
       (it->icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
	_changed_size_hints, it->obj);

   if (it->end)
     evas_object_event_callback_del_full
       (it->end, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
	_changed_size_hints, it->obj);

   eina_stringshare_del(it->label);

   if (it->long_timer) ecore_timer_del(it->long_timer);
   if (it->icon) evas_object_del(it->icon);
   if (it->end) evas_object_del(it->end);
   if (it->base) evas_object_del(it->base);
   free(it);
}

static void
_elm_list_process_deletions(Widget_Data *wd)
{
   Elm_List_Item *it;

   wd->walking++; // avoid nested deletion and also _sub_del() fix_items

   EINA_LIST_FREE(wd->to_delete, it)
     {
	_elm_list_item_call_del_cb(it);

	wd->items = eina_list_remove_list(wd->items, it->node);
	_elm_list_item_free(it);
     }

   wd->walking--;
}

static inline void
_elm_list_walk(Widget_Data *wd)
{
   if (wd->walking < 0)
     {
	fprintf(stderr, "ERROR: walking was negative. fixed!\n");
	wd->walking = 0;
     }
   wd->walking++;
}

static inline void
_elm_list_unwalk(Widget_Data *wd)
{
   wd->walking--;
   if (wd->walking < 0)
     {
	fprintf(stderr, "ERROR: walking became negative. fixed!\n");
	wd->walking = 0;
     }

   if (wd->walking)
     return;

   if (wd->to_delete)
     _elm_list_process_deletions(wd);

   if (wd->fix_pending)
     {
	wd->fix_pending = EINA_FALSE;
	_fix_items(wd->self);
	_sizing_eval(wd->self);
     }
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_List_Item *it;
   Eina_List *n;

   if (!wd) return;
   if (wd->walking != 0)
     fprintf(stderr, "ERROR: list deleted while walking.\n");

   _elm_list_walk(wd);
   EINA_LIST_FOREACH(wd->items, n, it) _elm_list_item_call_del_cb(it);
   _elm_list_unwalk(wd);
   if (wd->to_delete) fprintf(stderr, "ERROR: leaking nodes!\n");

   EINA_LIST_FREE(wd->items, it) _elm_list_item_free(it);
   eina_list_free(wd->selected);
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   if (!wd) return;
   if (wd->scr)
     {
        evas_object_size_hint_min_get(wd->scr, &minw, &minh);
        evas_object_size_hint_max_get(wd->scr, &maxw, &maxh);
        evas_object_size_hint_min_set(obj, minw, minh);
        evas_object_size_hint_max_set(obj, maxw, maxh);
     }
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_List_Item *it;
   Eina_List *n;
   
   if (!wd) return;
   if (wd->scr)
     {
        elm_scroller_custom_widget_base_theme_set(wd->scr, "list", "base");
        elm_object_style_set(wd->scr, elm_widget_style_get(obj));
//        edje_object_scale_set(wd->scr, elm_widget_scale_get(obj) * _elm_config->scale);
     }
   EINA_LIST_FOREACH(wd->items, n, it)
     {
        it->fixed = 0;
     }
   _fix_items(obj);
   _sizing_eval(obj);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     evas_object_focus_set(wd->scr, 1);
   else
     evas_object_focus_set(wd->scr, 0);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
//   _fix_items(data);
//   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   const Eina_List *l;
   Elm_List_Item *it;

   if (!wd) return;
   if (sub == NULL) abort();
   if (sub == wd->scr)
     wd->scr = NULL;
   else
     {
        EINA_LIST_FOREACH(wd->items, l, it)
          {
             if ((sub == it->icon) || (sub == it->end))
               {
                  if (it->icon == sub) it->icon = NULL;
                  if (it->end == sub) it->end = NULL;
                  evas_object_event_callback_del_full
                    (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, 
                     obj);
                  if (!wd->walking)
                    {
                       _fix_items(obj);
                       _sizing_eval(obj);
                    }
                  else
                    wd->fix_pending = EINA_TRUE;
                  break;
               }
          }
     }
}

static void
_item_hilight(Elm_List_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   const char *selectraise;

   if (!wd) return;
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it);
   if (it->hilighted) return;
   _elm_list_walk(wd);

   edje_object_signal_emit(it->base, "elm,state,selected", "elm");
   selectraise = edje_object_data_get(it->base, "selectraise");
   if ((selectraise) && (!strcmp(selectraise, "on")))
     evas_object_raise(it->base);
   it->hilighted = EINA_TRUE;

   _elm_list_unwalk(wd);
}

static void
_item_select(Elm_List_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);

   if (!wd) return;
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it);
   if (it->selected)
     {
	if (wd->always_select) goto call;
	return;
     }
   it->selected = EINA_TRUE;
   wd->selected = eina_list_append(wd->selected, it);
   call:
   _elm_list_walk(wd);

   if (it->func) it->func((void *)it->data, it->obj, it);
   evas_object_smart_callback_call(it->obj, "selected", it);

   _elm_list_unwalk(wd);
}

static void
_item_unselect(Elm_List_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   const char *stacking, *selectraise;

   if (!wd) return;
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it);
   if (!it->hilighted) return;
   _elm_list_walk(wd);

   edje_object_signal_emit(it->base, "elm,state,unselected", "elm");
   stacking = edje_object_data_get(it->base, "stacking");
   selectraise = edje_object_data_get(it->base, "selectraise");
   if ((selectraise) && (!strcmp(selectraise, "on")))
     {
	if ((stacking) && (!strcmp(stacking, "below")))
	  evas_object_lower(it->base);
     }
   it->hilighted = EINA_FALSE;
   if (it->selected)
     {
	it->selected = EINA_FALSE;
	wd->selected = eina_list_remove(wd->selected, it);
	evas_object_smart_callback_call(it->obj, "unselected", it);
     }

   _elm_list_unwalk(wd);
}

static void
_mouse_move(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_List_Item *it = data;
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Event_Mouse_Move *ev = event_info;

   if (!wd) return;
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it);
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
	if (!wd->on_hold)
	  {
	     wd->on_hold = EINA_TRUE;
             if (it->long_timer)
               {
                  ecore_timer_del(it->long_timer);
                  it->long_timer = NULL;
               }
	     _item_unselect(it);
	  }
     }
}

static int
_long_press(void *data)
{
   Elm_List_Item *it = data;
   Widget_Data *wd = elm_widget_data_get(it->obj);

   if (!wd) return 0;
   it->long_timer = NULL;
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it, 0);
   wd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call(it->obj, "longpressed", it);
   return 0;
}

static void
_mouse_down(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_List_Item *it = data;
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Event_Mouse_Down *ev = event_info;

   if (!wd) return;
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it);
   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) wd->on_hold = EINA_TRUE;
   else wd->on_hold = EINA_FALSE;
   wd->wasselected = it->selected;
   _item_hilight(it);
   wd->longpressed = EINA_FALSE;
   if (it->long_timer) ecore_timer_del(it->long_timer);
   it->long_timer = ecore_timer_add(1.0, _long_press, it);
   /* Always call the callbacks last - the user may delete our context! */
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     evas_object_smart_callback_call(it->obj, "clicked", it);
}

static void
_mouse_up(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_List_Item *it = data;
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Event_Mouse_Up *ev = event_info;

   if (!wd) return;
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it);
   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) wd->on_hold = EINA_TRUE;
   else wd->on_hold = EINA_FALSE;
   wd->longpressed = EINA_FALSE;
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
   if (wd->on_hold)
     {
	wd->on_hold = EINA_FALSE;
	return;
     }
   if (wd->longpressed)
     {
        if (!wd->wasselected) _item_unselect(it);
        wd->wasselected = 0;
        return;
     }

   _elm_list_walk(wd); // watch out "return" before unwalk!

   if (wd->multi)
     {
	if (!it->selected)
	  {
	     _item_hilight(it);
	     _item_select(it);
	  }
	else _item_unselect(it);
     }
   else
     {
	if (!it->selected)
	  {
	     while (wd->selected)
	       _item_unselect(wd->selected->data);
	     _item_hilight(it);
	     _item_select(it);
	  }
	else
	  {
	     const Eina_List *l, *l_next;
	     Elm_List_Item *it2;

	     EINA_LIST_FOREACH_SAFE(wd->selected, l, l_next, it2)
	       if (it2 != it) _item_unselect(it2);
	     _item_hilight(it);
	     _item_select(it);
	  }
     }

   _elm_list_unwalk(wd);
}

static Elm_List_Item *
_item_new(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_List_Item *it;

   if (!wd) return NULL;
   it = calloc(1, sizeof(Elm_List_Item));
   it->obj = obj;
   it->label = eina_stringshare_add(label);
   it->icon = icon;
   it->end = end;
   it->func = func;
   it->data = data;
   it->base = edje_object_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(it->base, EVAS_CALLBACK_MOUSE_DOWN,
				  _mouse_down, it);
   evas_object_event_callback_add(it->base, EVAS_CALLBACK_MOUSE_UP,
				  _mouse_up, it);
   evas_object_event_callback_add(it->base, EVAS_CALLBACK_MOUSE_MOVE,
				  _mouse_move, it);
   evas_object_size_hint_weight_set(it->base, 1.0, 1.0);
   evas_object_size_hint_align_set(it->base, -1.0, -1.0);
   if (it->icon)
     {
	elm_widget_sub_object_add(obj, it->icon);
	evas_object_event_callback_add(it->icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
     }
   if (it->end)
     {
	elm_widget_sub_object_add(obj, it->end);
	evas_object_event_callback_add(it->end, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
     }
   return it;
}

static void
_fix_items(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const Eina_List *l;
   Elm_List_Item *it;
   Evas_Coord minw[2] = { 0, 0 }, minh[2] = { 0, 0 };
   Evas_Coord mw, mh;
   int i, redo = 0;
   const char *style = elm_widget_style_get(obj);

   if (!wd) return;
   if (wd->walking)
     {
	wd->fix_pending = EINA_TRUE;
	return;
     }

   _elm_list_walk(wd); // watch out "return" before unwalk!

   EINA_LIST_FOREACH(wd->items, l, it)
     {
	if (it->deleted) continue;
	if (it->icon)
	  {
	     evas_object_size_hint_min_get(it->icon, &mw, &mh);
	     if (mw > minw[0]) minw[0] = mw;
	     if (mh > minh[0]) minh[0] = mh;
	  }
	if (it->end)
	  {
	     evas_object_size_hint_min_get(it->end, &mw, &mh);
	     if (mw > minw[1]) minw[1] = mw;
	     if (mh > minh[1]) minh[1] = mh;
	  }
     }
   if ((minw[0] != wd->minw[0]) || (minw[1] != wd->minw[1]) ||
       (minw[0] != wd->minh[0]) || (minh[1] != wd->minh[1]))
     {
	wd->minw[0] = minw[0];
	wd->minw[1] = minw[1];
	wd->minh[0] = minh[0];
	wd->minh[1] = minh[1];
	redo = 1;
     }
   i = 0;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
	if (it->deleted) continue;
	it->even = i & 0x1;
	if ((it->even != it->is_even) || (!it->fixed) || (redo))
	  {
	     const char *stacking;

	     if (wd->mode == ELM_LIST_COMPRESS)
	       {
		  if (it->even)
		    _elm_theme_object_set(obj, it->base, "list", "item_compress", style);
		  else
		    _elm_theme_object_set(obj, it->base, "list", "item_compress_odd", style);
	       }
	     else
	       {
		  if (it->even)
		    _elm_theme_object_set(obj, it->base, "list", "item", style);
		  else
		    _elm_theme_object_set(obj, it->base, "list", "item_odd", style);
	       }
	     stacking = edje_object_data_get(it->base, "stacking");
	     if (stacking)
	       {
		  if (!strcmp(stacking, "below"))
		    evas_object_lower(it->base);
		  else if (!strcmp(stacking, "above"))
		    evas_object_raise(it->base);
	       }
	     edje_object_part_text_set(it->base, "elm.text", it->label);
	     if ((!it->icon) && (minh[0] > 0))
	       {
		  it->icon = evas_object_rectangle_add(evas_object_evas_get(it->base));
		  evas_object_color_set(it->icon, 0, 0, 0, 0);
		  it->dummy_icon = EINA_TRUE;
	       }
	     if ((!it->end) && (minh[1] > 0))
	       {
		  it->end = evas_object_rectangle_add(evas_object_evas_get(it->base));
		  evas_object_color_set(it->end, 0, 0, 0, 0);
		  it->dummy_end = EINA_TRUE;
	       }
	     if (it->icon)
	       {
		  evas_object_size_hint_min_set(it->icon, minw[0], minh[0]);
		  evas_object_size_hint_max_set(it->icon, 99999, 99999);
		  edje_object_part_swallow(it->base, "elm.swallow.icon", it->icon);
	       }
	     if (it->end)
	       {
		  evas_object_size_hint_min_set(it->end, minw[1], minh[1]);
		  evas_object_size_hint_max_set(it->end, 99999, 99999);
		  edje_object_part_swallow(it->base, "elm.swallow.end", it->end);
	       }
	     if (!it->fixed)
	       {
		  // this may call up user and it may modify the list item
		  // but we're safe as we're flagged as walking.
		  // just don't process further
		  edje_object_message_signal_process(it->base);
		  if (it->deleted)
		    continue;
		  mw = mh = -1;
		  elm_coords_finger_size_adjust(1, &mw, 1, &mh);
		  edje_object_size_min_restricted_calc(it->base, &mw, &mh, mw, mh);
		  elm_coords_finger_size_adjust(1, &mw, 1, &mh);
		  evas_object_size_hint_min_set(it->base, mw, mh);
		  evas_object_show(it->base);
	       }
	     if ((it->selected) || (it->hilighted))
	       {
		  const char *selectraise;

		  // this may call up user and it may modify the list item
		  // but we're safe as we're flagged as walking.
		  // just don't process further
		  edje_object_signal_emit(it->base, "elm,state,selected", "elm");
		  if (it->deleted)
		    continue;

		  selectraise = edje_object_data_get(it->base, "selectraise");
		  if ((selectraise) && (!strcmp(selectraise, "on")))
		    evas_object_raise(it->base);
		  stacking = edje_object_data_get(it->base, "stacking");
	       }
	     it->fixed = EINA_TRUE;
	     it->is_even = it->even;
	  }
	i++;
     }

   _elm_list_unwalk(wd);

   mw = 0; mh = 0;
   evas_object_size_hint_min_get(wd->box, &mw, &mh);
   if (wd->scr)
     {
        if (wd->mode == ELM_LIST_LIMIT)
          elm_scroller_content_min_limit(wd->scr, 1, 0);
        else
          elm_scroller_content_min_limit(wd->scr, 0, 0);
     }
   _sizing_eval(obj);
}

static void
_hold_on(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_widget_scroll_hold_push(wd->scr);
}

static void
_hold_off(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_widget_scroll_hold_pop(wd->scr);
}

static void
_freeze_on(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_widget_scroll_hold_push(wd->scr);
}

static void
_freeze_off(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_widget_scroll_hold_pop(wd->scr);
}

/**
 * Adds a list object.
 *
 * @param parent The parent object
 * @return The created object or NULL upon failure
 *
 * @ingroup List
 */
EAPI Evas_Object *
elm_list_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   wd->self = obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "list");
   elm_widget_type_set(obj, "list");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, 1);

   wd->scr = elm_scroller_add(parent);
   elm_scroller_custom_widget_base_theme_set(wd->scr, "list", "base");
   elm_widget_resize_object_set(obj, wd->scr);

   elm_scroller_bounce_set(wd->scr, 0, 1);

   wd->box = elm_box_add(parent);
   elm_box_homogenous_set(wd->box, 1);
   evas_object_size_hint_weight_set(wd->box, 1.0, 0.0);
   evas_object_size_hint_align_set(wd->box, -1.0, 0.0);
   elm_scroller_content_set(wd->scr, wd->box);
   evas_object_show(wd->box);

   wd->mode = ELM_LIST_SCROLL;

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-on", _hold_on, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-off", _hold_off, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-on", _freeze_on, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-off", _freeze_off, obj);

   _sizing_eval(obj);
   return obj;
}

/**
 * Appends an item to the list object.
 *
 * @param obj The list object
 * @param label The label of the list item
 * @param icon The icon object to use for the left side of the item
 * @param end The icon object to use for the right side of the item
 * @param func The function to call when the item is clicked
 * @param data The data to associate with the item for related callbacks
 *
 * @return The created item or NULL upon failure
 *
 * @ingroup List
 */
EAPI Elm_List_Item *
elm_list_item_append(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_List_Item *it = _item_new(obj, label, icon, end, func, data);

   wd->items = eina_list_append(wd->items, it);
   it->node = eina_list_last(wd->items);
   elm_box_pack_end(wd->box, it->base);
   return it;
}

/**
 * Prepends an item to the list object.
 *
 * @param obj The list object
 * @param label The label of the list item
 * @param icon The icon object to use for the left side of the item
 * @param end The icon object to use for the right side of the item
 * @param func The function to call when the item is clicked
 * @param data The data to associate with the item for related callbacks
 *
 * @return The created item or NULL upon failure
 *
 * @ingroup List
 */
EAPI Elm_List_Item *
elm_list_item_prepend(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_List_Item *it = _item_new(obj, label, icon, end, func, data);

   wd->items = eina_list_prepend(wd->items, it);
   it->node = wd->items;
   elm_box_pack_start(wd->box, it->base);
   return it;
}

/**
 * Inserts an item into the list object before @p before.
 *
 * @param obj The list object
 * @param before The list item to insert before
 * @param label The label of the list item
 * @param icon The icon object to use for the left side of the item
 * @param end The icon object to use for the right side of the item
 * @param func The function to call when the item is clicked
 * @param data The data to associate with the item for related callbacks
 *
 * @return The created item or NULL upon failure
 *
 * @ingroup List
 */
EAPI Elm_List_Item *
elm_list_item_insert_before(Evas_Object *obj, Elm_List_Item *before, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data)
{
   Widget_Data *wd;
   Elm_List_Item *it;

   if ((!before) || (!before->node)) return NULL;
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(before, NULL);

   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   it = _item_new(obj, label, icon, end, func, data);
   wd->items = eina_list_prepend_relative_list(wd->items, it, before->node);
   it->node = before->node->prev;
   elm_box_pack_before(wd->box, it->base, before->base);
   return it;
}

/**
 * Inserts an item into the list object after @p after.
 *
 * @param obj The list object
 * @param after The list item to insert after
 * @param label The label of the list item
 * @param icon The icon object to use for the left side of the item
 * @param end The icon object to use for the right side of the item
 * @param func The function to call when the item is clicked
 * @param data The data to associate with the item for related callbacks
 *
 * @return The created item or NULL upon failure
 *
 * @ingroup List
 */
EAPI Elm_List_Item *
elm_list_item_insert_after(Evas_Object *obj, Elm_List_Item *after, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data)
{
   Widget_Data *wd;
   Elm_List_Item *it;

   if ((!after) || (!after->node)) return NULL;
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(after, NULL);

   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   it = _item_new(obj, label, icon, end, func, data);
   wd->items = eina_list_append_relative_list(wd->items, it, after->node);
   it->node = after->node->next;
   elm_box_pack_after(wd->box, it->base, after->base);
   return it;
}

/**
 * Insert a new item into the sorted list object.
 *
 * @param obj The list object
 * @param label The label of the list item
 * @param icon The icon object to use for the left side of the item
 * @param end The icon object to use for the right side of the item
 * @param func The function to call when the item is clicked
 * @param data The data to associate with the item for related callbacks
 * @param cmp_func The function called for the sort.
 *
 * @return The created item or NULL upon failure
 *
 * @ingroup List
 */
EAPI Elm_List_Item *
elm_list_item_sorted_insert(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data, Eina_Compare_Cb cmp_func)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_List_Item *it = _item_new(obj, label, icon, end, func, data);
   Eina_List *l;

   wd->items = eina_list_sorted_insert(wd->items, cmp_func, it);
   l = eina_list_data_find_list(wd->items, it);
   l = eina_list_next(l);
   if (!l)
     {
	it->node = eina_list_last(wd->items);
	elm_box_pack_end(wd->box, it->base);
     }
   else
     {
	Elm_List_Item *before = eina_list_data_get(l);
	it->node = before->node->prev;
	elm_box_pack_before(wd->box, it->base, before->base);
     }
   return it;
}

/**
 * Clears a list of all items.
 *
 * @param obj The list object
 *
 * @ingroup List
 */
EAPI void
elm_list_clear(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_List_Item *it;

   if (!wd) return;
   if (!wd->items) return;

   eina_list_free(wd->selected);
   wd->selected = NULL;

   if (wd->walking > 0)
     {
	Eina_List *n;
        
	EINA_LIST_FOREACH(wd->items, n, it)
	  {
	     if (it->deleted) continue;
	     it->deleted = EINA_TRUE;
	     wd->to_delete = eina_list_append(wd->to_delete, it);
	  }
	return;
     }

   _elm_list_walk(wd);

   EINA_LIST_FREE(wd->items, it)
     {
	_elm_list_item_call_del_cb(it);
	_elm_list_item_free(it);
     }

   _elm_list_unwalk(wd);

   _fix_items(obj);
   _sizing_eval(obj);
}

/**
 * Starts the list.  Call before running show() on the list object.
 *
 * @param obj The list object
 *
 * @ingroup List
 */
EAPI void
elm_list_go(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _fix_items(obj);
}

/**
 * Enables/disables the state of multi-select on the list object.
 *
 * @param obj The list object
 * @param multi If true, multi-select is enabled
 *
 * @ingroup List
 */
EAPI void
elm_list_multi_select_set(Evas_Object *obj, Eina_Bool multi)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->multi = multi;
}

/**
 * Gets the state of multi-select on the list object.
 *
 * @param obj The list object
 * @return If true, multi-select is enabled
 *
 * @ingroup List
 */
EAPI Eina_Bool
elm_list_multi_select_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->multi;
}

/**
 * Enables/disables horizontal mode of the list
 *
 * @param obj The list object
 * @param mode If true, horizontale mode is enabled
 *
 * @ingroup List
 */
EAPI void
elm_list_horizontal_mode_set(Evas_Object *obj, Elm_List_Mode mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->mode == mode) return;
   wd->mode = mode;
   if (wd->scr)
     {
        if (wd->mode == ELM_LIST_LIMIT)
          elm_scroller_content_min_limit(wd->scr, 1, 0);
        else
          elm_scroller_content_min_limit(wd->scr, 0, 0);
     }
}

/**
 * Gets the state of horizontal mode of the list
 *
 * @param obj The list object
 * @return If true, horizontale mode is enabled
 *
 * @ingroup List
 */
EAPI Elm_List_Mode
elm_list_horizontal_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_LIST_SCROLL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_LIST_SCROLL;
   return wd->mode;
}

/**
 * Enables/disables the state of always_select, meaning that
 * an item will always be selected.
 *
 * @param obj The list object
 * @param always_select If true, always_select is enabled
 *
 * @ingroup List
 */
EAPI void
elm_list_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->always_select = always_select;
}

/**
 * Gets the state of always_select.
 * See also elm_list_always_select_mode_set()
 *
 * @param obj The list object
 * @return If true, always_select is enabled
 *
 * @ingroup List
 */
EAPI Eina_Bool
elm_list_always_select_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->always_select;
}

/**
 * Returns a list of all the list items.
 *
 * @param obj The list object
 * @return An Eina_List* of the list items, or NULL on failure
 *
 * @ingroup List
 */
EAPI const Eina_List *
elm_list_items_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->items;
}

/**
 * Returns the currently selected list item.
 *
 * @param obj The list object
 * @return The selected list item, or NULL on failure
 *
 * @ingroup List
 */
EAPI Elm_List_Item *
elm_list_selected_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (wd->selected) return wd->selected->data;
   return NULL;
}

/**
 * Returns a list of the currently selected list items.
 *
 * @param obj The list object
 * @return An Eina_List* of the selected list items, or NULL on failure
 *
 * @ingroup List
 */
EAPI const Eina_List *
elm_list_selected_items_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->selected;
}

/**
 * Sets the selected state of @p it.
 *
 * @param it The list item
 * @param selected Enables/disables the selected state
 *
 * @ingroup List
 */
EAPI void
elm_list_item_selected_set(Elm_List_Item *it, Eina_Bool selected)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   if (!wd) return;
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it);
   selected = !!selected;
   if (it->selected == selected) return;

   _elm_list_walk(wd);

   if (selected)
     {
	if (!wd->multi)
	  {
	     while (wd->selected)
	       _item_unselect(wd->selected->data);
	  }
	_item_hilight(it);
	_item_select(it);
     }
   else
     _item_unselect(it);

   _elm_list_unwalk(wd);
}

/**
 * Gets the selected state of @p it.
 *
 * @param it The list item
 * @return If true, the item is selected
 *
 * @ingroup List
 */
EAPI Eina_Bool
elm_list_item_selected_get(Elm_List_Item *it)
{
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it, EINA_FALSE);
   return it->selected;
}

/**
 * Brings @p it to the center of the list view.
 *
 * @param it The list item
 *
 * @ingroup List
 */
EAPI void
elm_list_item_show(Elm_List_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Coord bx, by, bw, bh;
   Evas_Coord x, y, w, h;

   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it);
   evas_object_geometry_get(wd->box, &bx, &by, &bw, &bh);
   evas_object_geometry_get(it->base, &x, &y, &w, &h);
   x -= bx;
   y -= by;
   if (wd->scr)
     elm_scroller_region_show(wd->scr, x, y, w, h);
}

/**
 * Deletes item @p it from the list.
 *
 * @param it The list item to delete
 *
 * @ingroup List
 */
EAPI void
elm_list_item_del(Elm_List_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   if (!wd) return;
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it);

   if (it->selected) _item_unselect(it);

   if (wd->walking > 0)
     {
	if (it->deleted) return;
	it->deleted = EINA_TRUE;
	wd->to_delete = eina_list_append(wd->to_delete, it);
	return;
     }

   wd->items = eina_list_remove_list(wd->items, it->node);

   _elm_list_walk(wd);

   _elm_list_item_call_del_cb(it);
   _elm_list_item_free(it);

   _elm_list_unwalk(wd);
}

/**
 * Set the function called when a list item is freed.
 *
 * @param it The item to set the callback on
 * @param func The function called
 *
 * @ingroup List
 */
EAPI void
elm_list_item_del_cb_set(Elm_List_Item *it, Evas_Smart_Cb func)
{
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it);
   it->del_cb = func;
}

/**
 * Returns the data associated with the item.
 *
 * @param it The list item
 * @return The data associated with @p it
 *
 * @ingroup List
 */
EAPI void *
elm_list_item_data_get(const Elm_List_Item *it)
{
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it, NULL);
   return (void *)it->data;
}

/**
 * Returns the left side icon associated with the item.
 *
 * @param it The list item
 * @return The left side icon associated with @p it
 *
 * @ingroup List
 */
EAPI Evas_Object *
elm_list_item_icon_get(const Elm_List_Item *it)
{
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it, NULL);
   if (it->dummy_icon) return NULL;
   return it->icon;
}

/**
 * Sets the left side icon associated with the item.
 *
 * @param it The list item
 * @param icon The left side icon object to associate with @p it
 *
 * @ingroup List
 */
EAPI void
elm_list_item_icon_set(Elm_List_Item *it, Evas_Object *icon)
{
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it);
   if (it->icon == icon) return;
   if (it->dummy_icon && !icon) return;
   if (it->dummy_icon) evas_object_del(it->icon);
   if (!icon)
     {
	icon = evas_object_rectangle_add(evas_object_evas_get(it->obj));
	evas_object_color_set(icon, 0, 0, 0, 0);
	it->dummy_icon = EINA_TRUE;
     }
   it->icon = icon;
   if (it->base)
     edje_object_part_swallow(it->base, "elm.swallow.icon", icon);
}

/**
 * Gets the right side icon associated with the item.
 *
 * @param it The list item
 * @return The right side icon object associated with @p it
 *
 * @ingroup List
 */
EAPI Evas_Object *
elm_list_item_end_get(const Elm_List_Item *it)
{
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it, NULL);
   if (it->dummy_end) return NULL;
   return it->end;
}

/**
 * Gets the right side icon associated with the item.
 *
 * @param it The list item
 * @param icon The right side icon object to associate with @p it
 *
 * @ingroup List
 */
EAPI void
elm_list_item_end_set(Elm_List_Item *it, Evas_Object *end)
{
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it);
   if (it->end == end) return;
   if (it->dummy_end && !end) return;
   if (it->dummy_end) evas_object_del(it->end);
   if (!end)
     {
	end = evas_object_rectangle_add(evas_object_evas_get(it->obj));
	evas_object_color_set(end, 0, 0, 0, 0);
	it->dummy_end = EINA_TRUE;
     }
   it->end = end;
   if (it->base)
     edje_object_part_swallow(it->base, "elm.swallow.end", end);
}

/**
 * Gets the base object of the item.
 *
 * @param it The list item
 * @return The base object associated with @p it
 *
 * @ingroup List
 */
EAPI Evas_Object *
elm_list_item_base_get(const Elm_List_Item *it)
{
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it, NULL);
   return it->base;
}

/**
 * Gets the label of the item.
 *
 * @param it The list item
 * @return The label of @p it
 *
 * @ingroup List
 */
EAPI const char *
elm_list_item_label_get(const Elm_List_Item *it)
{
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it, NULL);
   return it->label;
}

/**
 * Sets the label of the item.
 *
 * @param it The list item
 * @param text The label of @p it
 *
 * @ingroup List
 */
EAPI void
elm_list_item_label_set(Elm_List_Item *it, const char *text)
{
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it);
   if (!eina_stringshare_replace(&it->label, text)) return;
   if (it->base)
     edje_object_part_text_set(it->base, "elm.text", it->label);
}

/**
 * Gets the item before @p it in the list.
 *
 * @param it The list item
 * @return The item before @p it, or NULL on failure
 *
 * @ingroup List
 */
EAPI Elm_List_Item *
elm_list_item_prev(const Elm_List_Item *it)
{
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it, NULL);
   if (it->node->prev) return it->node->prev->data;
   else return NULL;
}

/**
 * Gets the item after @p it in the list.
 *
 * @param it The list item
 * @return The item after @p it, or NULL on failure
 *
 * @ingroup List
 */
EAPI Elm_List_Item *
elm_list_item_next(const Elm_List_Item *it)
{
   ELM_LIST_ITEM_CHECK_DELETED_RETURN(it, NULL);
   if (it->node->next) return it->node->next->data;
   else return NULL;
}

/**
 * Set bounce mode
 *
 * This will enable or disable the scroller bounce mode for the list. See
 * elm_scroller_bounce_set() for details
 *
 * @param obj The list object
 * @param h_bounce Allow bounce horizontally
 * @param v_bounce Allow bounce vertically
 *
 * @ingroup List
 */
EAPI void
elm_list_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_scroller_bounce_set(wd->scr, h_bounce, v_bounce);
}

/**
 * Set the scrollbar policy
 *
 * This sets the scrollbar visibility policy for the given scroller.
 * ELM_SMART_SCROLLER_POLICY_AUTO means the scrollber is made visible if it
 * is needed, and otherwise kept hidden. ELM_SMART_SCROLLER_POLICY_ON turns
 * it on all the time, and ELM_SMART_SCROLLER_POLICY_OFF always keeps it off.
 * This applies respectively for the horizontal and vertical scrollbars.
 *
 * @param obj The list object
 * @param policy_h Horizontal scrollbar policy
 * @param policy_v Vertical scrollbar policy
 *
 * @ingroup List
 */
EAPI void
elm_list_scroller_policy_set(Evas_Object *obj, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_scroller_policy_set(wd->scr, policy_h, policy_v);
}

EAPI void
elm_list_scroller_policy_get(const Evas_Object *obj, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_scroller_policy_get(wd->scr, policy_h, policy_v);
}

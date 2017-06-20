#include "efl_ui_list_private.h"

#include <assert.h>

#define MY_CLASS EFL_UI_LIST_CLASS
#define MY_CLASS_NAME "Efl.Ui.List"

#define MY_PAN_CLASS EFL_UI_LIST_PAN_CLASS

#define SIG_CHILD_ADDED "child,added"
#define SIG_CHILD_REMOVED "child,removed"
#define SELECTED_PROP "selected"
#define MAX_ITEMS_PER_BLOCK 32

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHILD_ADDED, ""},
   {SIG_CHILD_REMOVED, ""},
   {NULL, NULL}
};

void _efl_ui_list_custom_layout(Efl_Ui_List *list);
void _efl_ui_list_item_select_set(Efl_Ui_List_Item *item, Eina_Bool select);
Eina_Bool _efl_ui_list_item_select_clear(Eo *obj);
Efl_Ui_List_Item *_child_new(Efl_Ui_List_Data *pd, Efl_Model *model);
void _child_remove(Efl_Ui_List_Data *pd, Efl_Ui_List_Item *item);
void _realize_items(Eo *obj, Efl_Ui_List_Data *pd);

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_select(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_escape(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"move", _key_action_move},
   {"select", _key_action_select},
   {"escape", _key_action_escape},
   {NULL, NULL}
};

static inline Eina_Bool
_horiz(Efl_Orient dir)
{
   return dir % 180 == EFL_ORIENT_RIGHT;
}

EOLIAN static void
_efl_ui_list_pan_elm_pan_pos_set(Eo *obj EINA_UNUSED, Efl_Ui_List_Pan_Data *psd, Evas_Coord x, Evas_Coord y)
{
   Evas_Coord ox, oy, ow, oh;
   Efl_Ui_List_Data *pd = psd->wpd;
   Efl_Ui_List_Item *litem;
   Eina_List *li;

   if ((x == pd->pan_x) && (y == pd->pan_y)) return;
   pd->pan_x = x;
   pd->pan_y = y;

   evas_object_geometry_get(psd->wobj, &ox, &oy, &ow, &oh);

   EINA_LIST_FOREACH(pd->realizes, li, litem)
     {
       if (litem->layout)
         evas_object_move(litem->layout, (litem->x + 0 - pd->pan_x), (litem->y + 0 - pd->pan_y));
     }
}

EOLIAN static void
_efl_ui_list_pan_elm_pan_pos_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Pan_Data *psd, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = psd->wpd->pan_x;
   if (y) *y = psd->wpd->pan_y;
}

EOLIAN static void
_efl_ui_list_pan_elm_pan_pos_max_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Pan_Data *psd, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord ow, oh;

   elm_interface_scrollable_content_viewport_geometry_get
              (psd->wobj, NULL, NULL, &ow, &oh);
   ow = psd->wpd->minw - ow;
   if (ow < 0) ow = 0;
   oh = psd->wpd->minh - oh;
   if (oh < 0) oh = 0;

   if (x) *x = ow;
   if (y) *y = oh;
}

EOLIAN static void
_efl_ui_list_pan_elm_pan_pos_min_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Pan_Data *psd EINA_UNUSED, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = 0;
   if (y) *y = 0;
}

EOLIAN static void
_efl_ui_list_pan_elm_pan_content_size_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Pan_Data *psd, Evas_Coord *w, Evas_Coord *h)
{
   Efl_Ui_List_Data *pd = psd->wpd;
   EINA_SAFETY_ON_NULL_RETURN(pd);

   if (w) *w = pd->minw;
   if (h) *h = pd->minh;
}

EOLIAN static void
_efl_ui_list_pan_efl_object_destructor(Eo *obj, Efl_Ui_List_Pan_Data *psd)
{
   efl_data_unref(psd->wobj, psd->wpd);
   efl_destructor(efl_super(obj, MY_PAN_CLASS));
}

#include "efl_ui_list_pan.eo.c"

static Eina_Bool
_efl_model_properties_has(Efl_Model *model, Eina_Stringshare *propfind)
{
   const Eina_Array *properties;
   Eina_Array_Iterator iter_prop;
   Eina_Stringshare *property;
   Eina_Bool ret = EINA_FALSE;
   unsigned i = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(model, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(propfind, EINA_FALSE);

   properties = efl_model_properties_get(model);

   EINA_ARRAY_ITER_NEXT(properties, i, property, iter_prop)
     {
        if (property == propfind)
          {
             ret = EINA_TRUE;
             break;
          }
     }

   return ret;
}

static void
_child_added_cb(void *data, const Efl_Event *event)
{
   Efl_Model_Children_Event* evt = event->info;
   Efl_Ui_List *obj = data;
   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);

   _child_new(pd, evt->child);
   evas_object_smart_changed(pd->obj);
}

static void
_child_removed_cb(void *data, const Efl_Event *event)
{
   Efl_Model_Children_Event* evt = event->info;
   Efl_Ui_List *obj = data;
   Efl_Ui_List_Item *item = NULL;
   Eina_List *li;

   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);

   EINA_LIST_FOREACH(pd->items, li, item)
     {
        if (item->model == evt->child)
          {
             _child_remove(pd, item);
             pd->items = eina_list_remove_list(pd->items, li);
             break;
          }
     }
   evas_object_smart_changed(pd->obj);
}

static void
_on_item_focused(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_List_Item *item = data;
   EFL_UI_LIST_DATA_GET_OR_RETURN(item->obj, pd);

   pd->focused = item;

   if (!_elm_config->item_select_on_focus_disable && !item->selected)
     _efl_ui_list_item_select_set(item, EINA_TRUE);
}

static void
_on_item_unfocused(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_List_Item *item = data;
   EFL_UI_LIST_DATA_GET_OR_RETURN(item->obj, pd);

   pd->focused = NULL;
}

static Eina_Bool
_long_press_cb(void *data)
{
   Efl_Ui_List_Item *item = data;

   item->long_timer = NULL;
   item->longpressed = EINA_TRUE;
   if (item->layout)
     efl_event_callback_legacy_call(item->layout, EFL_UI_EVENT_LONGPRESSED, item);

   return ECORE_CALLBACK_CANCEL;
}

static void
_on_item_mouse_down(void *data, Evas *evas EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Efl_Ui_List_Item *item = data;

   EFL_UI_LIST_DATA_GET_OR_RETURN(item->obj, pd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) pd->on_hold = EINA_TRUE;
   else pd->on_hold = EINA_FALSE;
   if (pd->on_hold) return;

   item->down = EINA_TRUE;
   item->longpressed = EINA_FALSE;

   ecore_timer_del(item->long_timer);
   item->long_timer = ecore_timer_add(_elm_config->longpress_timeout, _long_press_cb, item);
}

static void
_on_item_mouse_up(void *data, Evas *evas EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Efl_Ui_List_Item *item = data;

   EFL_UI_LIST_DATA_GET_OR_RETURN(item->obj, pd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) pd->on_hold = EINA_TRUE;
   else pd->on_hold = EINA_FALSE;
   if (pd->on_hold || !item->down) return;

   item->down = EINA_FALSE;
   ELM_SAFE_FREE(item->long_timer, ecore_timer_del);

   if (item->longpressed)
     {
        item->longpressed = EINA_FALSE;
        return;
     }

   if (pd->select_mode != ELM_OBJECT_SELECT_MODE_ALWAYS && item->selected)
     return;

   _efl_ui_list_item_select_set(item, EINA_TRUE);
}

static void
_item_selected_then(void * data, Efl_Event const* event)
{
   Efl_Ui_List_Item *item = data;
   EINA_SAFETY_ON_NULL_RETURN(item);
   Eina_Stringshare *selected;
   const Eina_Value_Type *vtype;
   Eina_Value *value = (Eina_Value *)((Efl_Future_Event_Success*)event->info)->value;

   item->future = NULL;
   vtype = eina_value_type_get(value);

   if (vtype == EINA_VALUE_TYPE_STRING || vtype == EINA_VALUE_TYPE_STRINGSHARE)
     {
        eina_value_get(value, &selected);
        Eina_Bool s = (strcmp(selected, "selected") ? EINA_FALSE : EINA_TRUE);

        if (item->selected == s) return;
        item->selected = s;

        EFL_UI_LIST_DATA_GET_OR_RETURN(item->obj, pd);
        if (item->selected)
          pd->selected = eina_list_append(pd->selected, item);
        else
          pd->selected = eina_list_remove(pd->selected, item);
     }
}

static void
_item_property_then(void * data, Efl_Event const* event)
{
   Efl_Ui_List_Item *item = data;
   EINA_SAFETY_ON_NULL_RETURN(item);
   Eina_Value *value = (Eina_Value *)((Efl_Future_Event_Success*)event->info)->value;
   const Eina_Value_Type *vtype = eina_value_type_get(value);
   char *style = NULL;

   item->future = NULL;
   if (vtype == EINA_VALUE_TYPE_STRING || vtype == EINA_VALUE_TYPE_STRINGSHARE)
     eina_value_get(value, &style);

   if (item->layout)
     elm_object_style_set(item->layout, style);
}

static void
_item_property_error(void * data, Efl_Event const* event EINA_UNUSED)
{
   Efl_Ui_List_Item *item = data;
   EINA_SAFETY_ON_NULL_RETURN(item);

   item->future = NULL;
}

static void
_efl_model_properties_changed_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_List_Item *item = data;
   EINA_SAFETY_ON_NULL_RETURN(item);
   Efl_Model_Property_Event *evt = event->info;
   Eina_Array_Iterator it;
   Eina_Stringshare *prop, *sprop;
   unsigned int i;

   if (!evt->changed_properties) return;

   sprop = eina_stringshare_add(SELECTED_PROP);
   EINA_ARRAY_ITER_NEXT(evt->changed_properties, i, prop, it)
     {
        if (prop == sprop)
          {
             item->future = efl_model_property_get(item->model, sprop);
             efl_future_then(item->future, &_item_selected_then, &_item_property_error, NULL, item);
          }
     }
}

void
_child_remove(Efl_Ui_List_Data *pd, Efl_Ui_List_Item *item)
{
   EINA_SAFETY_ON_NULL_RETURN(item);
   EINA_SAFETY_ON_NULL_RETURN(item->model);

   if (item->future)
     {
        efl_future_cancel(item->future);
        item->future = NULL;
     }

   if (pd->focused == item)
     {
        pd->focused = NULL;
     }

   if (item->selected)
     {
        item->selected = EINA_FALSE;
        pd->selected = eina_list_remove(pd->selected, item);
     }

   efl_event_callback_del(item->model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_changed_cb, item);

   efl_unref(item->model);
   free(item);
}

static Eina_Bool
_layout_realize(Efl_Ui_List_Data *pd, Efl_Ui_List_Item *item)
{
   Efl_Ui_List_Item_Event evt;

   evt.child = item->model;
   evt.layout = NULL;
   evt.index = item->index;
   efl_event_callback_call(item->obj, EFL_UI_LIST_EVENT_ITEM_REALIZED, &evt);

   pd->realizes = eina_list_append(pd->realizes, item);

   if (evt.layout == NULL)
     return EINA_FALSE;

   item->layout = efl_ref(evt.layout);
   elm_widget_sub_object_add(pd->obj, item->layout);
   evas_object_smart_member_add(item->layout, pd->pan_obj);

   evas_object_show(item->layout);

   efl_event_callback_add(item->layout, ELM_WIDGET_EVENT_FOCUSED, _on_item_focused, item);
   efl_event_callback_add(item->layout, ELM_WIDGET_EVENT_UNFOCUSED, _on_item_unfocused, item);
   evas_object_event_callback_add(item->layout, EVAS_CALLBACK_MOUSE_DOWN, _on_item_mouse_down, item);
   evas_object_event_callback_add(item->layout, EVAS_CALLBACK_MOUSE_UP, _on_item_mouse_up, item);

   if (pd->select_mode != ELM_OBJECT_SELECT_MODE_NONE)
     efl_ui_model_connect(item->layout, "signal/elm,state,%v", "selected");

   return EINA_TRUE;
}

static void
_layout_unrealize(Efl_Ui_List_Data *pd, Efl_Ui_List_Item *item)
{
   Efl_Ui_List_Item_Event evt;
   EINA_SAFETY_ON_NULL_RETURN(item);

   if (item->layout)
     {
        efl_event_callback_del(item->layout, ELM_WIDGET_EVENT_FOCUSED, _on_item_focused, item);
        efl_event_callback_del(item->layout, ELM_WIDGET_EVENT_UNFOCUSED, _on_item_unfocused, item);
        evas_object_event_callback_del_full(item->layout, EVAS_CALLBACK_MOUSE_DOWN, _on_item_mouse_down, item);
        evas_object_event_callback_del_full(item->layout, EVAS_CALLBACK_MOUSE_UP, _on_item_mouse_up, item);
        efl_ui_model_connect(item->layout, "signal/elm,state,%v", NULL);
        elm_widget_sub_object_del(pd->obj, item->layout);
        efl_unref(item->layout);
     }

   evt.child = item->model;
   evt.layout = item->layout;
   evt.index = item->index;
   efl_event_callback_call(item->obj, EFL_UI_LIST_EVENT_ITEM_UNREALIZED, &evt);

   pd->realizes = eina_list_remove(pd->realizes, item);
}

Efl_Ui_List_Item *
_child_new(Efl_Ui_List_Data *pd, Efl_Model *model)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, NULL);

   Eina_Stringshare *style_prop = eina_stringshare_add("style");
   Efl_Ui_List_Item *item = calloc(1, sizeof(Efl_Ui_List_Item));

   item->obj = pd->obj;
   item->model = efl_ref(model);
   item->layout = NULL;
   item->future = NULL;
   item->index = eina_list_count(pd->items);

   pd->items = eina_list_append(pd->items, item);

   efl_event_callback_add(item->model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_changed_cb, item);

   if (_efl_model_properties_has(item->model, style_prop))
     {
        item->future = efl_model_property_get(item->model, style_prop);
        efl_future_then(item->future, &_item_property_then, &_item_property_error, NULL, item);
     }
   eina_stringshare_del(style_prop);

   return item;
}

static void
_children_then(void * data, Efl_Event const* event)
{
   Efl_Ui_List_Data *pd = data;
   Eina_Accessor *acc = (Eina_Accessor*)((Efl_Future_Event_Success*)event->info)->value;
   Eo *child_model;
   unsigned i = 0;

   EINA_SAFETY_ON_NULL_RETURN(pd);
   EINA_SAFETY_ON_NULL_RETURN(acc);

   ELM_WIDGET_DATA_GET_OR_RETURN(pd->obj, wd);

   EINA_ACCESSOR_FOREACH(acc, i, child_model)
     {
        _child_new(pd, child_model);
     }

   evas_object_smart_changed(pd->obj);
}

static void
_efl_ui_list_children_free(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   Efl_Ui_List_Item *item;
   Eina_List *li;

   EINA_SAFETY_ON_NULL_RETURN(pd);

   if(!pd->items) return;


   EINA_LIST_FOREACH(pd->realizes, li, item)
     _layout_unrealize(pd, item);

   EINA_LIST_FREE(pd->items, item)
     _child_remove(pd, item);

   pd->items = NULL;
}

static void
_children_error(void * data EINA_UNUSED, Efl_Event const* event EINA_UNUSED)
{}

static void
_show_region_hook(void *data EINA_UNUSED, Evas_Object *obj)
{
   Evas_Coord x, y, w, h;

   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   elm_interface_scrollable_content_region_set(obj, x, y, w, h);
   elm_interface_scrollable_content_region_show(obj, x, y, w, h);
}

static void
_update_viewmodel(Eo *obj, Efl_Ui_List_Data *pd)
{
   _efl_ui_list_children_free(obj, pd);
   efl_future_then(efl_model_children_slice_get(pd->model, 0, 0),
                   &_children_then, &_children_error, NULL, pd);
}

EOLIAN static void
_efl_ui_list_select_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, Elm_Object_Select_Mode mode)
{
   Efl_Ui_List_Item *item;
   Eina_List *li;

   if (pd->select_mode == mode)
     return;

   if (pd->select_mode == ELM_OBJECT_SELECT_MODE_NONE)
     {
        EINA_LIST_FOREACH(pd->realizes, li, item)
          {
             if (!item->layout) continue;

             if (item->selected)
               elm_layout_signal_emit(item->layout, "elm,state,selected", "elm");

             efl_ui_model_connect(item->layout, "signal/elm,state,%v", "selected");
          }
     }
   else if (mode == ELM_OBJECT_SELECT_MODE_NONE)
     {
        EINA_LIST_FOREACH(pd->realizes, li, item)
          {
             if (!item->layout) continue;

             if (item->selected)
               elm_layout_signal_emit(item->layout, "elm,state,unselected", "elm");

             efl_ui_model_connect(item->layout, "signal/elm,state,%v", NULL);
          }
     }

   pd->select_mode = mode;
}

EOLIAN static Elm_Object_Select_Mode
_efl_ui_list_select_mode_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   return pd->select_mode;
}

EOLIAN static void
_efl_ui_list_default_style_set(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, Eina_Stringshare *style)
{
   eina_stringshare_replace(&pd->style, style);
}

EOLIAN static Eina_Stringshare *
_efl_ui_list_default_style_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   return pd->style;
}

//FIXME update layout
EOLIAN static void
_efl_ui_list_homogeneous_set(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, Eina_Bool homogeneous)
{
   pd->homogeneous = homogeneous;
}

EOLIAN static Eina_Bool
_efl_ui_list_homogeneous_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   return pd->homogeneous;
}

EOLIAN static Elm_Theme_Apply
_efl_ui_list_elm_widget_theme_apply(Eo *obj, Efl_Ui_List_Data *pd EINA_UNUSED)
{
   return elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Bool
_efl_ui_list_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

static void *
_efl_ui_list_eina_list_layout_get(const Eina_List *list)
{
   Efl_Ui_List_Item *litem = eina_list_data_get(list);

   return litem->layout;
}


EOLIAN static Evas_Object*
_efl_ui_list_elm_widget_focused_object_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
  if (!pd->focused) return NULL;
  return pd->focused->layout;
}

EOLIAN static Eina_Bool
_efl_ui_list_elm_widget_focus_next(Eo *obj, Efl_Ui_List_Data *pd, Elm_Focus_Direction dir, Evas_Object **next, Elm_Object_Item **next_item)
{
   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   if ((items = elm_widget_focus_custom_chain_get(obj)))
     list_data_get = eina_list_data_get;
   else
     {
        items = pd->items;
        list_data_get = _efl_ui_list_eina_list_layout_get;

        if (!items) return EINA_FALSE;
     }

   return elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next, next_item);
}

EOLIAN static Eina_Bool
_efl_ui_list_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_list_elm_widget_focus_direction(Eo *obj, Efl_Ui_List_Data *pd, const Evas_Object *base, double degree, Evas_Object **direction, Elm_Object_Item **direction_item, double *weight)
{
   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   if ((items = elm_widget_focus_custom_chain_get(obj)))
     list_data_get = eina_list_data_get;
   else
     {
        items = pd->items;
        list_data_get = _efl_ui_list_eina_list_layout_get;

        if (!items) return EINA_FALSE;
     }

   return elm_widget_focus_list_direction_get
         (obj, base, items, list_data_get, degree, direction, direction_item, weight);
}

EOLIAN static void
_efl_ui_list_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_List_Data *pd)
{
   if (pd->recalc) return;

   _efl_ui_list_custom_layout(obj);
}

EOLIAN static void
_efl_ui_list_efl_gfx_position_set(Eo *obj, Efl_Ui_List_Data *pd, Evas_Coord x, Evas_Coord y)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, x, y))
     return;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), x, y);

   evas_object_move(pd->hit_rect, x, y);
   evas_object_move(pd->pan_obj, x - pd->pan_x, y - pd->pan_y);
   _efl_ui_list_custom_layout(obj);
}

EOLIAN static void
_efl_ui_list_efl_gfx_size_set(Eo *obj, Efl_Ui_List_Data *pd, Evas_Coord w, Evas_Coord h)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, w, h))
     return;

   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);

   evas_object_resize(pd->hit_rect, w, h);
   printf("resize %d %d \n", w, h);
   _efl_ui_list_custom_layout(obj);
}

EOLIAN static void
_efl_ui_list_efl_canvas_group_group_member_add(Eo *obj, Efl_Ui_List_Data *pd, Evas_Object *member)
{
   efl_canvas_group_member_add(efl_super(obj, MY_CLASS), member);

   if (pd->hit_rect)
     evas_object_raise(pd->hit_rect);
}

EOLIAN static void
_efl_ui_list_elm_layout_sizing_eval(Eo *obj, Efl_Ui_List_Data *pd EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord vmw = 0, vmh = 0;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_gfx_size_hint_combined_min_get(obj, &minw, &minh);
   evas_object_size_hint_max_get(obj, &maxw, &maxh);
   edje_object_size_min_calc(wd->resize_obj, &vmw, &vmh);

   minw = vmw;
   minh = vmh;

   if ((maxw > 0) && (minw > maxw))
     minw = maxw;
   if ((maxh > 0) && (minh > maxh))
     minh = maxh;

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

EOLIAN static void
_efl_ui_list_efl_canvas_group_group_add(Eo *obj, Efl_Ui_List_Data *pd EINA_UNUSED)
{
   Efl_Ui_List_Pan_Data *pan_data;
   Evas_Coord minw, minh;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   pd->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_data_set(pd->hit_rect, "_elm_leaveme", obj);
   evas_object_smart_member_add(pd->hit_rect, obj);
   elm_widget_sub_object_add(obj, pd->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(pd->hit_rect, 0, 0, 0, 0);
   evas_object_show(pd->hit_rect);
   evas_object_repeat_events_set(pd->hit_rect, EINA_TRUE);

   elm_widget_on_show_region_hook_set(obj, _show_region_hook, NULL);

   if (!elm_layout_theme_set(obj, "list", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_interface_scrollable_objects_set(obj, wd->resize_obj, pd->hit_rect);
   elm_interface_scrollable_bounce_allow_set
           (obj, EINA_FALSE, _elm_config->thumbscroll_bounce_enable);

   pd->mode = ELM_LIST_COMPRESS;

   elm_interface_atspi_accessible_type_set(obj, ELM_ATSPI_TYPE_DISABLED);
   pd->pan_obj = efl_add(MY_PAN_CLASS, evas_object_evas_get(obj));
   pan_data = efl_data_scope_get(pd->pan_obj, MY_PAN_CLASS);
   efl_data_ref(obj, MY_CLASS);
   pan_data->wobj = obj;
   pan_data->wpd = pd;

   elm_interface_scrollable_extern_pan_set(obj, pd->pan_obj);
   evas_object_show(pd->pan_obj);

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   elm_interface_scrollable_mirrored_set(obj, elm_widget_mirrored_get(obj));
   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_list_efl_canvas_group_group_del(Eo *obj, Efl_Ui_List_Data *pd)
{
   pd->delete_me = EINA_TRUE;

   _efl_ui_list_children_free(obj, pd);

   ELM_SAFE_FREE(pd->pan_obj, evas_object_del);
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_efl_ui_list_efl_object_constructor(Eo *obj, Efl_Ui_List_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->obj = obj;
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_LIST);

   pd->style = eina_stringshare_add(elm_widget_style_get(obj));

   pd->orient = EFL_ORIENT_DOWN;
   pd->align.h = 0;
   pd->align.v = 0;

   return obj;
}

EOLIAN static void
_efl_ui_list_efl_object_destructor(Eo *obj, Efl_Ui_List_Data *pd)
{
   efl_unref(pd->model);
   eina_stringshare_del(pd->style);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_list_efl_ui_view_model_set(Eo *obj, Efl_Ui_List_Data *pd, Efl_Model *model)
{
   if (pd->model == model)
     return;

   if (pd->model)
     {
         efl_event_callback_del(pd->model, EFL_MODEL_EVENT_CHILD_ADDED, _child_added_cb, obj);
         efl_event_callback_del(pd->model, EFL_MODEL_EVENT_CHILD_REMOVED, _child_removed_cb, obj);
         efl_unref(pd->model);
         pd->model = NULL;
     }

   if (model)
     {
         pd->model = model;
         efl_ref(pd->model);
         efl_event_callback_add(pd->model, EFL_MODEL_EVENT_CHILD_ADDED, _child_added_cb, obj);
         efl_event_callback_add(pd->model, EFL_MODEL_EVENT_CHILD_REMOVED, _child_removed_cb, obj);
     }

   _update_viewmodel(obj, pd);
}

EOLIAN static Efl_Model *
_efl_ui_list_efl_ui_view_model_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   return pd->model;
}
EOLIAN const Elm_Atspi_Action *
_efl_ui_list_elm_interface_atspi_widget_action_elm_actions_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd EINA_UNUSED)
{
   static Elm_Atspi_Action atspi_actions[] = {
          { "move,prior", "move", "prior", _key_action_move},
          { "move,next", "move", "next", _key_action_move},
          { "move,up", "move", "up", _key_action_move},
          { "move,up,multi", "move", "up_multi", _key_action_move},
          { "move,down", "move", "down", _key_action_move},
          { "move,down,multi", "move", "down_multi", _key_action_move},
          { "move,first", "move", "first", _key_action_move},
          { "move,last", "move", "last", _key_action_move},
          { "select", "select", NULL, _key_action_select},
          { "select,multi", "select", "multi", _key_action_select},
          { "escape", "escape", NULL, _key_action_escape},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

EOLIAN Eina_List*
_efl_ui_list_elm_interface_atspi_accessible_children_get(Eo *obj, Efl_Ui_List_Data *pd)
{
   Eina_List *ret = NULL, *ret2 = NULL;
   Efl_Ui_List_Item *litem;
   Eina_List *li;

   EINA_LIST_FOREACH(pd->realizes, li, litem)
      ret = eina_list_append(ret, litem->layout);

   ret2 = elm_interface_atspi_accessible_children_get(efl_super(obj, MY_CLASS));

   return eina_list_merge(ret, ret2);
}

EOLIAN int
_efl_ui_list_elm_interface_atspi_selection_selected_children_count_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   return eina_list_count(pd->selected);
}

EOLIAN Eo*
_efl_ui_list_elm_interface_atspi_selection_selected_child_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, int child_idx)
{
   Efl_Ui_List_Item *item = eina_list_nth(pd->selected, child_idx);
   if (!item)
     return NULL;

   return item->layout;
}

EOLIAN Eina_Bool
_efl_ui_list_elm_interface_atspi_selection_child_select(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, int child_index)
{
   if (pd->select_mode != ELM_OBJECT_SELECT_MODE_NONE)
     {
        Efl_Ui_List_Item *item = eina_list_nth(pd->items, child_index);
        if (item)
           _efl_ui_list_item_select_set(item, EINA_TRUE);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_ui_list_elm_interface_atspi_selection_selected_child_deselect(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, int child_index)
{
   Efl_Ui_List_Item *item = eina_list_nth(pd->selected, child_index);
   if (item)
     {
        _efl_ui_list_item_select_set(item, EINA_FALSE);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_ui_list_elm_interface_atspi_selection_is_child_selected(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, int child_index)
{
   Efl_Ui_List_Item *item = eina_list_nth(pd->items, child_index);
   EINA_SAFETY_ON_NULL_RETURN_VAL(item, EINA_FALSE);
   return item->selected;
}

EOLIAN Eina_Bool
_efl_ui_list_elm_interface_atspi_selection_all_children_select(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   Efl_Ui_List_Item *item;
   Eina_List *l;

   if (pd->select_mode == ELM_OBJECT_SELECT_MODE_NONE)
     return EINA_FALSE;

   EINA_LIST_FOREACH(pd->items, l, item)
      _efl_ui_list_item_select_set(item, EINA_TRUE);

   return EINA_TRUE;
}

EOLIAN Eina_Bool
_efl_ui_list_elm_interface_atspi_selection_clear(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   Efl_Ui_List_Item *item;
   Eina_List *l;

   if (pd->select_mode == ELM_OBJECT_SELECT_MODE_NONE)
     return EINA_FALSE;

   EINA_LIST_FOREACH(pd->selected, l, item)
      _efl_ui_list_item_select_set(item, EINA_FALSE);

   return EINA_TRUE;
}

EOLIAN Eina_Bool
_efl_ui_list_elm_interface_atspi_selection_child_deselect(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, int child_index)
{
   Efl_Ui_List_Item *item = eina_list_nth(pd->items, child_index);
   if (item)
     {
      _efl_ui_list_item_select_set(item, EINA_FALSE);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params)
{
   EFL_UI_LIST_DATA_GET_OR_RETURN_VAL(obj, pd, EINA_FALSE);
   const char *dir = params;

   Evas_Coord page_x, page_y;
   Evas_Coord v_w, v_h;
   Evas_Coord x, y;

   elm_interface_scrollable_content_pos_get(obj, &x, &y);
   elm_interface_scrollable_page_size_get(obj, &page_x, &page_y);
   elm_interface_scrollable_content_viewport_geometry_get(obj, NULL, NULL, &v_w, &v_h);

/*
   Efl_Ui_List_Item *item = NULL;
   Elm_Object_Item *oitem = NULL;
   Elm_Layout *eoit = NULL;
   if (!strcmp(dir, "up") || !strcmp(dir, "up_multi"))
     {
        if (!elm_widget_focus_next_get(obj, ELM_FOCUS_UP, &eoit, &oitem))
          return EINA_FALSE;
     }
   else if (!strcmp(dir, "down") || !strcmp(dir, "down_multi"))
     {
        if (!elm_widget_focus_next_get(obj, ELM_FOCUS_DOWN, &eoit, &oitem))
          return EINA_FALSE;
        printf(">> %d\n", __LINE__);
     }
   else if (!strcmp(dir, "first"))
     {
        item = eina_list_data_get(pd->items);
        x = 0;
        y = 0;
        elm_widget_focus_next_object_set(obj, item->layout, ELM_FOCUS_UP);
     }
   else if (!strcmp(dir, "last"))
     {
        item = eina_list_data_get(eina_list_last(pd->items));
        elm_obj_pan_pos_max_get(pd->pan_obj, &x, &y);
     }
   else */
   if (!strcmp(dir, "prior"))
     {
        if (_horiz(pd->orient))
          {
             if (page_x < 0)
               x -= -(page_x * v_w) / 100;
             else
               x -= page_x;
          }
        else
          {
             if (page_y < 0)
               y -= -(page_y * v_h) / 100;
             else
               y -= page_y;
          }
     }
   else if (!strcmp(dir, "next"))
     {
        if (_horiz(pd->orient))
          {
             if (page_x < 0)
               x += -(page_x * v_w) / 100;
             else
               x += page_x;
          }
        else
          {
             if (page_y < 0)
               y += -(page_y * v_h) / 100;
             else
               y += page_y;
          }
     }
   else return EINA_FALSE;

   elm_interface_scrollable_content_pos_set(obj, x, y, EINA_TRUE);
/*
   if (item)
     eoit = item->layout;

   if (!eoit) return EINA_FALSE;

   elm_object_focus_set(eoit, EINA_TRUE);
*/
   return EINA_TRUE;
}

static Eina_Bool _key_action_select(Evas_Object *obj, const char *params EINA_UNUSED)
{
   EFL_UI_LIST_DATA_GET_OR_RETURN_VAL(obj, pd, EINA_FALSE);

   _efl_ui_list_item_select_set(pd->focused, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool _key_action_escape(Evas_Object *obj, const char *params EINA_UNUSED)
{
   return _efl_ui_list_item_select_clear(obj);
}

EOLIAN static Eina_Bool
_efl_ui_list_elm_widget_widget_event(Eo *obj, Efl_Ui_List_Data *pd, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   (void) src;
   Evas_Event_Key_Down *ev = event_info;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (!pd->items) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, MY_CLASS_NAME, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

Eina_Bool
_efl_ui_list_item_select_clear(Eo *obj)
{
   Eina_List *li;
   Efl_Ui_List_Item *item;
   EFL_UI_LIST_DATA_GET_OR_RETURN_VAL(obj, pd, EINA_FALSE);

   EINA_LIST_FOREACH(pd->selected, li, item)
      _efl_ui_list_item_select_set(item, EINA_FALSE);

   return EINA_TRUE;
}

void
_efl_ui_list_item_select_set(Efl_Ui_List_Item *item, Eina_Bool selected)
{
   Eina_Stringshare *sprop, *svalue;

   if (!item) return;

   EFL_UI_LIST_DATA_GET_OR_RETURN(item->obj, pd);

   if ((pd->select_mode == ELM_OBJECT_SELECT_MODE_NONE) ||
       (pd->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY))
     return;

   selected = !!selected;
   if (!item->model || item->selected == selected) return;

   sprop = eina_stringshare_add(SELECTED_PROP);
   svalue = (selected ? eina_stringshare_add("selected") : eina_stringshare_add("unselected"));

   if (_efl_model_properties_has(item->model, sprop))
     {
        Eina_Value v;
        eina_value_setup(&v, EINA_VALUE_TYPE_STRINGSHARE);
        eina_value_set(&v, svalue);
        efl_model_property_set(item->model, sprop, &v);
        eina_value_flush(&v);
     }
   eina_stringshare_del(sprop);
   eina_stringshare_del(svalue);

   //TODO I need call this event or catch only by model connect event?
   if (selected)
     efl_event_callback_legacy_call(item->layout, EFL_UI_EVENT_SELECTED, item);
   else
     efl_event_callback_legacy_call(item->layout, EFL_UI_EVENT_UNSELECTED, item);
}

typedef struct _Item_Calc Item_Calc;

struct _Item_Calc
{
   Evas_Object *obj;
   double weight[2];
   double align[2];
   int max[2];
   int pad[4];
   int want[2];
   int id;
   Efl_Ui_List_Item *litem;
};

Evas_Coord
_item_weight_calc(Efl_Ui_List_Item *item, Eina_Bool horizon)
{
   Evas_Coord old_w, old_h, minw = 0, minh = 0;

   efl_gfx_size_hint_combined_min_get(item->layout, &old_w, &old_h);
   edje_object_size_min_calc(elm_layout_edje_get(item->layout), &minw, &minh);

   if (old_w > minw) minw = old_w;
   if (old_h > minh) minh = old_h;
   evas_object_size_hint_min_set(item->layout, minw, minh);

   if (horizon)
     return minw;

   return minh;
}

void
_realize_items(Eo *obj, Efl_Ui_List_Data *pd)
{
   Efl_Ui_List_Item *litem;
   Eina_List *li, *subitems;
   int rlzw, objw, i;
   Eina_Bool horiz = _horiz(pd->orient);

   if (horiz)
     {
        rlzw = pd->rlzw;
        evas_object_geometry_get(obj, NULL, NULL, &objw, NULL);
     }
   else
     {
        rlzw = pd->rlzh;
        evas_object_geometry_get(obj, NULL, NULL, NULL, &objw);
     }

   i = eina_list_count(pd->realizes); //TODO add unrealized upper items
   subitems = eina_list_nth_list(pd->items, i);

   EINA_LIST_FOREACH(subitems, li, litem)
     {
        if (rlzw > objw)
          break;

        if (!_layout_realize(pd, litem))
          continue;

        rlzw += _item_weight_calc(litem, horiz);
     }

   return;
}

void
_efl_ui_list_custom_layout(Efl_Ui_List *ui_list)
{
   EFL_UI_LIST_DATA_GET_OR_RETURN(ui_list, pd);
   Efl_Ui_List_Item *litem;
   Evas_Object *o;
   Eina_List *li;
   int wantw = 0, wanth = 0; // requested size
   int boxx, boxy, boxw, boxh;
   Item_Calc *items, *item;
   Eina_Bool horiz = _horiz(pd->orient), zeroweight = EINA_FALSE;
   int id = 0, count, boxl = 0, boxr = 0, boxt = 0, boxb = 0;
   int length, want, pad, extra = 0, rounding = 0;
   double cur_pos = 0, weight[2] = { 0, 0 }, scale;
   double box_align[2];
   Eina_Bool box_fill[2] = { EINA_FALSE, EINA_FALSE };
   ELM_WIDGET_DATA_GET_OR_RETURN(ui_list, wd);

   _realize_items(ui_list, pd);

   evas_object_geometry_get(ui_list, &boxx, &boxy, &boxw, &boxh);
   efl_gfx_size_hint_margin_get(ui_list, &boxl, &boxr, &boxt, &boxb);

   scale = evas_object_scale_get(ui_list);
   // Box align: used if "item has max size and fill" or "no item has a weight"
   // Note: cells always expand on the orthogonal direction
   box_align[0] = pd->align.h;
   box_align[1] = pd->align.v;
   if (box_align[0] < 0)
     {
        box_fill[0] = EINA_TRUE;
        box_align[0] = 0.5;
     }
   if (box_align[1] < 0)
     {
        box_fill[1] = EINA_TRUE;
        box_align[1] = 0.5;
     }

   count = eina_list_count(pd->realizes);
   if (!count)
     {
        evas_object_size_hint_min_set(wd->resize_obj, 0, 0);
        return;
     }

   items = alloca(count * sizeof(*items));
#ifdef DEBUG
   memset(items, 0, count * sizeof(*items));
#endif

   Evas_Coord ow, oh;
   elm_interface_scrollable_content_viewport_geometry_get
              (ui_list, NULL, NULL, &ow, &oh);
   // scan all items, get their properties, calculate total weight & min size
   EINA_LIST_FOREACH(pd->realizes, li, litem)
     {
        item = &items[id];
        o = item->obj = litem->layout;
        item->litem = litem;

        efl_gfx_size_hint_weight_get(o, &item->weight[0], &item->weight[1]);
        efl_gfx_size_hint_align_get(o, &item->align[0], &item->align[1]);
        efl_gfx_size_hint_margin_get(o, &item->pad[0], &item->pad[1], &item->pad[2], &item->pad[3]);
        efl_gfx_size_hint_max_get(o, &item->max[0], &item->max[1]);
        efl_gfx_size_hint_combined_min_get(o, &item->want[0], &item->want[1]);

        if (item->weight[0] < 0) item->weight[0] = 0;
        if (item->weight[1] < 0) item->weight[1] = 0;

        if (item->align[0] < 0) item->align[0] = -1;
        if (item->align[1] < 0) item->align[1] = -1;
        if (item->align[0] > 1) item->align[0] = 1;
        if (item->align[1] > 1) item->align[1] = 1;

        if (item->want[0] < 0) item->want[0] = 0;
        if (item->want[1] < 0) item->want[1] = 0;

        if (item->max[0] <= 0) item->max[0] = INT_MAX;
        if (item->max[1] <= 0) item->max[1] = INT_MAX;
        if (item->max[0] < item->want[0]) item->max[0] = item->want[0];
        if (item->max[1] < item->want[1]) item->max[1] = item->want[1];

        item->want[0] += item->pad[0] + item->pad[1];
        item->want[1] += item->pad[2] + item->pad[3];

        weight[0] += item->weight[0];
        weight[1] += item->weight[1];
        if (horiz)
          {
             wantw += item->want[0];
             if (item->want[1] > wanth)
               wanth = item->want[1];
          }
        else
          {
             wanth += item->want[1];
             if (item->want[0] > wantw)
               wantw = item->want[0];
          }

        item->id = id++;
     }

   // box outer margin
   boxw -= boxl + boxr;
   boxh -= boxt + boxb;
   boxx += boxl;
   boxy += boxt;

   // total space & available space
   if (horiz)
     {
        length = boxw;
        want = wantw;
        pad = pd->pad.scalable ? (pd->pad.h * scale) : pd->pad.h;
     }
   else
     {
        length = boxh;
        want = wanth;
        pad = pd->pad.scalable ? (pd->pad.v * scale) : pd->pad.v;
     }

   // padding can not be squeezed (note: could make it an option)
   length -= pad * (count - 1);

   // available space. if <0 we overflow
   extra = length - want;

   int ilen;
   ilen = eina_list_count(pd->items);
   if (horiz)
     {
        pd->minw = wantw + boxl + boxr + pad * (count - 1);
        pd->minh = wanth + boxt + boxb;
        pd->rlzw = pd->minw;
        pd->rlzh = pd->minh;
        if (ilen > count)
          pd->minw *= ilen / count;
     }
   else
     {
        pd->minw = wantw + boxl + boxr;
        pd->minh = wanth + pad * (count - 1) + boxt + boxb;
        pd->rlzw = pd->minw;
        pd->rlzh = pd->minh;
        if (ilen > count)
          pd->minh *= ilen / count;
     }

   evas_object_size_hint_min_set(wd->resize_obj, pd->minw, pd->minh);

   if (extra < 0) extra = 0;

   if (EINA_DBL_EQ(weight[!horiz], 0))
     {
        if (box_fill[!horiz])
          {
             // box is filled, set all weights to be equal
             zeroweight = EINA_TRUE;
          }
        else
          {
             // move bounding box according to box align
             cur_pos = extra * box_align[!horiz];
          }
        weight[!horiz] = count;
     }

   for (id = 0; id < count; id++)
     {
        double cx, cy, cw, ch, x, y, w, h;
        item = &items[id];

        // extra rounding up (compensate cumulative error)
        if ((item->id == (count - 1)) && (cur_pos - floor(cur_pos) >= 0.5))
          rounding = 1;

        if (horiz)
          {
             cx = boxx + cur_pos;
             cy = boxy;
             cw = item->want[0] + rounding + (zeroweight ? 1.0 : item->weight[0]) * extra / weight[0];
             ch = boxh;
             cur_pos += cw + pad;
          }
        else
          {
             cx = boxx;
             cy = boxy + cur_pos;
             cw = boxw;
             ch = item->want[1] + rounding + (zeroweight ? 1.0 : item->weight[1]) * extra / weight[1];
             cur_pos += ch + pad;
          }

        // horizontally
        if (item->max[0] < INT_MAX)
          {
             w = MIN(MAX(item->want[0] - item->pad[0] - item->pad[1], item->max[0]), cw);
             if (item->align[0] < 0)
               {
                  // bad case: fill+max are not good together
                  x = cx + ((cw - w) * box_align[0]) + item->pad[0];
               }
             else
               x = cx + ((cw - w) * item->align[0]) + item->pad[0];
          }
        else if (item->align[0] < 0)
          {
             // fill x
             w = cw - item->pad[0] - item->pad[1];
             x = cx + item->pad[0];
          }
        else
          {
             w = item->want[0] - item->pad[0] - item->pad[1];
             x = cx + ((cw - w) * item->align[0]) + item->pad[0];
          }

        // vertically
        if (item->max[1] < INT_MAX)
          {
             h = MIN(MAX(item->want[1] - item->pad[2] - item->pad[3], item->max[1]), ch);
             if (item->align[1] < 0)
               {
                  // bad case: fill+max are not good together
                  y = cy + ((ch - h) * box_align[1]) + item->pad[2];
               }
             else
               y = cy + ((ch - h) * item->align[1]) + item->pad[2];
          }
        else if (item->align[1] < 0)
          {
             // fill y
             h = ch - item->pad[2] - item->pad[3];
             y = cy + item->pad[2];
          }
        else
          {
             h = item->want[1] - item->pad[2] - item->pad[3];
             y = cy + ((ch - h) * item->align[1]) + item->pad[2];
          }

        if (horiz)
          {
             if (h < pd->minh) h = pd->minh;
             if (h > oh) h = oh;
          }
        else
          {
             if (w < pd->minw) w = pd->minw;
             if (w > ow) w = ow;
          }

        item->litem->x = x;
        item->litem->y = y;
        evas_object_geometry_set(item->obj, (x + 0 - pd->pan_x), (y + 0 - pd->pan_y), w, h);
     }
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_LIST_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_list)

#include "efl_ui_list.eo.c"

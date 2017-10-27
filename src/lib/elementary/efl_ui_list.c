#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED
#define EFL_ACCESS_SELECTION_PROTECTED

#include <Elementary.h>
#include "efl_ui_list_private.h"
#include "efl_ui_list_precise_layouter.eo.h"

#include <assert.h>

#define MY_CLASS EFL_UI_LIST_CLASS
#define MY_CLASS_NAME "Efl.Ui.List"

#define MY_PAN_CLASS EFL_UI_LIST_PAN_CLASS

#define SIG_CHILD_ADDED "child,added"
#define SIG_CHILD_REMOVED "child,removed"
#define SELECTED_PROP "selected"
#define AVERAGE_SIZE_INIT 10

#undef DBG
#define DBG(...) do { \
    fprintf(stderr, __FILE__ ":" "%d %s ", __LINE__, __PRETTY_FUNCTION__); \
    fprintf(stderr,  __VA_ARGS__);                                     \
    fprintf(stderr, "\n"); fflush(stderr);                              \
  } while(0)

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHILD_ADDED, ""},
   {SIG_CHILD_REMOVED, ""},
   {NULL, NULL}
};

void _efl_ui_list_custom_layout(Efl_Ui_List *);
void _efl_ui_list_item_select_set(Efl_Ui_List_Item *, Eina_Bool);
Eina_Bool _efl_ui_list_item_select_clear(Eo *);
//static void _item_calc(Efl_Ui_List_Data *, Efl_Ui_List_Item *);
static void _layout_unrealize(Efl_Ui_List_Data *, Efl_Ui_List_Item *);
static Eina_Bool _update_items(Eo *, Efl_Ui_List_Data *);
static void _insert_at(Efl_Ui_List_Data* pd, int index, Efl_Model* child);
static void _remove_at(Efl_Ui_List_Data* pd, int index);
static void _layout(Efl_Ui_List_Data* pd);

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
_efl_ui_list_pan_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_List_Pan_Data *psd)
{
   DBG("");
   /* if (pd->recalc) return; */

   /* _efl_ui_list_custom_layout(obj); */
   evas_object_smart_changed(psd->wobj);
}


EOLIAN static void
_efl_ui_list_pan_elm_pan_pos_set(Eo *obj EINA_UNUSED, Efl_Ui_List_Pan_Data *psd, Evas_Coord x, Evas_Coord y)
{
   Evas_Coord ox, oy, ow, oh, cw;

   DBG("x: %d y: %d", (int)x, (int)y);
   if ((x == psd->x) && (y == psd->y)) return;

//   evas_object_geometry_get(pd->obj, &ox, &oy, &ow, &oh);
//   if (_horiz(pd->orient))
//     {
//        pd->pan.move_diff += x - pd->pan.x;
//        cw = ow / 4;
//     }
//   else
//     {
//        pd->pan.move_diff += y - pd->pan.y;
//        cw = oh / 4;
//     }

   psd->x = x;
   psd->y = y;

//   if (abs(pd->pan.move_diff) > cw)
//     {
//        pd->pan.move_diff = 0;
//        _update_items(obj, pd);
//     }
//   else
//     {
//        EINA_INARRAY_FOREACH(&pd->items.array, litem)
//          evas_object_move((*litem)->layout, ((*litem)->x + 0 - pd->pan.x), ((*litem)->y + 0 - pd->pan.y));
//     }
//
   evas_object_smart_changed(psd->wobj);
}

EOLIAN static void
_efl_ui_list_pan_elm_pan_pos_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Pan_Data *psd, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = psd->x;
   if (y) *y = psd->y;
}

EOLIAN static void
_efl_ui_list_pan_elm_pan_pos_max_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Pan_Data *psd, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord ow, oh;
   Eina_Size2D min;

   elm_interface_scrollable_content_viewport_geometry_get
              (psd->wobj, NULL, NULL, &ow, &oh);

   min = efl_ui_list_model_min_size_get(psd->wobj);
   ow = min.w - ow;
   if (ow < 0) ow = 0;
   oh = min.h - oh;
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
   Eina_Size2D min;
   min = efl_ui_list_model_min_size_get(psd->wobj);

   if (w) *w = min.w;
   if (h) *h = min.h;

   DBG("w: %d h: %d", *w, *h);
}

EOLIAN static void
_efl_ui_list_pan_efl_object_destructor(Eo *obj, Efl_Ui_List_Pan_Data *psd)
{
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

/* static void */
/* _child_added_cb(void *data, const Efl_Event *event) */
/* { */
/*    Efl_Model_Children_Event* evt = event->info; */
/*    Efl_Ui_List *obj = data; */
/*    EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd); */
/*    int index = evt->index - pd->realized.start; */

/*    pd->item_count++; */
/*    if (index >= 0 && index <= pd->realized.slice) */
/*      _insert_at(pd, index, evt->child); */
/*    else */
/*      evas_object_smart_changed(pd->obj); */
/* } */

/* static void */
/* _child_removed_cb(void *data, const Efl_Event *event) */
/* { */
/*    Efl_Model_Children_Event* evt = event->info; */
/*    Efl_Ui_List *obj = data; */
/*    EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd); */
/*    int index = evt->index - pd->realized.start; */

/*    pd->item_count--; */
/*    if (index >= 0 && index < pd->realized.slice) */
/*      _remove_at(pd, index); */
/*    else */
/*      evas_object_smart_changed(pd->obj); */
/* } */

static void
_on_item_focused(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_List_Item *item = data;

   EFL_UI_LIST_DATA_GET_OR_RETURN(item->list, pd);

   if (!_elm_config->item_select_on_focus_disable)
     _efl_ui_list_item_select_set(item, EINA_TRUE);
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

   EFL_UI_LIST_DATA_GET_OR_RETURN(item->list, pd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) pd->on_hold = EINA_TRUE;
   else pd->on_hold = EINA_FALSE;
   if (pd->on_hold) return;

   item->down = EINA_TRUE;
   assert(item->longpressed == EINA_FALSE);

   ecore_timer_del(item->long_timer);
   item->long_timer = ecore_timer_add(_elm_config->longpress_timeout, _long_press_cb, item);
}

static void
_on_item_mouse_up(void *data, Evas *evas EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Efl_Ui_List_Item *item = data;

   EFL_UI_LIST_DATA_GET_OR_RETURN(item->list, pd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) pd->on_hold = EINA_TRUE;
   else pd->on_hold = EINA_FALSE;
   if (pd->on_hold || !item->down) return;

   item->down = EINA_FALSE;
   ELM_SAFE_FREE(item->long_timer, ecore_timer_del);

   if (!item->longpressed)
     {
       if (pd->select_mode != ELM_OBJECT_SELECT_MODE_ALWAYS && item->selected)
         return;

       _efl_ui_list_item_select_set(item, EINA_TRUE);
     }
   else
     item->longpressed = EINA_FALSE;
}

static void
_item_selected_then(void * data, Efl_Event const* event)
{
   Efl_Ui_List_Item *item = data;
   EINA_SAFETY_ON_NULL_RETURN(item);
   Eina_Stringshare *selected;
   const Eina_Value_Type *vtype;
   Eina_Value *value = (Eina_Value *)((Efl_Future_Event_Success*)event->info)->value;

   vtype = eina_value_type_get(value);

   if (vtype == EINA_VALUE_TYPE_STRING || vtype == EINA_VALUE_TYPE_STRINGSHARE)
     {
        eina_value_get(value, &selected);
        Eina_Bool s = (strcmp(selected, "selected") ? EINA_FALSE : EINA_TRUE);
        EFL_UI_LIST_DATA_GET_OR_RETURN(item->list, pd);

        if (item->selected == s) return;

        item->selected = s;
        /* item->future = NULL; */

        if (item->selected)
          pd->selected_items = eina_list_append(pd->selected_items, item);
        else
          pd->selected_items = eina_list_remove(pd->selected_items, item);
     }
}

static void
_count_then(void * data, Efl_Event const* event)
{
   Efl_Ui_List_Data *pd = data;
   EINA_SAFETY_ON_NULL_RETURN(pd);
   int *count = ((Efl_Future_Event_Success*)event->info)->value;

   pd->item_count = *count;
   pd->count_future = NULL;

   _layout(pd);
}

static void
_count_error(void * data, Efl_Event const* event EINA_UNUSED)
{
   Efl_Ui_List_Data *pd = data;
   EINA_SAFETY_ON_NULL_RETURN(pd);
   pd->count_future = NULL;
}

static void
_item_style_property_then(void * data, Efl_Event const* event)
{
   Efl_Ui_List_Item *item = data;
   EINA_SAFETY_ON_NULL_RETURN(item);
   char *style = NULL;

   Eina_Value *value = (Eina_Value *)((Efl_Future_Event_Success*)event->info)->value;
   const Eina_Value_Type *vtype = eina_value_type_get(value);

   item->future = NULL;
   if (vtype == EINA_VALUE_TYPE_STRING || vtype == EINA_VALUE_TYPE_STRINGSHARE)
     eina_value_get(value, &style);

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

/* static void */
/* _item_min_calc(Efl_Ui_List_Data *pd, Efl_Ui_List_Item *item, Evas_Coord h, Evas_Coord w) */
/* { */
/*    Efl_Ui_List_Item *litem, **it; */

/*    if(_horiz(pd->orient)) */
/*      { */
/*         pdp->realized.w -= item->minw; */
/*         pd->realized.w += w; */
/*         if(pd->realized.h <= h) */
/*           pd->realized.h = h; */
/*         else if (pd->realized.h < item->minh) */
/*           { */
/*              pd->realized.h = h; */
/*              EINA_INARRAY_FOREACH(&pd->items.array, it) */
/*                { */
/*                   litem = *it; */
/*                   if (!litem) continue; */
/*                   if (pd->realized.h < litem->minh) */
/*                     pd->realized.h = litem->minh; */

/*                   if (litem != item && litem->minh == item->minh) */
/*                     break; */
/*                } */
/*           } */
/*      } */
/*    else */
/*      { */
/*         pd->realized.h -= item->minh; */
/*         pd->realized.h += h; */
/*         if(pd->realized.w <= w) */
/*           pd->realized.w = w; */
/*         else if (pd->realized.w == item->minw) */
/*           { */
/*              pd->realized.w = w; */
/*              EINA_INARRAY_FOREACH(&pd->items.array, it) */
/*                { */
/*                   litem = *it; */
/*                   if (!litem) continue; */
/*                   if (pd->realized.w < litem->minw) */
/*                     pd->realized.w = litem->minw; */

/*                   if (litem != item && litem->minw == item->minw) */
/*                     break; */
/*                } */
/*           } */
/*      } */

/*    item->minw = w; */
/*    item->minh = h; */
/* } */

static void
_on_item_size_hint_change(void *data, Evas *e EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efl_Ui_List_Item *item = data;
   EINA_SAFETY_ON_NULL_RETURN(item);
//   if (obj != item->layout) { printf("NOOOOOOOOOOOOOOOOOOOOOO\n"); return; }

   EFL_UI_LIST_DATA_GET_OR_RETURN(item->list, pd);
   //_item_calc(pd, item);
   evas_object_smart_changed(item->list);
}

static void
_layout_unrealize(Efl_Ui_List_Data *pd, Efl_Ui_List_Item *item)
{
   Efl_Ui_List_Item_Event evt;
   EINA_SAFETY_ON_NULL_RETURN(item);

   if (item->future)
     {
        efl_future_cancel(item->future);
        item->future = NULL;
     }

   /* TODO:calculate new min */
   //_item_min_calc(pd, item, 0, 0);

   evt.child = item->model;
   evt.layout = item->layout;
   evt.index = item->index;
   efl_event_callback_call(item->list, EFL_UI_LIST_EVENT_ITEM_UNREALIZED, &evt);
   efl_ui_view_model_set(item->layout, NULL);

   evas_object_hide(item->layout);
   evas_object_move(item->layout, -9999, -9999);
}

/* static Efl_Ui_List_Item* */
/* _child_setup(Efl_Ui_List_Data *pd, Efl_Model *model */
/*              , Eina_Inarray* recycle_layouts, int idx) */
/* { */
/*    EINA_SAFETY_ON_NULL_RETURN_VAL(pd, NULL); */

/*    Efl_Ui_List_Item* item = calloc(1, sizeof(Efl_Ui_List_Item)); */

/*    assert(model != NULL); */
/*    assert(item->layout == NULL); */
/*    assert(item->list == NULL); */

/*    item->list = pd->obj; */
/*    item->future = NULL; */
/*    item->model = efl_ref(model); */
/*    if (recycle_layouts && eina_inarray_count(recycle_layouts)) */
/*      item->layout = *(void**)eina_inarray_pop(recycle_layouts); */
/*    else */
/*      { */
/*         Eina_Stringshare *sselected = eina_stringshare_add("selected"); */
/*         item->layout = efl_ui_factory_create(pd->factory, item->model, pd->obj); */

/* //        if (pd->select_mode != ELM_OBJECT_SELECT_MODE_NONE && _efl_model_properties_has(item->model, sselected)) */
/*           efl_ui_model_connect(item->layout, "signal/elm,state,%v", "selected"); */

/*         eina_stringshare_del(sselected); */
/*      } */
/*    item->index = idx + pd->realized.start; */
/*    item->minw = item->minh = 0; */

/*    elm_widget_sub_object_add(pd->obj, item->layout); */
/*    evas_object_smart_member_add(item->layout, pd->pan.obj); */

/* //   FIXME: really need get it in model? */
/*    Eina_Stringshare *style_prop = eina_stringshare_add("style"); */
/*    if (_efl_model_properties_has(item->model, style_prop)) */
/*      { */
/*         item->future = efl_model_property_get(item->model, style_prop); */
/*         efl_future_then(item->future, &_item_style_property_then, &_item_property_error, NULL, item); */
/*      } */
/*    eina_stringshare_del(style_prop); */
/* // */
/*    _layout_realize(pd, item); */

/*    efl_event_callback_add(item->model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_changed_cb, item); */
/*    efl_event_callback_add(item->layout, EFL_UI_FOCUS_MANAGER_EVENT_FOCUSED, _on_item_focused, item); */

/*    evas_object_event_callback_add(item->layout, EVAS_CALLBACK_MOUSE_DOWN, _on_item_mouse_down, item); */
/*    evas_object_event_callback_add(item->layout, EVAS_CALLBACK_MOUSE_UP, _on_item_mouse_up, item); */
/*    evas_object_event_callback_add(item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change, item); */

/*    if (item->selected) */
/*      pd->selected_items = eina_list_append(pd->selected_items, item); */

/*    return item; */
/* } */

/* static void */
/* _child_release(Efl_Ui_List_Data* pd, Efl_Ui_List_Item** oitem, Eina_Inarray* recycle_layouts) */
/* { */
/*    Efl_Ui_List_Item* item = *oitem; */

/*    evas_object_event_callback_del_full(item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change, item); */
/*    efl_event_callback_del(item->model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_changed_cb, item); */
/*    evas_object_event_callback_del_full(item->layout, EVAS_CALLBACK_MOUSE_DOWN, _on_item_mouse_down, item); */
/*    evas_object_event_callback_del_full(item->layout, EVAS_CALLBACK_MOUSE_UP, _on_item_mouse_up, item); */

/*    efl_event_callback_del(item->layout, EFL_UI_FOCUS_MANAGER_EVENT_FOCUSED, _on_item_focused, item); */

/*    if (item->selected) */
/*      pd->selected_items = eina_list_remove(pd->selected_items, item); */

/*    item->list = NULL; */

/*    _layout_unrealize(pd, item); */

/*    efl_unref(item->model); */
/*    // discard elm_layout to thrash to be able to reuse it */
/*    assert(item->layout != NULL); */

/*    if (recycle_layouts) */
/*      eina_inarray_push(recycle_layouts, &item->layout); */
/*    else */
/*      efl_ui_factory_release(pd->factory, item->layout); */

/*    item->layout = NULL; */

/*    if (_horiz(pd->orient)) */
/*      pd->realized.w -= item->w; */
/*    else */
/*      pd->realized.h -= item->h; */

/*    free(item); */
/*    *oitem = NULL; */
/* } */

/* static void */
/* _remove_at(Efl_Ui_List_Data* pd, int index) */
/* { */
/*    Efl_Ui_List_Item **to_first, **from_first; */
/*    int i, j; */

/*    // fits, just move around */
/*    to_first = pd->items.array.members; */
/*    to_first += index; */
/*    from_first = to_first + 1; */

/*    _child_release(pd, to_first, NULL); */

/*    memmove(to_first, from_first, sizeof(Efl_Ui_List_Item*)*(pd->items.array.len - index - 1)); */

/*    for(i = index, j = 0; i < (int)pd->items.array.len-1; ++i, ++j) */
/*      { */
/*         to_first[j]->index--; */
/*      } */
/*    eina_inarray_pop(&pd->items.array); */

/*    pd->realized.slice--; */
/*    evas_object_smart_changed(pd->obj); */
/* } */

/* static void */
/* _insert_at(Efl_Ui_List_Data* pd, int index, Efl_Model* child) */
/* { */
/*    Efl_Ui_List_Item **members, *item; */
/*    int i; */

/*    item = _child_setup(pd,  child, NULL, index - pd->realized.start); */
/*    eina_inarray_insert_at(&pd->items.array, index, &item); */

/*    // fits, just move around */
/*    members = pd->items.array.members; */

/*    for(i = index+1; i < (int)pd->items.array.len; ++i) */
/*      { */
/*         members[i]->index++; */
/*      } */

/*    pd->realized.slice++; */
/*    evas_object_smart_changed(pd->obj); */
/* } */

/* static void */
/* _resize_children(Efl_Ui_List_Data* pd, int removing_before, int removing_after, */
/*                  Eina_Accessor* acc) */
/* { */
/*    Eina_Inarray recycle_layouts; */
/*    unsigned to_begin, from_begin, copy_size; */
/*    unsigned idx; */

/*    eina_inarray_setup(&recycle_layouts, sizeof(Efl_Ui_Layout*), 0); */

/*    EINA_SAFETY_ON_NULL_RETURN(pd); */
/*    EINA_SAFETY_ON_NULL_RETURN(acc); */
/*    ELM_WIDGET_DATA_GET_OR_RETURN(pd->obj, wd); */

/*    assert(pd->realized.slice == eina_inarray_count(&pd->items.array)); */
/*    // received slice start is after older slice start */
/*    if (removing_before > 0) */
/*      { */
/*         int i = 0; */
/*         while (i != removing_before) */
/*           { */
/*              Efl_Ui_List_Item** members = pd->items.array.members; */
/*              _child_release(pd, &members[i], &recycle_layouts); */
/*              ++i; */
/*           } */
/*         to_begin = 0; */
/*         from_begin = removing_before; */
/*      } */
/*    else */
/*      { */
/*         to_begin = -removing_before; */
/*         from_begin = 0; */
/*      } */

/*    if (removing_after > 0) */
/*      { */
/*         int i = pd->realized.slice - removing_after; */
/*         while (i != pd->realized.slice) */
/*           { */
/*             Efl_Ui_List_Item** members = pd->items.array.members; */
/*             _child_release(pd, &members[i], &recycle_layouts); */
/*             ++i; */
/*           } */
/*         copy_size = eina_inarray_count(&pd->items.array) */
/*           - (from_begin + removing_after); */
/*      } */
/*    else */
/*      { */
/*         copy_size = eina_inarray_count(&pd->items.array) - from_begin; */
/*      } */

/*    if (removing_after + removing_before >= 0) */
/*      { */
/*         if (from_begin != to_begin) */
/*           { */
/*              Efl_Ui_List_Item** from = pd->items.array.members; */
/*              Efl_Ui_List_Item** to = pd->items.array.members; */
/*              from += from_begin; */
/*              to += to_begin; */
/*              memmove(to, from, copy_size*sizeof(Efl_Ui_List_Item*)); */
/*           } */
/*         memset(&((Efl_Ui_List_Item**)pd->items.array.members)[copy_size+to_begin], 0, (eina_inarray_count(&pd->items.array) - (copy_size+to_begin))*sizeof(Efl_Ui_List_Item*)); */
/*         memset(&((Efl_Ui_List_Item**)pd->items.array.members)[0], 0, to_begin*sizeof(Efl_Ui_List_Item*)); */
/*         pd->items.array.len -= removing_before + removing_after; */
/*      } */
/*    else */
/*      { */
/*         Efl_Ui_List_Item** data = calloc(1, pd->outstanding_slice.slice*sizeof(Efl_Ui_List_Item*)); */
/*         Efl_Ui_List_Item** from_first = pd->items.array.members; */
/*         from_first += from_begin; */
/*         Efl_Ui_List_Item** to_first = data + to_begin; */
/*         memcpy(to_first, from_first, copy_size*sizeof(Efl_Ui_List_Item*)); */
/*         free(pd->items.array.members); */
/*         pd->items.array.members = data; */
/*         pd->items.array.len = pd->items.array.max = pd->outstanding_slice.slice; */
/*      } */

/*    pd->realized.start = pd->outstanding_slice.slice_start; */
/*    pd->realized.slice = pd->outstanding_slice.slice; */

/*    idx = 0; */
/*    while(removing_before < 0) */
/*      { */
/*         Efl_Ui_List_Item** members = pd->items.array.members; */
/*         Efl_Ui_List_Item** item = &members[idx]; */

/*         // initialize item */
/*         void* model = NULL; */
/*         int r = eina_accessor_data_get(acc, idx + pd->realized.start - pd->outstanding_slice.slice_start, &model); */
/*         assert(r != EINA_FALSE); */
/*         assert(model != NULL); */

/*         *item = _child_setup(pd, model, &recycle_layouts, idx); */

/*         idx++; */
/*         removing_before++; */
/*      } */

/*    idx = copy_size + to_begin; */
/*    while(removing_after < 0) */
/*      { */
/*         Efl_Ui_List_Item** members = pd->items.array.members; */
/*         Efl_Ui_List_Item** item = &members[idx]; */

/*         // initialize item */
/*         void* model = NULL; */
/*         int r = eina_accessor_data_get(acc, idx + pd->realized.start - pd->outstanding_slice.slice_start, &model); */
/*         assert(r != EINA_FALSE); */
/*         assert(model != NULL); */

/*         *item = _child_setup(pd, model, &recycle_layouts, idx); */

/*         idx++; */
/*         removing_after++; */
/*      } */

/*    { */
/*       Efl_Ui_Layout **layout; */
/*       EINA_INARRAY_FOREACH(&recycle_layouts, layout) */
/*         { */
/*            efl_ui_factory_release(pd->factory, *layout); */
/*         } */
/*       free(recycle_layouts.members); */
/*    } */
/* } */

/* static void */
/* _children_then(void * data, Efl_Event const* event) */
/* { */
/*    Efl_Ui_List_Data *pd = data; */
/*    Eina_Accessor *acc = (Eina_Accessor*)((Efl_Future_Event_Success*)event->info)->value; */
/*    int removing_before = -pd->realized.start + pd->outstanding_slice.slice_start; */
/*    int removing_after = pd->realized.start + pd->realized.slice */
/*      - (pd->outstanding_slice.slice_start + pd->outstanding_slice.slice); */

/*    pd->future = NULL; */
/*    // If current slice doesn't reach new slice */
/*    if(pd->realized.start + pd->realized.slice < pd->outstanding_slice.slice_start */
/*       || pd->outstanding_slice.slice_start + pd->outstanding_slice.slice < pd->realized.start) */
/*      { */
/*        removing_before = pd->realized.slice; */
/*        removing_after = -pd->outstanding_slice.slice; */
/*      } */

/*    /\* _resize_children(pd, removing_before, removing_after, acc); *\/ */
/*    evas_object_smart_changed(pd->obj); */
/* } */

//static void
//_efl_ui_list_children_free(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
//{
//   EINA_SAFETY_ON_NULL_RETURN(pd);
//
//   EINA_INARRAY_FOREACH(&recycle_layouts, layout)
//     {
//       efl_ui_factory_release(pd->factory, *layout);
//     }
//
//   pd->realized.slice = 0;
//   pd->realized.start = 0;
//   free(recycle_layouts.members);
//}

static void
_children_slice_error(void * data EINA_UNUSED, Efl_Event const* event EINA_UNUSED)
{
   Efl_Ui_List_Data *pd = data;
   EINA_SAFETY_ON_NULL_RETURN(pd);
   pd->slice_future = NULL;
}

static void
_show_region_hook(void *data EINA_UNUSED, Evas_Object *obj)
{
   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);
   Eina_Rect r = elm_obj_widget_show_region_get(obj);
   elm_pan_pos_get(pd->pan_obj, &r.x, &r.y);
   elm_interface_scrollable_content_region_show(obj, r.x, r.y, r.w, r.h);
}

EOLIAN static void
_efl_ui_list_select_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, Elm_Object_Select_Mode mode)
{
   /* Eina_Array_Iterator iterator; */
   /* Efl_Ui_List_Item *item; */
   /* unsigned int i; */

   if (pd->select_mode == mode)
     return;

   /* if (pd->select_mode == ELM_OBJECT_SELECT_MODE_NONE) */
   /*   { */
   /*      EINA_ARRAY_ITER_NEXT(pd->items.array, i, item, iterator) */
   /*        { */
   /*           if (item->selected) */
   /*             elm_layout_signal_emit(item->layout, "elm,state,selected", "elm"); */

   /*           efl_ui_model_connect(item->layout, "signal/elm,state,%v", "selected"); */
   /*        } */
   /*   } */
   /* else if (mode == ELM_OBJECT_SELECT_MODE_NONE) */
   /*   { */
   /*      EINA_ARRAY_ITER_NEXT(pd->items.array, i, item, iterator) */
   /*        { */
   /*           if (item->selected) */
   /*             elm_layout_signal_emit(item->layout, "elm,state,unselected", "elm"); */

   /*           efl_ui_model_connect(item->layout, "signal/elm,state,%v", NULL); */
   /*        } */
   /*   } */
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

EOLIAN static Efl_Ui_Theme_Apply
_efl_ui_list_elm_widget_theme_apply(Eo *obj, Efl_Ui_List_Data *pd EINA_UNUSED)
{
   return elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_list_efl_gfx_position_set(Eo *obj, Efl_Ui_List_Data *pd, Eina_Position2D p)
{
   int pan_x, pan_y;
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, p.x, p.y))
     return;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), p);

   elm_pan_pos_get(pd->pan_obj, &pan_x, &pan_y);
   evas_object_move(pd->hit_rect, p.x, p.y);
   evas_object_move(pd->pan_obj, p.x - pan_x, p.y - pan_y);
   evas_object_smart_changed(pd->obj);
}

EOLIAN static void
_efl_ui_list_elm_interface_scrollable_region_bring_in(Eo *obj, Efl_Ui_List_Data *pd, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   int pan_x, pan_y;
   elm_pan_pos_get(pd->pan_obj, &pan_x, &pan_y);
   elm_interface_scrollable_region_bring_in(efl_super(obj, MY_CLASS), x + pan_x, y + pan_y, w, h);
}

EOLIAN static void
_efl_ui_list_efl_gfx_size_set(Eo *obj, Efl_Ui_List_Data *pd, Eina_Size2D size)
{
   Evas_Coord oldw, oldh;
   Eina_Bool load = EINA_FALSE;

   DBG("w: %d h: %d", size.w, size.h);
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, size.w, size.h))
     return;

   evas_object_geometry_get(obj, NULL, NULL, &oldw, &oldh);
   efl_gfx_size_set(efl_super(obj, MY_CLASS), size);
   evas_object_resize(pd->hit_rect, size.w, size.h);

   if (_horiz(pd->orient))
     {
        if (size.w != oldw) load = EINA_TRUE;
     }
   else
     {
        if (size.h != oldh) load = EINA_TRUE;
     }

   /* if (load && _update_items(obj, pd)) */
   /*   return; */

   evas_object_smart_changed(pd->obj);
}

EOLIAN static void
_efl_ui_list_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_List_Data *pd)
{
   DBG("");
   /* if (pd->recalc) return; */

   _layout(pd);
   /* _efl_ui_list_custom_layout(obj); */
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
   Eina_Size2D min = { -1, }, max = {-1,};
   Evas_Coord vmw = 0, vmh = 0;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   min = efl_gfx_size_hint_combined_min_get(obj);
   evas_object_size_hint_max_get(obj, &max.w, &max.h);
   edje_object_size_min_calc(wd->resize_obj, &vmw, &vmh);

   min.w = vmw;
   min.h = vmh;

   if ((max.w > 0) && (min.w > max.w))
     min.w = max.w;
   if ((max.h > 0) && (min.h > max.h))
     min.h = max.h;

   evas_object_size_hint_min_set(obj, min.w, min.h);
   evas_object_size_hint_max_set(obj, max.w, max.h);
}

EOLIAN static void
_efl_ui_list_efl_canvas_group_group_add(Eo *obj, Efl_Ui_List_Data *pd EINA_UNUSED)
{
   Efl_Ui_List_Pan_Data *pan_data;
   Evas_Coord minw, minh;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   pd->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_data_set(pd->hit_rect, "_elm_leaveme", obj);
   evas_object_smart_member_add(pd->hit_rect, obj);
   elm_widget_sub_object_add(obj, pd->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(pd->hit_rect, 0, 0, 0, 0);
   evas_object_show(pd->hit_rect);
   evas_object_repeat_events_set(pd->hit_rect, EINA_TRUE);

   elm_widget_on_show_region_hook_set(obj, NULL, _show_region_hook, NULL);

   if (!elm_layout_theme_set(obj, "list", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_interface_scrollable_objects_set(obj, wd->resize_obj, pd->hit_rect);
   elm_interface_scrollable_bounce_allow_set
           (obj, EINA_FALSE, _elm_config->thumbscroll_bounce_enable);

   pd->mode = ELM_LIST_COMPRESS;

   elm_interface_atspi_accessible_type_set(obj, ELM_ATSPI_TYPE_DISABLED);
   pd->pan_obj = efl_add(MY_PAN_CLASS, evas_object_evas_get(obj));
   pan_data = efl_data_scope_get(pd->pan_obj, MY_PAN_CLASS);
   pan_data->wobj = obj;

   elm_interface_scrollable_extern_pan_set(obj, pd->pan_obj);
   evas_object_show(pd->pan_obj);

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_list_efl_canvas_group_group_del(Eo *obj, Efl_Ui_List_Data *pd)
{
   //_efl_ui_list_children_free(obj, pd);

   ELM_SAFE_FREE(pd->pan_obj, evas_object_del);
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_list_elm_widget_focus_manager_create(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *root)
{
   if (!pd->manager)
     pd->manager = efl_add(EFL_UI_FOCUS_MANAGER_CALC_CLASS, obj,
                          efl_ui_focus_manager_root_set(efl_added, root));

   return pd->manager;
}

EOLIAN static Eo *
_efl_ui_list_efl_object_finalize(Eo *obj, Efl_Ui_List_Data *pd)
{
   if(!pd->relayout)
     {
        pd->relayout = efl_add(EFL_UI_LIST_PRECISE_LAYOUTER_CLASS, obj);
        efl_ui_list_relayout_model_set(pd->relayout, pd->model);
     }
   return obj;
}

EOLIAN static Eo *
_efl_ui_list_efl_object_constructor(Eo *obj, Efl_Ui_List_Data *pd)
{
   Efl_Ui_Focus_Manager *manager;

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->obj = obj;
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_LIST);

   efl_ui_list_segarray_setup(&pd->segarray, 32);

   manager = elm_obj_widget_focus_manager_create(obj, obj);
   efl_composite_attach(obj, manager);
   _efl_ui_focus_manager_redirect_events_add(manager, obj);

   pd->style = eina_stringshare_add(elm_widget_style_get(obj));

   pd->factory = NULL;
   pd->orient = EFL_ORIENT_DOWN;
   pd->align.h = 0;
   pd->align.v = 0;
   pd->min.w = 0;
   pd->min.h = 0;

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
_efl_ui_list_layout_factory_set(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, Efl_Ui_Factory *factory)
{
   //TODO: clean all current layouts??
   if (pd->factory)
     efl_unref(pd->factory);

   pd->factory = efl_ref(factory);
}

EOLIAN static void
_efl_ui_list_efl_ui_view_model_set(Eo *obj, Efl_Ui_List_Data *pd, Efl_Model *model)
{
   if (pd->model == model)
     return;

   if (pd->count_future)
     {
        efl_future_cancel(pd->count_future);
        pd->count_future = NULL;
     }

   if (pd->model)
     {
        /* efl_event_callback_del(pd->model, EFL_MODEL_EVENT_CHILD_ADDED, _child_added_cb, obj); */
        /* efl_event_callback_del(pd->model, EFL_MODEL_EVENT_CHILD_REMOVED, _child_removed_cb, obj); */
        //TODO: FIXME: XXX: SegArray Clear
        efl_ui_list_segarray_setup(&pd->segarray, 32);
        efl_unref(pd->model);
        pd->model = NULL;
        pd->item_count = 0;
     }

   if (!pd->factory)
     pd->factory = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, obj);

   if (model)
     {
        pd->model = model;
        efl_ref(pd->model);
        /* efl_event_callback_add(pd->model, EFL_MODEL_EVENT_CHILD_ADDED, _child_added_cb, obj); */
        /* efl_event_callback_add(pd->model, EFL_MODEL_EVENT_CHILD_REMOVED, _child_removed_cb, obj); */
        pd->count_future = efl_model_children_count_get(pd->model);
        efl_future_then(pd->count_future, &_count_then, &_count_error, NULL, pd);
     }

   if (pd->relayout)
      efl_ui_list_relayout_model_set(pd->relayout, model);

   evas_object_smart_changed(pd->obj);
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
   Efl_Ui_List_Item **litem;
   Eina_List *ret = NULL, *ret2 = NULL;

   /* EINA_INARRAY_FOREACH(&pd->items.array, litem) */
   /*    ret = eina_list_append(ret, (*litem)->layout); */

   ret2 = elm_interface_atspi_accessible_children_get(efl_super(obj, MY_CLASS));
   return eina_list_merge(ret, ret2);
}

EOLIAN int
_efl_ui_list_efl_access_selection_selected_children_count_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   return eina_list_count(pd->selected_items);
}

EOLIAN Eo*
_efl_ui_list_efl_access_selection_selected_child_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, int child_index)
{
   if(child_index <  (int) eina_list_count(pd->selected_items))
     {
        Efl_Ui_List_Item* items = eina_list_nth(pd->selected_items, child_index);
        return items[child_index].layout;
     }
   else
     return NULL;
}

EOLIAN Eina_Bool
_efl_ui_list_efl_access_selection_child_select(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd EINA_UNUSED, int child_index EINA_UNUSED)
{
   /* if (pd->select_mode != ELM_OBJECT_SELECT_MODE_NONE) */
   /*   { */
   /*     if(child_index < eina_inlist_count(&pd->items.array)) */
   /*       { */
   /*          Efl_Ui_List_Item* items = pd->items.array.members; */
   /*          _efl_ui_list_item_select_set(&items[child_index], EINA_TRUE); */
   /*          return EINA_TRUE; */
   /*       } */
   /*   } */
   return EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_ui_list_efl_access_selection_selected_child_deselect(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd EINA_UNUSED, int child_index EINA_UNUSED)
{
/*    Efl_Ui_List_Item *item = eina_list_nth(pd->selected, child_index); */
/*    if (item) */
/*      { */
/*         _efl_ui_list_item_select_set(pd, item->layout, EINA_FALSE); */
/*         return EINA_TRUE; */
/*      } */
   return EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_ui_list_efl_access_selection_is_child_selected(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, int child_index)
{
   /* Efl_Ui_List_Item **item = eina_inarray_nth(&pd->items.array, child_index); */
   /* EINA_SAFETY_ON_NULL_RETURN_VAL(*item, EINA_FALSE); */
   return EINA_FALSE;// (*item)->selected;
}

EOLIAN Eina_Bool
_efl_ui_list_efl_access_selection_all_children_select(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   /* Efl_Ui_List_Item **item; */

   /* if (pd->select_mode == ELM_OBJECT_SELECT_MODE_NONE) */
   /*   return EINA_FALSE; */

   /* EINA_INARRAY_FOREACH(&pd->items.array, item) */
   /*    _efl_ui_list_item_select_set(*item, EINA_TRUE); */

   return EINA_TRUE;
}

EOLIAN Eina_Bool
_efl_ui_list_efl_access_selection_clear(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   Efl_Ui_List_Item *item;
   Eina_List *l;

   /* if (pd->select_mode == ELM_OBJECT_SELECT_MODE_NONE) */
   /*   return EINA_FALSE; */

   /* EINA_LIST_FOREACH(pd->selected_items, l, item) */
   /*    _efl_ui_list_item_select_set(item, EINA_FALSE); */

   return EINA_TRUE;
}

EOLIAN Eina_Bool
_efl_ui_list_efl_access_selection_child_deselect(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd EINA_UNUSED, int child_index EINA_UNUSED)
{
   /* Efl_Ui_List_Item *item = eina_array_data_get(pd->items.array, child_index); */
   /* if (item) */
   /*   { */
   /*    _efl_ui_list_item_select_set(item, EINA_FALSE); */
   /*      return EINA_TRUE; */
   /*   } */

   return EINA_FALSE;
}

static Eina_Bool
_key_action_move(Evas_Object *obj, const char *params)
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
        elm_obj_pan_pos_max_get(pd->pan.obj, &x, &y);
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
   return EINA_TRUE;
}

static Eina_Bool
_key_action_select(Evas_Object *obj, const char *params EINA_UNUSED)
{
   EFL_UI_LIST_DATA_GET_OR_RETURN_VAL(obj, pd, EINA_FALSE);

   /* Eo *focused = efl_ui_focus_manager_focused(pd->manager); */
   /* if (focused) */
   /*   _efl_ui_list_item_select_set(item, EINA_TRUE); */

   return EINA_FALSE;
}

static Eina_Bool
_key_action_escape(Evas_Object *obj, const char *params EINA_UNUSED)
{
   return _efl_ui_list_item_select_clear(obj);
}

EOLIAN static Eina_Bool
_efl_ui_list_elm_widget_widget_event(Eo *obj, Efl_Ui_List_Data *pd, Efl_Event const* event, Evas_Object *src/*, Evas_Callback_Type type, void *event_info*/)
{
   /* (void)src; */
   /* Evas_Event_Key_Down *ev = event->info; */

   /* if (event->desc != EFL_EVENT_KEY_DOWN) return EINA_FALSE; */
   /* if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE; */
   /* if (!eina_inarray_count(&pd->items.array)) return EINA_FALSE; */

   /* if (!_elm_config_key_binding_call(obj, MY_CLASS_NAME, ev, key_actions)) */
   /*   return EINA_FALSE; */

   /* ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD; */
   return EINA_TRUE;
}

Eina_Bool
_efl_ui_list_item_select_clear(Eo *obj)
{
   /* Eina_List *li; */
   /* Efl_Ui_List_Item *item; */
   /* EFL_UI_LIST_DATA_GET_OR_RETURN_VAL(obj, pd, EINA_FALSE); */

   /* EINA_LIST_FOREACH(pd->selected_items, li, item) */
   /*    _efl_ui_list_item_select_set(item, EINA_FALSE); */

   return EINA_TRUE;
}

void
_efl_ui_list_item_select_set(Efl_Ui_List_Item *item, Eina_Bool selected)
{
   /* Eina_Stringshare *sprop, *svalue; */

   /* assert(item != NULL); */
   /* EFL_UI_LIST_DATA_GET_OR_RETURN(item->list, pd); */

   /* if ((pd->select_mode == ELM_OBJECT_SELECT_MODE_NONE) || */
   /*     (pd->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)) */
   /*   return; */

   /* selected = !!selected; */
   /* /\* if (!item->model || item->selected == selected) return; *\/ */
   /* assert(item->model != NULL); */

   /* sprop = eina_stringshare_add(SELECTED_PROP); */
   /* svalue = (selected ? eina_stringshare_add("selected") : eina_stringshare_add("unselected")); */

   /* if (_efl_model_properties_has(item->model, sprop)) */
   /*   { */
   /*      Eina_Value v; */
   /*      eina_value_setup(&v, EINA_VALUE_TYPE_STRINGSHARE); */
   /*      eina_value_set(&v, svalue); */
   /*      efl_model_property_set(item->model, sprop, &v); */
   /*      eina_value_flush(&v); */
   /*   } */
   /* eina_stringshare_del(sprop); */
   /* eina_stringshare_del(svalue); */

   /* /\* //TODO I need call this event or catch only by model connect event? *\/ */
   /* if (selected) */
   /*   efl_event_callback_legacy_call(item->layout, EFL_UI_EVENT_SELECTED, item); */
   /* else */
   /*   efl_event_callback_legacy_call(item->layout, EFL_UI_EVENT_UNSELECTED, item); */
}

static void
_efl_ui_list_relayout_set(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd EINA_UNUSED, Efl_Ui_List_Relayout *object)
{
   if(pd->relayout)
     efl_unref(pd->relayout);
   pd->relayout = efl_ref(object);
}

static Efl_Ui_List_Relayout *
_efl_ui_list_relayout_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd EINA_UNUSED)
{
   return pd->relayout;
}

static void
_layout(Efl_Ui_List_Data *pd)
{
   if (!pd->model)
     return;

   Eina_Accessor* accessor = efl_ui_list_segarray_accessor_get(&pd->segarray);

   efl_ui_list_relayout_layout_do(pd->relayout, pd->obj, pd->segarray_first,
                                  efl_ui_list_segarray_count(&pd->segarray), accessor);
}

static void
_children_slice_then(void * data, Efl_Event const* event)
{
   Efl_Ui_List_Data *pd = data;
   Eina_Accessor *acc = (Eina_Accessor*)((Efl_Future_Event_Success*)event->info)->value;

   efl_ui_list_segarray_insert_accessor(&pd->segarray, pd->outstanding_slice.slice_start, acc);

   pd->segarray_first = pd->outstanding_slice.slice_start;
   pd->outstanding_slice.slice_start = pd->outstanding_slice.slice_count = 0;
   pd->slice_future = NULL;
}

/* EFL UI LIST MODEL INTERFACE */
EOLIAN static Eina_Size2D
_efl_ui_list_efl_ui_list_model_min_size_get(Eo *obj, Efl_Ui_List_Data *pd)
{
   return pd->min;
}

EOLIAN static void
_efl_ui_list_efl_ui_list_model_min_size_set(Eo *obj, Efl_Ui_List_Data *pd, Eina_Size2D min)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   pd->min.w = min.w;
   pd->min.h = min.h;

   evas_object_size_hint_min_set(wd->resize_obj, pd->min.w, pd->min.h);
   efl_event_callback_legacy_call(pd->pan_obj, ELM_PAN_EVENT_CHANGED, NULL);
}

EOLIAN static Efl_Ui_List_LayoutItem *
_efl_ui_list_efl_ui_list_model_realize(Eo *obj, Efl_Ui_List_Data *pd, Efl_Ui_List_LayoutItem *item)
{
   Efl_Ui_List_Item_Event evt;
   DBG("model_realize");
   EINA_SAFETY_ON_NULL_RETURN_VAL(item->children, item);

   item->layout = efl_ui_factory_create(pd->factory, item->children, obj);
//   evas_object_event_callback_del_full(item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change, item);
   efl_ui_view_model_set(item->layout, item->children);

   evt.child = item->children;
   evt.layout = item->layout;
   evt.index = item->index;
   efl_event_callback_call(obj, EFL_UI_LIST_EVENT_ITEM_REALIZED, &evt);

   evas_object_show(item->layout);
   return item;
}

EOLIAN static void
_efl_ui_list_efl_ui_list_model_unrealize(Eo *obj, Efl_Ui_List_Data *pd, Efl_Ui_List_LayoutItem *item)
{
   Efl_Ui_List_Item_Event evt;
   DBG("model_unrealize item:%p", item);
   EINA_SAFETY_ON_NULL_RETURN(item->layout);

   evas_object_hide(item->layout);
   evas_object_move(item->layout, -9999, -9999);

   evt.child = item->children;
   evt.layout = item->layout;
   evt.index = item->index;
   efl_event_callback_call(obj, EFL_UI_LIST_EVENT_ITEM_UNREALIZED, &evt);

   efl_ui_view_model_set(item->layout, NULL);
   efl_ui_factory_release(pd->factory, item->layout);
   item->layout = NULL;
}

EOLIAN static void
_efl_ui_list_efl_ui_list_model_load_range_set(Eo* obj, Efl_Ui_List_Data* pd, int first, int count)
{
   //FIXME?? slice cancel??
   if(!pd->slice_future)
     {
        pd->slice_future = efl_model_children_slice_get(pd->model, first, count);
        pd->outstanding_slice.slice_start = first;
        pd->outstanding_slice.slice_count = count;
        efl_future_then(pd->slice_future, &_children_slice_then, &_children_slice_error, NULL, pd);
     }
}

EOLIAN static int
_efl_ui_list_efl_ui_list_model_size_get(Eo *obj, Efl_Ui_List_Data *pd)
{
    /* TODO */
    return pd->item_count;
}

/* layout */
/* static Eina_Bool */
/* _update_items(Eo *obj, Efl_Ui_List_Data *pd) */
/* { */
/*    int want_slice, want_slice_start = 0; */
/*    Evas_Coord w = 0, h = 0; */
/*    Eina_Bool horz = _horiz(pd->orient); */

/*    /\* assert(!pd->future); *\/ */
/*    if (pd->future) */
/*      { */
/*         efl_future_cancel(pd->future); */
/*         pd->future = NULL; */
/*      } */

/*    int average_item_size = eina_inarray_count(&pd->items.array) ? (horz ? pd->realized.w : pd->realized.h) / eina_inarray_count(&pd->items.array) : AVERAGE_SIZE_INIT; */
/*    if(!average_item_size) */
/*      average_item_size = AVERAGE_SIZE_INIT; */

/*    evas_object_geometry_get(obj, NULL, NULL, &w, &h); */
/*    if (horz) */
/*      { */
/*         want_slice  = (w / average_item_size) * 2; */
/*         want_slice_start = (pd->pan.x / average_item_size) - (want_slice / 4); */
/*         if(want_slice_start < 0) */
/*           want_slice_start = 0; */
/*      } */
/*    else */
/*      { */
/*         want_slice =  (h / average_item_size) * 2; */
/*         want_slice_start = (pd->pan.y / average_item_size) - (want_slice / 4); */
/*         if(want_slice_start < 0) */
/*           want_slice_start = 0; */
/*      } */

/*    assert(want_slice_start >= 0); */
/*    if(want_slice < 8) */
/*      want_slice = 8; */
/*    if(want_slice_start + want_slice > pd->item_count) */
/*      { */
/*        if(want_slice > pd->item_count) */
/*          want_slice = pd->item_count; */
/*        want_slice_start = pd->item_count - want_slice; */
/*        assert(want_slice_start >= 0); */
/*      } */

/*    if(want_slice != 0) */
/*      { */
/*        pd->outstanding_slice.slice_start = want_slice_start; */
/*        pd->outstanding_slice.slice = want_slice; */
/*        pd->future = efl_model_children_slice_get(pd->model, want_slice_start, want_slice); */
/*        efl_future_then(pd->future, &_children_then, &_children_error, NULL, pd); */
/*      } */

/*      return EINA_TRUE; */
/* } */

/* void */
/* _efl_ui_list_custom_layout(Efl_Ui_List *ui_list) */
/* { */
/*    EFL_UI_LIST_DATA_GET_OR_RETURN(ui_list, pd); */
/*    Efl_Ui_List_Item *litem, **it; */

/*    Eina_Bool horiz = _horiz(pd->orient), zeroweight = EINA_FALSE; */
/*    Evas_Coord ow, oh, want, minw, minh; */
/*    int boxx, boxy, boxw, boxh, length, pad, extra = 0, rounding = 0; */
/*    int boxl = 0, boxr = 0, boxt = 0, boxb = 0; */
/*    double cur_pos = 0, scale, box_align[2],  weight[2] = { 0, 0 }; */
/*    Eina_Bool box_fill[2] = { EINA_FALSE, EINA_FALSE }; */
/*    int count = 0; */
/*    Eina_List *order = NULL; */

/*    ELM_WIDGET_DATA_GET_OR_RETURN(ui_list, wd); */

/*    evas_object_geometry_get(ui_list, &boxx, &boxy, &boxw, &boxh); */
/*    efl_gfx_size_hint_margin_get(ui_list, &boxl, &boxr, &boxt, &boxb); */

/*    scale = evas_object_scale_get(ui_list); */
/*    // Box align: used if "item has max size and fill" or "no item has a weight" */
/*    // Note: cells always expand on the orthogonal direction */
/*    box_align[0] = pd->align.h; */
/*    box_align[1] = pd->align.v; */
/*    if (box_align[0] < 0) */
/*      { */
/*         box_fill[0] = EINA_TRUE; */
/*         box_align[0] = 0.5; */
/*      } */
/*    if (box_align[1] < 0) */
/*      { */
/*         box_fill[1] = EINA_TRUE; */
/*         box_align[1] = 0.5; */
/*      } */

/*    count = eina_inarray_count(&pd->items.array); */

/*    elm_interface_scrollable_content_viewport_geometry_get */
/*               (ui_list, NULL, NULL, &ow, &oh); */
/*    // box outer margin */
/*    boxw -= boxl + boxr; */
/*    boxh -= boxt + boxb; */
/*    boxx += boxl; */
/*    boxy += boxt; */

/*    int average_item_size = eina_inarray_count(&pd->items.array) ? (/\*horz*\/ EINA_FALSE ? pd->realized.w : pd->realized.h) / eina_inarray_count(&pd->items.array) : AVERAGE_SIZE_INIT; */
/*    if(!average_item_size) */
/*      average_item_size = AVERAGE_SIZE_INIT; */

/*    // total space & available space */
/*    if (horiz) */
/*      { */
/*         length = boxw; */
/*         want = pd->realized.w; */
/*         pad = pd->pad.scalable ? (pd->pad.h * scale) : pd->pad.h; */

/*         // padding can not be squeezed (note: could make it an option) */
/*         length -= pad * (count - 1); */
/*         // available space. if <0 we overflow */
/*         extra = length - want; */

/*         minw = pd->realized.w + boxl + boxr + pad * (count - 1); */
/*         minh = pd->realized.h + boxt + boxb; */
/*         if (pd->item_count > count) */
/*           minw = pd->item_count * average_item_size; */
/*      } */
/*    else */
/*      { */
/*         length = boxh; */
/*         want = pd->realized.h; */
/*         pad = pd->pad.scalable ? (pd->pad.v * scale) : pd->pad.v; */

/*         // padding can not be squeezed (note: could make it an option) */
/*         length -= pad * (count - 1); */
/*         // available space. if <0 we overflow */
/*         extra = length - want; */

/*         minw = pd->realized.w + boxl + boxr; */
/*         minh = pd->realized.h + pad * (count - 1) + boxt + boxb; */
/*         if (pd->item_count > count) */
/*           minh = pd->item_count * average_item_size; */
/*      } */

/*    if (pd->minh != minh || pd->minw != minw) */
/*      { */
/*         pd->minw = minw; */
/*         pd->minh = minh; */

/*         efl_event_callback_legacy_call(pd->pan.obj, ELM_PAN_EVENT_CHANGED, NULL); */
/*      } */

/*    evas_object_size_hint_min_set(wd->resize_obj, pd->minw, pd->minh); */

/*    if (extra < 0) extra = 0; */

/*    weight[0] = pd->weight.x; */
/*    weight[1] = pd->weight.y; */
/*    if (EINA_DBL_EQ(weight[!horiz], 0)) */
/*      { */
/*         if (box_fill[!horiz]) */
/*           { */
/*              // box is filled, set all weights to be equal */
/*              zeroweight = EINA_TRUE; */
/*           } */
/*         else */
/*           { */
/*              // move bounding box according to box align */
/*              cur_pos = extra * box_align[!horiz]; */
/*           } */
/*         weight[!horiz] = count; */
/*      } */

/*    cur_pos += average_item_size * pd->realized.start; */
/*    // scan all items, get their properties, calculate total weight & min size */
/*    EINA_INARRAY_FOREACH(&pd->items.array, it) */
/*      { */
/*         litem = *it; */
/*         double cx, cy, cw, ch, x, y, w, h; */
/*         double align[2]; */
/*         int item_pad[4]; */
/*         Eina_Size2D max; */

/*         assert(litem->layout != NULL); */
/*         efl_gfx_size_hint_align_get(litem->layout, &align[0], &align[1]); */
/*         max = efl_gfx_size_hint_max_get(litem->layout); */
/*         efl_gfx_size_hint_margin_get(litem->layout, &item_pad[0], &item_pad[1], &item_pad[2], &item_pad[3]); */

/*         if (align[0] < 0) align[0] = -1; */
/*         if (align[1] < 0) align[1] = -1; */
/*         if (align[0] > 1) align[0] = 1; */
/*         if (align[1] > 1) align[1] = 1; */

/*         if (max.w <= 0) max.w = INT_MAX; */
/*         if (max.h <= 0) max.h = INT_MAX; */
/*         if (max.w < litem->minw) max.w = litem->minw; */
/*         if (max.h < litem->minh) max.h = litem->minh; */

/*         /\* // extra rounding up (compensate cumulative error) *\/ */
/*         /\* if ((id == (count - 1)) && (cur_pos - floor(cur_pos) >= 0.5)) *\/ */
/*         /\*   rounding = 1; *\/ */

/*         if (horiz) */
/*           { */
/*              cx = boxx + cur_pos; */
/*              cy = boxy; */
/*              cw = litem->minw + rounding + (zeroweight ? 1.0 : litem->wx) * extra / weight[0]; */
/*              ch = boxh; */
/*              cur_pos += cw + pad; */
/*           } */
/*         else */
/*           { */
/*              cx = boxx; */
/*              cy = boxy + cur_pos; */
/*              cw = boxw; */
/*              ch = litem->minh + rounding + (zeroweight ? 1.0 : litem->wy) * extra / weight[1]; */
/*              cur_pos += ch + pad; */
/*           } */

/*         // horizontally */
/*         if (max.w < INT_MAX) */
/*           { */
/*              w = MIN(MAX(litem->minw - item_pad[0] - item_pad[1], max.w), cw); */
/*              if (align[0] < 0) */
/*                { */
/*                   // bad case: fill+max are not good together */
/*                   x = cx + ((cw - w) * box_align[0]) + item_pad[0]; */
/*                } */
/*              else */
/*                x = cx + ((cw - w) * align[0]) + item_pad[0]; */
/*           } */
/*         else if (align[0] < 0) */
/*           { */
/*              // fill x */
/*              w = cw - item_pad[0] - item_pad[1]; */
/*              x = cx + item_pad[0]; */
/*           } */
/*         else */
/*           { */
/*              w = litem->minw - item_pad[0] - item_pad[1]; */
/*              x = cx + ((cw - w) * align[0]) + item_pad[0]; */
/*           } */

/*         // vertically */
/*         if (max.h < INT_MAX) */
/*           { */
/*              h = MIN(MAX(litem->minh - item_pad[2] - item_pad[3], max.h), ch); */
/*              if (align[1] < 0) */
/*                { */
/*                   // bad case: fill+max are not good together */
/*                   y = cy + ((ch - h) * box_align[1]) + item_pad[2]; */
/*                } */
/*              else */
/*                y = cy + ((ch - h) * align[1]) + item_pad[2]; */
/*           } */
/*         else if (align[1] < 0) */
/*           { */
/*              // fill y */
/*              h = ch - item_pad[2] - item_pad[3]; */
/*              y = cy + item_pad[2]; */
/*           } */
/*         else */
/*           { */
/*              h = litem->minh - item_pad[2] - item_pad[3]; */
/*              y = cy + ((ch - h) * align[1]) + item_pad[2]; */
/*           } */

/*         if (horiz) */
/*           { */
/*              if (h < pd->minh) h = pd->minh; */
/*              if (h > oh) h = oh; */
/*           } */
/*         else */
/*           { */
/*              if (w < pd->minw) w = pd->minw; */
/*              if (w > ow) w = ow; */
/*           } */

/*         evas_object_geometry_set(litem->layout, (x + 0 - pd->pan.x), (y + 0 - pd->pan.y), w, h); */

/*         litem->x = x; */
/*         litem->y = y; */
/*         order = eina_list_append(order, litem->layout); */

/* //        fprintf(stderr, "x: %.2f y: %.2f w: %.2f h: %.2f old x: %.2f old y: %.2f old w: %.2f old h: %.2f\n" */
/* //                , (x + 0 - pd->pan.x), (y + 0 - pd->pan.y), (float)w, (float)h */
/* //               , (float)litem->x, (float)litem->y, (float)litem->w, (float)litem->h); fflush(stderr); */
/* //        printf("obj=%d currpos=%.2f moved to X=%.2f, Y=%.2f average_item_size %d\n", litem->index, cur_pos, x, y */
/* //               , eina_inarray_count(&pd->items.array) ? (/\*horz*\/ EINA_FALSE ? pd->realized.w : pd->realized.h) / eina_inarray_count(&pd->items.array) : AVERAGE_SIZE_INIT); */
/*      } */

/*   efl_ui_focus_manager_calc_update_order(pd->manager, pd->obj, order); */
/* } */

/* Internal EO APIs and hidden overrides */

#define EFL_UI_LIST_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_list)

#include "efl_ui_list.eo.c"
#include "efl_ui_list_relayout.eo.c"
#include "efl_ui_list_model.eo.c"

#include "efl_ui_list_private.h"

#include <assert.h>

#define MY_CLASS EFL_UI_LIST_CLASS
#define MY_CLASS_NAME "Efl.Ui.List"

#define MY_PAN_CLASS EFL_UI_LIST_PAN_CLASS

#define SIG_CHILD_ADDED "child,added"
#define SIG_CHILD_REMOVED "child,removed"
#define SELECTED_PROP "selected"
#define AVERAGE_SIZE_INIT 10

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHILD_ADDED, ""},
   {SIG_CHILD_REMOVED, ""},
   {NULL, NULL}
};

void _efl_ui_list_custom_layout(Efl_Ui_List *);
void _efl_ui_list_item_select_set(Efl_Ui_List_Data *, Eo*, Eina_Bool);
Eina_Bool _efl_ui_list_item_select_clear(Eo *);
static Efl_Ui_List_Item *_child_new(Efl_Ui_List_Data *, Efl_Model *);
static void _child_remove(Efl_Ui_List_Data *, Efl_Ui_List_Item *);
static void _item_calc(Efl_Ui_List_Data *, Efl_Ui_List_Item *);
static void _layout_realize(Efl_Ui_List_Data *, Efl_Ui_List_Item *);
static void _layout_unrealize(Efl_Ui_List_Data *, Efl_Ui_List_Item *);
static Eina_Bool _update_items(Eo *, Efl_Ui_List_Data * /*, Eina_Bool*/);

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
   Evas_Coord ox, oy, ow, oh, cw;
   Efl_Ui_List_Data *pd = psd->wpd;
   Efl_Ui_List_Item *litem;

   fprintf(stderr, "%s %s:%d x %d y %d\n", __func__, __FILE__, __LINE__, x, y); fflush(stderr);
   EINA_SAFETY_ON_NULL_RETURN(pd);
   if (((x == pd->pan.x) && (y == pd->pan.y))) return;

   evas_object_geometry_get(pd->obj, &ox, &oy, &ow, &oh);
   if (_horiz(pd->orient))
     {
        pd->pan.move_diff += x - pd->pan.x;
        cw = ow / 4;
     }
   else
     {
        pd->pan.move_diff += y - pd->pan.y;
        cw = oh / 4;
     }

   pd->pan.x = x;
   pd->pan.y = y;

   if (abs(pd->pan.move_diff) > cw)
     {
        pd->pan.move_diff = 0;
        _update_items(obj, pd/*, EINA_FALSE*/);
     }
   else
     {
        EINA_INARRAY_FOREACH(&pd->items.array, litem)
          evas_object_move(litem->layout, (litem->x + 0 - pd->pan.x), (litem->y + 0 - pd->pan.y));
     }
}

EOLIAN static void
_efl_ui_list_pan_elm_pan_pos_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Pan_Data *psd, Evas_Coord *x, Evas_Coord *y)
{
   Efl_Ui_List_Data *pd = psd->wpd;

   if (x) *x = pd->pan.x;
   if (y) *y = pd->pan.y;
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
   /* pd->item_count++; */

//FIXME: the new data is visible? is yes reload sliced children and test if have changes
//   _child_new(pd, evt->child);
   evas_object_smart_changed(pd->obj);
}

static void
_child_removed_cb(void *data, const Efl_Event *event)
{
   Efl_Model_Children_Event* evt = event->info;
   Efl_Ui_List *obj = data;
   Efl_Ui_List_Item *item;

   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);
   /* pd->item_count--; */

   /* EINA_ARRAY_ITER_NEXT(pd->items.array, i, item, iterator) */
   /*   { */
   /*      if (item->model == evt->child) */
   /*        { */
   /*           _child_remove(pd, item); */
   /*           //FIXME pd->items = eina_list_remove_list(pd->items, li); */
   /*           evas_object_smart_changed(pd->obj); */
   /*           break; */
   /*        } */
   /*   } */
}

static void
_focused_element(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Ui_List_Data *pd = data;
   Eo *focused = event->info;
   EINA_SAFETY_ON_NULL_RETURN(pd);
   printf("item focused\n");

   if (!_elm_config->item_select_on_focus_disable)
     _efl_ui_list_item_select_set(pd, focused, EINA_TRUE);
}

static Eina_Bool
_long_press_cb(void *data)
{
   Efl_Ui_List_Item *item = data;

   item->long_timer = NULL;
   item->longpressed = EINA_TRUE;
   /* if (item->layout) */
   /*   efl_event_callback_legacy_call(item->layout, EFL_UI_EVENT_LONGPRESSED, NULL); */

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
   /* item->long_timer = ecore_timer_add(_elm_config->longpress_timeout, _long_press_cb, item); */
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

   _efl_ui_list_item_select_set(pd, item->layout, EINA_TRUE);
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
        /* if (item->selected) */
        /*   pd->selected = eina_list_append(pd->selected, item); */
        /* else */
        /*   pd->selected = eina_list_remove(pd->selected, item); */
     }
}

static void
_count_then(void * data, Efl_Event const* event)
{
   Efl_Ui_List_Data *pd = data;
   EINA_SAFETY_ON_NULL_RETURN(pd);
   int *count = ((Efl_Future_Event_Success*)event->info)->value;

   pd->item_count = *count;
   _update_items(pd->obj, pd/*, EINA_TRUE*/);
}

static void
_count_error(void * data, Efl_Event const* event EINA_UNUSED)
{
   Efl_Ui_List_Data *pd = data;
   EINA_SAFETY_ON_NULL_RETURN(pd);
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

   /* sprop = eina_stringshare_add(SELECTED_PROP); */
   /* EINA_ARRAY_ITER_NEXT(evt->changed_properties, i, prop, it) */
   /*   { */
   /*      if (prop == sprop) */
   /*        { */
   /*           item->future = efl_model_property_get(item->model, sprop); */
   /*           efl_future_then(item->future, &_item_selected_then, &_item_property_error, NULL, item); */
   /*        } */
   /*   } */
}

static void
_child_remove(Efl_Ui_List_Data *pd, Efl_Ui_List_Item *item)
{
   EINA_SAFETY_ON_NULL_RETURN(item);
   EINA_SAFETY_ON_NULL_RETURN(item->model);

   /* _layout_unrealize(pd, item); */

   /* elm_widget_sub_object_del(pd->obj, item->layout); */
   /* free(item); */
}

static void
_item_min_calc(Efl_Ui_List_Data *pd, Efl_Ui_List_Item *item, Evas_Coord h, Evas_Coord w)
{
   Efl_Ui_List_Item *litem;
   Evas_Coord min;

   if(_horiz(pd->orient))
     {
        pd->realized.w -= item->minw;
        pd->realized.w += w;
        if(pd->realized.h <= h)
          pd->realized.h = h;
        else if (pd->realized.h == item->minh)
          {
             pd->realized.h = h;
             EINA_INARRAY_FOREACH(&pd->items.array, litem)
               {
                  if (pd->realized.h < litem->minh)
                    pd->realized.h = litem->minh;

                  if (litem != item && litem->minh == item->minh)
                    break;
               }
          }
     }
   else
     {
        pd->realized.h -= item->minh;
        pd->realized.h += h;
        if(pd->realized.w <= w)
          pd->realized.w = w;
        else if (pd->realized.w == item->minw)
          {
             pd->realized.w = w;
             EINA_INARRAY_FOREACH(&pd->items.array, litem)
               {
                  if (pd->realized.w < litem->minw)
                    pd->realized.w = litem->minw;

                  if (litem != item && litem->minw == item->minw)
                    break;
               }
          }
     }

   item->minw = w;
   item->minh = h;
}

static int
_find_item_by_layout(const void *data1, const void *data2)
{
   Efl_Ui_List_Item *item = data2;
   Eo *layout = data1;

   if (item->layout == layout)
     return 0;

   return -1;
}

static void
_on_item_size_hint_change(void *data, Evas *e EINA_UNUSED,
                    Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Efl_Ui_List_Item *item, *members;
   Efl_Ui_List_Data *pd = data;
   EINA_SAFETY_ON_NULL_RETURN(pd);

   int idx = eina_inarray_search(&pd->items.array, obj, _find_item_by_layout);
   if (idx < 0)
     return;

   members = pd->items.array.members;
   item = &members[idx];

   _item_calc(pd, item);
   evas_object_smart_changed(pd->obj);
}

static void
_layout_realize(Efl_Ui_List_Data *pd, Efl_Ui_List_Item *item)
{
   Efl_Ui_List_Item_Event evt;

   efl_ui_view_model_set(item->layout, item->model); //XXX: move to realize??

   evt.child = item->model;
   evt.layout = item->layout;
   evt.index = item->index;

   /* efl_event_callback_add(item->model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_changed_cb, item); //XXX move to elm_layout obj?? */
   efl_event_callback_call(item->obj, EFL_UI_LIST_EVENT_ITEM_REALIZED, &evt);

   evas_object_show(item->layout);

   /* evas_object_event_callback_add(item->layout, EVAS_CALLBACK_MOUSE_DOWN, _on_item_mouse_down, item); */
   /* evas_object_event_callback_add(item->layout, EVAS_CALLBACK_MOUSE_UP, _on_item_mouse_up, item); */

   if (pd->select_mode != ELM_OBJECT_SELECT_MODE_NONE)
     efl_ui_model_connect(item->layout, "signal/elm,state,%v", "selected");

    _item_calc(pd, item);
   evas_object_event_callback_add(item->layout, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_item_size_hint_change, pd);
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

//  elm_obj_widget_focus_set(item->layout, EINA_FALSE);

   if (item->selected)
     {
        item->selected = EINA_FALSE;
/*        pd->selected = eina_list_remove(pd->selected, item); */
     }

   /* efl_event_callback_del(item->model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_changed_cb, item); */
   /* evas_object_event_callback_del_full(item->layout, EVAS_CALLBACK_MOUSE_DOWN, _on_item_mouse_down, item); */
   /* evas_object_event_callback_del_full(item->layout, EVAS_CALLBACK_MOUSE_UP, _on_item_mouse_up, item); */

   efl_ui_model_connect(item->layout, "signal/elm,state,%v", NULL);

   /* TODO:calculate new min */
   _item_min_calc(pd, item, 0, 0);

   evt.child = item->model;
   evt.layout = item->layout;
   evt.index = item->index;
   efl_event_callback_call(item->obj, EFL_UI_LIST_EVENT_ITEM_UNREALIZED, &evt);
   efl_ui_view_model_set(item->layout, NULL);

   efl_unref(item->model);

   evas_object_hide(item->layout);
   evas_object_move(item->layout, -9999, -9999);
}

static void
_child_setup(Efl_Ui_List_Data *pd, Efl_Ui_List_Item* item, Efl_Model *model
             , Eina_Inarray* recycle_layouts, int idx)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);

   assert(model != NULL);
   assert(item->layout == NULL);

   item->obj = pd->obj;
   item->model = efl_ref(model);
//   if(eina_inarray_count(recycle_layouts))
//     item->layout = *(void**)eina_inarray_pop(recycle_layouts);
//   else
//     item->layout = efl_add(ELM_LAYOUT_CLASS, pd->obj);
   item->layout = efl_ui_factory_create(pd->factory, item->model, pd->obj);
   item->future = NULL;
   item->index = idx + pd->realized.start;
   item->minw = item->minh = 0;

   elm_widget_sub_object_add(pd->obj, item->layout);
   evas_object_smart_member_add(item->layout, pd->pan.obj);

//   FIXME: really need get it in model?
   Eina_Stringshare *style_prop = eina_stringshare_add("style");
   if (_efl_model_properties_has(item->model, style_prop))
     {
        item->future = efl_model_property_get(item->model, style_prop);
        /* efl_future_then(item->future, &_item_property_then, &_item_property_error, NULL, item); */
     }
   eina_stringshare_del(style_prop);
//
   _layout_realize(pd, item);
}

static void
_child_release(Efl_Ui_List_Data* pd, Efl_Ui_List_Item* item, Eina_Inarray* recycle_layouts)
{
  fprintf(stderr, "%s %s:%d layout %p\n", __func__, __FILE__, __LINE__, item->layout); fflush(stderr);
  _layout_unrealize(pd, item);
  if(item->future)
    efl_future_cancel(item->future);
  efl_unref(item->model);
  // discard elm_layout to thrash to be able to reuse it
//  assert(item->layout != NULL);
//  eina_inarray_push(recycle_layouts, &item->layout);
  efl_ui_factory_release(pd->factory, item->layout);
  item->layout = NULL;
  if(_horiz(pd->orient))
    pd->realized.w -= item->w;
  else
    pd->realized.h -= item->h;
  fprintf(stderr, "%s %s:%d layout %p\n", __func__, __FILE__, __LINE__, item->layout); fflush(stderr);
}

static void
_children_then(void * data, Efl_Event const* event)
{
   Efl_Ui_List_Data *pd = data;
   Eina_Accessor *acc = (Eina_Accessor*)((Efl_Future_Event_Success*)event->info)->value;
   /* Eo *child_model; */
   Efl_Ui_List_Item *item;
   unsigned idx/*, count, diff*/;
   /* Eina_Bool horz = _horiz(pd->orient); */
   int removing_before = -pd->realized.start + pd->outstanding_slice.slice_start;
   int removing_after = pd->realized.start + pd->realized.slice
     - (pd->outstanding_slice.slice_start + pd->outstanding_slice.slice);
   unsigned to_begin, from_begin, copy_size;
   Eina_Inarray recycle_layouts;

   // If current slice doesn't reach new slice
   if(pd->realized.start + pd->realized.slice < pd->outstanding_slice.slice_start
      || pd->outstanding_slice.slice_start + pd->outstanding_slice.slice < pd->realized.start)
     {
       removing_before = pd->realized.slice;
       removing_after = -pd->outstanding_slice.slice;
     }
   
   eina_inarray_setup(&recycle_layouts, sizeof(Elm_Layout*), 0);

   fprintf(stderr, "_children_then removing_before %d removing_after %d\n", removing_before, removing_after); fflush(stderr);
   
   EINA_SAFETY_ON_NULL_RETURN(pd);

   pd->future = NULL;
   EINA_SAFETY_ON_NULL_RETURN(acc);

   ELM_WIDGET_DATA_GET_OR_RETURN(pd->obj, wd);

   assert(pd->realized.slice == eina_inarray_count(&pd->items.array));
   
   // received slice start is after older slice start
   if(removing_before > 0)
     {
       int i = 0;
       while(i != removing_before)
         {
           Efl_Ui_List_Item* members = pd->items.array.members;
           item = &members[i];
           fprintf(stderr, "%s %s:%d uninitializing item (idx %d) %d %p\n", __func__, __FILE__, __LINE__, i, i + pd->realized.start, item); fflush(stderr);
           _child_release(pd, item, &recycle_layouts);
           fprintf(stderr, "%s %s:%d uninitialized item (idx %d) %d %p\n", __func__, __FILE__, __LINE__, i, i + pd->realized.start, item); fflush(stderr);
           ++i;
         }
       to_begin = 0;
       from_begin = removing_before;
     }
   else
     {
       to_begin = -removing_before;
       from_begin = 0;
     }

   if(removing_after > 0)
     {
       int i = pd->realized.slice - removing_after;
       while(i != pd->realized.slice)
         {
           Efl_Ui_List_Item* members = pd->items.array.members;
           item = &members[i];
           fprintf(stderr, "%s %s:%d uninitializing item (idx %d) %d %p\n", __func__, __FILE__, __LINE__, i, i + pd->realized.start, item); fflush(stderr);
           _child_release(pd, item, &recycle_layouts);
           fprintf(stderr, "%s %s:%d uninitialized item (idx %d) %d %p\n", __func__, __FILE__, __LINE__, i, i + pd->realized.start, item); fflush(stderr);
           ++i;
         }
       copy_size = eina_inarray_count(&pd->items.array)
         - (from_begin + removing_after);
     }
   else
     {
       copy_size = eina_inarray_count(&pd->items.array) - from_begin;
     }

   fprintf(stderr, "%s %s:%d from_begin %d to_begin: %d copy_size %d\n", __func__, __FILE__, __LINE__, from_begin, to_begin, copy_size); fflush(stderr);
     
   if(removing_after + removing_before >= 0)
     {
       if(from_begin != to_begin)
         {
           fprintf(stderr, "%s %s:%d moving from: %d to %d with size %d\n", __func__, __FILE__, __LINE__, from_begin, to_begin, copy_size); fflush(stderr);
            memmove(&((Efl_Ui_List_Item*)pd->items.array.members)[to_begin],
                    &((Efl_Ui_List_Item*)pd->items.array.members)[from_begin],
                    copy_size*sizeof(Efl_Ui_List_Item));
         }
       fprintf(stderr, "%s %s:%d old len: %d\n", __func__, __FILE__, __LINE__, pd->items.array.len); fflush(stderr);
       fprintf(stderr, "%s %s:%d zeoring %p %d size %d\n", __func__, __FILE__, __LINE__
               , &((Efl_Ui_List_Item*)pd->items.array.members)[copy_size+to_begin]
               , copy_size+to_begin, eina_inarray_count(&pd->items.array)-(copy_size+to_begin)); fflush(stderr);
       memset(&((Efl_Ui_List_Item*)pd->items.array.members)[copy_size+to_begin], 0, (eina_inarray_count(&pd->items.array) - (copy_size+to_begin))*sizeof(Efl_Ui_List_Item));
       fprintf(stderr, "%s %s:%d zeoring %p %d size %d\n", __func__, __FILE__, __LINE__
               , &((Efl_Ui_List_Item*)pd->items.array.members)[0]
               , 0, to_begin); fflush(stderr);
       memset(&((Efl_Ui_List_Item*)pd->items.array.members)[0], 0, to_begin*sizeof(Efl_Ui_List_Item));
       pd->items.array.len -= removing_before + removing_after;
       fprintf(stderr, "%s %s:%d new len: %d\n", __func__, __FILE__, __LINE__, pd->items.array.len); fflush(stderr);
     }
   else
     {
       Efl_Ui_List_Item* data = calloc(1, pd->outstanding_slice.slice*sizeof(Efl_Ui_List_Item));
       fprintf(stderr, "new members allocated: %p\n", data);
       memcpy(&data[to_begin],
              &((Efl_Ui_List_Item*)pd->items.array.members)[from_begin], 
              copy_size*sizeof(Efl_Ui_List_Item));
       free(pd->items.array.members);
       pd->items.array.members = data;
       pd->items.array.len = pd->items.array.max = pd->outstanding_slice.slice;
     }

   pd->realized.start = pd->outstanding_slice.slice_start;
   pd->realized.slice = pd->outstanding_slice.slice;

   idx = 0;
   while(removing_before < 0)
     {
       Efl_Ui_List_Item* members = pd->items.array.members;
       Efl_Ui_List_Item* item = &members[idx];
       fprintf(stderr, "%s %s:%d initializing item %d %p layout %p\n", __func__, __FILE__, __LINE__, idx + pd->realized.start, item, item->layout); fflush(stderr);

       // initialize item
       void* model = NULL;
       int r = eina_accessor_data_get(acc, idx + pd->realized.start - pd->outstanding_slice.slice_start, &model);
       assert(r != EINA_FALSE);
       assert(model != NULL);

       _child_setup(pd, item, model, &recycle_layouts, idx);

       fprintf(stderr, "%s %s:%d initializing item %d %p layout %p\n", __func__, __FILE__, __LINE__, idx + pd->realized.start, item, item->layout); fflush(stderr);
       
       idx++;
       removing_before++;
     }

   idx = copy_size + to_begin;
   while(removing_after < 0)
     {
       Efl_Ui_List_Item* members = pd->items.array.members;
       Efl_Ui_List_Item* item = &members[idx];

       fprintf(stderr, "%s %s:%d initializing item (idx %d) %d %p layout %p\n", __func__, __FILE__, __LINE__, idx, idx + pd->realized.start, item, item->layout); fflush(stderr);       

       // initialize item
       void* model = NULL;
       int r = eina_accessor_data_get(acc, idx + pd->realized.start - pd->outstanding_slice.slice_start, &model);
       assert(r != EINA_FALSE);
       assert(model != NULL);

       _child_setup(pd, item, model, &recycle_layouts, idx);

       fprintf(stderr, "%s %s:%d initializing item (idx %d) %d %p layout %p\n", __func__, __FILE__, __LINE__, idx, idx + pd->realized.start, item, item->layout); fflush(stderr);
       
       idx++;
       removing_after++;
     }

   _efl_ui_list_custom_layout(pd->obj);
   
   /* count = eina_array_count(pd->items.array); */

   /* if (sd->slicestart < pd->realized.start) */
   /*   { */
   /*      diff = pd->realized.start - sd->newstart; */
   /*      diff = diff > count ? count : diff; */
   /*      idx = count - diff; */
   /*      if (diff) */
   /*        { */
   /*           Efl_Ui_List_Item **cacheit; */
   /*           size_t s = sizeof(Efl_Ui_List_Item *); */

   /*           cacheit = calloc(diff, s); */
   /*           memcpy(cacheit, array->data+idx, diff * s); */

   /*           for (i = idx; i < count; ++i) */
   /*             { */
   /*                item = eina_array_data_get(array, i); */
   /*                _layout_unrealize(pd, item); */
   /*             } */
   /*           memmove(array->data+diff, array->data, idx * s); */
   /*           memcpy(array->data, cacheit, diff * s); */
   /*           free(cacheit); */
   /*        } */
   /*      idx = 0; */
   /*   } */
   /* else */
   /*   { */
   /*      diff = sd->newstart - pd->realized.start; */
   /*      diff = diff > count ? count : diff; */
   /*      idx = count - diff; */
   /*      if (diff) */
   /*        { */
   /*           Efl_Ui_List_Item **cacheit; */
   /*           size_t s = sizeof(Efl_Ui_List_Item *); */

   /*           cacheit = calloc(diff, s); */
   /*           memcpy(cacheit, array->data, diff * s); */

   /*           for (i = 0; i < diff; ++i) */
   /*             { */
   /*                item = eina_array_data_get(array, i); */
   /*                _layout_unrealize(pd, item); */
   /*             } */
   /*           memmove(array->data, array->data+diff, idx * s); */
   /*           memcpy(array->data+idx, cacheit, diff * s); */
   /*           free(cacheit); */
   /*        } */
   /*   } */

   /* pd->realized.start = sd->newstart; */
   /*   EINA_ACCESSOR_FOREACH(acc, i, child_model) */
   /*     { */
   /*        if (idx < eina_array_count(array)) */
   /*          { */
   /*             item = eina_array_data_get(array, idx); */
   /*             item->model = efl_ref(child_model); */
   /*             item->index = sd->newstart + idx - 1; */

   /*             if (horz) */
   /*               pd->realized.w -= item->minw; */
   /*             else */
   /*               pd->realized.h -= item->minh; */

   /*             _layout_realize(pd, item); */
   /*          } */
   /*        else */
   /*          { */
   /*             item = _child_new(pd, child_model); */
   /*             eina_array_push(pd->items, item); */
   /*          } */
   /*        if (horz) */
   /*          { */
   /*             pd->realized.w += item->minw; */
   /*             if (item->minh > pd->realized.h) */
   /*               pd->realized.h = item->minh; */
   /*          } */
   /*        else */
   /*          { */
   /*             pd->realized.h += item->minh; */
   /*             if (item->minw > pd->realized.w) */
   /*               pd->realized.w = item->minw; */
   /*          } */
   /*        ++idx; */
   /*     } */

   /* pd->avsom = horz ? pd->realized.w : pd->realized.h; */
   /* free(sd); */
   /* pd->average_item_size = pd->avsom / eina_array_count(pd->items); */
   /* evas_object_smart_changed(pd->obj); */
}

static void
_efl_ui_list_children_free(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   Eina_Array_Iterator iterator;
   Efl_Ui_List_Item *item;
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN(pd);

   /* if(!pd->items.array) return; */

   /* EINA_ARRAY_ITER_NEXT(pd->items.array, i, item, iterator) */
   /*   { */
   /*      _layout_unrealize(pd, item); */
   /*      free(item); */
   /*      item = NULL; */
   /*   } */

   /* eina_array_clean(pd->items.array); */
}

static void
_children_error(void * data EINA_UNUSED, Efl_Event const* event EINA_UNUSED)
{
   Efl_Ui_List_Slice *sd = data;
   Efl_Ui_List_Data *pd = sd->pd;
   pd->future = NULL;
   /* free(data); */
}

static void
_show_region_hook(void *data EINA_UNUSED, Evas_Object *obj)
{
   Evas_Coord x, y, w, h;
   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);
   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   x += pd->pan.x;
   y += pd->pan.y;
   elm_interface_scrollable_content_region_show(obj, x, y, w, h);
}

EOLIAN static void
_efl_ui_list_select_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, Elm_Object_Select_Mode mode)
{
   Eina_Array_Iterator iterator;
   Efl_Ui_List_Item *item;
   unsigned int i;

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

EOLIAN static Eina_Bool
_efl_ui_list_elm_widget_on_focus(Eo *obj, Efl_Ui_List_Data *pd EINA_UNUSED, Elm_Object_Item *item)
{
   //TODO: with no focused focus first visible element

   if (!efl_ui_focus_manager_focused(pd->manager))
   {
     printf(">>>> no item focused\n");
     efl_ui_focus_manager_focus(pd->manager, obj);
   }

   return elm_obj_widget_on_focus(efl_super(obj, MY_CLASS), item);
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
   evas_object_move(pd->pan.obj, x - pd->pan.x, y - pd->pan.y);
   _efl_ui_list_custom_layout(obj);
}

EOLIAN static void
_efl_ui_list_elm_interface_scrollable_region_bring_in(Eo *obj, Efl_Ui_List_Data *pd, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
    elm_interface_scrollable_region_bring_in(efl_super(obj, MY_CLASS), x + pd->pan.x, y + pd->pan.y, w, h);
}

EOLIAN static void
_efl_ui_list_efl_gfx_size_set(Eo *obj, Efl_Ui_List_Data *pd, Evas_Coord w, Evas_Coord h)
{
   Evas_Coord oldw, oldh;
   Eina_Bool load = EINA_FALSE;
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, w, h))
     return;

   evas_object_geometry_get(obj, NULL, NULL, &oldw, &oldh);

   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);
   evas_object_resize(pd->hit_rect, w, h);


   if (_horiz(pd->orient))
     {
        if (w != oldw) load = EINA_TRUE;
     }
   else
     {
        if (h != oldh) load = EINA_TRUE;
     }

   fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);

   if (load && _update_items(obj, pd/*, EINA_TRUE*/))
     return;

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

   elm_widget_can_focus_set(obj, EINA_TRUE);

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
   //pd->average_item_size = AVERAGE_SIZE_INIT;

   elm_interface_atspi_accessible_type_set(obj, ELM_ATSPI_TYPE_DISABLED);
   pd->pan.obj = efl_add(MY_PAN_CLASS, evas_object_evas_get(obj));
   pan_data = efl_data_scope_get(pd->pan.obj, MY_PAN_CLASS);
   efl_data_ref(obj, MY_CLASS);
   pan_data->wobj = obj;
   pan_data->wpd = pd;
   pd->pan.x = 0;
   pd->pan.y = 0;

   elm_interface_scrollable_extern_pan_set(obj, pd->pan.obj);
   evas_object_show(pd->pan.obj);

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_list_efl_canvas_group_group_del(Eo *obj, Efl_Ui_List_Data *pd)
{
   /* _efl_ui_list_children_free(obj, pd); */

   /* eina_array_free(pd->items.array); */

   /* ELM_SAFE_FREE(pd->pan.obj, evas_object_del); */
   /* efl_canvas_group_del(efl_super(obj, MY_CLASS)); */
}

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_list_elm_widget_focus_manager_factory(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *root)
{

   if (!pd->manager)
     pd->manager = efl_add(EFL_UI_FOCUS_MANAGER_CLASS, obj,
     efl_ui_focus_manager_root_set(efl_added, root)
   );

   return pd->manager;
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

   eina_inarray_setup(&pd->items.array, sizeof(Efl_Ui_List_Item), 0);

   manager = elm_obj_widget_focus_manager_factory(obj, obj);
   efl_composite_attach(obj, manager);
   _efl_ui_focus_manager_redirect_events_add(manager, obj);
   efl_event_callback_add(obj, EFL_UI_FOCUS_MANAGER_EVENT_FOCUSED, _focused_element, pd);

   pd->style = eina_stringshare_add(elm_widget_style_get(obj));

   pd->factory = NULL;
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

   efl_event_callback_del(obj, EFL_UI_FOCUS_MANAGER_EVENT_FOCUSED, _focused_element, pd);
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

   if (pd->future) efl_future_cancel(pd->future);

   if (pd->model)
     {
        efl_event_callback_del(pd->model, EFL_MODEL_EVENT_CHILD_ADDED, _child_added_cb, obj);
        efl_event_callback_del(pd->model, EFL_MODEL_EVENT_CHILD_REMOVED, _child_removed_cb, obj);
        efl_unref(pd->model);
        pd->model = NULL;
     }

   _efl_ui_list_children_free(obj, pd);


   if (!pd->factory)
     pd->factory = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, obj);

   if (model)
     {
        pd->model = model;
        efl_ref(pd->model);
        efl_event_callback_add(pd->model, EFL_MODEL_EVENT_CHILD_ADDED, _child_added_cb, obj);
        efl_event_callback_add(pd->model, EFL_MODEL_EVENT_CHILD_REMOVED, _child_removed_cb, obj);
        efl_future_then(efl_model_children_count_get(pd->model), &_count_then, &_count_error, NULL, pd);
     }
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
   Efl_Ui_List_Item *litem;
   Eina_Array_Iterator iterator;
   unsigned int i;
   Eina_List *ret = NULL, *ret2 = NULL;

   /* EINA_ARRAY_ITER_NEXT(pd->items.array, i, litem, iterator) */
   /*    ret = eina_list_append(ret, litem->layout); */

   /* ret2 = elm_interface_atspi_accessible_children_get(efl_super(obj, MY_CLASS)); */

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
        /* Efl_Ui_List_Item *item = eina_array_data_get(pd->items.array, child_index); */
        /* if (item) */
        /*    _efl_ui_list_item_select_set(item, EINA_TRUE); */
        /* return EINA_TRUE; */
     }
   return EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_ui_list_elm_interface_atspi_selection_selected_child_deselect(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, int child_index)
{
   Efl_Ui_List_Item *item = eina_list_nth(pd->selected, child_index);
   if (item)
     {
        _efl_ui_list_item_select_set(pd, item->layout, EINA_FALSE);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_ui_list_elm_interface_atspi_selection_is_child_selected(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, int child_index)
{
   /* Efl_Ui_List_Item *item = eina_array_data_get(pd->items.array, child_index); */
   /* EINA_SAFETY_ON_NULL_RETURN_VAL(item, EINA_FALSE); */
   /* return item->selected; */
  return EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_ui_list_elm_interface_atspi_selection_all_children_select(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   Efl_Ui_List_Item *item;
   Eina_Array_Iterator iterator;
   unsigned int i;

   if (pd->select_mode == ELM_OBJECT_SELECT_MODE_NONE)
     return EINA_FALSE;

   /* EINA_ARRAY_ITER_NEXT(pd->items.array, i, item, iterator) */
   /*    _efl_ui_list_item_select_set(item, EINA_TRUE); */

   return EINA_TRUE;
}

EOLIAN Eina_Bool
_efl_ui_list_elm_interface_atspi_selection_clear(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   Efl_Ui_List_Item *item;
   Eina_List *l;

   if (pd->select_mode == ELM_OBJECT_SELECT_MODE_NONE)
     return EINA_FALSE;

   /* EINA_LIST_FOREACH(pd->selected, l, item) */
   /*    _efl_ui_list_item_select_set(item, EINA_FALSE); */

   return EINA_TRUE;
}

EOLIAN Eina_Bool
_efl_ui_list_elm_interface_atspi_selection_child_deselect(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, int child_index)
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

   Eo *focused = efl_ui_focus_manager_focused(pd->manager);
   if (focused)
     _efl_ui_list_item_select_set(pd, focused, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_key_action_escape(Evas_Object *obj, const char *params EINA_UNUSED)
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
   if (!eina_inarray_count(&pd->items.array)) return EINA_FALSE;

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

   /* EINA_LIST_FOREACH(pd->selected, li, item) */
   /*    _efl_ui_list_item_select_set(item, EINA_FALSE); */

   return EINA_TRUE;
}

void
_efl_ui_list_item_select_set(Efl_Ui_List_Data *pd, Eo* layout, Eina_Bool selected)
{
   Eina_Stringshare *sprop, *svalue;
   Eo *model;

   selected = !!selected;
   if (!layout || (pd->select_mode == ELM_OBJECT_SELECT_MODE_NONE) ||
       (pd->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY))
     return;

   model = efl_ui_view_model_get(layout);
   if (model) return;

   sprop = eina_stringshare_add(SELECTED_PROP);
   svalue = (selected ? eina_stringshare_add("selected") : eina_stringshare_add("unselected"));

   if (_efl_model_properties_has(model, sprop))
     {
        Eina_Value v;
        eina_value_setup(&v, EINA_VALUE_TYPE_STRINGSHARE);
        eina_value_set(&v, svalue);
        efl_model_property_set(model, sprop, &v);
        eina_value_flush(&v);
     }
   eina_stringshare_del(sprop);
   eina_stringshare_del(svalue);

   /* //TODO I need call this event or catch only by model connect event? */
   /* if (selected) */
   /*   efl_event_callback_legacy_call(item->layout, EFL_UI_EVENT_SELECTED, NULL); */
   /* else */
   /*   efl_event_callback_legacy_call(item->layout, EFL_UI_EVENT_UNSELECTED, NULL); */
}

static void
_item_calc(Efl_Ui_List_Data *pd, Efl_Ui_List_Item *item)
{
   Evas_Coord minw, minh;
   int pad[4];

   efl_gfx_size_hint_combined_min_get(item->layout, &minw, &minh);
   efl_gfx_size_hint_margin_get(item->layout, &pad[0], &pad[1], &pad[2], &pad[3]);
   efl_gfx_size_hint_weight_get(item->layout, &item->wx, &item->wy);

   if (item->wx < 0) item->wx = 0;
   if (item->wy < 0) item->wy = 0;

   minw += pad[0] + pad[1];
   minh += pad[2] + pad[3];

   pd->weight.x += item->wx;
   pd->weight.y += item->wy;

   _item_min_calc(pd, item, minh, minw);
   evas_object_size_hint_min_set(item->layout, minw, minh);
}

static Eina_Bool
_update_items(Eo *obj, Efl_Ui_List_Data *pd/*, Eina_Bool recalc*/)
{
   int want_slice, want_slice_start = 0;
   Evas_Coord w = 0, h = 0;
   Efl_Ui_List_Item *item;
   Eina_Bool horz = _horiz(pd->orient);

   printf("_update_items\n");

   assert(!pd->future);
   /* if (pd->future) */
   /*    efl_future_cancel(pd->future); */

   /* if (pd->items) */
   /*   count = eina_array_count(pd->items); */

   /* if ((!recalc && count == pd->item_count) || pd->average_item_size < 1) */
   /*   return EINA_FALSE; */

   int average_item_size = eina_inarray_count(&pd->items.array) ? (horz ? pd->realized.w : pd->realized.h) / eina_inarray_count(&pd->items.array) : AVERAGE_SIZE_INIT;
   if(!average_item_size)
     average_item_size = AVERAGE_SIZE_INIT;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (horz)
     {
        want_slice  = (w / /*pd->*/average_item_size) * 2;
        want_slice_start = (pd->pan.x / average_item_size) - (want_slice / 4);
        if(want_slice_start < 0)
          want_slice_start = 0;
     }
   else
     {
        want_slice =  (h / average_item_size) * 2;
        want_slice_start = (pd->pan.y / average_item_size) - (want_slice / 4);
        if(want_slice_start < 0)
          want_slice_start = 0;
     }

   /* if (!recalc && newstart == pd->realized.start && slice == pd->realized.slice) */
   /*   return EINA_FALSE; */

   /* if (count) */
   /*   { */
   /*      if (newstart < pd->realized.start) */
   /*        { */
   /*           if(pd->realized.start - newstart < slice) */
   /*               slice = pd->realized.start - newstart; */
   /*        } */
   /*      else if (newstart < pd->realized.start + count) */
   /*        { */
   /*           slicestart = pd->realized.start + count; */
   /*           slice -= slicestart - newstart; */
   /*        } */
   /*   } */

   /* if (slicestart + want_slice > pd->item_count) */
   /*   { */
   /*     int aux = (slicestart + slice - 1) - pd->item_count; */
   /*     slice -= aux; */
   /*     newstart = newstart - aux > 1 ? newstart - aux : 1; */
   /*   } */

   /* if (slice > 0) */
   /*   { */
   /*      sd = malloc(sizeof(Efl_Ui_List_Slice)); */
   /*      sd->pd = pd; */
   /*      sd->slicestart = slicestart; */
   /*      sd->newstart = newstart; */
   /*      sd->newslice = slice; */

   /*      pd->future = efl_model_children_slice_get(pd->model, slicestart, slice); */
   /*      efl_future_then(pd->future, &_children_then, &_children_error, NULL, sd); */
   /*   } */

   assert(want_slice_start >= 0);
   if(want_slice < 8)
     want_slice = 8;
   if(want_slice_start + want_slice > pd->item_count)
     {
       if(want_slice > pd->item_count)
         want_slice = pd->item_count;
       want_slice_start = pd->item_count - want_slice;
       assert(want_slice_start >= 0);
     }

   if(want_slice != 0)
     {
       pd->outstanding_slice.slice_start = want_slice_start;
       pd->outstanding_slice.slice = want_slice;
       fprintf(stderr, "slice_start: %d slice_count: %d\n", want_slice_start, want_slice); fflush(stderr);
       pd->future = efl_model_children_slice_get(pd->model, want_slice_start, want_slice);
       efl_future_then(pd->future, &_children_then, &_children_error, NULL, pd);
     }

   /* else */
   /*   { */
   /*     while (pd->realized.slice < (int)eina_array_count(pd->items)) */
   /*       { */
   /*          item = eina_array_pop(pd->items); */
   /*          if (!item) break; */
   /*          if (horz) */
   /*            pd->realized.w -= item->minw; */
   /*          else */
   /*            pd->realized.h -= item->minh; */
   /*          _child_remove(pd, item); */
   /*       } */
   /*     pd->avsom = horz ? pd->realized.w : pd->realized.h; */
   /*     if (eina_array_count(pd->items)) */
   /*       pd->average_item_size = pd->avsom / eina_array_count(pd->items); */

   /*     return EINA_FALSE; */
   /*   } */

   /* pd->realized.slice = slice; */
   /* if (!pd->items) */
   /*   pd->items = eina_array_new(slice); */
   
     return EINA_TRUE;
}

void
_efl_ui_list_custom_layout(Efl_Ui_List *ui_list)
{
   EFL_UI_LIST_DATA_GET_OR_RETURN(ui_list, pd);
   Efl_Ui_List_Item *litem = NULL;
   //   Evas_Object *o;
   Eina_Bool horiz = _horiz(pd->orient), zeroweight = EINA_FALSE;
   Evas_Coord ow, oh, want;
   int boxx, boxy, boxw, boxh, length, pad, extra = 0, rounding = 0;
   int boxl = 0, boxr = 0, boxt = 0, boxb = 0;
   double cur_pos = 0, scale, box_align[2],  weight[2] = { 0, 0 };
   Eina_Bool box_fill[2] = { EINA_FALSE, EINA_FALSE };
   unsigned int i;
   int id, count = 0;
   Eina_List *order = NULL;

   ELM_WIDGET_DATA_GET_OR_RETURN(ui_list, wd);

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

   count = eina_inarray_count(&pd->items.array);

   /* if (!count) */
   /*   { */
   /*      evas_object_size_hint_min_set(wd->resize_obj, 0, 0); */
   /*      return; */
   /*   } */

   elm_interface_scrollable_content_viewport_geometry_get
              (ui_list, NULL, NULL, &ow, &oh);
   // box outer margin
   boxw -= boxl + boxr;
   boxh -= boxt + boxb;
   boxx += boxl;
   boxy += boxt;

   int average_item_size = eina_inarray_count(&pd->items.array) ? (/*horz*/ EINA_FALSE ? pd->realized.w : pd->realized.h) / eina_inarray_count(&pd->items.array) : AVERAGE_SIZE_INIT;
   if(!average_item_size)
     average_item_size = AVERAGE_SIZE_INIT;

   // total space & available space
   if (horiz)
     {
        length = boxw;
        want = pd->realized.w;
        pad = pd->pad.scalable ? (pd->pad.h * scale) : pd->pad.h;

        // padding can not be squeezed (note: could make it an option)
        length -= pad * (count - 1);
        // available space. if <0 we overflow
        extra = length - want;

        pd->minw = pd->realized.w + boxl + boxr + pad * (count - 1);
        pd->minh = pd->realized.h + boxt + boxb;
        if (pd->item_count > count)
          pd->minw = pd->item_count * average_item_size;
     }
   else
     {
        length = boxh;
        want = pd->realized.h;
        pad = pd->pad.scalable ? (pd->pad.v * scale) : pd->pad.v;

        // padding can not be squeezed (note: could make it an option)
        length -= pad * (count - 1);
        // available space. if <0 we overflow
        extra = length - want;

        pd->minw = pd->realized.w + boxl + boxr;
        pd->minh = pd->realized.h + pad * (count - 1) + boxt + boxb;
        if (pd->item_count > count)
          pd->minh = pd->item_count * average_item_size;
     }

   evas_object_size_hint_min_set(wd->resize_obj, pd->minw, pd->minh);

   if (extra < 0) extra = 0;

   weight[0] = pd->weight.x;
   weight[1] = pd->weight.y;
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

   cur_pos += average_item_size * pd->realized.start;
   fprintf(stderr, "%s %s:%d array count %d average_item_size=%d\n", __func__, __FILE__, __LINE__, eina_inarray_count(&pd->items.array), average_item_size); fflush(stderr);

   // scan all items, get their properties, calculate total weight & min size
   EINA_INARRAY_FOREACH(&pd->items.array, litem)
     {
        double cx, cy, cw, ch, x, y, w, h;
        double align[2];
        int item_pad[4], max[2];

        //_item_calc(pd, litem);
        //fprintf(stderr, "%s %s:%d item %p layout %p\n", __func__, __FILE__, __LINE__, litem, litem->layout); fflush(stderr);
        assert(litem->layout != NULL);
        efl_gfx_size_hint_align_get(litem->layout, &align[0], &align[1]);
        efl_gfx_size_hint_max_get(litem->layout, &max[0], &max[1]);
        efl_gfx_size_hint_margin_get(litem->layout, &item_pad[0], &item_pad[1], &item_pad[2], &item_pad[3]);

        //fprintf(stderr, "%s %s:%d align[0] %f align[1] %f max[0] %d max[1] %d item_pad[0] %d item_pad[1] %d item_pad[2] %d item_pad[3] %d\n", __func__, __FILE__, __LINE__, (float)align[0], (float)align[1], max[0], max[1], item_pad[0], item_pad[1], item_pad[2], item_pad[3]); fflush(stderr);        

        if (align[0] < 0) align[0] = -1;
        if (align[1] < 0) align[1] = -1;
        if (align[0] > 1) align[0] = 1;
        if (align[1] > 1) align[1] = 1;

        if (max[0] <= 0) max[0] = INT_MAX;
        if (max[1] <= 0) max[1] = INT_MAX;
        if (max[0] < litem->minw) max[0] = litem->minw;
        if (max[1] < litem->minh) max[1] = litem->minh;

        /* // extra rounding up (compensate cumulative error) */
        /* if ((id == (count - 1)) && (cur_pos - floor(cur_pos) >= 0.5)) */
        /*   rounding = 1; */

        if (horiz)
          {
             cx = boxx + cur_pos;
             cy = boxy;
             cw = litem->minw + rounding + (zeroweight ? 1.0 : litem->wx) * extra / weight[0];
             ch = boxh;
             cur_pos += cw + pad;
          }
        else
          {
             cx = boxx;
             cy = boxy + cur_pos;
             cw = boxw;
             ch = litem->minh + rounding + (zeroweight ? 1.0 : litem->wy) * extra / weight[1];
             cur_pos += ch + pad;
          }

        // horizontally
        if (max[0] < INT_MAX)
          {
             w = MIN(MAX(litem->minw - item_pad[0] - item_pad[1], max[0]), cw);
             if (align[0] < 0)
               {
                  // bad case: fill+max are not good together
                  x = cx + ((cw - w) * box_align[0]) + item_pad[0];
               }
             else
               x = cx + ((cw - w) * align[0]) + item_pad[0];
          }
        else if (align[0] < 0)
          {
             // fill x
             w = cw - item_pad[0] - item_pad[1];
             x = cx + item_pad[0];
          }
        else
          {
             w = litem->minw - item_pad[0] - item_pad[1];
             x = cx + ((cw - w) * align[0]) + item_pad[0];
          }

        // vertically
        if (max[1] < INT_MAX)
          {
             h = MIN(MAX(litem->minh - item_pad[2] - item_pad[3], max[1]), ch);
             if (align[1] < 0)
               {
                  // bad case: fill+max are not good together
                  y = cy + ((ch - h) * box_align[1]) + item_pad[2];
               }
             else
               y = cy + ((ch - h) * align[1]) + item_pad[2];
          }
        else if (align[1] < 0)
          {
             // fill y
             h = ch - item_pad[2] - item_pad[3];
             y = cy + item_pad[2];
          }
        else
          {
             h = litem->minh - item_pad[2] - item_pad[3];
             y = cy + ((ch - h) * align[1]) + item_pad[2];
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

        evas_object_geometry_set(litem->layout, (x + 0 - pd->pan.x), (y + 0 - pd->pan.y), w, h);

        litem->x = x;
        litem->y = y;
        order = eina_list_append(order, litem->layout);

//        fprintf(stderr, "x: %.2f y: %.2f w: %.2f h: %.2f old x: %.2f old y: %.2f old w: %.2f old h: %.2f\n"
//                , (x + 0 - pd->pan.x), (y + 0 - pd->pan.y), (float)w, (float)h
//               , (float)litem->x, (float)litem->y, (float)litem->w, (float)litem->h); fflush(stderr);
//        printf("obj=%d currpos=%.2f moved to X=%.2f, Y=%.2f average_item_size %d\n", litem->index, cur_pos, x, y
//               , eina_inarray_count(&pd->items.array) ? (/*horz*/ EINA_FALSE ? pd->realized.w : pd->realized.h) / eina_inarray_count(&pd->items.array) : AVERAGE_SIZE_INIT);
     }

  efl_ui_focus_manager_update_order(pd->manager, pd->obj, order);
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_LIST_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_list)

#include "efl_ui_list.eo.c"

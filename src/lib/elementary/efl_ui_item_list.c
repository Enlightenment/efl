#include "efl_ui_item_priv.h"

#define MY_CLASS EFL_UI_ITEM_LIST_CLASS

typedef struct {
   Eo        *self;
   Eina_List *items; /* Efl_Ui_Item* */
   Efl_Model *model;

   Eina_List *pending_futures; // Efl_Future** (wref)

   Eina_Bool  model_owned : 1;
} Efl_Ui_Item_List_Data;

static void
_item_del_cb(void *data, const Efl_Event *event)
{
   Efl_Ui_Item_List_Data *pd = efl_data_scope_safe_get(data, MY_CLASS);
   if (!pd) return;

   pd->items = eina_list_remove(pd->items, event->object);
}

EOLIAN static Efl_Ui_Item *
_efl_ui_item_list_item_add(Eo *obj, Efl_Ui_Item_List_Data *pd,
                           const char *text, const char *icon)
{
   const Efl_Class *klass;
   Eo *item;

   klass = efl_ui_item_list_item_class_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(klass, NULL);

   item = efl_add(klass, obj,
                  efl_text_set(efl_added, text),
                  efl_ui_item_icon_set(efl_added, icon));
   pd->items = eina_list_append(pd->items, item);
   efl_event_callback_add(item, EFL_EVENT_DEL, _item_del_cb, obj);

   return item;
}

static void
_child_add_model(Eo *obj, Efl_Ui_Item_List_Data *pd, Efl_Model *model)
{
   const Efl_Class *klass;
   Eo *item;

   klass = efl_ui_item_list_item_class_get(obj);
   EINA_SAFETY_ON_NULL_RETURN(klass);

   item = efl_add(klass, obj, efl_ui_item_model_set(efl_added, model));
   pd->items = eina_list_append(pd->items, item);
   efl_event_callback_add(item, EFL_EVENT_DEL, _item_del_cb, obj);
}

static void
_pending_future_remove(Eo *obj EINA_UNUSED, Efl_Ui_Item_List_Data *pd, Efl_Future *future)
{
   Efl_Future **fut;
   Eina_List *li;

   EINA_LIST_FOREACH(pd->pending_futures, li, fut)
     if (*fut == future)
       {
          // WTF? where is the unuse function??
          pd->pending_futures = eina_list_remove_list(pd->pending_futures, li);
          efl_wref_del(*fut, fut);
          free(future);
          break;
       }
}

static void
_child_added_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Model_Children_Event *cevt = event->info;
   Efl_Ui_Item_List_Data *pd;
   Eo *obj = data;

   pd = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(pd);

   _pending_future_remove(obj, pd, event->object);
   _child_add_model(obj, pd, cevt->child);
}

static void
_child_removed_cb(void *data, const Efl_Event *event)
{
   Efl_Model_Children_Event *cevt = event->info;
   Efl_Ui_Item_List_Data *pd;
   Eina_List *li;
   Eo *obj = data;
   Eo *item, *model;

   pd = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(pd);

   INF("Child removed.");

   model = cevt->child;
   _pending_future_remove(obj, pd, event->object);

   EINA_LIST_FOREACH(pd->items, li, item)
     {
        if (model == efl_ui_item_model_get(item))
          {
             pd->items = eina_list_remove_list(pd->items, li);
             efl_event_callback_add(item, EFL_EVENT_DEL, _item_del_cb, obj);
             efl_del(item);
             break;
          }
     }
}

static void
_properties_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Item_List_Data *pd;
   Eo *obj = data;

   pd = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(pd);

   ERR("Model properties changed. Ignore.");
}

static void
_children_slice_cb(void *data, const Efl_Event *event)
{
   Efl_Future_Event_Success *success = event->info;
   Eina_Accessor *children = success->value;
   Efl_Ui_Item_List_Data *pd;
   Efl_Future **future;
   Efl_Model *child;
   Eina_List *li;
   Eo *obj = data;
   unsigned k;

   pd = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(pd);

   EINA_LIST_FOREACH(pd->pending_futures, li, future)
     if (*future == event->object)
       {
          // WTF? where is the unuse function??
          pd->pending_futures = eina_list_remove_list(pd->pending_futures, li);
          efl_wref_del(*future, future);
          free(future);
          break;
       }

   if (!children) return;
   EINA_ACCESSOR_FOREACH(children, k, child)
     _child_add_model(obj, pd, child);
}

EFL_CALLBACKS_ARRAY_DEFINE(_model_callbacks,
{ EFL_MODEL_EVENT_PROPERTIES_CHANGED, _properties_changed_cb },
{ EFL_MODEL_EVENT_CHILD_ADDED, _child_added_cb },
{ EFL_MODEL_EVENT_CHILD_REMOVED, _child_removed_cb })

EOLIAN static void
_efl_ui_item_list_model_set(Eo *obj, Efl_Ui_Item_List_Data *pd, Efl_Model *model)
{
   Efl_Callback_Array_Item *cb;
   Efl_Future **wfut, *future;
   size_t k;

   EINA_SAFETY_ON_NULL_RETURN(model);
   if (pd->model == model) return;

   if (pd->model)
     {
        Eo *item;

        EINA_LIST_FREE(pd->items, item)
          efl_del(item);

        efl_composite_detach(obj, pd->model);
        efl_event_callback_array_del(pd->model, _model_callbacks(), obj);
        for (k = 0, cb = _model_callbacks(); cb->desc;
             k++, cb = _model_callbacks() + k)
          efl_event_callback_forwarder_del(pd->model, cb->desc, obj);
        if (pd->model_owned)
          efl_del(pd->model);
     }

   EINA_LIST_FREE(pd->pending_futures, wfut)
     {
        efl_future_cancel(*wfut);
        free(wfut);
     }

   pd->model = model;
   pd->model_owned = EINA_FALSE;
   if (model)
     {
        efl_composite_attach(obj, pd->model);
        efl_event_callback_array_add(pd->model, _model_callbacks(), obj);
        for (k = 0, cb = _model_callbacks(); cb->desc;
             k++, cb = _model_callbacks() + k)
          efl_event_callback_forwarder_add(pd->model, cb->desc, obj);
        future = efl_model_children_slice_get(pd->model, 0, 0);
        efl_future_then(future, _children_slice_cb, NULL, NULL, obj);
        pd->pending_futures = eina_list_append(pd->pending_futures, future);
     }
}

EOLIAN static Efl_Model *
_efl_ui_item_list_model_get(Eo *obj EINA_UNUSED, Efl_Ui_Item_List_Data *pd)
{
   return pd->model;
}

EOLIAN static Eo *
_efl_ui_item_list_efl_object_constructor(Eo *obj, Efl_Ui_Item_List_Data *pd)
{
   pd->self = obj;
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->model_owned = EINA_TRUE;
   efl_ui_item_list_model_set(obj, efl_add(EFL_MODEL_ITEM_CLASS, obj));

   return obj;
}

#include "efl_ui_item_list.eo.c"

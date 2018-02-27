#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eina.h"
#include "Efl.h"
#include <Ecore.h>
#include "Eo.h"

#include "efl_model_composite_selection.eo.h"
#include "efl_model_accessor_view_private.h"

typedef struct _Efl_Model_Composite_Selection_Data
{
   Efl_Model_Composite_Selection* obj;
   struct {
      Efl_Model* selected_child;
   } exclusive_data;
   Eina_Bool is_exclusive;
  
} Efl_Model_Composite_Selection_Data;

typedef struct _Efl_Model_Composite_Selection_Children_Data
{
   Efl_Model_Composite_Selection_Data* pd;
} Efl_Model_Composite_Selection_Children_Data;

static Eo*
_efl_model_composite_selection_efl_object_constructor(Eo *obj,
  Efl_Model_Composite_Selection_Data *pd EINA_UNUSED)
{
   efl_constructor(efl_super(obj, EFL_MODEL_COMPOSITE_SELECTION_CLASS));
   efl_model_composite_boolean_property_add(obj, "selected", EINA_FALSE);
   pd->obj = obj;
   return obj;
}

/***************************/
static void _select_property_failed(void* data, Efl_Event const* event)
{
   Efl_Promise* promise = data;
   Efl_Future_Event_Failure* fail = event->info;
   efl_promise_failed_set(promise, fail->error);
   efl_unref(promise);
}

static void _select_property_then(void* data, Efl_Event const* event EINA_UNUSED)
{
   Efl_Promise* promise = data;
   Eina_Value* v = eina_value_new(EINA_VALUE_TYPE_UCHAR);
   eina_value_set(v, EINA_TRUE);
   efl_promise_value_set(promise, v, (Eina_Free_Cb)&eina_value_free);
   efl_unref(promise);
}

static void _select_slice_then(void* data EINA_UNUSED, Efl_Event const* event)
{
   Efl_Future_Event_Success* success = event->info;
   Eina_Accessor* accessor = eina_accessor_clone(success->value);
   Eina_Value value;
   Efl_Model* child;

   if (!eina_accessor_data_get(accessor, 0, (void **)&child)) return;
    
   eina_value_setup(&value, EINA_VALUE_TYPE_UCHAR);
   eina_value_set(&value, EINA_TRUE);
  
   efl_future_then(efl_model_property_set(child, "selected", &value),
                   _select_property_then, _select_property_failed, NULL, efl_ref(success->next));
}

static void _select_error(void* data EINA_UNUSED, Efl_Event const* event)
{
   Efl_Future_Event_Failure* fail = event->info;
   efl_promise_failed_set(fail->next, fail->error);
}

/***************************/

static Efl_Future*
_efl_model_composite_selection_select(Eo *obj,
                                      Efl_Model_Composite_Selection_Data *pd, int idx)
{
   return efl_future_then(efl_model_children_slice_get(obj, idx, 1),
                          &_select_slice_then,
                          &_select_error,
                          NULL, pd);
}

static void
_efl_model_composite_selection_exclusive_selection_set(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Selection_Data *pd, Eina_Bool exclusive)
{
   pd->is_exclusive = exclusive;
}

static Eina_Bool
_efl_model_composite_selection_exclusive_selection_get(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Selection_Data *pd)
{
  return pd->is_exclusive;
}

static void
_exclusive_future_link_then_cb(void* data, Efl_Event const* event)
{
   Efl_Future_Event_Success *success = event->info;
   efl_promise_value_set(data, success->value, NULL); // We would need to move the value
                                                      // Needs to set exclusive_child
   efl_unref(data);
}

static void
_exclusive_future_link_failed(void* data, Efl_Event const* event)
{
   Efl_Future_Event_Failure *failed = event->info;
   efl_promise_failed_set(data, failed->error);
   efl_unref(data);
}

static void
_exclusive_unselected_then_cb(void* data, Efl_Event const* event)
{
   Efl_Future_Event_Success *success = event->info;
   Eina_Value* true_value = eina_value_new(EINA_VALUE_TYPE_UCHAR);
   eina_value_set(true_value, EINA_TRUE);
   efl_future_then(efl_model_property_set(data, "selected", true_value),
                   _exclusive_future_link_then_cb, _exclusive_future_link_failed,
                   NULL, efl_ref(success->next));
   efl_unref(data);
}

static void
_exclusive_unselected_failed(void* data, Efl_Event const* event)
{
   Efl_Future_Event_Failure *failed = event->info;
   efl_promise_failed_set(data, failed->error);
   efl_unref(data);
}

static Efl_Future *
_efl_model_composite_selection_children_efl_model_property_set(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Selection_Children_Data *pd, const char *property, const Eina_Value *value)
{
   if(!strcmp("selected", property))
     {
        unsigned long v = EINA_FALSE;
        if(eina_value_type_get(value) != EINA_VALUE_TYPE_ULONG)
          {
            Eina_Value to;
            eina_value_setup(&to, EINA_VALUE_TYPE_ULONG);
            if(eina_value_convert(value, &to))
              eina_value_get(&to, &v);
            eina_value_flush(&to);
          }
        else
          eina_value_get(value, &v);

        if(v && pd->pd->is_exclusive)
          {
             if(pd->pd->exclusive_data.selected_child)
               {
                  // unset current selected
                  // set this child as current
                  // bookkeep this child as current selection
                  // return with future for this asynchronous task
                  Eina_Value* false_value = eina_value_new(EINA_VALUE_TYPE_UCHAR);
                  eina_value_set(false_value, EINA_FALSE);
                  return
                    efl_future_then(efl_model_property_set
                                    (pd->pd->exclusive_data.selected_child,
                                     property, false_value),
                                    _exclusive_unselected_then_cb,
                                    _exclusive_unselected_failed, NULL,
                                    efl_ref(obj));
               }
             else
               {
                  Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, efl_main_loop_get());
                  Efl_Future *rfuture = efl_promise_future_get(promise);
                  Eina_Value* true_value = eina_value_new(EINA_VALUE_TYPE_UCHAR);
                  eina_value_set(true_value, EINA_TRUE);
                  efl_future_then(efl_model_property_set
                                  (efl_super(obj, EFL_MODEL_COMPOSITE_SELECTION_CHILDREN_CLASS),
                                   "selected", true_value),
                                  _exclusive_future_link_then_cb, _exclusive_future_link_failed,
                                  NULL, efl_ref(promise));
                  return rfuture;
               }             
          }
        else
          {
            
          }
     }

   return efl_model_property_set(efl_super(obj, EFL_MODEL_COMPOSITE_SELECTION_CHILDREN_CLASS),
                                 property, value);
}

static Eo* _construct_children(void* pdata, Eo* child)
{
  Efl_Model_Composite_Selection_Data* pd = pdata;
  Eo* new_child = efl_add(EFL_MODEL_COMPOSITE_SELECTION_CHILDREN_CLASS, NULL);
  Efl_Model_Composite_Selection_Children_Data* data = efl_data_scope_get
    (new_child, EFL_MODEL_COMPOSITE_SELECTION_CHILDREN_CLASS);
  data->pd = pd;
  efl_composite_attach(new_child, efl_ref(child));
  return new_child;
}

static void _composited_children_slice_get_then(void* data, Efl_Event const* event)
{
  Efl_Future_Event_Success* success = event->info;
  Eina_Accessor* accessor = success->value;
  efl_promise_value_set(success->next,
                        efl_model_accessor_view_new(eina_accessor_clone(accessor), &_construct_children,
                                                    data),
                        (Eina_Free_Cb)&eina_accessor_free);
}
static void _composited_children_slice_get_fail(void* data EINA_UNUSED, Efl_Event const* event)
{
  Efl_Future_Event_Failure* failure = event->info;
  efl_promise_failed_set(failure->next, failure->error);
}

static Efl_Future *
_efl_model_composite_selection_efl_model_children_slice_get(Eo *obj, Efl_Model_Composite_Selection_Data *pd, unsigned int start, unsigned int count)
{
   Efl_Future* composited_future = efl_model_children_slice_get
     (efl_super(obj, EFL_MODEL_COMPOSITE_SELECTION_CLASS),start, count);
   return efl_future_then(composited_future, &_composited_children_slice_get_then,
                          &_composited_children_slice_get_fail, NULL, pd);
}

#include "efl_model_composite_selection.eo.c"
#include "efl_model_composite_selection_children.eo.c"

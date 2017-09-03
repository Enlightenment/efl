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
   return obj;
}

/***************************/

static void _select_slice_then(void* data, Eo_Event const* event)
{
  
}

/***************************/

static Efl_Future*
_efl_model_composite_selection_select(Eo *obj,
                                      Efl_Model_Composite_Selection_Data *pd, int idx)
{
   if(pd->is_exclusive)
     {
        
     }
   else
     {
        
     }
}

static Efl_Future *
_efl_model_composite_selection_children_efl_model_property_set(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Selection_Children_Data *pd, const char *property, const Eina_Value *value)
{

   fprintf(stderr, "%s %s:%d %s\n", __func__, __FILE__, __LINE__, property); fflush(stderr);
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

        
        if(v && pd->pd->is_exclusive)
          {
             Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
             Efl_Future *rfuture = efl_promise_future_get(promise);
             // unset current selected
             // set this child as current
             // bookkeep this child as current selection
             // return with future for this asynchronous task
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
  efl_composite_attach(new_child, child);
  return new_child;
}

static void _composited_children_slice_get_then(void* data, Efl_Event const* event)
{
  Efl_Promise* promise = data;
  Efl_Future_Event_Success* success = event->info;
  fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
  efl_promise_value_set(promise,
                        efl_model_accessor_view_new(success->value, &_construct_children,
                                                    NULL),
                        (Eina_Free_Cb)&eina_accessor_free);
  fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
}
static void _composited_children_slice_get_fail(void* data, Efl_Event const* event)
{
  Efl_Promise* promise = data;
  Efl_Future_Event_Failure* failure = event->info;
  fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
  efl_promise_failed_set(promise, failure->error);
}

static Efl_Future *
_efl_model_composite_selection_efl_model_children_slice_get(Eo *obj, Efl_Model_Composite_Selection_Data *pd, unsigned int start, unsigned int count)
{
   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   Efl_Future *rfuture = efl_promise_future_get(promise);
  fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);

   Efl_Future* composited_future = efl_model_children_slice_get
     (efl_super(obj, EFL_MODEL_COMPOSITE_SELECTION_CLASS),
      start, count);
  fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   efl_future_then(composited_future, &_composited_children_slice_get_then,
                   &_composited_children_slice_get_fail, NULL, promise);
  fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   
   return rfuture;
}

#include "efl_model_composite_selection.eo.c"
#include "efl_model_composite_selection_children.eo.c"


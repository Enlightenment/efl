#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eina.h"
#include "Efl.h"
#include <Ecore.h>
#include "Eo.h"

#include "efl_loop_model.eo.h"

typedef struct _Efl_Loop_Model_Watcher_Data Efl_Loop_Model_Watcher_Data;

struct _Efl_Loop_Model_Watcher_Data
{
   const char *property;
   Eina_Promise *p;
   Eo *obj;
};

static void _propagate_future(void *data, const Efl_Event *event);

static void
_efl_loop_model_wathcer_free(Efl_Loop_Model_Watcher_Data *wd)
{
   efl_event_callback_del(wd->obj, EFL_MODEL_EVENT_PROPERTIES_CHANGED,
                          _propagate_future, wd);
   eina_stringshare_del(wd->property);
   free(wd);
}

static void
_propagate_future(void *data, const Efl_Event *event)
{
   Efl_Model_Property_Event *ev = event->info;
   const char *property;
   unsigned int i;
   Eina_Array_Iterator it;
   Efl_Loop_Model_Watcher_Data *wd = data;

   EINA_ARRAY_ITER_NEXT(ev->changed_properties, i, property, it)
     if (property == wd->property || !strcmp(property, wd->property))
       {
          Eina_Value *v = efl_model_property_get(wd->obj, wd->property);

          if (eina_value_type_get(v) == EINA_VALUE_TYPE_ERROR)
            {
               Eina_Error err = 0;

               eina_value_get(v, &err);

               if (err == EAGAIN)
                 return ; // Not ready yet

               eina_promise_reject(wd->p, err);
            }
          else
            {
               eina_promise_resolve(wd->p, eina_value_reference_copy(v));
            }

          eina_value_free(v);
          _efl_loop_model_wathcer_free(wd);
          break ;
       }
}

static void
_event_cancel(void *data, const Eina_Promise *dead_ptr EINA_UNUSED)
{
   _efl_loop_model_wathcer_free(data);
}

static Eina_Future *
_efl_loop_model_efl_model_property_ready_get(Eo *obj, void *pd EINA_UNUSED, const char *property)
{
   Eina_Value *value = efl_model_property_get(obj, property);
   Eina_Future *f;

   if (eina_value_type_get(value) == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err = 0;

        eina_value_get(value, &err);
        eina_value_free(value);

        if (err == EAGAIN)
          {
             Efl_Loop_Model_Watcher_Data *wd = calloc(1, sizeof (Efl_Loop_Model_Watcher_Data));

             wd->obj = obj;
             wd->property = eina_stringshare_add(property);
             wd->p = eina_promise_new(efl_loop_future_scheduler_get(obj),
                                      _event_cancel, wd);

             efl_event_callback_add(obj,
                                    EFL_MODEL_EVENT_PROPERTIES_CHANGED,
                                    _propagate_future, wd);
          }

        return eina_future_rejected(efl_loop_future_scheduler_get(obj), err);
     }
   f = eina_future_resolved(efl_loop_future_scheduler_get(obj),
                            eina_value_reference_copy(value));
   eina_value_free(value);
   return efl_future_then(obj, f);
}

#include "efl_loop_model.eo.c"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include "ecore_private.h"

typedef struct _Efl_Loop_Consumer_Data Efl_Loop_Consumer_Data;
struct _Efl_Loop_Consumer_Data
{
};

static Efl_Loop *
_efl_loop_consumer_loop_get(const Eo *obj, Efl_Loop_Consumer_Data *pd EINA_UNUSED)
{
   Efl_Loop *loop = efl_provider_find(obj, EFL_LOOP_CLASS);
   if (!loop && eina_main_loop_is())
     {
        loop = efl_main_loop_get();
        ERR("Failed to get the loop on object %p from the main thread! "
            "Returning the main loop: %p", obj, loop);
     }
   return loop;
}

static void
_efl_loop_consumer_efl_object_parent_set(Eo *obj, Efl_Loop_Consumer_Data *pd EINA_UNUSED, Efl_Object *parent)
{
   if (parent != NULL && efl_provider_find(parent, EFL_LOOP_CLASS) == NULL)
     {
        ERR("parent=%p is not a provider of EFL_LOOP_CLASS!", parent);
        return;
     }

   efl_parent_set(efl_super(obj, EFL_LOOP_CONSUMER_CLASS), parent);
}

static Eina_Future *
_efl_loop_consumer_future_resolved(const Eo *obj, Efl_Loop_Consumer_Data *pd EINA_UNUSED,
                                   Eina_Value result)
{
   return eina_future_resolved(efl_loop_future_scheduler_get(obj), result);
}

static Eina_Future *
_efl_loop_consumer_future_rejected(const Eo *obj, Efl_Loop_Consumer_Data *pd EINA_UNUSED,
                                   Eina_Error error)
{
   return eina_future_rejected(efl_loop_future_scheduler_get(obj), error);
}

static void
_dummy_cancel(void *data EINA_UNUSED, const Eina_Promise *p EINA_UNUSED)
{
}

static Eina_Promise *
_efl_loop_consumer_promise_new(const Eo *obj, Efl_Loop_Consumer_Data *pd EINA_UNUSED)
{
   return eina_promise_new(efl_loop_future_scheduler_get(obj), _dummy_cancel, NULL);
}

#include "efl_loop_consumer.eo.c"

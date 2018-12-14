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

typedef struct _Efl_Loop_Consumer_Promise Efl_Loop_Consumer_Promise;
struct _Efl_Loop_Consumer_Promise
{
   EflLoopConsumerPromiseCancel func;
   Eina_Free_Cb free;
   void *data;
   Eo *obj;
};

static void
_cancel_free(void *data)
{
   Efl_Loop_Consumer_Promise *lcp = data;

   if (lcp->free) lcp->free(lcp->data);
   efl_unref(lcp->obj);
   free(lcp);
}

static void
_cancel_triggered(void *data, const Eina_Promise *p)
{
   Efl_Loop_Consumer_Promise *lcp = data;

   if (lcp->func) lcp->func(lcp->data, lcp->obj, p);
}

static void
_data_set(Eina_Promise *p, void *data)
{
   Efl_Loop_Consumer_Promise *lcp = eina_promise_data_get(p);
   lcp->data = data;
}

static void
_cancel_free_cb_set(Eina_Promise *p, Eina_Free_Cb free_cb)
{
   Efl_Loop_Consumer_Promise *lcp = eina_promise_data_get(p);
   lcp->free = free_cb;
}

static Eina_Promise *
_efl_loop_consumer_promise_new(Eo *obj, Efl_Loop_Consumer_Data *pd EINA_UNUSED,
                               void *cancel_data, EflLoopConsumerPromiseCancel cancel, Eina_Free_Cb cancel_free_cb)
{
   Efl_Loop_Consumer_Promise *lcp;
   Eina_Promise *p;

   lcp = calloc(1, sizeof (Efl_Loop_Consumer_Promise));
   if (!lcp) return NULL;

   lcp->func = cancel;
   lcp->data = cancel_data;
   lcp->free = cancel_free_cb;
   lcp->obj = efl_ref(obj);

   p = eina_promise_new(efl_loop_future_scheduler_get(obj), _cancel_triggered, lcp);
   eina_promise_data_set_cb_set(p, _data_set);
   eina_promise_data_free_cb_set_cb_set(p, _cancel_free_cb_set);
   eina_promise_data_free_cb_set(p, _cancel_free);
   return p;
}

#include "efl_loop_consumer.eo.c"

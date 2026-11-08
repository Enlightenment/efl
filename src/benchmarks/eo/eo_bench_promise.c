#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo_bench.h"
#include "class_simple.h"

static const Efl_Event_Description _dispatch_event = EFL_EVENT_DESCRIPTION("bench,dispatch");
EFL_SCHEDULER_ARRAY_DEFINE(_dispatch_array, &_dispatch_event);

static void
_promise_cancel_cb(void *data EINA_UNUSED, const Eina_Promise *dead EINA_UNUSED)
{
}

static Eina_Value
_future_then_cb(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   return v;
}

static void
bench_eo_promise_dispatch(int request)
{
   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   Eina_Future_Scheduler *scheduler = efl_event_future_scheduler_get(obj, _dispatch_array());
   int i;

   for (i = 0; i < request; i++)
     {
        Eina_Promise *p = eina_promise_new(scheduler, _promise_cancel_cb, NULL);
        Eina_Future *f = eina_future_new(p);

        eina_future_then(f, _future_then_cb, NULL);
        eina_promise_resolve(p, eina_value_int_init(i));
        efl_event_callback_call(obj, &_dispatch_event, NULL);
     }

   efl_unref(obj);
}

static void
bench_eo_promise_cancel(int request)
{
   Eo *obj[2];
   Eina_Future_Scheduler *scheduler[2];
   int i;

   for (i = 0; i < 2; i++)
     {
        obj[i] = efl_add_ref(SIMPLE_CLASS, NULL);
        scheduler[i] = efl_event_future_scheduler_get(obj[i], _dispatch_array());
     }

   for (i = 0; i < request; i++)
     {
        Eina_Promise *p = eina_promise_new(scheduler[i & 1], _promise_cancel_cb, NULL);
        Eina_Future *f = eina_future_new(p);

        eina_future_then(f, _future_then_cb, NULL);
        eina_future_cancel(f);
     }

   for (i = 0; i < 2; i++)
     {
        efl_unref(obj[i]);
     }
}

void eo_bench_promise(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "dispatch",
         EINA_BENCHMARK(bench_eo_promise_dispatch), _EO_BENCH_TIMES(20000, 5, 20000));
   eina_benchmark_register(bench, "cancel",
         EINA_BENCHMARK(bench_eo_promise_cancel), _EO_BENCH_TIMES(20000, 5, 20000));
}

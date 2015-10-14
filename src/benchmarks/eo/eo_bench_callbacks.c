#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo_bench.h"
#include "class_simple.h"

static Eina_Bool
_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   return EO_CALLBACK_CONTINUE;
}


static void
bench_eo_callbacks_add(int request)
{
   int i;
   Eo *obj = eo_add(SIMPLE_CLASS, NULL);

   for (i = 0 ; i < request ; i++)
     {
        eo_do(obj, eo_event_callback_priority_add(SIMPLE_FOO, (short) i, _cb, NULL));
     }

   eo_unref(obj);
}

static void
bench_eo_callbacks_call(int request)
{
   /* Distribution of calls per amount of callbacks in an object as recorded by
      running the genlist elementary_test test. */
   const double distribution[] = {
        0.2920468197,
        0.2073086496,
        0.217699456,
        0.0207158285,
        0.019707134,
        0.0359433565,
        0.0324896742,
        0.0104299639,
        0.028989003,
        0.0082496801,
        0.123214227,
        0.0001331351,
        0.0030730724
   };

   const int len = EINA_C_ARRAY_LENGTH(distribution);
   int i, j;
   Eo *obj[len] = { 0 };
   for (i = 0 ; i < len ; i++)
     {
        obj[i] = eo_add(SIMPLE_CLASS, NULL);

        for (j = 0 ; j < i ; j++)
          {
             eo_do(obj[i], eo_event_callback_priority_add(SIMPLE_FOO, (short) j, _cb, NULL));
          }
     }

   for (i = 0 ; i < len ; i++)
     {
        for (j = 0 ; j < (int) (distribution[i] * request) ; j++)
          {
             /* Miss finding the callbacks on purpose, so we measure worst case scenario. */
             eo_do(obj[i], eo_event_callback_call(SIMPLE_BAR, NULL));
          }
     }

   for (i = 0 ; i < len ; i++)
     {
        eo_unref(obj[i]);
     }
}

void eo_bench_callbacks(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "add",
         EINA_BENCHMARK(bench_eo_callbacks_add), _EO_BENCH_TIMES(1000, 10, 2000));
   eina_benchmark_register(bench, "call",
         EINA_BENCHMARK(bench_eo_callbacks_call), _EO_BENCH_TIMES(100000, 10, 500000));
}

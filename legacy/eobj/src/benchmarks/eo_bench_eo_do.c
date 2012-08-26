#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Eo.h"
#include "eo_bench.h"

#include "class_simple.h"

static void
bench_eo_do_general(int request)
{
   int i;
   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   for (i = 0 ; i < request ; i++)
     {
        eo_do(obj, simple_a_set(i));
     }

   eo_unref(obj);
}

void eo_bench_eo_do(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "general",
         EINA_BENCHMARK(bench_eo_do_general), 100, 10000, 500);
}

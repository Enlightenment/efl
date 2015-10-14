#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo_bench.h"
#include "class_simple.h"

static void
bench_eo_add_linear(int request)
{
   int i;
   Eo **objs = calloc(request, sizeof(Eo *));
   for (i = 0 ; i < request ; i++)
      objs[i] = eo_add(SIMPLE_CLASS, NULL);

   for (i = 0 ; i < request ; i++)
      eo_unref(objs[i]);
   free(objs);
}

static void
bench_eo_add_jump_by_2(int request)
{
   int i;
   Eo **objs = calloc(request, sizeof(Eo *));
   for (i = 0 ; i < request ; i++)
      objs[i] = eo_add(SIMPLE_CLASS, NULL);

   for (i = 0 ; i < request ; i += 2)
      eo_unref(objs[i]);

   for (i = 0 ; i < request ; i += 2)
      objs[i] = eo_add(SIMPLE_CLASS, NULL);

   for (i = 0 ; i < request ; i++)
      eo_unref(objs[i]);
   free(objs);
}

void eo_bench_eo_add(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "eo_add_linear",
         EINA_BENCHMARK(bench_eo_add_linear), _EO_BENCH_TIMES(1000, 10, 50000));
   eina_benchmark_register(bench, "eo_add_jump_by_2",
         EINA_BENCHMARK(bench_eo_add_jump_by_2), _EO_BENCH_TIMES(1000, 10, 50000));
}

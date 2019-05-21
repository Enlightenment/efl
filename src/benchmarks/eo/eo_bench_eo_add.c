#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo_bench.h"
#include "class_simple.h"

static void
bench_efl_add_linear(int request)
{
   int i;
   Eo **objs = calloc(request, sizeof(Eo *));
   for (i = 0 ; i < request ; i++)
      objs[i] = efl_add_ref(SIMPLE_CLASS, NULL);

   for (i = 0 ; i < request ; i++)
      efl_unref(objs[i]);
   free(objs);
}

static void
bench_efl_add_jump_by_2(int request)
{
   int i;
   Eo **objs = calloc(request, sizeof(Eo *));
   for (i = 0 ; i < request ; i++)
      objs[i] = efl_add_ref(SIMPLE_CLASS, NULL);

   for (i = 0 ; i < request ; i += 2)
      efl_unref(objs[i]);

   for (i = 0 ; i < request ; i += 2)
      objs[i] = efl_add_ref(SIMPLE_CLASS, NULL);

   for (i = 0 ; i < request ; i++)
      efl_unref(objs[i]);
   free(objs);
}

static void
bench_efl_add_shared_ownership(int request)
{
   int i;
   Eo **objs = calloc(request, sizeof(Eo *));
   Eo *p = efl_add_ref(SIMPLE_CLASS, NULL);
   for (i = 0; i < request; i++)
      objs[i] = efl_add_ref(SIMPLE_CLASS, p);
   efl_unref(p);
   for (i = 0; i < request; i++)
      efl_unref(objs[i]);
   free(objs);
}

static void
bench_efl_add_shared_ownership_alternative(int request)
{
   int i;
   Eo **objs = calloc(request, sizeof(Eo *));
   Eo *p = efl_add_ref(SIMPLE_CLASS, NULL);
   for (i = 0; i < request; i++)
      objs[i] = efl_add(SIMPLE_CLASS, p);
   for (i = 0; i < request; i++)
      efl_ref(objs[i]);
   for (i = 0; i < request; i++)
      efl_unref(objs[i]);
   efl_unref(p);
   free(objs);
}

void eo_bench_efl_add(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "efl_add_linear",
         EINA_BENCHMARK(bench_efl_add_linear), _EO_BENCH_TIMES(10, 500, 50));
   eina_benchmark_register(bench, "efl_add_jump_by_2",
         EINA_BENCHMARK(bench_efl_add_jump_by_2), _EO_BENCH_TIMES(10, 500, 50));
   eina_benchmark_register(bench, "efl_add_shared_ownership",
         EINA_BENCHMARK(bench_efl_add_shared_ownership), _EO_BENCH_TIMES(10, 500, 50));
   eina_benchmark_register(bench, "efl_add_shared_ownership_alternative",
         EINA_BENCHMARK(bench_efl_add_shared_ownership_alternative), _EO_BENCH_TIMES(10, 500, 50));
}

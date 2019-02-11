#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo_bench.h"
#include "class_simple.h"

static void
bench_eo_do_simple(int request)
{
   int i;
   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   for (i = 0 ; i < request ; i++)
     {
        simple_a_set(obj, i);
     }

   efl_unref(obj);
}

static void
bench_eo_do_two_objs(int request)
{
   int i;
   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj2 = efl_add_ref(SIMPLE_CLASS, NULL);
   for (i = 0 ; i < request ; i++)
     {
        simple_a_set(obj, i);
        simple_a_set(obj2, i);
     }

   efl_unref(obj);
   efl_unref(obj2);
}

static void
bench_eo_do_two_objs_growing_stack(int request)
{
   int i;
   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj2 = efl_add_ref(SIMPLE_CLASS, NULL);
   for (i = 0 ; i < request ; i++)
     {
        simple_other_call(obj, obj2, 20);
     }

   efl_unref(obj);
   efl_unref(obj2);
}

static const Efl_Class *cur_klass;

static void
_a_set(Eo *obj, void *class_data EINA_UNUSED, int a)
{
   simple_a_set(efl_super(obj, cur_klass), a);
}

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(simple_a_set, _a_set),
   );

   return efl_class_functions_set(klass, &ops, NULL, NULL);
}

static void
bench_eo_do_super(int request)
{
   static Efl_Class_Description class_desc = {
        EO_VERSION,
        "Simple2",
        EFL_CLASS_TYPE_REGULAR,
        0,
        _class_initializer,
        NULL,
        NULL
   };
   cur_klass = efl_class_new(&class_desc, SIMPLE_CLASS, NULL);

   int i;
   Eo *obj = efl_add_ref(cur_klass, NULL);
   for (i = 0 ; i < request ; i++)
     {
        simple_a_set(obj, i);
     }

   efl_unref(obj);
}

void eo_bench_eo_do(Eina_Benchmark *bench)
{
   eina_benchmark_register(bench, "simple",
         EINA_BENCHMARK(bench_eo_do_simple), _EO_BENCH_TIMES(1000, 10, 500000));
   eina_benchmark_register(bench, "super",
         EINA_BENCHMARK(bench_eo_do_super),  _EO_BENCH_TIMES(1000, 10, 500000));
   eina_benchmark_register(bench, "two_objs",
         EINA_BENCHMARK(bench_eo_do_two_objs), _EO_BENCH_TIMES(1000, 10, 500000));
   eina_benchmark_register(bench, "two_objs_growing_stack",
         EINA_BENCHMARK(bench_eo_do_two_objs_growing_stack), _EO_BENCH_TIMES(1000, 10, 40000));
}

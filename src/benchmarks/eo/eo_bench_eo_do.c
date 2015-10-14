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
   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   for (i = 0 ; i < request ; i++)
     {
        eo_do(obj, simple_a_set(i));
     }

   eo_unref(obj);
}

static void
bench_eo_do_two_objs(int request)
{
   int i;
   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   Eo *obj2 = eo_add(SIMPLE_CLASS, NULL);
   for (i = 0 ; i < request ; i++)
     {
        eo_do(obj, simple_a_set(i));
        eo_do(obj2, simple_a_set(i));
     }

   eo_unref(obj);
   eo_unref(obj2);
}

static void
bench_eo_do_two_objs_growing_stack(int request)
{
   int i;
   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   Eo *obj2 = eo_add(SIMPLE_CLASS, NULL);
   for (i = 0 ; i < request ; i++)
     {
        eo_do(obj, simple_other_call(obj2, 20));
     }

   eo_unref(obj);
   eo_unref(obj2);
}

static const Eo_Class *cur_klass;

static void
_a_set(Eo *obj, void *class_data EINA_UNUSED, int a)
{
   eo_do_super(obj, cur_klass, simple_a_set(a));
}

static Eo_Op_Description op_desc[] = {
     EO_OP_FUNC_OVERRIDE(simple_a_set, _a_set),
};

static void
bench_eo_do_super(int request)
{
   static Eo_Class_Description class_desc = {
        EO_VERSION,
        "Simple2",
        EO_CLASS_TYPE_REGULAR,
        EO_CLASS_DESCRIPTION_OPS(op_desc),
        NULL,
        0,
        NULL,
        NULL
   };
   cur_klass = eo_class_new(&class_desc, SIMPLE_CLASS, NULL);

   int i;
   Eo *obj = eo_add(cur_klass, NULL);
   for (i = 0 ; i < request ; i++)
     {
        eo_do(obj, simple_a_set(i));
     }

   eo_unref(obj);
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

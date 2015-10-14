#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include <Eina.h>

#include "Eo.h"
#include "eo_bench.h"

typedef struct _Eina_Benchmark_Case Eina_Benchmark_Case;
struct _Eina_Benchmark_Case
{
   const char *bench_case;
   void (*build)(Eina_Benchmark *bench);
};

static const Eina_Benchmark_Case etc[] = {
   { "eo_do", eo_bench_eo_do },
   { "eo_add", eo_bench_eo_add },
   { "eo_callbacks", eo_bench_callbacks },
   { NULL, NULL }
};

int
main(int argc, char **argv)
{
   Eina_Benchmark *test;
   unsigned int i;

   if (argc != 2)
      return -1;

   eina_init();
   eo_init();

   for (i = 0; etc[i].bench_case; ++i)
     {
        test = eina_benchmark_new(etc[i].bench_case, argv[1]);
        if (!test)
           continue;

        etc[i].build(test);

        eina_benchmark_run(test);

        eina_benchmark_free(test);
     }

   eo_shutdown();
   eina_shutdown();

   return 0;
}

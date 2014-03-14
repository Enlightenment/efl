#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "evas_bench.h"
#include "Eina.h"
#include "Evas.h"

typedef struct _Evas_Benchmark_Case Evas_Benchmark_Case;
struct _Evas_Benchmark_Case
{
   const char *bench_case;
   void (*build)(Eina_Benchmark *bench);
   Eina_Bool run_by_default;
};

static const Evas_Benchmark_Case etc[] = {
   { "Loader", evas_bench_loader, EINA_TRUE },
   { "Saver", evas_bench_saver, EINA_TRUE },
   { NULL, NULL, EINA_FALSE }
};

int
main(int argc, char **argv)
{
   Eina_Benchmark *test;
   unsigned int i;

   evas_init();

   for (i = 0; etc[i].bench_case; ++i)
     {
        if (argc == 2 && strcasecmp(etc[i].bench_case, argv[1]))
          continue;

        test = eina_benchmark_new(etc[i].bench_case, "default");
        if (!test)
          continue;

        etc[i].build(test);

        eina_benchmark_run(test);

        eina_benchmark_free(test);
     }

   evas_shutdown();

   return 0;
}

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eo_simple.h"
#include "eo2_simple.h"
#include "eo2_inherit.h"

static void report(struct timespec t0, struct timespec t1,
                   struct timespec t2, struct timespec t3, int n, int c)
{
   uint64_t dt0, dt1;

   dt0 = ((t1.tv_sec * 1000000000ULL) + t1.tv_nsec) -
      ((t0.tv_sec * 1000000000ULL) + t0.tv_nsec);
   dt1 = ((t3.tv_sec * 1000000000ULL) + t3.tv_nsec) -
      ((t2.tv_sec * 1000000000ULL) + t2.tv_nsec);

   printf("#%d EO: %5u [ms] %5u [ns]  - EO2: %5u [ms] %5u [ns]\n", n,
          (unsigned int)(dt0/1000000), (unsigned int)(dt0/c),
          (unsigned int)(dt1/1000000), (unsigned int)(dt1/c));
}

static void check(int val, int expected)
{
   static int count = 0;
   count++;
   if (val != expected)
     {
        fprintf(stderr, "check #%d failed %d != %d\n", count, val, expected);
        exit(count);
     }
}

#define EO_RUN_START                      \
   eo_do(eo_obj, eo_set(0));              \
   clock_gettime(CLOCK_MONOTONIC, &t0);

#define EO_RUN_END                        \
   clock_gettime(CLOCK_MONOTONIC, &t1);   \
   eo_do(eo_obj, eo_get(&a));             \
   check(a, n * k);                       \

#define EO2_RUN_START                     \
   eo2_do(eo2_obj, eo2_set(0); );         \
   clock_gettime(CLOCK_MONOTONIC, &t2);   \

#define EO2_RUN_END                       \
   clock_gettime(CLOCK_MONOTONIC, &t3);   \
   eo2_do(eo2_obj, a = eo2_get(); );      \
   check(a, n * k);                       \

static void
do_batch_test()
{
   int i, n, k, a, b, c;
   Eo *eo_obj, *eo2_obj;
   struct timespec t0, t1, t2, t3;

   a = b = c = 0;
   eo_obj = eo_add(EO_SIMPLE_CLASS, NULL);
   eo2_obj = eo2_add(EO2_SIMPLE_CLASS, NULL);

   /* check */
   eo_do(eo_obj, eo_get(&a), eo_set(10), eo_inc(), eo_get(&b), eo_inc(), eo_inc(), eo_get(&c));
   check(a, 66);
   check(b, 11);
   check(c, 13);
   eo2_do(eo2_obj,
          a = eo2_get();
          eo2_set(10);
          eo2_inc();
          b = eo2_get();
          eo2_inc();
          eo2_inc();
          c = eo2_get();
          );
   check(a, 66);
   check(b, 11);
   check(c, 13);

   n = 999999;

   // 1 call per batch
   k = 1;
   EO_RUN_START
   for (i = 0; i < n; i++)
     eo_do(eo_obj, eo_inc());
   EO_RUN_END
   EO2_RUN_START
   for (i = 0; i < n; i++)
     eo2_do(eo2_obj, eo2_inc(); );
   EO2_RUN_END
   report(t0, t1, t2, t3, k, n * k);

   // 3 calls per batch
   k = 3;
   EO_RUN_START
   for (i = 0; i < n; i++)
     eo_do(eo_obj, eo_inc(), eo_inc(), eo_inc());
   EO_RUN_END
   EO2_RUN_START
   for (i = 0; i < n; i++)
     eo2_do(eo2_obj, eo2_inc(); eo2_inc(); eo2_inc(); );
   EO2_RUN_END
   report(t0, t1, t2, t3, k, n * k);

   // 5 calls per batch
   k = 5;
   EO_RUN_START
   for (i = 0; i < n; i++)
     eo_do(eo_obj, eo_inc(), eo_inc(), eo_inc(), eo_inc(), eo_inc());
   EO_RUN_END
   EO2_RUN_START
   for (i = 0; i < n; i++)
     eo2_do(eo2_obj, eo2_inc(); eo2_inc(); eo2_inc(); eo2_inc(); eo2_inc(); );
   EO2_RUN_END
   report(t0, t1, t2, t3, k, n * k);

   // 7 calls per batch
   k = 7;
   EO_RUN_START
   for (i = 0; i < n; i++)
     eo_do(eo_obj, eo_inc(), eo_inc(), eo_inc(), eo_inc(), eo_inc(), eo_inc(), eo_inc());
   EO_RUN_END
   EO2_RUN_START
   for (i = 0; i < n; i++)
     eo2_do(eo2_obj, eo2_inc(); eo2_inc(); eo2_inc(); eo2_inc(); eo2_inc(); eo2_inc(); eo2_inc(); );
   EO2_RUN_END
   report(t0, t1, t2, t3, k, n * k);

   eo_del(eo_obj);
   eo_del(eo2_obj);
}

static void
inherit_batch_test()
{
   int a, b;
   Eo *eo2_obj;

   a = b = 0;
   eo2_obj = eo2_add(EO2_INHERIT_CLASS, NULL);

   eo2_do(eo2_obj, eo2_set(65); a = eo2_get(); b = eo2_inherit_get(); );
   check(a, 65);
   check(b, 68);
   eo2_do(eo2_obj, eo2_inc(); a = eo2_get(); b = eo2_inherit_get(); );
   check(a, 66);
   check(b, 69);

   eo_del(eo2_obj);
}

int
main(int argc EINA_UNUSED, char** argv EINA_UNUSED, char** env EINA_UNUSED)
{
   eo_init();

   do_batch_test();
   inherit_batch_test();

   eo_shutdown();

   return 0;
}


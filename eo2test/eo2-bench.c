#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eo_simple.h"
#include "eo2_simple.h"

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
   eo_do(obj, set(0));                    \
   clock_gettime(CLOCK_MONOTONIC, &t0);

#define EO_RUN_END                        \
   clock_gettime(CLOCK_MONOTONIC, &t1);   \
   eo_do(obj, get(&a));                   \
   check(a, n * k);                       \

#define EO2_RUN_START                     \
   eo2_do(obj2, set2(eo2_o, 0); );        \
   clock_gettime(CLOCK_MONOTONIC, &t2);   \

#define EO2_RUN_END                       \
   clock_gettime(CLOCK_MONOTONIC, &t3);   \
   eo2_do(obj2, a = get2(eo2_o); );       \
   check(a, n * k);                       \

int
main(int argc EINA_UNUSED, char** argv EINA_UNUSED, char** env EINA_UNUSED)
{
   int i, n, k, a, b, c;
   Eo *obj, *obj2;
   struct timespec t0, t1, t2, t3;

   eo_init();

   obj = eo_add(SIMPLE_CLASS, NULL);
   obj2 = eo_add(EO2_SIMPLE_CLASS, NULL);

   /* check */
   eo_do(obj, get(&a), set(10), inc(), get(&b), inc(), inc(), get(&c));
   check(a, 66);
   check(b, 11);
   check(c, 13);
   eo2_do(obj2,
          a = get2(eo2_o);
          set2(eo2_o, 10);
          inc2(eo2_o);
          b = get2(eo2_o);
          inc2(eo2_o);
          inc2(eo2_o);
          c = get2(eo2_o);
          );
   check(a, 66);
   check(b, 11);
   check(c, 13);

   n = 999999;

   // 1 call per batch
   k = 1;
   EO_RUN_START
   for (i = 0; i < n; i++)
     eo_do(obj, inc());
   EO_RUN_END
   EO2_RUN_START
   for (i = 0; i < n; i++)
     eo2_do(obj2, inc2(eo2_o); );
   EO2_RUN_END
   report(t0, t1, t2, t3, k, n * k);

   // 3 calls per batch
   k = 3;
   EO_RUN_START
   for (i = 0; i < n; i++)
     eo_do(obj, inc(), inc(), inc());
   EO_RUN_END
   EO2_RUN_START
   for (i = 0; i < n; i++)
     eo2_do(obj2, inc2(eo2_o); inc2(eo2_o); inc2(eo2_o); );
   EO2_RUN_END
   report(t0, t1, t2, t3, k, n * k);

   // 5 calls per batch
   k = 5;
   EO_RUN_START
   for (i = 0; i < n; i++)
     eo_do(obj, inc(), inc(), inc(), inc(), inc());
   EO_RUN_END
   EO2_RUN_START
   for (i = 0; i < n; i++)
     eo2_do(obj2, inc2(eo2_o); inc2(eo2_o); inc2(eo2_o); inc2(eo2_o); inc2(eo2_o); );
   EO2_RUN_END
   report(t0, t1, t2, t3, k, n * k);

   // 7 calls per batch
   k = 7;
   EO_RUN_START
   for (i = 0; i < n; i++)
     eo_do(obj, inc(), inc(), inc(), inc(), inc(), inc(), inc());
   EO_RUN_END
   EO2_RUN_START
   for (i = 0; i < n; i++)
     eo2_do(obj2, inc2(eo2_o); inc2(eo2_o); inc2(eo2_o); inc2(eo2_o); inc2(eo2_o); inc2(eo2_o); inc2(eo2_o); );
   EO2_RUN_END
   report(t0, t1, t2, t3, k, n * k);

   eo_del(obj);
   eo_del(obj2);

   eo_shutdown();

   return 0;
}


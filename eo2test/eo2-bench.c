
#include "eo_simple.h"
#include "eo2test.h"

int
main(int argc, char** argv, char** env)
{
   int i, a, b;
   Eo *obj, *obj2;
   uint64_t dt;
   struct timespec t0;
   struct timespec t1;

   eo_init();

   obj = eo_add(SIMPLE_CLASS, NULL);
   obj2 = eo_add(TEST_CLASS, NULL);

   /* EO */

   clock_gettime(CLOCK_MONOTONIC, &t0);
   for (i = 0; i < 1000000; i++)
     {
        eo_do(obj,
              simple_set(32, 12),
              simple_set(10, 14),
              simple_a_get(&a),
              simple_set(50, 24),
              simple_set(32, 12),
              simple_a_get(&a),
              simple_set(10, 14),
              simple_set(50, 24)
             );
     }
   clock_gettime(CLOCK_MONOTONIC, &t1);

   dt = ((t1.tv_sec * 1000000000) + t1.tv_nsec) -
      ((t0.tv_sec * 1000000000) + t0.tv_nsec);
   printf(" EO : time %7u [ms]\n", (unsigned int)(dt/1000000));

   /* EO2 */

   clock_gettime(CLOCK_MONOTONIC, &t0);
   for (i = 0; i < 1000000; i++)
     {
        // here we handle return values !
        eo2_do(obj2,
               a = inst_func_set(eo2_o, 32, 12);
               inst_func_set(eo2_o, 10, 14);
               a = inst_func_get(eo2_o);
               b = inst_func_set(eo2_o, 50, 24);
               inst_func_set(eo2_o, 32, 12);
               a = inst_func_get(eo2_o);
               a = inst_func_set(eo2_o, 10, 14);
               inst_func_set(eo2_o, 50, 24);
              );
     }
   clock_gettime(CLOCK_MONOTONIC, &t1);

   dt = ((t1.tv_sec * 1000000000) + t1.tv_nsec) -
      ((t0.tv_sec * 1000000000) + t0.tv_nsec);
   printf(" EO2: time %7u [ms]\n", (unsigned int)(dt/1000000));

   /* done */

   eo_del(obj);
   eo_del(obj2);

   eo_shutdown();

   return 0;
}


#include "Eobj.h"
#include "simple.h"
#include "mixin.h"
#include "mixin2.h"
#include "mixin3.h"

#include "../eunit_tests.h"

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eobj_init();

   Eobj *obj = eobj_add(SIMPLE_CLASS, NULL);

   eobj_do(obj, simple_a_set(1), simple_b_set(2));

   int a, b, sum = 0;
   eobj_do(obj, simple_a_get(&a), simple_b_get(&b),  mixin_ab_sum_get(&sum));
   fail_if(sum != a + b + 2); /* 2 for the two mixins... */

   eobj_do(obj, mixin_ab_sum_get(&sum), mixin_ab_sum_get(&sum));

   Mixin2_Public_Data *pd2 = eobj_data_get(obj, MIXIN2_CLASS);
   fail_if(pd2->count != 6);

   Mixin3_Public_Data *pd3 = eobj_data_get(obj, MIXIN3_CLASS);
   fail_if(pd3->count != 9);

   eobj_unref(obj);
   eobj_shutdown();
   return 0;
}


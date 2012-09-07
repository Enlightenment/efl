#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "simple.h"
#include "inherit.h"
#include "mixin.h"
#include "mixin2.h"
#include "mixin3.h"

#include "../eunit_tests.h"

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eo_init();

   Eo *obj = eo_add(SIMPLE_CLASS, NULL);

   eo_do(obj, simple_a_set(1), simple_b_set(2));

   int a, b, sum = 0;
   eo_do(obj, simple_a_get(&a), simple_b_get(&b),  mixin_ab_sum_get(&sum));
   fail_if(sum != a + b + 2); /* 2 for the two mixins... */

   eo_do(obj, mixin_ab_sum_get(&sum), mixin_ab_sum_get(&sum));

   Mixin2_Public_Data *pd2 = eo_data_get(obj, MIXIN2_CLASS);
   fail_if(pd2->count != 6);

   Mixin3_Public_Data *pd3 = eo_data_get(obj, MIXIN3_CLASS);
   fail_if(pd3->count != 9);

   eo_unref(obj);

   obj = eo_add(INHERIT_CLASS, NULL);
   eo_do(obj, simple_a_set(5), simple_a_get(&a));
   fail_if(a != 5);

   eo_unref(obj);
   eo_shutdown();
   return 0;
}


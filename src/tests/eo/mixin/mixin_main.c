#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin_simple.h"
#include "mixin_inherit.h"
#include "mixin_mixin.h"
#include "mixin_mixin2.h"
#include "mixin_mixin3.h"

#include "../eunit_tests.h"

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eo_init();

   Eo *obj = eo2_add(SIMPLE_CLASS, NULL);

   eo2_do(obj, simple_a_set(1), simple_b_set(2));

   int a, b, sum = 0;
   eo2_do(obj, a = simple_a_get(), b = simple_b_get(), sum = mixin_ab_sum_get());
   fail_if(sum != a + b + 2); /* 2 for the two mixins... */

   eo2_do(obj, sum = mixin_ab_sum_get(), sum = mixin_ab_sum_get());

   Mixin2_Public_Data *pd2 = eo_data_scope_get(obj, MIXIN2_CLASS);
   fail_if(pd2->count != 6);

   Mixin3_Public_Data *pd3 = eo_data_scope_get(obj, MIXIN3_CLASS);
   fail_if(pd3->count != 9);

   eo_unref(obj);

   obj = eo2_add(INHERIT_CLASS, NULL);
   eo2_do(obj, simple_a_set(5), a = simple_a_get());
   printf("%d\n", a);
   fail_if(a != 5);

   eo_unref(obj);
   eo_shutdown();
   return 0;
}


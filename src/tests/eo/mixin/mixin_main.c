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
   efl_object_init();

   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);

   simple_a_set(obj, 1);
   simple_b_set(obj, 2);

   int a = 0, b = 0, sum = 0;
   a = simple_a_get(obj);
   b = simple_b_get(obj);
   sum = mixin_ab_sum_get(obj);
   fail_if(sum != a + b + 2); /* 2 for the two mixins... */

   sum = mixin_ab_sum_get(obj);
   sum = mixin_ab_sum_get(obj);

   Mixin2_Public_Data *pd2 = efl_data_scope_get(obj, MIXIN2_CLASS);
   fail_if(pd2->count != 6);

   Mixin3_Public_Data *pd3 = efl_data_scope_get(obj, MIXIN3_CLASS);
   fail_if(pd3->count != 9);

   efl_unref(obj);

   obj = efl_add_ref(INHERIT_CLASS, NULL);
   simple_a_set(obj, 5);
   a = simple_a_get(obj);
   printf("%d\n", a);
   fail_if(a != 5);

   efl_unref(obj);
   efl_object_shutdown();
   return 0;
}


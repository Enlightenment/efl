#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "interface_simple.h"
#include "interface_interface.h"
#include "interface_interface2.h"

#include "../eunit_tests.h"

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eo_init();

   Eo *obj = eo2_add(SIMPLE_CLASS, NULL);

   eo2_do(obj, simple_a_set(1), simple_b_set(2));

   int a = 0, b = 0, sum = 0;
   eo2_do(obj, a = simple_a_get(), b = simple_b_get(), sum = interface_ab_sum_get());
   fail_if(sum != a + b);

   sum = 0;
   eo2_do(obj, sum = interface_ab_sum_get(), sum = interface_ab_sum_get());
   fail_if(sum != a + b);
   eo2_do(obj, sum = interface2_ab_sum_get2(), sum = interface2_ab_sum_get2());
   fail_if(sum != a + b + 1);

   eo_unref(obj);
   eo_shutdown();
   return 0;
}


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "simple.h"
#include "interface.h"
#include "interface2.h"

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
   eo_do(obj, simple_a_get(&a), simple_b_get(&b),  interface_ab_sum_get(&sum));
   fail_if(sum != a + b);

   sum = 0;
   eo_do(obj, interface_ab_sum_get(&sum), interface_ab_sum_get(&sum));
   fail_if(sum != a + b);
   eo_do(obj, interface2_ab_sum_get2(&sum), interface2_ab_sum_get2(&sum));
   fail_if(sum != a + b + 1);

   eo_unref(obj);
   eo_shutdown();
   return 0;
}


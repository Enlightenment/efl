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

   Eo *obj;
   eo_add(obj, SIMPLE_CLASS, NULL);

   eo_do(obj, simple_a_set(obj, 1), simple_b_set(obj, 2));

   int a = 0, b = 0, sum = 0;
   eo_do(obj, a = simple_a_get(obj), b = simple_b_get(obj), sum = interface_ab_sum_get(obj));
   fail_if(sum != a + b);

   sum = 0;
   eo_do(obj, sum = interface_ab_sum_get(obj), sum = interface_ab_sum_get(obj));
   fail_if(sum != a + b);
   fprintf(stderr, "%s %s:%d\n", __func__ , __FILE__, __LINE__);
   eo_do(obj, sum = interface2_ab_sum_get2(obj), sum = interface2_ab_sum_get2(obj));
   fprintf(stderr, "%s %s:%d\n", __func__ , __FILE__, __LINE__);
   fail_if(sum != a + b + 1);

   eo_unref(obj);
   eo_shutdown();
   return 0;
}


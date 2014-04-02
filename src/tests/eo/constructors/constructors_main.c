#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_simple.h"
#include "constructors_simple2.h"
#include "constructors_simple3.h"
#include "constructors_simple4.h"
#include "constructors_simple5.h"
#include "constructors_simple6.h"
#include "constructors_simple7.h"
#include "constructors_mixin.h"

#include "../eunit_tests.h"

int my_init_count = 0;

int
main(int argc, char *argv[])
{
   int ret = 0;
   (void) argc;
   (void) argv;
   eo_init();

   Eo *obj = eo_add(SIMPLE_CLASS, NULL);

   fail_if(my_init_count != 2);

   eo_do(obj, simple_a_set(1), simple_b_set(2));

   int a = 0, b = 0;
   eo_do(obj, a = simple_a_get(), b = simple_b_get(), mixin_add_and_print(5));
   fail_if(a != 1);
   fail_if(b != 2);

   eo_unref(obj);

   fail_if(my_init_count != 0);

   obj = eo_add(SIMPLE2_CLASS, NULL);
   fail_if(obj);

   obj = eo_add(SIMPLE3_CLASS, NULL);
   fail_if(obj);

   my_init_count = 0;
   obj = eo_add(SIMPLE4_CLASS, NULL);

   fail_if(my_init_count != 2);

   eo_unref(obj);

   fail_if(my_init_count != 0);

   obj = eo_add(SIMPLE5_CLASS, NULL);
   fail_if(!obj);
   eo_unref(obj);

   obj = eo_add(SIMPLE6_CLASS, NULL);
   fail_if(!obj);
   eo_unref(obj);

   obj = eo_add(SIMPLE7_CLASS, NULL);
   fail_if(obj);

   my_init_count = 0;
   obj = eo_add_custom(SIMPLE_CLASS, NULL, simple_constructor(7));
   fail_if(!obj);

   fail_if(my_init_count != 2);
   eo_do(obj, a = simple_a_get());
   fail_if(a != 7);

   eo_unref(obj);

   eo_shutdown();
   return ret;
}


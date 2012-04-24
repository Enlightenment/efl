#include "Eobj.h"
#include "simple.h"
#include "simple2.h"
#include "simple3.h"
#include "simple4.h"
#include "simple5.h"
#include "simple6.h"
#include "mixin.h"

#include "../eunit_tests.h"

int my_init_count = 0;

int
main(int argc, char *argv[])
{
   int ret = 0;
   (void) argc;
   (void) argv;
   eobj_init();

   Eobj *obj = eobj_add(SIMPLE_CLASS, NULL);

   fail_if(my_init_count != 2);

   eobj_do(obj, simple_a_set(1), simple_b_set(2));

   int a, b;
   eobj_do(obj, simple_a_get(&a), simple_b_get(&b),  mixin_add_and_print(5));

   eobj_unref(obj);

   fail_if(my_init_count != 0);

   obj = eobj_add(SIMPLE2_CLASS, NULL);
   fail_if(obj);

   obj = eobj_add(SIMPLE3_CLASS, NULL);
   fail_if(obj);

   my_init_count = 0;
   obj = eobj_add(SIMPLE4_CLASS, NULL);

   fail_if(my_init_count != 2);

   eobj_unref(obj);

   fail_if(my_init_count != 0);

   obj = eobj_add(SIMPLE5_CLASS, NULL);
   eobj_unref(obj);

   obj = eobj_add(SIMPLE6_CLASS, NULL);
   eobj_unref(obj);

   eobj_shutdown();
   return ret;
}


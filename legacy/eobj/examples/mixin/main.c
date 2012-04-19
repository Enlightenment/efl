#include "Eobj.h"
#include "simple.h"
#include "mixin.h"

#include "../eunit_tests.h"

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eobj_init();

   Eobj *obj = eobj_add(SIMPLE_CLASS, NULL);

   eobj_do(obj, SIMPLE_A_SET(1), SIMPLE_B_SET(2));

   int a, b, sum = 0;
   eobj_do(obj, SIMPLE_A_GET(&a), SIMPLE_B_GET(&b),  MIXIN_AB_SUM_GET(&sum));
   fail_if(sum != a + b + 2); /* 2 for the two mixins... */

   eobj_unref(obj);
   eobj_shutdown();
   return 0;
}


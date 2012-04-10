#include "Eobj.h"
#include "simple.h"
#include "mixin.h"

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eobj_init();

   Eobj *obj = eobj_add(SIMPLE_CLASS, NULL);

   eobj_do(obj, SIMPLE_A_SET(1), SIMPLE_B_SET(2));

   int a, b;
   eobj_do(obj, SIMPLE_A_GET(&a), SIMPLE_B_GET(&b),  MIXIN_ADD_AND_PRINT(5));

   eobj_unref(obj);
   eobj_shutdown();
   return 0;
}


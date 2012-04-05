#include "eobj.h"
#include "simple.h"
#include "simple2.h"
#include "simple3.h"
#include "simple4.h"
#include "mixin.h"

int my_init_count = 0;

int
main(int argc, char *argv[])
{
   int ret = 0;
   (void) argc;
   (void) argv;
   eobj_init();

   Eobj *obj = eobj_add(SIMPLE_CLASS, NULL);

   if (my_init_count != 2)
     {
        printf("Error! my_init_count == %d\n", my_init_count);
        ret = 1;
     }

   eobj_do(obj, SIMPLE_A_SET(1), SIMPLE_B_SET(2));

   int a, b;
   eobj_do(obj, SIMPLE_A_GET(&a), SIMPLE_B_GET(&b),  MIXIN_ADD_AND_PRINT(5));

   eobj_unref(obj);

   if (my_init_count != 0)
     {
        printf("Error! my_init_count == %d\n", my_init_count);
        ret = 1;
     }

   obj = eobj_add(SIMPLE2_CLASS, NULL);
   if (obj)
     {
        printf("Error! obj is supposed to be NULL.\n");
        ret = 1;
     }

   obj = eobj_add(SIMPLE3_CLASS, NULL);
   if (obj)
     {
        printf("Error! obj is supposed to be NULL.\n");
        ret = 1;
     }

   my_init_count = 0;
   obj = eobj_add(SIMPLE4_CLASS, NULL);

   if (my_init_count != 2)
     {
        printf("Error! my_init_count == %d\n", my_init_count);
        ret = 1;
     }

   eobj_unref(obj);

   if (my_init_count != 0)
     {
        printf("Error! my_init_count == %d\n", my_init_count);
        ret = 1;
     }

   eobj_shutdown();
   return ret;
}


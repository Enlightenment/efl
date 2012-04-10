#include "Eobj.h"
#include "simple.h"
#include "inherit.h"

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eobj_init();

   Eobj *obj = eobj_add(INHERIT_CLASS, NULL);

   eobj_do(obj, SIMPLE_A_SET(1), INHERIT_PROT_PRINT());

   Simple_Public_Data *pd = eobj_data_get(obj, SIMPLE_CLASS);
   printf("Pub: %d\n", pd->public_x2);

   eobj_unref(obj);
   eobj_shutdown();
   return 0;
}


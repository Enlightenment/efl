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

   eobj_do(obj, simple_a_set(1), inherit_prot_print());

   Simple_Public_Data *pd = eobj_data_get(obj, SIMPLE_CLASS);
   printf("Pub: %d\n", pd->public_x2);

   eobj_unref(obj);
   eobj_shutdown();
   return 0;
}


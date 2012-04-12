#include "Eobj.h"
#include "simple.h"
#include "inherit.h"
#include "inherit2.h"
#include "inherit3.h"

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eobj_init();

   Eobj *obj = eobj_add(INHERIT2_CLASS, NULL);

   eobj_do(obj, SIMPLE_A_SET(1));
   Simple_Public_Data *pd = eobj_data_get(obj, SIMPLE_CLASS);
   if (pd->a != 2)
     {
        return 1;
     }
   eobj_unref(obj);

   obj = eobj_add(INHERIT3_CLASS, NULL);

   eobj_do(obj, SIMPLE_A_SET(1));
   pd = eobj_data_get(obj, SIMPLE_CLASS);
   if (pd->a != 3)
     {
        return 1;
     }

   eobj_unref(obj);

   eobj_shutdown();
   return 0;
}


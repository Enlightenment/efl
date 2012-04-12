#include "Eobj.h"
#include "simple.h"
#include "inherit.h"
#include "inherit2.h"
#include "inherit3.h"

#include "../eunit_tests.h"

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eobj_init();

   Eobj *obj = eobj_add(INHERIT2_CLASS, NULL);

   eobj_do(obj, SIMPLE_A_SET(1));
   Simple_Public_Data *pd = eobj_data_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 2);

   eobj_unref(obj);

   obj = eobj_add(INHERIT3_CLASS, NULL);

   eobj_do(obj, SIMPLE_A_SET(1));
   pd = eobj_data_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 3);

   eobj_unref(obj);

   obj = eobj_add(INHERIT2_CLASS, NULL);
   eobj_do(obj, INHERIT2_PRINT());
   eobj_unref(obj);

   obj = eobj_add(SIMPLE_CLASS, NULL);
   fail_if(eobj_do(obj, INHERIT2_PRINT2()));
   eobj_unref(obj);

   eobj_shutdown();
   return 0;
}


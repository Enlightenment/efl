#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "access_simple.h"
#include "access_inherit.h"

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   efl_object_init();

   Eo *obj = efl_add_ref(INHERIT_CLASS, NULL);

   simple_a_set(obj, 1);
   inherit_prot_print(obj);

   Simple_Public_Data *pd = efl_data_scope_get(obj, SIMPLE_CLASS);
   printf("Pub: %d\n", pd->public_x2);

   efl_unref(obj);
   efl_object_shutdown();
   return 0;
}


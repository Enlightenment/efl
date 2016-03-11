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
   eo_init();

   Eo *obj = eo_add(INHERIT_CLASS, NULL);

   simple_a_set(obj, 1);
   inherit_prot_print(obj);

   Simple_Public_Data *pd = eo_data_scope_get(obj, SIMPLE_CLASS);
   printf("Pub: %d\n", pd->public_x2);

   eo_unref(obj);
   eo_shutdown();
   return 0;
}


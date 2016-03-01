#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "simple_simple.h"

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eo_init();

   Eo *obj = eo_add(SIMPLE_CLASS, NULL);

   simple_a_set(obj, 4);

   int a = 0, a2 = 0, a3 = 0;

   a = simple_a_get(obj);
   a3 = interface_a_power_3_get(obj);
   a2 = mixin_a_square_get(obj);

   printf("Got %d %d %d\n", a, a2, a3);

   eo_unref(obj);
   eo_shutdown();
   return 0;
}


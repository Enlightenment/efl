#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "simple.h"

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eo_init();

   Eo *obj = eo_add(SIMPLE_CLASS, NULL);

   eo_do(obj, simple_a_set(4));

   int a = 0, a2 = 0, a3 = 0;

   eo_do(obj, simple_a_get(&a),
              interface_a_power_3_get(&a3),
              mixin_a_square_get(&a2));

   printf("Got %d %d %d\n", a, a2, a3);

   eo_unref(obj);
   eo_shutdown();
   return 0;
}


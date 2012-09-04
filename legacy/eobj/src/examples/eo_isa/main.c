#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "simple.h"
#include "complex.h"

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eo_init();

   Eo *simpleobj = eo_add(SIMPLE_CLASS, NULL);
   Eo *complexobj = eo_add(COMPLEX_CLASS, NULL);

   printf("Simple: isa-simple:%d isa-complex:%d isa-mixin:%d isa-interface:%d\n",
         eo_isa(simpleobj, SIMPLE_CLASS),
         eo_isa(simpleobj, COMPLEX_CLASS),
         eo_isa(simpleobj, MIXIN_CLASS),
         eo_isa(simpleobj, INTERFACE_CLASS));
   printf("Complex: isa-simple:%d isa-complex:%d isa-mixin:%d isa-interface:%d\n",
         eo_isa(complexobj, SIMPLE_CLASS),
         eo_isa(complexobj, COMPLEX_CLASS),
         eo_isa(complexobj, MIXIN_CLASS),
         eo_isa(complexobj, INTERFACE_CLASS));

   eo_unref(simpleobj);
   eo_unref(complexobj);

   eo_shutdown();
   return 0;
}


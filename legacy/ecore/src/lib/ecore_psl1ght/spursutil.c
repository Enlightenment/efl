#include "spursutil.h"
#include <sys/spu.h>

#define SPURS_DEFAULT_PREFIX_NAME "spursutil"

Spurs *
initSpurs(const char *prefix_name)
{
   Spurs *spurs = NULL;
   SpursAttribute attributeSpurs;
   int ret;
   int i;

   ret = sysSpuInitialize (6, 0);
   printf ("sysSpuInitialize return %d\n", ret);

   /* initialize spurs */
   printf ("Initializing spurs\n");
   spurs = (void *)memalign (SPURS_ALIGN, sizeof (Spurs));
   printf ("Initializing spurs attribute\n");

   ret = spursAttributeInitialize (&attributeSpurs, 5, 250, 1000, true);
   if (ret)
     {
        printf ("error : spursAttributeInitialize failed  %x\n", ret);
        goto error;
     }

   printf ("Setting name prefix\n");
   if (!prefix_name)
     prefix_name = SPURS_DEFAULT_PREFIX_NAME;
   ret = spursAttributeSetNamePrefix (&attributeSpurs,
                                      prefix_name, strlen (prefix_name));
   if (ret)
     {
        printf ("error : spursAttributeInitialize failed %x\n", ret);
        goto error;
     }

   printf ("Initializing with attribute\n");
   ret = spursInitializeWithAttribute (spurs, &attributeSpurs);
   if (ret)
     {
        printf ("error: spursInitializeWithAttribute failed  %x\n", ret);
        goto error;
     }

   return spurs;

error:
   if (spurs)
     free (spurs);
   return NULL;
}

void
endSpurs(Spurs *spurs)
{
   spursFinalize (spurs);
   free (spurs);
}


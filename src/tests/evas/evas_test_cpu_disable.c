/* Evas takes its cpu features from eina, so EINA_CPU_DISABLE reaches the
 * evas software paths too. Evas reads the features once per process, at its
 * first init, so this runs as its own program with the variable set before
 * that rather than inside evas_suite. */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include <Evas.h>

#include "evas_common_private.h"

int
main(void)
{
   int mmx = -1, sse = -1, sse2 = -1;

   setenv("EINA_CPU_DISABLE", "mmx,sse", 1);
   evas_init();
   evas_common_cpu_can_do(&mmx, &sse, &sse2);
   evas_shutdown();

   if (mmx || sse)
     {
        fprintf(stderr, "EINA_CPU_DISABLE=mmx,sse left evas with mmx=%d sse=%d\n",
                mmx, sse);
        return 1;
     }
   return 0;
}

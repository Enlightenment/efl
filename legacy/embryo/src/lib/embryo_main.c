#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <time.h>

#include "Embryo.h"
#include "embryo_private.h"

static Embryo_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Embryo_Version *embryo_version = &_version;

static int _embryo_init_count = 0;

/*** EXPORTED CALLS ***/

EAPI int
embryo_init(void)
{
   if (++_embryo_init_count != 1)
     return _embryo_init_count;

   srand(time(NULL));

   return _embryo_init_count;
}

EAPI int
embryo_shutdown(void)
{
   if (--_embryo_init_count != 0)
     return _embryo_init_count;

   return _embryo_init_count;
}

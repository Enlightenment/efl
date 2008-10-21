/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <time.h>

#include "Embryo.h"
#include "embryo_private.h"

static int _embryo_init_count = 0;

/*** EXPORTED CALLS ***/

/**
 * @defgroup Embryo_Library_Group Library Maintenance Functions
 *
 * Functions that start up and shutdown the Embryo library.
 */

/**
 * Initialises the Embryo library.
 * @return  The number of times the library has been initialised without being
 *          shut down.
 * @ingroup Embryo_Library_Group
 */
EAPI int
embryo_init(void)
{
   _embryo_init_count++;
   if (_embryo_init_count > 1) return _embryo_init_count;

   srand(time(NULL));

   return _embryo_init_count;
}

/**
 * Shuts down the Embryo library.
 * @return  The number of times the library has been initialised without being
 *          shutdown.
 * @ingroup Embryo_Library_Group
 */
EAPI int
embryo_shutdown(void)
{
   _embryo_init_count--;
   if (_embryo_init_count > 0) return _embryo_init_count;

   return _embryo_init_count;
}

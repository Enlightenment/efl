#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>

#include "embryo_cc_prefix.h"

/* local subsystem functions */

/* local subsystem globals */

static Eina_Prefix *pfx = NULL;

/* externally accessible functions */
int
e_prefix_determine(char *argv0)
{
   if (pfx) return 1;
   eina_init();
   pfx = eina_prefix_new(argv0, e_prefix_determine,
                         "EMBRYO", "embryo", "include/default.inc",
                         PACKAGE_BIN_DIR,
                         PACKAGE_LIB_DIR,
                         PACKAGE_DATA_DIR,
                         PACKAGE_DATA_DIR);
   if (!pfx) return 0;
   return 1;
}

void
e_prefix_shutdown(void)
{
   eina_prefix_free(pfx);
   pfx = NULL;
   eina_shutdown();
}

const char *
e_prefix_get(void)
{
   return eina_prefix_get(pfx);
}

const char *
e_prefix_bin_get(void)
{
   return eina_prefix_bin_get(pfx);
}

const char *
e_prefix_data_get(void)
{
   return eina_prefix_data_get(pfx);
}

const char *
e_prefix_lib_get(void)
{
   return eina_prefix_lib_get(pfx);
}

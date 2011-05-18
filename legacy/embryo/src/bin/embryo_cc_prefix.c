#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h>
#include <fnmatch.h>
#include <limits.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>

#ifndef _MSC_VER
# include <unistd.h>
# include <sys/param.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>      /* for realpath */
#else
# include <pwd.h>
# include <grp.h>
# include <glob.h>
#endif /* ! HAVE_EVIL */

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

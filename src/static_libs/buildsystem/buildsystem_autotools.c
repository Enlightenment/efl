#ifdef _WIN32
# define BIN_EXT ".exe"
#else
# define BIN_EXT ""
#endif

#include "config.h"

#include <stdio.h>
#include <Eina.h>

Eina_Bool
bs_mod_get(char *path, size_t maxlen, const char *subsystem, const char *mod_name)
{
   if (!getenv("EFL_RUN_IN_TREE")) return EINA_FALSE;

   snprintf(path, maxlen, PACKAGE_BUILD_DIR"/src/modules/%s/%s/.libs/module"SHARED_LIB_SUFFIX, subsystem, mod_name);

   return EINA_TRUE;
}

Eina_Bool
bs_mod_dir_get(char *path, size_t maxlen, const char *subsystem, const char *mod_name)
{
   if (!getenv("EFL_RUN_IN_TREE")) return EINA_FALSE;

   snprintf(path, maxlen, PACKAGE_BUILD_DIR"/src/modules/%s/%s/.libs", subsystem, mod_name);

   return EINA_TRUE;
}

Eina_Bool
bs_binary_get(char *path, size_t maxlen, const char *subsystem, const char *bin_name)
{
   if (!getenv("EFL_RUN_IN_TREE")) return EINA_FALSE;

   snprintf(path, maxlen, PACKAGE_BUILD_DIR"/src/bin/%s/%s"BIN_EXT, subsystem, bin_name);

   return EINA_TRUE;
}

Eina_Bool
bs_data_path_get(char *path, size_t maxlen, const char *subsystem, const char *file)
{
   if (!getenv("EFL_RUN_IN_TREE")) return EINA_FALSE;

   snprintf(path, maxlen, PACKAGE_SRC_DIR"/data/%s/%s", subsystem, file);

   return EINA_TRUE;
}

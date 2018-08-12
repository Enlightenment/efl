#include "config.h"

#include <stdio.h>
#include <Eina.h>

Eina_Bool
bs_mod_get(char *path, size_t maxlen, const char *subsystem, const char *mod_name)
{
   if (!getenv("EFL_RUN_IN_TREE")) return EINA_FALSE;

   // there is a name conflict between a ethumb module and the emotion library,
   // this causes the module to link to itself, instead of the library
   if (!strcmp(subsystem, "ethumb"))
      snprintf(path, maxlen, PACKAGE_BUILD_DIR"/src/modules/%s/%s/lib%s_el"MOD_SUFFIX, subsystem, mod_name, mod_name);
   else
      snprintf(path, maxlen, PACKAGE_BUILD_DIR"/src/modules/%s/%s/lib%s"MOD_SUFFIX, subsystem, mod_name, mod_name);

   return EINA_TRUE;
}

Eina_Bool
bs_mod_dir_get(char *path, size_t maxlen, const char *subsystem, const char *mod_name)
{
   if (!getenv("EFL_RUN_IN_TREE")) return EINA_FALSE;

   snprintf(path, maxlen, PACKAGE_BUILD_DIR"/src/modules/%s/%s/", subsystem, mod_name);

   return EINA_TRUE;
}

Eina_Bool
bs_binary_get(char *path, size_t maxlen, const char *subsystem, const char *bin_name)
{
   if (!getenv("EFL_RUN_IN_TREE")) return EINA_FALSE;

   snprintf(path, maxlen, PACKAGE_BUILD_DIR"/src/bin/%s/%s"EXE_SUFFIX, subsystem, bin_name);

   return EINA_TRUE;
}

Eina_Bool
bs_data_path_get(char *path, size_t maxlen, const char *subsystem, const char *file)
{
   if (!getenv("EFL_RUN_IN_TREE")) return EINA_FALSE;

   snprintf(path, maxlen, PACKAGE_SRC_DIR"/data/%s/%s", subsystem, file);

   return EINA_TRUE;
}

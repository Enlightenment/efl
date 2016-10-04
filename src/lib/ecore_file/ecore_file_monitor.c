#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_file_private.h"

int
ecore_file_monitor_init(void)
{
   if (ecore_file_monitor_backend_init())
     return 1;
   return 0;
}

void
ecore_file_monitor_shutdown(void)
{
   ecore_file_monitor_backend_shutdown();
}

EAPI Ecore_File_Monitor *
ecore_file_monitor_add(const char           *path,
                       Ecore_File_Monitor_Cb func,
                       void                 *data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(path[0] == '\0', NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);

   return ecore_file_monitor_backend_add(path, func, data);
}

EAPI void
ecore_file_monitor_del(Ecore_File_Monitor *em)
{
   if (!em) return;
   EINA_SAFETY_ON_NULL_RETURN(em);
   ecore_file_monitor_backend_del(em);
}

EAPI const char *
ecore_file_monitor_path_get(Ecore_File_Monitor *em)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(em, NULL);
   return em->path;
}

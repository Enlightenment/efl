#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <evil_private.h>

#include <stdlib.h>
#include <string.h> /* strlen */

EVIL_API char *
evil_basename(char *path)
{
   // Expected from posix
   if ((path == NULL) || (*path == '\0') ) return ".";

   // Remove trailing '\\'
   unsigned int len = strlen(path);
   if ((len > 1) && ((path[len -1] == '/') || (path[len -1] == '\\')))
     path[len -1] = '\0';

   // Search for the last slash
   while(--len)
     if ((path[len] == '/') || (path[len] == '\\'))
       break;

   if(!len) return path;
   else return &path[len +1];
}

EVIL_API char *
evil_dirname(char *path)
{
   // Expected from posix
   if ((path == NULL) || (*path == '\0') ) return ".";

   // Search for the last slash ignoring trailing '\\'
   unsigned int len = strlen(path);
   while(--len)
     if ((path[len] == '/') || (path[len] == '\\'))
       break;
   path[len] = '\0';

   return path;
}

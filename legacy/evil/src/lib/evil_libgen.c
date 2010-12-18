#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>

#include "Evil.h"

char _evil_basename_buf[PATH_MAX];
char _evil_dirname_buf[PATH_MAX];

char *
evil_basename(char *path)
{
   char *p1;
   char *p2;
   size_t length;

   /* path must begin by "?:\" or "?:/" */
   if ((!path) ||
       (strlen(path) <= 3) ||
       ((path[0] < 'a' || path[0] > 'z') && (path[0] < 'A' || path[0] > 'Z')) ||
       (path[1] != ':') ||
       ((path[2] != '/') && (path[2] != '\\')))
     {
        memcpy(_evil_basename_buf, "C:\\", 4);
        return _evil_basename_buf;
     }

   /* '/' --> '\\' */
   length = strlen(path);
   p1 = strdup(path);
   if (!p1)
     {
        memcpy(_evil_basename_buf, "C:\\", 4);
        return _evil_basename_buf;
     }
   p2 = p1;
   while (*p2)
     {
        if (*p2 == '/') *p2 = '\\';
        p2++;
     }

   /* remove trailing backslashes */
   p2 = p1 + (length - 1);
   if (*p2 == '\\')
     {
       while (*p2 == '\\')
         p2--;
     }
   *(p2 + 1) = '\0';

   p2 = strrchr(p1, '\\');
   memcpy(_evil_basename_buf, p2 + 1, (p1 + length + 1) - p2);

   free(p1);

   return _evil_basename_buf;
}

char *
evil_dirname(char *path)
{
   char *p1;
   char *p2;
   size_t length;

   /* path must begin by "?:\" or "?:/" */
   if ((!path) ||
       (strlen(path) <= 3) ||
       ((path[0] < 'a' || path[0] > 'z') && (path[0] < 'A' || path[0] > 'Z')) ||
       (path[1] != ':') ||
       ((path[2] != '/') && (path[2] != '\\')))
     {
        memcpy(_evil_dirname_buf, "C:\\", 4);
        return _evil_dirname_buf;
     }

   /* '/' --> '\\' */
   length = strlen(path);
   p1 = strdup(path);
   if (!p1)
     {
        memcpy(_evil_dirname_buf, "C:\\", 4);
        return _evil_dirname_buf;
     }
   p2 = p1;
   while (*p2)
     {
        if (*p2 == '/') *p2 = '\\';
        p2++;
     }

   /* remove trailing backslashes */
   p2 = p1 + (length - 1);
   if (*p2 == '\\')
     {
       while (*p2 == '\\')
         p2--;
     }
   *(p2 + 1) = '\0';

   p2 = strrchr(p1, '\\');
   *p2 = '\0';
   memcpy(_evil_dirname_buf, p1, strlen(p1) + 1);

   free(p1);

   return _evil_dirname_buf;
}

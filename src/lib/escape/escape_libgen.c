#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <malloc.h>

#include "Escape.h"

static char _escape_basename_buf[PATH_MAX];
static char _escape_dirname_buf[PATH_MAX];

char *
escape_basename(char *path)
{
   char *p1;
   char *p2;
   size_t length;

   /* path must begin by "?:\" or "?:/" */
   if (!path || strlen(path) <= 1)
     {
        memcpy(_escape_basename_buf, path, PATH_MAX);
        return _escape_basename_buf;
     }

   length = strlen(path);
   p1 = strdup(path);
   if (!p1)
     {
        memcpy(_escape_basename_buf, path, PATH_MAX);
        return _escape_basename_buf;
     }

   /* remove trailing backslashes */
   p2 = p1 + (length - 1);
   if (*p2 == '/')
     {
        while (*p2 == '/')
          p2--;
     }
   *(p2 + 1) = '\0';

   p2 = strrchr(p1, '/');
   memcpy(_escape_basename_buf, p2 + 1, (p1 + length + 1) - p2);

   free(p1);

   return _escape_basename_buf;
}

char *
escape_dirname(char *path)
{
   char *p1;
   char *p2;
   size_t length;

   /* path must begin by "?:\" or "?:/" */
   if (!path || strlen(path) <= 1)
     {
        memcpy(_escape_dirname_buf, path, PATH_MAX);
        return _escape_dirname_buf;
     }

   length = strlen(path);
   p1 = strdup(path);
   if (!p1)
     {
        memcpy(_escape_dirname_buf, path, PATH_MAX);
        return _escape_dirname_buf;
     }
   /* remove trailing backslashes */
   p2 = p1 + (length - 1);
   if (*p2 == '/')
     {
        while (*p2 == '/')
          p2--;
     }
   *(p2 + 1) = '\0';

   p2 = strrchr(p1, '/');
   *p2 = '\0';
   memcpy(_escape_dirname_buf, p1, strlen(p1) + 1);

   free(p1);

   return _escape_dirname_buf;
}


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>

#include "Evil.h"

char _evil_basename_buf[PATH_MAX];

char *
evil_basename(char *path)
{
   char *p1;
   char *p2;
   size_t length;

   if (!path || !*path)
     {
        memcpy(_evil_basename_buf, ".", 2);
        return _evil_basename_buf;
     }
   
   length = strlen(path);
   p1 = strdup(path);
   if (!p1) return NULL;
   p2 = p1;
   while (p2)
     {
        if (*p2 == '/') *p2 = '\\';
        p2++;
     }
   if (p1[length - 1] =='\\') p1[--length] = '\0';

   p2 = strrchr(p1, '\\');
   if (!p2) memcpy(_evil_basename_buf, p1, length + 1);
   else memcpy(_evil_basename_buf, p2 + 1, (p1 + length + 1) - p2);

   free(p1);

   return _evil_basename_buf;
}

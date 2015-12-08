#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <direct.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "evil_macro.h"
#include "evil_stdio.h"
#include "evil_private.h"

#undef rename

int 
evil_rename(const char *src, const char* dst)
{
   struct stat st;

   if (stat(dst, &st) < 0)
        return rename(src, dst);

   if (stat(src, &st) < 0)
        return -1;

   if (S_ISDIR(st.st_mode))
     {
        rmdir(dst);
        return rename(src, dst);
     }

   if (S_ISREG(st.st_mode))
     {
        unlink(dst);
        return rename(src, dst);
     }

   return -1;
}

int
evil_mkdir(const char *dirname, mode_t mode EVIL_UNUSED)
{
   return _mkdir(dirname);
}


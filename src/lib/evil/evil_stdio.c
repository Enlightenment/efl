#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <direct.h>

#include "evil_private.h"

#undef rename

int
evil_rename(const char *src, const char* dst)
{
   DWORD res;

   res = GetFileAttributes(dst);
   if ((res != 0xffffffff) && (res & FILE_ATTRIBUTE_DIRECTORY))
     {
        if (!RemoveDirectory(dst))
          return -1;
     }

   return MoveFileEx(src, dst, MOVEFILE_REPLACE_EXISTING) ? 0 : -1;
}

int
evil_mkdir(const char *dirname, mode_t mode EVIL_UNUSED)
{
   return _mkdir(dirname);
}


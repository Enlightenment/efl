#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#undef WIN32_LEAN_AND_MEAN

#include <stdlib.h>
#include <stdio.h>

#ifndef __CEGCC__
# include <errno.h>
# include <sys/locking.h>
# include <io.h>
# include <shlobj.h>
# include <objidl.h>
#else
# include <sys/syslimits.h>
#endif /* __CEGCC__ */

#ifdef UNICODE
# include <wchar.h>
#endif /* UNICODE */

#include <sys/types.h>
#include <sys/timeb.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE___ATTRIBUTE__
# define __UNUSED__ __attribute__((unused))
#else
# define __UNUSED__
#endif /* HAVE___ATTRIBUTE__ */

#include "Evil.h"


#ifndef __CEGCC__

int fcntl(int fd, int cmd, ...)
{
   va_list va;
   HANDLE  h;
   int     res = -1;

   va_start (va, cmd);

   h = (HANDLE)_get_osfhandle(fd);
   if (h == INVALID_HANDLE_VALUE)
     return -1;

   if (cmd == F_SETFD)
     {
        long flag;

        flag = va_arg(va, long);
        if (flag == FD_CLOEXEC)
          {
             if (SetHandleInformation(h, HANDLE_FLAG_INHERIT, 0))
               res = 0;
          }
     }
   else if ((cmd == F_SETLK) || (cmd == F_SETLKW))
     {
        struct flock fl;
        off_t        length = 0;
        long         pos;

        fl = va_arg(va, struct flock);

        if (fl.l_len == 0)
          {
             length = _lseek(fd, 0L, SEEK_END);
             if (length != -1L)
               res = 0;
          }
        fl.l_len = length - fl.l_start - 1;

        pos = _lseek(fd, fl.l_start, fl.l_whence);
        if (pos != -1L)
          res = 0;

        if ((fl.l_type == F_RDLCK) || (fl.l_type == F_WRLCK))
          {
             if (cmd == F_SETLK)
               res = _locking(fd, _LK_NBLCK, fl.l_len); /* if cannot be locked, we return immediatly */
             else /* F_SETLKW */
               res = _locking(fd, _LK_LOCK, fl.l_len); /* otherwise, we try several times */
          }

        if (fl.l_type == F_UNLCK)
          res = _locking(fd, _LK_UNLCK, fl.l_len);
     }

   va_end(va);

   return res;
}

#endif /* ! __CEGCC__ */

static char *
replace(char *prev, char *value)
{
   if (value == NULL)
     return prev;

   if (prev)
     free (prev);
   return strdup (value);
}

char *
nl_langinfo(nl_item index)
{
   static char *result = NULL;
   static char *nothing = "";

   switch (index)
     {
      case CODESET:
        {
           char *p;
           result = replace(result, setlocale(LC_CTYPE, NULL));
           if ((p = strrchr(result, '.' )) == NULL)
             return nothing;

           if ((++p - result) > 2)
             strcpy(result, "cp");
           else
             *result = '\0';
           strcat(result, p);

           return result;
        }
     }

   return nothing;
}

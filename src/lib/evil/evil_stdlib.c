#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include "evil_macro.h"
#include "evil_stdlib.h"
#include "evil_private.h"

/*
 * Environment variable related functions
 *
 * int setenv (const char *name, const char *value, int overwrite);
 * void unsetenv (const char *name);
 *
 */

int
setenv(const char *name,
       const char *value,
       int         overwrite)
{
   char  *old_name;
   char  *str;
   size_t length;
   int    res;

   if (!name || !*name)
     return -1;

   /* if '=' is found, return EINVAL */
   if (strchr (name, '='))
     {
        errno = EINVAL;
        return -1;
     }

   /* if name is already set and overwrite is 0, we exit with success */
   old_name = getenv(name);
   if (!overwrite && old_name)
     return 0;

   length = value ? strlen(value) : 0;
   length += strlen(name) + 2;
   str = (char *)malloc(length);
   if (!str)
     {
        errno = ENOMEM;
        return -1;
     }
   if (!value)
     sprintf(str, "%s=", name);
   else
     sprintf(str, "%s=%s", name, value);
   res = _putenv(str);
   free(str);

   return res;
}

int
unsetenv(const char *name)
{
   return setenv(name, NULL, 1);
}


/*
 * Files related functions
 *
 */
static int
_mkstemp_init(char *__template, char **suffix, size_t *length, DWORD *val,
              size_t suffixlen)
{
   *length = strlen(__template);
   if ((*length < (6 + suffixlen))
       || (strncmp(__template + *length - 6 - suffixlen, "XXXXXX", 6) != 0))
     {
        errno = EINVAL;
        return 0;
     }

   *suffix = __template + *length - 6 - suffixlen;

   *val = GetTickCount();
   *val += GetCurrentProcessId();

   return 1;
}

static int
_mkstemp(char *suffix, int val)
{
   const char lookup[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
   DWORD v = val;

   suffix[0] = lookup[v % 62];
   v /= 62;
   suffix[1] = lookup[v % 62];
   v /= 62;
   suffix[2] = lookup[v % 62];
   v /= 62;
   suffix[3] = lookup[v % 62];
   v /= 62;
   suffix[4] = lookup[v % 62];
   v /= 62;
   suffix[5] = lookup[v % 62];
   v /= 62;

   val += 7777;

   return val;
}

EAPI char *
mkdtemp(char *__template)
{
   char      *suffix;
   DWORD      val;
   size_t     length;
   int        i;

   if (!__template)
     {
        errno = EINVAL;
        return NULL;
     }

   if (!_mkstemp_init(__template, &suffix, &length, &val, 0))
     return NULL;

   for (i = 0; i < 32768; i++)
     {
        val = _mkstemp(suffix, val);

        if (mkdir(__template) == 0)
          return __template;

        if (errno == EFAULT ||
            errno == ENOSPC ||
            errno == ENOMEM ||
            errno == ENOENT ||
            errno == ENOTDIR ||
            errno == EPERM ||
            errno == EROFS)
          return NULL;
     }

   errno = EEXIST;
   return NULL;
}

int
mkstemps(char *__template, int suffixlen)
{
   char      *suffix;
   DWORD      val;
   size_t     length;
   int        i;

   if (!__template || (suffixlen < 0))
     return 0;

   if (!_mkstemp_init(__template, &suffix, &length, &val, (size_t) suffixlen))
     return -1;

   for (i = 0; i < 32768; i++)
     {
        int fd;

        val = _mkstemp(suffix, val);

        fd = _open(__template, _O_RDWR | _O_BINARY | _O_CREAT | _O_EXCL, _S_IREAD | _S_IWRITE);
        if (fd >= 0)
          return fd;
     }

   errno = EEXIST;
   return -1;
}

int
mkstemp(char *__template)
{
   return mkstemps(__template, 0);
}

char *
realpath(const char *file_name, char *resolved_name)
{
   char *retname = NULL;  /* we will return this, if we fail */

   /* SUSv3 says we must set `errno = EINVAL', and return NULL,
    * if `name' is passed as a NULL pointer.
    */

   if (file_name == NULL)
     errno = EINVAL;

   /* Otherwise, `name' must refer to a readable filesystem object,
    * if we are going to resolve its absolute path name.
    */

   else if (access(file_name, 4) == 0)
     {
        /* If `name' didn't point to an existing entity,
         * then we don't get to here; we simply fall past this block,
         * returning NULL, with `errno' appropriately set by `access'.
         *
         * When we _do_ get to here, then we can use `_fullpath' to
         * resolve the full path for `name' into `resolved', but first,
         * check that we have a suitable buffer, in which to return it.
         */

       if ((retname = resolved_name) == NULL)
         {
            /* Caller didn't give us a buffer, so we'll exercise the
             * option granted by SUSv3, and allocate one.
             *
             * `_fullpath' would do this for us, but it uses `malloc', and
             * Microsoft's implementation doesn't set `errno' on failure.
             * If we don't do this explicitly ourselves, then we will not
             * know if `_fullpath' fails on `malloc' failure, or for some
             * other reason, and we want to set `errno = ENOMEM' for the
             * `malloc' failure case.
             */

           retname = malloc(_MAX_PATH);
         }

       /* By now, we should have a valid buffer.
        * If we don't, then we know that `malloc' failed,
        * so we can set `errno = ENOMEM' appropriately.
        */

       if (retname == NULL)
         errno = ENOMEM;

       /* Otherwise, when we do have a valid buffer,
        * `_fullpath' should only fail if the path name is too long.
        */

       else if ((retname = _fullpath(retname, file_name, _MAX_PATH)) == NULL)
         errno = ENAMETOOLONG;
     }

   /* By the time we get to here,
    * `retname' either points to the required resolved path name,
    * or it is NULL, with `errno' set appropriately, either of which
    * is our required return condition.
    */

   return retname;
}

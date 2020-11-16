#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>

#include "evil_private.h"

/*
 * Environment variable related functions
 *
 * int setenv (const char *name, const char *value, int overwrite);
 * void unsetenv (const char *name);
 *
 */

EVIL_API int
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

EVIL_API int
unsetenv(const char *name)
{
   return setenv(name, NULL, 1);
}


/*
 * Files related functions
 *
 */

EVIL_API char *
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

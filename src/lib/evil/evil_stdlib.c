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
 * char *getenv (const char *name);
 * int putenv (const char *string);
 * int setenv (const char *name, const char *value, int overwrite);
 * void unsetenv (const char *name);
 *
 */

#ifdef _WIN32_WCE

static char _evil_stdlib_getenv_buffer[PATH_MAX];

char *
getenv(const char *name)
{
   HKEY     key;
   wchar_t *wname;
   LONG     res;
   DWORD    type;
   DWORD    disposition;
   DWORD    size = PATH_MAX;

   if (!name || !*name)
     return NULL;

   if ((res = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                             TEXT("Software\\Efl\\Environment"),
                             0, NULL,
                             REG_OPTION_VOLATILE,
                             0, NULL,
                             &key, &disposition)) != ERROR_SUCCESS)
     {
        _evil_error_display(__FUNCTION__, res);
        return NULL;
     }

   wname = evil_char_to_wchar(name);
   if (!wname)
     {
        if ((res = RegCloseKey (key)) != ERROR_SUCCESS)
          _evil_error_display(__FUNCTION__, res);
        return NULL;
     }

   if ((res = RegQueryValueEx(key, wname,
                              NULL, &type,
                              (LPBYTE)&_evil_stdlib_getenv_buffer,
                              &size)) != ERROR_SUCCESS)
     {
        if ((res = RegCloseKey (key)) != ERROR_SUCCESS)
          _evil_error_display(__FUNCTION__, res);
        free(wname);
        return NULL;
     }

   free(wname);

   if ((res = RegCloseKey (key)) != ERROR_SUCCESS)
     {
        _evil_error_display(__FUNCTION__, res);
        return NULL;
     }

   if (_evil_stdlib_getenv_buffer[0] == '\0')
     return NULL;
   else
     {
        return _evil_stdlib_getenv_buffer;
     }
}

#endif /* _WIN32_WCE */

#ifdef __MINGW32CE__

int
putenv(const char *string)
{
   char *str;
   char *egal;
   char *name;
   char *value;

   str = strdup(string);
   if (!str)
     return -1;
   egal = strchr(str, '=');
   if (!egal)
     return -1;

   value = egal + 1;
   *egal = '\0';
   name = str;
   setenv(name, value, 1);
   free(str);

   return 0;
}

#endif /* __MINGW32CE__ */



int
setenv(const char *name,
       const char *value,
       int         overwrite)
{
#ifndef __MINGW32CE__

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

#else /* __MINGW32CE__ */

   HKEY     key;
   LONG     res;
   DWORD    disposition;
   wchar_t *wname;
   char    *data;
   DWORD    size;

   if (!name || !*name)
     return -1;

   /* if '=' is found, return an error */
   if (strchr (name, '='))
     return -1;

   if ((res = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                             TEXT("Software\\Efl\\Environment"),
                             0, NULL,
                             REG_OPTION_VOLATILE,
                             0, NULL,
                             &key,
                             &disposition)) != ERROR_SUCCESS)
     {
        _evil_error_display(__FUNCTION__, res);
        return -1;
     }

   /* if name is already set and overwrite is 0, we exit with success */
   if (!overwrite && (disposition == REG_OPENED_EXISTING_KEY))
     return 0;

   wname = evil_char_to_wchar(name);
   if (!wname)
     {
        if ((res = RegCloseKey (key)) != ERROR_SUCCESS)
          _evil_error_display(__FUNCTION__, res);
        return -1;
     }

   if (value)
     {
        size = strlen(value);
        data = malloc(sizeof(char) * (size + 1));
        if (!data)
          return -1;
        memcpy((void *)data, value, size);
        data[size] = '\0';
     }
   else
     {
        size = 0;
        data = malloc(sizeof(char));
        if (!data)
          return -1;
        data[0] = '\0';
     }
   if (!data)
     return -1;

   if ((res = RegSetValueEx(key,
                            (LPCWSTR)wname,
                            0, REG_SZ,
                            (const BYTE *)data,
                            size + 1)) != ERROR_SUCCESS)
     {
        free(wname);
        _evil_error_display(__FUNCTION__, res);
        if ((res = RegCloseKey (key)) != ERROR_SUCCESS)
          _evil_error_display(__FUNCTION__, res);
        return -1;
     }

   free(data);
   free(wname);

   if ((res = RegCloseKey (key)) != ERROR_SUCCESS)
     {
        _evil_error_display(__FUNCTION__, res);
        return -1;
     }

   return 0;

#endif /* ! __MINGW32CE__ */
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

        if (mkdir(__template))
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

#ifndef __MINGW32CE__
        fd = _open(__template, _O_RDWR | _O_BINARY | _O_CREAT | _O_EXCL, _S_IREAD | _S_IWRITE);
#else /* ! __MINGW32CE__ */
        {
           FILE    *f;
           wchar_t *wtemplate;

           wtemplate = evil_char_to_wchar(__template);
           if (!wtemplate)
             return -1;
           f = _wfopen(wtemplate, L"rwb");
           free(wtemplate);
           if (!f)
             {
                errno = EEXIST;
                return -1;
             }
           fd = (int)_fileno(f);
        }
#endif /* __MINGW32CE__ */
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
#ifndef __MINGW32CE__
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
#else
   char   cwd[PATH_MAX];
   size_t l1;
   size_t l2;
   size_t l;

   if (!file_name || !resolved_name)
     return NULL;

   if (!getcwd(cwd, PATH_MAX))
     return NULL;

   l1 = strlen(cwd);
   l2 = strlen(file_name);
   l = l1 + l2 + 2;

   if (l > PATH_MAX)
     l = PATH_MAX - 1;
   memcpy(resolved_name, cwd, l1);
   resolved_name[l1] = '\\';
   memcpy(resolved_name + l1 + 1, file_name, l2);
   resolved_name[l] = '\0';

   return resolved_name;
#endif /* __MINGW32CE__ */
}

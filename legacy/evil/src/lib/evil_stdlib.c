
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#ifndef __CEGCC__
# include <io.h>
#endif /* ! __CEGCC__ */
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include "Evil.h"

/*** Local ***/

static void
_evil_stdlib_error_display(const char *fct,
                           LONG        res)
{
   char *error;

   error = evil_format_message(res);
   fprintf(stderr, "[Evil] [%s] ERROR: %s\n", fct, error);
   free(error);
}

/*
 * Environment variable related functions
 *
 * char *getenv (const char *name);
 * int putenv (const char *string);
 * int setenv (const char *name, const char *value, int overwrite);
 * void unsetenv (const char *name);
 *
 */

#if defined(__CEGCC__) || defined(__MINGW32CE__)

static char _evil_stdlib_getenv_buffer[PATH_MAX];

char *
getenv(const char *name)
{
   HKEY     key;
   wchar_t *wname;
   char    *buffer;
   LONG     res;
   DWORD    type;
   DWORD    size = PATH_MAX;

   if (!name || !*name)
     return NULL;

   if ((res = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          TEXT("Software\\Efl\\Environment"),
                          0, 0,
                          &key)) != ERROR_SUCCESS)
     {
        _evil_stdlib_error_display("getenv", res);
        return NULL;
     }

   wname = evil_char_to_wchar(name);
   if (!wname)
     {
        if ((res = RegCloseKey (key)) != ERROR_SUCCESS)
          _evil_stdlib_error_display("getenv", res);
        return NULL;
     }

   if ((res = RegQueryValueEx(key, wname,
                              NULL, &type,
                              (LPBYTE)&buffer,
                              &size)) != ERROR_SUCCESS)
     {
        _evil_stdlib_error_display("getenv", res);
        if ((res = RegCloseKey (key)) != ERROR_SUCCESS)
          _evil_stdlib_error_display("getenv", res);
        free(wname);
        return NULL;
     }

   free(wname);

   if ((res = RegCloseKey (key)) != ERROR_SUCCESS)
     {
        _evil_stdlib_error_display("getenv", res);
        return NULL;
     }

   if (buffer[0] == '\0')
     return NULL;
   else
     {
        return _evil_stdlib_getenv_buffer;
     }
}

#endif /* __CEGCC__ || __MINGW32CE__ */

#if ! defined(__CEGCC__)

int
putenv(const char *string)
{
#if ! ( defined(__CEGCC__) || defined(__MINGW32CE__) )
   return _putenv(string);
#else
#endif /* __CEGCC__ || __MINGW32CE__ */
}

#endif /* ! __CEGCC__ */



int
setenv(const char *name,
       const char *value,
       int         overwrite)
{
#if ! ( defined(__CEGCC__) || defined(__MINGW32CE__) )

   char *old_name;
   char *str;
   int   length;
   int   res;

   if (!name || !*name)
     return -1;

   /* if '=' is found, return EINVAL */
   if (strchr (name, '='))
     {
#ifdef HAVE_ERRNO_H
        errno = EINVAL;
#endif /* HAVE_ERRNO_H */
        return -1;
     }

   /* if name is already set and overwrite is 0, we exit with success */
   old_name = getenv(name);
   if (!overwrite && old_name)
     return 0;

   length = strlen(name) + strlen(value) + 2;
   str = (char *)malloc(length);
   if (!str)
     {
#ifdef HAVE_ERRNO_H
        errno = ENOMEM;
#endif /* HAVE_ERRNO_H */
        return -1;
     }
   if (!value)
     sprintf(str, "%s=", name);
   else
     sprintf(str, "%s=%s", name, value);
   res = _putenv(str);
   free(str);

   return res;

#else /* __CEGCC__ || __MINGW32CE__ */

   HKEY     key;
   LONG     res;
   DWORD    disposition;

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
        _evil_stdlib_error_display("setenv", res);
        return -1;
     }

   /* if name is already set and overwrite is 0, we exit with success */
   if (!overwrite && (disposition == REG_OPENED_EXISTING_KEY))
     return 0;

   if ((res = RegSetValueEx(key,
                            (LPCWSTR)name,
                            0, REG_SZ,
                            (const BYTE *)value,
                            strlen(value) + 1)) != ERROR_SUCCESS)
     {
        _evil_stdlib_error_display("setenv", res);
        if ((res = RegCloseKey (key)) != ERROR_SUCCESS)
          _evil_stdlib_error_display("getenv", res);
        return -1;
     }

   if ((res = RegCloseKey (key)) != ERROR_SUCCESS)
     {
        _evil_stdlib_error_display("getenv", res);
        return -1;
     }

#endif /* __CEGCC__ || __MINGW32CE__ */

   return 0;
}

#if ! defined(__CEGCC__)

int
unsetenv(const char *name)
{
   return setenv(name, NULL, 1);
}

#endif /* ! __CEGCC__ */


/*
 * Files related functions
 *
 */

int
mkstemp(char *template)
{
   const char lookup[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
   char *suffix;
   DWORD val;
   int   length;
   int   i;

   if (!template)
     return 0;

   length = strlen(template);
   if ((length < 6) ||
       (strncmp (template + length - 6, "XXXXXX", 6)))
     {
#ifdef HAVE_ERRNO_H
        errno = EINVAL;
#endif /* HAVE_ERRNO_H */
        return -1;
     }

   suffix = template + length - 6;

   val = GetTickCount();
   val += GetCurrentProcessId();

   for (i = 0; i < 32768; i++)
     {
        DWORD v;
        int fd;

        v = val;

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

        fd = _open(template, _O_RDWR | _O_BINARY | _O_CREAT | _O_EXCL, _S_IREAD | _S_IWRITE);
        if (fd >= 0)
          return fd;

        val += 7777;
     }

#ifdef HAVE_ERRNO_H
   errno = EEXIST;
#endif /* HAVE_ERRNO_H */
   return -1;
}


char *
realpath(const char *file_name, char *resolved_name)
{
#if ! ( defined(__CEGCC__) || defined(__MINGW32CE__) )
   return _fullpath(resolved_name, file_name, PATH_MAX);
#else
   int length;

   length = strlen(file_name);
   if ((length + 1) > PATH_MAX)
     length = PATH_MAX - 1;
   memcpy(resolved_name, file_name, length);
   resolved_name[length] = '\0';

   return resolved_name;
#endif /* __CEGCC__ || __MINGW32CE__ */
}

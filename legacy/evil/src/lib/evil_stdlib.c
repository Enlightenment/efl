
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#ifndef __CEGCC__
# include <io.h>
#endif /* __CEGCC__ */
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include "Evil.h"
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

#if defined(__CEGCC__) || defined(__MINGW32CE__)

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

#endif /* __CEGCC__ || __MINGW32CE__ */

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
#if ! ( defined(__CEGCC__) || defined(__MINGW32CE__) )

   char  *old_name;
   char  *str;
   size_t length;
   int    res;

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

#endif /* __CEGCC__ || __MINGW32CE__ */
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
mkstemp(char *__template)
{
   const char lookup[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
   char      *suffix;
   DWORD      val;
   size_t     length;
   int        i;

   if (!__template)
     return 0;

   length = strlen(__template);
   if ((length < 6) ||
       (strncmp (__template + length - 6, "XXXXXX", 6)))
     {
#ifdef HAVE_ERRNO_H
        errno = EINVAL;
#endif /* HAVE_ERRNO_H */
        return -1;
     }

   suffix = __template + length - 6;

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

#if ! ( defined(__CEGCC__) || defined(__MINGW32CE__) )
        fd = _open(__template, _O_RDWR | _O_BINARY | _O_CREAT | _O_EXCL, _S_IREAD | _S_IWRITE);
#else /* __CEGCC__ || __MINGW32CE__ */
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
#ifdef HAVE_ERRNO_H
                errno = EEXIST;
#endif /* HAVE_ERRNO_H */
                return -1;
             }
           fd = _fileno(f);
        }
#endif /* __CEGCC__ || __MINGW32CE__ */
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
#endif /* __CEGCC__ || __MINGW32CE__ */
}

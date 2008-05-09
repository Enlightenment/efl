#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#if defined(__CEGCC__) || defined(__MINGW32CE__)
# include <limits.h>
#endif /* __MINGW32CE__ */

#include "../Evil.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "dlfcn.h"

#if HAVE___ATTRIBUTE__
# define __UNUSED__ __attribute__((unused))
#else
# define __UNUSED__
#endif /* HAVE___ATTRIBUTE__ */


static char *dl_err = NULL;
static int dl_err_viewed = 0;

static void
get_last_error(char *desc)
{
   TCHAR *str;
   char  *str2;
   int    l1;
   int    l2;

   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                 FORMAT_MESSAGE_FROM_SYSTEM |
                 FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, GetLastError(),
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 (LPTSTR)&str, 0, NULL);

#if defined(__CEGCC__) || defined(__MINGW32CE__)
   str2 = evil_wchar_to_char(str);
#else
   str2 = str;
#endif /* ! __CEGCC__ && ! __MINGW32CE__ */

   l1 = strlen(desc);
   l2 = strlen(str2);

   if (dl_err)
     free(dl_err);

   dl_err = (char *)malloc(sizeof(char) * (l1 + l2 + 1));
   if (!dl_err)
     dl_err = strdup("not enough resource");
   else
     {
        memcpy(dl_err, desc, l1);
        memcpy(dl_err + l1, str2, l2);
        dl_err[l1 + l2] = '\0';
     }
   LocalFree(str2);
   dl_err_viewed = 0;
}

void *
dlopen(const char* path, int mode __UNUSED__)
{
   HMODULE module = NULL;

   if (!path)
     {
        module = GetModuleHandle(NULL);
        if (!module)
          get_last_error("GetModuleHandle returned: ");
     }
   else
     {
        char *new_path;
        int   l;
        int   i;

        /* according to MSDN, we must change the slash to backslash */
        l = strlen(path);
        new_path = (char *)malloc(sizeof(char) * (l + 1));
        if (!new_path)
          {
             if (dl_err)
               free(dl_err);
             dl_err = strdup("not enough resource");
             dl_err_viewed = 0;
             return NULL;
          }
        for (i = 0; i <= l; i++)
          {
             if (path[i] == '/')
               new_path[i] = '\\';
             else
               new_path[i] = path[i];
          }
#if defined(__CEGCC__) || defined(__MINGW32CE__)
        {
           wchar_t *wpath;

           wpath = evil_char_to_wchar(new_path);
           module = LoadLibrary(wpath);
           free(wpath);
        }
#else
        module = LoadLibraryEx(new_path, NULL,
                               LOAD_WITH_ALTERED_SEARCH_PATH);
#endif /* ! __CEGCC__ && ! __MINGW32CE__ */
        if (!module)
          get_last_error("LoadLibraryEx returned: ");

        free(new_path);
     }

   return module;
}

int
dlclose(void* handle)
{
   if (FreeLibrary(handle))
     return 0;
   else
     {
        get_last_error("FreeLibrary returned: ");
        return -1;
     }
}

void *
dlsym(void *handle, const char *symbol)
{
   FARPROC fp;

#if defined(__CEGCC__) || defined(__MINGW32CE__)
   {
      wchar_t *wsymbol;

      wsymbol = evil_char_to_wchar(symbol);
      fp = GetProcAddress(handle, wsymbol);
      free(wsymbol);
   }
#else
   fp = GetProcAddress(handle, symbol);
#endif /* ! __CEGCC__ && ! __MINGW32CE__ */
   if (!fp)
     get_last_error("GetProcAddress returned: ");

   return fp;
}

int
dladdr (void *addr __UNUSED__, Dl_info *info)
{
   TCHAR tpath[PATH_MAX];
   char *path;
   char *tmp;
   int   length;
   int   ret = 0;

  if (!info)
    return 0;

   ret = GetModuleFileName(GetModuleHandle(NULL), (LPTSTR)&tpath, PATH_MAX);
   if (!ret)
     return 0;

#if defined(__CEGCC__) || defined(__MINGW32CE__)
   path = evil_wchar_to_char(tpath);
#else
   path = tpath;
#endif /* ! __CEGCC__ && ! __MINGW32CE__ */

   length = strlen (path);
   if (length >= PATH_MAX)
     {
       length = PATH_MAX - 1;
       path[PATH_MAX - 1] = '\0';
     }

   /* replace '\' by '/' */
   tmp = path;
   while (*tmp)
     {
        if (*tmp == '\\') *tmp = '/';
        tmp++;
     }

   memcpy (info->dli_fname, path, length + 1);
   info->dli_fbase = NULL;
   info->dli_sname = NULL;
   info->dli_saddr = NULL;

#if defined(__CEGCC__) || defined(__MINGW32CE__)
   free (path);
#endif /* __CEGCC__ || __MINGW32CE__ */

   return 1;
}

char *
dlerror (void)
{
   if (!dl_err_viewed)
     {
        dl_err_viewed = 1;
        return dl_err;
     }
   else
     {
        if (dl_err)
          free(dl_err);
        return NULL;
     }
}

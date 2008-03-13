#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#ifdef __MINGW32CE__
# include <limits.h>
#endif /* __MINGW32CE__ */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE___ATTRIBUTE__
# define __UNUSED__ __attribute__((unused))
#else
# define __UNUSED__
#endif /* HAVE___ATTRIBUTE__ */


static char *dl_err = NULL;
static int dl_err_viewed = 0;

#if defined(__CEGCC__) || defined(__MINGW32CE__)

static wchar_t *
string_to_wchar(const char *text)
{
   wchar_t *wtext;
   int      wsize;

   wsize = MultiByteToWideChar(CP_ACP, 0, text, strlen(text) + 1, NULL, 0);
   if (wsize == 0 || wsize > ULONG_MAX/sizeof(wchar_t))
     return NULL;

   wtext = malloc(wsize * sizeof(wchar_t));
   if (wtext)
     if (!MultiByteToWideChar(CP_ACP, 0, text, strlen(text) + 1, wtext, wsize))
       return NULL;

   return wtext;
}

static char *
wchar_to_string(const wchar_t *text)
{
   char * atext;
   int    size;
   int    asize;

   size = wcslen(text) + 1;

   asize = WideCharToMultiByte(CP_ACP, 0, text, size, NULL, 0, NULL, NULL);
   if (asize == 0)
     return NULL;

   atext = (char*)malloc((asize + 1) * sizeof(char));

   if (atext)
     if (!WideCharToMultiByte(CP_ACP, 0, text, size, atext, asize, NULL, NULL))
       return NULL;
   atext[asize] = '\0';

   return atext;
}

#endif /* __CEGCC__ || __MINGW32CE__ */

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
   str2 = wchar_to_string(str);
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

           wpath = string_to_wchar(new_path);
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

      wsymbol = string_to_wchar(symbol);
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

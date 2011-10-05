#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>

#if defined(__MINGW32CE__) || defined(_MSC_VER)
# include <limits.h>
#endif /* __MINGW32CE__ || _MSC_VER */

#include <windows.h>
#include <psapi.h> /*  EnumProcessModules(Ex) */

#include "../Evil.h"

#include "dlfcn.h"


static char *dl_err = NULL;
static int dl_err_viewed = 0;

static void
get_last_error(char *desc)
{
   char  *str;
   size_t l1;
   size_t l2;

   str = evil_last_error_get();

   l1 = strlen(desc);
   l2 = strlen(str);

   if (dl_err)
     free(dl_err);

   dl_err = (char *)malloc(sizeof(char) * (l1 + l2 + 1));
   if (!dl_err)
     dl_err = strdup("not enough resource");
   else
     {
        memcpy(dl_err, desc, l1);
        memcpy(dl_err + l1, str, l2);
        dl_err[l1 + l2] = '\0';
     }
   free(str);
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
        char        *new_path;
        size_t       l;
        unsigned int i;

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
#ifdef UNICODE
        {
           wchar_t *wpath;

           wpath = evil_char_to_wchar(new_path);
           module = LoadLibrary(wpath);
           free(wpath);
        }
#else
        module = LoadLibraryEx(new_path, NULL,
                               LOAD_WITH_ALTERED_SEARCH_PATH);
#endif /* ! UNICODE */
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
   FARPROC fp = NULL;
   LPCTSTR new_symbol;

   if (!symbol || !*symbol) return NULL;

#ifdef UNICODE
   new_symbol = evil_char_to_wchar(symbol);
#else
   new_symbol = symbol;
#endif /* UNICODE */

   if (handle == RTLD_DEFAULT)
     {
        HMODULE modules[1024];
        DWORD needed;
        DWORD i;

        /* TODO: use EnumProcessModulesEx() on Windows >= Vista */
        if (!EnumProcessModules(GetCurrentProcess(),
                                modules, sizeof(modules), &needed))
          return NULL;

        for (i = 0; i < (needed /  sizeof(HMODULE)); i++)
          {
            fp = GetProcAddress(modules[i], new_symbol);
            if (fp) break;
          }
     }
   else
     fp = GetProcAddress(handle, new_symbol);

#ifdef UNICODE
   free((void *)new_symbol);
#endif /* UNICODE */

   if (!fp)
     get_last_error("GetProcAddress returned: ");

   return fp;
}

int
dladdr (const void *addr __UNUSED__, Dl_info *info)
{
   TCHAR  tpath[PATH_MAX];
   MEMORY_BASIC_INFORMATION mbi;
   char  *path;
   char  *tmp;
   size_t length;
   int    ret = 0;

   if (!info)
     return 0;

#ifdef _WIN32_WINNT
   length = VirtualQuery(addr, &mbi, sizeof(mbi));
   if (!length)
     return 0;

   if (mbi.State != MEM_COMMIT)
     return 0;

   if (!mbi.AllocationBase)
     return 0;

   ret = GetModuleFileName((HMODULE)mbi.AllocationBase, (LPTSTR)&tpath, PATH_MAX);
   if (!ret)
     return 0;
#else
   ret = GetModuleFileName(NULL, (LPTSTR)&tpath, PATH_MAX);
   if (!ret)
     return 0;
#endif

#ifdef UNICODE
   path = evil_wchar_to_char(tpath);
#else
   path = tpath;
#endif /* ! UNICODE */

   length = strlen (path);
   if (length >= PATH_MAX)
     {
       length = PATH_MAX - 1;
       path[PATH_MAX - 1] = '\0';
     }

   /* replace '/' by '\' */
   tmp = path;
   while (*tmp)
     {
        if (*tmp == '/') *tmp = '\\';
        tmp++;
     }

   memcpy (info->dli_fname, path, length + 1);
   info->dli_fbase = NULL;
   info->dli_sname = NULL;
   info->dli_saddr = NULL;

#ifdef UNICODE
   free (path);
#endif /* ! UNICODE */

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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>

#include "evil_private.h"

#include <psapi.h> /*  EnumProcessModules(Ex) */


static char *_dl_err = NULL;
static int _dl_err_viewed = 0;

static void
_dl_get_last_error(char *desc)
{
   const char *str;
   size_t l1;
   size_t l2;

   str = evil_last_error_get();

   l1 = strlen(desc);
   l2 = strlen(str);

   if (_dl_err)
     free(_dl_err);

   _dl_err = (char *)malloc(sizeof(char) * (l1 + l2 + 1));
   if (!_dl_err)
     _dl_err = strdup("not enough resource");
   else
     {
        memcpy(_dl_err, desc, l1);
        memcpy(_dl_err + l1, str, l2);
        _dl_err[l1 + l2] = '\0';
     }
   _dl_err_viewed = 0;
}

void *
dlopen(const char* path, int mode EVIL_UNUSED)
{
   HMODULE module = NULL;

   if (!path)
     {
        module = GetModuleHandle(NULL);
        if (!module)
          _dl_get_last_error("GetModuleHandle returned: ");
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
             if (_dl_err)
               free(_dl_err);
             _dl_err = strdup("not enough resource");
             _dl_err_viewed = 0;
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
          _dl_get_last_error("LoadLibraryEx returned: ");

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
        _dl_get_last_error("FreeLibrary returned: ");
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
          {
#ifdef UNICODE
             _dl_get_last_error("EnumProcessModules returned: ");
             free((void *)new_symbol);
#endif /* UNICODE */
             return NULL;
          }

        for (i = 0; i < (needed / sizeof(HMODULE)); i++)
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
     _dl_get_last_error("GetProcAddress returned: ");

   return fp;
}

char *
dlerror (void)
{
   if (!_dl_err_viewed)
     {
        _dl_err_viewed = 1;
        return _dl_err;
     }
   else
     {
        if (_dl_err)
          free(_dl_err);
        return NULL;
     }
}

#ifdef _GNU_SOURCE

static char _dli_fname[MAX_PATH];
static char _dli_sname[MAX_PATH]; /* a symbol should have at most 255 char */

static int
_dladdr_comp(const void *p1, const void *p2)
{
   return ( *(int *)p1 - *(int *)p2);
}

int
dladdr (const void *addr, Dl_info *info)
{
   TCHAR tpath[PATH_MAX];
   MEMORY_BASIC_INFORMATION mbi;
   unsigned char *base;
   char *path;
   size_t length;

   IMAGE_NT_HEADERS *nth;
   IMAGE_EXPORT_DIRECTORY *ied;
   DWORD *addresses;
   WORD *ordinals;
   DWORD *names;
   DWORD *tmp;
   DWORD res;
   DWORD rva_addr;
   DWORD i;

   if (!info)
     return 0;

   info->dli_fname = NULL;
   info->dli_fbase = NULL;
   info->dli_sname = NULL;
   info->dli_saddr = NULL;

   /* Get the name and base address of the module */

   if (!VirtualQuery(addr, &mbi, sizeof(mbi)))
     {
        _dl_get_last_error("VirtualQuery returned: ");
        return 0;
     }

   if (mbi.State != MEM_COMMIT)
     return 0;

   if (!mbi.AllocationBase)
     return 0;

   base = (unsigned char *)mbi.AllocationBase;

   if (!GetModuleFileName((HMODULE)base, (LPTSTR)&tpath, PATH_MAX))
     {
        _dl_get_last_error("GetModuleFileName returned: ");
        return 0;
     }

# ifdef UNICODE
   path = evil_wchar_to_char(tpath);
# else
   path = tpath;
# endif /* ! UNICODE */

   length = strlen(path);
   if (length >= PATH_MAX)
     {
       length = PATH_MAX - 1;
       path[PATH_MAX - 1] = '\0';
     }

   memcpy(_dli_fname, path, length + 1);
   info->dli_fname = (const char *)_dli_fname;
   info->dli_fbase = base;

# ifdef UNICODE
        free(path);
# endif /* ! UNICODE */

   /* get the name and the address of the required symbol */

   if (((IMAGE_DOS_HEADER *)base)->e_magic != IMAGE_DOS_SIGNATURE)
     {
        SetLastError(1276);
        return 0;
     }

   nth = (IMAGE_NT_HEADERS *)(base + ((IMAGE_DOS_HEADER *)base)->e_lfanew);
   if (nth->Signature != IMAGE_NT_SIGNATURE)
     {
        SetLastError(1276);
        return 0;
     }

   /* no exported symbols ? it's an EXE and we exit without error */
   if (nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress == 0)
     {
        return 1;
     }

   /* we assume now that the PE file is well-formed, so checks only when needed */
   ied = (IMAGE_EXPORT_DIRECTORY *)(base + nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
   addresses = (DWORD *)(base + ied->AddressOfFunctions);
   ordinals = (WORD *)(base + ied->AddressOfNameOrdinals);
   names = (DWORD *)(base + ied->AddressOfNames);

   /* the addresses are not ordered, so we need to order them */
   tmp = malloc(ied->NumberOfFunctions * sizeof(DWORD));
   if (!tmp)
     {
        SetLastError(8);
        return 0;
     }

   memcpy(tmp, addresses, ied->NumberOfFunctions * sizeof(DWORD));
   qsort(tmp, ied->NumberOfFunctions, sizeof(DWORD), _dladdr_comp);
   rva_addr = (unsigned char *)addr - base;
   res = (DWORD)(-1);
   for (i = 0; i < ied->NumberOfFunctions; i++)
     {
        if (tmp[i] < rva_addr)
          continue;

        res = tmp[i];
        break;
     }

   /* if rva_addr is too high, we store the latest address */
   if (res == (DWORD)(-1))
     res = tmp[ied->NumberOfFunctions - 1];

   free(tmp);

   for (i = 0; i < ied->NumberOfNames; i++)
     {
        if (addresses[ordinals[i]] == res)
          {
             char *name;

             name = (char *)(base + names[i]);
             length = strlen(name);
             if (length >= PATH_MAX)
               {
                  length = PATH_MAX - 1;
                  name[PATH_MAX - 1] = '\0';
               }
             memcpy(_dli_sname, name, length + 1);
             info->dli_sname = (const char *)_dli_sname;
             info->dli_saddr = base + res;
             return 1;
          }
     }

   return 0;
}

#endif /* _GNU_SOURCE */

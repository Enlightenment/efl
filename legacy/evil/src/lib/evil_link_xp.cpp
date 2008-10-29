
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

# include <shlobj.h>
# include <objidl.h>

#include "Evil.h"


/*
 * Symbolic links and directory related functions
 *
 */


/* REMARK: Windows has no symbolic link. */
/*         Nevertheless, it can create and read .lnk files */
int
symlink(const char *oldpath, const char *newpath)
{
   wchar_t        new_path[MB_CUR_MAX];
   IShellLink    *pISL;
   IShellLink   **shell_link;
   IPersistFile  *pIPF;
   IPersistFile **persit_file;
   HRESULT        res;

   res = CoInitialize(NULL);
   if (FAILED(res))
     {
        if (res == E_OUTOFMEMORY)
          errno = ENOMEM;
        return -1;
     }

   /* Hack to cleanly remove a warning */
   shell_link = &pISL;
   if (FAILED(CoCreateInstance(CLSID_ShellLink,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_IShellLink,
                               (void **)shell_link)))
     goto no_instance;

   if (FAILED(pISL->SetPath(oldpath)))
     goto no_setpath;

   /* Hack to cleanly remove a warning */
   persit_file = &pIPF;
   if (FAILED(pISL->QueryInterface(IID_IPersistFile, (void **)persit_file)))
     goto no_queryinterface;

   mbstowcs(new_path, newpath, MB_CUR_MAX);
   if (FAILED(pIPF->Save(new_path, FALSE)))
     goto no_save;

   pIPF->Release();
   pISL->Release();
   CoUninitialize();

   return 0;

 no_save:
   pIPF->Release();
 no_queryinterface:
 no_setpath:
   pISL->Release();
 no_instance:
   CoUninitialize();
   return -1;
}

ssize_t
readlink(const char *path, char *buf, size_t bufsiz)
{
   wchar_t        old_path[MB_CUR_MAX];
   char           new_path[PATH_MAX];
   IShellLink    *pISL;
   IShellLink   **shell_link;
   IPersistFile  *pIPF;
   IPersistFile **persit_file;
   unsigned int   length;
   HRESULT        res;

   res = CoInitialize(NULL);
   if (FAILED(res))
     {
        if (res == E_OUTOFMEMORY)
          errno = ENOMEM;
        return -1;
     }

   /* Hack to cleanly remove a warning */
   persit_file = &pIPF;
   if (FAILED(CoCreateInstance(CLSID_ShellLink,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_IPersistFile,
                               (void **)persit_file)))
     goto no_instance;

   mbstowcs(old_path, path, MB_CUR_MAX);
   if (FAILED(pIPF->Load(old_path, STGM_READWRITE)))
     goto no_load;

   shell_link = &pISL;
   if (FAILED(pIPF->QueryInterface(IID_IShellLink, (void **)shell_link)))
     goto no_queryinterface;

   if (FAILED(pISL->GetPath(new_path, PATH_MAX, NULL, 0)))
     goto no_getpath;

   length = strlen(new_path);
   if (length > bufsiz)
     length = bufsiz;

   memcpy(buf, new_path, length);

   pISL->Release();
   pIPF->Release();
   CoUninitialize();

   return length;

 no_getpath:
   pISL->Release();
 no_queryinterface:
 no_load:
   pIPF->Release();
 no_instance:
   CoUninitialize();
   return -1;
}

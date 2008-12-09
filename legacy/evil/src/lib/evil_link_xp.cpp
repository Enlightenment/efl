
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */

# include <shlobj.h>
# include <objidl.h>
#include <cstdio>

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
   wchar_t      *wnewpath;
   IShellLink   *pISL;
   IPersistFile *pIPF;
   HRESULT       res;
   size_t        size;

   res = CoInitialize(NULL);
   if (FAILED(res))
     {
        if (res == E_OUTOFMEMORY)
          errno = ENOMEM;
        return -1;
     }

   if (FAILED(CoCreateInstance(CLSID_ShellLink,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_IShellLink,
                               (void **)&pISL)))
     goto no_instance;

   if (FAILED(pISL->SetPath(oldpath)))
     goto no_setpath;

   if (FAILED(pISL->QueryInterface(IID_IPersistFile, (void **)&pIPF)))
     goto no_queryinterface;

   size = mbstowcs(NULL, newpath, 0);
   wnewpath = (wchar_t *)malloc((size + 1) * sizeof(wchar_t));
   if (!wnewpath)
     goto malloc_failure;
   if (mbstowcs(wnewpath, newpath, size + 1) == (size_t)(-1))
     goto translation_failure;
   if (FAILED(pIPF->Save(wnewpath, FALSE)))
     goto no_save;

   free(wnewpath);
   pIPF->Release();
   pISL->Release();
   CoUninitialize();

   return 0;

 no_save:
 translation_failure:
 malloc_failure:
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
   wchar_t      *wpath;
   char          new_path[PATH_MAX];
   IShellLink   *pISL;
   IPersistFile *pIPF;
   unsigned int  length;
   HRESULT       res;
   size_t        size;

   res = CoInitialize(NULL);
   if (FAILED(res))
     {
        if (res == E_OUTOFMEMORY)
          errno = ENOMEM;
        return -1;
     }

   if (FAILED(CoCreateInstance(CLSID_ShellLink,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_IShellLink,
                               (void **)&pISL)))
     goto couninitialize;

   if (FAILED(pISL->QueryInterface(IID_IPersistFile, (void **)&pIPF)))
     goto release_shell_link;

   size = mbstowcs(NULL, path, 0);
   wpath = (wchar_t *)malloc((size + 1) * sizeof(wchar_t));
   if (!wpath)
     goto release_persist_file;

   mbstowcs(wpath, path, size + 1);
   if (FAILED(pIPF->Load(wpath, STGM_READ)))
     goto free_wpath;

   if (FAILED(pISL->Resolve(NULL, SLR_UPDATE | SLR_NO_UI)))
     goto free_wpath;

   if (FAILED(pISL->GetPath(new_path, PATH_MAX, NULL, 0)))
     goto free_wpath;

   length = strlen(new_path);
   if (length > bufsiz)
     length = bufsiz;

   memcpy(buf, new_path, length);

   free(wpath);
   pISL->Release();
   pIPF->Release();
   CoUninitialize();

   return length;

 free_wpath:
   free(wpath);
 release_persist_file:
   pIPF->Release();
 release_shell_link:
   pISL->Release();
 couninitialize:
   CoUninitialize();
   return -1;
}

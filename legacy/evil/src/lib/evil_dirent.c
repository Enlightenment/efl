
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <dirent.h>

#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif

#include "Evil.h"


struct DIR
{
   struct dirent   dirent;
   WIN32_FIND_DATA data;
   HANDLE          handle;
};


#ifndef __CEGCC__

DIR *opendir(char const *name)
{
   DIR     *dir;
   char    *tmp1;
   char    *tmp2;
   DWORD    attr;
   int      l;
#ifdef UNICODE
   wchar_t *wname;
   char    *d_name;
#endif

   /* valid name */
   if (!name || !*name)
     {
#ifdef HAVE_ERRNO_H
        errno = ENOENT;
#endif
        return NULL;
     }

#ifdef UNICODE
   wname = evil_char_to_wchar(name);
   if (!wname)
     {
#ifdef HAVE_ERRNO_H
        errno = ENOMEM;
#endif
        return NULL;
     }

   if((attr = GetFileAttributes(wname)) == 0xFFFFFFFF)
#else
   if((attr = GetFileAttributes(name)) == 0xFFFFFFFF)
#endif
     {
#ifdef HAVE_ERRNO_H
        errno = ENOENT;
#endif
        return NULL;
     }

#ifdef UNICODE
   free(wname);
#endif

     /* directory */
     if (attr != FILE_ATTRIBUTE_DIRECTORY)
     {
#ifdef HAVE_ERRNO_H
        errno = ENOTDIR;
#endif
        return NULL;
     }

     dir = (DIR *)malloc(sizeof(DIR));
     if (!dir)
     {
#ifdef HAVE_ERRNO_H
        errno = ENOMEM;
#endif
        return NULL;
     }

   l = strlen(name);
   tmp1 = (char *)malloc(sizeof(char) * l + 5);
   if (!tmp1)
     {
#ifdef HAVE_ERRNO_H
        errno = ENOMEM;
#endif
        return NULL;
     }

   memcpy(tmp1, name, l);
   memcpy(tmp1 + l, "\\*.*", 5);

   tmp2 = tmp1;
   while (*tmp2)
     {
        if (*tmp2 == '/') *tmp2 = '\\';
        tmp2++;
     }

#ifdef UNICODE
   wname = evil_char_to_wchar(tmp1);
   if (!wname)
     {
#ifdef HAVE_ERRNO_H
        errno = ENOMEM;
#endif
        free(tmp1);

        return NULL;
     }
   dir->handle = FindFirstFile(wname, &dir->data);
   free(wname);
#else
   dir->handle = FindFirstFile(tmp1, &dir->data);
#endif

   free(tmp1);

   if (dir->handle == INVALID_HANDLE_VALUE)
     {
        free(dir);
        return NULL;
     }

#ifdef UNICODE
   d_name = evil_wchar_to_char(dir->data.cFileName);
   strcpy(dir->dirent.d_name, d_name);
   free(d_name);
#else
   strcpy(dir->dirent.d_name, dir->data.cFileName);
#endif
   dir->dirent.d_mode = (int)dir->data.dwFileAttributes;

   return dir;
}

int closedir(DIR *dir)
{
   if (!dir)
     {
#ifdef HAVE_ERRNO_H
        errno = EBADF;
#endif
        return -1;
     }

   if (dir->handle != INVALID_HANDLE_VALUE)
     FindClose(dir->handle);
   free(dir);

   return 0;
}

struct dirent *readdir(DIR *dir)
{
#ifdef UNICODE
   char *d_name;
#endif

   if (!dir)
     {
#ifdef HAVE_ERRNO_H
        errno = EBADF;
#endif
        return NULL;
     }

   if (dir->handle == INVALID_HANDLE_VALUE)
     return NULL;

#ifdef UNICODE
     d_name = evil_wchar_to_char(dir->data.cFileName);
     strcpy(dir->dirent.d_name, d_name);
     free(d_name);
#else
     strcpy(dir->dirent.d_name, dir->data.cFileName);
#endif

   if (!FindNextFile(dir->handle, &dir->data))
     {
        FindClose(dir->handle);
        dir->handle = INVALID_HANDLE_VALUE;
     }

   return &dir->dirent;
}

#endif /* ! __CEGCC__ */

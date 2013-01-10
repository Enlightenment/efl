#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <dirent.h>
#include <errno.h>

#include "Evil.h"


struct DIR
{
   struct dirent   dirent;
   WIN32_FIND_DATA data;
   HANDLE          handle;
};


DIR *opendir(char const *name)
{
   DIR     *dir;
   char    *tmp1;
   char    *tmp2;
   DWORD    attr;
   size_t   l;
#ifdef UNICODE
   wchar_t *wname;
   char    *d_name;
#endif

   /* valid name */
   if (!name || !*name)
     {
        errno = ENOENT;
        return NULL;
     }

#ifdef UNICODE
   wname = evil_char_to_wchar(name);
   if (!wname)
     {
        errno = ENOMEM;
        return NULL;
     }

   if((attr = GetFileAttributes(wname)) == 0xFFFFFFFF)
#else
   if((attr = GetFileAttributes(name)) == 0xFFFFFFFF)
#endif
     {
        errno = ENOENT;
        return NULL;
     }

#ifdef UNICODE
   free(wname);
#endif

   /* directory */
   if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
     {
        errno = ENOTDIR;
        return NULL;
     }

   dir = (DIR *)malloc(sizeof(DIR));
   if (!dir)
     {
        errno = ENOMEM;
        return NULL;
     }

   l = strlen(name);
   tmp1 = (char *)malloc(sizeof(char) * l + 5);
   if (!tmp1)
     {
        errno = ENOMEM;
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
        errno = ENOMEM;
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

   if (dir->data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
     dir->dirent.d_type = DT_DIR;
   else
     dir->dirent.d_type = DT_UNKNOWN;

   return dir;
}

int closedir(DIR *dir)
{
   if (!dir)
     {
        errno = EBADF;
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
        errno = EBADF;
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

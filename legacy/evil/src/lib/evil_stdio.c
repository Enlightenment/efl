
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "Evil.h"
#include "evil_private.h"

#undef fopen


#if defined (_WIN32_WCE) && ! defined (__CEGCC__)

/*
 * Error related functions
 *
 */

void perror (const char *s __UNUSED__)
{
   fprintf(stderr, "[Windows CE] error\n");
}

/*
 * Stream related functions
 *
 */

FILE *evil_fopen(const char *path, const char *mode)
{
   FILE *f;
   char *filename;

   if (!path || !*path)
     return NULL;

   if (*path != '\\')
     {
        char buf[PATH_MAX];
        int   l1;
        int   l2;

        if (!evil_getcwd(buf, PATH_MAX))
          return NULL;

        l1 = strlen(buf);
        l2 = strlen(path);
        filename = (char *)malloc(l1 + 1 + l2 + 1);
        if (!filename)
          return NULL;
        memcpy(filename, buf, l1);
        filename[l1] = '\\';
        memcpy(filename + l1 + 1, path, l2);
        filename[l1 + 1 + l2] = '\0';
     }
   else
     filename = (char *)path;

   f = fopen(filename, mode);
   if (*path != '\\')
     free(filename);

   return f;
}

void evil_rewind(FILE *stream)
{
   fseek(stream, 0, SEEK_SET);
}


#endif /* _WIN32_WCE && ! __CEGCC__ */


#ifdef _WIN32_WCE

FILE *evil_fopen_native(const char *path, const char *mode)
{
   HANDLE   handle;
   char    *filename;
   wchar_t *wfilename;
   DWORD    access = GENERIC_READ;
   DWORD    creation;

   if (!path || !*path || !mode || !*mode)
     return NULL;

   if (*path != '\\')
     {
        char buf[PATH_MAX];
        int   l1;
        int   l2;

        if (!evil_getcwd(buf, PATH_MAX))
          return NULL;

        l1 = strlen(buf);
        l2 = strlen(path);
        filename = (char *)malloc(l1 + 1 + l2 + 1);
        if (!filename)
          return NULL;
        memcpy(filename, buf, l1);
        filename[l1] = '\\';
        memcpy(filename + l1 + 1, path, l2);
        filename[l1 + 1 + l2] = '\0';
     }
   else
     filename = (char *)path;

   wfilename = evil_char_to_wchar(filename);
   if (*path != '\\')
     free(filename);

   if (!wfilename)
     return NULL;

   if (*mode == 'r')
     {
        access = GENERIC_READ;
        creation = OPEN_EXISTING;
     }
   if (*mode == 'w')
     {
        access = GENERIC_WRITE;
        creation = CREATE_ALWAYS;
     }

   handle = CreateFile(wfilename,
                       access,
                       0, NULL,
                       creation,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
   free(wfilename);

   if (handle == INVALID_HANDLE_VALUE)
     {
        _evil_last_error_display(__FUNCTION__);
        return NULL;
     }

   return (FILE *)handle;
}

size_t evil_fread_native(void* buffer, size_t size, size_t count, FILE* stream)
{
   HANDLE handle;
   DWORD  bytes_read;
   BOOL   res;

   if ((size == 0) || (count == 0))
     return 0;

   handle = (HANDLE)stream;
   res = ReadFile(handle, buffer, size * count, &bytes_read, NULL);
   if (!res)
     {
        _evil_last_error_display(__FUNCTION__);
        return 0;
     }

   return (bytes_read != size * count) ? 0 : 1;
}

int evil_fclose_native(FILE *stream)
{
   if (!CloseHandle((HANDLE)stream))
     {
        _evil_last_error_display(__FUNCTION__);
        return -1;
     }

   return 0;
}

#endif /* _WIN32_WCE */

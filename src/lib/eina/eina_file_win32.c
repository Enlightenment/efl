/* EINA - EFL data type library
 * Copyright (C) 2010 Vincent Torri
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <evil_private.h>
#include <fcntl.h>

#include "eina_config.h"
#include "eina_private.h"
#include "eina_alloca.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_file.h"
#include "eina_stringshare.h"
#include "eina_hash.h"
#include "eina_list.h"
#include "eina_lock.h"
#include "eina_log.h"
#include "eina_file_common.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#ifdef MAP_FAILED
# undef MAP_FAILED
#endif
#define MAP_FAILED ((void *)-1)

typedef struct _Eina_File_Iterator        Eina_File_Iterator;
typedef struct _Eina_File_Direct_Iterator Eina_File_Direct_Iterator;
typedef struct _Eina_File_Map             Eina_File_Map;

struct _Eina_File_Iterator
{
   Eina_Iterator   iterator;

   WIN32_FIND_DATA data;
   HANDLE          handle;
   size_t          length;
   Eina_Bool       is_last : 1;

   char            dir[1];
};

struct _Eina_File_Direct_Iterator
{
   Eina_Iterator         iterator;

   WIN32_FIND_DATA       data;
   HANDLE                handle;
   size_t                length;
   Eina_Bool             is_last : 1;

   Eina_File_Direct_Info info;

   char                  dir[1];
};

int _eina_file_log_dom = -1;

static Eina_Bool
_eina_file_win32_is_dir(const char *dir)
{
#ifdef UNICODE
   wchar_t *wdir = NULL;
#endif
   DWORD    attr;

   /* check if it's a directory */
#ifdef UNICODE
   wdir = evil_char_to_wchar(dir);
   if (!wdir)
     return EINA_FALSE;

   attr = GetFileAttributes(wdir);
   free(wdir);
#else
   attr = GetFileAttributes(dir);
#endif

   if (attr == INVALID_FILE_ATTRIBUTES)
     return EINA_FALSE;

   if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
     return EINA_FALSE;

   return EINA_TRUE;
}

static HANDLE
_eina_file_win32_first_file(const char *dir, WIN32_FIND_DATA *fd)
{
   char buf[4096];
   HANDLE h;
   size_t l = strlen(dir);
#ifdef UNICODE
   wchar_t *wdir = NULL;
#endif

   l = strlen(dir);
   if ((l + 5) > sizeof(buf))
     return INVALID_HANDLE_VALUE;

   memcpy(buf, dir, l);
   memcpy(buf + l, "\\*.*", 5);

#ifdef UNICODE
   wdir = evil_char_to_wchar(buf);
   if (!wdir)
     return INVALID_HANDLE_VALUE;

   h = FindFirstFileEx(wdir, FindExInfoBasic, fd, FindExSearchNameMatch, NULL, 0);
   free(wdir);
#else
   h = FindFirstFileEx(buf, FindExInfoBasic, fd, FindExSearchNameMatch, NULL, 0);
#endif

   if (!h)
     return INVALID_HANDLE_VALUE;

   while ((fd->cFileName[0] == '.') &&
          ((fd->cFileName[1] == '\0') ||
           ((fd->cFileName[1] == '.') && (fd->cFileName[2] == '\0'))))
     {
        if (!FindNextFile(h, fd))
          {
             FindClose(h);
             return INVALID_HANDLE_VALUE;
          }
     }

   return h;
}

static Eina_Bool
_eina_file_win32_ls_iterator_next(Eina_File_Iterator *it, void **data)
{
#ifdef UNICODE
   wchar_t  *old_name;
#else
   char     *old_name;
#endif
   char     *name;
   char     *cname;
   size_t    length;
   Eina_Bool is_last;
   Eina_Bool res = EINA_TRUE;

   if (it->handle == INVALID_HANDLE_VALUE)
     {
        if (GetLastError() == ERROR_NO_MORE_FILES)
          it->is_last = EINA_TRUE;
        return EINA_FALSE;
     }

   is_last = it->is_last;
#ifdef UNICODE
   old_name = _wcsdup(it->data.cFileName);
#else
   old_name = _strdup(it->data.cFileName);
#endif
   if (!old_name)
     return EINA_FALSE;

   do {
      if (!FindNextFile(it->handle, &it->data))
        {
           if (GetLastError() == ERROR_NO_MORE_FILES)
             it->is_last = EINA_TRUE;
           else
             res = EINA_FALSE;
        }
   } while ((it->data.cFileName[0] == '.') &&
            ((it->data.cFileName[1] == '\0') ||
             ((it->data.cFileName[1] == '.') && (it->data.cFileName[2] == '\0')))); /* FIXME: what about UNICODE ? */

#ifdef UNICODE
   cname = evil_wchar_to_char(old_name);
   if (!cname)
     return EINA_FALSE;
#else
     cname = old_name;
#endif

   length = strlen(cname);
   name = alloca(length + 2 + it->length);

   memcpy(name,                  it->dir, it->length);
   memcpy(name + it->length,     "\\",    1);
   memcpy(name + it->length + 1, cname,   length + 1);

   *data = (char *)eina_stringshare_add(name);

#ifdef UNICODE
   free(cname);
#endif
   free(old_name);

   if (is_last)
     res = EINA_FALSE;

   return res;
}

static HANDLE
_eina_file_win32_ls_iterator_container(Eina_File_Iterator *it)
{
   return it->handle;
}

static void
_eina_file_win32_ls_iterator_free(Eina_File_Iterator *it)
{
   if (it->handle != INVALID_HANDLE_VALUE)
     FindClose(it->handle);

   EINA_MAGIC_SET(&it->iterator, 0);
   free(it);
}

static Eina_Bool
_eina_file_win32_direct_ls_iterator_next(Eina_File_Direct_Iterator *it, void **data)
{
#ifdef UNICODE
   wchar_t  *old_name;
#else
   char     *old_name;
#endif
   char     *cname;
   size_t    length;
   DWORD     attr;
   Eina_Bool is_last;
   Eina_Bool res = EINA_TRUE;

   if (it->handle == INVALID_HANDLE_VALUE)
     {
        if (GetLastError() == ERROR_NO_MORE_FILES)
          it->is_last = EINA_TRUE;
        return EINA_FALSE;
     }

   attr = it->data.dwFileAttributes;
   is_last = it->is_last;
#ifdef UNICODE
   old_name = _wcsdup(it->data.cFileName);
#else
   old_name = _strdup(it->data.cFileName);
#endif
   if (!old_name)
     return EINA_FALSE;

   do {
      if (!FindNextFile(it->handle, &it->data))
        {
           if (GetLastError() == ERROR_NO_MORE_FILES)
             it->is_last = EINA_TRUE;
           else
             res = EINA_FALSE;
        }

#ifdef UNICODE
     length = wcslen(old_name);
#else
     length = strlen(old_name);
#endif
     if (it->info.name_start + length + 1 >= PATH_MAX)
       {
          free(old_name);
#ifdef UNICODE
          old_name = _wcsdup(it->data.cFileName);
#else
          old_name = _strdup(it->data.cFileName);
#endif
          continue;
       }

   } while ((it->data.cFileName[0] == '.') &&
            ((it->data.cFileName[1] == '\0') ||
             ((it->data.cFileName[1] == '.') && (it->data.cFileName[2] == '\0')))); /* FIXME: what about UNICODE ? */

#ifdef UNICODE
   cname = evil_wchar_to_char(old_name);
   if (!cname)
     return EINA_FALSE;
#else
     cname = old_name;
#endif

   memcpy(it->info.path + it->info.name_start, cname, length);
   it->info.name_length = length;
   it->info.path_length = it->info.name_start + length;
   it->info.path[it->info.path_length] = '\0';

   if (attr & FILE_ATTRIBUTE_DIRECTORY)
     it->info.type = EINA_FILE_DIR;
   else if (attr & (FILE_ATTRIBUTE_ARCHIVE |
                    FILE_ATTRIBUTE_COMPRESSED |
                    FILE_ATTRIBUTE_HIDDEN |
                    FILE_ATTRIBUTE_NORMAL |
                    FILE_ATTRIBUTE_SPARSE_FILE |
                    FILE_ATTRIBUTE_TEMPORARY |
                    FILE_ATTRIBUTE_REPARSE_POINT))
     it->info.type = EINA_FILE_REG;
   else
     it->info.type = EINA_FILE_UNKNOWN;

   *data = &it->info;

#ifdef UNICODE
   free(cname);
#endif

   free(old_name);

   if (is_last)
     res = EINA_FALSE;

   return res;
}

static HANDLE
_eina_file_win32_direct_ls_iterator_container(Eina_File_Direct_Iterator *it)
{
   return it->handle;
}

static void
_eina_file_win32_direct_ls_iterator_free(Eina_File_Direct_Iterator *it)
{
   if (it->handle != INVALID_HANDLE_VALUE)
     FindClose(it->handle);

   EINA_MAGIC_SET(&it->iterator, 0);
   free(it);
}

void
eina_file_real_close(Eina_File *file)
{
   Eina_File_Map *map;

   EINA_LIST_FREE(file->dead_map, map)
     {
        UnmapViewOfFile(map->map);
        free(map);
     }

   if (file->handle != INVALID_HANDLE_VALUE)
     {
        if (!file->copied && file->global_map != MAP_FAILED)
          UnmapViewOfFile(file->global_map);
        CloseHandle(file->handle);
     }
}

static void
_eina_file_map_close(Eina_File_Map *map)
{
   if (map->map != MAP_FAILED)
     UnmapViewOfFile(map->map);
   free(map);
}

static char *
_eina_file_sep_find(char *s)
{
   for (; *s != '\0'; ++s)
     if ((*s == '\\') || (*s == '/'))
       return s;

   return NULL;
}

static unsigned char _eina_file_random_uchar(unsigned char *c)
{
  /*
   * Helper function for mktemp.
   *
   * Only characters from 'a' to 'z' and '0' to '9' are considered
   * because on Windows, file system is case insensitive. That means
   * 36 possible values.
   * To increase randomness, we consider the greatest multiple of 36
   * within 255 : 7*36 = 252, that is, values from 0 to 251 and choose
   * a random value in this interval.
   */
  do {
    BCryptGenRandom(_eina_bcrypt_provider, c, sizeof(UCHAR), 0);
  } while (*c > 251);

  *c = '0' + *c % 36;
  if (*c > '9')
    *c += 'a' - '9' - 1;

  return *c;
}

static int
_eina_file_mkstemp_init(char *__template, size_t *length, int suffixlen)
{
  if (!__template || (suffixlen < 0))
     {
        errno = EINVAL;
        return 0;
     }

  *length = strlen(__template);
  if ((*length < (6 + (size_t)suffixlen))
       || (strncmp(__template + *length - 6 - suffixlen, "XXXXXX", 6) != 0))
     {
        errno = EINVAL;
        return 0;
     }
   return 1;
}

static void
_eina_file_tmpname(char *__template, size_t length, int suffixlen)
{
   unsigned char *suffix;

   suffix = (unsigned char *)(__template + length - 6 - suffixlen);
   *suffix = _eina_file_random_uchar(suffix);
   suffix++;
   *suffix = _eina_file_random_uchar(suffix);
   suffix++;
   *suffix = _eina_file_random_uchar(suffix);
   suffix++;
   *suffix = _eina_file_random_uchar(suffix);
   suffix++;
   *suffix = _eina_file_random_uchar(suffix);
   suffix++;
   *suffix = _eina_file_random_uchar(suffix);
   suffix++;
}

static int
_eina_file_mkstemps(char *__template, int suffixlen)
{
   size_t length;
   int i;

   if (!_eina_file_mkstemp_init(__template, &length, suffixlen))
     return -1;

   for (i = 0; i < 32768; i++)
     {
        int fd;

        _eina_file_tmpname(__template, length, suffixlen);

        fd = _open(__template,
                   _O_RDWR | _O_BINARY | _O_CREAT | _O_EXCL,
                   _S_IREAD | _S_IWRITE);
        if (fd >= 0)
          return fd;
     }

   errno = EEXIST;
   return -1;
}

static char *
_eina_file_mkdtemp(char *__template)
{
   size_t length;
   int i;

   if (!_eina_file_mkstemp_init(__template, &length, 0))
     return NULL;

   for (i = 0; i < 32768; i++)
     {
        _eina_file_tmpname(__template, length, 0);
        if (CreateDirectory(__template, NULL) == TRUE)
          return __template;
     }

   return NULL;
}


/**
 * @endcond
 */


/* ================================================================ *
 *   Simplified logic for portability layer with eina_file_common   *
 * ================================================================ */

Eina_Tmpstr *
eina_file_current_directory_get(const char *path, size_t len)
{
   char *tmp;
   DWORD l;

   l = GetCurrentDirectory(0, NULL);
   if (l == 0) return NULL;

   tmp = alloca(sizeof (char) * (l + len + 2));
   l = GetCurrentDirectory(l + 1, tmp);
   tmp[l] = '\\';
   memcpy(tmp + l + 1, path, len);
   tmp[l + len + 1] = '\0';

   return eina_tmpstr_add_length(tmp, l + len + 1);
}

char *
eina_file_cleanup(Eina_Tmpstr *path)
{
   char *result;

   result = strdup(path ? path : "");
   eina_tmpstr_del(path);

   if (!result)
     return NULL;

   EINA_PATH_TO_UNIX(result);

   return result;
}

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EINA_API Eina_Bool
eina_file_path_relative(const char *path)
{
   /* see
    * https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file#paths
    * absolute path if:
    * - is an UNC path (begins with \\)
    * - has a drive letter (C:\). \ is important here, otherwise it is relative
    * - begins with \
    */

   if (!path || *path == '\\')
     return EINA_FALSE;

   if ((((*path >= 'a') && (*path <= 'z')) ||
        ((*path >= 'A') && (*path <= 'Z'))) &&
       (path[1] == ':') &&
       ((path[2] == '\\') || (path[2] == '/')))
     return EINA_FALSE;

   return EINA_TRUE;
}

EINA_API Eina_Bool
eina_file_dir_list(const char *dir,
                   Eina_Bool recursive,
                   Eina_File_Dir_List_Cb cb,
                   void *data)
{
   WIN32_FIND_DATA file;
   HANDLE h;

   EINA_SAFETY_ON_NULL_RETURN_VAL(cb,  EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(dir[0] == '\0', EINA_FALSE);

   if (!_eina_file_win32_is_dir(dir))
     return EINA_FALSE;

   h = _eina_file_win32_first_file(dir, &file);

   if (h == INVALID_HANDLE_VALUE)
      return EINA_FALSE;

   do
     {
        char *filename;

# ifdef UNICODE
        filename = evil_wchar_to_char(file.cFileName);
# else
        filename = file.cFileName;
# endif /* ! UNICODE */
        if ((filename[0] == '.') &&
            ((filename[1] == '\0') ||
             ((filename[1] == '.') && (filename[2] == '\0'))))
           continue;

        cb(filename, dir, data);

        if (recursive == EINA_TRUE)
          {
             char *path;

             path = alloca(strlen(dir) + strlen(filename) + 2);
             strcpy(path, dir);
             strcat(path, "/");
             strcat(path, filename);

             if (!(file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                continue;

             eina_file_dir_list(path, recursive, cb, data);
          }

# ifdef UNICODE
        free(filename);
# endif /* UNICODE */

     } while (FindNextFile(h, &file));
   FindClose(h);

   return EINA_TRUE;
}

EINA_API Eina_Array *
eina_file_split(char *path)
{
   Eina_Array *ea;
   char *current;
   size_t length;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);

   ea = eina_array_new(16);

   if (!ea)
      return NULL;

   for (current = _eina_file_sep_find(path);
        current;
        path = current + 1, current = _eina_file_sep_find(path))
     {
        length = current - path;

        if (length == 0)
           continue;

        eina_array_push(ea, path);
        *current = '\0';
     }

   if (*path != '\0')
        eina_array_push(ea, path);

   return ea;
}

EINA_API Eina_Iterator *
eina_file_ls(const char *dir)
{
   Eina_File_Iterator *it;
   size_t              length;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, NULL);

   if (!dir || !*dir)
      return NULL;

   if (!_eina_file_win32_is_dir(dir))
     return NULL;

   length = strlen(dir);

   it = calloc(1, sizeof (Eina_File_Iterator) + length);
   if (!it)
      return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->handle = _eina_file_win32_first_file(dir, &it->data);
   if ((it->handle == INVALID_HANDLE_VALUE) && (GetLastError() != ERROR_NO_MORE_FILES))
     goto free_it;

   memcpy(it->dir, dir, length + 1);
   if ((dir[length - 1] != '\\') && (dir[length - 1] != '/'))
      it->length = length;
   else
      it->length = length - 1;

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_file_win32_ls_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_eina_file_win32_ls_iterator_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_file_win32_ls_iterator_free);

   return &it->iterator;

 free_it:
   free(it);

   return NULL;
}

EINA_API Eina_Iterator *
eina_file_direct_ls(const char *dir)
{
   Eina_File_Direct_Iterator *it;
   size_t                     length;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, NULL);

   if (!dir || !*dir)
      return NULL;

   length = strlen(dir);

   if (length + 12 + 2 >= MAX_PATH)
      return NULL;

   it = calloc(1, sizeof(Eina_File_Direct_Iterator) + length);
   if (!it)
      return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->handle = _eina_file_win32_first_file(dir, &it->data);
   if ((it->handle == INVALID_HANDLE_VALUE) && (GetLastError() != ERROR_NO_MORE_FILES))
     goto free_it;

   memcpy(it->dir, dir, length + 1);
   it->length = length;

   memcpy(it->info.path, dir, length);
   if ((dir[length - 1] == '\\') || (dir[length - 1] == '/'))
      it->info.name_start = length;
   else
     {
        it->info.path[length] = '\\';
        it->info.name_start = length + 1;
     }

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_file_win32_direct_ls_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_eina_file_win32_direct_ls_iterator_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_file_win32_direct_ls_iterator_free);

   return &it->iterator;

 free_it:
   free(it);

   return NULL;
}

EINA_API Eina_Iterator *
eina_file_stat_ls(const char *dir)
{
   return eina_file_direct_ls(dir);
}

EINA_API Eina_Bool
eina_file_refresh(Eina_File *file)
{
   WIN32_FILE_ATTRIBUTE_DATA fad;
   ULARGE_INTEGER length;
   ULARGE_INTEGER mtime;
   Eina_Bool r = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);

   if (file->virtual) return EINA_FALSE;

   if (!GetFileAttributesEx(file->filename, GetFileExInfoStandard, &fad))
     return EINA_FALSE;

   length.u.LowPart = fad.nFileSizeLow;
   length.u.HighPart = fad.nFileSizeHigh;
   mtime.u.LowPart = fad.ftLastWriteTime.dwLowDateTime;
   mtime.u.HighPart = fad.ftLastWriteTime.dwHighDateTime;

   if (file->length != length.QuadPart)
     {
        eina_file_flush(file, length.QuadPart);
        r = EINA_TRUE;
     }

   file->length = length.QuadPart;
   file->mtime = mtime.QuadPart;

   return r;
}

EINA_API Eina_File *
eina_file_open(const char *path, Eina_Bool shared)
{
   Eina_File *file;
   Eina_File *n;
   Eina_Stringshare *filename;
   HANDLE handle;
   WIN32_FILE_ATTRIBUTE_DATA fad;
   ULARGE_INTEGER length;
   ULARGE_INTEGER mtime;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);

   filename = eina_file_sanitize(path);
   if (!filename) return NULL;

   /* FIXME: how to emulate shm_open ? Just OpenFileMapping ? */
#if 0
   if (shared)
     handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ,
                         NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY,
                         NULL);
   else
#endif
     handle = CreateFile(filename,
                         GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                         NULL);

   if (handle == INVALID_HANDLE_VALUE)
     {
       errno = GetLastError();
       WRN("eina_file_open() failed with file %s: %s",
           filename, evil_format_message(errno));
       goto free_file;
     }

   if (!GetFileAttributesEx(filename, GetFileExInfoStandard, &fad))
     {
        errno = GetLastError();
        goto close_handle;
     }

   length.u.LowPart = fad.nFileSizeLow;
   length.u.HighPart = fad.nFileSizeHigh;
   mtime.u.LowPart = fad.ftLastWriteTime.dwLowDateTime;
   mtime.u.HighPart = fad.ftLastWriteTime.dwHighDateTime;

   eina_lock_take(&_eina_file_lock_cache);

   file = eina_hash_find(_eina_file_cache, filename);
   if (file &&
       (file->mtime != mtime.QuadPart || file->length != length.QuadPart))
     {
        file->delete_me = EINA_TRUE;
        eina_hash_del(_eina_file_cache, file->filename, file);
        file = NULL;
     }

   if (!file)
     {
        n = malloc(sizeof(Eina_File));
        if (!n)
          {
             eina_lock_release(&_eina_file_lock_cache);
             goto close_handle;
          }

        memset(n, 0, sizeof(Eina_File));
        n->filename = filename;
        n->map = eina_hash_new(EINA_KEY_LENGTH(eina_file_map_key_length),
                               EINA_KEY_CMP(eina_file_map_key_cmp),
                               EINA_KEY_HASH(eina_file_map_key_hash),
                               EINA_FREE_CB(_eina_file_map_close),
                               3);
        n->rmap = eina_hash_pointer_new(NULL);
        n->global_map = MAP_FAILED;
        n->length = length.QuadPart;
        n->mtime = mtime.QuadPart;
        n->handle = handle;
        n->shared = shared;
        eina_lock_new(&n->lock);
        eina_hash_direct_add(_eina_file_cache, n->filename, n);

        EINA_MAGIC_SET(n, EINA_FILE_MAGIC);
     }
   else
     {
        CloseHandle(handle);

        n = file;
     }
   eina_lock_take(&n->lock);
   n->refcount++;
   eina_lock_release(&n->lock);

   eina_lock_release(&_eina_file_lock_cache);

   return n;

 close_handle:
   CloseHandle(handle);
 free_file:
   WRN("Could not open file [%s].", filename);
   eina_stringshare_del(filename);

   return NULL;
}

EINA_API Eina_Bool
eina_file_unlink(const char *pathname)
{
   Eina_Stringshare *unlink_path = eina_file_sanitize(pathname);
   Eina_File *file = eina_hash_find(_eina_file_cache, unlink_path);
   Eina_Bool r = EINA_FALSE;

   if (file)
     {
        if (file->handle != INVALID_HANDLE_VALUE)
          {
             CloseHandle(file->handle);

             file->handle = CreateFile(unlink_path,
                                       GENERIC_READ,
                                       FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       NULL,
                                       OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
                                       NULL);

            if (file->handle != INVALID_HANDLE_VALUE)
              {
                 CloseHandle(file->handle);
                 file->handle = INVALID_HANDLE_VALUE;
                 r = EINA_TRUE;
                 goto finish;
              }
          }
     }

   if (unlink(unlink_path) >= 0) r = EINA_TRUE;
   eina_stringshare_del(unlink_path);

 finish:
   return r;
}


EINA_API Eina_Iterator *eina_file_xattr_get(Eina_File *file EINA_UNUSED)
{
   return NULL;
}

EINA_API Eina_Iterator *eina_file_xattr_value_get(Eina_File *file EINA_UNUSED)
{
   return NULL;
}

EINA_API void
eina_file_map_populate(Eina_File *file EINA_UNUSED, Eina_File_Populate rule EINA_UNUSED, const void *map EINA_UNUSED,
                       unsigned long int offset EINA_UNUSED, unsigned long int length EINA_UNUSED)
{
}

EINA_API void *
eina_file_map_all(Eina_File *file, Eina_File_Populate rule EINA_UNUSED)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);

   if (file->virtual) return eina_file_virtual_map_all(file);

   eina_lock_take(&file->lock);
   if (file->global_map == MAP_FAILED)
     {
        DWORD max_size_high;
        DWORD max_size_low;
        HANDLE fm;


        max_size_high = (DWORD)((file->length & 0xffffffff00000000ULL) >> 32);
        max_size_low = (DWORD)(file->length & 0x00000000ffffffffULL);
        fm = CreateFileMapping(file->handle, NULL, PAGE_READONLY,
                                     max_size_high, max_size_low, NULL);
        if (!fm)
          {
             eina_lock_release(&file->lock);
             return NULL;
          }

        file->global_map = MapViewOfFile(fm, FILE_MAP_READ,
                             0, 0, file->length);
        CloseHandle(fm);
        if (!file->global_map)
          file->global_map = MAP_FAILED;
     }

   if (file->global_map != MAP_FAILED)
     {
        file->global_refcount++;
        eina_lock_release(&file->lock);
        return file->global_map;
     }

   eina_lock_release(&file->lock);
   return NULL;
}

EINA_API void *
eina_file_map_new(Eina_File *file, Eina_File_Populate rule,
                  unsigned long int offset, unsigned long int length)
{
   Eina_File_Map *map;
   unsigned long int key[2];

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);

   if (offset > file->length)
     return NULL;
   if (offset + length > file->length)
     return NULL;

   if (offset == 0UL && length == file->length)
     return eina_file_map_all(file, rule);

   if (file->virtual)
     return eina_file_virtual_map_new(file, offset, length);

   key[0] = offset;
   key[1] = length;

   eina_lock_take(&file->lock);

   map = eina_hash_find(file->map, &key);
   if (!map)
     {
        SYSTEM_INFO si;
        HANDLE fm;
        __int64 map_size;
        DWORD view_offset;
        DWORD view_length;
        DWORD granularity;

        map = malloc(sizeof (Eina_File_Map));
        if (!map)
          goto on_error;

        /*
         * the size of the mapping object is the offset plus the length,
         * which might be greater than a DWORD
         */
        map_size = (__int64)offset + (__int64)length;
        fm = CreateFileMapping(file->handle, NULL, PAGE_READONLY,
                               (DWORD)((map_size >> 32) & 0x00000000ffffffffULL),
                               (DWORD)(map_size & 0x00000000ffffffffULL),
                               NULL);
        if (!fm)
          goto on_error;

        /*
         * get the system allocation granularity as the
         * offset passed to MapViewOfFile() must be a
         * multiple of this granularity
         */
        GetSystemInfo(&si);
        granularity = si.dwAllocationGranularity;

        /*
         * view_offset is the greatest multiple of granularity, less or equal
         * than offset (and can be stored in a DWORD)
         */
        view_offset = (offset / granularity) * granularity;
        view_length = (offset - view_offset) + length;
        map->map = MapViewOfFile(fm, FILE_MAP_READ,
                                 0,
                                 view_offset,
                                 view_length);
        CloseHandle(fm);
        if (!map->map)
          goto on_error;

        map->ret = (unsigned char *)map->map + (offset - view_offset);
        map->offset = offset;
        map->length = length;
        map->refcount = 0;

        eina_hash_add(file->map, &key, map);
        eina_hash_direct_add(file->rmap, map->map, map);
     }

   map->refcount++;

   eina_lock_release(&file->lock);

   return map->ret;

 on_error:
   free(map);
   eina_lock_release(&file->lock);

   return NULL;
}

EINA_API void
eina_file_map_free(Eina_File *file, void *map)
{
   EINA_SAFETY_ON_NULL_RETURN(file);

   if (file->virtual)
     {
        eina_file_virtual_map_free(file, map);
        return;
     }

   eina_lock_take(&file->lock);

   if (file->global_map == map)
     {
        file->global_refcount--;

        if (file->global_refcount > 0) goto on_exit;

        UnmapViewOfFile(file->global_map);
        file->global_map = MAP_FAILED;
     }
   else
     {
        eina_file_common_map_free(file, map, _eina_file_map_close);
     }

 on_exit:
   eina_lock_release(&file->lock);
}

EINA_API Eina_Bool
eina_file_map_faulted(Eina_File *file, void *map EINA_UNUSED)
{
#warning "We need to handle access to corrupted memory mapped file."
  if (file->virtual) return EINA_FALSE;
  /*
   * FIXME:
   * vc++ : http://msdn.microsoft.com/en-us/library/windows/desktop/aa366801%28v=vs.85%29.aspx
   *
   * mingw-w64 :
   * - 32 bits : there is a way to implement __try/__except/__final in C.
   *   see excpt.h header for 32-bits
   * - 64 bits : some inline assembly required for it.  See as example our
   *   startup-code in WinMainCRTStartup() in crtexe.c :
{
  int ret = 255;
#ifdef __SEH__
  asm ("\t.l_startw:\n"
    "\t.seh_handler __C_specific_handler, @except\n"
    "\t.seh_handlerdata\n"
    "\t.long 1\n"
    "\t.rva .l_startw, .l_endw, _gnu_exception_handler ,.l_endw\n"
    "\t.text"
    );
#endif
  mingw_app_type = 1;
  __security_init_cookie ();
  ret = __tmainCRTStartup ();
#ifdef __SEH__
  asm ("\tnop\n"
    "\t.l_endw: nop\n");
#endif
  return ret;
}
   */
   return EINA_FALSE;
}

EINA_API int
eina_file_statat(void *container EINA_UNUSED, Eina_File_Direct_Info *info, Eina_Stat *st)
{
   struct __stat64 buf;

   EINA_SAFETY_ON_NULL_RETURN_VAL(info, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(st, -1);

   if (_stat64(info->path, &buf))
     {
        info->type = EINA_FILE_UNKNOWN;
        return -1;
     }

   if (info->type == EINA_FILE_UNKNOWN)
     {
        if (S_ISREG(buf.st_mode))
          info->type = EINA_FILE_REG;
        else if (S_ISDIR(buf.st_mode))
          info->type = EINA_FILE_DIR;
        else
          info->type = EINA_FILE_UNKNOWN;
     }

   st->dev = buf.st_dev;
   st->ino = buf.st_ino;
   st->mode = buf.st_mode;
   st->nlink = buf.st_nlink;
   st->uid = buf.st_uid;
   st->gid = buf.st_gid;
   st->rdev = buf.st_rdev;
   st->size = buf.st_size;
   st->blksize = 0;
   st->blocks = 0;
   st->atime = buf.st_atime;
   st->mtime = buf.st_mtime;
   st->ctime = buf.st_ctime;
   st->atimensec = 0;
   st->mtimensec = 0;
   st->ctimensec = 0;

   return 0;
}

EINA_API int
eina_file_mkstemp(const char *templatename, Eina_Tmpstr **path)
{
   char buffer[PATH_MAX];
   const char *XXXXXX = NULL, *sep;
   int fd, len;

   EINA_SAFETY_ON_NULL_RETURN_VAL(templatename, -1);

   sep = strchr(templatename, '/');
   if (!sep) sep = strchr(templatename, '\\');
   if (sep)
     {
        len = eina_strlcpy(buffer, templatename, sizeof(buffer));
     }
   else
     {
        len = eina_file_path_join(buffer, sizeof(buffer),
                                  eina_environment_tmp_get(), templatename);
     }

   if ((XXXXXX = strstr(buffer, "XXXXXX.")) != NULL)
     fd = _eina_file_mkstemps(buffer, buffer + len - XXXXXX - 6);
   else
     fd = _eina_file_mkstemps(buffer, 0);

   if (fd < 0)
     {
        if (path) *path = NULL;
        return -1;
     }

   if (path) *path = eina_tmpstr_add(buffer);
   return fd;
}

EINA_API Eina_Bool
eina_file_mkdtemp(const char *templatename, Eina_Tmpstr **path)
{
   char buffer[PATH_MAX];
   char *tmpdirname, *sep;

   EINA_SAFETY_ON_NULL_RETURN_VAL(templatename, EINA_FALSE);

   sep = strchr(templatename, '/');
   if (!sep) sep = strchr(templatename, '\\');
   if (sep)
     {
        eina_strlcpy(buffer, templatename, sizeof(buffer));
     }
   else
     {
        eina_file_path_join(buffer, sizeof(buffer),
                            eina_environment_tmp_get(), templatename);
     }

   tmpdirname = _eina_file_mkdtemp(buffer);
   if (tmpdirname == NULL)
     {
        if (path) *path = NULL;
        return EINA_FALSE;
     }

   if (path) *path = eina_tmpstr_add(tmpdirname);
   return EINA_TRUE;
}

EINA_API Eina_Bool
eina_file_access(const char *path, Eina_File_Access_Mode mode)
{
   DWORD attr;

   if (!path || !*path)
     return EINA_FALSE;

   if ((mode != EINA_FILE_ACCESS_MODE_EXIST) &&
       ((mode >> 3) != 0))
     return EINA_FALSE;

   /*
    * Always check for existence for both files and directories
   */
   attr = GetFileAttributes(path);
   if (attr == INVALID_FILE_ATTRIBUTES)
     return EINA_FALSE;

   /*
    * On Windows a file or path is either read/write or read only.
    * So if it exists, it has at least read access.
    * So do something only if mode is EXEC or WRITE
    */

   if (mode & EINA_FILE_ACCESS_MODE_EXEC)
     {
        if (!(attr & FILE_ATTRIBUTE_DIRECTORY) &&
            !eina_str_has_extension(path, ".exe") &&
            !eina_str_has_extension(path, ".bat"))
          return EINA_FALSE;
     }

   if (mode & EINA_FILE_ACCESS_MODE_WRITE)
     {
        if (attr == INVALID_FILE_ATTRIBUTES)
          return EINA_FALSE;

        if (attr & FILE_ATTRIBUTE_READONLY)
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

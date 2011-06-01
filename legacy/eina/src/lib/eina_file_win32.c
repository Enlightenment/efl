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

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

//#include <Evil.h>

#include "eina_config.h"
#include "eina_private.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_file.h"
#include "eina_stringshare.h"
#include "eina_hash.h"
#include "eina_list.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#ifndef EINA_LOG_COLOR_DEFAULT
#define EINA_LOG_COLOR_DEFAULT EINA_COLOR_CYAN
#endif

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_file_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eina_file_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_file_log_dom, __VA_ARGS__)

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

struct _Eina_File
{
   const char *filename;

   Eina_Hash *map;
   Eina_Hash *rmap;
   void *global_map;

   ULONGLONG length;
   ULONGLONG mtime;

   int refcount;
   int global_refcount;

   HANDLE handle;
   HANDLE fm;

   Eina_Bool shared : 1;
   Eina_Bool delete_me : 1;
};

struct _Eina_File_Map
{
   void *map;

   unsigned long int offset;
   unsigned long int length;

   int refcount;
};

static Eina_Hash *_eina_file_cache = NULL;
static Eina_List *_eina_file_cache_lru = NULL;
static Eina_List *_eina_file_cache_delete = NULL;

static int _eina_file_log_dom = -1;

static void
_eina_file_win32_backslash_change(char *dir)
{
   char *tmp;

   tmp = dir;
   while (*tmp)
     {
        if (*tmp == '/') *tmp = '\\';
        tmp++;
     }
}

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

   if (attr == 0xFFFFFFFF)
     return EINA_FALSE;

   if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
     return EINA_FALSE;

   return EINA_TRUE;
}

static char *
_eina_file_win32_dir_new(const char *dir)
{
   char *new_dir;
   size_t length;

   length = strlen(dir);

   new_dir = (char *)malloc(sizeof(char) * length + 5);
   if (!new_dir)
     return NULL;

   memcpy(new_dir, dir, length);
   memcpy(new_dir + length, "\\*.*", 5);
   _eina_file_win32_backslash_change(new_dir);

   return new_dir;
}

static HANDLE
_eina_file_win32_first_file(const char *dir, WIN32_FIND_DATA *fd)
{
  HANDLE h;
#ifdef UNICODE
   wchar_t *wdir = NULL;

   wdir = evil_char_to_wchar(dir);
   if (!wdir)
     return NULL;

   h = FindFirstFile(wdir, fd);
   free(wdir);
#else
   h = FindFirstFile(dir, fd);
#endif

   if (!h)
     return NULL;

   while ((fd->cFileName[0] == '.') &&
          ((fd->cFileName[1] == '\0') ||
           ((fd->cFileName[1] == '.') && (fd->cFileName[2] == '\0'))))
     {
        if (!FindNextFile(h, fd))
          return NULL;
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
     return EINA_FALSE;

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
     return EINA_FALSE;

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
   else if (attr & FILE_ATTRIBUTE_REPARSE_POINT)
     it->info.type = EINA_FILE_LNK;
   else if (attr & (FILE_ATTRIBUTE_ARCHIVE |
                    FILE_ATTRIBUTE_COMPRESSED |
                    FILE_ATTRIBUTE_COMPRESSED |
                    FILE_ATTRIBUTE_HIDDEN |
                    FILE_ATTRIBUTE_NORMAL |
                    FILE_ATTRIBUTE_SPARSE_FILE |
                    FILE_ATTRIBUTE_TEMPORARY))
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

static void
_eina_file_real_close(Eina_File *file)
{
   eina_hash_free(file->rmap);
   eina_hash_free(file->map);

   if (file->global_map != MAP_FAILED)
     UnmapViewOfFile(file->global_map);

   CloseHandle(file->fm);
   CloseHandle(file->handle);

   eina_stringshare_del(file->filename);

   free(file);
}

static void
_eina_file_map_close(Eina_File_Map *map)
{
   if (map->map != MAP_FAILED)
     UnmapViewOfFile(map->map);
   free(map);
}

static unsigned int
_eina_file_map_key_length(const void *key __UNUSED__)
{
   return sizeof (unsigned long int) * 2;
}

static int
_eina_file_map_key_cmp(const unsigned long int *key1, int key1_length __UNUSED__,
                       const unsigned long int *key2, int key2_length __UNUSED__)
{
   if (key1[0] - key2[0] == 0) return key1[1] - key2[1];
   return key1[0] - key2[0];
}

static int
_eina_file_map_key_hash(const unsigned long int *key, int key_length __UNUSED__)
{
   return eina_hash_int64(&key[0], sizeof (unsigned long int))
     ^ eina_hash_int64(&key[1], sizeof (unsigned long int));
}

Eina_Bool
eina_file_init(void)
{
   _eina_file_log_dom = eina_log_domain_register("eina_file",
                                                 EINA_LOG_COLOR_DEFAULT);
   if (_eina_file_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_file");
        return EINA_FALSE;
     }

   _eina_file_cache = eina_hash_string_djb2_new(EINA_FREE_CB(_eina_file_real_close));
   if (!_eina_file_cache)
     {
        ERR("Could not create cache.");
        eina_log_domain_unregister(_eina_file_log_dom);
        _eina_file_log_dom = -1;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool
eina_file_shutdown(void)
{
   Eina_File *f;
   Eina_List *l;

   EINA_LIST_FREE(_eina_file_cache_delete, f)
     _eina_file_real_close(f);

   EINA_LIST_FOREACH(_eina_file_cache_lru, l, f)
     eina_hash_del(_eina_file_cache, f->filename, f);

   if (eina_hash_population(_eina_file_cache) > 0)
     {
        Eina_Iterator *it;
        const char *key;

        it = eina_hash_iterator_key_new(_eina_file_cache);
        EINA_ITERATOR_FOREACH(it, key)
          ERR("File [%s] still open !", key);
        eina_iterator_free(it);
     }

   eina_hash_free(_eina_file_cache);

   eina_log_domain_unregister(_eina_file_log_dom);
   _eina_file_log_dom = -1;
   return EINA_TRUE;
}


/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI Eina_Bool
eina_file_dir_list(const char *dir,
                   Eina_Bool recursive,
                   Eina_File_Dir_List_Cb cb,
                   void *data)
{
   WIN32_FIND_DATA file;
   HANDLE h;
   char *new_dir;

   EINA_SAFETY_ON_NULL_RETURN_VAL(cb,  EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(dir[0] == '\0', EINA_FALSE);

   if (!_eina_file_win32_is_dir(dir))
     return EINA_FALSE;

   new_dir = _eina_file_win32_dir_new(dir);
   if (!new_dir)
      return EINA_FALSE;

   h = _eina_file_win32_first_file(new_dir, &file);

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
        if (!strcmp(filename, ".") || !strcmp(filename, ".."))
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

EAPI Eina_Array *
eina_file_split(char *path)
{
   Eina_Array *ea;
   char *current;
   size_t length;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);

   ea = eina_array_new(16);

   if (!ea)
      return NULL;

   for (current = strchr(path, '\\');
        current;
        path = current + 1, current = strchr(path, '\\'))
     {
        length = current - path;

        if (length <= 0)
           continue;

        eina_array_push(ea, path);
        *current = '\0';
     }

   if (*path != '\0')
        eina_array_push(ea, path);

   return ea;
}

EAPI Eina_Iterator *
eina_file_ls(const char *dir)
{
   Eina_File_Iterator *it;
   char               *new_dir;
   size_t              length;

   if (!dir || !*dir)
      return NULL;

   if (!_eina_file_win32_is_dir(dir))
     return NULL;

   length = strlen(dir);

   it = calloc(1, sizeof (Eina_File_Iterator) + length);
   if (!it)
      return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   new_dir = _eina_file_win32_dir_new(dir);
   if (!new_dir)
      goto free_it;

   it->handle = _eina_file_win32_first_file(new_dir, &it->data);
   free(new_dir);
   if (it->handle == INVALID_HANDLE_VALUE)
     goto free_it;

   memcpy(it->dir, dir, length + 1);
   if (dir[length - 1] != '\\')
      it->length = length;
   else
      it->length = length - 1;
   _eina_file_win32_backslash_change(it->dir);

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_file_win32_ls_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_eina_file_win32_ls_iterator_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_file_win32_ls_iterator_free);

   return &it->iterator;

 free_it:
   free(it);

   return NULL;
}

EAPI Eina_Iterator *
eina_file_direct_ls(const char *dir)
{
   Eina_File_Direct_Iterator *it;
   char                      *new_dir;
   size_t                     length;

   if (!dir || !*dir)
      return NULL;

   length = strlen(dir);

   if (length + 12 + 2 >= MAX_PATH)
      return NULL;

   it = calloc(1, sizeof(Eina_File_Direct_Iterator) + length);
   if (!it)
      return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   new_dir = _eina_file_win32_dir_new(dir);
   if (!new_dir)
      goto free_it;

   it->handle = _eina_file_win32_first_file(new_dir, &it->data);
   free(new_dir);
   if (it->handle == INVALID_HANDLE_VALUE)
     goto free_it;

   memcpy(it->dir, dir, length + 1);
   it->length = length;
   _eina_file_win32_backslash_change(it->dir);

   memcpy(it->info.path, dir, length);
   if (dir[length - 1] == '\\')
      it->info.name_start = length;
   else
     {
        it->info.path[length] = '\\';
        it->info.name_start = length + 1;
     }
   _eina_file_win32_backslash_change(it->info.path);

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_file_win32_direct_ls_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_eina_file_win32_direct_ls_iterator_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_file_win32_direct_ls_iterator_free);

   return &it->iterator;

 free_it:
   free(it);

   return NULL;
}

EAPI Eina_Iterator *
eina_file_stat_ls(const char *dir)
{
   return eina_file_direct_ls(dir);
}

EAPI Eina_File *
eina_file_open(const char *filename, Eina_Bool shared)
{
   Eina_File *file;
   Eina_File *n;
   HANDLE handle;
   HANDLE fm;
   WIN32_FILE_ATTRIBUTE_DATA fad;
   ULARGE_INTEGER length;
   ULARGE_INTEGER mtime;
   Eina_Bool create = EINA_FALSE;

   /* FIXME: always open absolute path (need to fix filename according to current
      directory) */

   /* FIXME: how to emulate shm_open ? Just OpenFileMapping ? */
#if 0
   if (shared)
     handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ,
                         NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY,
                         NULL);
   else
#endif
     handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ,
                         NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY,
                         NULL);

   if (handle == INVALID_HANDLE_VALUE)
     return NULL;

   fm = CreateFileMapping(handle, NULL, PAGE_READONLY, 0, 0, NULL);
   if (!fm)
     goto close_handle;

   if (!GetFileAttributesEx(filename, GetFileExInfoStandard, &fad))
     goto close_fm;

   length.u.LowPart = fad.nFileSizeLow;
   length.u.HighPart = fad.nFileSizeHigh;
   mtime.u.LowPart = fad.ftLastWriteTime.dwLowDateTime;
   mtime.u.HighPart = fad.ftLastWriteTime.dwHighDateTime;

   file = eina_hash_find(_eina_file_cache, filename);
   if (file &&
       (file->mtime != mtime.QuadPart || file->length != length.QuadPart))
     {
        create = EINA_TRUE;

        if (file->refcount == 0)
          {
             _eina_file_cache_lru = eina_list_prepend(_eina_file_cache_lru, file);
             eina_hash_del(_eina_file_cache, file->filename, file);

             file = NULL;
          }
        else if (!file->delete_me)
          {
             file->delete_me = EINA_TRUE;
             _eina_file_cache_delete = eina_list_prepend(_eina_file_cache_delete, file);
          }
     }

   if (!file || create)
     {
        n = calloc(1, sizeof (Eina_File));
        if (!n)
          goto close_fm;

        n->filename = eina_stringshare_add(filename);
        n->map = eina_hash_new(EINA_KEY_LENGTH(_eina_file_map_key_length),
                               EINA_KEY_CMP(_eina_file_map_key_cmp),
                               EINA_KEY_HASH(_eina_file_map_key_hash),
                               EINA_FREE_CB(_eina_file_map_close),
                               3);
        n->rmap = eina_hash_pointer_new(NULL);
        n->global_map = MAP_FAILED;
        n->length = length.QuadPart;
        n->mtime = mtime.QuadPart;
        n->refcount = 0;
        n->handle = handle;
        n->fm = fm;
        n->shared = shared;
        n->delete_me = EINA_FALSE;

        eina_hash_set(_eina_file_cache, filename, n);
     }
   else
     {
        CloseHandle(fm);
        CloseHandle(handle);

        n = file;

        if (n->refcount == 0)
          _eina_file_cache_lru = eina_list_remove(_eina_file_cache_lru, n);
     }

   n->refcount++;

   return n;

 close_fm:
   CloseHandle(fm);
 close_handle:
   CloseHandle(handle);

   return NULL;
}

EAPI void
eina_file_close(Eina_File *file)
{
   file->refcount--;

   if (file->refcount != 0) return ;

   if (file->delete_me)
     {
        _eina_file_cache_delete = eina_list_remove(_eina_file_cache_delete, file);
        _eina_file_real_close(file);
     }
   else
     {
        _eina_file_cache_lru = eina_list_prepend(_eina_file_cache_lru, file);
     }
}

EAPI unsigned long int
eina_file_size_get(Eina_File *file)
{
   return file->length;
}

EAPI time_t
eina_file_mtime_get(Eina_File *file)
{
  return file->mtime;
}

EAPI const char *
eina_file_filename_get(Eina_File *file)
{
   return file->filename;
}

EAPI void *
eina_file_map_all(Eina_File *file, Eina_File_Populate rule __UNUSED__)
{
   if (file->global_map == MAP_FAILED)
     {
        void  *data;

        data = MapViewOfFile(file->fm, FILE_MAP_READ,
                             0, 0, file->length);
        if (!data)
          file->global_map = MAP_FAILED;
        else
          file->global_map = data;
     }

   if (file->global_map != MAP_FAILED)
     {
        file->global_refcount++;
        return file->global_map;
     }

   return NULL;
}

EAPI void *
eina_file_map_new(Eina_File *file, Eina_File_Populate rule,
                  unsigned long int offset, unsigned long int length)
{
   Eina_File_Map *map;
   unsigned long int key[2];

   if (offset > file->length)
     return NULL;
   if (offset + length > file->length)
     return NULL;

   if (offset == 0 && length == file->length)
     return eina_file_map_all(file, rule);

   key[0] = offset;
   key[1] = length;

   map = eina_hash_find(file->map, &key);
   if (!map)
     {
        void  *data;

        map = malloc(sizeof (Eina_File_Map));
        if (!map) return NULL;

        data = MapViewOfFile(file->fm, FILE_MAP_READ,
                             offset & 0xffff0000,
                             offset & 0x0000ffff,
                             length);
        if (!data)
          map->map = MAP_FAILED;
        else
          map->map = data;

        map->offset = offset;
        map->length = length;
        map->refcount = 0;

        if (map->map == MAP_FAILED)
          {
             free(map);
             return NULL;
          }

        eina_hash_add(file->map, &key, map);
        eina_hash_direct_add(file->rmap, map->map, map);
     }

   map->refcount++;

   return map->map;
}

EAPI void
eina_file_map_free(Eina_File *file, void *map)
{
   if (file->global_map == map)
     {
        file->global_refcount--;

        if (file->global_refcount > 0) return ;

        /* FIXME: are we sure that file->global_map != MAP_FAILED ? */
        if (file->global_map != MAP_FAILED)
          UnmapViewOfFile(file->global_map);
        file->global_map = MAP_FAILED;
     }
   else
     {
        Eina_File_Map *em;
        unsigned long int key[2];

        em = eina_hash_find(file->rmap, &map);
        if (!em) return ;

        em->refcount--;

        if (em->refcount > 0) return ;

        key[0] = em->offset;
        key[1] = em->length;

        eina_hash_del(file->rmap, &map, em);
        eina_hash_del(file->map, &key, em);
     }
}

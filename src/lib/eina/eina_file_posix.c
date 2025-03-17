/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga, Vincent Torri
 * Copyright (C) 2010-2011 Cedric Bail
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

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#ifdef HAVE_DIRENT_H
# include <dirent.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif
#include <fcntl.h>

#if defined(__linux__)
# include <sys/syscall.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif

#define PATH_DELIM '/'

#include "eina_config.h"
#include "eina_private.h"
#include "eina_alloca.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_cpu.h"
#include "eina_file.h"
#include "eina_stringshare.h"
#include "eina_hash.h"
#include "eina_list.h"
#include "eina_lock.h"
#include "eina_mmap.h"
#include "eina_log.h"
#include "eina_xattr.h"
#include "eina_file_common.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#define EINA_SMALL_PAGE eina_cpu_page_size()

// FIXME: This assumes HugeTLB size of 2Mb. How to get this information at runtime?
#define EINA_HUGE_PAGE (2 * 1024 * 1024)
#define EINA_HUGE_PAGE_MIN (8 * EINA_HUGE_PAGE)

#ifdef HAVE_DIRENT_H
typedef struct _Eina_File_Iterator Eina_File_Iterator;
struct _Eina_File_Iterator
{
   Eina_Iterator iterator;

   DIR *dirp;
   int length;

   char dir[1];
};
#endif

int _eina_file_log_dom = -1;

/*
 * This complex piece of code is needed due to possible race condition.
 * The code and description of the issue can be found at :
 * http://womble.decadent.org.uk/readdir_r-advisory.html
 */
#ifdef HAVE_DIRENT_H
static long
_eina_name_max(DIR *dirp EINA_UNUSED)
{
   long name_max;

#if defined(HAVE_FPATHCONF) && defined(HAVE_DIRFD) && defined(_PC_NAME_MAX)
   name_max = fpathconf(dirfd(dirp), _PC_NAME_MAX);

   if (name_max == -1)
     {
# if defined(NAME_MAX)
        name_max = (NAME_MAX > 255) ? NAME_MAX : 255;
# else
        name_max = PATH_MAX;
# endif
     }
#else
# if defined(NAME_MAX)
   name_max = (NAME_MAX > 255) ? NAME_MAX : 255;
# else
#  ifdef _PC_NAME_MAX
#   warning "buffer size for readdir_r cannot be determined safely, best effort, but racy"
   name_max = pathconf(dirp, _PC_NAME_MAX);
#  else
#   error "buffer size for readdir_r cannot be determined safely"
#  endif
# endif
#endif

   return name_max;
}

static Eina_Bool
_eina_file_ls_iterator_next(Eina_File_Iterator *it, void **data)
{
   struct dirent *dp;
   char *name;
   size_t length;

   do
     {
        dp = readdir(it->dirp);
        if (dp == NULL)
          return EINA_FALSE;
     }
   while ((dp->d_name[0] == '.') &&
          ((dp->d_name[1] == '\0') ||
           ((dp->d_name[1] == '.') && (dp->d_name[2] == '\0'))));

#ifdef _DIRENT_HAVE_D_NAMLEN
   length = dp->d_namlen;
#else
   length = strlen(dp->d_name);
#endif
   name = alloca(length + 2 + it->length);

   memcpy(name,                  it->dir,    it->length);
   memcpy(name + it->length,     "/",        1);
   memcpy(name + it->length + 1, dp->d_name, length + 1);

   *data = (char *)eina_stringshare_add(name);
   return EINA_TRUE;
}

static DIR *
_eina_file_ls_iterator_container(Eina_File_Iterator *it)
{
   return it->dirp;
}

static void
_eina_file_ls_iterator_free(Eina_File_Iterator *it)
{
   closedir(it->dirp);

   EINA_MAGIC_SET(&it->iterator, 0);
   free(it);
}

typedef struct _Eina_File_Direct_Iterator Eina_File_Direct_Iterator;
struct _Eina_File_Direct_Iterator
{
   Eina_Iterator iterator;

   DIR *dirp;
   int length;

   Eina_File_Direct_Info info;

   char dir[1];
};

static Eina_Bool
_eina_file_direct_ls_iterator_next(Eina_File_Direct_Iterator *it, void **data)
{
   struct dirent *dp;
   size_t length;

   do
     {
        dp = readdir(it->dirp);
        if (dp == NULL)
          return EINA_FALSE;

#ifdef _DIRENT_HAVE_D_NAMLEN
        length = dp->d_namlen;
#else
        length = strlen(dp->d_name);
#endif
        if (it->info.name_start + length + 1 >= EINA_PATH_MAX)
           continue;
     }
   while ((dp->d_name[0] == '.') &&
          ((dp->d_name[1] == '\0') ||
           ((dp->d_name[1] == '.') && (dp->d_name[2] == '\0'))));

   memcpy(it->info.path + it->info.name_start, dp->d_name, length);
   it->info.name_length = length;
   it->info.path_length = it->info.name_start + length;
   it->info.path[it->info.path_length] = '\0';

#ifdef _DIRENT_HAVE_D_TYPE
   switch (dp->d_type)
     {
     case DT_FIFO:
       it->info.type = EINA_FILE_FIFO;
       break;
     case DT_CHR:
       it->info.type = EINA_FILE_CHR;
       break;
     case DT_DIR:
       it->info.type = EINA_FILE_DIR;
       break;
     case DT_BLK:
       it->info.type = EINA_FILE_BLK;
       break;
     case DT_REG:
       it->info.type = EINA_FILE_REG;
       break;
     case DT_LNK:
       it->info.type = EINA_FILE_LNK;
       break;
     case DT_SOCK:
       it->info.type = EINA_FILE_SOCK;
       break;
     case DT_WHT:
       it->info.type = EINA_FILE_WHT;
       break;
     default:
       it->info.type = EINA_FILE_UNKNOWN;
       break;
     }
#else
   it->info.type = EINA_FILE_UNKNOWN;
#endif

   *data = &it->info;
   return EINA_TRUE;
}

static DIR *
_eina_file_direct_ls_iterator_container(Eina_File_Direct_Iterator *it)
{
   return it->dirp;
}

static void
_eina_file_direct_ls_iterator_free(Eina_File_Direct_Iterator *it)
{
   closedir(it->dirp);

   EINA_MAGIC_SET(&it->iterator, 0);
   free(it);
}

static Eina_Bool
_eina_file_stat_ls_iterator_next(Eina_File_Direct_Iterator *it, void **data)
{
   Eina_Stat st;

   if (!_eina_file_direct_ls_iterator_next(it, data))
     return EINA_FALSE;

   if (it->info.type == EINA_FILE_UNKNOWN)
     {
        if (eina_file_statat(it->dirp, &it->info, &st) != 0)
          it->info.type = EINA_FILE_UNKNOWN;
     }

   return EINA_TRUE;
}
#endif

void
eina_file_real_close(Eina_File *file)
{
   Eina_File_Map *map;

   EINA_LIST_FREE(file->dead_map, map)
     {
        munmap(map->map, map->length);
        free(map);
     }

   if (file->fd != -1)
     {
        if (!file->copied && file->global_map != MAP_FAILED)
          munmap(file->global_map, file->length);
        close(file->fd);
     }
}

static void
_eina_file_map_close(Eina_File_Map *map)
{
   munmap(map->map, map->length);
   free(map);
}

#ifndef MAP_POPULATE
static unsigned int
_eina_file_map_populate(char *map, unsigned long int size, Eina_Bool hugetlb)
{
   unsigned int r = 0xDEADBEEF;
   unsigned long int i;
   unsigned int s;

   if (size == 0) return 0;

   s = hugetlb ? EINA_HUGE_PAGE : EINA_SMALL_PAGE;

   for (i = 0; i < size; i += s)
     r ^= map[i];

   r ^= map[size - 1];

   return r;
}
#endif

static char *
_page_aligned_address(const char *map, unsigned long int offset, Eina_Bool hugetlb)
{
   const uintptr_t align = hugetlb ? EINA_HUGE_PAGE : EINA_SMALL_PAGE;
   uintptr_t pmap = (uintptr_t) map;

   pmap = (pmap + offset) - ((pmap + offset) & (align - 1));

   return (char *) pmap;
}

static int
_eina_file_map_rule_apply(Eina_File_Populate rule, const void *map, unsigned long int offset,
                          unsigned long int size, unsigned long long maplen, Eina_Bool hugetlb)
{
   int tmp = 42;
   int flag = MADV_RANDOM;
   char *addr;

   switch (rule)
     {
      case EINA_FILE_RANDOM: flag = MADV_RANDOM; break;
      case EINA_FILE_SEQUENTIAL: flag = MADV_SEQUENTIAL; break;
      case EINA_FILE_POPULATE: flag = MADV_WILLNEED; break;
      case EINA_FILE_WILLNEED: flag = MADV_WILLNEED; break;
      case EINA_FILE_DONTNEED: flag = MADV_DONTNEED; break;
#ifdef MADV_REMOVE
      case EINA_FILE_REMOVE: flag = MADV_REMOVE; break;
#elif defined (MADV_FREE)
      case EINA_FILE_REMOVE: flag = MADV_FREE; break;
#else
# warning "EINA_FILE_REMOVE does not have system support"
#endif
      default: return tmp; break;
     }

   if (offset >= maplen) return tmp;

   // Align address, clamp size
   addr = _page_aligned_address(map, offset, hugetlb);
   if (size > 0)
     {
        size += ((char *) map + offset) - addr;
        offset -= ((char *) map + offset) - addr;
        if ((offset + size) > maplen)
          {
             if (offset > maplen) return tmp;
             size = maplen - offset;
          }
     }

   madvise(addr, size, flag);

#ifndef MAP_POPULATE
   if (rule == EINA_FILE_POPULATE)
     tmp ^= _eina_file_map_populate(addr, size, hugetlb);
#else
   (void) hugetlb;
#endif

   return tmp;
}

static Eina_Bool
_eina_file_timestamp_compare(Eina_File *f, struct stat *st)
{
   if (f->mtime != st->st_mtime) return EINA_FALSE;
   if (f->length != (unsigned long long) st->st_size) return EINA_FALSE;
   if (f->inode != st->st_ino) return EINA_FALSE;
#ifdef _STAT_VER_LINUX
# ifdef st_mtime
   if (f->mtime_nsec != (unsigned long int)st->st_mtim.tv_nsec)
     return EINA_FALSE;
# else
   if (f->mtime_nsec != (unsigned long int)st->st_mtimensec)
     return EINA_FALSE;
# endif
#endif
   return EINA_TRUE;
}

static void
slprintf(char *str, size_t size, const char *format, ...)
{
   va_list ap;

   va_start(ap, format);

   vsnprintf(str, size, format, ap);
   str[size - 1] = 0;

   va_end(ap);
}

/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

static Eina_Bool
_eina_file_mmap_faulty_one(void *addr, long page_size,
                           Eina_File_Map *m)
{
   if ((unsigned char *) addr < (((unsigned char *)m->map) + m->length) &&
       (((unsigned char *) addr) + page_size) >= (unsigned char *) m->map)
     {
        m->faulty = EINA_TRUE;
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

Eina_Bool
eina_file_mmap_faulty(void *addr, long page_size)
{
   Eina_File_Map *m;
   Eina_File *f;
   Eina_Iterator *itf;
   Eina_Iterator *itm;
   Eina_Bool faulty = EINA_FALSE;

   eina_lock_take(&_eina_file_lock_cache);

   itf = eina_hash_iterator_data_new(_eina_file_cache);
   EINA_ITERATOR_FOREACH(itf, f)
     {
        eina_lock_take(&f->lock);

        if (f->global_map != MAP_FAILED)
          {
             if ((unsigned char *)addr <
                 (((unsigned char *)f->global_map) + f->length) &&
                 (((unsigned char *)addr) + page_size) >=
                  (unsigned char *)f->global_map)
               {
                  f->global_faulty = EINA_TRUE;
                  faulty = EINA_TRUE;
               }
          }

        if (!faulty)
          {
             itm = eina_hash_iterator_data_new(f->map);
             EINA_ITERATOR_FOREACH(itm, m)
               {
                  faulty = _eina_file_mmap_faulty_one(addr, page_size, m);
                  if (faulty) break;
               }
             eina_iterator_free(itm);
          }

        if (!faulty)
          {
             Eina_List *l;

             EINA_LIST_FOREACH(f->dead_map, l, m)
               {
                  faulty = _eina_file_mmap_faulty_one(addr, page_size, m);
                  if (faulty) break;
               }
          }

        eina_lock_release(&f->lock);

        if (faulty) break;
     }
   eina_iterator_free(itf);

   eina_lock_release(&_eina_file_lock_cache);
   return faulty;
}

/* ================================================================ *
 *   Simplified logic for portability layer with eina_file_common   *
 * ================================================================ */

Eina_Tmpstr *
eina_file_current_directory_get(const char *path, size_t len)
{
  char cwd[PATH_MAX];
  char *tmp = NULL;

  tmp = getcwd(cwd, PATH_MAX);
  if (!tmp) return NULL;

  len += strlen(cwd) + 2;
  tmp = alloca(sizeof (char) * len);

  slprintf(tmp, len, "%s/%s", cwd, path);

  return eina_tmpstr_add_length(tmp, len);
}

char *
eina_file_cleanup(Eina_Tmpstr *path)
{
   char *result;

   result = strdup(path ? path : "");
   eina_tmpstr_del(path);

   return result;
}

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/



EINA_API Eina_Bool
eina_file_path_relative(const char *path)
{
   if (!path)
     return EINA_FALSE;

   return *path != '/';
}

EINA_API Eina_Bool
eina_file_dir_list(const char *dir,
                   Eina_Bool recursive,
                   Eina_File_Dir_List_Cb cb,
                   void *data)
{
   Eina_File_Direct_Info *info;
   Eina_Iterator *it;

   EINA_SAFETY_ON_NULL_RETURN_VAL(cb,  EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(dir[0] == '\0', EINA_FALSE);

   it = eina_file_stat_ls(dir);
   if (!it)
      return EINA_FALSE;

   EINA_ITERATOR_FOREACH(it, info)
     {
        cb(info->path + info->name_start, dir, data);

        if (recursive == EINA_TRUE && info->type == EINA_FILE_DIR)
          {
             eina_file_dir_list(info->path, recursive, cb, data);
          }
     }

   eina_iterator_free(it);

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

   for (current = strchr(path, PATH_DELIM);
        current;
        path = current + 1, current = strchr(path, PATH_DELIM))
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
#ifdef HAVE_DIRENT_H
   Eina_File_Iterator *it;
   size_t length;
   DIR *dirp;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, NULL);

   length = strlen(dir);
   if (length < 1)
      return NULL;

   dirp = opendir(dir);
   if (!dirp)
      return NULL;

   it = calloc(1, sizeof (Eina_File_Iterator) + length);
   if (EINA_UNLIKELY(!it))
     {
        closedir(dirp);
        return NULL;
     }

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->dirp = dirp;

   memcpy(it->dir, dir, length + 1);
   if (dir[length - 1] != '/')
      it->length = length;
   else
      it->length = length - 1;

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_file_ls_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
         _eina_file_ls_iterator_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_file_ls_iterator_free);

   return &it->iterator;
#else
   (void) dir;
   return NULL;
#endif
}

EINA_API Eina_Iterator *
eina_file_direct_ls(const char *dir)
{
#ifdef HAVE_DIRENT_H
   Eina_File_Direct_Iterator *it;
   size_t length;
   DIR *dirp;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, NULL);

   length = strlen(dir);
   if (length < 1)
      return NULL;

   dirp = opendir(dir);
   if (!dirp)
      return NULL;

   it = calloc(1, sizeof(Eina_File_Direct_Iterator) + length);
   if (EINA_UNLIKELY(!it))
     {
        closedir(dirp);
        return NULL;
     }

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->dirp = dirp;

   if (length + _eina_name_max(it->dirp) + 2 >= EINA_PATH_MAX)
     {
        _eina_file_direct_ls_iterator_free(it);
        return NULL;
     }

   memcpy(it->dir,       dir, length + 1);
   it->length = length;

   memcpy(it->info.path, dir, length);
   if (dir[length - 1] == '/')
      it->info.name_start = length;
   else
     {
        it->info.path[length] = '/';
        it->info.name_start = length + 1;
     }

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_file_direct_ls_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
         _eina_file_direct_ls_iterator_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_file_direct_ls_iterator_free);

   return &it->iterator;
#else
   (void) dir;
   return NULL;
#endif
}

EINA_API Eina_Iterator *
eina_file_stat_ls(const char *dir)
{
#ifdef HAVE_DIRENT_H
   Eina_File_Direct_Iterator *it;
   size_t length;
   DIR *dirp;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, NULL);

   length = strlen(dir);
   if (length < 1)
      return NULL;

   dirp = opendir(dir);
   if (!dirp)
      return NULL;

   it = calloc(1, sizeof(Eina_File_Direct_Iterator) + length);
   if (EINA_UNLIKELY(!it))
     {
        closedir(dirp);
        return NULL;
     }

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->dirp = dirp;

   if (length + _eina_name_max(it->dirp) + 2 >= EINA_PATH_MAX)
     {
        _eina_file_direct_ls_iterator_free(it);
        return NULL;
     }

   memcpy(it->dir,       dir, length + 1);
   it->length = length;

   memcpy(it->info.path, dir, length);
   if (dir[length - 1] == '/')
      it->info.name_start = length;
   else
     {
        it->info.path[length] = '/';
        it->info.name_start = length + 1;
     }

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_file_stat_ls_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
         _eina_file_direct_ls_iterator_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_file_direct_ls_iterator_free);

   return &it->iterator;
#else
   (void) dir;
   return NULL;
#endif
}

EINA_API Eina_File *
eina_file_open(const char *path, Eina_Bool shared)
{
   Eina_File *file;
   Eina_File *n;
   Eina_Stringshare *filename;
   struct stat file_stat;
   int fd = -1;
   Eina_Statgen statgen;

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);

   filename = eina_file_sanitize(path);
   if (!filename) return NULL;

   statgen = eina_file_statgen_get();
   eina_lock_take(&_eina_file_lock_cache);
   file = eina_hash_find(_eina_file_cache, filename);
   statgen = eina_file_statgen_get();
   if ((!file) || (file->statgen != statgen) || (statgen == 0))
     {
        if (shared)
          {
#ifdef HAVE_SHM_OPEN
             fd = shm_open(filename, O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
             if ((fd != -1)  && (!eina_file_close_on_exec(fd, EINA_TRUE)))
               goto on_error;
#else
             goto on_error;
#endif
          }
        else
          {
#ifdef HAVE_OPEN_CLOEXEC
             fd = open(filename, O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO | O_CLOEXEC);
#else
             fd = open(filename, O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
             if ((fd != -1)  && (!eina_file_close_on_exec(fd, EINA_TRUE)))
               goto on_error;
#endif
          }
        if (fd < 0) goto on_error;

        if (fstat(fd, &file_stat))
          goto on_error;
        if (file) file->statgen = statgen;

        if ((file) && !_eina_file_timestamp_compare(file, &file_stat))
          {
             file->delete_me = EINA_TRUE;
             eina_hash_del(_eina_file_cache, file->filename, file);
             file = NULL;
          }
     }

   if (!file)
     {
        n = malloc(sizeof(Eina_File));
        if (!n)
          goto on_error;

        memset(n, 0, sizeof(Eina_File));
        n->filename = filename;
        n->map = eina_hash_new(EINA_KEY_LENGTH(eina_file_map_key_length),
                               EINA_KEY_CMP(eina_file_map_key_cmp),
                               EINA_KEY_HASH(eina_file_map_key_hash),
                               EINA_FREE_CB(_eina_file_map_close),
                               3);
        n->rmap = eina_hash_pointer_new(NULL);
        n->global_map = MAP_FAILED;
        n->length = file_stat.st_size;
        n->mtime = file_stat.st_mtime;
#ifdef _STAT_VER_LINUX
        n->mtime_nsec = (unsigned long int)file_stat.st_mtim.tv_nsec;
#endif
        n->inode = file_stat.st_ino;
        n->fd = fd;
        n->shared = shared;
        eina_lock_new(&n->lock);
        eina_hash_direct_add(_eina_file_cache, n->filename, n);

	EINA_MAGIC_SET(n, EINA_FILE_MAGIC);
     }
   else
     {
        if (fd >= 0) close(fd);
        n = file;
     }
   eina_lock_take(&n->lock);
   n->refcount++;
   eina_lock_release(&n->lock);

   eina_lock_release(&_eina_file_lock_cache);

   return n;

 on_error:
   eina_lock_release(&_eina_file_lock_cache);
   INF("Could not open file [%s].", filename);
   eina_stringshare_del(filename);

   if (fd >= 0) close(fd);
   return NULL;
}

EINA_API Eina_Bool
eina_file_refresh(Eina_File *file)
{
   struct stat file_stat;
   Eina_Bool r = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);

   if (file->virtual) return EINA_FALSE;

   if (fstat(file->fd, &file_stat))
     return EINA_FALSE;

   if (file->length != (unsigned long int) file_stat.st_size)
     {
        eina_file_flush(file, file_stat.st_size);
        r = EINA_TRUE;
     }

   file->length = file_stat.st_size;
   file->mtime = file_stat.st_mtime;
#ifdef _STAT_VER_LINUX
   file->mtime_nsec = (unsigned long int)file_stat.st_mtim.tv_nsec;
#endif
   file->inode = file_stat.st_ino;

   return r;
}

EINA_API Eina_Bool
eina_file_unlink(const char *pathname)
{
   if ( unlink(pathname) < 0)
     {
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EINA_API void *
eina_file_map_all(Eina_File *file, Eina_File_Populate rule)
{
   int flags = MAP_SHARED;
   void *ret = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);

   if (file->virtual) return eina_file_virtual_map_all(file);

   // bsd people will lack this feature
#ifdef MAP_POPULATE
   if (rule == EINA_FILE_POPULATE) flags |= MAP_POPULATE;
#endif
#ifdef MAP_HUGETLB
   if (file->length >= EINA_HUGE_PAGE_MIN) flags |= MAP_HUGETLB;
#endif

   eina_mmap_safety_enabled_set(EINA_TRUE);
   eina_lock_take(&file->lock);
   if (file->global_map == MAP_FAILED)
     file->global_map = mmap(NULL, file->length, PROT_READ, flags, file->fd, 0);
#ifdef MAP_HUGETLB
   if ((file->global_map == MAP_FAILED) && (flags & MAP_HUGETLB))
     {
       flags &= ~MAP_HUGETLB;
       file->global_map = mmap(NULL, file->length, PROT_READ, flags, file->fd, 0);
     }
#endif

   if (file->global_map != MAP_FAILED)
     {
        Eina_Bool hugetlb = EINA_FALSE;

#ifdef MAP_HUGETLB
        hugetlb = !!(flags & MAP_HUGETLB);
#endif
        if (!file->global_refcount)
          file->global_hugetlb = hugetlb;
        else
          hugetlb = file->global_hugetlb;

        _eina_file_map_rule_apply(rule, file->global_map, 0, file->length, file->length, hugetlb);
        file->global_refcount++;
        ret = file->global_map;
     }

   eina_lock_release(&file->lock);
   return ret;
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

   if (offset == 0 && length == file->length)
     return eina_file_map_all(file, rule);

   if (file->virtual)
     return eina_file_virtual_map_new(file, offset, length);

   key[0] = offset;
   key[1] = length;

   eina_mmap_safety_enabled_set(EINA_TRUE);
   eina_lock_take(&file->lock);

   map = eina_hash_find(file->map, &key);
   if (!map)
     {
        int flags = MAP_SHARED;

// bsd people will lack this feature
#ifdef MAP_POPULATE
        if (rule == EINA_FILE_POPULATE) flags |= MAP_POPULATE;
#endif
#ifdef MAP_HUGETLB
        if (length >= EINA_HUGE_PAGE_MIN) flags |= MAP_HUGETLB;
#endif

        map = malloc(sizeof (Eina_File_Map));
        if (!map) goto on_error;

        map->map = mmap(NULL, length, PROT_READ, flags, file->fd, offset);
#ifdef MAP_HUGETLB
        if (map->map == MAP_FAILED && (flags & MAP_HUGETLB))
          {
             flags &= ~MAP_HUGETLB;
             map->map = mmap(NULL, length, PROT_READ, flags, file->fd, offset);
          }

        map->hugetlb = !!(flags & MAP_HUGETLB);
#else
        map->hugetlb = EINA_FALSE;
#endif
        map->offset = offset;
        map->length = length;
        map->refcount = 0;

        if (map->map == MAP_FAILED) goto on_error;

        eina_hash_add(file->map, &key, map);
        eina_hash_direct_add(file->rmap, &map->map, map);
     }

   map->refcount++;

   _eina_file_map_rule_apply(rule, map->map, 0, length, map->length, map->hugetlb);

   eina_lock_release(&file->lock);

   return map->map;

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

        munmap(file->global_map, file->length);
        file->global_map = MAP_FAILED;
     }
   else
     {
        eina_file_common_map_free(file, map, _eina_file_map_close);
     }

 on_exit:
   eina_lock_release(&file->lock);
}

EINA_API void
eina_file_map_populate(Eina_File *file, Eina_File_Populate rule, const void *map,
                       unsigned long int offset, unsigned long int length)
{
   Eina_File_Map *em;

   EINA_SAFETY_ON_NULL_RETURN(file);
   eina_lock_take(&file->lock);
   if (map == file->global_map)
     _eina_file_map_rule_apply(rule, map, offset, length, file->length, file->global_hugetlb);
   else if ((em = eina_hash_find(file->rmap, &map)) != NULL)
     _eina_file_map_rule_apply(rule, map, offset, length, em->length, em->hugetlb);
   eina_lock_release(&file->lock);
}

EINA_API Eina_Bool
eina_file_map_faulted(Eina_File *file, void *map)
{
   Eina_Bool r = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);

   if (file->virtual) return EINA_FALSE;

   eina_lock_take(&file->lock);

   if (file->global_map == map)
     {
        r = file->global_faulty;
     }
   else
     {
        Eina_File_Map *em;

        em = eina_hash_find(file->rmap, &map);
        if (em)
          {
             r = em->faulty;
          }
        else
          {
             Eina_List *l;

             EINA_LIST_FOREACH(file->dead_map, l, em)
               if (em->map == map)
                 {
                    r = em->faulty;
                    break;
                 }
          }
     }

   eina_lock_release(&file->lock);

   return r;
}

EINA_API Eina_Iterator *
eina_file_xattr_get(Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);

   if (file->virtual) return NULL;

   return eina_xattr_fd_ls(file->fd);
}

EINA_API Eina_Iterator *
eina_file_xattr_value_get(Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);

   if (file->virtual) return NULL;

   return eina_xattr_value_fd_ls(file->fd);
}

EINA_API int
eina_file_statat(void *container, Eina_File_Direct_Info *info, Eina_Stat *st)
{
   struct stat buf;
#ifdef HAVE_ATFILE_SOURCE
   int fd;
#endif

   EINA_SAFETY_ON_NULL_RETURN_VAL(info, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(st, -1);

#ifdef HAVE_ATFILE_SOURCE
   fd = dirfd((DIR*) container);
   if (fstatat(fd, info->path + info->name_start, &buf, 0))
#else
   (void)container;
   if (stat(info->path, &buf))
#endif
     {
        if (info->type != EINA_FILE_LNK)
          info->type = EINA_FILE_UNKNOWN;
        return -1;
     }

   if (info->type == EINA_FILE_UNKNOWN)
     {
        if (S_ISREG(buf.st_mode))
          info->type = EINA_FILE_REG;
        else if (S_ISDIR(buf.st_mode))
          info->type = EINA_FILE_DIR;
        else if (S_ISCHR(buf.st_mode))
          info->type = EINA_FILE_CHR;
        else if (S_ISBLK(buf.st_mode))
          info->type = EINA_FILE_BLK;
        else if (S_ISFIFO(buf.st_mode))
          info->type = EINA_FILE_FIFO;
        else if (S_ISLNK(buf.st_mode))
          info->type = EINA_FILE_LNK;
        else if (S_ISSOCK(buf.st_mode))
          info->type = EINA_FILE_SOCK;
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
   st->blksize = buf.st_blksize;
   st->blocks = buf.st_blocks;
   st->atime = buf.st_atime;
   st->mtime = buf.st_mtime;
   st->ctime = buf.st_ctime;
#ifdef _STAT_VER_LINUX
   st->atimensec = buf.st_atim.tv_nsec;
   st->mtimensec = buf.st_mtim.tv_nsec;
   st->ctimensec = buf.st_ctim.tv_nsec;
#else
   st->atimensec = 0;
   st->mtimensec = 0;
   st->ctimensec = 0;
#endif
   return 0;
}

///////////////////////////////////////////////////////////////////////////
// this below is funky avoiding opendir to avoid heap allocations thus
// getdents and all the os specific stuff as this is intendedf for use
// between fork and exec normally ... this is important
#if defined(__FreeBSD__)
# define do_getdents(fd, buf, size) getdents(fd, buf, size)
typedef struct
{
#if __FreeBSD__ > 11
   ino_t          d_ino;
   off_t          d_off;
   unsigned short d_reclen;
   unsigned char  d_type;
   unsigned char  ____pad0;
   unsigned short d_namlen;
   unsigned short ____pad1;
   char           d_name[4096];
#else
   __uint32_t     d_fileno;
   __uint16_t     d_reclen;
   __uint8_t      d_type;
   __uint8_t      d_namlen;
   char           d_name[4096];
#endif
} Dirent;
#elif defined(__OpenBSD__)
# define do_getdents(fd, buf, size) getdents(fd, buf, size)
typedef struct
{
   __ino_t        d_ino;
   __off_t        d_off;
   unsigned short d_reclen;
   unsigned char  d_type;
   unsigned char  d_namlen;
   unsigned char  ____pad[4];
   char           d_name[4096];
} Dirent;
#elif defined(__linux__)
# define do_getdents(fd, buf, size) syscall(SYS_getdents64, fd, buf, size)
// getdents64 added un glibc 2.30 ... so use raw syscall - will work
// from some linux 2.4 on... so ... i think that's ok. :)
//# define do_getdents(fd, buf, size) getdents64(fd, buf, size)
typedef struct
{
   ino64_t        d_ino;
   off64_t        d_off;
   unsigned short d_reclen;
   unsigned char  d_type;
   char           d_name[4096];
} Dirent;
#endif

EINA_API void
eina_file_close_from(int fd, int *except_fd)
{
#if defined(_WIN32)
   // XXX: what do to here? anything?
#else
#ifdef HAVE_DIRENT_H
//# if 0
# if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__linux__)
   int dirfd;
   Dirent *d;
   char buf[4096 + 128];
   int *closes = NULL;
   int num_closes = 0, i, j, clo, num;
   const char *fname;
   ssize_t pos, ret;
   Eina_Bool do_read;

   // note - this api is EXPECTED to be called in between a fork() and exec()
   // when no threads are running. if you use this outside that context then
   // it may not work as intended and may miss some fd's etc.
   dirfd = open("/proc/self/fd", O_RDONLY | O_DIRECTORY);
   if (dirfd < 0) dirfd = open("/dev/fd", O_RDONLY | O_DIRECTORY);
   if (dirfd >= 0)
     {
        // count # of closes - the dir list should/will not change as its
        // the fd's we have open so we can read it twice with no changes
        // to it
        do_read = EINA_TRUE;
        for (;;)
          {
skip:
             if (do_read)
               {
                  pos = 0;
                  ret = do_getdents(dirfd, buf, 4096);
                  if (ret <= 0) break;
                  do_read = EINA_FALSE;
               }
             d = (Dirent *)(buf + pos);
             fname = d->d_name;
             pos += d->d_reclen;
             if (pos >= ret) do_read = EINA_TRUE;
             if (!((fname[0] >= '0') && (fname[0] <= '9'))) continue;
             num = atoi(fname);
             if (num < fd) continue;
             if (except_fd)
               {
                  for (j = 0; except_fd[j] >= 0; j++)
                    {
                       if (except_fd[j] == num) goto skip;
                    }
               }
             num_closes++;
          }
        // alloc closes list and walk again to fill it - on stack to avoid
        // heap allocs
        closes = alloca(num_closes * sizeof(int));
        if ((closes) && (num_closes > 0))
          {
             clo = 0;
             lseek(dirfd, 0, SEEK_SET);
             do_read = EINA_TRUE;
             for (;;)
               {
skip2:
                  if (do_read)
                    {
                       pos = 0;
                       ret = do_getdents(dirfd, buf, 4096);
                       if (ret <= 0) break;
                       do_read = EINA_FALSE;
                    }
                  d = (Dirent *)(buf + pos);
                  fname = d->d_name;
                  pos += d->d_reclen;
                  if (pos >= ret) do_read = EINA_TRUE;
                  if (!((fname[0] >= '0') && (fname[0] <= '9'))) continue;
                  num = atoi(fname);
                  if (num < fd) continue;
                  if (except_fd)
                    {
                       for (j = 0; except_fd[j] >= 0; j++)
                         {
                            if (except_fd[j] == num) goto skip2;
                         }
                    }
                  if (clo < num_closes) closes[clo] = num;
                  clo++;
               }
             // in case we somehow don't fill up all of closes in 2nd pass
             // (this shouldn't happen as no threads are running and we
             // do nothing to modify the fd set between 2st and 2nd pass).
             // set rest num_closes to clo so we don't close invalid values
             num_closes = clo;
          }
        close(dirfd);
        // now go close all those fd's - some may be invalid like the dir
        // reading fd above... that's ok.
        for (i = 0; i < num_closes; i++)
          {
             close(closes[i]);
          }
        return;
     }
# else
   DIR *dir;
   int *closes = NULL;
   int num_closes = 0, i, j, clo, num;
   struct dirent *dp;
   const char *fname;

   dir = opendir("/proc/self/fd");
   if (!dir) dir = opendir("/dev/fd");
   if (dir)
     {
        // count # of closes - the dir list should/will not change as its
        // the fd's we have open so we can read it twice with no changes
        // to it
        for (;;)
          {
skip:
             if (!(dp = readdir(dir))) break;
             fname = dp->d_name;
             if (!((fname[0] >= '0') && (fname[0] <= '9'))) continue;
             num = atoi(fname);
             if (num < fd) continue;
             if (except_fd)
               {
                  for (j = 0; except_fd[j] >= 0; j++)
                    {
                       if (except_fd[j] == num) goto skip;
                    }
               }
             num_closes++;
          }
        // alloc closes list and walk again to fill it - on stack to avoid
        // heap allocs
        closes = alloca(num_closes * sizeof(int));
        if ((closes) && (num_closes > 0))
          {
             clo = 0;
             seekdir(dir, 0);
             for (;;)
               {
skip2:
                  if (!(dp = readdir(dir))) break;
                  fname = dp->d_name;
                  if (!((fname[0] >= '0') && (fname[0] <= '9'))) continue;
                  num = atoi(fname);
                  if (num < fd) continue;
                  if (except_fd)
                    {
                       for (j = 0; except_fd[j] >= 0; j++)
                         {
                            if (except_fd[j] == num) goto skip2;
                         }
                    }
                  if (clo < num_closes) closes[clo] = num;
                  clo++;
               }
          }
        closedir(dir);
        // now go close all those fd's - some may be invalide like the dir
        // reading fd above... that's ok.
        for (i = 0; i < num_closes; i++)
          {
             close(closes[i]);
          }
        return;
     }
# endif
#endif
   int max = 1024;

#ifdef HAVE_SYS_RESOURCE_H
   struct rlimit lim;
   if (getrlimit(RLIMIT_NOFILE, &lim) < 0) return;
   max = lim.rlim_max;
#endif
   for (i = fd; i < max;)
     {
        if (except_fd)
          {
             int j;

             for (j = 0; except_fd[j] >= 0; j++)
               {
                  if (except_fd[j] == i) goto skip3;
               }
          }
        close(i);
skip3:
        i++;
     }
#endif
}

EINA_API int
eina_file_mkstemp(const char *templatename, Eina_Tmpstr **path)
{
   char buffer[PATH_MAX];
   const char *XXXXXX = NULL, *sep;
   int fd, len;
   mode_t old_umask;

   EINA_SAFETY_ON_NULL_RETURN_VAL(templatename, -1);

   sep = strchr(templatename, '/');
   if (sep)
     {
        len = eina_strlcpy(buffer, templatename, sizeof(buffer));
     }
   else
     {
        len = eina_file_path_join(buffer, sizeof(buffer),
                                  eina_environment_tmp_get(), templatename);
     }

   /*
    * Unix:
    * Make sure temp file is created with secure permissions,
    * http://man7.org/linux/man-pages/man3/mkstemp.3.html#NOTES
    */
   old_umask = umask(S_IRWXG|S_IRWXO);
   if ((XXXXXX = strstr(buffer, "XXXXXX.")) != NULL)
     {
        int suffixlen = buffer + len - XXXXXX - 6;
        fd = mkstemps(buffer, suffixlen);
     }
   else
     fd = mkstemp(buffer);
   umask(old_umask);

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
   if (sep)
     {
        eina_strlcpy(buffer, templatename, sizeof(buffer));
     }
   else
     {
        eina_file_path_join(buffer, sizeof(buffer),
                            eina_environment_tmp_get(), templatename);
     }

   tmpdirname = mkdtemp(buffer);
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
   if (!path || !*path)
     return EINA_FALSE;

   return access(path, mode) == 0;
}

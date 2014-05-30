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

#ifdef HAVE_ESCAPE
# include <Escape.h>
#endif

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#define EINA_SMALL_PAGE eina_cpu_page_size()
#define EINA_HUGE_PAGE 16 * 1024 * 1024

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
_eina_name_max(DIR *dirp)
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

static size_t
_eina_dirent_buffer_size(DIR *dirp)
{
   long name_max = _eina_name_max(dirp);
   size_t name_end;

   name_end = (size_t) offsetof(struct dirent, d_name) + name_max + 1;

   return (name_end > sizeof (struct dirent) ? name_end : sizeof (struct dirent));
}

static Eina_Bool
_eina_file_ls_iterator_next(Eina_File_Iterator *it, void **data)
{
   struct dirent *dp;
   char *name;
   size_t length;

   dp = alloca(_eina_dirent_buffer_size(it->dirp));

   do
     {
        if (readdir_r(it->dirp, dp, &dp))
          return EINA_FALSE;
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

   dp = alloca(_eina_dirent_buffer_size(it->dirp));

   do
     {
        if (readdir_r(it->dirp, dp, &dp))
           return EINA_FALSE;
        if (!dp)
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

   if (file->global_map != MAP_FAILED)
     munmap(file->global_map, file->length);

   if (file->fd != -1) close(file->fd);
}

static void
_eina_file_map_close(Eina_File_Map *map)
{
   munmap(map->map, map->length);
   free(map);
}

#ifndef MAP_POPULATE
static unsigned int
_eina_file_map_populate(char *map, unsigned int size, Eina_Bool hugetlb)
{
   unsigned int r = 0xDEADBEEF;
   unsigned int i;
   unsigned int s;

   s = hugetlb ? EINA_HUGE_PAGE : EINA_SMALL_PAGE;

   for (i = 0; i < size; i += s)
     r ^= map[i];

   r ^= map[size];

   return r;
}
#endif

static int
_eina_file_map_rule_apply(Eina_File_Populate rule, void *addr, unsigned long int size, Eina_Bool hugetlb)
{
   int tmp = 42;
   int flag = MADV_RANDOM;

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
# if (defined __USE_MISC && defined st_mtime)
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

   _eina_file_cache = eina_hash_string_djb2_new(NULL);
   if (!_eina_file_cache)
     {
        ERR("Could not create cache.");
        eina_log_domain_unregister(_eina_file_log_dom);
        _eina_file_log_dom = -1;
        return EINA_FALSE;
     }

   eina_lock_new(&_eina_file_lock_cache);

   return EINA_TRUE;
}

Eina_Bool
eina_file_shutdown(void)
{
   if (eina_hash_population(_eina_file_cache) > 0)
     {
        Eina_Iterator *it;
        const char *key;

        it = eina_hash_iterator_key_new(_eina_file_cache);
        EINA_ITERATOR_FOREACH(it, key)
	  {
             Eina_File *f = eina_hash_find(_eina_file_cache, key);
             ERR("File [%s] still open %i times !", key, f->refcount);
          }
        eina_iterator_free(it);
     }

   eina_hash_free(_eina_file_cache);

   eina_lock_free(&_eina_file_lock_cache);

   eina_log_domain_unregister(_eina_file_log_dom);
   _eina_file_log_dom = -1;
   return EINA_TRUE;
}

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

void
eina_file_mmap_faulty(void *addr, long page_size)
{
   Eina_File_Map *m;
   Eina_File *f;
   Eina_Iterator *itf;
   Eina_Iterator *itm;

   /* NOTE: I actually don't know if other thread are running, I will try to take the lock.
      It may be possible that if other thread are not running and they were in the middle of
      accessing an Eina_File this lock are still taken and we will result as a deadlock. */
   eina_lock_take(&_eina_file_lock_cache);

   itf = eina_hash_iterator_data_new(_eina_file_cache);
   EINA_ITERATOR_FOREACH(itf, f)
     {
        Eina_Bool faulty = EINA_FALSE;

        eina_lock_take(&f->lock);

        if (f->global_map != MAP_FAILED)
          {
             if ((unsigned char *) addr < (((unsigned char *)f->global_map) + f->length) &&
                 (((unsigned char *) addr) + page_size) >= (unsigned char *) f->global_map)
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
}

/* ================================================================ *
 *   Simplified logic for portability layer with eina_file_common   *
 * ================================================================ */  

Eina_Bool
eina_file_path_relative(const char *path)
{
   if (*path != '/') return EINA_TRUE;
   return EINA_FALSE;
}

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



EAPI Eina_Bool
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

   for (current = strchr(path, PATH_DELIM);
        current;
        path = current + 1, current = strchr(path, PATH_DELIM))
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

EAPI Eina_Iterator *
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

EAPI Eina_Iterator *
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

EAPI Eina_File *
eina_file_open(const char *path, Eina_Bool shared)
{
   Eina_File *file;
   Eina_File *n;
   char *filename;
   struct stat file_stat;
   int fd = -1;
#ifdef HAVE_FCNTL
   int flags;
#endif

   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);

   filename = eina_file_path_sanitize(path);
   if (!filename) return NULL;

   if (shared)
#ifdef HAVE_SHM_OPEN
     fd = shm_open(filename, O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
#else
     goto on_error;
#endif
   else
     fd = open(filename, O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);

   if (fd < 0) goto on_error;

#ifdef HAVE_FCNTL
   flags = fcntl(fd, F_GETFD);
   if (flags == -1)
     goto on_error;

   flags |= FD_CLOEXEC;
   if (fcntl(fd, F_SETFD, flags) == -1)
     goto on_error;
#endif

   if (fstat(fd, &file_stat))
     goto on_error;

   eina_lock_take(&_eina_file_lock_cache);

   file = eina_hash_find(_eina_file_cache, filename);
   if ((file) && !_eina_file_timestamp_compare(file, &file_stat))
     {
        file->delete_me = EINA_TRUE;
        eina_hash_del(_eina_file_cache, file->filename, file);
        file = NULL;
     }

   if (!file)
     {
        n = malloc(sizeof(Eina_File) + strlen(filename) + 1);
        if (!n)
	  {
             eina_lock_release(&_eina_file_lock_cache);
             goto on_error;
	  }

        memset(n, 0, sizeof(Eina_File));
        n->filename = (char*) (n + 1);
        strcpy((char*) n->filename, filename);
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
# if (defined __USE_MISC && defined st_mtime)
        n->mtime_nsec = (unsigned long int)file_stat.st_mtim.tv_nsec;
# else
        n->mtime_nsec = (unsigned long int)file_stat.st_mtimensec;
# endif
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
        close(fd);
        n = file;
     }
   eina_lock_take(&n->lock);
   n->refcount++;
   eina_lock_release(&n->lock);

   eina_lock_release(&_eina_file_lock_cache);

   free(filename);

   return n;

 on_error:
   WRN("Could not open file [%s].", filename);

   free(filename);
   if (fd >= 0) close(fd);
   return NULL;
}

EAPI Eina_Bool
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
# if (defined __USE_MISC && defined st_mtime)
   file->mtime_nsec = (unsigned long int)file_stat.st_mtim.tv_nsec;
# else
   file->mtime_nsec = (unsigned long int)file_stat.st_mtimensec;
# endif
#endif
   file->inode = file_stat.st_ino;

   return r;
}

EAPI void *
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
   if (file->length > EINA_HUGE_PAGE) flags |= MAP_HUGETLB;
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
        _eina_file_map_rule_apply(rule, file->global_map, file->length, hugetlb);
        file->global_refcount++;
        ret = file->global_map;
     }

   eina_lock_release(&file->lock);
   return ret;
}

EAPI void *
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
        if (length > EINA_HUGE_PAGE) flags |= MAP_HUGETLB;
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

   _eina_file_map_rule_apply(rule, map->map, length, map->hugetlb);

   eina_lock_release(&file->lock);

   return map->map;

 on_error:
   free(map);
   eina_lock_release(&file->lock);

   return NULL;
}

EAPI void
eina_file_map_free(Eina_File *file, void *map)
{
   EINA_SAFETY_ON_NULL_RETURN(file);

   if (file->virtual)
     return eina_file_virtual_map_free(file, map);

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

EAPI void
eina_file_map_populate(Eina_File *file, Eina_File_Populate rule, const void *map,
                       unsigned long int offset, unsigned long int length)
{
   Eina_File_Map *em;
   
   EINA_SAFETY_ON_NULL_RETURN(file);
   eina_lock_take(&file->lock);
   em = eina_hash_find(file->rmap, &map);
   if (em) _eina_file_map_rule_apply(rule, ((char *)em->map) + offset,
                                     length, em->hugetlb);
   eina_lock_release(&file->lock);
}

EAPI Eina_Bool
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

EAPI Eina_Iterator *
eina_file_xattr_get(Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);

   if (file->virtual) return NULL;

   return eina_xattr_fd_ls(file->fd);
}

EAPI Eina_Iterator *
eina_file_xattr_value_get(Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);

   if (file->virtual) return NULL;

   return eina_xattr_value_fd_ls(file->fd);
}

EAPI int
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
# if (defined __USE_MISC && defined st_mtime)
   st->atimensec = buf.st_atim.tv_nsec;
   st->mtimensec = buf.st_mtim.tv_nsec;
   st->ctimensec = buf.st_ctim.tv_nsec;
# else
   st->atimensec = buf.st_atimensec;
   st->mtimensec = buf.st_mtimensec;
   st->ctimensec = buf.st_ctimensec;
# endif
#else
   st->atimensec = 0;
   st->mtimensec = 0;
   st->ctimensec = 0;
#endif
   return 0;
}


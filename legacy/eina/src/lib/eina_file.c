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

#include <string.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define PATH_DELIM '/'

#include "eina_config.h"
#include "eina_private.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_file.h"
#include "eina_stringshare.h"
#include "eina_hash.h"
#include "eina_list.h"
#include "eina_lock.h"
#include "eina_mmap.h"

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

#define EINA_SMALL_PAGE 4096
# define EINA_HUGE_PAGE 16 * 1024 * 1024

typedef struct _Eina_File_Iterator Eina_File_Iterator;
typedef struct _Eina_File_Map Eina_File_Map;

struct _Eina_File_Iterator
{
   Eina_Iterator iterator;

   DIR *dirp;
   int length;

   char dir[1];
};

struct _Eina_File
{
   const char *filename;

   Eina_Hash *map;
   Eina_Hash *rmap;
   void *global_map;

   Eina_Lock lock;

   unsigned long long length;
   time_t mtime;
   ino_t inode;

   int refcount;
   int global_refcount;

   int fd;

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
static Eina_Lock _eina_file_lock_cache;

static int _eina_file_log_dom = -1;

/*
 * This complex piece of code is needed due to possible race condition.
 * The code and description of the issue can be found at :
 * http://womble.decadent.org.uk/readdir_r-advisory.html
 */
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
   struct stat st;

   if (!_eina_file_direct_ls_iterator_next(it, data))
     return EINA_FALSE;

   if (it->info.type == EINA_FILE_UNKNOWN)
     {
#ifdef HAVE_FSTATAT
        int fd;

        fd = dirfd(it->dirp);
        if (fstatat(fd, it->info.path + it->info.name_start, &st, 0))
#else
        if (stat(it->info.path, &st))
#endif
          it->info.type = EINA_FILE_UNKNOWN;
        else
          {
             if (S_ISREG(st.st_mode))
               it->info.type = EINA_FILE_REG;
             else if (S_ISDIR(st.st_mode))
               it->info.type = EINA_FILE_DIR;
             else if (S_ISCHR(st.st_mode))
               it->info.type = EINA_FILE_CHR;
             else if (S_ISBLK(st.st_mode))
               it->info.type = EINA_FILE_BLK;
             else if (S_ISFIFO(st.st_mode))
               it->info.type = EINA_FILE_FIFO;
             else if (S_ISLNK(st.st_mode))
               it->info.type = EINA_FILE_LNK;
             else if (S_ISSOCK(st.st_mode))
               it->info.type = EINA_FILE_SOCK;
             else
               it->info.type = EINA_FILE_UNKNOWN;
          }
     }

   return EINA_TRUE;
}

static void
_eina_file_real_close(Eina_File *file)
{
   if (file->refcount != 0) return;

   eina_hash_free(file->rmap);
   eina_hash_free(file->map);

   if (file->global_map != MAP_FAILED)
     munmap(file->global_map, file->length);

   close(file->fd);

   free(file);
}

static void
_eina_file_map_close(Eina_File_Map *map)
{
   munmap(map->map, map->length);
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

#ifndef MAP_POPULATE
static int
_eina_file_map_populate(char *map, unsigned int size)
{
   int r = 0xDEADBEEF;
   int i;
   int s;

#ifdef MAP_HUGETLB
   s = size > EINA_HUGE_PAGE ? EINA_HUGE_PAGE : EINA_SMALL_PAGE;
#else
   s = EINA_SMALL_PAGE;
#endif

   for (i = 0; i < size; i += s)
     r ^= map[i];

   r ^= map[size];

   return r;
}
#endif

static int
_eina_file_map_rule_apply(Eina_File_Populate rule, void *addr, unsigned long int size)
{
   int tmp = 42;
   int flag = MADV_RANDOM;

   switch (rule)
     {
      case EINA_FILE_RANDOM: flag = MADV_RANDOM; break;
      case EINA_FILE_SEQUENTIAL: flag = MADV_SEQUENTIAL; break;
      case EINA_FILE_POPULATE: flag = MADV_WILLNEED; break;
      case EINA_FILE_WILLNEED: flag = MADV_WILLNEED; break;
     }

   madvise(addr, size, flag);

#ifndef MAP_POPULATE
   if (rule == EINA_FILE_POPULATE)
     tmp ^= _eina_file_map_populate(addr, size);
#endif

   return tmp;
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
          ERR("File [%s] still open !", key);
        eina_iterator_free(it);
     }

   eina_hash_free(_eina_file_cache);

   eina_lock_free(&_eina_file_lock_cache);

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
   Eina_File_Iterator *it;
   size_t length;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, NULL);

   length = strlen(dir);
   if (length < 1)
      return NULL;

   it = calloc(1, sizeof (Eina_File_Iterator) + length);
   if (!it)
      return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->dirp = opendir(dir);
   if (!it->dirp)
     {
        free(it);
        return NULL;
     }

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
}

EAPI Eina_Iterator *
eina_file_direct_ls(const char *dir)
{
   Eina_File_Direct_Iterator *it;
   size_t length;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, NULL);

   length = strlen(dir);
   if (length < 1)
      return NULL;

   it = calloc(1, sizeof(Eina_File_Direct_Iterator) + length);
   if (!it)
      return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->dirp = opendir(dir);
   if (!it->dirp)
     {
        free(it);
        return NULL;
     }

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
}

EAPI Eina_Iterator *
eina_file_stat_ls(const char *dir)
{
   Eina_File_Direct_Iterator *it;
   size_t length;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, NULL);

   length = strlen(dir);
   if (length < 1)
      return NULL;

   it = calloc(1, sizeof(Eina_File_Direct_Iterator) + length);
   if (!it)
      return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->dirp = opendir(dir);
   if (!it->dirp)
     {
        free(it);
        return NULL;
     }

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
}

EAPI Eina_File *
eina_file_open(const char *filename, Eina_Bool shared)
{
   Eina_File *file;
   Eina_File *n;
   struct stat file_stat;
   int fd;
   int flags;

   EINA_SAFETY_ON_NULL_RETURN_VAL(filename, NULL);

   /*
     FIXME: always open absolute path
     (need to fix filename according to current directory)
   */

   if (shared)
#ifdef HAVE_SHMOPEN
     fd = shm_open(filename, O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
#else
     return NULL;
#endif
   else
     fd = open(filename, O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);

   if (fd < 0) return NULL;

#ifdef HAVE_EXECVP
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
   if ((file) &&
       ((file->mtime != file_stat.st_mtime) ||
        (file->length != (unsigned long long) file_stat.st_size) ||
        (file->inode != file_stat.st_ino)))
      // FIXME: handle sub-second resolution correctness
     {
        file->delete_me = EINA_TRUE;
        eina_hash_del(_eina_file_cache, file->filename, file);
        file = NULL;
     }

   if (!file)
     {
        n = malloc(sizeof (Eina_File) + strlen(filename) + 1);
        if (!n)
	  {
             eina_lock_release(&_eina_file_lock_cache);
             goto on_error;
	  }

        n->filename = (char*) (n + 1);
        strcpy((char*) n->filename, filename);
        n->map = eina_hash_new(EINA_KEY_LENGTH(_eina_file_map_key_length),
                               EINA_KEY_CMP(_eina_file_map_key_cmp),
                               EINA_KEY_HASH(_eina_file_map_key_hash),
                               EINA_FREE_CB(_eina_file_map_close),
                               3);
        n->rmap = eina_hash_pointer_new(NULL);
        n->global_map = MAP_FAILED;
	n->global_refcount = 0;
        n->length = file_stat.st_size;
        n->mtime = file_stat.st_mtime;
        n->inode = file_stat.st_ino;
        n->refcount = 0;
        n->fd = fd;
        n->shared = shared;
        n->delete_me = EINA_FALSE;
        eina_lock_new(&n->lock);
        eina_hash_direct_add(_eina_file_cache, n->filename, n);
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

   return n;

 on_error:
   close(fd);
   return NULL;
}

EAPI void
eina_file_close(Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN(file);

   eina_lock_take(&file->lock);
   file->refcount--;
   eina_lock_release(&file->lock);

   if (file->refcount != 0) return;
   eina_lock_take(&_eina_file_lock_cache);

   eina_hash_del(_eina_file_cache, file->filename, file);

   eina_lock_release(&_eina_file_lock_cache);
}

EAPI size_t
eina_file_size_get(Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, 0);
   return file->length;
}

EAPI time_t
eina_file_mtime_get(Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, 0);
   return file->mtime;
}

EAPI const char *
eina_file_filename_get(Eina_File *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);
   return file->filename;
}

EAPI void *
eina_file_map_all(Eina_File *file, Eina_File_Populate rule)
{
   int flags = MAP_SHARED;
   void *ret = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);

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

   if (file->global_map != MAP_FAILED)
     {
        _eina_file_map_rule_apply(rule, file->global_map, file->length);
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
        map->offset = offset;
        map->length = length;
        map->refcount = 0;

        if (map->map == MAP_FAILED) goto on_error;

        eina_hash_add(file->map, &key, map);
        eina_hash_direct_add(file->rmap, map->map, map);
     }

   map->refcount++;

   _eina_file_map_rule_apply(rule, map->map, length);

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
        Eina_File_Map *em;
        unsigned long int key[2];

        em = eina_hash_find(file->rmap, &map);
        if (!em) return ;

        em->refcount--;

        if (em->refcount > 0) goto on_exit;

        key[0] = em->offset;
        key[1] = em->length;

        eina_hash_del(file->rmap, &map, em);
        eina_hash_del(file->map, &key, em);
     }

 on_exit:
   eina_lock_release(&file->lock);
}



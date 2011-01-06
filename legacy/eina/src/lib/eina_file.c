/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga, Vincent Torri
 * Copyright (C) 2010 Cedric Bail
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
#include <fcntl.h>
#include <sys/mman.h>

#define PATH_DELIM '/'

#ifdef __sun
# ifndef NAME_MAX
#  define NAME_MAX 255
# endif
#endif

#include "eina_config.h"
#include "eina_private.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_file.h"
#include "eina_stringshare.h"
#include "eina_hash.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

typedef struct _Eina_File_Iterator Eina_File_Iterator;
struct _Eina_File_Iterator
{
   Eina_Iterator iterator;

   DIR *dirp;
   int length;

   char dir[1];
};

/*
 * This complex piece of code is needed due to possible race condition.
 * The code and description of the issue can be found at :
 * http://womble.decadent.org.uk/readdir_r-advisory.html
 */
static size_t
_eina_dirent_buffer_size(DIR *dirp)
{
   long name_max;
   size_t name_end;

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

/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Eina_File_Group File
 *
 * @brief Functions to traverse directories and split paths.
 *
 * @li eina_file_dir_list() list the content of a directory,
 * recusrsively or not, and can call a callback function for eachfound
 * file.
 * @li eina_file_split() split a path into all the subdirectories that
 * compose it, according to the separator of the file system.
 *
 * @{
 */

/**
 * @brief List all files on the directory calling the function for every file found.
 *
 * @param dir The directory name.
 * @param recursive Iterate recursively in the directory.
 * @param cb The callback to be called.
 * @param data The data to pass to the callback.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function lists all the files in @p dir. To list also all the
 * sub directoris recursively, @p recursive must be set to #EINA_TRUE,
 * otherwise it must be set to #EINA_FALSE. For each found file, @p cb
 * is called and @p data is passed to it.
 *
 * If @p cb or @p dir are @c NULL, or if @p dir is a string of size 0,
 * or if @p dir can not be opened, this function returns #EINA_FALSE
 * immediately. otherwise, it returns #EINA_TRUE.
 */
EAPI Eina_Bool
eina_file_dir_list(const char *dir,
                   Eina_Bool recursive,
                   Eina_File_Dir_List_Cb cb,
                   void *data)
{
   int dlength;
   struct dirent *de;
   DIR *d;
#ifndef _DIRENT_HAVE_D_TYPE
   struct stat st;
#endif

   EINA_SAFETY_ON_NULL_RETURN_VAL(cb,  EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dir, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(dir[0] == '\0', EINA_FALSE);

   d = opendir(dir);
   if (!d)
      return EINA_FALSE;

   dlength = strlen(dir);
   de = alloca(_eina_dirent_buffer_size(d));

   while ((!readdir_r(d, de, &de) && de))
     {
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
           continue;

        cb(de->d_name, dir, data);
        /* d_type is only available on linux and bsd (_BSD_SOURCE) */

        if (recursive == EINA_TRUE)
          {
             char *path;
             int length;

#ifdef _DIRENT_HAVE_D_NAMLEN
             length = de->d_namlen;
#else
             length = strlen(de->d_name);
#endif
             path = alloca(dlength + length + 2);
             strcpy(path, dir);
             strcat(path, "/");
             strcat(path, de->d_name);
#ifdef _DIRENT_HAVE_D_TYPE
             if (de->d_type != DT_DIR)
                continue;
#else
             if (stat(path, &st))
                continue;
             if (!S_ISDIR(st.st_mode))
                continue;
#endif
             eina_file_dir_list(path, recursive, cb, data);
          }
     }

   closedir(d);

   return EINA_TRUE;
}

/**
 * @brief Split a path according to the delimiter of the filesystem.
 *
 * @param path The path to split.
 * @return An array of the parts of the path to split.
 *
 * This function splits @p path according to the delimiter of the used
 * filesystem. If  @p path is @c NULL or if the array can not be
 * created, @c NULL is returned, otherwise, an array with the
 * different parts of @p path is returned.
 */
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

/**
 * Get an iterator to list the content of a directory.
 *
 * Iterators are cheap to be created and allow interruption at any
 * iteration. At each iteration, only the next directory entry is read
 * from the filesystem with readdir_r().
 *
 * The iterator will handle the user a stringshared value with the
 * full path. One must call eina_stringshare_del() on it after usage
 * to not leak!
 *
 * The eina_file_direct_ls() function will provide a possibly faster
 * alternative if you need to filter the results somehow, like
 * checking extension.
 *
 * The iterator will walk over '.' and '..' without returning them.
 *
 * The iterator container is the DIR* corresponding to the current walk.
 *
 * @param  dir The name of the directory to list
 * @return Return an Eina_Iterator that will walk over the files and
 *         directory in the pointed directory. On failure it will
 *         return NULL. The iterator emits stringshared value with the
 *         full path and must be freed with eina_stringshare_del().
 *
 * @see eina_file_direct_ls()
 */
EAPI Eina_Iterator *
eina_file_ls(const char *dir)
{
   Eina_File_Iterator *it;
   size_t length;

   if (!dir)
      return NULL;

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

/**
 * Get an iterator to list the content of a directory, with direct information.
 *
 * Iterators are cheap to be created and allow interruption at any
 * iteration. At each iteration, only the next directory entry is read
 * from the filesystem with readdir_r().
 *
 * The iterator returns the direct pointer to couple of useful information in
 * #Eina_File_Direct_Info and that pointer should not be modified anyhow!
 *
 * The iterator will walk over '.' and '..' without returning them.
 *
 * The iterator container is the DIR* corresponding to the current walk.
 *
 * @param  dir The name of the directory to list

 * @return Return an Eina_Iterator that will walk over the files and
 *         directory in the pointed directory. On failure it will
 *         return NULL. The iterator emits #Eina_File_Direct_Info
 *         pointers that could be used but not modified. The lifetime
 *         of the returned pointer is until the next iteration and
 *         while the iterator is live, deleting the iterator
 *         invalidates the pointer. It will not call stat() when filesystem
 *         doesn't provide information to fill type from readdir_r().
 *
 * @see eina_file_ls()
 */
EAPI Eina_Iterator *
eina_file_direct_ls(const char *dir)
{
   Eina_File_Direct_Iterator *it;
   size_t length;

   if (!dir)
      return NULL;

   length = strlen(dir);
   if (length < 1)
      return NULL;

   if (length + NAME_MAX + 2 >= EINA_PATH_MAX)
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

/**
 * Get an iterator to list the content of a directory, with direct information.
 *
 * Iterators are cheap to be created and allow interruption at any
 * iteration. At each iteration, only the next directory entry is read
 * from the filesystem with readdir_r().
 *
 * The iterator returns the direct pointer to couple of useful information in
 * #Eina_File_Direct_Info and that pointer should not be modified anyhow!
 *
 * The iterator will walk over '.' and '..' without returning them.
 *
 * The iterator container is the DIR* corresponding to the current walk.
 *
 * @param  dir The name of the directory to list

 * @return Return an Eina_Iterator that will walk over the files and
 *         directory in the pointed directory. On failure it will
 *         return NULL. The iterator emits #Eina_File_Direct_Info
 *         pointers that could be used but not modified. The lifetime
 *         of the returned pointer is until the next iteration and
 *         while the iterator is live, deleting the iterator
 *         invalidates the pointer. It will call stat() when filesystem
 *         doesn't provide information to fill type from readdir_r().
 *
 * @see eina_file_direct_ls()
 */
EAPI Eina_Iterator *
eina_file_stat_ls(const char *dir)
{
   Eina_File_Direct_Iterator *it;
   size_t length;

   if (!dir)
      return NULL;

   length = strlen(dir);
   if (length < 1)
      return NULL;

   if (length + NAME_MAX + 2 >= EINA_PATH_MAX)
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


/*
  H : half
  X : done

  [ ] nom du fichier pour shm_open
  [ ] stringshare pour le fichier
  [H] flags dans des enum
  [X] fstat pour la taille
  [H] enregistrer dans un cache (hash) les fichiers ouverts
  [ ] garder la liste des map ouvertes et les detruire
  [X] mmap tout le fichier
  [ ] hash pour les blocs mmap
  [ ] taille de mmap : sauvegarder dans Eina_File
 */

#include "eina_log.h"

static Eina_Hash *_eina_file_files = NULL;

/* _eina_file_files = eina_hash_string_small_new(NULL) */

typedef enum
{
  EINA_FILE_FLAG_READ = 0,
  EINA_FILE_FLAG_WRITE = 1,
  EINA_FILE_FLAG_READ_WRITE = 2,
  EINA_FILE_FLAG_CREATE = 1 << 2,
  EINA_FILE_FLAG_TRUNCATE = 1 << 3
} Eina_File_Flag;

typedef enum
{
  EINA_FILE_MODE_READ = 1 << 0,
  EINA_FILE_MODE_WRITE = 1 << 1
} Eina_File_Mode;

typedef enum
{
  EINA_FILE_MAP_PROT_EXEC,
  EINA_FILE_MAP_PROT_READ,
  EINA_FILE_MAP_PROT_WRITE,
  EINA_FILE_MAP_PROT_NONE
} Eina_File_Map_Prot;

typedef enum
{
  /* No MAP_FIXED, its use is discouraged */
  EINA_FILE_MAP_FLAG_SHARED,
  EINA_FILE_MAP_FLAG_PRIVATE
} Eina_File_Map_Flag;

typedef struct
{
  char          *filename;
  Eina_File_Flag flag;
  Eina_File_Mode mode;
  int            fd;
  off_t          file_size;
  off_t          map_size;
  void          *map_full;
  Eina_Hash     *map_blocks;
  Eina_Bool      shared : 1;
} Eina_File;

static int _eina_file_log_dom = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_file_log_dom, __VA_ARGS__)

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

   return EINA_TRUE;
}

Eina_Bool
eina_file_shutdown(void)
{
   eina_log_domain_unregister(_eina_file_log_dom);
   _eina_file_log_dom = -1;
   return EINA_TRUE;
}

EAPI Eina_File *
eina_file_open(const char *filename, Eina_File_Flag flag, Eina_File_Mode mode, Eina_Bool shared)
{
  struct stat st;
  Eina_File *file;
  Eina_File *cache;
  int oflag;
  mode_t omode = 0;

  if (!filename || !*filename) return NULL;

  file = (Eina_File *)calloc(1, sizeof(Eina_File));
  if (!file) return NULL;

  if (shared)
    {
      size_t length;

      length = strlen(filename) + 1;
      file->filename = (char *)malloc(length + 1);
      if (!file->filename) goto free_file;
      *file->filename = '/';
      memcpy(file->filename + 1, filename, length);
    }
  else
    {
      file->filename = strdup(filename);
      if (!file->filename) goto free_file;
    }

  file->flag = flag;
  file->mode = mode;
  file->shared = shared;
  switch (flag)
    {
    case EINA_FILE_FLAG_READ:
      oflag = O_RDONLY;
      break;
    case EINA_FILE_FLAG_WRITE:
      oflag = O_WRONLY;
      break;
    case EINA_FILE_FLAG_READ_WRITE:
      oflag = O_RDWR;
      break;
    default:
      oflag = O_RDONLY;
      break;
    }

  if (flag & EINA_FILE_FLAG_CREATE) oflag |= O_CREAT;
  if (flag & EINA_FILE_FLAG_TRUNCATE) oflag |= O_TRUNC;

  if (mode & EINA_FILE_MODE_READ) omode |= S_IRUSR;
  if (mode & EINA_FILE_MODE_WRITE) omode |= S_IWUSR;

  if (shared)
    {
      file->fd = shm_open(file->filename, oflag, omode);
      if (file->fd == -1)
        goto free_filename;
    }
  else
    {
      file->fd = open(file->filename, oflag, omode);
      if (file->fd == -1)
        goto free_filename;
    }

  if (fstat(file->fd, &st) != 0)
    {
      ERR("Could not retrieve file size");
      goto close_file;
    }

  file->file_size = st.st_size;

  cache = eina_hash_find(_eina_file_files, file->filename);
  if (cache)
    {
      /* faire plus de tests ? */
      if (cache->file_size != file->file_size)
        {
          eina_hash_modify(_eina_file_files, file->filename, file);
          free(cache->filename);
          free(cache);
        }
      else
        return cache;
    }
  else
    eina_hash_add(_eina_file_files, file->filename, file);

  return file;

 close_file:
  if (shared)
    shm_unlink(file->filename);
  else
    close(file->fd);
 free_filename:
  free(file->filename);
 free_file:
  free(file);

  return NULL;
}

EAPI void
eina_file_close(Eina_File *file)
{
  if (!file)
    return;

  if (eina_hash_find(_eina_file_files, file->filename))
    eina_hash_del(_eina_file_files, file->filename, file);

  if (file->shared)
    {
      if (shm_unlink(file->filename) != 0)
        ERR("Could not unlink %s", file->filename);
    }
  else
    {
      if (close(file->fd) != 0)
        ERR("Could not close %s", file->filename);
    }

  free(file->filename);
  free(file);
}

EAPI void *
eina_file_map_full(Eina_File *file, Eina_File_Map_Prot prot, Eina_File_Map_Flag flags)
{
  if (!file || !file->shared)
    return NULL;

  if (!file->map_full)
    {
      file->map_size = file->file_size;
      file->map_full = mmap(NULL, file->map_size, prot, flags, file->fd, 0);
    }

  return file->map_full;
}

EAPI Eina_Bool
eina_file_unmap_full(Eina_File *file)
{
  if (!file || !file->map_full)
    return EINA_FALSE;

  return (munmap(file->map_full, file->map_size) == 0) ? EINA_TRUE : EINA_FALSE;
}

EAPI void *
eina_file_map(Eina_File *file, void *start, size_t length, Eina_File_Map_Prot prot, Eina_File_Map_Flag flags, off_t offset)
{
  if (!file)
    return NULL;

  if (length > (size_t)file->file_size)
    {
      ERR("Wrong length");
      return NULL;
    }

  file->map_size = length;

  return mmap(start, file->map_size, prot, flags, file->fd, offset);
}

EAPI Eina_Bool
eina_file_unmap(Eina_File *file, void *start)
{
  if (!file)
    return EINA_FALSE;

  return (munmap(start, file->map_size) == 0) ? EINA_TRUE : EINA_FALSE;
}

/**
 * @}
 */

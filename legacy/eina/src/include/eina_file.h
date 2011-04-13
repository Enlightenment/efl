/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga
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

#ifndef EINA_FILE_H_
#define EINA_FILE_H_

#include <limits.h>

#include "eina_types.h"
#include "eina_array.h"
#include "eina_iterator.h"


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
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_File_Group File
 *
 * @{
 */

/**
 * @typedef Eina_File_Direct_Info
 * A typedef to #_Eina_File_Direct_Info.
 */
typedef struct _Eina_File_Direct_Info Eina_File_Direct_Info;

/**
 * @typedef Eina_File_Dir_List_Cb
 * Type for a callback to be called when iterating over the files of a
 * directory.
 */
typedef void (*Eina_File_Dir_List_Cb)(const char *name, const char *path, void *data);

/**
 * @typedef Eina_File_Type
 * file type in Eina_File_Direct_Info.
 */
typedef enum {
  EINA_FILE_UNKNOWN, /**< Unknown file type. */
  EINA_FILE_FIFO,    /**< Named pipe (FIFO) type (unused on Windows). */
  EINA_FILE_CHR,     /**< Character device type (unused on Windows). */
  EINA_FILE_DIR,     /**< Directory type. */
  EINA_FILE_BLK,     /**< Block device type (unused on Windows). */
  EINA_FILE_REG,     /**< Regular file type. */
  EINA_FILE_LNK,     /**< Symbolic link type. */
  EINA_FILE_SOCK,    /**< UNIX domain socket type (unused on Windows). */
  EINA_FILE_WHT      /**< Whiteout file type (unused on Windows). */
} Eina_File_Type;

typedef struct _Eina_File Eina_File;

typedef enum {
  EINA_FILE_RANDOM,     /**< Advise random memory access to the mapped memory. */
  EINA_FILE_SEQUENTIAL, /**< Advise sequential memory access to the mapped memory. */
  EINA_FILE_WILLNEED,   /**< Advise need for all the mapped memory. */
  EINA_FILE_POPULATE    /**< Request all the mapped memory. */
} Eina_File_Populate;

/* Why do this? Well PATH_MAX may vary from when eina itself is compiled
 * to when the app using eina is compiled. exposing the path buffer below
 * cant safely and portably vary based on how/when you compile. it should
 * always be the same for both eina inside AND for apps outside that use eina
 * so define this to 8192 - most PATH_MAX values are like 4096 or 1024 (with
 * windows i think being 260), so 8192 should cover almost all cases. there
 * is a possibility that PATH_MAX could be more than 8192. if anyone spots
 * a path_max that is bigger - let us know, but, for now we will assume
 * it never happens */
#define EINA_PATH_MAX 8192
/**
 * @struct _Eina_File_Direct_Info
 * A structure to store informations of a path.
 */
struct _Eina_File_Direct_Info
{
   size_t               path_length; /**< size of the whole path */
   size_t               name_length; /**< size of the filename/basename component */
   size_t               name_start; /**< where the filename/basename component starts */
   Eina_File_Type       type; /**< file type */
   char                 path[EINA_PATH_MAX]; /**< the path */
};

/**
 * @def EINA_FILE_DIR_LIST_CB
 * @brief cast to an #Eina_File_Dir_List_Cb.
 *
 * @param function The function to cast.
 *
 * This macro casts @p function to Eina_File_Dir_List_Cb.
 */
#define EINA_FILE_DIR_LIST_CB(function) ((Eina_File_Dir_List_Cb)function)


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
EAPI Eina_Bool eina_file_dir_list(const char           *dir,
                                  Eina_Bool             recursive,
                                  Eina_File_Dir_List_Cb cb,
                                  void                 *data) EINA_ARG_NONNULL(1, 3);

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
EAPI Eina_Array    *eina_file_split(char *path) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

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
EAPI Eina_Iterator *eina_file_ls(const char *dir) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

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
 * 
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
EAPI Eina_Iterator *eina_file_stat_ls(const char *dir) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

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
 * 
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
EAPI Eina_Iterator *eina_file_direct_ls(const char *dir) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Get a read-only handler to a file.
 *
 * @param name Filename to open
 * @param shared Requested a shm
 *
 * The file are only open in read only mode. Name should be an absolute path to
 * prevent cache mistake. A handler could be shared among multiple instance and
 * will be correctly refcounted. File are automatically closed on exec.
 */
EAPI Eina_File *eina_file_open(const char *name, Eina_Bool shared) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Unref file handler.
 *
 * @param file File handler to unref.
 *
 * This doesn't close the file, it will remain open until it leave the cache.
 */
EAPI void eina_file_close(Eina_File *file);

/**
 * @brief Get file size at open time.
 *
 * @param file The file handler to request the size from.
 * @return The length of the file.
 */
EAPI unsigned long int eina_file_size_get(Eina_File *file);

/**
 * @brief Get the last modification time of an open file.
 *
 * @param file The file handler to request the modification time from.
 * @return The last modification time.
 */
EAPI time_t eina_file_mtime_get(Eina_File *file);

/**
 * @brief Get the filename of an open file.
 *
 * @param file The file handler to request the name from.
 * @return Stringshared filename of the file.
 */
EAPI const char *eina_file_filename_get(Eina_File *file);

/**
 * @brief Map all the file to a buffer.
 *
 * @param file The file handler to map in memory
 * @param rule The rule to apply to the mapped memory
 * @return A pointer to a buffer that map all the file content. @c NULL if it fail.
 */
EAPI void *eina_file_map_all(Eina_File *file, Eina_File_Populate rule);

/**
 * @brief Map a part of the file.
 *
 * @param file The file handler to map in memory
 * @param rule The rule to apply to the mapped memory
 * @param offset The offset inside the file
 * @param length The length of the memory to map
 * @return A valid pointer to the system memory with @p length valid byte in it. And @c NULL if not inside the file or anything else goes wrong.
 *
 * This does handle refcounting so it will share map that target the same memory area.
 */
EAPI void *eina_file_map_new(Eina_File *file, Eina_File_Populate rule,
                             unsigned long int offset, unsigned long int length);

/**
 * @brief Unref and unmap memory if possible.
 *
 * @param file The file handler to unmap memory from.
 * @param map Memory map to unref and unmap.
 */
EAPI void eina_file_map_free(Eina_File *file, void *map);

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_FILE_H_ */

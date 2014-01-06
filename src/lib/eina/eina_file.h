/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga
 *                    2011 Cedric Bail
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
#include <time.h>
#include <sys/stat.h>

#include "eina_types.h"
#include "eina_array.h"
#include "eina_iterator.h"
#include "eina_tmpstr.h"

/**
 * @page eina_file_example_01_page
 * @dontinclude eina_file_01.c
 *
 * For brevity includes, variable declarations and initialization was omitted
 * from this page, however the full source code can be seen @ref
 * eina_file_example_01 "here".
 *
 * Here we have a simple callback to print the name of a file and the path that
 * contains it:
 * @skip static
 * @until }
 *
 * We can use this callback in the following call:
 * @skipline eina_file_dir_list
 *
 * The above was a way to print the files in a directory, but it is not the only
 * one:
 * @until iterator_free
 *
 * And now two ways to get more information than just file names:
 * @until iterator_free
 * @until iterator_free
 *
 * The above ways of getting files on a list may produce the same output, but
 * they have an important difference, eina_file_direct_ls() will @b not call
 * stat, this means that on some systems it might not have file type
 * information. On the other hand it might be faster than eina_file_stat_ls().
 */
/**
 * @page eina_file_example_01
 * @include eina_file_01.c
 * @example eina_file_01.c
 */
/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */
/**
 * @addtogroup Eina_File_Group File
 *
 * @brief Functions to handle files and directories.
 *
 * This functions make it easier to do a number o file and directory operations
 * such as getting the list of files in a directory, spliting paths and finding
 * out file size and type.
 *
 * @warning All functions in this group are @b blocking which means they make
 * take a long time to return, use them carefully.
 *
 * See an example @ref eina_file_example_01_page "here".
 *
 * @{
 */

/**
 * @typedef Eina_File_Direct_Info
 * A typedef to #_Eina_File_Direct_Info.
 */
typedef struct _Eina_File_Direct_Info Eina_File_Direct_Info;

/**
 * @typedef Eina_Stat
 * A typedef to #_Eina_Stat.
 * @since 1.2
 */
typedef struct _Eina_Stat Eina_Stat;

/**
 * @typedef Eina_File_Lines
 * A typedef to #_Eina_File_Lines.
 */
typedef struct _Eina_File_Line Eina_File_Line;

/**
 * @typedef Eina_File_Dir_List_Cb
 * Type for a callback to be called when iterating over the files of a
 * directory.
 * @param The file name EXCLUDING the path
 * @param path The path passed to eina_file_dir_list()
 * @param data The data passed to eina_file_dir_list()
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
/**
 * @typedef Eina_File_Populate
 * File access type used in Eina_File_Direct_info.
 */
typedef enum {
  EINA_FILE_RANDOM,     /**< Advise random memory access to the mapped memory. */
  EINA_FILE_SEQUENTIAL, /**< Advise sequential memory access to the mapped memory. */
  EINA_FILE_WILLNEED,   /**< Advise need for all the mapped memory. */
  EINA_FILE_POPULATE,   /**< Request all the mapped memory. */
  EINA_FILE_DONTNEED,   /**< Indicate that the memory is no longer needed. This may result in the memory being removed from any caches if applicable. @since 1.8 */
  EINA_FILE_REMOVE      /**< This memory is to be released and any content will be lost. Subsequent accesses will succeed but return fresh memory as if accessed for the first time. This may not suceed if the filesystem does not support it. @since 1.8 */
} Eina_File_Populate;

/* Why do this? Well PATH_MAX may vary from when eina itself is compiled
 * to when the app using eina is compiled. exposing the path buffer below
 * can't safely and portably vary based on how/when you compile. it should
 * always be the same for both eina inside AND for apps outside that use eina
 * so define this to 8192 - most PATH_MAX values are like 4096 or 1024 (with
 * windows i think being 260), so 8192 should cover almost all cases. there
 * is a possibility that PATH_MAX could be more than 8192. if anyone spots
 * a path_max that is bigger - let us know, but, for now we will assume
 * it never happens */
/**
 * @def EINA_PATH_MAX
 * @brief The constant defined as the highest value for PATH_MAX.
 */
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
 * @struct _Eina_Stat
 * A structure to store informations of a path.
 * @since 1.2
 */
struct _Eina_Stat
{
   unsigned long int    dev;
   unsigned long int    ino;
   unsigned int         mode;
   unsigned int         nlink;
   unsigned int         uid;
   unsigned int         gid;
   unsigned long int    rdev;
   unsigned long int    size;
   unsigned long int    blksize;
   unsigned long int    blocks;
   unsigned long int    atime;
   unsigned long int    atimensec;
   unsigned long int    mtime;
   unsigned long int    mtimensec;
   unsigned long int    ctime;
   unsigned long int    ctimensec;
};

/**
 * @struct _Eina_File_Line
 * A structure to store information of line
 * @since 1.3
 */
struct _Eina_File_Line
{
  const char *start;
  const char *end;
  unsigned int index;
  unsigned long long length;
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
 * This function calls @p cb for each file that is in @p dir. To have @p cb
 * called on files that are in subdirectories of @p dir @p recursive should
 * be #EINA_TRUE. In other words if @p recursive is #EINA_FALSE, only direct children
 * of @p dir will be operated on, if @p recursive is #EINA_TRUE the entire tree
 * of files that is below @p dir will be operated on.
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
 * created, @c NULL is returned, otherwise, an array with each part of @p path
 * is returned.
 */
EAPI Eina_Array    *eina_file_split(char *path) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Get an iterator to list the content of a directory.
 *
 * @param  dir The name of the directory to list
 * @return Return an Eina_Iterator that will walk over the files and directories
 *         in @p dir. On failure it will return @c NULL.
 *
 * Returns an iterator for shared strings, the name of each file in @p dir will
 * only be fetched when advancing the iterator, which means there is very little
 * cost associated with creating the list and stopping halfway through it.
 *
 * @warning The iterator will hand the user a stringshared value with the full
 * path. The user must free the string using eina_stringshare_del() on it.
 *
 * @note The container for the iterator is of type DIR*.
 * @note The iterator will walk over '.' and '..' without returning them.
 *
 * @see eina_file_direct_ls()
 */
EAPI Eina_Iterator *eina_file_ls(const char *dir) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Get an iterator to list the content of a directory, with direct
 * information.
 *
 * @param  dir The name of the directory to list
 *
 * @return Return an Eina_Iterator that will walk over the files and
 *         directory in the pointed directory. On failure it will
 *         return NULL.
 *
 * Returns an iterator for Eina_File_Direct_Info, the name of each file in @p
 * dir will only be fetched when advancing the iterator, which means there is
 * cost associated with creating the list and stopping halfway through it.
 *
 * @warning The Eina_File_Direct_Info returned by the iterator <b>must not</b>
 * be modified in any way.
 * @warning When the iterator is advanced or deleted the Eina_File_Direct_Info
 * returned is no longer valid.
 *
 * @note The container for the iterator is of type DIR*.
 * @note The iterator will walk over '.' and '..' without returning them.
 * @note The difference between this function and eina_file_direct_ls() is that
 *       it guarantees the file type information will be correct incurring a
 *       possible performance penalty.
 *
 * @see eina_file_direct_ls()
 */
EAPI Eina_Iterator *eina_file_stat_ls(const char *dir) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Use information provided by Eina_Iterator of eina_file_stat_ls or eina_file_direct_ls
 * to call stat in the most efficient way on your system.
 *
 * @param container The container returned by the Eina_Iterator using eina_iterator_container_get().
 * @param info The content of the current Eina_File_Direct_Info provided by the Eina_Iterator
 * @param buf Where to put the result of the stat
 * @return On success @c 0 is returned, On error @c -1 is returned and errno is set appropriately.
 *
 * This function calls fstatat or stat depending on what your system supports. This makes it efficient and simple
 * to use on your side without complex detection already done inside Eina on what the system can do.
 *
 * @see eina_file_direct_ls()
 * @see eina_file_stat_ls()
 * @since 1.2
 */
EAPI int eina_file_statat(void *container, Eina_File_Direct_Info *info, Eina_Stat *buf) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Generate and create a uniquely named temporary file from template.
 *        Generated file is opened with the open(2) O_EXCL flag.
 *
 * @param [in] templatename is a string. The last six characters of templatename must be XXXXXX. 
 * @param [out] path Where to put the name of the created file. If not NULL
 *              should be released by eina_tmpstr_del.
 * @return On success @c file descriptor of the temporary file is returned, 
 *         On error @c -1 is returned, in which case errno is set appropriately.
 *
 * This function calls mkstemp, generates a unique temporary filename 
 * from template, creates and opens the file, and returns an open file 
 * descriptor for the file. 
 *
 * @see eina_file_mkdtemp()
 * @since 1.8
 */
EAPI int eina_file_mkstemp(const char *templatename, Eina_Tmpstr **path);

/**
 * @brief Generate and create a uniquely named temporary directory from template.
 *
 * @param [in] templatename is a string. The last six characters of templatename must be XXXXXX.
 * @param [out] path Where to put the name of the created directory. If not NULL
 *              should be released by eina_tmpstr_del.
 * @return On success @c EINA_TRUE is returned, On error @c EINA_FALSE is returned, 
 *         in which case errno is set appropriately.
 *
 * This function calls mkdtemp. The directory is then created with permissions 0700. 
 *
 * @see eina_file_mkstemp()
 * @since 1.8
 */
EAPI Eina_Bool eina_file_mkdtemp(const char *templatename, Eina_Tmpstr **path);

/**
 * @brief Get an iterator to list the content of a directory, with direct
 * information.
 *
 * @param  dir The name of the directory to list
 *
 * @return Return an Eina_Iterator that will walk over the files and
 *         directory in the pointed directory. On failure it will
 *         return @c NULL.
 *
 * Returns an iterator for Eina_File_Direct_Info, the name of each file in @p
 * dir will only be fetched when advancing the iterator, which means there is
 * cost associated with creating the list and stopping halfway through it.
 *
 * @warning If readdir_r doesn't contain file type information file type will
 * be DT_UNKNOW.
 * @warning The Eina_File_Direct_Info returned by the iterator <b>must not</b>
 * be modified in any way.
 * @warning When the iterator is advanced or deleted the Eina_File_Direct_Info
 * returned is no longer valid.
 *
 * @note The container for the iterator is of type DIR*.
 * @note The iterator will walk over '.' and '..' without returning them.
 * @note The difference between this function and eina_file_stat_ls() is that
 *       it may not get the file type information however it is likely to be
 *       faster.
 *
 * @see eina_file_ls()
 */
EAPI Eina_Iterator *eina_file_direct_ls(const char *dir) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Sanitize file path.
 *
 * @param path The path to sanitize
 *
 * @return an allocated string with the sanitized path.
 *
 * This function take care of adding the current working directory if it's a
 * relative path and also remove all '..' and '//' reference in the original
 * path.
 *
 * @since 1.1
 */
EAPI char *eina_file_path_sanitize(const char *path);


/**
 * @typedef Eina_File_Copy_Progress
 * used to report progress during eina_file_copy(), where @c done
 * is the bytes already copied and @c size is the total file size.
 *
 * If returns #EINA_FALSE, it will stop the copy.
 */
typedef Eina_Bool (*Eina_File_Copy_Progress)(void *data, unsigned long long done, unsigned long long total);

/**
 * @typedef Eina_File_Copy_Flags
 * what to copy from file.
 */
typedef enum {
  EINA_FILE_COPY_DATA       = 0,
  EINA_FILE_COPY_PERMISSION = (1 << 0),
  EINA_FILE_COPY_XATTR      = (1 << 1)
} Eina_File_Copy_Flags;

/**
 * Copy one file to another using the fastest possible way, report progress.
 *
 * This function will try  splice if it is available. It
 * will block until the whole file is copied or it fails.
 *
 * During the progress it may call back @a cb with the progress summary.
 *
 * @param src the source file.
 * @param dst the destination file.
 * @param flags controls what is copied (data is always copied).
 * @param cb if provided will be called with file copy progress information.
 * @param cb_data context data to provide to @a cb during copy.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise (and @a dst
 *         will be deleted)
 */
EAPI Eina_Bool eina_file_copy(const char *src, const char *dst, Eina_File_Copy_Flags flags, Eina_File_Copy_Progress cb, const void *cb_data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Get a read-only handler to a file.
 *
 * @param name Filename to open
 * @param shared Requested a shm
 * @return Eina_File handle to the file
 *
 * Opens a file in read-only mode. @p name should be an absolute path. An
 * Eina_File handle can be shared among multiple instances if @p shared
 * is #EINA_TRUE.
 *
 * @since 1.1
 */
EAPI Eina_File *eina_file_open(const char *name, Eina_Bool shared) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Create a virtual file from a memory pointer.
 *
 * @param virtual_name A virtual name for Eina_File, if #NULL, a generated one will be given
 * @param data The memory pointer to take data from
 * @param length The length of the data in memory
 * @param copy #EINA_TRUE if the data must be copied
 * @return Eina_File handle to the file
 *
 * @since 1.8
 */
EAPI Eina_File *
eina_file_virtualize(const char *virtual_name, const void *data, unsigned long long length, Eina_Bool copy) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Tell if a file is a real file or only exist in memory
 *
 * @param file The file to test
 * @return #EINA_TRUE if the file is a virtual file
 *
 * @since 1.8
 */
EAPI Eina_Bool
eina_file_virtual(Eina_File *file) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Refresh file information
 *
 * @param file The file to refresh
 * @return #EINA_TRUE if the file has changed
 *
 * All current map continue to exist. You need to manually delete
 * and recreate them to have the new correct mapping.
 *
 * @since 1.8
 */
EAPI Eina_Bool eina_file_refresh(Eina_File *file);  

/**
 * @brief Dup a read-only handler of a previously open file.
 *
 * @param file To duplicate a reference to
 * @return Eina_File handle to the duplicated file
 *
 * Opens a file in read-only mode.
 *
 * @since 1.8
 */
EAPI Eina_File * eina_file_dup(const Eina_File *file);

/**
 * @brief Unref file handler.
 *
 * @param file File handler to unref.
 *
 * Decrement file's refcount and if it reaches zero close it.
 *
 * @since 1.1
 */
EAPI void eina_file_close(Eina_File *file);

/**
 * @brief Get file size at open time.
 *
 * @param file The file handler to request the size from.
 * @return The length of the file.
 *
 * @since 1.1
 */
EAPI size_t eina_file_size_get(const Eina_File *file);

/**
 * @brief Get the last modification time of an open file.
 *
 * @param file The file handler to request the modification time from.
 * @return The last modification time.
 *
 * @since 1.1
 */
EAPI time_t eina_file_mtime_get(const Eina_File *file);

/**
 * @brief Get the filename of an open file.
 *
 * @param file The file handler to request the name from.
 * @return Stringshared filename of the file.
 *
 * @since 1.1
 */
EAPI const char *eina_file_filename_get(const Eina_File *file);

/**
 * @brief Get the eXtended attribute of an open file.
 *
 * @param file The file handler to request the eXtended attribute from.
 * @return an iterator.
 *
 * The iterator will list all eXtended attribute name without allocating
 * them, so you need to copy them yourself if needed.
 *
 * @since 1.2
 */
EAPI Eina_Iterator *eina_file_xattr_get(Eina_File *file);

/**
 * @brief Get the eXtended attribute of an open file.
 *
 * @param file The file handler to request the eXtended attribute from.
 * @return an iterator.
 *
 * The iterator will list all eXtended attribute without allocating
 * them, so you need to copy them yourself if needed. It is returning
 * Eina_Xattr structure.
 *
 * @since 1.2
 */
EAPI Eina_Iterator *eina_file_xattr_value_get(Eina_File *file);

/**
 * @brief Map all the file to a buffer.
 *
 * @param file The file handler to map in memory
 * @param rule The rule to apply to the mapped memory
 * @return A pointer to a buffer that map all the file content. @c NULL if it fail.
 *
 * @since 1.1
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
 *
 * @since 1.1
 */
EAPI void *eina_file_map_new(Eina_File *file, Eina_File_Populate rule,
                             unsigned long int offset, unsigned long int length);

/**
 * @brief Unref and unmap memory if possible.
 *
 * @param file The file handler to unmap memory from.
 * @param map Memory map to unref and unmap.
 *
 * @since 1.1
 */
EAPI void eina_file_map_free(Eina_File *file, void *map);

/**
 * @brief Ask the OS to populate or otherwise pages of memory in file mapping
 * 
 * @param file The file handle from which the map comes
 * @param map Memory that was mapped inside of which the memory range is
 * @param offset The offset in bytes from the start of the map address
 * @param length The length in bytes of the memory region to populate
 * 
 * This advises the operating system as to what to do with the memory mapped
 * to the given @p file. This affects a specific range of memory and may not
 * be honored if the system chooses to ignore the request.
 * 
 * @since 1.8
 */
EAPI void
eina_file_map_populate(Eina_File *file, Eina_File_Populate rule, const void *map,
                       unsigned long int offset, unsigned long int length);

/**
 * @brief Map line by line in memory efficiently with an Eina_Iterator
 * @param file The file to run over
 * @return an Eina_Iterator that will produce @typedef Eina_File_Lines.
 *
 * This function return an iterator that will act like fgets without the
 * useless memcpy. Be aware that once eina_iterator_next has been called,
 * nothing garanty you that the memory will still be mapped.
 *
 * @since 1.3
 */
EAPI Eina_Iterator *eina_file_map_lines(Eina_File *file);

/**
 * @brief Tell if their was an IO error during the life of a mmaped file
 *
 * @param file The file handler to the mmaped file.
 * @param map Memory map to check if an error occurred on it.
 * @return #EINA_TRUE if there was an IO error, #EINA_FALSE otherwise.
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_file_map_faulted(Eina_File *file, void *map);

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_FILE_H_ */

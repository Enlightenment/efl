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
#include "eina_str.h"

/**
 * @page eina_file_example_01_page
 * @dontinclude eina_file_01.c
 *
 * For brevity, variable declarations and initialization are omitted
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
 * The above is a way to print the files in a directory, but it is not the only
 * one:
 * @until iterator_free
 *
 * And now two ways to get more information than just file names:
 * @until iterator_free
 * @until iterator_free
 *
 * The above mentioned ways of getting files on a list may produce the same output, but
 * they have an important difference, eina_file_direct_ls() does @b not call
 * stat, this means that on some systems it might not have file type
 * information. On the other hand, it might be faster than eina_file_stat_ls().
 *
 * @page eina_file_example_01
 * @include eina_file_01.c
 * @example eina_file_01.c
 */

/**
 * @defgroup Eina_File_Group File
 * @ingroup Eina_Tools_Group
 *
 * @brief This group discusses the functions to handle files and directories.
 *
 * @details These functions make it easier to do a number of file and directory operations
 *          such as getting the list of files in a directory, splitting paths, and finding
 *          out file size and type.
 *
 * @warning All functions in this group are @b blocking, which means they may
 *          take a long time to return, use them carefully.
 *
 * See an example @ref eina_file_example_01_page "here".
 *
 * @{
 */

/**
 * @typedef Eina_File_Direct_Info
 * @brief A type for #_Eina_File_Direct_Info.
 */
typedef struct _Eina_File_Direct_Info Eina_File_Direct_Info;

/**
 * @typedef Eina_Stat
 * @brief A type for #_Eina_Stat.
 * @since 1.2
 */
typedef struct _Eina_Stat Eina_Stat;

/**
 * @typedef Eina_File_Line
 * @brief A type for #_Eina_File_Line.
 */
typedef struct _Eina_File_Line Eina_File_Line;

/**
 * @typedef Eina_File_Dir_List_Cb
 * @brief Type for a callback to be called when iterating over the files of a
 *        directory.
 * @param[in] name The file name EXCLUDING the path
 * @param[in] path The path passed to eina_file_dir_list()
 * @param[in] data The data passed to eina_file_dir_list()
 */
typedef void (*Eina_File_Dir_List_Cb)(const char *name, const char *path, void *data);

/**
 * @typedef Eina_File_Type
 * @brief Type for enumeration of a file type.
 * @details This type is used in #Eina_File_Direct_Info.
 */
typedef enum {
  EINA_FILE_UNKNOWN, /**< Unknown file type */
  EINA_FILE_FIFO,    /**< Named pipe (FIFO) type (unused on Windows) */
  EINA_FILE_CHR,     /**< Character device type (unused on Windows) */
  EINA_FILE_DIR,     /**< Directory type */
  EINA_FILE_BLK,     /**< Block device type (unused on Windows) */
  EINA_FILE_REG,     /**< Regular file type */
  EINA_FILE_LNK,     /**< Symbolic link type (unused on Windows) */
  EINA_FILE_SOCK,    /**< UNIX domain socket type (unused on Windows) */
  EINA_FILE_WHT      /**< Whiteout file type (unused on Windows) */
} Eina_File_Type;

/**
 * @typedef Eina_File
 * @brief A file handle.
 */
typedef struct _Eina_File Eina_File;

/**
 * @typedef Eina_File_Populate
 * @brief Type for enumeration of a file access type.
 * @details This type is used in #Eina_File_Direct_Info.
 */
typedef enum {
  EINA_FILE_RANDOM,     /**< Advise random memory access to the mapped memory */
  EINA_FILE_SEQUENTIAL, /**< Advise sequential memory access to the mapped memory */
  EINA_FILE_WILLNEED,   /**< Advise need for all the mapped memory */
  EINA_FILE_POPULATE,   /**< Request for all the mapped memory */
  EINA_FILE_DONTNEED,   /**< Indicate that the memory is no longer needed. This may result in the memory being removed from any caches if applicable. @since 1.8 */
  EINA_FILE_REMOVE      /**< This memory is to be released and any content will be lost. Subsequent accesses will succeed but return fresh memory as if accessed for the first time. This may not succeed if the filesystem does not support it. @since 1.8 */
} Eina_File_Populate;

/* Why do this? Well PATH_MAX may vary from when eina itself is compiled
 * to when the app using eina is compiled. Exposing the path buffer below
 * can't safely and portably vary based on how/when you compile. It should
 * always be the same for both eina inside AND for apps outside that use eina,
 * so define this to 8192 - most PATH_MAX values are like 4096 or 1024 (with
 * Windows I think being 260), so 8192 should cover almost all the cases. There
 * is a possibility that PATH_MAX could be more than 8192. If anyone spots
 * a path_max that is bigger - let us know, but, for now we assume that
 * it never happens */
/**
 * @def EINA_PATH_MAX
 * @brief Definition for the constant defined as the highest value for PATH_MAX.
 */
#define EINA_PATH_MAX 8192

/**
 * @def EINA_PATH_SEP_C
 * @brief The constant defined the path separator as the character '\'
 * on Windows and '/' otherwise.
 *
 * @since 1.16
 */
/**
 * @def EINA_PATH_SEP_S
 * @brief The constant defined the path separator as the string "\" on Windows
 * and "/" otherwise.
 *
 * @since 1.16
 */
#ifdef _WIN32
# define EINA_PATH_SEP_C '\\'
# define EINA_PATH_SEP_S "\\"
#else
# define EINA_PATH_SEP_C '/'
# define EINA_PATH_SEP_S "/"
#endif

/**
 * @struct _Eina_File_Direct_Info
 * @brief The structure to store information of a path.
 */
struct _Eina_File_Direct_Info
{
   size_t               path_length;         /**< Size of the whole path */
   size_t               name_length;         /**< Size of the filename/basename component */
   size_t               name_start;          /**< Start position of the filename/basename component */
   Eina_File_Type       type;                /**< File type */
   char                 path[EINA_PATH_MAX]; /**< The path */
};

/**
 * @struct _Eina_Stat
 * @brief The structure to store some file statistics.
 * @since 1.2
 */
struct _Eina_Stat
{
   unsigned long int    dev;       /**< The device where this file is located */
   unsigned long int    ino;       /**< The inode */
   unsigned int         mode;      /**< The mode */
   unsigned int         nlink;     /**< The link number */
   unsigned int         uid;       /**< The owner user id */
   unsigned int         gid;       /**< The owner group id */
   unsigned long int    rdev;      /**< The remote device */
   unsigned long int    size;      /**< The file size in bytes */
   unsigned long int    blksize;   /**< The block size in bytes */
   unsigned long int    blocks;    /**< The number of blocks allocated */
   unsigned long int    atime;     /**< The timestamp when the file was last accessed */
   unsigned long int    atimensec; /**< The nano version of the timestamp when the file was last accessed */
   unsigned long int    mtime;     /**< The timestamp when the file was last modified */
   unsigned long int    mtimensec; /**< The nano version of the timestamp when the file was modified */
   unsigned long int    ctime;     /**< The timestamp when the file was created */
   unsigned long int    ctimensec; /**< The nano version of the timestamp when the file was created */
};

/**
 * @struct _Eina_File_Line
 * @brief The structure to store information of a line.
 * @since 1.3
 */
struct _Eina_File_Line
{
  const char *start;         /**< The start of the line */
  const char *end;           /**< The end of the line */
  unsigned int index;        /**< The line number */
  unsigned long long length; /**< The number of characters in the line */
};

/**
 * @def EINA_FILE_DIR_LIST_CB
 * @brief The macro to cast to an #Eina_File_Dir_List_Cb.
 * @details This macro casts @p function to #Eina_File_Dir_List_Cb.
 * @param[in] function The function to cast
 *
 */
#define EINA_FILE_DIR_LIST_CB(function) ((Eina_File_Dir_List_Cb)function)


/**
 * @brief Lists all the files on the directory by calling the function for every file found.
 * @details This function calls @p cb for each file that is in @p dir. To have @p cb
 *          called on the files that are in subdirectories of @p dir, @p recursive should
 *          be #EINA_TRUE. In other words, if @p recursive is #EINA_FALSE, only direct children
 *          of @p dir are operated on, if @p recursive is #EINA_TRUE the entire tree
 *          of files that is below @p dir is operated on.
 *
 *          If @p cb or @p dir is @c NULL or if @p dir is a string of size 0,
 *          or if @p dir cannot be opened, this function returns #EINA_FALSE
 *          immediately. Otherwise, it returns #EINA_TRUE.
 *
 * @param[in] dir The directory name
 * @param[in] recursive Iterate recursively in the directory
 * @param[in] cb The callback to be called
 * @param[in] data The data to pass to the callback
 * @return #EINA_TRUE on success, otherwise #EINA_FALSE
 */
EAPI Eina_Bool eina_file_dir_list(const char           *dir,
                                  Eina_Bool             recursive,
                                  Eina_File_Dir_List_Cb cb,
                                  void                 *data) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Splits a path according to the delimiter of the filesystem.
 * @details This function splits @p path according to the delimiter of the used
 *          filesystem. If  @p path is @c NULL or if the array cannot be
 *          created, @c NULL is returned, otherwise an array with each part of @p path
 *          is returned.
 *
 * @param[in] path The path to split
 * @return An array of the parts of the path to split
 *
 */
EAPI Eina_Array    *eina_file_split(char *path) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Gets an iterator to list the content of a directory.
 * @details This returns an iterator for shared strings, the name of each file in @p dir is
 *          only fetched when advancing the iterator, which means there is very little
 *          cost associated with creating the list and stopping halfway through it.
 *
 * @param[in] dir The name of the directory to list
 * @return An #Eina_Iterator that walks over the files and directories
 *         in @p dir. On failure, it returns @c NULL.
 *
 * @note The iterator hands the user a stringshared value with the full
 *       path. The user must free the string using eina_stringshare_del() on it.
 *
 * @note The container for the iterator is of type DIR*.
 * @note The iterator walks over '.' and '..' without returning them.
 *
 * @see eina_file_direct_ls()
 */
EAPI Eina_Iterator *eina_file_ls(const char *dir) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Gets an iterator to list the content of a directory, with direct
 *        information.
 * @details This returns an iterator for #Eina_File_Direct_Info, the name of each file in @p
 *          dir is only fetched when advancing the iterator, which means there is
 *          cost associated with creating the list and stopping halfway through it.
 *
 * @param[in] dir The name of the directory to list
 *
 * @return An #Eina_Iterator that walks over the files and
 *         directories in @p dir. On failure, it returns @c NULL.
 *
 * @warning The #Eina_File_Direct_Info returned by the iterator <b>must not</b>
 *          be modified in any way.
 * @warning When the iterator is advanced or deleted the #Eina_File_Direct_Info
 *          returned is no longer valid.
 *
 * @note The container for the iterator is of type DIR*.
 * @note The iterator walks over '.' and '..' without returning them.
 * @note The difference between this function and eina_file_direct_ls() is that
 *       it guarantees the file type information to be correct by incurring a
 *       possible performance penalty.
 *
 * @see eina_file_direct_ls()
 */
EAPI Eina_Iterator *eina_file_stat_ls(const char *dir) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Uses information provided by #Eina_Iterator of eina_file_stat_ls() or eina_file_direct_ls()
 *        to call stat in the most efficient way on your system.
 * @details This function calls fstatat or stat depending on what your system supports. This makes it efficient and simple
 *          to use on your side without complex detection already done inside Eina on what the system can do.
 *
 * @param[in] container The container returned by #Eina_Iterator using eina_iterator_container_get()
 * @param[in] info The content of the current #Eina_File_Direct_Info provided by #Eina_Iterator
 * @param[in] buf The location put the result of the stat
 * @return @c 0 is returned on success, otherwise @c -1 is returned on error and errno is set appropriately
 *
 * @see eina_file_direct_ls()
 * @see eina_file_stat_ls()
 *
 * @since 1.2
 */
EAPI int eina_file_statat(void *container, Eina_File_Direct_Info *info, Eina_Stat *buf) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Generates and creates a uniquely named temporary file from a template name.
 *        The generated file is opened with the open(2) @c O_EXCL flag.
 * @details This function calls mkstemp(), generates a unique temporary filename
 *          from template, creates and opens the file, and returns an open file
 *          descriptor for the file.
 *
 * @param[in] templatename This is a string. It must contain the six characters 'XXXXXX'
 *                         at the end or directly followed by an extension as in
 *                         'prefixXXXXXX.ext'.
 * @param[out] path The path to the created temporary file, or @c NULL in case of failure.
 *                  It must be released by eina_tmpstr_del().
 * @return On success @c file descriptor of the temporary file is returned,
 *         On error @c -1 is returned, in which case @c errno is set appropriately.
 *
 * @note If a filename extension was specified in @p templatename, then the new @p path
 *       will also contain this extension (since 1.10).
 *
 * @note If the @p templatename is a simple file name (no relative or absolute
 *       path to another directory), then a temporary file will be created inside
 *       the system temporary directory (@see eina_environment_tmp_get()). If the
 *       @p templatename contains a directory separator ('/', or '\\' on Windows)
 *       then the file will be created inside this directory, which must exist and
 *       be writeable. Use ./filename.XXXXXX to create files in the current
 *       working directory. (since 1.17)
 *
 * @see eina_file_mkdtemp()
 * @since 1.8
 */
EAPI int eina_file_mkstemp(const char *templatename, Eina_Tmpstr **path) EINA_ARG_NONNULL(1);

/**
 * @brief Generates and creates a uniquely named temporary directory from a template name.
 * @details This function calls mkdtemp(). The directory is then created with
 *           permissions 0700.
 *
 * @param[in] templatename This is a string. The last six characters of @p templatename
 *                         must be XXXXXX.
 * @param[out] path The path to the created temporary directory, or @c NULL in case of failure.
 *                  It must be released by eina_tmpstr_del().
 * @return On success @c EINA_TRUE is returned, On error @c EINA_FALSE is returned,
 *                    in which case @c errno is set appropriately.
 *
 * @note If the @p templatename is a simple directory name (no relative or absolute
 *       path to another directory), then a temporary directory will be created inside
 *       the system temporary directory (@see eina_environment_tmp_get()). If the
 *       @p templatename contains a directory separator ('/', or '\\' on Windows)
 *       then the temporary directory will be created inside that other directory,
 *       which must exist and be writeable. (since 1.17)
 *
 * @see eina_file_mkstemp()
 * @since 1.8
 */
EAPI Eina_Bool eina_file_mkdtemp(const char *templatename, Eina_Tmpstr **path) EINA_ARG_NONNULL(1,2);

/**
 * @brief Gets an iterator to list the content of a directory, with direct
 *        information.
 * @details This returns an iterator for #Eina_File_Direct_Info, the name of each file in
 *          @p dir is only fetched when advancing the iterator, which means there is
 *          cost associated with creating the list and stopping halfway through it.
 *
 * @param[in] dir The name of the directory to list
 *
 * @return An Eina_Iterator that walks over the files and
 *         directories in @p dir. On failure, it returns @c NULL.
 *
 * @warning If readdir_r doesn't contain file type information, file type is
 *          EINA_FILE_UNKNOWN.
 * @warning The #Eina_File_Direct_Info returned by the iterator <b>must not</b>
 *          be modified in any way.
 * @warning When the iterator is advanced or deleted the #Eina_File_Direct_Info
 *          returned is no longer valid.
 *
 * @note The container for the iterator is of type DIR*.
 * @note The iterator walks over '.' and '..' without returning them.
 * @note The difference between this function and eina_file_stat_ls() is that
 *       it may not get the file type information however it is likely to be
 *       faster.
 *
 * @see eina_file_ls()
 */
EAPI Eina_Iterator *eina_file_direct_ls(const char *dir) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Sanitizes the file path.
 * @details This function takes care of adding the current working directory if its a
 *          relative path and also removes all '..' and '//' references in the original
 *          path.
 *
 * @param[in] path The path to sanitize
 *
 * @return An allocated string with the sanitized path
 *
 * @since 1.1
 */
EAPI char *eina_file_path_sanitize(const char *path);

/**
 * @typedef Eina_File_Copy_Progress
 * @brief Type for a callback to report file copy progress.
 * @details This function is used to report progress during eina_file_copy(), where @p done
 *          is the bytes already copied and @c size is the total file size.
 *
 * @note If it returns #EINA_FALSE, it will stop the copy.
 */
typedef Eina_Bool (*Eina_File_Copy_Progress)(void *data, unsigned long long done, unsigned long long total);

/**
 * @typedef Eina_File_Copy_Flags
 * @brief Enumeration for a flag what to copy from file.
 */
typedef enum {
  EINA_FILE_COPY_DATA       = 0,
  EINA_FILE_COPY_PERMISSION = (1 << 0),
  EINA_FILE_COPY_XATTR      = (1 << 1)
} Eina_File_Copy_Flags;

/**
 * @brief Copies one file to another using the fastest possible way and report progress.
 * @details This function tries to splice if it is available. It is blocked
 *          until the whole file is copied or it fails.
 *
 * @param[in] src The source file.
 * @param[in] dst The destination file.
 * @param[in] flags Controls what is copied (data is always copied).
 * @param[in] cb If it is provided will be called with file copy progress information.
 * @param[in] cb_data Context data to provide to @p cb during copy.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise (and @p dst
 *         will be deleted)
 *
 * @note During the progress it may call back @p cb with the progress summary.
 */
EAPI Eina_Bool eina_file_copy(const char *src, const char *dst, Eina_File_Copy_Flags flags, Eina_File_Copy_Progress cb, const void *cb_data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Gets a read-only handler to a file.
 * @details This opens a file in the read-only mode. @p name should be an absolute path. An
 *          #Eina_File handle can be shared among multiple instances if @p shared
 *          is #EINA_TRUE, otherwise.
 *
 * @param[in] name The filename to open
 * @param[in] shared Requested a shm

 * @return An #Eina_File handle to the file
 *
 * @since 1.1
 */
EAPI Eina_File *eina_file_open(const char *name, Eina_Bool shared) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Creates a virtual file from a memory pointer.
 *
 * @param[in] virtual_name A virtual name for Eina_File, if @c NULL, a generated one will be given
 * @param[in] data The memory pointer to take data from
 * @param[in] length The length of the data in memory
 * @param[in] copy #EINA_TRUE if the data must be copied
 * @return #Eina_File handle to the file
 *
 * @since 1.8
 */
EAPI Eina_File *
eina_file_virtualize(const char *virtual_name, const void *data, unsigned long long length, Eina_Bool copy) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Tells if a file is a real file or only exists in memory.
 *
 * @param[in] file The file to test
 * @return #EINA_TRUE if the file is a virtual file
 *
 * @since 1.8
 */
EAPI Eina_Bool
eina_file_virtual(Eina_File *file) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Refreshes file information.
 * @details All current map continue to exist. You need to manually delete
 *          and recreate them to have the new correct mapping.
 *
 * @param[in,out] file The file to refresh
 * @return #EINA_TRUE if the file has changed
 *
 * @since 1.8
 */
EAPI Eina_Bool eina_file_refresh(Eina_File *file);

/**
 * @brief Duplicates a read-only handler of a previously open file.
 *
 * @param[in] file To duplicate a reference to
 * @return #Eina_File handle of the duplicated file
 *
 * @note Opens a file in read-only mode.
 *
 * @since 1.8
 */
EAPI Eina_File * eina_file_dup(const Eina_File *file);

/**
 * @brief Unrefs the file handler.
 * @details This decrements the file's reference count and if it reaches zero it closes the file.
 *
 * @param[in] file The file handler to unref
 *
 * @since 1.1
 */
EAPI void eina_file_close(Eina_File *file);

/**
 * @brief Gets the file size at open time.
 *
 * @param[in] file The file handler to request the size from
 * @return The length of the file
 *
 * @since 1.1
 */
EAPI size_t eina_file_size_get(const Eina_File *file);

/**
 * @brief Gets the last modification time of an open file.
 *
 * @param[in] file The file handler to request the modification time from
 * @return The last modification time
 *
 * @since 1.1
 */
EAPI time_t eina_file_mtime_get(const Eina_File *file);

/**
 * @brief Gets the filename of an open file.
 *
 * @param[in] file The file handler to request the name from
 * @return The stringshared filename of the file
 *
 * @since 1.1
 */
EAPI const char *eina_file_filename_get(const Eina_File *file);

/**
 * @brief Gets the extended attribute of an open file.
 *
 * @param[in] file The file handler to request the extended attribute from
 * @return An iterator
 *
 * @note The iterator lists all the extended attribute names without allocating
 *       them, so you need to copy them yourself if needed.
 *
 * @since 1.2
 */
EAPI Eina_Iterator *eina_file_xattr_get(Eina_File *file);

/**
 * @brief Gets the extended attribute of an open file.
 *
 * @param[in] file The file handler to request the extended attribute from
 * @return An iterator
 *
 * @note The iterator lists all the extended attribute names without allocating
 *       them, so you need to copy them yourself if needed. It returns the
 *       Eina_Xattr structure.
 *
 * @since 1.2
 */
EAPI Eina_Iterator *eina_file_xattr_value_get(Eina_File *file);

/**
 * @brief Maps all the files to a buffer.
 *
 * @param[in] file The file handler to map in the memory
 * @param[in] rule The rule to apply to the mapped memory
 * @return A pointer to a buffer that maps all the file content \n
 *         @c NULL if it fails
 *
 * @since 1.1
 */
EAPI void *eina_file_map_all(Eina_File *file, Eina_File_Populate rule);

/**
 * @brief Maps a part of the file.
 *
 * @param[in] file The file handler to map in the memory
 * @param[in] rule The rule to apply to the mapped memory
 * @param[in] offset The offset inside the file
 * @param[in] length The length of the memory to map
 * @return A valid pointer to the system memory with @p length valid bytes in it \n
 *         And @c NULL if not inside the file or anything else goes wrong.
 *
 * @note This does handle reference counting so it can share the same memory area.
 *
 * @since 1.1
 */
EAPI void *eina_file_map_new(Eina_File *file, Eina_File_Populate rule,
                             unsigned long int offset, unsigned long int length);

/**
 * @brief Unrefs and unmaps memory if possible.
 *
 * @param[in] file The file handler to unmap memory from
 * @param[in] map The memory map to unref and unmap
 *
 * @since 1.1
 */
EAPI void eina_file_map_free(Eina_File *file, void *map);

/**
 * @brief Asks the OS to populate or otherwise pages of memory in file mapping.
 * @details This advises the operating system as to what to do with the memory mapped
 *          to the given @p file. This affects a specific range of memory and may not
 *          be honored if the system chooses to ignore the request.
 * 
 * @param[in] file The file handle from which the map comes
 * @param[in] rule The rule to apply to the mapped memory
 * @param[in] map Memory that was mapped inside of which the memory range is
 * @param[in] offset The offset in bytes from the start of the map address
 * @param[in] length The length in bytes of the memory region to populate
 * 
 * @since 1.8
 */
EAPI void
eina_file_map_populate(Eina_File *file, Eina_File_Populate rule, const void *map,
                       unsigned long int offset, unsigned long int length);

/**
 * @brief Maps line by line in the memory efficiently using an #Eina_Iterator.
 * @details This function returns an iterator that acts like fgets without
 *          useless memcpy. Be aware that once eina_iterator_next has been called,
 *          nothing can guarantee that the memory is still going to be mapped.
 *
 * @param[in] file The file to run over
 * @return An Eina_Iterator that produces #Eina_File_Line
 *
 * @since 1.3
 */
EAPI Eina_Iterator *eina_file_map_lines(Eina_File *file);

/**
 * @brief Tells whether there has been an IO error during the life of a mmaped file.
 *
 * @param[in] file The file handler to the mmaped file
 * @param[in] map The memory map to check if an error occurred on it
 * @return #EINA_TRUE if there has been an IO error, otherwise #EINA_FALSE
 *
 * @since 1.2
 */
EAPI Eina_Bool eina_file_map_faulted(Eina_File *file, void *map);

/**
 * @brief Joins two paths of known length.
 * @details This function is similar to eina_str_join_len(), but the separator
 *          is '\' on Windows and '/' otherwise.
 *
 * @param[out] dst The buffer to store the result.
 * @param[in] size Size (in byte) of the buffer.
 * @param[in] a First path to use.
 * @param[in] a_len length of @p a.
 * @param[in] b Second path to use.
 * @param[in] b_len length of @p b.
 * @return The number of characters printed.
 *
 * @see eina_str_join_len()
 * @see eina_file_path_join()
 *
 * @since 1.16
 */
static inline size_t eina_file_path_join_len(char *dst,
                                             size_t size,
                                             const char *a,
                                             size_t a_len,
                                             const char *b,
                                             size_t b_len);

/**
 * @brief Joins two paths of known length.
 * @details This function is similar to eina_file_path_join_len(), but will compute
 *          the length of @p a and @p b using strlen(). The path separator is
 *          '\' on Windows and '/' otherwise.
 *
 * @param[out] dst The buffer to store the result.
 * @param[in] size Size (in byte) of the buffer.
 * @param[in] a First string to use.
 * @param[in] b Second string to use.
 * @return The number of characters printed.
 *
 * @see eina_file_path_join_len()
 *
 * @since 1.16
 */
static inline size_t eina_file_path_join(char *dst,
                                         size_t size,
                                         const char *a,
                                         const char *b);


/**
 * @brief Unlinks file.
 * @details This function is a wrapper around the unlink() system call. It removes a link to
 *          a file.
 *
 * @param[in] pathname File name to unlink.
 * @return #EINA_TRUE if the unlink was successful, #EINA_FALSE otherwise..
 *
 * @since 1.19
 */
EAPI Eina_Bool eina_file_unlink(const char *pathname);

/**
 * @brief Make sure a file descriptor will be closed on exec.
 * @details This function is a wrapper around the fnctl() system call. It makes sure
 *          that the fd will be closed whenever exec is called.
 *
 * @param[in] fd File descriptor to enforce close on exec on.
 * @param[in] on #EINA_TRUE will turn close on exec on, #EINA_FALSE will turn it off.
 * @return #EINA_TRUE if it will be closed on exec, #EINA_FALSE otherwise..
 *
 * @since 1.20
 */
EAPI Eina_Bool eina_file_close_on_exec(int fd, Eina_Bool on);

#include "eina_inline_file.x"

/**
 * @}
 */

#endif /* EINA_FILE_H_ */

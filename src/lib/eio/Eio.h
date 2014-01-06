/* EIO - Core asynchronous input/output operation library.
 * Copyright (C) 2010 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
 *           Gustavo Sverzut Barbieri <barbieri@gmail.com>
 *           Vincent "caro" Torri  <vtorri at univ-evry dot fr>
 *           Stephen "okra" Houston <unixtitan@gmail.com>
 *           Guillaume "kuri" Friloux <guillaume.friloux@asp64.com>
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

#ifndef EIO_H__
# define EIO_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <Eina.h>
#include <Eet.h>
#include <Efl_Config.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EIO_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EIO_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */


#ifdef __cplusplus
extern "C" {
#endif

#define EIO_VERSION_MAJOR EFL_VERSION_MAJOR
#define EIO_VERSION_MINOR EFL_VERSION_MINOR

    /**
    * @typedef Eio_Version
    * Represents the current version of EIO
    */
   typedef struct _Eio_Version
     {
        int major; /**< Major version number */
        int minor; /**< Minor version number */
        int micro; /**< Micro version number */
        int revision; /**< Revision number */
     } Eio_Version;

   EAPI extern Eio_Version *eio_version;

/**
 * @file
 * @brief Eio asynchronous input/output library
 *
 * These routines are used for Eio.
 */

/**
 * @page eio_main Eio
 *
 * @date 2012 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref eio_main_intro
 * @li @ref eio_main_compiling
 * @li @ref eio_main_next_steps
 * @li @ref eio_main_intro_example
 *
 * @section eio_main_intro Introduction
 *
 * The Eio library is a library that implements an API for asynchronous
 * input/output operation. Most operation are done in a separated thread
 * to prevent lock. See @ref Eio_Group. Some helper to work on data
 * received in Eio callback are also provided see @ref Eio_Helper.
 * It is also possible to work asynchronously on Eina_File with @ref Eio_Map
 * or on Eet_File with @ref Eio_Eet. It come with way to manipulate
 * eXtended attribute assynchronously with @ref Eio_Xattr.
 *
 * This library is cross-platform and can be compiled and used on
 * Linux, BSD, Opensolaris and Windows (XP and CE). It is heavily
 * based on @ref Ecore_Main_Loop_Group.
 *
 * @section eio_main_compiling How to compile
 *
 * Eio is a library your application links to. The procedure for this is
 * very simple. You simply have to compile your application with the
 * appropriate compiler flags that the @c pkg-config script outputs. For
 * example:
 *
 * Compiling C or C++ files into object files:
 *
 * @verbatim
   gcc -c -o main.o main.c `pkg-config --cflags eio`
   @endverbatim
 *
 * Linking object files into a binary executable:
 *
 * @verbatim
   gcc -o my_application main.o `pkg-config --libs eio`
   @endverbatim
 *
 * See @ref pkgconfig
 *
 * @section eio_main_next_steps Next Steps
 *
 * After you understood what Eio is and installed it in your system
 * you should proceed understanding the programming interface.
 *
 * Recommended reading:
 *
 * @li @ref Eio_Helper for common functions and library initialization.
 * @li @ref Eio_Map to manipulate files asynchronously (mmap).
 * @li @ref Eio_Xattr to access file extended attributes (xattr).
 * @li @ref Eio_Monitor to monitor for file changes (inotify).
 * @li @ref Eio_Eet to access Eet files asynchronously.
 *
 * @section eio_main_intro_example Introductory Example
 *
 * @include eio_file_ls.c
 *
 * More examples can be found at @ref eio_examples.
 *
 * @{
 */

/**
 * @enum _Eio_File_Op
 *
 * @brief Input/Output operations on files.
 *
 * This enum represents the operations that can be done.
 */
enum _Eio_File_Op
{
  EIO_FILE_COPY, /**< I/O operation is about a specific file copy */
  EIO_FILE_MOVE, /**< I/O operation is about a specific file move */
  EIO_DIR_COPY, /**< I/O operation is about a specific directory copy */
  EIO_DIR_MOVE, /**< I/O operation is about a specific directory move */
  /** I/O operation is about destroying a path:
   * source will point to base path to be destroyed,
   * and dest will point to to path destroyed by this I/O
   */
  EIO_UNLINK,
  EIO_FILE_GETPWNAM, /**< I/O operation is trying to get uid from user name */
  EIO_FILE_GETGRNAM /**< I/O operation is trying to get gid from user name */
};

/**
 * @typedef Eio_File_Op
 * Input/Output operations on files.
 */
typedef enum _Eio_File_Op Eio_File_Op;

/**
 * @typedef Eio_File
 * Generic asynchronous I/O reference.
 */
typedef struct _Eio_File Eio_File;

/**
 * @typedef Eio_Progress
 * Progress information on a specific operation.
 */
typedef struct _Eio_Progress Eio_Progress;

typedef Eina_Bool (*Eio_Filter_Cb)(void *data, Eio_File *handler, const char *file);
typedef void      (*Eio_Main_Cb)(void *data, Eio_File *handler, const char *file);

typedef Eina_Bool (*Eio_Filter_Direct_Cb)(void *data, Eio_File *handler, const Eina_File_Direct_Info *info);
typedef Eina_Bool (*Eio_Filter_Dir_Cb)(void *data, Eio_File *handler, Eina_File_Direct_Info *info);
typedef void      (*Eio_Main_Direct_Cb)(void *data, Eio_File *handler, const Eina_File_Direct_Info *info);

typedef void (*Eio_Stat_Cb)(void *data, Eio_File *handler, const Eina_Stat *stat);
typedef void (*Eio_Progress_Cb)(void *data, Eio_File *handler, const Eio_Progress *info);

typedef void      (*Eio_Eet_Open_Cb)(void *data, Eio_File *handler, Eet_File *file);
typedef void      (*Eio_Open_Cb)(void *data, Eio_File *handler, Eina_File *file);
typedef Eina_Bool (*Eio_Filter_Map_Cb)(void *data, Eio_File *handler, void *map, size_t length);
typedef void      (*Eio_Map_Cb)(void *data, Eio_File *handler, void *map, size_t length);

typedef void (*Eio_Done_Data_Cb)(void *data, Eio_File *handler, const char *read_data, unsigned int size);
typedef void (*Eio_Done_String_Cb)(void *data, Eio_File *handler, const char *xattr_string);
typedef void (*Eio_Done_Double_Cb)(void *data, Eio_File *handler, double xattr_double);
typedef void (*Eio_Done_Int_Cb)(void *data, Eio_File *handler, int i);

typedef void (*Eio_Done_ERead_Cb)(void *data, Eio_File *handler, void *decoded);
typedef void (*Eio_Done_Read_Cb)(void *data, Eio_File *handler, void *read_data, unsigned int size);
typedef void (*Eio_Done_Cb)(void *data, Eio_File *handler);
typedef void (*Eio_Error_Cb)(void *data, Eio_File *handler, int error);
typedef void (*Eio_Eet_Error_Cb)(void *data, Eio_File *handler, Eet_Error err);

/**
  * @struct _Eio_Progress
  * @brief Represents the current progress of the operation.
  */
struct _Eio_Progress
{
   Eio_File_Op op; /**< I/O type */

   long long current; /**< Current step in the I/O operation */
   long long max; /**< Number of total steps to complete this I/O */
   float percent; /**< Percent done for the I/O operation */

   const char *source; /**< source of the I/O operation */
   const char *dest; /**< target of the I/O operation */
};

/**
 * @brief List contents of a directory without locking your app.
 * @param dir The directory to list.
 * @param filter_cb Callback used to decide if the file will be passed to main_cb
 * @param main_cb Callback called for each listed file if it was not filtered.
 * @param done_cb Callback called when the ls operation is done.
 * @param error_cb Callback called when either the directory could not be opened or the operation has been canceled.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * This function is responsible for listing the content of a directory without blocking your application.
 * It's equivalent to the "ls" shell command. Every file will be passed to the
 * filter_cb, so it's your job to decide if you want to pass the file to the
 * main_cb or not. Return EINA_TRUE to pass it to the main_cb or EINA_FALSE to
 * ignore it.
 */
EAPI Eio_File *eio_file_ls(const char *dir,
			   Eio_Filter_Cb filter_cb,
			   Eio_Main_Cb main_cb,
			   Eio_Done_Cb done_cb,
			   Eio_Error_Cb error_cb,
			   const void *data);

/**
 * @brief List contents of a directory without locking your app.
 * @param dir The directory to list.
 * @param filter_cb Callback used to decide if the file will be passed to main_cb
 * @param main_cb Callback called from the main loop for each accepted file (not filtered).
 * @param done_cb Callback called from the main loop after the contents of the directory has been listed.
 * @param error_cb Callback called from the main loop when either the directory could not be opened or the operation has been canceled.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * eio_file_direct_ls runs eina_file_direct_ls in a separate thread using
 * ecore_thread_feedback_run. This prevents any blocking in your apps.
 * Every file will be passed to the filter_cb, so it's your job to decide if you
 * want to pass the file to the main_cb or not. Return EINA_TRUE to pass it to
 * the main_cb or EINA_FALSE to ignore it.
 */
EAPI Eio_File *eio_file_direct_ls(const char *dir,
				  Eio_Filter_Direct_Cb filter_cb,
				  Eio_Main_Direct_Cb main_cb,
				  Eio_Done_Cb done_cb,
				  Eio_Error_Cb error_cb,
				  const void *data);

/**
 * @brief List content of a directory without locking your app.
 * @param dir The directory to list.
 * @param filter_cb Callback used to decide if the file will be passed to main_cb
 * @param main_cb Callback called from the main loop for each accepted file (not filtered).
 * @param done_cb Callback called from the main loop after the contents of the directory has been listed.
 * @param error_cb Callback called from the main loop when either the directory could not be opened or the operation has been canceled.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * Every file will be passed to the filter_cb, so it's your job to decide if you
 * want to pass the file to the main_cb or not. Return EINA_TRUE to pass it to
 * the main_cb or EINA_FALSE to ignore it.
 *
 */
EAPI Eio_File *eio_file_stat_ls(const char *dir,
                                Eio_Filter_Direct_Cb filter_cb,
                                Eio_Main_Direct_Cb main_cb,
                                Eio_Done_Cb done_cb,
                                Eio_Error_Cb error_cb,
                                const void *data);

/**
 * @brief List the content of a directory and all it's sub-content asynchronously
 * @param dir The directory to list.
 * @param filter_cb Callback used to decide if the file will be passed to main_cb
 * @param main_cb Callback called from the main loop for each accepted file (not filtered).
 * @param done_cb Callback called from the main loop after the contents of the directory has been listed.
 * @param error_cb Callback called from the main loop when either the directory could not be opened or the operation has been canceled.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * eio_dir_stat_ls() runs eina_file_stat_ls() recursively in a separate thread using
 * ecore_thread_feedback_run. This prevents any blocking in your apps.
 * Every file will be passed to the
 * filter_cb, so it's your job to decide if you want to pass the file to the
 * main_cb or not. Return EINA_TRUE to pass it to the main_cb or EINA_FALSE to
 * ignore it.
 */
EAPI Eio_File *eio_dir_stat_ls(const char *dir,
                               Eio_Filter_Direct_Cb filter_cb,
                               Eio_Main_Direct_Cb main_cb,
                               Eio_Done_Cb done_cb,
                               Eio_Error_Cb error_cb,
                               const void *data);

/**
 * @brief List the content of a directory and all it's sub-content asynchronously
 * @param dir The directory to list.
 * @param filter_cb Callback used to decide if the file will be passed to main_cb
 * @param main_cb Callback called from the main loop for each accepted file (not filtered).
 * @param done_cb Callback called from the main loop after the contents of the directory has been listed.
 * @param error_cb Callback called from the main loop when either the directory could not be opened or the operation has been canceled.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * eio_dir_direct_ls() runs eina_file_direct_ls() recursively in a separate thread using
 * ecore_thread_feedback_run. This prevents any blocking in your apps.
 * Every file will be passed to the filter_cb, so it's your job to decide if you
 * want to pass the file to the main_cb or not. Return EINA_TRUE to pass it to
 * the main_cb or EINA_FALSE to ignore it.
 */
EAPI Eio_File *eio_dir_direct_ls(const char *dir,
				 Eio_Filter_Dir_Cb filter_cb,
				 Eio_Main_Direct_Cb main_cb,
				 Eio_Done_Cb done_cb,
				 Eio_Error_Cb error_cb,
				 const void *data);

/**
 * @brief Stat a file/directory.
 * @param path The path to stat.
 * @param done_cb Callback called from the main loop when stat was successfully called.
 * @param error_cb Callback called from the main loop when stat failed or has been canceled.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * eio_file_direct_stat calls stat in another thread. This prevents any blocking in your apps.
 */
EAPI Eio_File *eio_file_direct_stat(const char *path,
				    Eio_Stat_Cb done_cb,
				    Eio_Error_Cb error_cb,
				    const void *data);

/**
 * @brief Change right of a path.
 * @param path The directory path to change access right.
 * @param mode The permission to set, follow (mode & ~umask & 0777).
 * @param done_cb Callback called when the operation is completed.
 * @param error_cb Callback called from if something goes wrong.
 * @param data Unmodified user data passed to callbacks.
 * @return A reference to the I/O operation.
 *
 * Set a new permission of a path changing it to the mode passed as argument.
 * It's equivalent to the chmod command.
 */
EAPI Eio_File *eio_file_chmod(const char *path,
                              mode_t mode,
                              Eio_Done_Cb done_cb,
                              Eio_Error_Cb error_cb,
                              const void *data);

/**
 * @brief Change owner of a path.
 * @param path The directory path to change owner.
 * @param user The new user to set (can be NULL).
 * @param group The new group to set (can be NULL).
 * @param done_cb Callback called when the operation is completed.
 * @param error_cb Callback called from if something goes wrong.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * This function will change the owner of a path, setting it to the user and
 * group passed as argument. It's equivalent to the chown shell command.
 */
EAPI Eio_File *eio_file_chown(const char *path,
                              const char *user,
                              const char *group,
                              Eio_Done_Cb done_cb,
                              Eio_Error_Cb error_cb,
                              const void *data);

/**
 * @brief Unlink a file/directory.
 * @param path The path to unlink.
 * @param done_cb Callback called when the operation is completed.
 * @param error_cb Callback called from if something goes wrong.
 * @param data Unmodified user data passed to callbacks.
 * @return A reference to the I/O operation.
 *
 * This function will erase a file.
 */
EAPI Eio_File *eio_file_unlink(const char *path,
			       Eio_Done_Cb done_cb,
			       Eio_Error_Cb error_cb,
			       const void *data);

/**
 * @brief Create a new directory.
 * @param path The directory path to create.
 * @param mode The permission to set, follow (mode & ~umask & 0777).
 * @param done_cb Callback called when the operation is completed.
 * @param error_cb Callback called from if something goes wrong.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * Creates a new directory using the mode provided.
 */
EAPI Eio_File *eio_file_mkdir(const char *path,
			      mode_t mode,
			      Eio_Done_Cb done_cb,
			      Eio_Error_Cb error_cb,
			      const void *data);

/**
 * @brief Move a file asynchronously
 * @param source Should be the name of the file to move the data from.
 * @param dest Should be the name of the file to move the data to.
 * @param progress_cb Callback called to know the progress of the move.
 * @param done_cb Callback called when the move is done.
 * @param error_cb Callback called when something goes wrong.
 * @param data Unmodified user data passed to callbacks
 *
 * This function will copy a file from source to dest. It will try to use splice
 * if possible, if not it will fallback to mmap/write. It will try to preserve
 * access right, but not user/group identification.
 */
EAPI Eio_File *eio_file_move(const char *source,
			     const char *dest,
			     Eio_Progress_Cb progress_cb,
			     Eio_Done_Cb done_cb,
			     Eio_Error_Cb error_cb,
			     const void *data);

/**
 * @brief Copy a file asynchronously
 * @param source Should be the name of the file to copy the data from.
 * @param dest Should be the name of the file to copy the data to.
 * @param progress_cb Callback called to know the progress of the copy.
 * @param done_cb Callback called when the copy is done.
 * @param error_cb Callback called when something goes wrong.
 * @param data Unmodified user data passed to callbacks
 *
 * This function will copy a file from source to dest. It will try to use splice
 * if possible, if not it will fallback to mmap/write. It will try to preserve
 * access right, but not user/group identification.
 */
EAPI Eio_File *eio_file_copy(const char *source,
			     const char *dest,
			     Eio_Progress_Cb progress_cb,
			     Eio_Done_Cb done_cb,
			     Eio_Error_Cb error_cb,
			     const void *data);

/**
 * @brief Move a directory and it's content asynchronously
 * @param source Should be the name of the directory to copy the data from.
 * @param dest Should be the name of the directory to copy the data to.
 * @param filter_cb Possible to deny the move of some files/directories.
 * @param progress_cb Callback called to know the progress of the copy.
 * @param done_cb Callback called when the copy is done.
 * @param error_cb Callback called when something goes wrong.
 * @param data Unmodified user data passed to callbacks
 *
 * This function will move a directory and all it's content from source to dest.
 * It will try first to rename the directory, if not it will try to use splice
 * if possible, if not it will fallback to mmap/write.
 * It will try to preserve access right, but not user/group identity.
 * Every file will be passed to the filter_cb, so it's your job to decide if you
 * want to pass the file to the main_cb or not. Return EINA_TRUE to pass it to
 * the main_cb or EINA_FALSE to ignore it.
 *
 * @note if a rename occur, the filter callback will not be called.
 */
EAPI Eio_File *eio_dir_move(const char *source,
			    const char *dest,
                            Eio_Filter_Direct_Cb filter_cb,
			    Eio_Progress_Cb progress_cb,
			    Eio_Done_Cb done_cb,
			    Eio_Error_Cb error_cb,
			    const void *data);

/**
 * @brief Copy a directory and it's content asynchronously
 * @param source Should be the name of the directory to copy the data from.
 * @param dest Should be the name of the directory to copy the data to.
 * @param filter_cb Possible to deny the move of some files/directories.
 * @param progress_cb Callback called to know the progress of the copy.
 * @param done_cb Callback called when the copy is done.
 * @param error_cb Callback called when something goes wrong.
 * @param data Unmodified user data passed to callbacks
 *
 * This function will copy a directory and all it's content from source to dest.
 * It will try to use splice if possible, if not it will fallback to mmap/write.
 * It will try to preserve access right, but not user/group identity.
 * Every file will be passed to the filter_cb, so it's your job to decide if you
 * want to pass the file to the main_cb or not. Return EINA_TRUE to pass it to
 * the main_cb or EINA_FALSE to ignore it.
 */
EAPI Eio_File *eio_dir_copy(const char *source,
			    const char *dest,
                            Eio_Filter_Direct_Cb filter_cb,
			    Eio_Progress_Cb progress_cb,
			    Eio_Done_Cb done_cb,
			    Eio_Error_Cb error_cb,
			    const void *data);

/**
 * @brief Remove a directory and it's content asynchronously
 * @param path Should be the name of the directory to destroy.
 * @param filter_cb Possible to deny the move of some files/directories.
 * @param progress_cb Callback called to know the progress of the copy.
 * @param done_cb Callback called when the copy is done.
 * @param error_cb Callback called when something goes wrong.
 * @param data Unmodified user data passed to callbacks
 *
 * This function will remove a directory and all it's content.
 * Every file will be passed to the filter_cb, so it's your job to decide if you
 * want to pass the file to the main_cb or not. Return EINA_TRUE to pass it to
 * the main_cb or EINA_FALSE to ignore it.
 */
EAPI Eio_File *eio_dir_unlink(const char *path,
                              Eio_Filter_Direct_Cb filter_cb,
			      Eio_Progress_Cb progress_cb,
			      Eio_Done_Cb done_cb,
			      Eio_Error_Cb error_cb,
			      const void *data);
/**
 * @}
 */


/**
 * @defgroup Eio_Xattr Eio manipulation of eXtended attribute.
 * @ingroup Eio
 *
 * @brief A set of function to manipulate data associated with a specific file
 *
 * The functions provided by this API are responsible to manage Extended
 * attribute files. Like file authors, character encoding, checksum, etc.
 * @{
 */

/**
 * @brief Assynchronously list all eXtended attribute
 * @param path The path to get the eXtended attribute from.
 * @param filter_cb Callback called in the thread to validate the eXtended attribute.
 * @param main_cb Callback called in the main loop for each accepted eXtended attribute.
 * @param done_cb Callback called in the main loop when the all the eXtended attribute have been listed.
 * @param error_cb Callback called in the main loop when something goes wrong during the listing of the eXtended attribute.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 */
EAPI Eio_File *eio_file_xattr(const char *path,
			      Eio_Filter_Cb filter_cb,
			      Eio_Main_Cb main_cb,
			      Eio_Done_Cb done_cb,
			      Eio_Error_Cb error_cb,
			      const void *data);

/**
 * @brief Define an extented attribute on a file/directory.
 * @param path The path to set the attribute on.
 * @param attribute The name of the attribute to define.
 * @param xattr_int The value to link the attribute with.
 * @param flags Wether to insert, replace or create the attribute.
 * @param done_cb The callback called from the main loop when setxattr succeeded.
 * @param error_cb The callback called from the main loop when setxattr failed.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * eio_file_xattr_int_set calls eina_xattr_int_set from another thread. This prevents blocking in your apps. If
 * the writing succeeded, the done_cb will be called even if a cancel was requested, but came to late.
 */
EAPI Eio_File *eio_file_xattr_int_set(const char *path,
				      const char *attribute,
				      int xattr_int,
				      Eina_Xattr_Flags flags,
				      Eio_Done_Cb done_cb,
				      Eio_Error_Cb error_cb,
				      const void *data);

/**
 * @brief Define an extented attribute on a file/directory.
 * @param path The path to set the attribute on.
 * @param attribute The name of the attribute to define.
 * @param xattr_double The value to link the attribute with.
 * @param flags Wether to insert, replace or create the attribute.
 * @param done_cb The callback called from the main loop when setxattr succeeded.
 * @param error_cb The callback called from the main loop when setxattr failed.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * eio_file_xattr_double_set calls eina_xattr_double_set from another thread. This prevents blocking in your apps. If
 * the writing succeeded, the done_cb will be called even if a cancel was requested, but came to late.
 */
EAPI Eio_File *eio_file_xattr_double_set(const char *path,
					 const char *attribute,
					 double xattr_double,
					 Eina_Xattr_Flags flags,
					 Eio_Done_Cb done_cb,
					 Eio_Error_Cb error_cb,
					 const void *data);
/**
 * @brief Define a string extented attribute on a file/directory.
 * @param path The path to set the attribute on.
 * @param attribute The name of the attribute to define.
 * @param xattr_string The string to link the attribute with.
 * @param flags Wether to insert, replace or create the attribute.
 * @param done_cb The callback called from the main loop when setxattr succeeded.
 * @param error_cb The callback called from the main loop when setxattr failed.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * eio_file_xattr_string_set calls eina_xattr_string_set from another thread. This prevents blocking in your apps. If
 * the writing succeeded, the done_cb will be called even if a cancel was requested, but came to late.
 */
EAPI Eio_File *eio_file_xattr_string_set(const char *path,
					 const char *attribute,
					 const char *xattr_string,
					 Eina_Xattr_Flags flags,
					 Eio_Done_Cb done_cb,
					 Eio_Error_Cb error_cb,
					 const void *data);
/**
 * @brief Define an extented attribute on a file/directory.
 * @param path The path to set the attribute on.
 * @param attribute The name of the attribute to define.
 * @param xattr_data The data to link the attribute with.
 * @param xattr_size The size of the data to set.
 * @param flags Wether to insert, replace or create the attribute.
 * @param done_cb The callback called from the main loop when setxattr succeeded.
 * @param error_cb The callback called from the main loop when setxattr failed.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * eio_file_xattr_set calls setxattr from another thread. This prevents blocking in your apps. If
 * the writing succeeded, the done_cb will be called even if a cancel was requested, but came to late.
 */
EAPI Eio_File *eio_file_xattr_set(const char *path,
				  const char *attribute,
				  const char *xattr_data,
				  unsigned int xattr_size,
				  Eina_Xattr_Flags flags,
				  Eio_Done_Cb done_cb,
				  Eio_Error_Cb error_cb,
				  const void *data);

/**
 * @brief Retrieve the extended attribute of a file/directory.
 * @param path The path to retrieve the extended attribute from.
 * @param attribute The name of the attribute to retrieve.
 * @param done_cb Callback called from the main loop when getxattr succeeded.
 * @param error_cb Callback called from the main loop when getxattr failed or has been canceled.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * eio_file_xattr_get calls getxattr from another thread. This prevents blocking in your apps.
 */
EAPI Eio_File *eio_file_xattr_get(const char *path,
				  const char *attribute,
				  Eio_Done_Data_Cb done_cb,
				  Eio_Error_Cb error_cb,
				  const void *data);
/**
 * @brief Retrieve a extended attribute of a file/directory.
 * @param path The path to retrieve the extended attribute from.
 * @param attribute The name of the attribute to retrieve.
 * @param done_cb Callback called from the main loop when getxattr succeeded.
 * @param error_cb Callback called from the main loop when getxattr failed or has been canceled.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * eio_file_xattr_int_get calls eina_xattr_int_get from another thread. This prevents blocking in your apps.
 */
EAPI Eio_File *eio_file_xattr_int_get(const char *path,
				      const char *attribute,
				      Eio_Done_Int_Cb done_cb,
				      Eio_Error_Cb error_cb,
				      const void *data);
/**
 * @brief Retrieve a extended attribute of a file/directory.
 * @param path The path to retrieve the extended attribute from.
 * @param attribute The name of the attribute to retrieve.
 * @param done_cb Callback called from the main loop when getxattr succeeded.
 * @param error_cb Callback called from the main loop when getxattr failed or has been canceled.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * eio_file_xattr_double_get calls eina_xattr_double_get from another thread. This prevents blocking in your apps.
 */
EAPI Eio_File *eio_file_xattr_double_get(const char *path,
					 const char *attribute,
					 Eio_Done_Double_Cb done_cb,
					 Eio_Error_Cb error_cb,
					 const void *data);
/**
 * @brief Retrieve a string extended attribute of a file/directory.
 * @param path The path to retrieve the extended attribute from.
 * @param attribute The name of the attribute to retrieve.
 * @param done_cb Callback called from the main loop when getxattr succeeded.
 * @param error_cb Callback called from the main loop when getxattr failed or has been canceled.
 * @param data Unmodified user data passed to callbacks
 * @return A reference to the I/O operation.
 *
 * eio_file_xattr_string_get calls eina_xattr_string_get from another thread. This prevents blocking in your apps.
 */
EAPI Eio_File *eio_file_xattr_string_get(const char *path,
					 const char *attribute,
					 Eio_Done_String_Cb done_cb,
					 Eio_Error_Cb error_cb,
					 const void *data);

/**
 * @}
 */

/**
 * @defgroup Eio_Helper Eio Reference helper API
 * @ingroup Eio
 *
 * @brief This are helper provided around core Eio API.
 *
 * This set of functions do provide helper to work around data
 * provided by Eio without the need to look at system header.
 *
 * @{
 */


/**
 * @brief Initialize eio and all it's required submodule.
 * @return the current number of eio users.
 */
EAPI int eio_init(void);

/**
 * @brief Shutdown eio and all it's submodule if possible.
 * @return the number of pending users of eio.
 */
EAPI int eio_shutdown(void);

/**
 * @brief Return the container during EIO operation
 * @param ls The asynchronous I/O operation to retrieve container from.
 * @return NULL if not available, a DIRP if it is.
 *
 * This is only available and make sense in the thread callback, not in
 * the mainloop.
 */
EAPI void *eio_file_container_get(Eio_File *ls);

/**
 * @brief Cancel any Eio_File.
 * @param ls The asynchronous I/O operation to cancel.
 * @return EINA_FALSE if the destruction is delayed, EINA_TRUE if it's done.
 *
 * This will cancel any kind of I/O operation and cleanup the mess. This means
 * that it could take time to cancel an I/O.
 */
EAPI Eina_Bool eio_file_cancel(Eio_File *ls);

/**
 * @brief Check if an Eio_File operation has been cancelled.
 * @param ls The asynchronous I/O operation to check.
 * @return EINA_TRUE if it was canceled, EINA_FALSE other wise.
 *
 * In case of an error it also return EINA_TRUE.
 */
EAPI Eina_Bool eio_file_check(Eio_File *ls);

/**
 * @brief Associate data with the current filtered file.
 * @param ls The Eio_File ls request currently calling the filter callback.
 * @param key The key to associate data to.
 * @param data The data to associate the data to.
 * @param free_cb Optionally a function to call to free the associated data,
 * @p data is passed as the callback data parameter. If no @p free_cb is provided
 * the user @p data remains untouched.
 * @return EINA_TRUE if insertion was fine.
 *
 * This function can only be safely called from within the filter callback.
 * If you don't need to copy the key around you can use @ref eio_file_associate_direct_add
 */
EAPI Eina_Bool eio_file_associate_add(Eio_File *ls,
				      const char *key,
				      const void *data, Eina_Free_Cb free_cb);

/**
 * @brief Associate data with the current filtered file.
 * @param ls The Eio_File ls request currently calling the filter callback.
 * @param key The key to associate data to (will not be copied, and the pointer will not be used as long as the file is not notified).
 * @param data The data to associate the data to.
 * @param free_cb The function to call to free the associated data, @p free_cb will be called if not specified.
 * @return EINA_TRUE if insertion was fine.
 *
 * This function can only be safely called from within the filter callback.
 * If you need eio to make a proper copy of the @p key to be safe use
 * @ref eio_file_associate_add instead.
 */
EAPI Eina_Bool eio_file_associate_direct_add(Eio_File *ls,
					     const char *key,
					     const void *data, Eina_Free_Cb free_cb);

/**
 * @brief Get the data associated during the filter callback inside the main loop
 * @param ls The Eio_File ls request currently calling the notify callback.
 * @param key The key pointing to the data to retrieve.
 * @return the data associated with the key or @p NULL if not found.
 */
EAPI void *eio_file_associate_find(Eio_File *ls, const char *key);

/**
 * @brief get access time from a Eina_Stat
 * @param stat the structure to get the atime from
 * @return the last accessed time
 *
 * This take care of doing type conversion to match rest of EFL time API.
 * @note some filesystem don't update that information.
 */
static inline double eio_file_atime(const Eina_Stat *stat);

/**
 * @brief get modification time from a Eina_Stat
 * @param stat the structure to get the mtime from
 * @return the last modification time
 *
 * This take care of doing type conversion to match rest of EFL time API.
 */
static inline double eio_file_mtime(const Eina_Stat *stat);

/**
 * @brief get the size of the file described in Eina_Stat
 * @param stat the structure to get the size from
 * @return the size of the file
 */
static inline long long eio_file_size(const Eina_Stat *stat);

/**
 * @brief tell if the stated path was a directory or not.
 * @param stat the structure to get the size from
 * @return EINA_TRUE, if it was.
 */
static inline Eina_Bool eio_file_is_dir(const Eina_Stat *stat);

/**
 * @brief tell if the stated path was a link or not.
 * @param stat the structure to get the size from
 * @return EINA_TRUE, if it was.
 */
static inline Eina_Bool eio_file_is_lnk(const Eina_Stat *stat);

/**
 * @}
 */

/**
 *
 */

/**
 * @defgroup Eio_Map Manipulate an Eina_File asynchronously
 * @ingroup Eio
 *
 * @brief This function help manipulating file asynchronously.
 *
 * This set of function work on top of Eina_File and Ecore_Thread to
 * do basic operations in a file, like openning, closing and mapping a file to
 * memory.
 * @{
 */

/**
 * @brief Assynchronously open a file.
 * @param name The file to open.
 * @param shared If it's a shared memory file.
 * @param open_cb Callback called in the main loop when the file has been successfully opened.
 * @param error_cb Callback called in the main loop when the file couldn't be opened.
 * @param data Unmodified user data passed to callbacks
 * @return Pointer to the file if successfull or NULL otherwise.
 *
 */
EAPI Eio_File *eio_file_open(const char *name, Eina_Bool shared,
                             Eio_Open_Cb open_cb,
                             Eio_Error_Cb error_cb,
                             const void *data);

/**
 * @brief Assynchronously close a file.
 * @param f The file to close.
 * @param done_cb Callback called in the main loop when the file has been successfully closed.
 * @param error_cb Callback called in the main loop when the file couldn't be closed.
 * @param data Unmodified user data passed to callbacks
 * @return Pointer to the file if successfull or NULL otherwise.
 */
EAPI Eio_File *eio_file_close(Eina_File *f,
                              Eio_Done_Cb done_cb,
                              Eio_Error_Cb error_cb,
                              const void *data);

/**
 * @brief Assynchronously map a file in memory.
 * @param f The file to map.
 * @param rule The rule to apply to the map.
 * @param filter_cb Callback called in the thread to validate the content of the map.
 * @param map_cb Callback called in the main loop when the file has been successfully mapped.
 * @param error_cb Callback called in the main loop when the file can't be mapped.
 * @param data Unmodified user data passed to callbacks
 * @return Pointer to the file if successfull or NULL otherwise.
 *
 * The container of the Eio_File is the Eina_File.
 */
EAPI Eio_File *eio_file_map_all(Eina_File *f,
                                Eina_File_Populate rule,
                                Eio_Filter_Map_Cb filter_cb,
                                Eio_Map_Cb map_cb,
                                Eio_Error_Cb error_cb,
                                const void *data);

/**
 * @brief Assynchronously map a part of a file in memory.
 * @param f The file to map.
 * @param rule The rule to apply to the map.
 * @param offset The offset inside the file
 * @param length The length of the memory to map
 * @param filter_cb Callback called in the thread to validate the content of the map.
 * @param map_cb Callback called in the main loop when the file has been successfully mapped.
 * @param error_cb Callback called in the main loop when the file can't be mapped.
 * @param data Unmodified user data passed to callbacks
 * @return Pointer to the file if successfull or NULL otherwise.
 *
 * The container of the Eio_File is the Eina_File.
 */
EAPI Eio_File *eio_file_map_new(Eina_File *f,
                                Eina_File_Populate rule,
                                unsigned long int offset,
                                unsigned long int length,
                                Eio_Filter_Map_Cb filter_cb,
                                Eio_Map_Cb map_cb,
                                Eio_Error_Cb error_cb,
                                const void *data);

/**
 * @}
 */

/**
 * @defgroup Eio_Eet Eio asynchronous API for Eet file.
 * @ingroup Eio
 *
 * @brief This set of functions help in the asynchronous use of Eet
 *
 * @{
 */

/**
 * @brief Open an eet file on disk, and returns a handle to it asynchronously.
 * @param filename The file path to the eet file. eg: @c "/tmp/file.eet".
 * @param mode The mode for opening. Either EET_FILE_MODE_READ,
 *        EET_FILE_MODE_WRITE or EET_FILE_MODE_READ_WRITE.
 * @param eet_cb The callback to call when the file has been successfully opened.
 * @param error_cb Callback called in the main loop when the file can't be opened.
 * @param data Unmodified user data passed to callbacks
 * @return NULL in case of a failure.
 *
 * This function calls eet_open() from another thread using Ecore_Thread.
 */
EAPI Eio_File *eio_eet_open(const char *filename,
                            Eet_File_Mode mode,
			    Eio_Eet_Open_Cb eet_cb,
			    Eio_Error_Cb error_cb,
			    const void *data);
/**
 * @brief Close an eet file handle and flush pending writes asynchronously.
 * @param ef A valid eet file handle.
 * @param done_cb Callback called from the main loop when the file has been closed.
 * @param error_cb Callback called in the main loop when the file can't be closed.
 * @param data Unmodified user data passed to callbacks
 * @return NULL in case of a failure.
 *
 * This function will call eet_close() from another thread by
 * using Ecore_Thread. You should assume that the Eet_File is dead after this
 * function is called.
 */
EAPI Eio_File *eio_eet_close(Eet_File *ef,
			     Eio_Done_Cb done_cb,
			     Eio_Eet_Error_Cb error_cb,
			     const void *data);

/**
 * @brief Sync content of an eet file handle, flushing pending writes asynchronously.
 * @param ef A valid eet file handle.
 * @param done_cb Callback called from the main loop when the file has been synced.
 * @param error_cb Callback called in the main loop when the file can't be synced.
 * @param data Unmodified user data passed to callbacks
 * @return NULL in case of a failure.
 *
 * This function will call eet_sync() from another thread. As long as the done_cb or
 * error_cb haven't be called, you must keep @p ef open.
 */
EAPI Eio_File *eio_eet_sync(Eet_File *ef,
                            Eio_Done_Cb done_cb,
                            Eio_Eet_Error_Cb error_cb,
                            const void *data);

/**
 * @brief Write a data structure from memory and store in an eet file
 * using a cipher asynchronously.
 * @param ef The eet file handle to write to.
 * @param edd The data descriptor to use when encoding.
 * @param name The key to store the data under in the eet file.
 * @param cipher_key The key to use as cipher.
 * @param write_data A pointer to the data structure to save and encode.
 * @param compress Compression flags for storage.
 * @param done_cb Callback called from the main loop when the data has been put in the Eet_File.
 * @param error_cb Callback called in the main loop when the file can't be written.
 * @param user_data Private data given to each callback.
 * @return NULL in case of a failure.
 */
EAPI Eio_File *eio_eet_data_write_cipher(Eet_File *ef,
					 Eet_Data_Descriptor *edd,
					 const char *name,
					 const char *cipher_key,
					 void *write_data,
					 int compress,
					 Eio_Done_Int_Cb done_cb,
					 Eio_Error_Cb error_cb,
					 const void *user_data);

/**
 * @brief Read a data structure from an eet file and decodes it using a cipher asynchronously.
 * @param ef The eet file handle to read from.
 * @param edd The data descriptor handle to use when decoding.
 * @param name The key the data is stored under in the eet file.
 * @param cipher_key The key to use as cipher.
 * @param done_cb Callback called from the main loop when the data has been read and decoded.
 * @param error_cb Callback called in the main loop when the data can't be read.
 * @param data Unmodified user data passed to callbacks
 * @return NULL in case of a failure.
 */
EAPI Eio_File *eio_eet_data_read_cipher(Eet_File *ef,
                                        Eet_Data_Descriptor *edd,
                                        const char *name,
                                        const char *cipher_key,
                                        Eio_Done_ERead_Cb done_cb,
                                        Eio_Error_Cb error_cb,
                                        const void *data);

/**
 * @brief Write image data to the named key in an eet file asynchronously.
 * @param ef A valid eet file handle opened for writing.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param cipher_key The key to use as cipher.
 * @param write_data A pointer to the image pixel data.
 * @param w The width of the image in pixels.
 * @param h The height of the image in pixels.
 * @param alpha The alpha channel flag.
 * @param compress The compression amount.
 * @param quality The quality encoding amount.
 * @param lossy The lossiness flag.
 * @param done_cb Callback called from the main loop when the data has been put in the Eet_File.
 * @param error_cb Callback called in the main loop when the file can't be written.
 * @param user_data Private data given to each callback.
 * @return NULL in case of a failure.
 */
EAPI Eio_File *eio_eet_data_image_write_cipher(Eet_File *ef,
                                               const char *name,
                                               const char *cipher_key,
                                               void *write_data,
                                               unsigned int w,
                                               unsigned int h,
                                               int alpha,
                                               int compress,
                                               int quality,
                                               int lossy,
                                               Eio_Done_Int_Cb done_cb,
                                               Eio_Error_Cb error_cb,
                                               const void *user_data);

/**
 * @brief Read a specified entry from an eet file and return data
 * @param ef A valid eet file handle opened for reading.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param done_cb Callback called from the main loop when the data has been read.
 * @param error_cb Callback called in the main loop when the data can't be read.
 * @param data Unmodified user data passed to callbacks
 * @return NULL in case of a failure.
 */
EAPI Eio_File *eio_eet_read_direct(Eet_File *ef,
                                   const char *name,
                                   Eio_Done_Data_Cb done_cb,
                                   Eio_Error_Cb error_cb,
                                   const void *data);

/**
 * @brief Read a specified entry from an eet file and return data
 * @param ef A valid eet file handle opened for reading.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param cipher_key The key to use as cipher.
 * @param done_cb Callback called from the main loop when the data has been read.
 * @param error_cb Callback called in the main loop when the data can't be read.
 * @param data Unmodified user data passed to callbacks
 * @return NULL in case of a failure.
 */
EAPI Eio_File *eio_eet_read_cipher(Eet_File *ef,
                                   const char *name,
                                   const char *cipher_key,
                                   Eio_Done_Read_Cb done_cb,
                                   Eio_Error_Cb error_cb,
                                   const void *data);

/**
 * @brief Write a specified entry to an eet file handle using a cipher.
 * @param ef A valid eet file handle opened for writing.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param write_data Pointer to the data to be stored.
 * @param size Length in bytes in the data to be stored.
 * @param compress Compression flags (1 == compress, 0 = don't compress).
 * @param cipher_key The key to use as cipher.
 * @param done_cb Callback called from the main loop when the data has been put in the Eet_File.
 * @param error_cb Callback called in the main loop when the file can't be written.
 * @param user_data Private data given to each callback.
 * @return NULL in case of a failure.
 */
EAPI Eio_File *eio_eet_write_cipher(Eet_File *ef,
                                    const char *name,
                                    void *write_data,
                                    int size,
                                    int compress,
                                    const char *cipher_key,
                                    Eio_Done_Int_Cb done_cb,
                                    Eio_Error_Cb error_cb,
                                    const void *user_data);

/**
 * @}
 */

/**
 * @defgroup Eio_Monitor Eio file and directory monitoring API
 * @ingroup Eio
 *
 * @brief These function monitor changes in directories and files
 *
 * These functions use the best available method to monitor changes on a specified directory
 * or file. They send ecore events when changes occur, and they maintain internal refcounts to
 * reduce resource consumption on duplicate monitor targets.
 *
 * @{
 */

EAPI extern int EIO_MONITOR_FILE_CREATED; /**< A new file was created in a watched directory */
EAPI extern int EIO_MONITOR_FILE_DELETED; /**< A watched file was deleted, or a file in a watched directory was deleted */
EAPI extern int EIO_MONITOR_FILE_MODIFIED; /**< A file was modified in a watched directory */
EAPI extern int EIO_MONITOR_FILE_CLOSED; /**< A file was closed in a watched directory. This event is never sent on Windows */
EAPI extern int EIO_MONITOR_DIRECTORY_CREATED; /**< A new directory was created in a watched directory */
EAPI extern int EIO_MONITOR_DIRECTORY_DELETED; /**< A directory has been deleted: this can be either a watched directory or one of its subdirectories */
EAPI extern int EIO_MONITOR_DIRECTORY_MODIFIED; /**< A directory has been modified in a watched directory */
EAPI extern int EIO_MONITOR_DIRECTORY_CLOSED; /**< A directory has been closed in a watched directory. This event is never sent on Windows */
EAPI extern int EIO_MONITOR_SELF_RENAME; /**< The monitored path has been renamed, an error could happen just after if the renamed path doesn't exist */
EAPI extern int EIO_MONITOR_SELF_DELETED; /**< The monitored path has been removed */
EAPI extern int EIO_MONITOR_ERROR; /**< During operation the monitor failed and will no longer work. eio_monitor_del must be called on it. */

typedef struct _Eio_Monitor Eio_Monitor;

typedef struct _Eio_Monitor_Error Eio_Monitor_Error;
typedef struct _Eio_Monitor_Event Eio_Monitor_Event;

struct _Eio_Monitor_Error
{
   Eio_Monitor *monitor;
   int error;
};

struct _Eio_Monitor_Event
{
   Eio_Monitor *monitor;
   const char *filename;
};

/**
 * @brief Adds a file/directory to monitor (inotify mechanism)
 * @param path file/directory to monitor
 * @return NULL in case of a failure or a pointer to the monitor in case of
 * success.
 *
 * This function will add the given path to its internal
 * list of files to monitor. It utilizes the inotify mechanism
 * introduced in kernel 2.6.13 for passive monitoring.
 */
EAPI Eio_Monitor *eio_monitor_add(const char *path);

/**
 * @brief Adds a file/directory to monitor
 * @param path file/directory to monitor
 * @return NULL in case of a failure or a pointer to the monitor in case of
 * success.
 * @warning Do NOT pass non-stringshared strings to this function!
 *  If you don't know what this means, use eio_monitor_add().
 *
 * This fuction is just like eio_monitor_add(), however the string passed by
 * argument must be created using eina_stringshare_add().
 */
EAPI Eio_Monitor *eio_monitor_stringshared_add(const char *path);

/**
 * @brief Deletes a path from the “watched” list
 * @param monitor The Eio_Monitor you want to stop watching.
 *  It can only be an Eio_Monitor returned to you from calling
 *  eio_monitor_add() or eio_monitor_stringshared_add()
 */
EAPI void eio_monitor_del(Eio_Monitor *monitor);

/**
 * @brief returns the path being watched by the given
 *  Eio_Monitor.
 * @param monitor Eio_Monitor to return the path of
 * @return The stringshared path belonging to @p monitor
 */
EAPI const char *eio_monitor_path_get(Eio_Monitor *monitor);

/**
 * @}
 */

#include "eio_inline_helper.x"

#ifdef __cplusplus
}
#endif


#endif

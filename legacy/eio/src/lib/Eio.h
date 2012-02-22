/* EIO - EFL data type library
 * Copyright (C) 2010 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
 *           Vincent "caro" Torri  <vtorri at univ-evry dot fr>
 *	     Stephen "okra" Houston <unixtitan@gmail.com>
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

#ifdef _MSC_VER
# include <Evil.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <Eina.h>
#include <Eet.h>

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

/**
 * @defgroup Eio_Group Eio Reference API
 *
 * @brief This are the core asynchronous input/output operation
 *
 * All the function in this group do perform input/output operation
 * in a separated thread and use the infrastructure provided by
 * Ecore_Thread and Eina to work.
 *
 * @{
 */

/**
 * @enum _Eio_File_Op
 * Input/Output operations on files.
 */
enum _Eio_File_Op
{
  EIO_FILE_COPY, /**< IO operation is about a specific file copy */
  EIO_FILE_MOVE, /**< IO operation is about a specific file move */
  EIO_DIR_COPY, /**< IO operation is about a specific directory copy */
  EIO_DIR_MOVE, /**< IO operation is about a specific directory move */
  EIO_UNLINK, /**< IO operation is about a destroying a path (source will point to base path to be destroyed and dest to path destroyed by this IO */
  EIO_FILE_GETPWNAM, /**< IO operation is trying to get uid from user name */
  EIO_FILE_GETGRNAM /**< IO operation is trying to get gid from user name */
};

/**
 * @typedef Eio_File_Op
 * Input/Output operations on files.
 */
typedef enum _Eio_File_Op Eio_File_Op;

/**
 * @typedef Eio_File
 * Generic asynchronous IO reference.
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

typedef void (*Eio_Stat_Cb)(void *data, Eio_File *handler, const struct stat *stat);
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

struct _Eio_Progress
{
   Eio_File_Op op; /**< IO type */

   long long current; /**< Current step in the IO operation */
   long long max; /**< Number of step to do to complete this IO */
   float percent; /**< Percent done of the IO operation */

   const char *source; /**< source of the IO operation */
   const char *dest; /**< target of the IO operation */
};

/**
 * @brief List content of a directory without locking your app.
 * @param dir The directory to list.
 * @param filter_cb Callback called from another thread.
 * @param main_cb Callback called from the main loop for each accepted file.
 * @param done_cb Callback called from the main loop when the content of the directory has been listed.
 * @param error_cb Callback called from the main loop when the directory could not be opened or listing content has been canceled.
 * @param data Data passed to callback and not modified at all by eio_file_ls.
 * @return A reference to the IO operation.
 *
 * eio_file_ls run eina_file_ls in a separated thread using ecore_thread_feedback_run. This prevent
 * any lock in your apps.
 */
EAPI Eio_File *eio_file_ls(const char *dir,
			   Eio_Filter_Cb filter_cb,
			   Eio_Main_Cb main_cb,
			   Eio_Done_Cb done_cb,
			   Eio_Error_Cb error_cb,
			   const void *data);

/**
 * @brief List content of a directory without locking your app.
 * @param dir The directory to list.
 * @param filter_cb Callback called from another thread.
 * @param main_cb Callback called from the main loop for each accepted file.
 * @param done_cb Callback called from the main loop when the content of the directory has been listed.
 * @param error_cb Callback called from the main loop when the directory could not be opened or listing content has been canceled.
 * @param data Data passed to callback and not modified at all by eio_file_direct_ls.
 * @return A reference to the IO operation.
 *
 * eio_file_direct_ls run eina_file_direct_ls in a separated thread using
 * ecore_thread_feedback_run. This prevent any lock in your apps.
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
 * @param filter_cb Callback called from another thread.
 * @param main_cb Callback called from the main loop for each accepted file.
 * @param done_cb Callback called from the main loop when the content of the directory has been listed.
 * @param error_cb Callback called from the main loop when the directory could not be opened or listing content has been canceled.
 * @param data Data passed to callback and not modified at all by eio_file_stat_ls.
 * @return A reference to the IO operation.
 *
 * eio_file_stat_ls() run eina_file_stat_ls() in a separated thread using
 * ecore_thread_feedback_run. This prevent any lock in your apps.
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
 * @param filter_cb Callback called from another thread.
 * @param main_cb Callback called from the main loop for each accepted file.
 * @param done_cb Callback called from the main loop when the content of the directory has been listed.
 * @param error_cb Callback called from the main loop when the directory could not be opened or listing content has been canceled.
 * @param data Data passed to callback and not modified at all by eio_dir_stat_find.
 * @return A reference to the IO operation.
 *
 * eio_dir_stat_ls() run eina_file_stat_ls() recursivly in a separated thread using
 * ecore_thread_feedback_run. This prevent any lock in your apps.
 */
EAPI Eio_File *eio_dir_stat_ls(const char *dir,
                               Eio_Filter_Dir_Cb filter_cb,
                               Eio_Main_Direct_Cb main_cb,
                               Eio_Done_Cb done_cb,
                               Eio_Error_Cb error_cb,
                               const void *data);

/**
 * @brief List the content of a directory and all it's sub-content asynchronously
 * @param dir The directory to list.
 * @param filter_cb Callback called from another thread.
 * @param main_cb Callback called from the main loop for each accepted file.
 * @param done_cb Callback called from the main loop when the content of the directory has been listed.
 * @param error_cb Callback called from the main loop when the directory could not be opened or listing content has been canceled.
 * @param data Data passed to callback and not modified at all by eio_dir_stat_find.
 * @return A reference to the IO operation.
 *
 * eio_dir_direct_ls() run eina_file_direct_ls() recursivly in a separated thread using
 * ecore_thread_feedback_run. This prevent any lock in your apps.
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
 * @param done_cb Callback called from the main loop when stat was successfully called..
 * @param error_cb Callback called from the main loop when stat failed or has been canceled.
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_direct_stat basically call stat in another thread. This prevent any lock in your apps.
 */
EAPI Eio_File *eio_file_direct_stat(const char *path,
				    Eio_Stat_Cb done_cb,
				    Eio_Error_Cb error_cb,
				    const void *data);

/**
 * @brief Change right of a path.
 * @param path The directory path to change access right.
 * @param mode The permission to set, follow (mode & ~umask & 0777).
 * @param done_cb Callback called from the main loop when the directory has been created.
 * @param error_cb Callback called from the main loop when the directory failed to be created or has been canceled.
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_chmod basically call chmod in another thread. This prevent any lock in your apps.
 */
EAPI Eio_File *eio_file_chmod(const char *path,
                              mode_t mode,
                              Eio_Done_Cb done_cb,
                              Eio_Error_Cb error_cb,
                              const void *data);

/**
 * @brief Change owner of a path.
 * @param path The directory path to change owner.
 * @param user The new user to set (could be NULL).
 * @param group The new group to set (could be NULL).
 * @param done_cb Callback called from the main loop when the directory has been created.
 * @param error_cb Callback called from the main loop when the directory failed to be created or has been canceled.
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_chown determine the uid/gid that correspond to both user and group string and then call chown. This prevent any lock in your apps by calling
 * this function from another thread. The string could be the name of the user or the name of the group or directly their numerical value.
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
 * @param done_cb Callback called from the main loop when the directory has been created.
 * @param error_cb Callback called from the main loop when the directory failed to be created or has been canceled.
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_unlink basically call unlink in another thread. This prevent any lock in your apps.
 */
EAPI Eio_File *eio_file_unlink(const char *path,
			       Eio_Done_Cb done_cb,
			       Eio_Error_Cb error_cb,
			       const void *data);

/**
 * @brief Create a new directory.
 * @param path The directory path to create.
 * @param mode The permission to set, follow (mode & ~umask & 0777).
 * @param done_cb Callback called from the main loop when the directory has been created.
 * @param error_cb Callback called from the main loop when the directory failed to be created or has been canceled.
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_mkdir basically call mkdir in another thread. This prevent any lock in your apps.
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
 * @param data Private data given to callback.
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
 * @param data Private data given to callback.
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
 * @param data Private data given to callback.
 *
 * This function will move a directory and all it's content from source to dest.
 * It will try first to rename the directory, if not it will try to use splice
 * if possible, if not it will fallback to mmap/write.
 * It will try to preserve access right, but not user/group identity.
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
 * @param data Private data given to callback.
 *
 * This function will copy a directory and all it's content from source to dest.
 * It will try to use splice if possible, if not it will fallback to mmap/write.
 * It will try to preserve access right, but not user/group identity.
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
 * @param data Private data given to callback.
 *
 * This function will remove a directory and all it's content.
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
 *
 * @brief A set of function to manipulate data associated with a specific file
 *
 * @{
 */

/**
 * @brief Assynchronously list all eXtended attribute
 * @param path The path to get the eXtended attribute from.
 * @param filter_cb Callback called in the thread to validate the eXtended attribute.
 * @param main_cb Callback called in the main loop for each accepted eXtended attribute.
 * @param done_cb Callback called in the main loop when the all the eXtended attribute have been listed.
 * @param error_cb Callback called in the main loop when something goes wrong during the listing of the eXtended attribute.
 * @param data Data passed to callback and not modified at all by eio_file_ls.
 * @return A reference to the IO operation.
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
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_int_set call eina_xattr_int_set from another thread. This prevent lock in your apps. If
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
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_double_set call eina_xattr_double_set from another thread. This prevent lock in your apps. If
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
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_string_set call eina_xattr_string_set from another thread. This prevent lock in your apps. If
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
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_set call setxattr from another thread. This prevent lock in your apps. If
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
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_get call getxattr from another thread. This prevent lock in your apps.
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
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_int_get call eina_xattr_int_get from another thread. This prevent lock in your apps.
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
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_double_get call eina_xattr_double_get from another thread. This prevent lock in your apps.
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
 * @param data Private data given to callback.
 * @return A reference to the IO operation.
 *
 * eio_file_xattr_string_get call eina_xattr_string_get from another thread. This prevent lock in your apps.
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
 * @param ls The asynchronous IO operation to retrieve container from.
 * @return NULL if not available, a DIRP if it is.
 *
 * This is only available and make sense in the thread callback, not in
 * the mainloop.
 */
EAPI void *eio_file_container_get(Eio_File *ls);

/**
 * @brief Cancel any Eio_File.
 * @param ls The asynchronous IO operation to cancel.
 * @return EINA_FALSE if the destruction is delayed, EINA_TRUE if it's done.
 *
 * This will cancel any kind of IO operation and cleanup the mess. This means
 * that it could take time to cancel an IO.
 */
EAPI Eina_Bool eio_file_cancel(Eio_File *ls);

/**
 * @brief Check if an Eio_File operation has been cancelled.
 * @param ls The asynchronous IO operation to check.
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
 * @param free_cb The function to call to free the associated data, @p free will be called if not specified.
 * @return EINA_TRUE if insertion was fine.
 *
 * This function could only be safely called from within the filter callback.
 * If you don't need to copy the key around you can use @ref eio_file_associate_direct_add
 */
EAPI Eina_Bool eio_file_associate_add(Eio_File *ls,
				      const char *key,
				      const void *data, Eina_Free_Cb free_cb);

/**
 * @brief Associate data with the current filtered file.
 * @param ls The Eio_File ls request currently calling the filter callback.
 * @param key The key to associate data to (will not be copied, and the pointer will be used as long as the file is not notified).
 * @param data The data to associate the data to.
 * @param free_cb The function to call to free the associated data, @p free will be called if not specified.
 * @return EINA_TRUE if insertion was fine.
 *
 * This function could only be safely called from within the filter callback.
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
 * @brief get access time from a struct stat
 * @param stat the structure to get the atime from
 * @return the last accessed time
 *
 * This take care of doing type conversion to match rest of EFL time API.
 * @note some filesystem don't update that information.
 */
static inline double eio_file_atime(const struct stat *stat);

/**
 * @brief get modification time from a struct stat
 * @param stat the structure to get the mtime from
 * @return the last modification time
 *
 * This take care of doing type conversion to match rest of EFL time API.
 */
static inline double eio_file_mtime(const struct stat *stat);

/**
 * @brief get the size of the file described in struct stat
 * @param stat the structure to get the size from
 * @return the size of the file
 */
static inline long long eio_file_size(const struct stat *stat);

/**
 * @brief tell if the stated path was a directory or not.
 * @param stat the structure to get the size from
 * @return EINA_TRUE, if it was.
 */
static inline Eina_Bool eio_file_is_dir(const struct stat *stat);

/**
 * @brief tell if the stated path was a link or not.
 * @param stat the structure to get the size from
 * @return EINA_TRUE, if it was.
 */
static inline Eina_Bool eio_file_is_lnk(const struct stat *stat);

/**
 * @}
 */

/**
 *
 */

/**
 * @defgroup Eio_Map Manipulate an Eina_File assynchronously
 *
 * @brief This function help manipulating file assynchronously.
 *
 * This set of function work on top of Eina_File and Ecore_Thread to
 * never block when reading the content of a file.
 * @{
 */

/**
 * @brief Assynchronously open a file.
 * @param name The file to open.
 * @param shared If it's an shm file.
 * @param open_cb Callback called in the main loop when the file has been successfully opened.
 * @param error_cb Callback called in the main loop when the file couldn't be opened.
 * @param data Private data given to each callback.
 * @return NULL in case of a failure.
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
 * @param data Private data given to each callback.
 * @return NULL in case of a failure.
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
 * @param data Private data given to each callback.
 * @return NULL in case of a failure.
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
 * @param data Private data given to each callback.
 * @return NULL in case of a failure.
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
 *
 * @brief This set of functions help use Eet asynchronously
 *
 * @{
 */

/**
 * Open an eet file on disk, and returns a handle to it assynchronously.
 * @param filename The file path to the eet file. eg: @c "/tmp/file.eet".
 * @param mode The mode for opening. Either EET_FILE_MODE_READ,
 *        EET_FILE_MODE_WRITE or EET_FILE_MODE_READ_WRITE.
 * @param eet_cb The callback to call when the file has been successfully opened.
 * @param error_cb Callback called in the main loop when the file can't be opened.
 * @param data Private data given to each callback.
 * @return NULL in case of a failure.
 *
 * This function call eet_open() from another thread using Ecore_Thread.
 */
EAPI Eio_File *eio_eet_open(const char *filename,
                            Eet_File_Mode mode,
			    Eio_Eet_Open_Cb eet_cb,
			    Eio_Error_Cb error_cb,
			    const void *data);
/**
 * Close an eet file handle and flush pending writes assynchronously.
 * @param ef A valid eet file handle.
 * @param done_cb Callback called from the main loop when the file has been closed.
 * @param error_cb Callback called in the main loop when the file can't be closed.
 * @param data Private data given to each callback.
 * @return NULL in case of a failure.
 *
 * This function will call eet_close() from another thread by
 * using Ecore_Thread. You should assume that the Eet_File is dead after this
 * function call.
 */
EAPI Eio_File *eio_eet_close(Eet_File *ef,
			     Eio_Done_Cb done_cb,
			     Eio_Eet_Error_Cb error_cb,
			     const void *data);

/**
 * Sync content of an eet file handle, flushing pending writes assynchronously.
 * @param ef A valid eet file handle.
 * @param done_cb Callback called from the main loop when the file has been synced.
 * @param error_cb Callback called in the main loop when the file can't be synced.
 * @param data Private data given to each callback.
 * @return NULL in case of a failure.
 *
 * This function will call eet_flush() from another thread. As long as done_cb or
 * error_cb, haven't be called, you must keep ef open.
 */
EAPI Eio_File *eio_eet_sync(Eet_File *ef,
                            Eio_Done_Cb done_cb,
                            Eio_Eet_Error_Cb error_cb,
                            const void *data);

/**
 * Write a data structure from memory and store in an eet file
 * using a cipher assynchronously.
 * @param ef The eet file handle to write to.
 * @param edd The data descriptor to use when encoding.
 * @param name The key to store the data under in the eet file.
 * @param cipher_key The key to use as cipher.
 * @param write_data A pointer to the data structure to ssave and encode.
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
 * Read a data structure from an eet file and decodes it using a cipher assynchronously.
 * @param ef The eet file handle to read from.
 * @param edd The data descriptor handle to use when decoding.
 * @param name The key the data is stored under in the eet file.
 * @param cipher_key The key to use as cipher.
 * @param done_cb Callback called from the main loop when the data has been read and decoded.
 * @param error_cb Callback called in the main loop when the data can't be read.
 * @param data Private data given to each callback.
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
 * Write image data to the named key in an eet file assynchronously.
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
 * Read a specified entry from an eet file and return data
 * @param ef A valid eet file handle opened for reading.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param done_cb Callback called from the main loop when the data has been read.
 * @param error_cb Callback called in the main loop when the data can't be read.
 * @param data Private data given to each callback.
 * @return NULL in case of a failure.
 */
EAPI Eio_File *eio_eet_read_direct(Eet_File *ef,
                                   const char *name,
                                   Eio_Done_Data_Cb done_cb,
                                   Eio_Error_Cb error_cb,
                                   const void *data);

/**
 * Read a specified entry from an eet file and return data
 * @param ef A valid eet file handle opened for reading.
 * @param name Name of the entry. eg: "/base/file_i_want".
 * @param cipher_key The key to use as cipher.
 * @param done_cb Callback called from the main loop when the data has been read.
 * @param error_cb Callback called in the main loop when the data can't be read.
 * @param data Private data given to each callback.
 * @return NULL in case of a failure.
 */
EAPI Eio_File *eio_eet_read_cipher(Eet_File *ef,
                                   const char *name,
                                   const char *cipher_key,
                                   Eio_Done_Read_Cb done_cb,
                                   Eio_Error_Cb error_cb,
                                   const void *data);

/**
 * Write a specified entry to an eet file handle using a cipher.
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
 *
 * @brief This function help monitoring change in a directory or on a file.
 *
 * This function use the best available method to monitor change on a directory
 * or file. It send ecore event when needed and does refcount of all monitored
 * path to avoid heavy ressource consuption.
 *
 * @{
 */

EAPI extern int EIO_MONITOR_FILE_CREATED; /**< Notify creation of a new file in a watched directory */
EAPI extern int EIO_MONITOR_FILE_DELETED; /**< Notify destruction of a watched file or in a watched directory */
EAPI extern int EIO_MONITOR_FILE_MODIFIED; /**< Notify modification of a file in a watched directory */
EAPI extern int EIO_MONITOR_FILE_CLOSED; /**< Notify closing of a file in a watched directory */
EAPI extern int EIO_MONITOR_DIRECTORY_CREATED; /**< Notify creation of a new directory in a watched directory */
EAPI extern int EIO_MONITOR_DIRECTORY_DELETED; /**< Notify destruction of a watched directory or in a watched directory */
EAPI extern int EIO_MONITOR_DIRECTORY_MODIFIED; /**< Notify modification of a directory in a watched directory */
EAPI extern int EIO_MONITOR_DIRECTORY_CLOSED; /**< Notify closing of a directory in a watched directory */
EAPI extern int EIO_MONITOR_SELF_RENAME; /**< Notify that Eio monitored path has been renamed, an error could happen just after if the renamed path doesn't exist */
EAPI extern int EIO_MONITOR_SELF_DELETED; /**< Notify that Eio monitored path has been removed */
EAPI extern int EIO_MONITOR_ERROR; /**< Notify that during operation the pointed monitor failed and will no longer work. eio_monitor_del is required on it. */

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
 * @return NULL in case of a failure.
 *
 * This function will add the given path to its internal
 * list of files to monitor. Its using inotify mechanism
 * introduced in kernel 2.6.13 to get non active monitoring.
 */
EAPI Eio_Monitor *eio_monitor_add(const char *path);

/**
 * @param path file/directory to monitor
 * @return NULL in case of a failure.
 * Does the same thing than eio_monitor_add() but
 * expects an eina_stringshared buffer.
 * @warning dont give anything else than a stringshared buffer!
 *  if you dont know what you do, use eio_monitor_add().
 */
EAPI Eio_Monitor *eio_monitor_stringshared_add(const char *path);

/**
 * @brief Deletes a path from the “watched” list
 * @param monitor Eio_Monitor you want to stop watching.
 *  it can only be an Eio_Monitor given to you when calling
 *  eio_monitor_add() or eio_monitor_stringshared_add()
 */
EAPI void eio_monitor_del(Eio_Monitor *monitor);

/**
 * @brief returns the path being watched by the given
 *  Eio_Monitor.
 * @param monitor Eio_Monitor that you want to get path
 * @return returns a stringshared buffer, do not free it!
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

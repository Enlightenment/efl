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
typedef void (*Eio_Main_Cb)(void *data, Eio_File *handler, const char *file);

typedef Eina_Bool (*Eio_Filter_Direct_Cb)(void *data, Eio_File *handler, const Eina_File_Direct_Info *info);
typedef void (*Eio_Main_Direct_Cb)(void *data, Eio_File *handler, const Eina_File_Direct_Info *info);

typedef void (*Eio_Stat_Cb)(void *data, Eio_File *handler, const struct stat *stat);
typedef void (*Eio_Progress_Cb)(void *data, Eio_File *handler, const Eio_Progress *info);

typedef void (*Eio_Done_Cb)(void *data, Eio_File *handler);
typedef void (*Eio_Error_Cb)(void *data, Eio_File *handler, int error);

struct _Eio_Progress
{
   Eio_File_Op op; /**< IO type */

   long long current; /**< Current step in the IO operation */
   long long max; /**< Number of step to do to complete this IO */
   float percent; /**< Percent done of the IO operation */

   const char *source; /**< source of the IO operation */
   const char *dest; /**< target of the IO operation */
};

EAPI int eio_init(void);
EAPI int eio_shutdown(void);

EAPI Eio_File *eio_file_ls(const char *dir,
			   Eio_Filter_Cb filter_cb,
			   Eio_Main_Cb main_cb,
			   Eio_Done_Cb done_cb,
			   Eio_Error_Cb error_cb,
			   const void *data);

EAPI Eio_File *eio_file_direct_ls(const char *dir,
				  Eio_Filter_Direct_Cb filter_cb,
				  Eio_Main_Direct_Cb main_cb,
				  Eio_Done_Cb done_cb,
				  Eio_Error_Cb error_cb,
				  const void *data);

EAPI Eio_File *eio_file_stat_ls(const char *dir,
                                Eio_Filter_Direct_Cb filter_cb,
                                Eio_Main_Direct_Cb main_cb,
                                Eio_Done_Cb done_cb,
                                Eio_Error_Cb error_cb,
                                const void *data);

EAPI Eio_File *eio_dir_stat_ls(const char *dir,
                               Eio_Filter_Direct_Cb filter_cb,
                               Eio_Main_Direct_Cb main_cb,
                               Eio_Done_Cb done_cb,
                               Eio_Error_Cb error_cb,
                               const void *data);

EAPI Eio_File *eio_file_direct_stat(const char *path,
				    Eio_Stat_Cb done_cb,
				    Eio_Error_Cb error_cb,
				    const void *data);

EAPI Eio_File *eio_file_chmod(const char *path,
                              mode_t mode,
                              Eio_Done_Cb done_cb,
                              Eio_Error_Cb error_cb,
                              const void *data);

EAPI Eio_File *eio_file_chown(const char *path,
                              const char *user,
                              const char *group,
                              Eio_Done_Cb done_cb,
                              Eio_Error_Cb error_cb,
                              const void *data);

EAPI Eio_File *eio_file_unlink(const char *path,
			       Eio_Done_Cb done_cb,
			       Eio_Error_Cb error_cb,
			       const void *data);

EAPI Eio_File *eio_file_mkdir(const char *path,
			      mode_t mode,
			      Eio_Done_Cb done_cb,
			      Eio_Error_Cb error_cb,
			      const void *data);

EAPI Eio_File *eio_file_move(const char *source,
			     const char *dest,
			     Eio_Progress_Cb progress_cb,
			     Eio_Done_Cb done_cb,
			     Eio_Error_Cb error_cb,
			     const void *data);

EAPI Eio_File *eio_file_copy(const char *source,
			     const char *dest,
			     Eio_Progress_Cb progress_cb,
			     Eio_Done_Cb done_cb,
			     Eio_Error_Cb error_cb,
			     const void *data);

EAPI Eio_File *eio_dir_move(const char *source,
			    const char *dest,
                            Eio_Filter_Direct_Cb filter_cb,
			    Eio_Progress_Cb progress_cb,
			    Eio_Done_Cb done_cb,
			    Eio_Error_Cb error_cb,
			    const void *data);

EAPI Eio_File *eio_dir_copy(const char *source,
			    const char *dest,
                            Eio_Filter_Direct_Cb filter_cb,
			    Eio_Progress_Cb progress_cb,
			    Eio_Done_Cb done_cb,
			    Eio_Error_Cb error_cb,
			    const void *data);

EAPI Eio_File *eio_dir_unlink(const char *path,
                              Eio_Filter_Direct_Cb filter_cb,
			      Eio_Progress_Cb progress_cb,
			      Eio_Done_Cb done_cb,
			      Eio_Error_Cb error_cb,
			      const void *data);

EAPI void *eio_file_container_get(Eio_File *ls);

EAPI Eina_Bool eio_file_cancel(Eio_File *ls);

EAPI Eina_Bool eio_file_associate_add(Eio_File *ls,
				      const char *key,
				      void *data, Eina_Free_Cb free_cb);
EAPI Eina_Bool eio_file_associate_direct_add(Eio_File *ls,
					     const char *key,
					     void *data, Eina_Free_Cb free_cb);
EAPI void *eio_file_associate_find(Eio_File *ls, const char *key);

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

static inline double eio_file_atime(const struct stat *stat);
static inline double eio_file_mtime(const struct stat *stat);
static inline long long eio_file_size(const struct stat *stat);
static inline Eina_Bool eio_file_is_dir(const struct stat *stat);
static inline Eina_Bool eio_file_is_lnk(const struct stat *stat);

/**
 * @}
 */

/**
 *
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

EAPI Eio_Monitor *eio_monitor_add(const char *path);
EAPI Eio_Monitor *eio_monitor_stringshared_add(const char *path);
EAPI void eio_monitor_del(Eio_Monitor *monitor);
EAPI const char *eio_monitor_path_get(Eio_Monitor *monitor);

/**
 * @}
 */

#include "eio_inline_helper.x"

#ifdef __cplusplus
}
#endif


#endif

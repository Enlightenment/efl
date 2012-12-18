#ifndef ECORE_FILE_H
#define ECORE_FILE_H

/*
 * TODO:
 * - More events, move/rename of directory file
 */

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_FILE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_FILE_BUILD */
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

/**
 * @file Ecore_File.h
 * @brief Files utility functions
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Ecore_File_Group Ecore_File - Files and directories convenience functions
 * @ingroup Ecore
 *
 * @{
 */

/**
 * @typedef Ecore_File_Monitor
 * Abstract type used when monitoring a directory.
 */
typedef struct _Ecore_File_Monitor       Ecore_File_Monitor;

/**
 * @typedef Ecore_File_Download_Job
 * Abstract type used when aborting a download.
 */
typedef struct _Ecore_File_Download_Job  Ecore_File_Download_Job;

/**
 * @typedef _Ecore_File_Event
 * The event type returned when a file or directory is monitored.
 */
typedef enum _Ecore_File_Event
{
   ECORE_FILE_EVENT_NONE,              /**< No event. */
   ECORE_FILE_EVENT_CREATED_FILE,      /**< Created file event. */
   ECORE_FILE_EVENT_CREATED_DIRECTORY, /**< Created directory event. */
   ECORE_FILE_EVENT_DELETED_FILE,      /**< Deleted file event. */
   ECORE_FILE_EVENT_DELETED_DIRECTORY, /**< Deleted directory event. */
   ECORE_FILE_EVENT_DELETED_SELF,      /**< Deleted monitored directory event. */
   ECORE_FILE_EVENT_MODIFIED,          /**< Modified file or directory event. */
   ECORE_FILE_EVENT_CLOSED             /**< Closed file event */
} Ecore_File_Event;

/**
 * @typedef Ecore_File_Monitor_Cb
 * Callback type used when a monitored directory has changes.
 */
typedef void (*Ecore_File_Monitor_Cb)(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path);

/**
 * @typedef Ecore_File_Download_Completion_Cb
 * Callback type used when a download is finished.
 */
typedef void (*Ecore_File_Download_Completion_Cb)(void *data, const char *file, int status);

/**
 * @typedef _Ecore_File_Progress_Return
 * What to do with the download as a return from the 
 * Ecore_File_Download_Progress_Cb function, if provided.
 */
typedef enum _Ecore_File_Progress_Return
{
   ECORE_FILE_PROGRESS_CONTINUE = 0,   /**< Continue the download. */
   ECORE_FILE_PROGRESS_ABORT = 1       /**< Abort the download. */
} Ecore_File_Progress_Return;

/**
 * @typedef Ecore_File_Download_Progress_Cb
 * Callback type used while a download is in progress.
 */
typedef int (*Ecore_File_Download_Progress_Cb)(void *data,
                                               const char *file,
                                               long int dltotal,
                                               long int dlnow,
                                               long int ultotal,
                                               long int ulnow);

/* File operations */

EAPI int            ecore_file_init         (void);
EAPI int            ecore_file_shutdown     (void);
EAPI long long      ecore_file_mod_time     (const char *file);
EAPI long long      ecore_file_size         (const char *file);
EAPI Eina_Bool      ecore_file_exists       (const char *file);
EAPI Eina_Bool      ecore_file_is_dir       (const char *file);
EAPI Eina_Bool      ecore_file_mkdir        (const char *dir);
EAPI int            ecore_file_mkdirs       (const char **dirs);
EAPI int            ecore_file_mksubdirs    (const char *base, const char **subdirs);
EAPI Eina_Bool      ecore_file_rmdir        (const char *dir);
EAPI Eina_Bool      ecore_file_recursive_rm (const char *dir);
EAPI Eina_Bool      ecore_file_mkpath       (const char *path);
EAPI int            ecore_file_mkpaths      (const char **paths);
EAPI Eina_Bool      ecore_file_cp           (const char *src, const char *dst);
EAPI Eina_Bool      ecore_file_mv           (const char *src, const char *dst);
EAPI Eina_Bool      ecore_file_symlink      (const char *src, const char *dest);
EAPI char          *ecore_file_realpath     (const char *file);
EAPI Eina_Bool      ecore_file_unlink       (const char *file);
EAPI Eina_Bool      ecore_file_remove       (const char *file);
EAPI const char    *ecore_file_file_get     (const char *path);
EAPI char          *ecore_file_dir_get      (const char *path);
EAPI Eina_Bool      ecore_file_can_read     (const char *file);
EAPI Eina_Bool      ecore_file_can_write    (const char *file);
EAPI Eina_Bool      ecore_file_can_exec     (const char *file);
EAPI char          *ecore_file_readlink     (const char *link);
EAPI Eina_List     *ecore_file_ls           (const char *dir);
EAPI Eina_Iterator *ecore_file_ls_iterator  (const char *dir);
EAPI char          *ecore_file_app_exe_get  (const char *app);
EAPI char          *ecore_file_escape_name  (const char *filename);
EAPI char          *ecore_file_strip_ext    (const char *file);
EAPI int            ecore_file_dir_is_empty (const char *dir);

/* Monitoring */

EAPI Ecore_File_Monitor *ecore_file_monitor_add(const char *path,
                                                Ecore_File_Monitor_Cb func,
                                                void *data);
EAPI void                ecore_file_monitor_del(Ecore_File_Monitor *ecore_file_monitor);
EAPI const char         *ecore_file_monitor_path_get(Ecore_File_Monitor *ecore_file_monitor);

/* Path */

EAPI Eina_Bool  ecore_file_path_dir_exists(const char *in_dir);
EAPI Eina_Bool  ecore_file_app_installed(const char *exe);
EAPI Eina_List *ecore_file_app_list(void);

/* Download */

EAPI Eina_Bool ecore_file_download(const char *url,
                                   const char *dst,
                                   Ecore_File_Download_Completion_Cb completion_cb,
                                   Ecore_File_Download_Progress_Cb progress_cb,
                                   void *data,
                                   Ecore_File_Download_Job **job_ret);
EAPI Eina_Bool ecore_file_download_full(const char *url,
                                        const char *dst,
                                        Ecore_File_Download_Completion_Cb completion_cb,
                                        Ecore_File_Download_Progress_Cb progress_cb,
                                        void *data,
                                        Ecore_File_Download_Job **job_ret,
                                        Eina_Hash *headers);

EAPI void      ecore_file_download_abort_all(void);
EAPI void      ecore_file_download_abort(Ecore_File_Download_Job *job);
EAPI Eina_Bool ecore_file_download_protocol_available(const char *protocol);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

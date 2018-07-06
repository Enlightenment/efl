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
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
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
#endif

/**
 * @file Ecore_File.h
 * @brief Files utility functions.
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

/**
 * @brief Initializes the Ecore_File library.
 *
 * @return @c 1 or greater on success, otherwise @c 0 on error.
 *
 * This function sets up Ecore_File and the services it will use
 * (monitoring, downloading, PATH related feature). It returns 0 on
 * failure, otherwise it returns the number of times it has already
 * been called.
 *
 * When Ecore_File is not used anymore, call ecore_file_shutdown()
 * to shut down the Ecore_File library.
 */
EAPI int            ecore_file_init         (void);

/**
 * @brief Shuts down the Ecore_File library.
 *
 * @return @c 0 when the library is completely shut down, @c 1 or
 * greater otherwise.
 *
 * This function shuts down the Ecore_File library. It returns 0 when it has
 * been called the same number of times than ecore_file_init(). In that case
 * it shuts down all the services it uses.
 */
EAPI int            ecore_file_shutdown     (void);

/**
 * @brief Gets the time of the last modification to the given file.
 *
 * @param file The name of the file.
 * @return Return the time of the last data modification, or @c 0 on
 * failure.
 *
 * This function returns the time of the last modification of
 * @p file. On failure, it returns 0.
 */
EAPI long long      ecore_file_mod_time     (const char *file);

/**
 * @brief Gets the size of the given file.
 *
 * @param file The name of the file.
 * @return Return the size of the file in bytes, or @c 0 on failure.
 *
 * This function returns the size of @p file in bytes. On failure, it
 * returns 0.
 */
EAPI long long      ecore_file_size         (const char *file);

/**
 * @brief Checks if the given file exists.
 *
 * @param file The name of the file.
 * @return @c EINA_TRUE if the @p file exists, @c EINA_FALSE otherwise.
 *
 * This function returns @c EINA_TRUE if @p file exists on local filesystem,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool      ecore_file_exists       (const char *file);

/**
 * @brief Checks if the given file is a directory.
 *
 * @param file The name of the file.
 * @return @c EINA_TRUE if the file exists and is a directory, @c EINA_FALSE
 * otherwise.
 *
 * This function returns @c EINA_TRUE if @p file exists exists and is a
 * directory on local filesystem, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool      ecore_file_is_dir       (const char *file);

/**
 * @brief Creates a new directory.
 *
 * @param  dir The name of the directory to create
 * @return @c EINA_TRUE on successful creation, @c EINA_FALSE otherwise.
 *
 * This function creates the directory @p dir, with the mode S_IRUSR |
 * S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH on UNIX
 * (mode is unsued on Windows). On success, it returns @c EINA_TRUE,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool      ecore_file_mkdir        (const char *dir);

/**
 * @brief Creates complete directory in a batch.
 *
 * @param dirs The list of directories, null terminated.
 * @return The number of successful directories created, -1 if dirs is
 * @c NULL.
 *
 * This function creates all the directories that are in the null
 * terminated array @p dirs. The function loops over the directories
 * and call ecore_file_mkdir(). This function returns -1 if @p dirs is
 * @c NULL, otherwise if returns the number of successfully created
 * directories.
 */
EAPI int            ecore_file_mkdirs       (const char **dirs);

/**
 * @brief Creates complete list of sub-directories in a batch (optimized).
 *
 * @param base The base directory to act on.
 * @param subdirs The list of directories, null terminated.
 * @return The number of successful directories created, @c -1 on failure.
 *
 * This function creates all the directories that are in the null
 * terminated array @p subdirs in the @p base directory. If @p base does
 * not exist, it will be created. The function loops over the directories
 * and call ecore_file_mkdir(). The whole path of the directories must
 * exist. So if base/a/b/c wants to be created, @p subdirs must
 * contain "a", "a/b" and "a/b/c", in that order. This function
 * returns -1 if @p subdirs or @p base are @c NULL, or if @p base is
 * empty ("\0"). It returns 0 is @p base is not a directory or
 * invalid, or if it can't be created. Otherwise if returns the number
 * of successfully created directories.
 */
EAPI int            ecore_file_mksubdirs    (const char *base, const char **subdirs);

/**
 * @brief Deletes the given empty directory.
 *
 * @param  dir The name of the directory to delete.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function deletes @p dir. It returns @c EINA_TRUE on success,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool      ecore_file_rmdir        (const char *dir);

/**
 * @brief Deletes the given directory and all its contents.
 *
 * @param  dir The name of the directory to delete.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function delete @p dir and all its contents. If @p dir is a
 * link only the link is removed. It returns @c EINA_TRUE on success,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool      ecore_file_recursive_rm (const char *dir);

/**
 * @brief Creates a complete path.
 *
 * @param path The path to create
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function creates @p path and all the subdirectories it
 * contains. The separator is '/' or '\'. If @p path exists, this
 * function returns @c EINA_TRUE immediately. It returns @c EINA_TRUE on
 * success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool      ecore_file_mkpath       (const char *path);

/**
 * @brief Creates complete paths in a batch.
 *
 * @param paths list of paths, null terminated.
 * @return The number of successful paths created, @c -1 if paths is NULL.
 *
 * This function creates all the directories that are in the null
 * terminated array @p paths. The function loops over the directories
 * and call ecore_file_mkpath(), hence on Windows, '\' must be
 * replaced by '/' before calling that function. This function
 * returns -1 if @p paths is @c NULL. Otherwise if returns the number
 * of successfully created directories.
 */
EAPI int            ecore_file_mkpaths      (const char **paths);

/**
 * @brief Copies the given file to the given destination.
 *
 * @param  src The name of the source file.
 * @param  dst The name of the destination file.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function copies @p src to @p dst. If the absolute path name of
 * @p src and @p dst can not be computed, or if they are equal, or if
 * the copy fails, the function returns @c EINA_FALSE, otherwise it
 * returns @c EINA_TRUE.
 */
EAPI Eina_Bool      ecore_file_cp           (const char *src, const char *dst);

/**
 * @brief Moves the given file to the given destination.
 *
 * @param  src The name of the source file.
 * @param  dst The name of the destination file.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function moves @p src to @p dst. It returns @c EINA_TRUE on
 * success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool      ecore_file_mv           (const char *src, const char *dst);

/**
 * @brief Creates a symbolic link.
 *
 * @param  src The name of the file to link.
 * @param  dest The name of link.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function creates the symbolic link @p dest of @p src. It returns
 * @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * @note On windows, this function always returns @c EINA_FALSE.
 */
EAPI Eina_Bool      ecore_file_symlink      (const char *src, const char *dest);

/**
 * @brief Gets the canonicalized absolute path name.
 *
 * @param  file The file path.
 * @return The canonicalized absolute pathname or an empty string on
 * failure.
 *
 * This function returns the absolute path name of @p file as a newly
 * allocated string. If @p file is @c NULL, or on error, this function
 * returns an empty string. Otherwise, it returns the absolute path
 * name. When not needed anymore, the returned value must be freed.
 */
EAPI char          *ecore_file_realpath     (const char *file);

/**
 * @brief Deletes the given file.
 *
 * @param  file The name of the file to delete.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function deletes @p file. It returns @c EINA_TRUE on success,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool      ecore_file_unlink       (const char *file);

/**
 * @brief Removes the given file or directory.
 *
 * @param  file The name of the file or directory to delete.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function removes @p file. It returns @c EINA_TRUE on success,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool      ecore_file_remove       (const char *file);

/**
 * @brief Gets the filename from a given path.
 *
 * @param  path The complete path.
 * @return The file name.
 *
 * This function returns the file name of @p path. If @p path is
 * @c NULL, the functions returns @c NULL.
 */
EAPI const char    *ecore_file_file_get     (const char *path);

/**
 * @brief Gets the directory where the given file resides.
 *
 * @param  file The name of the file.
 * @return The directory name.
 *
 * This function returns the directory where @p file resides as anewly
 * allocated string. If @p file is @c NULL or on error, this function
 * returns @c NULL. When not needed anymore, the returned value must
 * be freed.
 */
EAPI char          *ecore_file_dir_get      (const char *file);

/**
 * @brief Checks if the given file can be read.
 *
 * @param  file The name of the file.
 * @return @c EINA_TRUE if the @p file is readable, @c EINA_FALSE otherwise.
 *
 * This function returns @c EINA_TRUE if @p file can be read, @c EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool      ecore_file_can_read     (const char *file);

/**
 * @brief Checks if the given file can be written.
 *
 * @param  file The name of the file.
 * @return @c EINA_TRUE if the @p file is writable, @c EINA_FALSE otherwise.
 *
 * This function returns @c EINA_TRUE if @p file can be written, @c EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool      ecore_file_can_write    (const char *file);

/**
 * @brief Checks if the given file can be executed.
 *
 * @param  file The name of the file.
 * @return @c EINA_TRUE if the @p file can be executed, @c EINA_FALSE
 * otherwise.
 *
 * This function returns @c EINA_TRUE if @p file can be executed, @c EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool      ecore_file_can_exec     (const char *file);

/**
 * @brief Gets the path pointed by the given link.
 *
 * @param  link The name of the link.
 * @return The path pointed by link or NULL.
 *
 * This function returns the path pointed by @p link as a newly
 * allocated string. On failure, the function returns @c NULL. When not
 * needed anymore, the returned value must be freed.
 *
 * @note On windows, this function always returns @c NULL.
 */
EAPI char          *ecore_file_readlink     (const char *link);

/**
 * @brief Gets the list of the files and directories in the given
 * directory.
 *
 * @param  dir The name of the directory to list
 * @return Return an Eina_List containing all the files in the directory;
 *         on failure it returns NULL.
 *
 * This function returns a list of allocated strings of all the files
 * and directories contained in @p dir. The list will be sorted with
 * strcoll as compare function. That means that you may want to set
 * the current locale for the category LC_COLLATE with
 * setlocale(). For more information see the manual pages of strcoll
 * and setlocale. The list will not contain the directory entries for
 * '.' and '..'. On failure, @c NULL is returned. When not needed
 * anymore, the list elements must be freed.
 */
EAPI Eina_List     *ecore_file_ls           (const char *dir);

/**
 * @brief Returns the executable from the given command.
 *
 * @param app The application command, with parameters.
 * @return The executable from @p app as a newly allocated string. Arguments
 * are removed and escape characters are handled. If @p app is @c NULL, or
 * on failure, the function returns @c NULL. When not needed anymore, the
 * returned value must be freed.
 */
EAPI char          *ecore_file_app_exe_get  (const char *app);

/**
 * @brief Adds the escape sequence ('\\') to the given file name.
 *
 * @param  filename The file name.
 * @return The file name with special characters escaped.
 *
 * This function adds the escape sequence ('\\') to the given file
 * name and returns the result as a newly allocated string. If the
 * length of the returned string is longer than PATH_MAX, or on
 * failure, @c NULL is returned. When not needed anymore, the returned
 * value must be freed.
 */
EAPI char          *ecore_file_escape_name  (const char *filename);

/**
 * @brief Removes the extension from the given file name.
 *
 * @param  path The name of the file.
 * @return A newly allocated string with the extension stripped out, or
 * @c NULL on errors.
 *
 * This function removes the extension from @p path and returns the
 * result as a newly allocated string. If @p path is @c NULL, or on
 * failure, the function returns @c NULL. When not needed anymore, the
 * returned value must be freed.
 */
EAPI char          *ecore_file_strip_ext    (const char *path);

/**
 * @brief Checks if the given directory is empty.
 *
 * @param  dir The name of the directory to check.
 * @return @c 1 if directory is empty, @c 0 if it has at least one file, or
 * @c -1 in case of errors.
 *
 * This functions checks if @p dir is empty. The '.' and '..' files
 * will be ignored. If @p dir is empty, 1 is returned, if it contains
 * at least one file, @c 0 is returned. On failure, @c -1 is returned.
 */
EAPI int            ecore_file_dir_is_empty (const char *dir);

/* Monitoring */

/**
 * @brief Monitors the given path using inotify, Windows notification, or polling.
 *
 * @param  path The path to monitor.
 * @param  func The function to call on changes.
 * @param  data The data passed to func.
 * @return An Ecore_File_Monitor pointer or NULL on failure.
 *
 * This function monitors @p path. If @p path is @c NULL, or is an
 * empty string, or none of the notify methods (Inotify, Windows
 * notification or polling) is available, or if @p path does not exist
 * the function returns @c NULL. Otherwise, it returns a newly 
 * allocated Ecore_File_Monitor object and the monitoring begins.
 * When one of the Ecore_File_Event event is notified, @p func is called
 * and @p data is passed to @p func.Call ecore_file_monitor_del() to
 * stop the monitoring.
 */
EAPI Ecore_File_Monitor *ecore_file_monitor_add(const char *path,
                                                Ecore_File_Monitor_Cb func,
                                                void *data);

/**
 * @brief Stops the monitoring of the given path.
 *
 * @param em The Ecore_File_Monitor to stop.
 *
 * This function stops the the monitoring of the path that has been
 * monitored by ecore_file_monitor_add(). @p em must be the value
 * returned by ecore_file_monitor_add(). If @p em is @c NULL, or none
 * of the notify methods (Inotify, Windows notification or polling) is
 * availablethis function does nothing.
 */
EAPI void                ecore_file_monitor_del(Ecore_File_Monitor *em);

/**
 * @brief Gets the monitored path.
 *
 * @param  em The Ecore_File_Monitor to query.
 * @return The path that is monitored by @p em.
 *
 * This function returns the monitored path that has been
 * monitored by ecore_file_monitor_add(). @p em must be the value
 * returned by ecore_file_monitor_add(). If @p em is @c NULL, the
 * function returns @c NULL.
 */
EAPI const char         *ecore_file_monitor_path_get(Ecore_File_Monitor *em);

/* Path */

/**
 * @brief Checks if the given directory is in PATH.
 *
 * @param in_dir The name of the directory to search in PATH.
 * @return @c EINA_TRUE if the directory exist in PATH, @c EINA_FALSE otherwise.
 *
 * This function checks if @p in_dir is in the environment variable
 * PATH. If @p in_dir is @c NULL, or if PATH is empty, or @p in_dir is
 * not in PATH, the function returns @c EINA_FALSE, otherwise it returns
 * @c EINA_TRUE.
 */
EAPI Eina_Bool  ecore_file_path_dir_exists(const char *in_dir);

/**
 * @brief Checks if the given application is installed.
 *
 * @param  exe The name of the application
 * @return @c EINA_TRUE if the @p exe is in PATH and is executable,
 * @c EINA_FALSE otherwise.
 *
 * This function checks if @p exe exists in PATH and is executable. If
 * @p exe is @c NULL or is not executable, the function returns
 * @c EINA_FALSE, otherwise it returns @c EINA_TRUE.
 */
EAPI Eina_Bool  ecore_file_app_installed(const char *exe);

/**
 * @brief Gets a list of all the applications installed on the system.
 *
 * @return An Eina_List containing all the executable files in the
 * system.
 *
 * This function returns a list of allocated strings of all the
 * executable files. If no files are found, the function returns
 * @c NULL. When not needed anymore, the element of the list must be
 * freed.
 */
EAPI Eina_List *ecore_file_app_list(void);

/* Download */

/**
 * @brief Downloads the given url to the given destination.
 *
 * @param  url The complete url to download.
 * @param  dst The local file to save the downloaded to.
 * @param  completion_cb A callback called on download complete.
 * @param  progress_cb A callback called during the download operation.
 * @param  data User data passed to both callbacks.
 * @param  job_ret Job used to abort the download.
 * @return @c EINA_TRUE if the download start or @c EINA_FALSE on failure.
 *
 * This function starts the download of the URL @p url and saves it to
 * @p dst. @p url must provide the protocol, including 'http://',
 * 'ftp://' or 'file://'. Ecore_File must be compiled with CURL to
 * download using http and ftp protocols. If @p dst is ill-formed, or
 * if it already exists, the function returns @c EINA_FALSE. When the
 * download is complete, the callback @p completion_cb is called and
 * @p data is passed to it. The @p status parameter of @p completion_cb
 * will be filled with the status of the download (200, 404,...). The
 * @p progress_cb is called during the download operation, each time a
 * packet is received or when CURL wants. It can be used to display the
 * percentage of the downloaded file. Return 0 from this callback, if provided,
 * to continue the operation or anything else to abort the download. The only
 * operations that can be aborted are those with protocol 'http' or 'ftp'. In
 * that case @p job_ret can be passed to ecore_file_download_abort() to abort
 * that download job. Similarly ecore_file_download_abort_all() can be used to
 * abort all download operations. This function returns @c EINA_TRUE if the
 * download starts, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool ecore_file_download(const char *url,
                                   const char *dst,
                                   Ecore_File_Download_Completion_Cb completion_cb,
                                   Ecore_File_Download_Progress_Cb progress_cb,
                                   void *data,
                                   Ecore_File_Download_Job **job_ret);

/**
 * @brief Downloads the given url to the given destination with additional headers.
 *
 * @param  url The complete url to download.
 * @param  dst The local file to save the downloaded to.
 * @param  completion_cb A callback called on download complete.
 * @param  progress_cb A callback called during the download operation.
 * @param  data User data passed to both callbacks.
 * @param  job_ret Job used to abort the download.
 * @param  headers pointer of header lists.
 * @return @c EINA_TRUE if the download start or @c EINA_FALSE on failure.
 */
EAPI Eina_Bool ecore_file_download_full(const char *url,
                                        const char *dst,
                                        Ecore_File_Download_Completion_Cb completion_cb,
                                        Ecore_File_Download_Progress_Cb progress_cb,
                                        void *data,
                                        Ecore_File_Download_Job **job_ret,
                                        Eina_Hash *headers);

/**
 * @brief Aborts all downloads.
 *
 * This function aborts all the downloads that have been started by
 * ecore_file_download(). It loops over the started downloads and call
 * ecore_file_download_abort() for each of them. To abort only one
 * specific download operation, call ecore_file_download_abort().
 */
EAPI void      ecore_file_download_abort_all(void);

/**
 * @brief Aborts the given download job and call the completion_cb
 * callbck with a status of 1 (error).
 *
 * @param job The download job to abort.
 *
 * This function aborts a download operation started by
 * ecore_file_download(). @p job is the #Ecore_File_Download_Job
 * structure filled by ecore_file_download(). If it is @c NULL, this
 * function does nothing. To abort all the currently downloading
 * operations, call ecore_file_download_abort_all().
 */
EAPI void      ecore_file_download_abort(Ecore_File_Download_Job *job);

/**
 * @brief Checks if the given protocol is available.
 *
 * @param  protocol The protocol to check.
 * @return @c EINA_TRUE if protocol is handled, @c EINA_FALSE otherwise.
 *
 * This function returns @c EINA_TRUE if @p protocol is supported,
 * @c EINA_FALSE otherwise. @p protocol can be 'http://', 'ftp://' or
 * 'file://'. Ecore_FILE must be compiled with CURL to handle http and
 * ftp protocols.
 */
EAPI Eina_Bool ecore_file_download_protocol_available(const char *protocol);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif

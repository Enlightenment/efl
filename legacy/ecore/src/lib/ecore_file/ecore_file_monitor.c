#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_file_private.h"

typedef enum {
     ECORE_FILE_MONITOR_TYPE_NONE,
#ifdef HAVE_INOTIFY
     ECORE_FILE_MONITOR_TYPE_INOTIFY,
#endif
#ifdef HAVE_NOTIFY_WIN32
     ECORE_FILE_MONITOR_TYPE_NOTIFY_WIN32,
#endif
#ifdef HAVE_POLL
     ECORE_FILE_MONITOR_TYPE_POLL
#endif
} Ecore_File_Monitor_Type;

static Ecore_File_Monitor_Type monitor_type = ECORE_FILE_MONITOR_TYPE_NONE;

int
ecore_file_monitor_init(void)
{
#ifdef HAVE_INOTIFY
   monitor_type = ECORE_FILE_MONITOR_TYPE_INOTIFY;
   if (ecore_file_monitor_inotify_init())
     return 1;
#endif
#ifdef HAVE_NOTIFY_WIN32
   monitor_type = ECORE_FILE_MONITOR_TYPE_NOTIFY_WIN32;
   if (ecore_file_monitor_win32_init())
     return 1;
#endif
#ifdef HAVE_POLL
   monitor_type = ECORE_FILE_MONITOR_TYPE_POLL;
   if (ecore_file_monitor_poll_init())
     return 1;
#endif
   monitor_type = ECORE_FILE_MONITOR_TYPE_NONE;
   return 0;
}

void
ecore_file_monitor_shutdown(void)
{
   switch (monitor_type)
     {
      case ECORE_FILE_MONITOR_TYPE_NONE:
         break;
#ifdef HAVE_INOTIFY
      case ECORE_FILE_MONITOR_TYPE_INOTIFY:
         ecore_file_monitor_inotify_shutdown();
         break;
#endif
#ifdef HAVE_NOTIFY_WIN32
      case ECORE_FILE_MONITOR_TYPE_NOTIFY_WIN32:
         ecore_file_monitor_win32_shutdown();
         break;
#endif
#ifdef HAVE_POLL
      case ECORE_FILE_MONITOR_TYPE_POLL:
         ecore_file_monitor_poll_shutdown();
         break;
#endif
     }
}

/**
 * @addtogroup Ecore_File_Group Ecore_File - Files and directories convenience functions
 *
 * @{
 */

/**
 * @brief Monitor the given path using inotify, Windows notification, or polling.
 *
 * @param  path The path to monitor.
 * @param  func The function to call on changes.
 * @param  data The data passed to func.
 * @return An Ecore_File_Monitor pointer or NULL on failure.
 *
 * This function monitors @p path. If @p path is @c NULL, or is an
 * empty string, or none of the notify methods (Inotify, Windows
 * notification or polling) is available, or if @p path is not a file,
 * the function returns @c NULL. Otherwise, it returns a newly
 * allocated Ecore_File_Monitor object and the monitoring begins. When
 * one of the #Ecore_File_Event event is notified, @p func is called
 * and @p data is passed to @p func. Call ecore_file_monitor_del() to
 * stop the monitoring.
 */
EAPI Ecore_File_Monitor *
ecore_file_monitor_add(const char           *path,
                       Ecore_File_Monitor_Cb func,
                       void                 *data)
{
   if (!path || !*path)
     return NULL;

   switch (monitor_type)
     {
      case ECORE_FILE_MONITOR_TYPE_NONE:
         return NULL;
#ifdef HAVE_INOTIFY
      case ECORE_FILE_MONITOR_TYPE_INOTIFY:
         return ecore_file_monitor_inotify_add(path, func, data);
#endif
#ifdef HAVE_NOTIFY_WIN32
      case ECORE_FILE_MONITOR_TYPE_NOTIFY_WIN32:
         return ecore_file_monitor_win32_add(path, func, data);
#endif
#ifdef HAVE_POLL
      case ECORE_FILE_MONITOR_TYPE_POLL:
         return ecore_file_monitor_poll_add(path, func, data);
#endif
     }
   return NULL;
}

/**
 * @brief Stop the monitoring of the given path.
 *
 * @param em The Ecore_File_Monitor to stop.
 *
 * This function stops the the monitoring of the path that has been
 * monitored by ecore_file_monitor_add(). @p em must be the value
 * returned by ecore_file_monitor_add(). If @p em is @c NULL, or none
 * of the notify methods (Inotify, Windows notification or polling) is
 * availablethis function does nothing.
 */
EAPI void
ecore_file_monitor_del(Ecore_File_Monitor *em)
{
   if (!em)
     return;

   switch (monitor_type)
     {
      case ECORE_FILE_MONITOR_TYPE_NONE:
         break;
#ifdef HAVE_INOTIFY
      case ECORE_FILE_MONITOR_TYPE_INOTIFY:
         ecore_file_monitor_inotify_del(em);
         break;
#endif
#ifdef HAVE_NOTIFY_WIN32
      case ECORE_FILE_MONITOR_TYPE_NOTIFY_WIN32:
         ecore_file_monitor_win32_del(em);
         break;
#endif
#ifdef HAVE_POLL
      case ECORE_FILE_MONITOR_TYPE_POLL:
         ecore_file_monitor_poll_del(em);
         break;
#endif
     }
}

/**
 * @brief Get the monitored path.
 *
 * @param  em The Ecore_File_Monitor to query.
 * @return The path that is monitored by @p em.
 *
 * This function returns the monitored path that has been
 * monitored by ecore_file_monitor_add(). @p em must be the value
 * returned by ecore_file_monitor_add(). If @p em is @c NULL, the
 * function returns @c NULL.
 */
EAPI const char *
ecore_file_monitor_path_get(Ecore_File_Monitor *em)
{
   if (!em)
     return NULL;
   return em->path;
}

/**
 * @}
 */

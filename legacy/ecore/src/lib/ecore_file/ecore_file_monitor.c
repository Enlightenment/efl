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
 * Monitor a path using inotify or polling
 * @param  path The path to monitor
 * @param  func The function to call on changes
 * @param  data The data passed to func
 * @return An Ecore_File_Monitor pointer or NULL on failure
 */
EAPI Ecore_File_Monitor *
ecore_file_monitor_add(const char *path,
                       void      (*func) (void               *data,
                                          Ecore_File_Monitor *em,
                                          Ecore_File_Event    event,
                                          const char         *path),
                       void       *data)
{
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
 * Stop monitoring a path
 * @param  em The Ecore_File_Monitor to stop
 */
EAPI void
ecore_file_monitor_del(Ecore_File_Monitor *em)
{
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
 * Get the monitored path
 * @param  em The Ecore_File_Monitor to query
 * @return The path that is monitored by @p em
 */
EAPI const char *
ecore_file_monitor_path_get(Ecore_File_Monitor *em)
{
   return em->path;
}

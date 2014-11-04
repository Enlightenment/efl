#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_file_private.h"

int
ecore_file_monitor_init(void)
{
   if (ecore_file_monitor_backend_init())
     return 1;
   return 0;
}

void
ecore_file_monitor_shutdown(void)
{
   ecore_file_monitor_backend_shutdown();
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
 * notification or polling) is available, or if @p path does not exist
 * the function returns @c NULL. Otherwise, it returns a newly 
 * allocated Ecore_File_Monitor object and the monitoring begins.
 * When one of the Ecore_File_Event event is notified, @p func is called
 * and @p data is passed to @p func.Call ecore_file_monitor_del() to
 * stop the monitoring.
 */
EAPI Ecore_File_Monitor *
ecore_file_monitor_add(const char           *path,
                       Ecore_File_Monitor_Cb func,
                       void                 *data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(path[0] == '\0', NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);

   return ecore_file_monitor_backend_add(path, func, data);
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
   if (!em) return;
   EINA_SAFETY_ON_NULL_RETURN(em);
   ecore_file_monitor_backend_del(em);
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
   EINA_SAFETY_ON_NULL_RETURN_VAL(em, NULL);
   return em->path;
}

/**
 * @}
 */

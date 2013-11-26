#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_drm_private.h"

/* local variables */
static int _ecore_drm_init_count = 0;

/* external variables */
int _ecore_drm_log_dom = -1;
#ifdef LOG_TO_FILE
FILE *lg;
#endif

/**
 * @defgroup Ecore_Drm_Init_Group Drm Library Init and Shutdown Functions
 * 
 * Functions that start and shutdown the Ecore_Drm Library.
 */

/**
 * Initialize the Ecore_Drm library
 * 
 * @return  The number of times the library has been initialized without
 *          being shut down. 0 is returned if an error occurs.
 * 
 * @ingroup Ecore_Drm_Init_Group
 */
EAPI int 
ecore_drm_init(void)
{
   /* if we have already initialized, return the count */
   if (++_ecore_drm_init_count != 1) return _ecore_drm_init_count;

   /* try to init eina */
   if (!eina_init()) return --_ecore_drm_init_count;

   /* set logging level */
   eina_log_level_set(EINA_LOG_LEVEL_DBG);

   /* optionally log output to a file */
#ifdef LOG_TO_FILE
   int log_fd;
   char log_path[PATH_MAX];
   mode_t um;

   /* assemble logging file path */
   strcpy(log_path, "/tmp/ecore_drm_XXXXXX");

   /* create temporary logging file */
   um = umask(S_IRWXG | S_IRWXO);
   log_fd = mkstemp(log_path);
   umask(um);

   /* try to open logging file */
   if (!(lg = fdopen(log_fd, "w")))
     goto log_err;

   eina_log_print_cb_set(eina_log_print_cb_file, lg);
#endif

   /* try to create logging domain */
   _ecore_drm_log_dom = 
     eina_log_domain_register("ecore_drm", ECORE_DRM_DEFAULT_LOG_COLOR);
   if (!_ecore_drm_log_dom)
     {
        EINA_LOG_ERR("Could not create log domain for Ecore_Drm");
        goto log_err;
     }

   /* set default logging level for this domain */
   if (!eina_log_domain_level_check(_ecore_drm_log_dom, EINA_LOG_LEVEL_DBG))
     eina_log_domain_level_set(_ecore_drm_log_dom, EINA_LOG_LEVEL_DBG);

   /* return init count */
   return _ecore_drm_init_count;

log_err:
#ifdef LOG_TO_FILE
   if (lg) fclose(lg);
#endif

   /* shutdown eina */
   eina_shutdown();

   return --_ecore_drm_init_count;
}

/**
 * Shutdown the Ecore_Drm library.
 * 
 * @return  The number of times the library has been initialized without
 *          being shutdown. 0 is returned if an error occurs.
 * 
 * @ingroup Ecore_Drm_Init_Group
 */
EAPI int 
ecore_drm_shutdown(void)
{
   /* if we are still in use, decrement init count and get out */
   if (--_ecore_drm_init_count != 0) return _ecore_drm_init_count;

   /* unregsiter log domain */
   eina_log_domain_unregister(_ecore_drm_log_dom);
   _ecore_drm_log_dom = -1;

#ifdef LOG_TO_FILE
   if (lg) fclose(lg);
#endif

   /* shutdown eina */
   eina_shutdown();

   /* return init count */
   return _ecore_drm_init_count;
}

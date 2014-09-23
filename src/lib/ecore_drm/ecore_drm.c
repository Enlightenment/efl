#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_drm_private.h"

/* local variables */
static int _ecore_drm_init_count = 0;
static char *sid;

/* external variables */
int _ecore_drm_log_dom = -1;

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

   /* try to init ecore */
   if (!ecore_init()) 
     {
        eina_shutdown();
        return --_ecore_drm_init_count;
     }

   /* try to init ecore_event */
   if (!ecore_event_init())
     {
        ecore_shutdown();
        eina_shutdown();
        return --_ecore_drm_init_count;
     }

   /* set logging level */
   eina_log_level_set(EINA_LOG_LEVEL_DBG);

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
     eina_log_domain_level_set("ecore_drm", EINA_LOG_LEVEL_DBG);

   /* get sd-login properties we need */
   if (sd_pid_get_session(getpid(), &sid) < 0) goto sd_err;

   /* try to init dbus */
   if (!_ecore_drm_dbus_init(sid)) goto dbus_err;

   /* try to init eeze */
   if (!eeze_init()) goto eeze_err;

   /* return init count */
   return _ecore_drm_init_count;

eeze_err:
   _ecore_drm_dbus_shutdown();
dbus_err:
   free(sid);
sd_err:
   eina_log_domain_unregister(_ecore_drm_log_dom);
   _ecore_drm_log_dom = -1;
log_err:
   ecore_event_shutdown();
   ecore_shutdown();
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

   /* close eeze */
   eeze_shutdown();

   /* cleanup dbus */
   _ecore_drm_dbus_shutdown();

   /* shutdown ecore_event */
   ecore_event_shutdown();

   /* shutdown ecore */
   ecore_shutdown();

   /* unregsiter log domain */
   eina_log_domain_unregister(_ecore_drm_log_dom);
   _ecore_drm_log_dom = -1;

   /* shutdown eina */
   eina_shutdown();

   free(sid);

   /* return init count */
   return _ecore_drm_init_count;
}

EAPI void *
ecore_drm_gbm_get(Ecore_Drm_Device *dev)
{
   if (!dev) return NULL;

#ifdef HAVE_GBM
   return dev->gbm;
#endif

   return NULL;
}

EAPI unsigned int 
ecore_drm_gbm_format_get(Ecore_Drm_Device *dev)
{
   if (!dev) return 0;
   return dev->format;
}

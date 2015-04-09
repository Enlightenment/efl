#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Ecore_Drm.h"
#include "ecore_drm_private.h"

/* local variables */
static int _ecore_drm_init_count = 0;

/* external variables */
int _ecore_drm_log_dom = -1;

EAPI int ECORE_DRM_EVENT_ACTIVATE = 0;
EAPI int ECORE_DRM_EVENT_PAGE_FLIP = 0;
EAPI int ECORE_DRM_EVENT_VBLANK = 0;

static void
_ecore_drm_event_activate_free(void *data EINA_UNUSED, void *event)
{
   free(event);
}

void
_ecore_drm_event_activate_send(Eina_Bool active)
{
   Ecore_Drm_Event_Activate *e;

   if (!(e = calloc(1, sizeof(Ecore_Drm_Event_Activate)))) return;

   e->active = active;
   ecore_event_add(ECORE_DRM_EVENT_ACTIVATE, e, 
                   _ecore_drm_event_activate_free, NULL);
}

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

   /* try to init eeze */
   if (!eeze_init()) goto eeze_err;

   ECORE_DRM_EVENT_ACTIVATE = ecore_event_type_new();
   ECORE_DRM_EVENT_PAGE_FLIP = ecore_event_type_new();
   ECORE_DRM_EVENT_VBLANK = ecore_event_type_new();

   /* return init count */
   return _ecore_drm_init_count;

eeze_err:
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
   /* _ecore_drm_init_count should not go below zero. */
   if (_ecore_drm_init_count < 1)
     {
        ERR("Ecore_Drm Shutdown called without Ecore_Drm Init");
        return 0;
     }

   /* if we are still in use, decrement init count and get out */
   if (--_ecore_drm_init_count != 0) return _ecore_drm_init_count;

   /* close eeze */
   eeze_shutdown();

   /* shutdown ecore_event */
   ecore_event_shutdown();

   /* shutdown ecore */
   ecore_shutdown();

   /* unregsiter log domain */
   eina_log_domain_unregister(_ecore_drm_log_dom);
   _ecore_drm_log_dom = -1;

   /* shutdown eina */
   eina_shutdown();

   /* return init count */
   return _ecore_drm_init_count;
}

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

/* local variables */
static int _ecore_wl2_init_count = 0;

/* external variables */
int _ecore_wl2_log_dom = -1;

/* public API functions */
EAPI int
ecore_wl2_init(void)
{
   if (++_ecore_wl2_init_count != 1) return _ecore_wl2_init_count;

   /* try to initialize Eina */
   if (!eina_init()) return --_ecore_wl2_init_count;

   /* try to create Eina logging domain */
   _ecore_wl2_log_dom =
     eina_log_domain_register("ecore_wl2", ECORE_WL2_DEFAULT_LOG_COLOR);
   if (_ecore_wl2_log_dom < 0)
     {
        EINA_LOG_ERR("Cannot create a log domain for Ecore Wl2");
        goto eina_err;
     }

   /* try to initialize Ecore */
   if (!ecore_init())
     {
        ERR("Could not initialize Ecore");
        goto ecore_err;
     }

   /* try to initialize Ecore_Event */
   if (!ecore_event_init())
     {
        ERR("Could not initialize Ecore_Event");
        goto ecore_event_err;
     }

   return _ecore_wl2_init_count;

ecore_event_err:
   ecore_shutdown();

ecore_err:
   eina_log_domain_unregister(_ecore_wl2_log_dom);
   _ecore_wl2_log_dom = -1;

eina_err:
   eina_shutdown();
   return --_ecore_wl2_init_count;
}

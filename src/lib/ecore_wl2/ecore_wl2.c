#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

/* local variables */
static int _ecore_wl2_init_count = 0;

/* external variables */
int _ecore_wl2_log_dom = -1;

/* public API variables */
EAPI int ECORE_WL2_EVENT_GLOBAL_ADDED = 0;
EAPI int ECORE_WL2_EVENT_GLOBAL_REMOVED = 0;
EAPI int ECORE_WL2_EVENT_FOCUS_IN = 0;
EAPI int ECORE_WL2_EVENT_FOCUS_OUT = 0;
EAPI int ECORE_WL2_EVENT_DND_ENTER = 0;
EAPI int ECORE_WL2_EVENT_DND_LEAVE = 0;
EAPI int ECORE_WL2_EVENT_DND_MOTION = 0;

static void
_cb_wl_log_print(const char *format, va_list args)
{
   EINA_LOG_DOM_INFO(_ecore_wl2_log_dom, format, args);
}

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

   /* handle creating new Ecore_Wl2 event types */
   if (!ECORE_WL2_EVENT_GLOBAL_ADDED)
     {
        ECORE_WL2_EVENT_GLOBAL_ADDED = ecore_event_type_new();
        ECORE_WL2_EVENT_GLOBAL_REMOVED = ecore_event_type_new();
        ECORE_WL2_EVENT_FOCUS_IN = ecore_event_type_new();
        ECORE_WL2_EVENT_FOCUS_OUT = ecore_event_type_new();
        ECORE_WL2_EVENT_DND_ENTER = ecore_event_type_new();
        ECORE_WL2_EVENT_DND_LEAVE = ecore_event_type_new();
        ECORE_WL2_EVENT_DND_MOTION = ecore_event_type_new();
     }

   wl_log_set_handler_server(_cb_wl_log_print);

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

EAPI int
ecore_wl2_shutdown(void)
{
   if (_ecore_wl2_init_count < 1)
     {
        ERR("Ecore_Wl2 shutdown called without Ecore_Wl2 Init");
        return 0;
     }

   if (--_ecore_wl2_init_count != 0) return _ecore_wl2_init_count;

   /* reset events */
   ECORE_WL2_EVENT_GLOBAL_ADDED = 0;
   ECORE_WL2_EVENT_GLOBAL_REMOVED = 0;

   /* shutdown Ecore_Event */
   ecore_event_shutdown();

   /* shutdown Ecore */
   ecore_shutdown();

   /* unregister logging domain */
   eina_log_domain_unregister(_ecore_wl2_log_dom);
   _ecore_wl2_log_dom = -1;

   /* shutdown eina */
   eina_shutdown();

   return _ecore_wl2_init_count;
}

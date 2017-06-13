#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

/* local variables */
static int _ecore_wl2_init_count = 0;

/* external variables */
Eina_Bool no_session_recovery;
int _ecore_wl2_log_dom = -1;

/* public API variables */
EAPI int ECORE_WL2_EVENT_CONNECT = 0;
EAPI int ECORE_WL2_EVENT_DISCONNECT = 0;
EAPI int ECORE_WL2_EVENT_GLOBAL_ADDED = 0;
EAPI int ECORE_WL2_EVENT_GLOBAL_REMOVED = 0;
EAPI int ECORE_WL2_EVENT_FOCUS_IN = 0;
EAPI int ECORE_WL2_EVENT_FOCUS_OUT = 0;
EAPI int ECORE_WL2_EVENT_DND_ENTER = 0;
EAPI int ECORE_WL2_EVENT_DND_LEAVE = 0;
EAPI int ECORE_WL2_EVENT_DND_MOTION = 0;
EAPI int ECORE_WL2_EVENT_DND_DROP = 0;
EAPI int ECORE_WL2_EVENT_DND_END = 0;
EAPI int ECORE_WL2_EVENT_DATA_SOURCE_END = 0;
EAPI int ECORE_WL2_EVENT_DATA_SOURCE_DROP = 0;
EAPI int ECORE_WL2_EVENT_DATA_SOURCE_ACTION = 0;
EAPI int ECORE_WL2_EVENT_DATA_SOURCE_TARGET = 0;
EAPI int ECORE_WL2_EVENT_DATA_SOURCE_SEND = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_CONFIGURE = 0;
EAPI int ECORE_WL2_EVENT_SYNC_DONE = 0;
EAPI int ECORE_WL2_EVENT_OFFER_DATA_READY = 0;
EAPI int ECORE_WL2_EVENT_SEAT_NAME_CHANGED = 0;
EAPI int ECORE_WL2_EVENT_SEAT_CAPABILITIES_CHANGED = 0;
EAPI int ECORE_WL2_EVENT_DEVICE_ADDED = 0;
EAPI int ECORE_WL2_EVENT_DEVICE_REMOVED = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_CONFIGURE_COMPLETE = 0;
EAPI int ECORE_WL2_EVENT_SEAT_KEYMAP_CHANGED = 0;
EAPI int ECORE_WL2_EVENT_SEAT_KEYBOARD_REPEAT_CHANGED = 0;
EAPI int ECORE_WL2_EVENT_SEAT_SELECTION = 0;
EAPI int ECORE_WL2_EVENT_OUTPUT_TRANSFORM = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_ROTATE = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE_DONE = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_REQUEST = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_DONE = 0;
EAPI int ECORE_WL2_EVENT_AUX_HINT_ALLOWED = 0;

EAPI int _ecore_wl2_event_window_www = -1;
EAPI int _ecore_wl2_event_window_www_drag = -1;

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
        ECORE_WL2_EVENT_CONNECT = ecore_event_type_new();
        ECORE_WL2_EVENT_DISCONNECT = ecore_event_type_new();
        ECORE_WL2_EVENT_GLOBAL_ADDED = ecore_event_type_new();
        ECORE_WL2_EVENT_GLOBAL_REMOVED = ecore_event_type_new();
        ECORE_WL2_EVENT_FOCUS_IN = ecore_event_type_new();
        ECORE_WL2_EVENT_FOCUS_OUT = ecore_event_type_new();
        ECORE_WL2_EVENT_DND_ENTER = ecore_event_type_new();
        ECORE_WL2_EVENT_DND_LEAVE = ecore_event_type_new();
        ECORE_WL2_EVENT_DND_MOTION = ecore_event_type_new();
        ECORE_WL2_EVENT_DND_DROP = ecore_event_type_new();
        ECORE_WL2_EVENT_DND_END = ecore_event_type_new();
        ECORE_WL2_EVENT_DATA_SOURCE_END = ecore_event_type_new();
        ECORE_WL2_EVENT_DATA_SOURCE_DROP = ecore_event_type_new();
        ECORE_WL2_EVENT_DATA_SOURCE_ACTION = ecore_event_type_new();
        ECORE_WL2_EVENT_DATA_SOURCE_TARGET = ecore_event_type_new();
        ECORE_WL2_EVENT_DATA_SOURCE_SEND = ecore_event_type_new();
        ECORE_WL2_EVENT_WINDOW_CONFIGURE = ecore_event_type_new();
        ECORE_WL2_EVENT_SYNC_DONE = ecore_event_type_new();
        ECORE_WL2_EVENT_OFFER_DATA_READY = ecore_event_type_new();
        ECORE_WL2_EVENT_SEAT_NAME_CHANGED = ecore_event_type_new();
        ECORE_WL2_EVENT_SEAT_CAPABILITIES_CHANGED = ecore_event_type_new();
        ECORE_WL2_EVENT_DEVICE_ADDED = ecore_event_type_new();
        ECORE_WL2_EVENT_DEVICE_REMOVED = ecore_event_type_new();
        _ecore_wl2_event_window_www = ecore_event_type_new();
        _ecore_wl2_event_window_www_drag = ecore_event_type_new();
        ECORE_WL2_EVENT_WINDOW_CONFIGURE_COMPLETE = ecore_event_type_new();
        ECORE_WL2_EVENT_SEAT_KEYMAP_CHANGED = ecore_event_type_new();
        ECORE_WL2_EVENT_SEAT_KEYBOARD_REPEAT_CHANGED = ecore_event_type_new();
        ECORE_WL2_EVENT_SEAT_SELECTION = ecore_event_type_new();
        ECORE_WL2_EVENT_OUTPUT_TRANSFORM = ecore_event_type_new();
        ECORE_WL2_EVENT_WINDOW_ROTATE = ecore_event_type_new();
        ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE = ecore_event_type_new();
        ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE_DONE =
          ecore_event_type_new();
        ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_REQUEST = ecore_event_type_new();
        ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_DONE = ecore_event_type_new();
        ECORE_WL2_EVENT_AUX_HINT_ALLOWED = ecore_event_type_new();
     }
   if (!no_session_recovery)
     no_session_recovery = !!getenv("EFL_NO_WAYLAND_SESSION_RECOVERY");

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
   ecore_event_type_flush(ECORE_WL2_EVENT_CONNECT,
                          ECORE_WL2_EVENT_DISCONNECT,
                          ECORE_WL2_EVENT_GLOBAL_ADDED,
                          ECORE_WL2_EVENT_GLOBAL_REMOVED,
                          ECORE_WL2_EVENT_FOCUS_IN,
                          ECORE_WL2_EVENT_FOCUS_OUT,
                          ECORE_WL2_EVENT_DND_ENTER,
                          ECORE_WL2_EVENT_DND_LEAVE,
                          ECORE_WL2_EVENT_DND_MOTION,
                          ECORE_WL2_EVENT_DND_DROP,
                          ECORE_WL2_EVENT_DND_END,
                          ECORE_WL2_EVENT_DATA_SOURCE_END,
                          ECORE_WL2_EVENT_DATA_SOURCE_DROP,
                          ECORE_WL2_EVENT_DATA_SOURCE_ACTION,
                          ECORE_WL2_EVENT_DATA_SOURCE_TARGET,
                          ECORE_WL2_EVENT_DATA_SOURCE_SEND,
                          ECORE_WL2_EVENT_WINDOW_CONFIGURE,
                          ECORE_WL2_EVENT_SYNC_DONE,
                          ECORE_WL2_EVENT_OFFER_DATA_READY,
                          ECORE_WL2_EVENT_SEAT_NAME_CHANGED,
                          ECORE_WL2_EVENT_SEAT_CAPABILITIES_CHANGED,
                          ECORE_WL2_EVENT_DEVICE_ADDED,
                          ECORE_WL2_EVENT_DEVICE_REMOVED,
                          ECORE_WL2_EVENT_WINDOW_CONFIGURE_COMPLETE,
                          ECORE_WL2_EVENT_SEAT_KEYMAP_CHANGED,
                          ECORE_WL2_EVENT_SEAT_KEYBOARD_REPEAT_CHANGED,
                          ECORE_WL2_EVENT_SEAT_SELECTION,
                          ECORE_WL2_EVENT_OUTPUT_TRANSFORM,
                          ECORE_WL2_EVENT_WINDOW_ROTATE,
                          ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE,
                          ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE_DONE,
                          ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_REQUEST,
                          ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_DONE,
                          ECORE_WL2_EVENT_AUX_HINT_ALLOWED);

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

EAPI void
ecore_wl2_session_recovery_disable(void)
{
   no_session_recovery = EINA_TRUE;
}

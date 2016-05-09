#include "elput_private.h"

/* local variables */
static int _elput_init_count = 0;

/* external variables */
int _elput_log_dom = -1;

EAPI int ELPUT_EVENT_SEAT_CAPS = -1;
EAPI int ELPUT_EVENT_SEAT_FRAME = -1;
EAPI int ELPUT_EVENT_KEYMAP_SEND = -1;
EAPI int ELPUT_EVENT_MODIFIERS_SEND = -1;
EAPI int ELPUT_EVENT_DEVICE_CHANGE = -1;
EAPI int ELPUT_EVENT_SESSION_ACTIVE = -1;

EAPI int
elput_init(void)
{
   if (++_elput_init_count != 1) return _elput_init_count;

   if (!eina_init()) goto eina_err;
   if (!ecore_init()) goto ecore_err;
   if (!ecore_event_init()) goto ecore_event_err;
   if (!eeze_init()) goto eeze_err;

   _elput_log_dom = eina_log_domain_register("elput", ELPUT_DEFAULT_LOG_COLOR);
   if (!_elput_log_dom)
     {
        EINA_LOG_ERR("Could not create logging domain for Elput");
        goto log_err;
     }

   ELPUT_EVENT_SEAT_CAPS = ecore_event_type_new();
   ELPUT_EVENT_SEAT_FRAME = ecore_event_type_new();
   ELPUT_EVENT_KEYMAP_SEND = ecore_event_type_new();
   ELPUT_EVENT_MODIFIERS_SEND = ecore_event_type_new();
   ELPUT_EVENT_DEVICE_CHANGE = ecore_event_type_new();
   ELPUT_EVENT_SESSION_ACTIVE = ecore_event_type_new();

   return _elput_init_count;

log_err:
   eeze_shutdown();
eeze_err:
   ecore_event_shutdown();
ecore_event_err:
   ecore_shutdown();
ecore_err:
   eina_shutdown();
eina_err:
   return --_elput_init_count;
}

EAPI int
elput_shutdown(void)
{
   if (_elput_init_count < 1) return 0;
   if (--_elput_init_count != 0) return _elput_init_count;

   ELPUT_EVENT_SEAT_CAPS = -1;
   ELPUT_EVENT_SEAT_FRAME = -1;
   ELPUT_EVENT_KEYMAP_SEND = -1;
   ELPUT_EVENT_MODIFIERS_SEND = -1;
   ELPUT_EVENT_DEVICE_CHANGE = -1;
   ELPUT_EVENT_SESSION_ACTIVE = -1;

   eina_log_domain_unregister(_elput_log_dom);
   _elput_log_dom = -1;

   eeze_shutdown();
   ecore_event_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return _elput_init_count;
}

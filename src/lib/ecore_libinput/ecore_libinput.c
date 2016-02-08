#include "ecore_libinput_private.h"

static int _ecore_libinput_init_count = 0;

int _ecore_libinput_log_dom = -1;

EAPI int
ecore_libinput_init(void)
{
   if (++_ecore_libinput_init_count != 1) return _ecore_libinput_init_count;

   if (!eina_init()) goto eina_err;

   _ecore_libinput_log_dom =
     eina_log_domain_register("ecore_libinput", ECORE_LIBINPUT_COLOR_DEFAULT);
   if (!_ecore_libinput_log_dom)
     {
        EINA_LOG_ERR("Could not create logging domain");
        goto log_err;
     }

   if (!ecore_event_init())
     {
        ERR("Could not init Ecore_Event");
        goto ecore_event_err;
     }

   if (!eeze_init())
     {
        ERR("Could not init Eeze");
        goto eeze_err;
     }

   return _ecore_libinput_init_count;

eeze_err:
   ecore_event_shutdown();
ecore_event_err:
   eina_log_domain_unregister(_ecore_libinput_log_dom);
   _ecore_libinput_log_dom = -1;
log_err:
   eina_shutdown();
eina_err:
   return --_ecore_libinput_init_count;
}

EAPI int
ecore_libinput_shutdown(void)
{
   if (--_ecore_libinput_init_count != 0) return _ecore_libinput_init_count;

   eeze_shutdown();
   ecore_event_shutdown();

   eina_log_domain_unregister(_ecore_libinput_log_dom);
   _ecore_libinput_log_dom = -1;

   eina_shutdown();
   return _ecore_libinput_init_count;
}

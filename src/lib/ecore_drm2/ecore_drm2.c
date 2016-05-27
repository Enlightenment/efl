#include "ecore_drm2_private.h"

static int _ecore_drm2_init_count = 0;

int _ecore_drm2_log_dom = -1;

EAPI int ECORE_DRM2_EVENT_OUTPUT_CHANGED = -1;
EAPI int ECORE_DRM2_EVENT_ACTIVATE = -1;

EAPI int
ecore_drm2_init(void)
{
   if (++_ecore_drm2_init_count != 1) return _ecore_drm2_init_count;

   if (!eina_init()) goto eina_err;

   if (!ecore_init())
     {
        EINA_LOG_ERR("Could not initialize Ecore library");
        goto ecore_err;
     }

   if (!eeze_init())
     {
        EINA_LOG_ERR("Could not initialize Eeze library");
        goto eeze_err;
     }

   if (!elput_init())
     {
        EINA_LOG_ERR("Could not initialize Elput library");
        goto elput_err;
     }

   _ecore_drm2_log_dom =
     eina_log_domain_register("ecore_drm2", ECORE_DRM2_DEFAULT_LOG_COLOR);
   if (!_ecore_drm2_log_dom)
     {
        EINA_LOG_ERR("Could not create logging domain for Ecore_Drm2");
        goto log_err;
     }

   ECORE_DRM2_EVENT_OUTPUT_CHANGED = ecore_event_type_new();
   ECORE_DRM2_EVENT_ACTIVATE = ecore_event_type_new();

   return _ecore_drm2_init_count;

log_err:
   elput_shutdown();
elput_err:
   eeze_shutdown();
eeze_err:
   ecore_shutdown();
ecore_err:
   eina_shutdown();
eina_err:
   return --_ecore_drm2_init_count;
}

EAPI int
ecore_drm2_shutdown(void)
{
   if (_ecore_drm2_init_count < 1)
     {
        ERR("Ecore_Drm2 shutdown called without init");
        return 0;
     }

   if (--_ecore_drm2_init_count != 0) return _ecore_drm2_init_count;

   ECORE_DRM2_EVENT_OUTPUT_CHANGED = -1;
   ECORE_DRM2_EVENT_ACTIVATE = -1;

   eina_log_domain_unregister(_ecore_drm2_log_dom);
   _ecore_drm2_log_dom = -1;

   elput_shutdown();
   eeze_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return _ecore_drm2_init_count;
}

#include "ecore_drm2_private.h"

static int _ecore_drm2_init_count = 0;
static void *drm_lib = NULL;

int _ecore_drm2_log_dom = -1;

EAPI int ECORE_DRM2_EVENT_OUTPUT_CHANGED = -1;
EAPI int ECORE_DRM2_EVENT_ACTIVATE = -1;

static Eina_Bool
_ecore_drm2_link(void)
{
   int i, fail;
   const char *drm_libs[] =
     {
        "libdrm.so.2",
        "libdrm.so.1",
        "libdrm.so.0",
        "libdrm.so",
        NULL,
     };

#define SYM(lib, xx)                         \
   do {                                      \
      sym_ ## xx = dlsym(lib, #xx);          \
      if (!(sym_ ## xx)) {                   \
         fail = 1;                           \
      }                                      \
   } while (0)

   if (drm_lib) return EINA_TRUE;

   for (i = 0; drm_libs[i]; i++)
     {
        drm_lib = dlopen(drm_libs[i], RTLD_LOCAL | RTLD_LAZY);
        if (!drm_lib) continue;

        fail = 0;

        SYM(drm_lib, drmIoctl);
        /* SYM(drm_lib, drmClose); */
        /* SYM(drm_lib, drmWaitVBlank); */
        SYM(drm_lib, drmHandleEvent);
        SYM(drm_lib, drmGetVersion);
        SYM(drm_lib, drmFreeVersion);
        SYM(drm_lib, drmModeGetProperty);
        SYM(drm_lib, drmModeFreeProperty);
        SYM(drm_lib, drmModeGetPropertyBlob);
        SYM(drm_lib, drmModeFreePropertyBlob);
        SYM(drm_lib, drmModeDestroyPropertyBlob);
        SYM(drm_lib, drmModeObjectGetProperties);
        SYM(drm_lib, drmModeFreeObjectProperties);
        SYM(drm_lib, drmModeCreatePropertyBlob);
#ifdef HAVE_ATOMIC_DRM
        SYM(drm_lib, drmModeAtomicAlloc);
        SYM(drm_lib, drmModeAtomicFree);
        SYM(drm_lib, drmModeAtomicAddProperty);
        SYM(drm_lib, drmModeAtomicCommit);
        SYM(drm_lib, drmModeAtomicSetCursor);
#endif
        SYM(drm_lib, drmModeGetEncoder);
        SYM(drm_lib, drmModeFreeEncoder);
        SYM(drm_lib, drmModeGetCrtc);
        SYM(drm_lib, drmModeFreeCrtc);
        SYM(drm_lib, drmModeSetCrtc);
        SYM(drm_lib, drmModeGetResources);
        SYM(drm_lib, drmModeFreeResources);
        SYM(drm_lib, drmModeGetConnector);
        SYM(drm_lib, drmModeFreeConnector);
        SYM(drm_lib, drmModeConnectorSetProperty);
        SYM(drm_lib, drmGetCap);
        SYM(drm_lib, drmSetClientCap);
        SYM(drm_lib, drmModeGetPlaneResources);
        SYM(drm_lib, drmModeFreePlaneResources);
        SYM(drm_lib, drmModeGetPlane);
        SYM(drm_lib, drmModeFreePlane);
        SYM(drm_lib, drmModeAddFB);
        SYM(drm_lib, drmModeRmFB);
        SYM(drm_lib, drmModeDirtyFB);

        if (fail)
          {
             dlclose(drm_lib);
             drm_lib = NULL;
          }
        else
          break;
     }

   if (!drm_lib) return EINA_FALSE;
   return EINA_TRUE;
}

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

   if (!_ecore_drm2_link()) goto link_err;

   return _ecore_drm2_init_count;

link_err:
   eina_log_domain_unregister(_ecore_drm2_log_dom);
   _ecore_drm2_log_dom = -1;
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

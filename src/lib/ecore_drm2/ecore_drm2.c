#include "ecore_drm2_private.h"

/* local variables */
static int _ecore_drm2_init_count = 0;
static void *_drm_lib = NULL;

/* external variables */
int _ecore_drm2_log_dom = -1;

/* external drm function prototypes (for dlopen) */
int (*sym_drmIoctl)(int fd, unsigned long request, void *arg) = NULL;
void *(*sym_drmModeGetResources)(int fd) = NULL;
void (*sym_drmModeFreeResources)(drmModeResPtr ptr) = NULL;
int (*sym_drmGetCap)(int fd, uint64_t capability, uint64_t *value);
int (*sym_drmSetClientCap)(int fd, uint64_t capability, uint64_t value) = NULL;
void *(*sym_drmModeGetProperty)(int fd, uint32_t propertyId);
void (*sym_drmModeFreeProperty)(drmModePropertyPtr ptr);
void *(*sym_drmModeGetPropertyBlob)(int fd, uint32_t blob_id);
void (*sym_drmModeFreePropertyBlob)(drmModePropertyBlobPtr ptr);
int (*sym_drmModeCreatePropertyBlob)(int fd, const void *data, size_t size, uint32_t *id);
int (*sym_drmModeDestroyPropertyBlob)(int fd, uint32_t id);
void *(*sym_drmModeObjectGetProperties)(int fd, uint32_t object_id, uint32_t object_type);
void (*sym_drmModeFreeObjectProperties)(drmModeObjectPropertiesPtr ptr);
void *(*sym_drmModeGetPlaneResources)(int fd);
void (*sym_drmModeFreePlaneResources)(drmModePlaneResPtr ptr);
void *(*sym_drmModeGetPlane)(int fd, uint32_t plane_id);
void (*sym_drmModeFreePlane)(drmModePlanePtr ptr);
void *(*sym_drmModeGetConnector)(int fd, uint32_t connectorId);
void (*sym_drmModeFreeConnector)(drmModeConnectorPtr ptr);
void *(*sym_drmModeGetEncoder)(int fd, uint32_t encoder_id);
void (*sym_drmModeFreeEncoder)(drmModeEncoderPtr ptr);
void *(*sym_drmModeGetCrtc)(int fd, uint32_t crtcId);
void (*sym_drmModeFreeCrtc)(drmModeCrtcPtr ptr);
void *(*sym_drmModeAtomicAlloc)(void) = NULL;
void (*sym_drmModeAtomicFree)(drmModeAtomicReqPtr req) = NULL;
int (*sym_drmModeAtomicAddProperty)(drmModeAtomicReqPtr req, uint32_t object_id, uint32_t property_id, uint64_t value) = NULL;
int (*sym_drmModeAtomicCommit)(int fd, drmModeAtomicReqPtr req, uint32_t flags, void *user_data) = NULL;
void (*sym_drmModeAtomicSetCursor)(drmModeAtomicReqPtr req, int cursor) = NULL;
int (*sym_drmWaitVBlank)(int fd, drmVBlank *vbl) = NULL;
int (*sym_drmModeAddFB)(int fd, uint32_t width, uint32_t height, uint8_t depth, uint8_t bpp, uint32_t pitch, uint32_t bo_handle, uint32_t *buf_id);
int (*sym_drmModeAddFB2)(int fd, uint32_t width, uint32_t height, uint32_t pixel_format, uint32_t bo_handles[4], uint32_t pitches[4], uint32_t offsets[4], uint32_t *buf_id, uint32_t flags);
int (*sym_drmModeRmFB)(int fd, uint32_t bufferId);
int (*sym_drmModePageFlip)(int fd, uint32_t crtc_id, uint32_t fb_id, uint32_t flags, void *user_data);
int (*sym_drmModeDirtyFB)(int fd, uint32_t bufferId, drmModeClipPtr clips, uint32_t num_clips);
int (*sym_drmModeCrtcSetGamma)(int fd, uint32_t crtc_id, uint32_t size, uint16_t *red, uint16_t *green, uint16_t *blue);
int (*sym_drmModeConnectorSetProperty)(int fd, uint32_t connector_id, uint32_t property_id, uint64_t value);
int (*sym_drmModeSetPlane)(int fd, uint32_t plane_id, uint32_t crtc_id, uint32_t fb_id, uint32_t flags, int32_t crtc_x, int32_t crtc_y, uint32_t crtc_w, uint32_t crtc_h, uint32_t src_x, uint32_t src_y, uint32_t src_w, uint32_t src_h);
bool (*sym_drmModeFormatModifierBlobIterNext)(const drmModePropertyBlobRes *blob, drmModeFormatModifierIterator *iter);
int (*sym_drmHandleEvent)(int fd, drmEventContextPtr evctx);
int (*sym_drmModeSetCrtc)(int fd, uint32_t crtcId, uint32_t bufferId, uint32_t x, uint32_t y, uint32_t *connectors, int count, drmModeModeInfoPtr mode);

EAPI int ECORE_DRM2_EVENT_ACTIVATE = -1;
EAPI int ECORE_DRM2_EVENT_DISPLAY_CHANGED = -1;

/* local static functions */
static Eina_Bool
_ecore_drm2_link(void)
{
   int i;
   Eina_Bool fail = EINA_FALSE;
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
         fail = EINA_TRUE;                   \
      }                                      \
   } while (0)

   if (_drm_lib) return EINA_TRUE;

   for (i = 0; drm_libs[i]; i++)
     {
        _drm_lib = dlopen(drm_libs[i], RTLD_LOCAL | RTLD_LAZY);
        if (!_drm_lib) continue;

        fail = EINA_FALSE;

        /* TODO: Sym needed libdrm functions */
	SYM(_drm_lib, drmIoctl);
        SYM(_drm_lib, drmModeGetResources);
        SYM(_drm_lib, drmModeFreeResources);
        SYM(_drm_lib, drmGetCap);
        SYM(_drm_lib, drmSetClientCap);
        SYM(_drm_lib, drmModeGetProperty);
        SYM(_drm_lib, drmModeFreeProperty);
        SYM(_drm_lib, drmModeGetPropertyBlob);
        SYM(_drm_lib, drmModeFreePropertyBlob);
        SYM(_drm_lib, drmModeCreatePropertyBlob);
        SYM(_drm_lib, drmModeDestroyPropertyBlob);
        SYM(_drm_lib, drmModeObjectGetProperties);
        SYM(_drm_lib, drmModeFreeObjectProperties);
        SYM(_drm_lib, drmModeGetPlaneResources);
        SYM(_drm_lib, drmModeFreePlaneResources);
        SYM(_drm_lib, drmModeGetPlane);
        SYM(_drm_lib, drmModeFreePlane);
        SYM(_drm_lib, drmModeGetConnector);
        SYM(_drm_lib, drmModeFreeConnector);
        SYM(_drm_lib, drmModeGetEncoder);
        SYM(_drm_lib, drmModeFreeEncoder);
        SYM(_drm_lib, drmModeGetCrtc);
        SYM(_drm_lib, drmModeFreeCrtc);
        SYM(_drm_lib, drmModeAtomicAlloc);
        SYM(_drm_lib, drmModeAtomicFree);
        SYM(_drm_lib, drmModeAtomicAddProperty);
        SYM(_drm_lib, drmModeAtomicCommit);
        SYM(_drm_lib, drmModeAtomicSetCursor);
        SYM(_drm_lib, drmWaitVBlank);
	SYM(_drm_lib, drmModeAddFB);
	SYM(_drm_lib, drmModeAddFB2);
	SYM(_drm_lib, drmModeRmFB);
	SYM(_drm_lib, drmModeAddFB);
	SYM(_drm_lib, drmModePageFlip);
	SYM(_drm_lib, drmModeDirtyFB);
	SYM(_drm_lib, drmModeCrtcSetGamma);
	SYM(_drm_lib, drmModeConnectorSetProperty);
        SYM(_drm_lib, drmModeSetPlane);
        SYM(_drm_lib, drmModeFormatModifierBlobIterNext);
        SYM(_drm_lib, drmHandleEvent);
        SYM(_drm_lib, drmModeSetCrtc);

        if (fail)
          {
             dlclose(_drm_lib);
             _drm_lib = NULL;
          }
        else
          break;
     }

   if (!_drm_lib) return EINA_FALSE;
   return EINA_TRUE;
}

/* API functions */
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

   ECORE_DRM2_EVENT_ACTIVATE = ecore_event_type_new();
   ECORE_DRM2_EVENT_DISPLAY_CHANGED = ecore_event_type_new();

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

   if (_drm_lib) dlclose(_drm_lib);

   ECORE_DRM2_EVENT_ACTIVATE = -1;
   ECORE_DRM2_EVENT_DISPLAY_CHANGED = -1;

   eina_log_domain_unregister(_ecore_drm2_log_dom);
   _ecore_drm2_log_dom = -1;

   elput_shutdown();
   eeze_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return _ecore_drm2_init_count;
}

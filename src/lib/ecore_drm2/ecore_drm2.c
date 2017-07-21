#include "ecore_drm2_private.h"

static int _ecore_drm2_init_count = 0;
static void *drm_lib = NULL;

int _ecore_drm2_log_dom = -1;

int (*sym_drmHandleEvent)(int fd, drmEventContext *evctx) = NULL;
void *(*sym_drmGetVersion)(int fd) = NULL;
void (*sym_drmFreeVersion)(void *drmver) = NULL;
void *(*sym_drmModeGetProperty)(int fd, uint32_t propertyId) = NULL;
void (*sym_drmModeFreeProperty)(drmModePropertyPtr ptr) = NULL;
void *(*sym_drmModeGetPropertyBlob)(int fd, uint32_t blob_id) = NULL;
void (*sym_drmModeFreePropertyBlob)(drmModePropertyBlobPtr ptr) = NULL;
int (*sym_drmModeDestroyPropertyBlob)(int fd, uint32_t id) = NULL;
int (*sym_drmIoctl)(int fd, unsigned long request, void *arg) = NULL;
void *(*sym_drmModeObjectGetProperties)(int fd, uint32_t object_id, uint32_t object_type) = NULL;
void (*sym_drmModeFreeObjectProperties)(drmModeObjectPropertiesPtr ptr) = NULL;
int (*sym_drmModeCreatePropertyBlob)(int fd, const void *data, size_t size, uint32_t *id) = NULL;
void *(*sym_drmModeAtomicAlloc)(void) = NULL;
void (*sym_drmModeAtomicFree)(drmModeAtomicReqPtr req) = NULL;
int (*sym_drmModeAtomicAddProperty)(drmModeAtomicReqPtr req, uint32_t object_id, uint32_t property_id, uint64_t value) = NULL;
int (*sym_drmModeAtomicCommit)(int fd, drmModeAtomicReqPtr req, uint32_t flags, void *user_data) = NULL;
void (*sym_drmModeAtomicSetCursor)(drmModeAtomicReqPtr req, int cursor) = NULL;
int (*sym_drmModeAtomicMerge)(drmModeAtomicReqPtr base, drmModeAtomicReqPtr augment);
void *(*sym_drmModeGetEncoder)(int fd, uint32_t encoder_id) = NULL;
void (*sym_drmModeFreeEncoder)(drmModeEncoderPtr ptr) = NULL;
void *(*sym_drmModeGetCrtc)(int fd, uint32_t crtcId) = NULL;
void (*sym_drmModeFreeCrtc)(drmModeCrtcPtr ptr) = NULL;
int (*sym_drmModeSetCrtc)(int fd, uint32_t crtcId, uint32_t bufferId, uint32_t x, uint32_t y, uint32_t *connectors, int count, drmModeModeInfoPtr mode) = NULL;
void *(*sym_drmModeGetResources)(int fd) = NULL;
void (*sym_drmModeFreeResources)(drmModeResPtr ptr) = NULL;
void *(*sym_drmModeGetConnector)(int fd, uint32_t connectorId) = NULL;
void (*sym_drmModeFreeConnector)(drmModeConnectorPtr ptr) = NULL;
int (*sym_drmModeConnectorSetProperty)(int fd, uint32_t connector_id, uint32_t property_id, uint64_t value) = NULL;
int (*sym_drmGetCap)(int fd, uint64_t capability, uint64_t *value) = NULL;
int (*sym_drmSetClientCap)(int fd, uint64_t capability, uint64_t value) = NULL;
void *(*sym_drmModeGetPlaneResources)(int fd) = NULL;
void (*sym_drmModeFreePlaneResources)(drmModePlaneResPtr ptr) = NULL;
void *(*sym_drmModeGetPlane)(int fd, uint32_t plane_id) = NULL;
void (*sym_drmModeFreePlane)(drmModePlanePtr ptr) = NULL;
int (*sym_drmModeAddFB)(int fd, uint32_t width, uint32_t height, uint8_t depth, uint8_t bpp, uint32_t pitch, uint32_t bo_handle, uint32_t *buf_id) = NULL;
int (*sym_drmModeAddFB2)(int fd, uint32_t width, uint32_t height, uint32_t pixel_format, uint32_t bo_handles[4], uint32_t pitches[4], uint32_t offsets[4], uint32_t *buf_id, uint32_t flags) = NULL;
int (*sym_drmModeRmFB)(int fd, uint32_t bufferId) = NULL;
int (*sym_drmModePageFlip)(int fd, uint32_t crtc_id, uint32_t fb_id, uint32_t flags, void *user_data) = NULL;
int (*sym_drmModeDirtyFB)(int fd, uint32_t bufferId, drmModeClipPtr clips, uint32_t num_clips) = NULL;
int (*sym_drmModeCrtcSetGamma)(int fd, uint32_t crtc_id, uint32_t size, uint16_t *red, uint16_t *green, uint16_t *blue) = NULL;
int (*sym_drmPrimeFDToHandle)(int fd, int prime_fd, uint32_t *handle) = NULL;
int (*sym_drmWaitVBlank)(int fd, drmVBlank *vbl) = NULL;

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
        SYM(drm_lib, drmWaitVBlank);
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
        SYM(drm_lib, drmModeAtomicAlloc);
        SYM(drm_lib, drmModeAtomicFree);
        SYM(drm_lib, drmModeAtomicAddProperty);
        SYM(drm_lib, drmModeAtomicCommit);
        SYM(drm_lib, drmModeAtomicSetCursor);
        SYM(drm_lib, drmModeAtomicMerge);
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
        SYM(drm_lib, drmModeAddFB2);
        SYM(drm_lib, drmModeRmFB);
        SYM(drm_lib, drmModePageFlip);
        SYM(drm_lib, drmModeDirtyFB);
        SYM(drm_lib, drmModeCrtcSetGamma);
        SYM(drm_lib, drmPrimeFDToHandle);

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

EAPI int
ecore_drm2_event_handle(Ecore_Drm2_Device *dev, Ecore_Drm2_Context *drmctx)
{
   drmEventContext ctx;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, -1);

   memset(&ctx, 0, sizeof(ctx));
   ctx.version = 2;
   ctx.page_flip_handler = drmctx->page_flip_handler;
   ctx.vblank_handler = drmctx->vblank_handler;

   return sym_drmHandleEvent(dev->fd, &ctx);
}

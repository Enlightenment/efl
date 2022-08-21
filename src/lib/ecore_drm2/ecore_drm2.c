#include "ecore_drm2_private.h"

/* local variables */
static int _ecore_drm2_init_count = 0;
static void *_drm_lib = NULL;

/* external variables */
int _ecore_drm2_log_dom = -1;

/* external drm function prototypes (for dlopen) */
void *(*sym_drmModeGetResources)(int fd) = NULL;
void (*sym_drmModeFreeResources)(drmModeResPtr ptr) = NULL;
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
        SYM(_drm_lib, drmModeGetResources);
        SYM(_drm_lib, drmModeFreeResources);
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

   eina_log_domain_unregister(_ecore_drm2_log_dom);
   _ecore_drm2_log_dom = -1;

   elput_shutdown();
   eeze_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return _ecore_drm2_init_count;
}

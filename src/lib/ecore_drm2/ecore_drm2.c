#include "ecore_drm2_private.h"

/* local variables */
static int _ecore_drm2_init_count = 0;
static void *_drm_lib = NULL;

/* external variables */
int _ecore_drm2_log_dom = -1;

/* external drm function prototypes (for dlopen) */
void *(*sym_drmModeGetResources)(int fd) = NULL;
void (*sym_drmModeFreeResources)(drmModeResPtr ptr) = NULL;

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

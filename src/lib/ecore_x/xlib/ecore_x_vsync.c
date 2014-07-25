#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ECORE_X_VSYNC_DRI2 1

#ifdef ECORE_X_VSYNC_DRI2
// relevant header bits of dri/drm inlined here to avoid needing external
// headers to build
/// drm
typedef unsigned int drm_magic_t;

typedef enum
{
   DRM_VBLANK_ABSOLUTE = 0x00000000,
   DRM_VBLANK_RELATIVE = 0x00000001,
   DRM_VBLANK_EVENT = 0x04000000,
   DRM_VBLANK_FLIP = 0x08000000,
   DRM_VBLANK_NEXTONMISS = 0x10000000,
   DRM_VBLANK_SECONDARY = 0x20000000,
   DRM_VBLANK_SIGNAL = 0x40000000
}
drmVBlankSeqType;

typedef struct _drmVBlankReq
{
   drmVBlankSeqType type;
   unsigned int     sequence;
   unsigned long    signal;
} drmVBlankReq;

typedef struct _drmVBlankReply
{
   drmVBlankSeqType type;
   unsigned int     sequence;
   long             tval_sec;
   long             tval_usec;
} drmVBlankReply;

typedef union _drmVBlank
{
   drmVBlankReq   request;
   drmVBlankReply reply;
} drmVBlank;

#define DRM_EVENT_CONTEXT_VERSION 2

typedef struct _drmEventContext
{
   int version;
   void (*vblank_handler)(int fd,
                          unsigned int sequence,
                          unsigned int tv_sec,
                          unsigned int tv_usec,
                          void *user_data);
   void (*page_flip_handler)(int fd,
                             unsigned int sequence,
                             unsigned int tv_sec,
                             unsigned int tv_usec,
                             void *user_data);
} drmEventContext;

static int (*sym_drmClose)(int fd) = NULL;
static int (*sym_drmGetMagic)(int fd,
                              drm_magic_t *magic) = NULL;
static int (*sym_drmWaitVBlank)(int fd,
                                drmVBlank *vbl) = NULL;
static int (*sym_drmHandleEvent)(int fd,
                                 drmEventContext *evctx) = NULL;

//// dri

static Bool (*sym_DRI2QueryExtension)(Display *display,
                                      int *eventBase,
                                      int *errorBase) = NULL;
static Bool (*sym_DRI2QueryVersion)(Display *display,
                                    int *major,
                                    int *minor) = NULL;
static Bool (*sym_DRI2Connect)(Display *display,
                               XID window,
                               char **driverName,
                               char **deviceName) = NULL;
static Bool (*sym_DRI2Authenticate)(Display *display,
                                    XID window,
                                    drm_magic_t magic) = NULL;

//// dri/drm data needed
static int dri2_event = 0;
static int dri2_error = 0;
static int dri2_major = 0;
static int dri2_minor = 0;
static char *device_name = 0;
static char *driver_name = 0;
static drm_magic_t drm_magic;

static int drm_fd = -1;
static int drm_event_is_busy = 0;
static int drm_animators_interval = 1;
static drmEventContext drm_evctx;
static Ecore_Fd_Handler *dri_drm_fdh = NULL;

static void *dri_lib = NULL;
static void *drm_lib = NULL;

static Window dri_drm_vsync_root = 0;

static Eina_Bool
_dri_drm_tick_schedule(void)
{
   drmVBlank vbl;

   vbl.request.type = DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT;
   vbl.request.sequence = drm_animators_interval;
   vbl.request.signal = 0;
   if (sym_drmWaitVBlank(drm_fd, &vbl) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

static void
_dri_drm_tick_begin(void *data EINA_UNUSED)
{
   drm_event_is_busy = 1;
   _dri_drm_tick_schedule();
}

static void
_dri_drm_tick_end(void *data EINA_UNUSED)
{
   drm_event_is_busy = 0;
}

static void
_dri_drm_vblank_handler(int fd EINA_UNUSED,
                        unsigned int frame EINA_UNUSED,
                        unsigned int sec EINA_UNUSED,
                        unsigned int usec EINA_UNUSED,
                        void *data EINA_UNUSED)
{
   if (drm_event_is_busy)
     {
        ecore_animator_custom_tick();
        _dri_drm_tick_schedule();
     }
}

static Eina_Bool
_dri_drm_cb(void *data EINA_UNUSED,
            Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   sym_drmHandleEvent(drm_fd, &drm_evctx);
   return ECORE_CALLBACK_RENEW;
}

// yes. most evil. we dlopen libdrm and libGL etc. to manually find smbols
// so we can be as compatible as possible given the whole mess of the
// gl/dri/drm etc. world. and handle graceful failure at runtime not
// compile time
static int
_dri_drm_link(void)
{
   const char *drm_libs[] =
   {
      "libdrm.so.2",
      "libdrm.so.1",
      "libdrm.so.0",
      "libdrm.so",
      NULL,
   };
   const char *dri_libs[] =
   {
      "libdri2.so.2",
      "libdri2.so.1",
      "libdri2.so.0",
      "libdri2.so",
      "libGL.so.4",
      "libGL.so.3",
      "libGL.so.2",
      "libGL.so.1",
      "libGL.so.0",
      "libGL.so",
      NULL,
   };
   int i, fail;
#define SYM(lib, xx)                            \
  do {                                          \
       sym_ ## xx = dlsym(lib, #xx);            \
       if (!(sym_ ## xx)) {                     \
            fail = 1;                           \
         }                                      \
    } while (0)

   if (dri_lib) return 1;
   for (i = 0; drm_libs[i]; i++)
     {
        drm_lib = dlopen(drm_libs[i], RTLD_LOCAL | RTLD_LAZY);
        if (drm_lib)
          {
             fail = 0;
             SYM(drm_lib, drmClose);
             SYM(drm_lib, drmWaitVBlank);
             SYM(drm_lib, drmHandleEvent);
             if (fail)
               {
                  dlclose(drm_lib);
                  drm_lib = NULL;
               }
             else break;
          }
     }
   if (!drm_lib) return 0;
   for (i = 0; dri_libs[i]; i++)
     {
        dri_lib = dlopen(dri_libs[i], RTLD_LOCAL | RTLD_LAZY);
        if (dri_lib)
          {
             fail = 0;
             SYM(dri_lib, DRI2QueryExtension);
             SYM(dri_lib, DRI2QueryVersion);
             SYM(dri_lib, DRI2Connect);
             SYM(dri_lib, DRI2Authenticate);
             if (fail)
               {
                  dlclose(dri_lib);
                  dri_lib = NULL;
               }
             else break;
          }
     }
   if (!dri_lib)
     {
        dlclose(drm_lib);
        drm_lib = NULL;
        return 0;
     }
   return 1;
}

static int
_dri_drm_init(void)
{
   if (!sym_DRI2QueryExtension(_ecore_x_disp, &dri2_event, &dri2_error))
     return 0;
   if (!sym_DRI2QueryVersion(_ecore_x_disp, &dri2_major, &dri2_minor))
     return 0;
   if (dri2_major < 2)
     return 0;
   if (!sym_DRI2Connect(_ecore_x_disp, dri_drm_vsync_root, &driver_name, &device_name))
     return 0;
   drm_fd = open(device_name, O_RDWR);
   if (drm_fd < 0)
     return 0;
   sym_drmGetMagic(drm_fd, &drm_magic);
   if (!sym_DRI2Authenticate(_ecore_x_disp, dri_drm_vsync_root, drm_magic))
     {
        close(drm_fd);
        drm_fd = -1;
        return 0;
     }
   memset(&drm_evctx, 0, sizeof(drm_evctx));
   drm_evctx.version = DRM_EVENT_CONTEXT_VERSION;
   drm_evctx.vblank_handler = _dri_drm_vblank_handler;
   drm_evctx.page_flip_handler = NULL;

   if (!_dri_drm_tick_schedule())
     {
        close(drm_fd);
        drm_fd = -1;
        return 0;
     }
   
   dri_drm_fdh = ecore_main_fd_handler_add(drm_fd, ECORE_FD_READ,
                                           _dri_drm_cb, NULL, NULL, NULL);
   if (!dri_drm_fdh)
     {
        close(drm_fd);
        drm_fd = -1;
        return 0;
     }
   return 1;
}

static void
_dri_drm_shutdown(void)
{
   if (drm_fd >= 0)
     {
        close(drm_fd);
        drm_fd = -1;
     }
   if (dri_drm_fdh)
     {
        ecore_main_fd_handler_del(dri_drm_fdh);
        dri_drm_fdh = NULL;
     }
}

#endif

EAPI Eina_Bool
ecore_x_vsync_animator_tick_source_set(Ecore_X_Window win)
{
#ifdef ECORE_X_VSYNC_DRI2
   Ecore_X_Window root;

   root = ecore_x_window_root_get(win);
   if (root != dri_drm_vsync_root)
     {
        dri_drm_vsync_root = root;
        if (dri_drm_vsync_root)
          {
             if (!_dri_drm_link())
               {
                  ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_TIMER);
                  return EINA_FALSE;
               }
             _dri_drm_shutdown();
             if (!_dri_drm_init())
               {
                  dri_drm_vsync_root = 0;
                  ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_TIMER);
                  return EINA_FALSE;
               }
             ecore_animator_custom_source_tick_begin_callback_set
               (_dri_drm_tick_begin, NULL);
             ecore_animator_custom_source_tick_end_callback_set
               (_dri_drm_tick_end, NULL);
             ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_CUSTOM);
          }
        else
          {
             if (drm_fd >= 0)
               {
                  _dri_drm_shutdown();
                  ecore_animator_custom_source_tick_begin_callback_set
                    (NULL, NULL);
                  ecore_animator_custom_source_tick_end_callback_set
                    (NULL, NULL);
                  ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_TIMER);
               }
          }
     }
   return EINA_TRUE;
#else
   return EINA_FALSE;
   win = 0;
#endif
}


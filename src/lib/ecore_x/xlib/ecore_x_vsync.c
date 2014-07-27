#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_Con.h"

#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ECORE_X_VSYNC_DRI2 1

static Ecore_X_Window vsync_root = 0;





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
        // XXX: set looptime
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
   if (!sym_DRI2Connect(_ecore_x_disp, vsync_root, &driver_name, &device_name))
     return 0;
   drm_fd = open(device_name, O_RDWR);
   if (drm_fd < 0)
     return 0;
   sym_drmGetMagic(drm_fd, &drm_magic);
   if (!sym_DRI2Authenticate(_ecore_x_disp, vsync_root, drm_magic))
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

static Eina_Bool
_dri_animator_tick_source_set(void)
{
   if (vsync_root)
     {
        if (!_dri_drm_link())
          {
             ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_TIMER);
             return EINA_FALSE;
          }
        _dri_drm_shutdown();
        if (!_dri_drm_init())
          {
             vsync_root = 0;
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
   return EINA_TRUE;
}
#endif









static Ecore_Con_Server *vsync_server = NULL;
static Eina_Bool handlers = EINA_FALSE;
static Eina_Prefix *_prefix = NULL;

static Eina_Bool
vsync_server_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Server_Add *ev = event;
   if (ev->server != vsync_server) return EINA_TRUE;
   return EINA_FALSE;
}

static Eina_Bool
vsync_server_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Server_Del *ev = event;
   if (ev->server != vsync_server) return EINA_TRUE;
   if (vsync_server)
     {
        ecore_con_server_del(vsync_server);
        vsync_server = NULL;
        ecore_animator_custom_source_tick_begin_callback_set(NULL, NULL);
        ecore_animator_custom_source_tick_end_callback_set(NULL, NULL);
        ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_TIMER);
     }
   return EINA_FALSE;
}

static Eina_Bool
vsync_server_data(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Server_Data *ev = event;
   int i;
   double t;
   char *d;
   if (ev->server != vsync_server) return EINA_TRUE;
   d = ev->data;;
   for (i = 0; i < ev->size - (int)(sizeof(double) - 1); i++)
     {
        memcpy(&t, &(d[i]), sizeof(double));
        ecore_loop_time_set(t);
        ecore_animator_custom_tick();
     }
   return EINA_FALSE;
}

static void
vsync_tick_begin(void *data EINA_UNUSED)
{
   char val = 1;
   ecore_con_server_send(vsync_server, &val, 1);
}

static void
vsync_tick_end(void *data EINA_UNUSED)
{
   char val = 0;
   ecore_con_server_send(vsync_server, &val, 1);
}

static Eina_Bool
_glvsync_animator_tick_source_set(void)
{
   if (!vsync_server)
     {
        char buf[4096], *disp, *s;
        int tries = 0;

        if (!handlers)
          {
             _prefix = eina_prefix_new(NULL, ecore_x_vsync_animator_tick_source_set,
                                       "ECORE_X", "ecore_x", "checkme",
                                       PACKAGE_BIN_DIR, PACKAGE_LIB_DIR,
                                       PACKAGE_DATA_DIR, PACKAGE_DATA_DIR);
             ecore_con_init();
          }
        disp = getenv("DISPLAY");
        if (disp) disp = ":0";
        snprintf(buf, sizeof(buf), "ecore-x-vsync-%s", disp);
        for (s = buf; *s; s++)
          {
             if (*s == ':') *s = '=';
          }
        vsync_server = ecore_con_server_connect(ECORE_CON_LOCAL_USER, buf, 1, NULL);
        while (!vsync_server)
          {
             tries++;
             if (tries > 50) return EINA_FALSE;
             snprintf(buf, sizeof(buf), "%s/ecore_x/bin/%s/ecore_x_vsync",
                      eina_prefix_lib_get(_prefix), MODULE_ARCH);
             ecore_exe_run(buf, NULL);
             usleep(10000);
             vsync_server = ecore_con_server_connect(ECORE_CON_LOCAL_USER, buf, 0, NULL);
          }
        if (!handlers)
          {
             ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD, vsync_server_add, NULL);
             ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL, vsync_server_del, NULL);
             ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA, vsync_server_data, NULL);
             handlers = EINA_FALSE;
          }
     }
   if (vsync_root)
     {
        ecore_animator_custom_source_tick_begin_callback_set(vsync_tick_begin, NULL);
        ecore_animator_custom_source_tick_end_callback_set(vsync_tick_end, NULL);
        ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_CUSTOM);
     }
   else
     {
        if (vsync_server)
          {
             ecore_con_server_del(vsync_server);
             vsync_server = NULL;
          }
        ecore_animator_custom_source_tick_begin_callback_set(NULL, NULL);
        ecore_animator_custom_source_tick_end_callback_set(NULL, NULL);
        ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_TIMER);
     }
   return EINA_TRUE;
}

// XXX: missing mode 3 == separate x connection with compiled in dri2 proto
// handling ala mesa (taken from mesa likely)

static int mode = 0;

static void
_vsync_init(void)
{
   static int done = 0;

   if (done) return;

#ifdef ECORE_X_VSYNC_DRI2
   // preferred inline dri/drm if possible
   if (_dri_drm_link())
     {
        _dri_drm_shutdown();
        if (_dri_drm_init())
          {
             mode = 1;
          }
     }
#endif
   // nvidia gl vsync slave mode
   if (mode == 0)
     {
        struct stat stb;

        if (!stat("/dev/nvidiactl", &stb))
          {
             mode = 2;
          }
     }
   done = 1;
}

EAPI Eina_Bool
ecore_x_vsync_animator_tick_source_set(Ecore_X_Window win)
{
   Ecore_X_Window root;

   root = ecore_x_window_root_get(win);
   if (root != vsync_root)
     {
        _vsync_init();
        vsync_root = root;
#ifdef ECORE_X_VSYNC_DRI2
        if (mode == 1) return _dri_animator_tick_source_set();
        else
#endif
        if (mode == 2) return _glvsync_animator_tick_source_set();
        else return EINA_FALSE;
     }
   return EINA_TRUE;
}

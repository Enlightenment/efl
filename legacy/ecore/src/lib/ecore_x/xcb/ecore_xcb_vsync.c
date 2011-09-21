#include "ecore_xcb_private.h"
# include <fcntl.h>
# include <dlfcn.h>
# include <X11/Xlib-xcb.h>

#define ECORE_XCB_VSYNC_DRI2 1
#define DRM_EVENT_CONTEXT_VERSION 2

#ifdef ECORE_XCB_VSYNC_DRI2

/* relevant header bits of dri/drm inlined here to avoid needing external */
/* headers to build drm */
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
} drmVBlankSeqType;

typedef struct _drmVBlankReq
{
   drmVBlankSeqType type;
   unsigned int sequence;
   unsigned long signal;
} drmVBlankReq;

typedef struct _drmVBlankReply
{
   drmVBlankSeqType type;
   unsigned int sequence;
   long tval_sec, tval_usec;
} drmVBlankReply;

typedef union _drmVBlank
{
   drmVBlankReq request;
   drmVBlankReply reply;
} drmVBlank;

typedef struct _drmEventContext
{
   int version;
   void (*vblank_handler)(int fd, unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec, void *user_data);
   void (*page_flip_handler)(int fd, unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec, void *user_data);
} drmEventContext;

static int (*sym_drmClose) (int fd) = NULL;
static int (*sym_drmGetMagic) (int fd, drm_magic_t * magic) = NULL;
static int (*sym_drmWaitVBlank) (int fd, drmVBlank *vbl) = NULL;
static int (*sym_drmHandleEvent) (int fd, drmEventContext *evctx) = NULL;

/* dri */
static Bool (*sym_DRI2QueryExtension) (Display *display, int *eventBase, int *errorBase) = NULL;
static Bool (*sym_DRI2QueryVersion) (Display *display, int *major, int *minor) = NULL;
static Bool (*sym_DRI2Connect) (Display *display, XID window, char **driverName, char **deviceName) = NULL;
static Bool (*sym_DRI2Authenticate) (Display *display, XID window, drm_magic_t magic) = NULL;

/* local function prototypes */
static Eina_Bool _ecore_xcb_dri_link(void);
static Eina_Bool _ecore_xcb_dri_start(void);
static void _ecore_xcb_dri_shutdown(void);

static Eina_Bool _ecore_xcb_dri_cb(void *data __UNUSED__, Ecore_Fd_Handler *fdh __UNUSED__);
static void _ecore_xcb_dri_tick_begin(void *data __UNUSED__);
static void _ecore_xcb_dri_tick_end(void *data __UNUSED__);
static void _ecore_xcb_dri_tick_schedule(void);
static void _ecore_xcb_dri_vblank_handler(int fd __UNUSED__, unsigned int frame __UNUSED__, unsigned int sec __UNUSED__, unsigned int usec __UNUSED__, void *data __UNUSED__);

/* local variables */
static Ecore_X_Window _vsync_root = 0;
static int _drm_fd = -1;
static Ecore_Fd_Handler *_drm_fdh = NULL;
static unsigned int _drm_magic = 0;
static Eina_Bool _drm_event_busy = EINA_FALSE;
static void *_drm_lib = NULL;
static void *_dri_lib = NULL;
static drmEventContext _drm_evctx;
#endif

void 
_ecore_xcb_dri_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
}

void 
_ecore_xcb_dri_finalize(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
}

EAPI Eina_Bool 
ecore_x_vsync_animator_tick_source_set(Ecore_X_Window win) 
{
#ifdef ECORE_XCB_VSYNC_DRI2
   Ecore_X_Window root;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_VSYNC_DRI2
   root = ecore_x_window_root_get(win);
   if (root != _vsync_root) 
     {
        _vsync_root = root;
        if (_vsync_root) 
          {
             if (!_ecore_xcb_dri_link()) 
               {
                  ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_TIMER);
                  return EINA_FALSE;
               }
             _ecore_xcb_dri_shutdown();
             if (!_ecore_xcb_dri_start()) 
               {
                  _vsync_root = 0;
                  ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_TIMER);
                  return EINA_FALSE;
               }
             ecore_animator_custom_source_tick_begin_callback_set
               (_ecore_xcb_dri_tick_begin, NULL);
             ecore_animator_custom_source_tick_end_callback_set
               (_ecore_xcb_dri_tick_end, NULL);
             ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_CUSTOM);
          }
        else 
          {
             if (_drm_fd >= 0) 
               {
                  _ecore_xcb_dri_shutdown();
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

/* local functions */
#ifdef ECORE_XCB_VSYNC_DRI2
static Eina_Bool 
_ecore_xcb_dri_link(void) 
{
   const char *_drm_libs[] = 
     {
        "libdrm.so.2", 
        "libdrm.so.1", 
        "libdrm.so.0", 
        "libdrm.so", 
        NULL,
     };
   const char *_dri_libs[] = 
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
   int i = 0, fail = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

# define SYM(lib, xx) \
   do { \
      sym_## xx = dlsym(lib, #xx); \
      if (!(sym_## xx)) { \
         fprintf(stderr, "%s\n", dlerror()); \
         fail = 1; \
      } \
   } while (0);

   if (_drm_lib) return EINA_TRUE;

   for (i = 0; _drm_libs[i]; i++) 
     {
        _drm_lib = dlopen(_drm_libs[i], (RTLD_LOCAL | RTLD_LAZY));
        if (_drm_lib) 
          {
             fail = 0;
             SYM(_drm_lib, drmClose);
             SYM(_drm_lib, drmGetMagic);
             SYM(_drm_lib, drmWaitVBlank);
             SYM(_drm_lib, drmHandleEvent);
             if (fail) 
               {
                  dlclose(_drm_lib);
                  _drm_lib = NULL;
               }
             else
               break;
          }
     }
   if (!_drm_lib) return EINA_FALSE;
   for (i = 0; _dri_libs[i]; i++) 
     {
        if ((_dri_lib = dlopen(_dri_libs[i], (RTLD_LOCAL | RTLD_LAZY)))) 
          {
             fail = 0;
             SYM(_dri_lib, DRI2QueryExtension);
             SYM(_dri_lib, DRI2QueryVersion);
             SYM(_dri_lib, DRI2Connect);
             SYM(_dri_lib, DRI2Authenticate);
             if (fail) 
               {
                  dlclose(_dri_lib);
                  _dri_lib = NULL;
               }
             else
               break;
          }
     }
   if (!_dri_lib) 
     {
        dlclose(_drm_lib);
        _drm_lib = NULL;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool 
_ecore_xcb_dri_start(void) 
{
   Ecore_X_Display *disp;
   int _dri2_event = 0, _dri2_error = 0;
   int _dri2_major = 0, _dri2_minor = 0;
   char *device = NULL, *driver = NULL;

   disp = ecore_x_display_get();
   if (!sym_DRI2QueryExtension(disp, &_dri2_event, &_dri2_error))
     return 0;
   if (!sym_DRI2QueryVersion(disp, &_dri2_major, &_dri2_minor))
     return 0;
   if (_dri2_major < 2) return 0;
   if (!sym_DRI2Connect(disp, _vsync_root, &driver, &device))
     return 0;

   _drm_fd = open(device, O_RDWR);
   if (_drm_fd < 0) return 0;

   sym_drmGetMagic(_drm_fd, &_drm_magic);
   if (!sym_DRI2Authenticate(disp, _vsync_root, _drm_magic))
     {
        close(_drm_fd);
        _drm_fd = -1;
        return EINA_FALSE;
     }

   memset(&_drm_evctx, 0, sizeof(_drm_evctx));
   _drm_evctx.version = DRM_EVENT_CONTEXT_VERSION;
   _drm_evctx.vblank_handler = _ecore_xcb_dri_vblank_handler;
   _drm_evctx.page_flip_handler = NULL;

   _drm_fdh = ecore_main_fd_handler_add(_drm_fd, ECORE_FD_READ, 
                                        _ecore_xcb_dri_cb, NULL, NULL, NULL);
   if (!_drm_fdh) 
     {
        close(_drm_fd);
        _drm_fd = -1;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void 
_ecore_xcb_dri_shutdown(void) 
{
   if (_drm_fd >= 0) 
     {
        close(_drm_fd);
        _drm_fd = -1;
     }
   if (_drm_fdh) 
     {
        ecore_main_fd_handler_del(_drm_fdh);
        _drm_fdh = NULL;
     }
}

static Eina_Bool 
_ecore_xcb_dri_cb(void *data __UNUSED__, Ecore_Fd_Handler *fdh __UNUSED__) 
{
   sym_drmHandleEvent(_drm_fd, &_drm_evctx);
   return ECORE_CALLBACK_RENEW;
}

static void 
_ecore_xcb_dri_tick_begin(void *data __UNUSED__) 
{
   _drm_event_busy = EINA_TRUE;
   _ecore_xcb_dri_tick_schedule();
}

static void 
_ecore_xcb_dri_tick_end(void *data __UNUSED__) 
{
   _drm_event_busy = EINA_FALSE;
}

static void 
_ecore_xcb_dri_tick_schedule(void) 
{
   drmVBlank vbl;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   vbl.request.type = (DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT);
   vbl.request.sequence = 1;
   vbl.request.signal = 0;

   sym_drmWaitVBlank(_drm_fd, &vbl);
}

static void 
_ecore_xcb_dri_vblank_handler(int fd __UNUSED__, unsigned int frame __UNUSED__, unsigned int sec __UNUSED__, unsigned int usec __UNUSED__, void *data __UNUSED__) 
{
   ecore_animator_custom_tick();
   if (_drm_event_busy) _ecore_xcb_dri_tick_schedule();
}
#endif

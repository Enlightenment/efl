#include "ecore_xcb_private.h"
#ifdef ECORE_XCB_DRI
# include <fcntl.h>
# include <dlfcn.h>
# include <xcb/dri2.h>
#endif

#define ECORE_XCB_VSYNC_DRI2 1
#define DRM_EVENT_CONTEXT_VERSION 2

#ifdef ECORE_XCB_DRI

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

#endif

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
static Eina_Bool _dri2_avail = EINA_FALSE;
static Ecore_X_Window _vsync_root = 0;
static int _drm_fd = -1;
static Ecore_Fd_Handler *_drm_fdh = NULL;
static unsigned int _drm_magic = 0;
static Eina_Bool _drm_event_busy = EINA_FALSE;
static void *_drm_lib = NULL;
#ifdef ECORE_XCB_DRI
static drmEventContext _drm_evctx;
#endif

void 
_ecore_xcb_dri_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_DRI
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_dri2_id);
#endif
}

void 
_ecore_xcb_dri_finalize(void) 
{
#ifdef ECORE_XCB_DRI
   const xcb_query_extension_reply_t *ext_reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_DRI
   ext_reply = xcb_get_extension_data(_ecore_xcb_conn, &xcb_dri2_id);
   if ((ext_reply) && (ext_reply->present))
     {
        xcb_dri2_query_version_cookie_t dcookie;
        xcb_dri2_query_version_reply_t *dreply;

        dcookie = 
          xcb_dri2_query_version_unchecked(_ecore_xcb_conn, 
                                           XCB_DRI2_MAJOR_VERSION, 
                                           XCB_DRI2_MINOR_VERSION);
        dreply = xcb_dri2_query_version_reply(_ecore_xcb_conn, dcookie, NULL);
        if (dreply) 
          {
             if (dreply->major_version >= 2) _dri2_avail = EINA_TRUE;
             free(dreply);
          }
     }
#endif
}

EAPI Eina_Bool 
ecore_x_vsync_animator_tick_source_set(Ecore_X_Window win) 
{
#ifdef ECORE_XCB_DRI
   Ecore_X_Window root;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!_dri2_avail) return EINA_FALSE;

#ifdef ECORE_XCB_DRI
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
static Eina_Bool 
_ecore_xcb_dri_link(void) 
{
#ifdef ECORE_XCB_DRI
   const char *_drm_libs[] = 
     {
        "libdrm.so.2", 
        "libdrm.so.1", 
        "libdrm.so.0", 
        "libdrm.so", 
        NULL,
     };
   int i = 0, fail = 0;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_DRI
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
   return EINA_TRUE;
#endif
   return EINA_FALSE;
}

static Eina_Bool 
_ecore_xcb_dri_start(void) 
{
#ifdef ECORE_XCB_DRI
   xcb_dri2_connect_cookie_t cookie;
   xcb_dri2_connect_reply_t *reply;
   xcb_dri2_authenticate_cookie_t acookie;
   xcb_dri2_authenticate_reply_t *areply;
   char *device = NULL, *driver = NULL;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!_dri2_avail) return EINA_FALSE;

#ifdef ECORE_XCB_DRI
   cookie = xcb_dri2_connect_unchecked(_ecore_xcb_conn, 
                                       _vsync_root, XCB_DRI2_DRIVER_TYPE_DRI);
   reply = xcb_dri2_connect_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;
   driver = xcb_dri2_connect_driver_name(reply);
   device = xcb_dri2_connect_device_name(reply);
   free(reply);

   if (!(_drm_fd = open(device, O_RDWR))) 
     {
        _drm_fd = -1;
        return EINA_FALSE;
     }

   sym_drmGetMagic(_drm_fd, &_drm_magic);

   acookie = 
     xcb_dri2_authenticate_unchecked(_ecore_xcb_conn, _vsync_root, _drm_magic);
   areply = xcb_dri2_authenticate_reply(_ecore_xcb_conn, acookie, NULL);
   if (!areply) 
     {
        close(_drm_fd);
        _drm_fd = -1;
        return EINA_FALSE;
     }
   free(areply);

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
#endif

   return EINA_FALSE;
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
#ifdef ECORE_XCB_DRI
   sym_drmHandleEvent(_drm_fd, &_drm_evctx);
#endif
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
#ifdef ECORE_XCB_DRI
   drmVBlank vbl;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_DRI
   vbl.request.type = (DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT);
   vbl.request.sequence = 1;
   vbl.request.signal = 0;

   sym_drmWaitVBlank(_drm_fd, &vbl);
#endif
}

static void 
_ecore_xcb_dri_vblank_handler(int fd __UNUSED__, unsigned int frame __UNUSED__, unsigned int sec __UNUSED__, unsigned int usec __UNUSED__, void *data __UNUSED__) 
{
   ecore_animator_custom_tick();
   if (_drm_event_busy) _ecore_xcb_dri_tick_schedule();
}

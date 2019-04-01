#ifndef _ECORE_DRM2_PRIVATE_H
# define _ECORE_DRM2_PRIVATE_H

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include "Ecore.h"
# include "ecore_private.h"
# include "Eeze.h"
# include "Elput.h"
# include <Ecore_Drm2.h>

# include <unistd.h>
# include <strings.h>
# include <sys/mman.h>
# include <fcntl.h>
# include <ctype.h>

# include <sys/ioctl.h>
# include <dlfcn.h>

# include <drm.h>
# include <drm_mode.h>
# include <drm_fourcc.h>
# include <xf86drm.h>
# include <xf86drmMode.h>

#ifndef DRM2_NODEFS
extern int _ecore_drm2_log_dom;
extern Eina_Bool _ecore_drm2_use_atomic;

# ifdef ECORE_DRM2_DEFAULT_LOG_COLOR
#  undef ECORE_DRM2_DEFAULT_LOG_COLOR
# endif
# define ECORE_DRM2_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_drm2_log_dom, __VA_ARGS__)
#endif

typedef struct _Ecore_Drm2_Atomic_State Ecore_Drm2_Atomic_State;

typedef struct _Ecore_Drm2_Atomic_Blob
{
   uint32_t id, value;
   size_t len;
   void *data;
} Ecore_Drm2_Atomic_Blob;

typedef struct _Ecore_Drm2_Atomic_Property
{
   uint32_t id;
   uint64_t value;
} Ecore_Drm2_Atomic_Property;

typedef struct _Ecore_Drm2_Connector_State
{
   uint32_t obj_id;
   Ecore_Drm2_Atomic_Property crtc;
   Ecore_Drm2_Atomic_Property dpms;
   Ecore_Drm2_Atomic_Property aspect;
   Ecore_Drm2_Atomic_Property scaling;
   Ecore_Drm2_Atomic_Blob edid;
} Ecore_Drm2_Connector_State;

typedef struct _Ecore_Drm2_Crtc_State
{
   uint32_t obj_id;
   int index;
   Ecore_Drm2_Atomic_Property active;
   Ecore_Drm2_Atomic_Blob mode;
} Ecore_Drm2_Crtc_State;

typedef struct _Ecore_Drm2_Plane_State
{
   uint32_t obj_id, mask;
   Ecore_Drm2_Atomic_Property type;
   Ecore_Drm2_Atomic_Property cid, fid;
   Ecore_Drm2_Atomic_Property sx, sy, sw, sh;
   Ecore_Drm2_Atomic_Property cx, cy, cw, ch;
   Ecore_Drm2_Atomic_Property rotation;

   /* these are not part of an atomic state, but we store these here
    * so that we do not have to refetch properties when iterating planes */
   uint32_t rotation_map[6];
   uint32_t supported_rotations;

   uint32_t num_formats;
   uint32_t *formats;

   Eina_Bool in_use;
} Ecore_Drm2_Plane_State;

struct _Ecore_Drm2_Atomic_State
{
   int crtcs, conns, planes;
   Ecore_Drm2_Crtc_State *crtc_states;
   Ecore_Drm2_Connector_State *conn_states;
   Ecore_Drm2_Plane_State *plane_states;
};

# ifndef DRM_CAP_CURSOR_WIDTH
#  define DRM_CAP_CURSOR_WIDTH 0x8
# endif

# ifndef DRM_CAP_CURSOR_HEIGHT
#  define DRM_CAP_CURSOR_HEIGHT 0x9
# endif

typedef enum _Ecore_Drm2_Backlight_Type
{
   ECORE_DRM2_BACKLIGHT_RAW,
   ECORE_DRM2_BACKLIGHT_PLATFORM,
   ECORE_DRM2_BACKLIGHT_FIRMWARE
} Ecore_Drm2_Backlight_Type;

typedef enum _Ecore_Drm2_Transform
{
   ECORE_DRM2_TRANSFORM_NORMAL,
   ECORE_DRM2_TRANSFORM_90,
   ECORE_DRM2_TRANSFORM_180,
   ECORE_DRM2_TRANSFORM_270,
   ECORE_DRM2_TRANSFORM_FLIPPED,
   ECORE_DRM2_TRANSFORM_FLIPPED_90,
   ECORE_DRM2_TRANSFORM_FLIPPED_180,
   ECORE_DRM2_TRANSFORM_FLIPPED_270
} Ecore_Drm2_Transform;

struct _Ecore_Drm2_Fb
{
   int fd;
   int w, h;
   int depth, bpp;
   short ref;
   int scanout_count;
   uint32_t id, handles[4];
   uint32_t strides[4], sizes[4];
   uint32_t format;

   void *gbm_bo;
   void *mmap;

   Ecore_Drm2_Fb_Status_Handler status_handler;
   void *status_data;

   Eina_Bool gbm : 1;
   Eina_Bool dmabuf : 1;
   Eina_Bool dead : 1;
};

struct _Ecore_Drm2_Plane
{
   int type;
   Ecore_Drm2_Plane_State *state;
   Ecore_Drm2_Output *output;
   Ecore_Drm2_Fb *fb;
   Eina_Bool dead;
   Eina_Bool scanout;
};

struct _Ecore_Drm2_Output_Mode
{
   uint32_t id;
   uint32_t flags;
   int32_t width, height;
   uint32_t refresh;
   drmModeModeInfo info;
};

/* A half step - we still keep an fb for the canvas
 * and an atomic_req for all atomic state (including
 * the canvas fb).
 * The non atomic code only uses the canvas fb.
 */
typedef struct _Ecore_Drm2_Output_State
{
   Ecore_Drm2_Fb *fb;
   drmModeAtomicReq *atomic_req;
} Ecore_Drm2_Output_State;

struct _Ecore_Drm2_Output
{
   Eina_Stringshare *name;
   Eina_Stringshare *make, *model, *serial;

   int fd;
   int pipe;
   int x, y, w, h, pw, ph;
   int rotation;

   long fallback_sec, fallback_usec;

   uint32_t subpixel;
   uint32_t crtc_id, conn_id, conn_type;
   uint32_t scale;
   uint16_t gamma;

   struct
     {
        char eisa[13];
        char monitor[13];
        char pnp[5];
        char serial[13];
        unsigned char *blob; // unused when doing atomic
     } edid;

   struct
     {
        const char *path;
        double value, max;
        Ecore_Drm2_Backlight_Type type;
     } backlight;

   struct
     {
        const char *to;
        Ecore_Drm2_Relative_Mode mode;
     } relative;

   drmModeCrtcPtr ocrtc;

   /* prep is for state we're preparing and have never
    * attempted to commit */
   Ecore_Drm2_Output_State prep;

   Ecore_Drm2_Output_State current, next, pending;

   Eina_Matrix4 matrix, inverse;
   Ecore_Drm2_Transform transform;

   /* unused when doing atomic */
   drmModePropertyPtr dpms;

   Ecore_Timer *flip_timeout;

   Ecore_Drm2_Output_Mode *current_mode;
   Eina_List *modes;

   void *user_data;

   Ecore_Drm2_Crtc_State *crtc_state;
   Ecore_Drm2_Connector_State *conn_state;

   Eina_List *plane_states;
   Eina_List *planes;
   Eina_List *fbs;

   Eina_Bool connected : 1;
   Eina_Bool primary : 1;
   Eina_Bool cloned : 1;
   Eina_Bool enabled : 1;
};

struct _Ecore_Drm2_Device
{
   Elput_Manager *em;

   int fd;
   const char *path;

   int num_crtcs;
   uint32_t *crtcs;

   struct
     {
        uint32_t width, height;
     } min, max;

   struct
     {
        int width, height;
     } cursor;

   Eeze_Udev_Watch *watch;
   Ecore_Event_Handler *active_hdlr;
   Ecore_Event_Handler *device_change_hdlr;

   Ecore_Drm2_Atomic_State *state;

   Eina_List *outputs;
};

Eina_Bool _fb_atomic_flip_test(Ecore_Drm2_Output *output);
void _ecore_drm2_fb_ref(Ecore_Drm2_Fb *);
void _ecore_drm2_fb_deref(Ecore_Drm2_Fb *);
void _ecore_drm2_fb_buffer_release(Ecore_Drm2_Output *output, Ecore_Drm2_Output_State *s);

/* extern int (*sym_drmClose)(int fd); */
extern int (*sym_drmWaitVBlank)(int fd, drmVBlank *vbl);
extern int (*sym_drmHandleEvent)(int fd, drmEventContext *evctx);
extern void *(*sym_drmGetVersion)(int fd);
extern void (*sym_drmFreeVersion)(void *drmver);
extern void *(*sym_drmModeGetProperty)(int fd, uint32_t propertyId);
extern void (*sym_drmModeFreeProperty)(drmModePropertyPtr ptr);
extern void *(*sym_drmModeGetPropertyBlob)(int fd, uint32_t blob_id);
extern void (*sym_drmModeFreePropertyBlob)(drmModePropertyBlobPtr ptr);
extern int (*sym_drmModeDestroyPropertyBlob)(int fd, uint32_t id);
extern int (*sym_drmIoctl)(int fd, unsigned long request, void *arg);
extern void *(*sym_drmModeObjectGetProperties)(int fd, uint32_t object_id, uint32_t object_type);
extern void (*sym_drmModeFreeObjectProperties)(drmModeObjectPropertiesPtr ptr);
extern int (*sym_drmModeCreatePropertyBlob)(int fd, const void *data, size_t size, uint32_t *id);
extern void *(*sym_drmModeAtomicAlloc)(void);
extern void (*sym_drmModeAtomicFree)(drmModeAtomicReqPtr req);
extern int (*sym_drmModeAtomicAddProperty)(drmModeAtomicReqPtr req, uint32_t object_id, uint32_t property_id, uint64_t value);
extern int (*sym_drmModeAtomicCommit)(int fd, drmModeAtomicReqPtr req, uint32_t flags, void *user_data);
extern void (*sym_drmModeAtomicSetCursor)(drmModeAtomicReqPtr req, int cursor);
extern int (*sym_drmModeAtomicMerge)(drmModeAtomicReqPtr base, drmModeAtomicReqPtr augment);
extern void *(*sym_drmModeGetEncoder)(int fd, uint32_t encoder_id);
extern void (*sym_drmModeFreeEncoder)(drmModeEncoderPtr ptr);
extern void *(*sym_drmModeGetCrtc)(int fd, uint32_t crtcId);
extern void (*sym_drmModeFreeCrtc)(drmModeCrtcPtr ptr);
extern int (*sym_drmModeSetCrtc)(int fd, uint32_t crtcId, uint32_t bufferId, uint32_t x, uint32_t y, uint32_t *connectors, int count, drmModeModeInfoPtr mode);
extern void *(*sym_drmModeGetResources)(int fd);
extern void (*sym_drmModeFreeResources)(drmModeResPtr ptr);
extern void *(*sym_drmModeGetConnector)(int fd, uint32_t connectorId);
extern void (*sym_drmModeFreeConnector)(drmModeConnectorPtr ptr);
extern int (*sym_drmModeConnectorSetProperty)(int fd, uint32_t connector_id, uint32_t property_id, uint64_t value);
extern int (*sym_drmGetCap)(int fd, uint64_t capability, uint64_t *value);
extern int (*sym_drmSetClientCap)(int fd, uint64_t capability, uint64_t value);
extern void *(*sym_drmModeGetPlaneResources)(int fd);
extern void (*sym_drmModeFreePlaneResources)(drmModePlaneResPtr ptr);
extern void *(*sym_drmModeGetPlane)(int fd, uint32_t plane_id);
extern void (*sym_drmModeFreePlane)(drmModePlanePtr ptr);
extern int (*sym_drmModeAddFB)(int fd, uint32_t width, uint32_t height, uint8_t depth, uint8_t bpp, uint32_t pitch, uint32_t bo_handle, uint32_t *buf_id);
extern int (*sym_drmModeAddFB2)(int fd, uint32_t width, uint32_t height, uint32_t pixel_format, uint32_t bo_handles[4], uint32_t pitches[4], uint32_t offsets[4], uint32_t *buf_id, uint32_t flags);
extern int (*sym_drmModeRmFB)(int fd, uint32_t bufferId);
extern int (*sym_drmModePageFlip)(int fd, uint32_t crtc_id, uint32_t fb_id, uint32_t flags, void *user_data);
extern int (*sym_drmModeDirtyFB)(int fd, uint32_t bufferId, drmModeClipPtr clips, uint32_t num_clips);
extern int (*sym_drmModeCrtcSetGamma)(int fd, uint32_t crtc_id, uint32_t size, uint16_t *red, uint16_t *green, uint16_t *blue);
extern int (*sym_drmPrimeFDToHandle)(int fd, int prime_fd, uint32_t *handle);

#endif

#ifndef _ECORE_DRM2_PRIVATE_H
# define _ECORE_DRM2_PRIVATE_H

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

/* include system headers */
# include <unistd.h>
# include <strings.h>
# include <sys/mman.h>
# include <fcntl.h>
# include <ctype.h>
# include <sys/ioctl.h>
# include <dlfcn.h>
# include <time.h>

/* include drm headers */
# include <drm.h>
# include <drm_mode.h>
# include <drm_fourcc.h>
# include <xf86drm.h>
# include <xf86drmMode.h>

/* include needed EFL headers */
# include "Ecore.h"
# include "ecore_private.h"
# include "Eeze.h"
# include "Elput.h"
# include <Ecore_Drm2.h>

/* define necessary vars/macros for ecore_drm2 log domain */
extern int _ecore_drm2_log_dom;

# ifdef ECORE_DRM2_DEFAULT_LOG_COLOR
#  undef ECORE_DRM2_DEFAULT_LOG_COLOR
# endif
# define ECORE_DRM2_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_drm2_log_dom, __VA_ARGS__)

/* internal structures & enums (not exposed) */
typedef enum _Ecore_Drm2_Backlight_Type
{
   ECORE_DRM2_BACKLIGHT_RAW,
   ECORE_DRM2_BACKLIGHT_PLATFORM,
   ECORE_DRM2_BACKLIGHT_FIRMWARE
} Ecore_Drm2_Backlight_Type;

typedef enum _Ecore_Drm2_Thread_Op_Code
{
   ECORE_DRM2_THREAD_CODE_FILL,
   ECORE_DRM2_THREAD_CODE_DEBUG
} Ecore_Drm2_Thread_Op_Code;

typedef enum _Ecore_Drm2_Connector_State_Changes
{
   ECORE_DRM2_CONNECTOR_STATE_CRTC = (1 << 0),
   ECORE_DRM2_CONNECTOR_STATE_DPMS = (1 << 1),
   ECORE_DRM2_CONNECTOR_STATE_ASPECT = (1 << 2),
   ECORE_DRM2_CONNECTOR_STATE_SCALING = (1 << 3),
} Ecore_Drm2_Connector_State_Changes;

typedef enum _Ecore_Drm2_Crtc_State_Changes
{
   ECORE_DRM2_CRTC_STATE_ACTIVE = (1 << 0),
   ECORE_DRM2_CRTC_STATE_MODE = (1 << 1),
} Ecore_Drm2_Crtc_State_Changes;

typedef enum _Ecore_Drm2_Plane_State_Changes
{
   ECORE_DRM2_PLANE_STATE_CID = (1 << 0),
   ECORE_DRM2_PLANE_STATE_FID = (1 << 1),
   ECORE_DRM2_PLANE_STATE_ROTATION = (1 << 2),
} Ecore_Drm2_Plane_State_Changes;

typedef enum _Ecore_Drm2_Display_State_Changes
{
   ECORE_DRM2_DISPLAY_STATE_GAMMA = (1 << 0),
   ECORE_DRM2_DISPLAY_STATE_ROTATION = (1 << 1),
   ECORE_DRM2_DISPLAY_STATE_BACKLIGHT = (1 << 2),
   ECORE_DRM2_DISPLAY_STATE_MODE = (1 << 3),
   ECORE_DRM2_DISPLAY_STATE_PRIMARY = (1 << 4),
   ECORE_DRM2_DISPLAY_STATE_ENABLED = (1 << 5),
   ECORE_DRM2_DISPLAY_STATE_POSITION = (1 << 6),
} Ecore_Drm2_Display_State_Changes;

typedef struct _Ecore_Drm2_Atomic_Blob
{
   uint32_t id, value;
   size_t len;
   void *data;
} Ecore_Drm2_Atomic_Blob;

typedef struct _Ecore_Drm2_Atomic_Property
{
   uint32_t id, flags;
   uint64_t value;
} Ecore_Drm2_Atomic_Property;

typedef struct _Ecore_Drm2_Atomic_Range
{
   uint32_t id, flags;
   uint64_t min, max, value;
} Ecore_Drm2_Atomic_Range;

typedef struct _Ecore_Drm2_Connector_State
{
   uint32_t obj_id, changes;
   Ecore_Drm2_Atomic_Property crtc;
   Ecore_Drm2_Atomic_Property dpms;
   Ecore_Drm2_Atomic_Property aspect;
   Ecore_Drm2_Atomic_Property scaling;
   Ecore_Drm2_Atomic_Blob edid;

   /* TODO ?? */
   /* Ecore_Drm2_Atomic_Property link-status; */
   /* Ecore_Drm2_Atomic_Property non-desktop; */
   /* Ecore_Drm2_Atomic_Property TILE; */
   /* Ecore_Drm2_Atomic_Property underscan; */
   /* Ecore_Drm2_Atomic_Property underscan hborder; */
   /* Ecore_Drm2_Atomic_Property underscan vborder; */
   /* Ecore_Drm2_Atomic_Property max bpc; */
   /* Ecore_Drm2_Atomic_Property HDR_OUTPUT_METADATA; */
   /* Ecore_Drm2_Atomic_Property vrr_capable; */
   /* Ecore_Drm2_Atomic_Property Content Protection; */
   /* Ecore_Drm2_Atomic_Property HDCP Content Type; */
   /* Ecore_Drm2_Atomic_Property subconnector; */
   /* Ecore_Drm2_Atomic_Property panel orientation; */
} Ecore_Drm2_Connector_State;

typedef struct _Ecore_Drm2_Crtc_State
{
   uint32_t obj_id, changes;
   /* int index; */
   Ecore_Drm2_Atomic_Property active;
   Ecore_Drm2_Atomic_Blob mode;

   /* TODO ?? */
   /* Ecore_Drm2_Atomic_Property background; */
   /* Ecore_Drm2_Atomic_Property OUT_FENCE_PTR; */
   /* Ecore_Drm2_Atomic_Property VRR_ENABLED; */
   /* Ecore_Drm2_Atomic_Property DEGAMMA_LUT; */
   /* Ecore_Drm2_Atomic_Property DEGAMMA_LUT_SIZE; */
   /* Ecore_Drm2_Atomic_Property CTM; */
   /* Ecore_Drm2_Atomic_Property GAMMA_LUT; */
   /* Ecore_Drm2_Atomic_Property GAMMA_LUT_SIZE; */
} Ecore_Drm2_Crtc_State;

typedef struct _Ecore_Drm2_Plane_State
{
   uint32_t obj_id, mask, changes;
   uint32_t num_formats, *formats;

   Ecore_Drm2_Atomic_Property type;
   Ecore_Drm2_Atomic_Property cid, fid;
   Ecore_Drm2_Atomic_Property sx, sy, sw, sh;
   Ecore_Drm2_Atomic_Property cx, cy, cw, ch;
   Ecore_Drm2_Atomic_Property rotation;
   Ecore_Drm2_Atomic_Range zpos;

   /* TODO ?? */
   /* Ecore_Drm2_Atomic_Property IN_FENCE_FD; */
   /* Ecore_Drm2_Atomic_Property IN_FORMATS; */
   /* Ecore_Drm2_Atomic_Property COLOR_ENCODING; */
   /* Ecore_Drm2_Atomic_Property COLOR_RANGE; */
   /* Ecore_Drm2_Atomic_Blob FB_DAMAGE_CLIPS; */

   /* NB: these are not part of an atomic state, but we store these here
    * so that we do not have to refetch properties when iterating planes */
   uint32_t rotation_map[6];
   uint32_t supported_rotations;

   Eina_Bool in_use : 1;
} Ecore_Drm2_Plane_State;

typedef struct _Ecore_Drm2_Display_State
{
   uint32_t changes;

   int x, y;

   struct
     {
	uint16_t *r, *g, *b;
	uint16_t size;
     } gamma;

   uint64_t rotation;
   double backlight;

   Ecore_Drm2_Display_Mode *mode;

   Eina_Bool primary : 1;
   Eina_Bool enabled : 1;
} Ecore_Drm2_Display_State;

/* opaque API structures */
struct _Ecore_Drm2_Fb
{
   int fd;
   int w, h;
   int depth, bpp;
   uint32_t format;

   uint32_t id, handles[4];
   uint32_t strides[4], sizes[4];

   void *mmap, *bo;
};

struct _Ecore_Drm2_Plane
{
   int fd;
   uint32_t id;

   drmModePlanePtr drmPlane;

   struct
     {
        Ecore_Drm2_Plane_State *current;
        Ecore_Drm2_Plane_State *pending;
     } state;

   Ecore_Thread *thread;
};

struct _Ecore_Drm2_Display_Mode
{
   uint32_t id;
   uint32_t flags, refresh;
   int32_t width, height;
   drmModeModeInfo info;
};

struct _Ecore_Drm2_Display
{
   /* int fd; */
   int x, y;
   int pw, ph; // physical dimensions
   Eina_Stringshare *name, *make, *model, *serial;

   uint32_t subpixel;
   uint64_t msc;

   /* uint32_t supported_rotations; */

   struct
     {
        char eisa[13];
        char monitor[13];
        char pnp[5];
        char serial[13];
     } edid;

   struct
     {
        const char *path;
        double max;
        Ecore_Drm2_Backlight_Type type;
     } backlight;

   struct
     {
        const char *to;
        Ecore_Drm2_Relative_Mode mode;
     } relative;

   struct
     {
        Ecore_Drm2_Display_State *current;
        Ecore_Drm2_Display_State *pending;
     } state;

   Ecore_Drm2_Device *dev;
   Ecore_Drm2_Crtc *crtc;
   Ecore_Drm2_Connector *conn;

   Eina_List *modes;

   Ecore_Thread *thread;

   void *user_data;

   Eina_Bool connected : 1;
};

struct _Ecore_Drm2_Connector
{
   uint32_t id;
   uint32_t type;
   int fd;

   drmModeConnector *drmConn;

   struct
     {
        Ecore_Drm2_Connector_State *current;
        Ecore_Drm2_Connector_State *pending;
     } state;

   Ecore_Thread *thread;

   Eina_Bool writeback : 1;
};

struct _Ecore_Drm2_Crtc
{
   uint32_t id;
   uint32_t pipe;
   int fd;

   drmModeCrtcPtr drmCrtc;

   /* TODO: store FBs */

   struct
     {
        Ecore_Drm2_Crtc_State *current;
        Ecore_Drm2_Crtc_State *pending;
     } state;

   Ecore_Thread *thread;
};

struct _Ecore_Drm2_Device
{
   Elput_Manager *em;

   int fd;
   int clock_id;

   struct
     {
        int width, height;
     } cursor;

   Ecore_Event_Handler *session_hdlr;
   Ecore_Event_Handler *device_hdlr;

   Eina_List *crtcs;
   Eina_List *conns;
   Eina_List *displays;
   Eina_List *planes;

   Eina_Bool atomic : 1;
   /* Eina_Bool gbm_mods : 1; */
   Eina_Bool aspect_ratio : 1;
};

/* internal function prototypes */
Eina_Bool _ecore_drm2_crtcs_create(Ecore_Drm2_Device *dev);
void _ecore_drm2_crtcs_destroy(Ecore_Drm2_Device *dev);
Eina_Bool _ecore_drm2_crtcs_mode_set(Ecore_Drm2_Crtc *crtc, Ecore_Drm2_Display_Mode *mode);

Eina_Bool _ecore_drm2_connectors_create(Ecore_Drm2_Device *dev);
void _ecore_drm2_connectors_destroy(Ecore_Drm2_Device *dev);

Eina_Bool _ecore_drm2_displays_create(Ecore_Drm2_Device *dev);
void _ecore_drm2_displays_destroy(Ecore_Drm2_Device *dev);

Eina_Bool _ecore_drm2_planes_create(Ecore_Drm2_Device *dev);
void _ecore_drm2_planes_destroy(Ecore_Drm2_Device *dev);
Ecore_Drm2_Plane *_ecore_drm2_planes_primary_find(Ecore_Drm2_Device *dev, unsigned int crtc_id);

/* external drm function prototypes (for dlopen) */
extern int (*sym_drmIoctl)(int fd, unsigned long request, void *arg);
extern void *(*sym_drmModeGetResources)(int fd);
extern void (*sym_drmModeFreeResources)(drmModeResPtr ptr);
extern int (*sym_drmGetCap)(int fd, uint64_t capability, uint64_t *value);
extern int (*sym_drmSetClientCap)(int fd, uint64_t capability, uint64_t value);
extern void *(*sym_drmModeGetProperty)(int fd, uint32_t propertyId);
extern void (*sym_drmModeFreeProperty)(drmModePropertyPtr ptr);
extern void *(*sym_drmModeGetPropertyBlob)(int fd, uint32_t blob_id);
extern void (*sym_drmModeFreePropertyBlob)(drmModePropertyBlobPtr ptr);
extern int (*sym_drmModeCreatePropertyBlob)(int fd, const void *data, size_t size, uint32_t *id);
extern int (*sym_drmModeDestroyPropertyBlob)(int fd, uint32_t id);
extern void *(*sym_drmModeObjectGetProperties)(int fd, uint32_t object_id, uint32_t object_type);
extern void (*sym_drmModeFreeObjectProperties)(drmModeObjectPropertiesPtr ptr);
extern void *(*sym_drmModeGetPlaneResources)(int fd);
extern void (*sym_drmModeFreePlaneResources)(drmModePlaneResPtr ptr);
extern void *(*sym_drmModeGetPlane)(int fd, uint32_t plane_id);
extern void (*sym_drmModeFreePlane)(drmModePlanePtr ptr);
extern void *(*sym_drmModeGetConnector)(int fd, uint32_t connectorId);
extern void (*sym_drmModeFreeConnector)(drmModeConnectorPtr ptr);
extern void *(*sym_drmModeGetEncoder)(int fd, uint32_t encoder_id);
extern void (*sym_drmModeFreeEncoder)(drmModeEncoderPtr ptr);
extern void *(*sym_drmModeGetCrtc)(int fd, uint32_t crtcId);
extern void (*sym_drmModeFreeCrtc)(drmModeCrtcPtr ptr);
extern void *(*sym_drmModeAtomicAlloc)(void);
extern void (*sym_drmModeAtomicFree)(drmModeAtomicReqPtr req);
extern int (*sym_drmModeAtomicAddProperty)(drmModeAtomicReqPtr req, uint32_t object_id, uint32_t property_id, uint64_t value);
extern int (*sym_drmModeAtomicCommit)(int fd, drmModeAtomicReqPtr req, uint32_t flags, void *user_data);
extern void (*sym_drmModeAtomicSetCursor)(drmModeAtomicReqPtr req, int cursor);
extern int (*sym_drmWaitVBlank)(int fd, drmVBlank *vbl);
extern int (*sym_drmModeAddFB)(int fd, uint32_t width, uint32_t height, uint8_t depth, uint8_t bpp, uint32_t pitch, uint32_t bo_handle, uint32_t *buf_id);
extern int (*sym_drmModeAddFB2)(int fd, uint32_t width, uint32_t height, uint32_t pixel_format, uint32_t bo_handles[4], uint32_t pitches[4], uint32_t offsets[4], uint32_t *buf_id, uint32_t flags);
extern int (*sym_drmModeRmFB)(int fd, uint32_t bufferId);
extern int (*sym_drmModePageFlip)(int fd, uint32_t crtc_id, uint32_t fb_id, uint32_t flags, void *user_data);
extern int (*sym_drmModeDirtyFB)(int fd, uint32_t bufferId, drmModeClipPtr clips, uint32_t num_clips);
extern int (*sym_drmModeCrtcSetGamma)(int fd, uint32_t crtc_id, uint32_t size, uint16_t *red, uint16_t *green, uint16_t *blue);

#endif

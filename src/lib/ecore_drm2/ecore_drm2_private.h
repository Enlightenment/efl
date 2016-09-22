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
# include <xf86drm.h>
# include <xf86drmMode.h>
# include <drm_mode.h>
# include <drm_fourcc.h>

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

# ifdef HAVE_ATOMIC_DRM
typedef struct _Ecore_Drm2_Atomic_State Ecore_Drm2_Atomic_State;

typedef struct _Ecore_Drm2_Atomic_Blob
{
   uint32_t id, value;
   size_t len;
   void *data;
} Ecore_Drm2_Atomic_Blob;

typedef struct _Ecore_Drm2_Atomic_Property
{
   uint32_t id, value;
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
} Ecore_Drm2_Plane_State;

struct _Ecore_Drm2_Atomic_State
{
   int crtcs, conns, planes;
   Ecore_Drm2_Crtc_State *crtc_states;
   Ecore_Drm2_Connector_State *conn_states;
   Ecore_Drm2_Plane_State *plane_states;
};
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

typedef enum _Ecore_Drm2_Rotation
{
   ECORE_DRM2_ROTATION_NORMAL = 1,
   ECORE_DRM2_ROTATION_90 = 2,
   ECORE_DRM2_ROTATION_180 = 4,
   ECORE_DRM2_ROTATION_270 = 8,
   ECORE_DRM2_ROTATION_REFLECT_X = 16,
   ECORE_DRM2_ROTATION_REFLECT_Y = 32
} Ecore_Drm2_Rotation;

struct _Ecore_Drm2_Fb
{
   int fd;
   int w, h;
   int depth, bpp;
   uint32_t id, hdl;
   uint32_t stride, size;
   uint32_t format;

   void *gbm_bo;

   Eina_Bool gbm : 1;
   Eina_Bool busy : 1;

   void *mmap;
};

struct _Ecore_Drm2_Output_Mode
{
   uint32_t flags;
   int32_t width, height;
   uint32_t refresh;
   drmModeModeInfo info;
};

struct _Ecore_Drm2_Output
{
   Eina_Stringshare *name;
   Eina_Stringshare *make, *model, *serial;

   int fd;
   int pipe;
   int x, y, w, h, pw, ph;

   uint32_t subpixel;
   uint32_t crtc_id, conn_id, conn_type;
   uint32_t scale;

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
        int value, max;
        Ecore_Drm2_Backlight_Type type;
     } backlight;

   drmModeCrtcPtr ocrtc;

   Ecore_Drm2_Fb *current, *next, *pending;

   Eina_Matrix4 matrix, inverse;
   Ecore_Drm2_Transform transform;

   /* unused when doing atomic */
   drmModePropertyPtr dpms;

   Ecore_Drm2_Output_Mode *current_mode;
   Eina_List *modes;

   Eina_List *planes;

   void *user_data;
   Ecore_Drm2_Release_Handler release_cb;
   void *release_data;

# ifdef HAVE_ATOMIC_DRM
   Ecore_Drm2_Crtc_State *crtc_state;
   Ecore_Drm2_Connector_State *conn_state;
   Ecore_Drm2_Plane_State *plane_state;
# endif

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
        uint32_t crtc, conn;
     } alloc;

   struct
     {
        uint32_t width, height;
     } min, max;

   Eeze_Udev_Watch *watch;
   Ecore_Event_Handler *active_hdlr;
   Ecore_Event_Handler *device_change_hdlr;

# ifdef HAVE_ATOMIC_DRM
   Ecore_Drm2_Atomic_State *state;
# endif

   Eina_List *outputs;
};

#endif

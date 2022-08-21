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

/* define external variable for atomic usage */
extern Eina_Bool _ecore_drm2_atomic_use;

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

/* internal structures (not exposed) */
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

} Ecore_Drm2_Connector_State;

typedef struct _Ecore_Drm2_Crtc_State
{
   uint32_t obj_id;
   int index;
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
   uint32_t obj_id, mask;
   Ecore_Drm2_Atomic_Property type;
   Ecore_Drm2_Atomic_Property cid, fid;
   Ecore_Drm2_Atomic_Property sx, sy, sw, sh;
   Ecore_Drm2_Atomic_Property cx, cy, cw, ch;
   Ecore_Drm2_Atomic_Property rotation;

   /* NB: these are not part of an atomic state, but we store these here
    * so that we do not have to refetch properties when iterating planes */
   uint32_t rotation_map[6];
   uint32_t supported_rotations;

   uint32_t num_formats;
   uint32_t *formats;

   Eina_Bool in_use;

   /* TODO ?? */
   /* Ecore_Drm2_Atomic_Property IN_FENCE_FD; */
   /* Ecore_Drm2_Atomic_Property IN_FORMATS; */
   /* Ecore_Drm2_Atomic_Property COLOR_ENCODING; */
   /* Ecore_Drm2_Atomic_Property COLOR_RANGE; */
} Ecore_Drm2_Plane_State;

typedef struct _Ecore_Drm2_Atomic_State
{
   int crtcs, conns, planes;
   Ecore_Drm2_Crtc_State *crtc_states;
   Ecore_Drm2_Connector_State *conn_states;
   Ecore_Drm2_Plane_State *plane_states;
} Ecore_Drm2_Atomic_State;

/* opaque API structures */
struct _Ecore_Drm2_Device
{
   Elput_Manager *em;

   int fd;

   Ecore_Drm2_Atomic_State *atomic_state;
};

struct _Ecore_Drm2_Output
{
   Eina_Stringshare *name, *make, *model, *serial;
};

/* internal function prototypes */
Eina_Bool _ecore_drm2_atomic_state_fill(Ecore_Drm2_Atomic_State *state, int fd);

/* external drm function prototypes (for dlopen) */
extern void *(*sym_drmModeGetResources)(int fd);
extern void (*sym_drmModeFreeResources)(drmModeResPtr ptr);
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

#endif

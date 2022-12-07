#ifndef _ECORE_DRM2_H
# define _ECORE_DRM2_H

# ifdef EAPI
#  undef EAPI
# endif

# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else // if __GNUC__ >= 4
#   define EAPI
#  endif // if __GNUC__ >= 4
# else // ifdef __GNUC__
#  define EAPI
# endif // ifdef __GNUC__

# ifdef EFL_BETA_API_SUPPORT

/* public enum for rotation */
typedef enum _Ecore_Drm2_Rotation
{
   ECORE_DRM2_ROTATION_NORMAL = 1,
   ECORE_DRM2_ROTATION_90 = 2,
   ECORE_DRM2_ROTATION_180 = 4,
   ECORE_DRM2_ROTATION_270 = 8,
   ECORE_DRM2_ROTATION_REFLECT_X = 16,
   ECORE_DRM2_ROTATION_REFLECT_Y = 32
} Ecore_Drm2_Rotation;

/* opaque structure to represent a drm crtc */
typedef struct _Ecore_Drm2_Crtc Ecore_Drm2_Crtc;

/* opaque structure to represent a drm connector */
typedef struct _Ecore_Drm2_Connector Ecore_Drm2_Connector;

/* opaque structure to represent a drm display mode */
typedef struct _Ecore_Drm2_Display_Mode Ecore_Drm2_Display_Mode;

/* opaque structure to represent a drm display */
typedef struct _Ecore_Drm2_Display Ecore_Drm2_Display;

/* opaque structure to represent a drm plane */
typedef struct _Ecore_Drm2_Plane Ecore_Drm2_Plane;

/* opaque structure to represent a drm device */
typedef struct _Ecore_Drm2_Device Ecore_Drm2_Device;

/* API functions */
EAPI int ecore_drm2_init(void);
EAPI int ecore_drm2_shutdown(void);

/* Device API functions */
EAPI Ecore_Drm2_Device *ecore_drm2_device_open(const char *seat, unsigned int tty);
EAPI void ecore_drm2_device_close(Ecore_Drm2_Device *dev);
EAPI void ecore_drm2_device_cursor_size_get(Ecore_Drm2_Device *dev, int *width, int *height);
EAPI void ecore_drm2_device_preferred_depth_get(Ecore_Drm2_Device *dev, int *depth, int *bpp);
EAPI void ecore_drm2_device_screen_size_range_get(Ecore_Drm2_Device *dev, int *minw, int *minh, int *maxw, int *maxh);

/* Display API functions */
EAPI char *ecore_drm2_display_name_get(Ecore_Drm2_Display *disp);

/* XXX: These are 'test' APIs */
EAPI void ecore_drm2_display_mode_set(Ecore_Drm2_Display *disp, Ecore_Drm2_Display_Mode *mode, int x, int y);

# endif

#endif

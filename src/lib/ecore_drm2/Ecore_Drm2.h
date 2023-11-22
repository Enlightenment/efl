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

/* public enum for relative mode */
typedef enum _Ecore_Drm2_Relative_Mode
{
   ECORE_DRM2_RELATIVE_MODE_UNKNOWN,
   ECORE_DRM2_RELATIVE_MODE_NONE,
   ECORE_DRM2_RELATIVE_MODE_CLONE,
   ECORE_DRM2_RELATIVE_MODE_TO_LEFT,
   ECORE_DRM2_RELATIVE_MODE_TO_RIGHT,
   ECORE_DRM2_RELATIVE_MODE_TO_ABOVE,
   ECORE_DRM2_RELATIVE_MODE_TO_BELOW
} Ecore_Drm2_Relative_Mode;

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
EAPI const Eina_List *ecore_drm2_device_crtcs_get(Ecore_Drm2_Device *dev);
EAPI void ecore_drm2_device_pointer_max_set(Ecore_Drm2_Device *dev, int w, int h);
EAPI Eina_Bool ecore_drm2_device_pointer_rotation_set(Ecore_Drm2_Device *dev, int rotation);
EAPI void ecore_drm2_device_calibrate(Ecore_Drm2_Device *dev, int w, int h);

/* Display API functions */
EAPI char *ecore_drm2_display_name_get(Ecore_Drm2_Display *disp);
EAPI char *ecore_drm2_display_model_get(Ecore_Drm2_Display *disp);
EAPI Eina_Bool ecore_drm2_display_backlight_get(Ecore_Drm2_Display *disp);
EAPI Eina_Bool ecore_drm2_display_connected_get(Ecore_Drm2_Display *disp);
EAPI unsigned int ecore_drm2_display_connector_type_get(Ecore_Drm2_Display *disp);
EAPI unsigned int ecore_drm2_display_subpixel_get(Ecore_Drm2_Display *disp);
EAPI void ecore_drm2_display_physical_size_get(Ecore_Drm2_Display *disp, int *w, int *h);
EAPI int ecore_drm2_display_dpms_get(Ecore_Drm2_Display *disp);
EAPI void ecore_drm2_display_dpms_set(Ecore_Drm2_Display *disp, uint64_t level);
EAPI Eina_Bool ecore_drm2_display_enabled_get(Ecore_Drm2_Display *disp);
EAPI void ecore_drm2_display_enabled_set(Ecore_Drm2_Display *disp, Eina_Bool enabled);
EAPI char *ecore_drm2_display_edid_get(Ecore_Drm2_Display *disp);
EAPI const Eina_List *ecore_drm2_display_modes_get(Ecore_Drm2_Display *disp);
EAPI void ecore_drm2_display_mode_info_get(Ecore_Drm2_Display_Mode *mode, int *w, int *h, unsigned int *refresh, unsigned int *flags);
EAPI Eina_Bool ecore_drm2_display_primary_get(Ecore_Drm2_Display *disp);
EAPI void ecore_drm2_display_primary_set(Ecore_Drm2_Display *disp, Eina_Bool primary);
EAPI const Eina_List *ecore_drm2_displays_get(Ecore_Drm2_Device *dev);
EAPI void ecore_drm2_display_info_get(Ecore_Drm2_Display *disp, int *x, int *y, int *w, int *h, unsigned int *refresh);
EAPI int ecore_drm2_display_rotation_get(Ecore_Drm2_Display *disp);
EAPI void ecore_drm2_display_rotation_set(Ecore_Drm2_Display *disp, uint64_t rotation);
EAPI void ecore_drm2_display_mode_set(Ecore_Drm2_Display *disp, Ecore_Drm2_Display_Mode *mode, int x, int y);
EAPI Ecore_Drm2_Crtc *ecore_drm2_display_crtc_get(Ecore_Drm2_Display *disp);
EAPI Eina_Bool ecore_drm2_display_possible_crtc_get(Ecore_Drm2_Display *disp, Ecore_Drm2_Crtc *crtc);
EAPI int ecore_drm2_display_supported_rotations_get(Ecore_Drm2_Display *disp);
EAPI void ecore_drm2_display_relative_mode_set(Ecore_Drm2_Display *disp, Ecore_Drm2_Relative_Mode mode);
EAPI void ecore_drm2_display_relative_to_set(Ecore_Drm2_Display *disp, const char *relative);

# endif

#endif

#ifndef _ECORE_DRM2_H
# define _ECORE_DRM2_H

# include <Ecore.h>

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

typedef enum _Ecore_Drm2_Rotation
{
   ECORE_DRM2_ROTATION_NORMAL = 1,
   ECORE_DRM2_ROTATION_90 = 2,
   ECORE_DRM2_ROTATION_180 = 4,
   ECORE_DRM2_ROTATION_270 = 8,
   ECORE_DRM2_ROTATION_REFLECT_X = 16,
   ECORE_DRM2_ROTATION_REFLECT_Y = 32
} Ecore_Drm2_Rotation;

typedef enum _Ecore_Drm2_Fb_Status
{
   ECORE_DRM2_FB_STATUS_SCANOUT_ON = 1,
   ECORE_DRM2_FB_STATUS_SCANOUT_OFF = 2,
   ECORE_DRM2_FB_STATUS_RELEASE = 4,
   ECORE_DRM2_FB_STATUS_DELETED = 8,
   ECORE_DRM2_FB_STATUS_PLANE_ASSIGN = 16,
   ECORE_DRM2_FB_STATUS_PLANE_RELEASE = 32,
} Ecore_Drm2_Fb_Status;

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

/* opaque structure to represent a drm device */
typedef struct _Ecore_Drm2_Device Ecore_Drm2_Device;

/* opaque structure to represent a framebuffer object */
typedef struct _Ecore_Drm2_Fb Ecore_Drm2_Fb;

/* opaque structure to represent an output device */
typedef struct _Ecore_Drm2_Output Ecore_Drm2_Output;

/* opaque structure to represent an output mode */
typedef struct _Ecore_Drm2_Output_Mode Ecore_Drm2_Output_Mode;

/* opaque structure to represent a hardware plane */
typedef struct _Ecore_Drm2_Plane Ecore_Drm2_Plane;

/* structure to represent event for output changes */
typedef struct _Ecore_Drm2_Event_Output_Changed
{
   unsigned int id;
   int x, y, w, h;
   int phys_width, phys_height;
   unsigned int refresh, scale;
   int subpixel, transform;
   const char *make, *model, *name;
   Eina_Bool connected : 1;
   Eina_Bool enabled : 1;
} Ecore_Drm2_Event_Output_Changed;

/* structure to represent event for session state changes */
typedef struct _Ecore_Drm2_Event_Activate
{
   Eina_Bool active : 1;
} Ecore_Drm2_Event_Activate;

/* structure to represent a drm event context */
typedef struct _Ecore_Drm2_Context
{
   int version;
   void (*vblank_handler)(int fd, unsigned int sequence, unsigned int tv_sec,
                          unsigned int tv_usec, void *user_data);
   void (*page_flip_handler)(int fd, unsigned int sequence, unsigned int tv_sec,
                             unsigned int tv_usec, void *user_data);
} Ecore_Drm2_Context;

EAPI extern int ECORE_DRM2_EVENT_OUTPUT_CHANGED;
EAPI extern int ECORE_DRM2_EVENT_ACTIVATE;

typedef void (*Ecore_Drm2_Release_Handler)(void *data, Ecore_Drm2_Fb *b);
typedef void (*Ecore_Drm2_Fb_Status_Handler)(Ecore_Drm2_Fb *b, Ecore_Drm2_Fb_Status status, void *data);

/**
 * @file
 * @brief Ecore functions for dealing with drm, virtual terminals
 *
 * @defgroup Ecore_Drm2_Group Ecore_Drm2 - Drm Integration
 * @ingroup Ecore
 *
 * Ecore_Drm2 provides a wrapper and functions for using libdrm
 *
 * @li @ref Ecore_Drm2_Init_Group
 * @li @ref Ecore_Drm2_Device_Group
 * @li @ref Ecore_Drm2_Output_Group
 * @li @ref Ecore_Drm2_Fb_Group
 * @li @ref Ecore_Drm2_Plane_Group
 */

/**
 * @defgroup Ecore_Drm2_Init_Group Drm library Init and Shutdown functions
 *
 * Functions that start and shutdown the Ecore_Drm2 library
 */

/**
 * Initialize the Ecore_Drm2 library
 *
 * @return  The number of times the library has been initialized without
 *          being shut down. 0 is returned if an error occurs.
 *
 * @ingroup Ecore_Drm2_Init_Group
 * @since 1.18
 */
EAPI int ecore_drm2_init(void);

/**
 * Shutdown the Ecore_Drm2 library
 *
 * @return  The number of times the library has been initialized without
 *          being shutdown. 0 is returned if an error occurs.
 *
 * @ingroup Ecore_Drm2_Init_Group
 * @since 1.18
 */
EAPI int ecore_drm2_shutdown(void);

/**
 * Read and process pending Drm events
 *
 * @param dev drm device
 * @param ctx
 *
 * @return 0 on success, -1 otherwise
 *
 * @note: Do not ever use this function in applications !!!
 * This is a special-purpose API function and should not be used by
 * application developers.
 *
 * @ingroup Ecore_Drm_Init_Group
 * @since 1.19
 */
EAPI int ecore_drm2_event_handle(Ecore_Drm2_Device *dev, Ecore_Drm2_Context *drmctx);

/**
 * @defgroup Ecore_Drm2_Device_Group Drm device functions
 *
 * Functions that deal with finding, opening, closing, or obtaining various
 * information about a drm device
 */

/**
 * Try to open the Ecore_Drm2_Device for a given seat
 *
 * @param seat
 * @param tty
 *
 * @return An Ecore_Drm2_Device or NULL on failure.
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI Ecore_Drm2_Device *ecore_drm2_device_open(const char *seat, unsigned int tty);

/**
 * Close an open Ecore_Drm2_Device
 *
 * @param device
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_close(Ecore_Drm2_Device *device);

/**
 * Get the type of clock used by a given Ecore_Drm2_Device
 *
 * @param device
 *
 * @return The clockid_t used by this drm device
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI int ecore_drm2_device_clock_id_get(Ecore_Drm2_Device *device);

/**
 * Get the size of the cursor supported by a given Ecore_Drm2_Device
 *
 * @param device
 * @param width
 * @param height
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_cursor_size_get(Ecore_Drm2_Device *device, int *width, int *height);

/**
 * Get the current pointer position
 *
 * @param device
 * @param x
 * @param y
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_pointer_xy_get(Ecore_Drm2_Device *device, int *x, int *y);

/**
 * Warp the pointer position to given coordinates
 *
 * @param dev
 * @param x
 * @param y
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_pointer_warp(Ecore_Drm2_Device *device, int x, int y);

/**
 * Set a left handed mode for the given device
 *
 * @param device
 * @param left
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_device_pointer_left_handed_set(Ecore_Drm2_Device *device, Eina_Bool left);

/**
 * Set which window is to be used for input events
 *
 * @param device
 * @param window
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_window_set(Ecore_Drm2_Device *device, unsigned int window);

/**
 * Set maximium position that pointer device is allowed to move
 *
 * @param device
 * @param w
 * @param h
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_pointer_max_set(Ecore_Drm2_Device *device, int w, int h);

/**
 * Set pointer acceleration speed
 *
 * @param device
 * @param speed
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.21
 */
EAPI void ecore_drm2_device_pointer_accel_speed_set(Ecore_Drm2_Device *device, double speed);

/**
 * Set pointer acceleration profile
 *
 * @param device
 * @param profile
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.21
 */
EAPI void ecore_drm2_device_pointer_accel_profile_set(Ecore_Drm2_Device *device, uint32_t profile);

/**
 * Set pointer value rotation
 *
 * @param device
 * @param rotation
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.20
 */
EAPI Eina_Bool ecore_drm2_device_pointer_rotation_set(Ecore_Drm2_Device *device, int rotation);

/**
 * Enable or disable pointer tap-to-click
 *
 * @param device
 * @param enabled
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.22
 */
EAPI void ecore_drm2_device_touch_tap_to_click_enabled_set(Ecore_Drm2_Device *device, Eina_Bool enabled);

/**
 * Set info to be used on keyboards
 *
 * @param device
 * @param context
 * @param keymap
 * @param group
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.20
 */
EAPI void ecore_drm2_device_keyboard_info_set(Ecore_Drm2_Device *device, void *context, void *keymap, int group);

/**
 * Set a group layout to be used on keyboards
 *
 * @param device
 * @param group
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.20
 */
EAPI void ecore_drm2_device_keyboard_group_set(Ecore_Drm2_Device *device, int group);

/**
 * Get the crtcs of a given device
 *
 * @param device
 * @param num
 *
 * @return The crtcs of this given device or NULL on failure
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI unsigned int *ecore_drm2_device_crtcs_get(Ecore_Drm2_Device *device, int *num);

/**
 * Get the minimum and maximum screen size range
 *
 * @param device
 * @param *minw
 * @param *minh
 * @param *maxw
 * @param *maxh
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_screen_size_range_get(Ecore_Drm2_Device *device, int *minw, int *minh, int *maxw, int *maxh);

/**
 * Calibrate any input devices for given screen size
 *
 * @param device
 * @param w
 * @param h
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_calibrate(Ecore_Drm2_Device *device, int w, int h);

/**
 * Try to switch to a given virtual terminal
 *
 * @param device
 * @param vt
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_device_vt_set(Ecore_Drm2_Device *device, int vt);

/**
 * Get if a given device prefers the use of shadow buffers
 *
 * @param device
 *
 * @return EINA_TRUE if preferred, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.19
 */
EAPI Eina_Bool ecore_drm2_device_prefer_shadow(Ecore_Drm2_Device *device);

/**
 * @defgroup Ecore_Drm2_Output_Group Drm output functions
 *
 * Functions that deal with setup of outputs
 */

/**
 * Iterate drm resources and create outputs
 *
 * @param device
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_outputs_create(Ecore_Drm2_Device *device);

/**
 * Destroy any created outputs
 *
 * @param device
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI void ecore_drm2_outputs_destroy(Ecore_Drm2_Device *device);

/**
 * Get the list of outputs from a drm device
 *
 * @param device
 *
 * @return
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI const Eina_List *ecore_drm2_outputs_get(Ecore_Drm2_Device *device);

/**
 * Get the dpms level of a given output
 *
 * @param output
 *
 * @return Integer value representing the state of DPMS on a given output
 *         or -1 on error
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI int ecore_drm2_output_dpms_get(Ecore_Drm2_Output *output);

/**
 * Set the dpms level of a given output
 *
 * @param output
 * @param level
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI void ecore_drm2_output_dpms_set(Ecore_Drm2_Output *output, int level);

/**
 * Get the edid of a given output
 *
 * @param output
 *
 * @return A string representing the edid
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI char *ecore_drm2_output_edid_get(Ecore_Drm2_Output *output);

/**
 * Get if a given output has a backlight
 *
 * @param output
 *
 * @return EINA_TRUE if this output has a backlight, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_output_backlight_get(Ecore_Drm2_Output *output);

/**
 * Find an output at the given position
 *
 * @param device
 * @param x
 * @param y
 *
 * @return An Ecore_Drm2_Output which exists at the given coordinates, or NULL on failure
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Ecore_Drm2_Output *ecore_drm2_output_find(Ecore_Drm2_Device *device, int x, int y);

/**
 * Get the dpi of a given output
 *
 * @param output
 * @param xdpi
 * @param ydpi
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.19
 */
EAPI void ecore_drm2_output_dpi_get(Ecore_Drm2_Output *output, int *xdpi, int *ydpi);

/**
 * Get the id of the crtc that an output is using
 *
 * @param output
 *
 * @return A valid crtc id or 0 on failure
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI unsigned int ecore_drm2_output_crtc_get(Ecore_Drm2_Output *output);

/**
 * Return the most recently set Ecore_Drm2_Fb for a given output
 *
 * This may be the currently scanned out buffer, a buffer currently being
 * flipped to scanout, or a buffer that has been submit but may not
 * actually ever hit scanout at all.
 *
 * @param output
 *
 * @return The latest Ecore_Drm2_Fb submit for this output, or NULL otherwise
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.19
 */
EAPI Ecore_Drm2_Fb *ecore_drm2_output_latest_fb_get(Ecore_Drm2_Output *output);

/**
 * Get if a given output is marked as the primary output
 *
 * @param output
 *
 * @return EINA_TRUE if output is primary, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_output_primary_get(Ecore_Drm2_Output *output);

/**
 * Set a given output to be primary
 *
 * @param output
 * @param primary
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI void ecore_drm2_output_primary_set(Ecore_Drm2_Output *output, Eina_Bool primary);

/**
 * Get if a given output is enabled
 *
 * @param output
 *
 * @return EINA_TRUE if enabled, EINA_FALSE otherwise.
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_output_enabled_get(Ecore_Drm2_Output *output);

/**
 * Set if a given output is enabled
 *
 * @param output
 * @param enabled
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI void ecore_drm2_output_enabled_set(Ecore_Drm2_Output *output, Eina_Bool enabled);

/**
 * Get the physical size of a given output
 *
 * This function will give the physical size (in mm) of an output
 *
 * @param output
 * @param *w
 * @param *h
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI void ecore_drm2_output_physical_size_get(Ecore_Drm2_Output *output, int *w, int *h);

/**
 * Get a list of the modes supported on a given output
 *
 * @param output
 *
 * @return An Eina_List of the modes supported for this output
 *
 * @note The returned list should not be freed
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI const Eina_List *ecore_drm2_output_modes_get(Ecore_Drm2_Output *output);

/**
 * Get information from an existing output mode
 *
 * @param mode
 * @param w
 * @param h
 * @param refresh
 * @param flags
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI void ecore_drm2_output_mode_info_get(Ecore_Drm2_Output_Mode *mode, int *w, int *h, unsigned int *refresh, unsigned int *flags);

/**
 * Set a given mode to be used on a given output
 *
 * @param output
 * @param mode
 * @param x
 * @param y
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_output_mode_set(Ecore_Drm2_Output *output, Ecore_Drm2_Output_Mode *mode, int x, int y);

/**
 * Get the name of a given output
 *
 * @param output
 *
 * @return A string representing the output's name. Caller should free this return.
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI char *ecore_drm2_output_name_get(Ecore_Drm2_Output *output);

/**
 * Get the model of a given output
 *
 * @param output
 *
 * @return A string representing the output's model. Caller should free this return.
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI char *ecore_drm2_output_model_get(Ecore_Drm2_Output *output);

/**
 * Get if a given output is connected
 *
 * @param output
 *
 * @return EINA_TRUE if connected, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_output_connected_get(Ecore_Drm2_Output *output);

/**
 * Get if a given output is cloned
 *
 * @param output
 *
 * @return EINA_TRUE if cloned, EINA_FALSE otherwise.
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_output_cloned_get(Ecore_Drm2_Output *output);

/**
 * Get the connector type of a given output
 *
 * @param output
 *
 * @return An unsigned integer representing the type of connector for this output
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI unsigned int ecore_drm2_output_connector_type_get(Ecore_Drm2_Output *output);

/**
 * Get the geometry and refresh rate for a given output
 *
 * @param output
 * @param *x
 * @param *y
 * @param *w
 * @param *h
 * @param *refresh
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.21
 */
EAPI void ecore_drm2_output_info_get(Ecore_Drm2_Output *output, int *x, int *y, int *w, int *h, unsigned int *refresh);

/**
 * Get if an output can be used on a given crtc
 *
 * This function will loop the possible crtcs of an encoder to determine if
 * a given output can be assigned to a given crtc
 *
 * @param output
 * @param crtc
 *
 * @return EINA_TRUE if the output can be assigned to given crtc, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_output_possible_crtc_get(Ecore_Drm2_Output *output, unsigned int crtc);

/**
 * Set the gamma level of an Ecore_Drm_Output
 *
 * This function will set the gamma of an Ecore_Drm2_Output
 *
 * @param output The Ecore_Drm2_Output to set the gamma level on
 * @param size The gamma table size to set
 * @param red The amount to scale the red channel
 * @param green The amount to scale the green channel
 * @param blue The amount to scale the blue channel
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.19
 */
EAPI void ecore_drm2_output_gamma_set(Ecore_Drm2_Output *output, uint16_t size, uint16_t *red, uint16_t *green, uint16_t *blue);

/**
 * Get the supported rotations of a given output
 *
 * @param output
 *
 * @return An integer representing possible rotations, or -1 on failure
 *
 * @note This function will only return valid values if Atomic support
 *       is enabled as it requires hardware plane support.
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.19
 */
EAPI int ecore_drm2_output_supported_rotations_get(Ecore_Drm2_Output *output);

/**
 * Set a rotation on a given output
 *
 * @param output
 * @param rotation
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @note This function will only work if Atomic support
 *       is enabled as it requires hardware plane support.
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.19
 */
EAPI Eina_Bool ecore_drm2_output_rotation_set(Ecore_Drm2_Output *output, int rotation);

/**
 * Get current output rotation
 *
 * @param output
 *
 * @return An integer representing the output current rotation
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.22
 */
EAPI int ecore_drm2_output_rotation_get(Ecore_Drm2_Output *output);

/**
 * Set the user data for the output's page flip handler
 *
 * @param output The output to update user data for
 * @param data The new user data pointer
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.19
 */
EAPI void ecore_drm2_output_user_data_set(Ecore_Drm2_Output *o, void *data);

/**
 * Get the user data for a given output
 *
 * @param output The output to get user data for
 *
 * @return The user data associated with given output
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.21
 */
EAPI void *ecore_drm2_output_user_data_get(Ecore_Drm2_Output *output);

/**
 * Get the subpixel state of the output
 * @param output the output
 * @return The state value
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.20
 */
EAPI unsigned int ecore_drm2_output_subpixel_get(const Ecore_Drm2_Output *output);

/**
 * Set the relative mode for an output
 *
 * @param output The output to set relative mode
 * @param relative The relative mode to set
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.21
 */
EAPI void ecore_drm2_output_relative_mode_set(Ecore_Drm2_Output *output, Ecore_Drm2_Relative_Mode mode);

/**
 * Get the relative mode of an output
 *
 * @param output The output to retrieve relative mode for
 *
 * @return The relative mode of a given output
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.21
 */
EAPI Ecore_Drm2_Relative_Mode ecore_drm2_output_relative_mode_get(Ecore_Drm2_Output *output);

/**
 * Set which output a given output is relative to
 *
 * @param output The output for which to set relative
 * @param relative The output for which the first output is relative to
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.21
 */
EAPI void ecore_drm2_output_relative_to_set(Ecore_Drm2_Output *output, const char *relative);

/**
 * Get which output is relative to a given output
 *
 * @param output The output for which to retrieve relative
 *
 * @return The name of the output which is relative to the given output or NULL
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.21
 */
EAPI const char *ecore_drm2_output_relative_to_get(Ecore_Drm2_Output *output);

/**
 * @defgroup Ecore_Drm2_Fb_Group Drm framebuffer functions
 *
 * Functions that deal with setup of framebuffers
 */

/**
 * Create a new framebuffer object
 *
 * @param dev
 * @param width
 * @param height
 * @param depth
 * @param bpp
 * @param format
 *
 * @return A newly create framebuffer object, or NULL on failure
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.18
 */
EAPI Ecore_Drm2_Fb *ecore_drm2_fb_create(Ecore_Drm2_Device *dev, int width, int height, int depth, int bpp, unsigned int format);

EAPI Ecore_Drm2_Fb *ecore_drm2_fb_gbm_create(Ecore_Drm2_Device *dev, int width, int height, int depth, int bpp, unsigned int format, unsigned int handle, unsigned int stride, void *bo);

/**
 * Get a framebuffer's mmap'd data
 *
 * @param fb
 *
 * @return The mmap'd area of the framebuffer or NULL on failure
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.18
 */
EAPI void *ecore_drm2_fb_data_get(Ecore_Drm2_Fb *fb);

/**
 * Get a framebuffer's size
 *
 * @param fb
 *
 * @return size of the framebuffers' mmap'd data or 0 on failure
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.18
 */
EAPI unsigned int ecore_drm2_fb_size_get(Ecore_Drm2_Fb *fb);

/**
 * Get a framebuffer's stride
 *
 * @param fb
 *
 * @return stride of the framebuffer or 0 on failure
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.18
 */
EAPI unsigned int ecore_drm2_fb_stride_get(Ecore_Drm2_Fb *fb);

/**
 * Mark regions of a framebuffer as dirty
 *
 * @param fb
 * @param rects
 * @param count
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.18
 */
EAPI void ecore_drm2_fb_dirty(Ecore_Drm2_Fb *fb, Eina_Rectangle *rects, unsigned int count);

/**
 * Schedule a pageflip to the given Ecore_Drm2_Fb
 *
 * The caller is responsible for running a page flip handler
 * and calling ecore_drm2_fb_flip_complete() when it completes.
 *
 * @param fb
 * @param output
 *
 * @return The result of drmModePageFlip function call
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.18
 */
EAPI int ecore_drm2_fb_flip(Ecore_Drm2_Fb *fb, Ecore_Drm2_Output *output);

/**
 * Must be called by a page flip handler when the flip completes.
 *
 * @param output
 *
 * @return Whether there's an undisplayed buffer still in the queue.
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.18
 */
EAPI Eina_Bool ecore_drm2_fb_flip_complete(Ecore_Drm2_Output *output);

/**
 * Return the Ecore_Drm2_Fb's busy status
 *
 * @param fb
 *
 * @return The busy status
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.19
 */
EAPI Eina_Bool ecore_drm2_fb_busy_get(Ecore_Drm2_Fb *fb);

/**
 * Try to force a framebuffer release for an output
 *
 * This tries to release the next or optionally pending, or current
 * buffer from the output.  If successful there will be a release callback
 * to the registered handler, and the fb will no longer be flagged busy.
 *
 * Releasing buffers committed to scanout will potentially cause flicker,
 * so this is only done when the panic flag is set.
 *
 * @param output The output to force release
 * @param panic Try to release even buffers committed to scanout
 *
 * @return EINA_TRUE if a buffer was released
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.19
 */
EAPI Eina_Bool ecore_drm2_fb_release(Ecore_Drm2_Output *o, Eina_Bool panic);

/**
 * Get the Framebuffer's gbm buffer object
 *
 * @param fb The framebuffer to query
 *
 * @return The gbm bo for the framebuffer
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.19
 */
EAPI void *ecore_drm2_fb_bo_get(Ecore_Drm2_Fb *fb);

/**
 * Import a dmabuf object as a Framebuffer
 *
 * @param dev
 * @param width
 * @param height
 * @param depth
 * @param bpp
 * @param format
 * @param stride
 * @param dmabuf_fd
 * @param dmabuf_fd_count
 *
 * @return A newly created framebuffer object, or NULL on failure
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.20
 *
 */
EAPI Ecore_Drm2_Fb *ecore_drm2_fb_dmabuf_import(Ecore_Drm2_Device *dev, int width, int height, int depth, int bpp, unsigned int format, unsigned int strides[4], int dmabuf_fd[4], int dmabuf_fd_count);

/**
 * Discard a framebuffer object
 *
 * Decreases the refcount on a fb object.  It will be destroyed when it's
 * no longer attached to scanout or otherwise in use.
 *
 * @param fb
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.20
 */
EAPI void ecore_drm2_fb_discard(Ecore_Drm2_Fb *fb);

/**
 * @defgroup Ecore_Drm2_Plane_Group Functions that deal with hardware planes
 *
 * Functions that deal with hardware plane manipulation
 */

/**
 * Find a hardware plane where a given Ecore_Drm2_Fb can go based on format and size
 *
 * @param output
 * @param fb
 *
 * @return A newly allocated plane object, or NULL otherwise
 *
 * @ingroup Ecore_Drm2_Plane_Group
 * @since 1.20
 */
EAPI Ecore_Drm2_Plane *ecore_drm2_plane_assign(Ecore_Drm2_Output *output, Ecore_Drm2_Fb *fb, int x, int y);

/**
 * Remove a hardware plane from display
 *
 * @param plane
 *
 * @ingroup Ecore_Drm2_Plane_Group
 * @since 1.20
 */
EAPI void ecore_drm2_plane_release(Ecore_Drm2_Plane *plane);

/**
 * Set plane destination values
 *
 * @param plane
 * @param x
 * @param y
 * @param w
 * @param h
 *
 * @ingroup Ecore_Drm2_Plane_Group
 * @since 1.20
 */
EAPI void ecore_drm2_plane_destination_set(Ecore_Drm2_Plane *plane, int x, int y, int w, int h);

/**
 * Set plane frame buffer
 *
 * @param plane
 * @param fb
 *
 * @return whether the plane state has been successfully changed or not
 *
 * @ingroup Ecore_Drm2_Plane_Group
 * @since 1.20
 */
EAPI Eina_Bool ecore_drm2_plane_fb_set(Ecore_Drm2_Plane *plane, Ecore_Drm2_Fb *fb);

/**
 * Register a callback for buffer status updates
 *
 * When a flip completes ecore_drm2 may release a buffer.  Use this callback
 * if you need to do bookkeeping or locking on buffer release.
 *
 * Additionally, an fb may be placed on scanout or removed from scanout by
 * evas.  When this happens a compositor needs to ensure the buffers aren't
 * released back to a client while they're on scanout.
 *
 * @param fb The fb to register the callback on
 * @param handler The function to handle the callback
 * @param data The user data to pass to the callback
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.20
 */
EAPI void ecore_drm2_fb_status_handler_set(Ecore_Drm2_Fb *fb, Ecore_Drm2_Fb_Status_Handler handler, void *data);

/**
 * Get the time of the last vblank
 *
 * Query the display hardware for the time of a vblank, potentially blocking.
 *
 * If sequence is 0 the time of the last vblank will be immediately returned,
 * if it's above zero that number of vblanks will pass before the function
 * returns.
 *
 * @param output
 * @param sequence
 * @param sec
 * @param usec
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.20
 */
EAPI Eina_Bool ecore_drm2_output_blanktime_get(Ecore_Drm2_Output *output, int sequence, long *sec, long *usec);

/**
 * Get the fd of an Ecore_Drm2_Device
 *
 * Query the fd of the device.
 *
 * @param device
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.20
 */
EAPI int ecore_drm2_device_fd_get(Ecore_Drm2_Device *device);

/**
 * Check if there's a pageflip in progress for an output
 *
 * Checks whether an output has submit a flip but not yet had
 * a callback completion event for that flip yet.
 *
 * @param output
 * @return Whether there's a flip in progress or not
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.20
 */
EAPI Eina_Bool ecore_drm2_output_pending_get(Ecore_Drm2_Output *output);

/**
 * Set the background color of an output's crtc
 *
 * @param output
 * @param r
 * @param g
 * @param b
 * @param a
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise
 *
 * @note This requires support from the video driver in order to function
 *
 * @since 1.23
 */
EAPI Eina_Bool ecore_drm2_output_background_color_set(Ecore_Drm2_Output *output, uint64_t r, uint64_t g, uint64_t b, uint64_t a);

/**
 * Check if vblank is supported by the current video driver
 *
 * @param dev
 *
 * @return EINA_TRUE if vblank is supported, EINA_FALSE otherwise
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.23 */
EAPI Eina_Bool ecore_drm2_vblank_supported(Ecore_Drm2_Device *dev);

# endif

#endif

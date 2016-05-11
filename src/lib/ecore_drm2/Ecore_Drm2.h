#ifndef _ECORE_DRM2_H
# define _ECORE_DRM2_H

# include <Ecore.h>

# ifdef EAPI
#  undef EAPI
# endif

# ifdef _MSC_VER
#  ifdef BUILDING_DLL
#   define EAPI __declspec(dllexport)
#  else // ifdef BUILDING_DLL
#   define EAPI __declspec(dllimport)
#  endif // ifdef BUILDING_DLL
# else // ifdef _MSC_VER
#  ifdef __GNUC__
#   if __GNUC__ >= 4
#    define EAPI __attribute__ ((visibility("default")))
#   else // if __GNUC__ >= 4
#    define EAPI
#   endif // if __GNUC__ >= 4
#  else // ifdef __GNUC__
#   define EAPI
#  endif // ifdef __GNUC__
# endif // ifdef _MSC_VER

# ifdef EFL_BETA_API_SUPPORT

/* opaque structure to represent a drm device */
typedef struct _Ecore_Drm2_Device Ecore_Drm2_Device;

/* opaque structure to represent a framebuffer object */
typedef struct _Ecore_Drm2_Fb Ecore_Drm2_Fb;

/* opaque structure to represent an output device */
typedef struct _Ecore_Drm2_Output Ecore_Drm2_Output;

/* opaque structure to represent an output mode */
typedef struct _Ecore_Drm2_Output_Mode Ecore_Drm2_Output_Mode;

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

EAPI extern int ECORE_DRM2_EVENT_OUTPUT_CHANGED;
EAPI extern int ECORE_DRM2_EVENT_ACTIVATE;

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
 * @defgroup Ecore_Drm2_Device_Group Drm device functions
 *
 * Functions that deal with finding, opening, closing, or obtaining various
 * information about a drm device
 */

/**
 * Try to find a drm device on a given seat
 *
 * @param seat
 * @param tty
 * @param sync
 *
 * @return A newly allocated Ecore_Drm2_Device on success, NULL otherwise
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI Ecore_Drm2_Device *ecore_drm2_device_find(const char *seat, unsigned int tty, Eina_Bool sync);

/**
 * Try to open a given Ecore_Drm2_Device
 *
 * @param device
 *
 * @return A valid file descriptor if open succeeds, -1 otherwise.
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI int ecore_drm2_device_open(Ecore_Drm2_Device *device);

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
 * Free a given Ecore_Drm2_Device
 *
 * @param device
 *
 * @ingroup Ecore_Drm2_Device_Group
 * @since 1.18
 */
EAPI void ecore_drm2_device_free(Ecore_Drm2_Device *device);

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
 * Get the geometry of a given output
 *
 * @param output
 * @param x
 * @param y
 * @param w
 * @param h
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI void ecore_drm2_output_geometry_get(Ecore_Drm2_Output *output, int *x, int *y, int *w, int *h);

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
 * Return the next Ecore_Drm2_Fb to be used on a given output
 *
 * @param output
 *
 * @return The next Ecore_Drm2_Fb which is scheduled to to be flipped, or NULL otherwise
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI Ecore_Drm2_Fb *ecore_drm2_output_next_fb_get(Ecore_Drm2_Output *output);

/**
 * Set the next Ecore_Drm2_Fb to be used on a given output
 *
 * @param output
 * @param fb
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI void ecore_drm2_output_next_fb_set(Ecore_Drm2_Output *output, Ecore_Drm2_Fb *fb);

/**
 * Get the size of the crtc for a given output
 *
 * @param output
 * @param *w
 * @param *h
 *
 * @ingroup Ecore_Drm2_Output_Group
 * @since 1.18
 */
EAPI void ecore_drm2_output_crtc_size_get(Ecore_Drm2_Output *output, int *w, int *h);

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
 * @defgroup Ecore_Drm2_Fb_Group Drm framebuffer functions
 *
 * Functions that deal with setup of framebuffers
 */

/**
 * Create a new framebuffer object
 *
 * @param fd
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
EAPI Ecore_Drm2_Fb *ecore_drm2_fb_create(int fd, int width, int height, int depth, int bpp, unsigned int format);

EAPI Ecore_Drm2_Fb *ecore_drm2_fb_gbm_create(int fd, int width, int height, int depth, int bpp, unsigned int format, unsigned int handle, unsigned int stride);

/**
 * Destroy a framebuffer object
 *
 * @param fb
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.18
 */
EAPI void ecore_drm2_fb_destroy(Ecore_Drm2_Fb *fb);

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
 * @param fb
 * @param output
 * @param data
 *
 * @return The result of drmModePageFlip function call
 *
 * @ingroup Ecore_Drm2_Fb_Group
 * @since 1.18
 */
EAPI int ecore_drm2_fb_flip(Ecore_Drm2_Fb *fb, Ecore_Drm2_Output *output, void *data);

# endif

#endif

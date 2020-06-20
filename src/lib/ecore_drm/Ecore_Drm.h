#ifndef _ECORE_DRM_H
# define _ECORE_DRM_H

# include <xf86drm.h>
# include <xf86drmMode.h>
# include <drm_fourcc.h>
# include <Ecore.h>
# include <Eeze.h>
# include <xkbcommon/xkbcommon.h>

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

# warning The Ecore_Drm library has been deprecated. Please use the Ecore_Drm2 library

# ifdef __cplusplus
extern "C" {
# endif

# define ECORE_DRM_PLANE_ROTATION_NORMAL 1
# define ECORE_DRM_PLANE_ROTATION_90 2
# define ECORE_DRM_PLANE_ROTATION_180 4
# define ECORE_DRM_PLANE_ROTATION_270 8
# define ECORE_DRM_PLANE_ROTATION_REFLECT_X 16
# define ECORE_DRM_PLANE_ROTATION_REFLECT_Y 32

typedef enum _Ecore_Drm_Evdev_Capabilities
{
   EVDEV_KEYBOARD = (1 << 0),
   EVDEV_BUTTON = (1 << 1),
   EVDEV_MOTION_ABS = (1 << 2),
   EVDEV_MOTION_REL = (1 << 3),
   EVDEV_TOUCH = (1 << 4),
} Ecore_Drm_Evdev_Capabilities;

typedef enum _Ecore_Drm_Evdev_Event_Type
{
   EVDEV_NONE,
   EVDEV_ABSOLUTE_TOUCH_DOWN,
   EVDEV_ABSOLUTE_MOTION,
   EVDEV_ABSOLUTE_TOUCH_UP,
   EVDEV_ABSOLUTE_MT_DOWN,
   EVDEV_ABSOLUTE_MT_MOTION,
   EVDEV_ABSOLUTE_MT_UP,
   EVDEV_RELATIVE_MOTION,
} Ecore_Drm_Evdev_Event_Type;

typedef enum _Ecore_Drm_Seat_Capabilities
{
   EVDEV_SEAT_POINTER = (1 << 0),
   EVDEV_SEAT_KEYBOARD = (1 << 1),
   EVDEV_SEAT_TOUCH = (1 << 2),
} Ecore_Drm_Seat_Capabilities;

/* structure for fb objects */
typedef struct _Ecore_Drm_Fb
{
   Eina_Bool from_client : 1;
   unsigned int id, hdl;
   unsigned int stride, size;
   int fd;
   void *mmap;
   int w, h;
   Eina_Bool pending_flip : 1;
} Ecore_Drm_Fb;

struct _Ecore_Drm_Device
{
   int id;
   unsigned int vt;
   const char *seat;
   char *session;

   struct
     {
        int fd;
        const char *name;
        const char *path;
        clockid_t clock;
        Ecore_Fd_Handler *hdlr;
        Ecore_Idle_Enterer *idler;
     } drm;

   unsigned int min_width, min_height;
   unsigned int max_width, max_height;

   unsigned int crtc_count;
   unsigned int *crtcs;
   unsigned int crtc_allocator;
   unsigned int conn_allocator;

   Eina_List *seats;
   Eina_List *inputs;
   Eina_List *outputs;
   Eina_List *sprites;

   struct
     {
        int fd;
        int kbd_mode;
        const char *name;
        Ecore_Event_Handler *event_hdlr;
        Ecore_Event_Handler *switch_hdlr;
     } tty;

   unsigned int format;
   Eina_Bool use_hw_accel : 1;
   Eina_Bool cursors_broken : 1;

   struct xkb_context *xkb_ctx;

   unsigned int window;

   Eeze_Udev_Watch *watch;

   int current_fb;
   Ecore_Drm_Fb *current, *next;
   Ecore_Drm_Fb *dumb[2];

   drmEventContext drm_ctx;

   Eina_Bool active : 1;
   Eina_Bool left_handed : 1;
};

struct _Ecore_Drm_Event_Activate
{
   Eina_Bool active;
};

struct _Ecore_Drm_Event_Output
{
   unsigned int id;
   int x, y;
   int w, h;
   int phys_width, phys_height;
   unsigned int refresh;
   int subpixel_order;
   int transform;
   const char *make;
   const char *model;
   const char *name; /** @since 1.15 */
   Eina_Bool plug : 1;
};

typedef enum _Ecore_Drm_Plane_Type
{
   ECORE_DRM_PLANE_TYPE_OVERLAY,
   ECORE_DRM_PLANE_TYPE_PRIMARY,
   ECORE_DRM_PLANE_TYPE_CURSOR
} Ecore_Drm_Plane_Type;

/* opaque structure to represent a drm device */
typedef struct _Ecore_Drm_Device Ecore_Drm_Device;

/* structure to represent a drm output mode */
typedef struct _Ecore_Drm_Output_Mode
{
   unsigned int flags;
   int width, height;
   unsigned int refresh;
   drmModeModeInfo info;
} Ecore_Drm_Output_Mode;

/* opaque structure to represent a drm output */
typedef struct _Ecore_Drm_Output Ecore_Drm_Output;

/* opaque structure to represent a drm input */
typedef struct _Ecore_Drm_Input Ecore_Drm_Input;

/* opaque structure to represent a drm evdev input */
typedef struct _Ecore_Drm_Evdev Ecore_Drm_Evdev;

/* opaque structure to represent a drm seat */
typedef struct _Ecore_Drm_Seat Ecore_Drm_Seat;

/* opaque structure to represent a drm sprite */
typedef struct _Ecore_Drm_Sprite Ecore_Drm_Sprite;

/* structure to inform drm activation state */
typedef struct _Ecore_Drm_Event_Activate Ecore_Drm_Event_Activate;

/* opaque structure to represent a drm hardware plane */
/** @since 1.18 */
typedef struct _Ecore_Drm_Plane Ecore_Drm_Plane;

/* structure to inform drm output plug events */
/** @since 1.14 */
typedef struct _Ecore_Drm_Event_Output Ecore_Drm_Event_Output;

/** @since 1.14 */
typedef void (*Ecore_Drm_Pageflip_Cb)(void *data);

EAPI extern int ECORE_DRM_EVENT_ACTIVATE;

EAPI extern int ECORE_DRM_EVENT_OUTPUT; /**< @since 1.14 */

EAPI extern int ECORE_DRM_EVENT_SEAT_ADD; /**< @since 1.14 */

/**
 * @file
 * @defgroup Ecore_Drm_Group Ecore_Drm - Drm Integration
 * @ingroup Ecore
 * @brief Ecore functions for dealing with drm, virtual terminals.
 *
 * Ecore_Drm provides a wrapper and functions for using libdrm.
 *
 * @li @ref Ecore_Drm_Init_Group
 * @li @ref Ecore_Drm_Device_Group
 * @li @ref Ecore_Drm_Tty_Group
 * @li @ref Ecore_Drm_Output_Group
 * @li @ref Ecore_Drm_Input_Group
 * @li @ref Ecore_Drm_Fb_Group
 *
 */

EAPI int ecore_drm_init(void);
EAPI int ecore_drm_shutdown(void);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Finds a drm device in the system.
 *
 * @param name The name of the device to find. If NULL, this function will
 *             search for the default drm device.
 * @param seat The name of the seat where this device may be found. If NULL,
 *             this function will use a default seat name 'seat0'.
 *
 * @return An opaque Ecore_Drm_Device structure representing the card.
 *
 */
EAPI Ecore_Drm_Device *ecore_drm_device_find(const char *name, const char *seat);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Frees an Ecore_Drm_Device.
 *
 * This function will cleanup and free any previously allocated Ecore_Drm_Device.
 *
 * @param dev The Ecore_Drm_Device to free
 *
 */
EAPI void ecore_drm_device_free(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Opens an Ecore_Drm_Device.
 *
 * This function will open an existing Ecore_Drm_Device for use.
 *
 * @param dev The Ecore_Drm_Device to try and open
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool ecore_drm_device_open(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Closes an Ecore_Drm_Device.
 *
 * This function will close a previously opened Ecore_Drm_Device
 *
 * @param dev The Ecore_Drm_Device to free
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool ecore_drm_device_close(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Gets if a given Ecore_Drm_Device is master.
 *
 * This function will check if the given drm device is set to master
 *
 * @param dev The Ecore_Drm_Device to check
 *
 * @return @c EINA_TRUE if device is master, @c EINA_FALSE otherwise
 *
 */
EAPI Eina_Bool ecore_drm_device_master_get(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Sets a given Ecore_Drm_Device to master.
 *
 * This function will attempt to set a given drm device to be master
 *
 * @param dev The Ecore_Drm_Device to set
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool ecore_drm_device_master_set(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Tells a given Ecore_Drm_Device to stop being master.
 *
 * This function will attempt to ask a drm device to stop being master
 *
 * @param dev The Ecore_Drm_Device to set
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool ecore_drm_device_master_drop(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Gets the file descriptor of Ecore_Drm_Devices.
 *
 * This function will get the file descriptor of drm device
 *
 * @param dev The Ecore_Drm_Device to get
 *
 * @return fd Value on success, @c -1 on failure
 *
 */
EAPI int ecore_drm_device_fd_get(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Sets the window of Ecore_Drm_Devices.
 *
 * This function will set the window for given drm devices.
 *
 * @param dev The Ecore_Drm_Device for which window is set
 * @param window The window to set
 *
 * @since 1.10
 */
EAPI void ecore_drm_device_window_set(Ecore_Drm_Device *dev, unsigned int window);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Gets the name of the Ecore_Drm_Device.
 *
 * This function will return the name of Ecore_Drm_Device.
 *
 * @param dev The Ecore_Drm_Device to get name
 *
 * @return device name on success, NULL on failure
 *
 *
 * @since 1.10
 */
EAPI const char *ecore_drm_device_name_get(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Sets up an Ecore_Drm_Device for software rendering.
 *
 * This function will setup an Ecore_Drm_Device for software rendering
 * which includes creating dumb buffers to render into.
 *
 * @param dev The Ecore_Drm_Device to setup for software rendering
 *
 * @return EINA_TRUE on success, EINA_FALSE on failure
 *
 * @since 1.14
 */
EAPI Eina_Bool ecore_drm_device_software_setup(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Sets a left handed mode at the given Ecore_Drm_Device.
 * This function will loop all the registered inputs in Ecore_Drm_Device and
 * set/unset left handed mode.
 *
 * @param dev The Ecore_Drm_Device to set left handed mode
 * @param left_handed The flag of enable/disable left handed mode
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 *
 * @since 1.17
 */
EAPI Eina_Bool ecore_drm_device_pointer_left_handed_set(Ecore_Drm_Device *dev, Eina_Bool left_handed);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Sets up a cached context to use same context for each devices.
 * This function will setup a cached context to use same context for each devices.
 * This function will be called before initialize Ecore_Drm.
 *
 * @param ctx struct xkb_context used in libxkbcommon
 *
 * @since 1.17
 */
EAPI void ecore_drm_device_keyboard_cached_context_set(struct xkb_context *ctx);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Sets up a cached keymap to use same keymap for each devices
 * This function will setup a cached keymap to use same keymap for each devices.
 * This function will be called before initialize Ecore_Drm.
 *
 * @param map struct xkb_keymap used in libxkbcommon
 *
 * @since 1.17
 */
EAPI void ecore_drm_device_keyboard_cached_keymap_set(struct xkb_keymap *map);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Finds an Ecore_Drm_Output at the given coordinates.
 *
 * This function will loop all the existing outputs in Ecore_Drm_Device and
 * return an output if one exists that encapsulates the given coordinates.
 *
 * @param dev The Ecore_Drm_Device to search
 * @param x The x coordinate
 * @param y The y coordinate
 *
 * @return An Ecore_Drm_Output if one exists at these coordinates, or NULL
 *
 * @since 1.14
 */
EAPI Ecore_Drm_Output *ecore_drm_device_output_find(Ecore_Drm_Device *dev, int x, int y);

/**
 * @ingroup Ecore_Drm_Tty_Group
 * @brief Opens a tty for use.
 *
 * @param dev  The Ecore_Drm_Device that this tty will belong to.
 * @param name The name of the tty to try and open.
 *             If NULL, /dev/tty0 will be used.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool ecore_drm_tty_open(Ecore_Drm_Device *dev, const char *name);

/**
 * @ingroup Ecore_Drm_Tty_Group
 * @brief Closes an already opened tty.
 *
 * @param dev The Ecore_Drm_Device which owns this tty.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool ecore_drm_tty_close(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Tty_Group
 * @brief Releases a virtual terminal.
 *
 * @param dev The Ecore_Drm_Device which owns this tty.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool ecore_drm_tty_release(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Tty_Group
 * @brief Acquires a virtual terminal.
 *
 * @param dev The Ecore_Drm_Device which owns this tty.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 *
 */
EAPI Eina_Bool ecore_drm_tty_acquire(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Tty_Group
 * @brief Gets the opened virtual terminal file descriptor.
 *
 * @param dev The Ecore_Drm_Device which owns this tty.
 *
 * @return    The tty fd opened from previous call to ecore_drm_tty_open
 *
 *
 * @since 1.10
 */
EAPI int ecore_drm_tty_get(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Creates outputs for a drm device.
 *
 * This function will create outputs for Ecore_Drm_Device.
 *
 * @param dev The Ecore_Drm_Device device for which outputs
 *            needs to be created
 *
 * @return EINA_TRUE on success, EINA_FALSE on failure.
 *
 */
EAPI Eina_Bool ecore_drm_outputs_create(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Frees an Ecore_Drm_Output.
 *
 * This function will cleanup and free any previously allocated Ecore_Drm_Output.
 *
 * @param output The Ecore_Drm_Output to free
 *
 */
EAPI void ecore_drm_output_free(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Sets the cursor size for Ecore_Drm_Output.
 *
 * This function will set the cursor size of Ecore_Drm_Output.
 *
 * @param output The Ecore_Drm_Output to set
 * @param handle The Drm handle
 * @param w The width of cursor
 * @param h The height of cursor
 *
 */
EAPI void ecore_drm_output_cursor_size_set(Ecore_Drm_Output *output, int handle, int w, int h);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Enables a Ecore_Drm_Output.
 *
 * This function will enable an Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to enable
 *
 * @since 1.14
 */
EAPI Eina_Bool ecore_drm_output_enable(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Disables a Ecore_Drm_Output.
 *
 * This function will disable an Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to disable
 *
 * @since 1.14
 */
EAPI void ecore_drm_output_disable(Ecore_Drm_Output *output);

/* TODO: Doxy */
EAPI void ecore_drm_output_fb_release(Ecore_Drm_Output *output, Ecore_Drm_Fb *fb);

/* TODO: Doxy */
EAPI void ecore_drm_output_repaint(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the output size of Ecore_Drm_Device.
 *
 * This function will give the output size of Ecore_Drm_Device.
 *
 * @param dev The Ecore_Drm_Device to get output size
 * @param output The output id whose information needs to be retrieved
 * @param *w The parameter in which output width is stored
 * @param *h The parameter in which output height is stored
 *
 */
EAPI void ecore_drm_output_size_get(Ecore_Drm_Device *dev, int output, int *w, int *h);

/**
 * TODO: Doxy
 * @since 1.12
 */
EAPI void ecore_drm_outputs_geometry_get(Ecore_Drm_Device *dev, int *x, int *y, int *w, int *h);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the crtc id of an output.
 *
 * This function will return the default crtc id for an output.
 *
 * @param output The Ecore_Drm_Output to get the default crtc of
 *
 * @return The id of the default crtc for this output
 *
 * @since 1.14
 */
EAPI unsigned int ecore_drm_output_crtc_id_get(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the crtc buffer of an output.
 *
 * This function will return the default buffer id for an output.
 *
 * @param output The Ecore_Drm_Output to get the default buffer of
 *
 * @return The id of the default buffer for this output
 *
 * @since 1.14
 */
EAPI unsigned int ecore_drm_output_crtc_buffer_get(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the connector id of an output.
 *
 * This function will return the default connector id for an output.
 *
 * @param output The Ecore_Drm_Output to get the default connector of
 *
 * @return The id of the default connector id for this output
 *
 * @since 1.14
 */
EAPI unsigned int ecore_drm_output_connector_id_get(Ecore_Drm_Output *output);

/** @defgroup Ecore_Drm_Input_Group Drm input handling
 *  @{
 */
EAPI Eina_Bool ecore_drm_inputs_create(Ecore_Drm_Device *dev);
EAPI void ecore_drm_inputs_destroy(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_inputs_enable(Ecore_Drm_Input *input);
EAPI void ecore_drm_inputs_disable(Ecore_Drm_Input *input);
EAPI void ecore_drm_inputs_device_axis_size_set(Ecore_Drm_Evdev *dev, int w, int h);
/**
 * @}
 */

EAPI Eina_Bool ecore_drm_sprites_create(Ecore_Drm_Device *dev);
EAPI void ecore_drm_sprites_destroy(Ecore_Drm_Device *dev);
EAPI void ecore_drm_sprites_fb_set(Ecore_Drm_Sprite *sprite, int fb_id, int flags);
EAPI Eina_Bool ecore_drm_sprites_crtc_supported(Ecore_Drm_Output *output, unsigned int supported);

EAPI Ecore_Drm_Fb *ecore_drm_fb_create(Ecore_Drm_Device *dev, int width, int height);
EAPI void ecore_drm_fb_destroy(Ecore_Drm_Fb *fb);

/**
 * @ingroup Ecore_Drm_Fb_Group
 * @brief Marks an Ecore_Drm_Fb as dirty.
 *
 * This function mark an Ecore_Drm_Fb as being dirty.
 *
 * @param fb The Ecore_Drm_Fb to mark as dirty
 * @param rects The regions of the Ecore_Drm_Fb which are dirty
 * @param count The number of regions
 *
 * @since 1.14
 */
EAPI void ecore_drm_fb_dirty(Ecore_Drm_Fb *fb, Eina_Rectangle *rects, unsigned int count);

/**
 * @ingroup Ecore_Drm_Fb_Group
 * @brief Sets an Ecore_Drm_Fb as the current framebuffer.
 *
 * This function will set the given Ecore_Drm_Fb as the framebuffer used
 * across all outputs.
 *
 * @param dev The Ecore_Drm_Device to use
 * @param fb The Ecore_Drm_Fb to make the current framebuffer
 *
 * @deprecated just call ecore_drm_fb_send() instead.
 *
 * @since 1.14
 */
EINA_DEPRECATED EAPI void ecore_drm_fb_set(Ecore_Drm_Device *dev, Ecore_Drm_Fb *fb);

/**
 * @internal
 * @ingroup Ecore_Drm_Fb_Group
 * @brief Sends an Ecore_Drm_Fb to the Ecore_Drm_Device.
 *
 * This function will call drmModePageFlip for the given device using the
 * given Ecore_Drm_Fb as the framebuffer.
 *
 * @param dev The Ecore_Drm_Device to use
 * @param fb The Ecore_Drm_Fb to send
 * @param cb The function to call when the page flip has completed
 * @param data The data to pass to the callback function
 *
 * @since 1.14
 */
EAPI void ecore_drm_fb_send(Ecore_Drm_Device *dev, Ecore_Drm_Fb *fb, Ecore_Drm_Pageflip_Cb func, void *data);

EAPI Eina_Bool ecore_drm_launcher_connect(Ecore_Drm_Device *dev);
EAPI void ecore_drm_launcher_disconnect(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the output position of Ecore_Drm_Output.
 *
 * This function will give the output position of Ecore_Drm_Output.
 *
 * @param output The Ecore_Drm_Output to get position for
 * @param *x The parameter in which output x co-ordinate is stored
 * @param *y The parameter in which output y co-ordinate is stored
 *
 * @since 1.14
 */
EAPI void ecore_drm_output_position_get(Ecore_Drm_Output *output, int *x, int *y);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the current resolution of Ecore_Drm_Output.
 *
 * This function will give the current resolution of Ecore_Drm_Output.
 *
 * @param output The Ecore_Drm_Output to get resolution for
 * @param *w The parameter in which output width is stored
 * @param *h The parameter in which output height is stored
 * @param *refresh The parameter in which output refresh rate is stored
 *
 * @since 1.14
 */
EAPI void ecore_drm_output_current_resolution_get(Ecore_Drm_Output *output, int *w, int *h, unsigned int *refresh);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the physical size of Ecore_Drm_Output.
 *
 * This function will give the physical size (in mm) of Ecore_Drm_Output.
 *
 * @param output The Ecore_Drm_Output to get physical size for
 * @param *w The parameter in which output physical width is stored
 * @param *h The parameter in which output physical height is stored
 *
 * @since 1.14
 */
EAPI void ecore_drm_output_physical_size_get(Ecore_Drm_Output *output, int *w, int *h);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the subpixel order of Ecore_Drm_Output.
 *
 * This function will give the subpixel order of Ecore_Drm_Output.
 *
 * @param output The Ecore_Drm_Output to get subpixel order for
 * @return The output subpixel order
 *
 * @since 1.14
 */
EAPI unsigned int ecore_drm_output_subpixel_order_get(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the model of Ecore_Drm_Output.
 *
 * This function will give the model of Ecore_Drm_Output.
 *
 * @param output The Ecore_Drm_Output to get model for
 * @return The model (do NOT eina_stringshare_del this return!)
 *
 * @since 1.14
 */
EAPI Eina_Stringshare *ecore_drm_output_model_get(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the make of Ecore_Drm_Output.
 *
 * This function will give the make of Ecore_Drm_Output.
 *
 * @param output The Ecore_Drm_Output to get model for
 * @return The make (do NOT eina_stringshare_del this return!)
 *
 * @since 1.14
 */
EAPI Eina_Stringshare *ecore_drm_output_make_get(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the name of Ecore_Drm_Output.
 *
 * This function will give the name of Ecore_Drm_Output.
 *
 * @param output The Ecore_Drm_Output to get name for
 * @return The name. Caller should free this return.
 *
 * @since 1.15
 */
EAPI char *ecore_drm_output_name_get(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Sets the dpms level of an Ecore_Drm_Output.
 *
 * This function will set the DPMS level of an Ecore_Drm_Output.
 *
 * @param output The Ecore_Drm_Output to set the dpms level on
 * @param level The level to set
 *
 * @since 1.14
 */
EAPI void ecore_drm_output_dpms_set(Ecore_Drm_Output *output, int level);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Sets the gamma level of an Ecore_Drm_Output.
 *
 * This function will set the gamma of an Ecore_Drm_Output.
 *
 * @param output The Ecore_Drm_Output to set the gamma level on
 * @param size The gamma table size to set
 * @param r The amount to scale the red channel
 * @param g The amount to scale the green channel
 * @param b The amount to scale the blue channel
 *
 * @since 1.14
 */
EAPI void ecore_drm_output_gamma_set(Ecore_Drm_Output *output, uint16_t size, uint16_t *r, uint16_t *g, uint16_t *b);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Gets the pointer position of Ecore_Drm_Device.
 *
 * This function will give the pointer position of Ecore_Drm_Device.
 *
 * @param dev The Ecore_Drm_Device to get pointer position for
 * @param *x The parameter in which output x co-ordinate is stored
 * @param *y The parameter in which output y co-ordinate is stored
 *
 * @since 1.14
 */
EAPI void ecore_drm_device_pointer_xy_get(Ecore_Drm_Device *dev, int *x, int *y);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Warps the pointer position of Ecore_Drm_Device.
 *
 * This function will set the pointer position of Ecore_Drm_Device.
 *
 * @param dev The Ecore_Drm_Device to set pointer position for
 * @param x The new x co-ordinate
 * @param y The new y co-ordinate
 *
 * @since 1.18
 */
EAPI void ecore_drm_device_pointer_warp(Ecore_Drm_Device *dev, int x, int y);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Gets the list of drm devices which are allocated.
 *
 * @return Eina_List of drm devices, NULL otherwise
 *
 * @since 1.14
 */
EAPI const Eina_List *ecore_drm_devices_get(void);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Gets the minimum and maximum screen size range.
 *
 * @param dev The Ecore_Drm_Device to get screen size range from
 * @param *minw The parameter in which smallest width is stored
 * @param *minh The parameter in which smallest height is stored
 * @param *maxw The parameter in which largest width is stored
 * @param *maxh The parameter in which largest height is stored
 *
 * @since 1.15
 */
EAPI void ecore_drm_screen_size_range_get(Ecore_Drm_Device *dev, int *minw, int *minh, int *maxw, int *maxh);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets if a given output is connected.
 *
 * @param output The Ecore_Drm_Output to get the connected status of
 *
 * @return EINA_TRUE if output is connected, EINA_FALSE otherwise
 *
 * @since 1.15
 */
EAPI Eina_Bool ecore_drm_output_connected_get(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the connector type of a given Ecore_Drm_Output.
 *
 * @param output The Ecore_Drm_Output to get the connector type of
 *
 * @return An unsigned integer representing the type of connector for this output
 *
 * @since 1.15
 */
EAPI unsigned int ecore_drm_output_connector_type_get(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets if a given output has a backlight.
 *
 * @param output The Ecore_Drm_Output to get the backlight of
 *
 * @return EINA_TRUE if this output has a backlight, EINA_FALSE otherwise
 *
 * @since 1.15
 */
EAPI Eina_Bool ecore_drm_output_backlight_get(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the edid of a given output.
 *
 * @param output The Ecore_Drm_Output to get the edid of
 *
 * @return A string representing the edid
 *
 * @since 1.15
 */
EAPI char *ecore_drm_output_edid_get(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets a list of the modes supported on a given output.
 *
 * @param output The Ecore_Drm_Output to get the modes for
 *
 * @return An Eina_List of the modes supported for this output
 *
 * @note The returned list should not be freed
 *
 * @since 1.15
 */
EAPI Eina_List *ecore_drm_output_modes_get(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the output which is marked as primary.
 *
 * @param dev The Ecore_Drm_Device to get the primary output from
 *
 * @return The primary Ecore_Drm_Output or NULL if no primary output is set
 *
 * @since 1.15
 */
EAPI Ecore_Drm_Output *ecore_drm_output_primary_get(Ecore_Drm_Device *dev);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Sets a given output as primary.
 *
 * @param output The Ecore_Drm_Output to set as primary
 *
 * @since 1.15
 */
EAPI void ecore_drm_output_primary_set(Ecore_Drm_Output *output);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets the size of the crtc for a given output.
 *
 * @param output The Ecore_Drm_Output to get the crtc size of
 * @param *width The parameter in which width is stored
 * @param *height The parameter in which height is stored
 *
 * @since 1.15
 */
EAPI void ecore_drm_output_crtc_size_get(Ecore_Drm_Output *output, int *width, int *height);

/**
 * @ingroup Ecore_Drm_Device_Group
 * @brief Finds an Ecore_Drm_Output which has the given name.
 *
 * This function will loop all the existing outputs in Ecore_Drm_Device and
 * return an output if one exists that matches the given name.
 *
 * @param dev The Ecore_Drm_Device to search
 * @param name The Ecore_Drm_Output matching this name
 *
 * @return An Ecore_Drm_Output if one exists at these coordinates or NULL
 *
 * @since 1.15
 */
EAPI Ecore_Drm_Output *ecore_drm_device_output_name_find(Ecore_Drm_Device *dev, const char *name);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Gets if an Ecore_Drm_Output can be used on a given crtc.
 *
 * This function will loop the possible crtcs of an encoder to determine if
 * a given output can be assigned to a given crtc
 *
 * @param output The Ecore_Drm_Output to test if can be used on crtc
 * @param crtc The crtc to test an Ecore_Drm_Output against
 *
 * @return @c EINA_TRUE if the output can be assigned to given crtc, @c EINA_FALSE otherwise
 *
 * @since 1.15
 */
EAPI Eina_Bool ecore_drm_output_possible_crtc_get(Ecore_Drm_Output *output, unsigned int crtc);

/**
 * @ingroup Ecore_Drm_Output_Group
 * @brief Sets a given mode to be used on an Ecore_Drm_Output.
 *
 * This function will set the given mode to be used on a given Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to set the mode on
 * @param mode A valid Ecore_Drm_Output_Mode to set or NULL to disable the output
 * @param x The X position to set this output to
 * @param y The Y position to set this output to
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 *
 * @since 1.15
 */
EAPI Eina_Bool ecore_drm_output_mode_set(Ecore_Drm_Output *output, Ecore_Drm_Output_Mode *mode, int x, int y);

/* TODO: doxy */
/* @since 1.18 */
EAPI unsigned int ecore_drm_output_supported_rotations_get(Ecore_Drm_Output *output, Ecore_Drm_Plane_Type type);

/* TODO: doxy */
/* @since 1.18 */
EAPI Eina_Bool ecore_drm_output_rotation_set(Ecore_Drm_Output *output, Ecore_Drm_Plane_Type type, unsigned int rotation);

/**
 * @ingroup Ecore_Drm_Input_Group
 * @brief Enables key remap functionality on a Ecore_Drm_Evdev
 *
 * This function will enable the key remap functionality to the given Ecore_Drm_Evdev
 *
 * @param edev The Ecore_Drm_Evdev to enable the key remap on
 * @param enable A valid Eina_Bool to enable or disable the key remap on the device
 *
 * @return EINA_FALSE is returned if the Ecore_Drm_Evdev is not valid, or if no libinput device has been
 * assigned to it yet. EINA_TRUE will be returned if enabling key remap for this device succeeded.
 *
 * @since 1.17
 */
EAPI Eina_Bool ecore_drm_evdev_key_remap_enable(Ecore_Drm_Evdev *edev, Eina_Bool enable);

/**
 * @ingroup Ecore_Drm_Input_Group
 * @brief Sets a given set of keys as remapped keys on a Ecore_Drm_Evdev
 *
 * This function will create a hash table of remapping keys as a member of the given Ecore_Drm_Evdev
 *
 * @param edev The Ecore_Drm_Evdev to set the remapping keys on
 * @param from_keys A set of keycodes which contains the original keycode
 * @param to_keys A set of keycodes which contains the keycode to be remapped
 * @param num The number of keys to be applied
 *
 * @return EINA_FALSE is returned if the Ecore_Drm_Evdev is not valid, if no libinput device has been
 * assigned to it yet, if key remap is not enabled yet, or the some of the given parameters such as
 * from_keys, to_keys, num are not valid. EINA_TRUE will be returned if setting key remap for this device succeeded.
 *
 * @since 1.17
 */
EAPI Eina_Bool ecore_drm_evdev_key_remap_set(Ecore_Drm_Evdev *edev, int *from_keys, int *to_keys, int num);

# ifdef __cplusplus
}
# endif

# undef EAPI
# define EAPI

#endif

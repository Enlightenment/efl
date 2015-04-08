#ifndef _ECORE_DRM_H
# define _ECORE_DRM_H

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

# include <xf86drm.h>
# include <xf86drmMode.h>
# include <drm_fourcc.h>
# include <Ecore.h>
# include <Eeze.h>

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
   Eina_Bool plug : 1;
};

/* opaque structure to represent a drm device */
typedef struct _Ecore_Drm_Device Ecore_Drm_Device;

/* opaque structure to represent a drm output mode */
typedef struct _Ecore_Drm_Output_Mode Ecore_Drm_Output_Mode;

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

/* structure to inform drm output plug events */
/** @since 1.14 */
typedef struct _Ecore_Drm_Event_Output Ecore_Drm_Event_Output;

EAPI extern int ECORE_DRM_EVENT_ACTIVATE;

EAPI extern int ECORE_DRM_EVENT_OUTPUT; /**< @since 1.14 */

EAPI extern int ECORE_DRM_EVENT_SEAT_ADD; /**< @since 1.14 */

/**
 * @file
 * @brief Ecore functions for dealing with drm, virtual terminals
 * 
 * @defgroup Ecore_Drm_Group Ecore_Drm - Drm Integration
 * @ingroup Ecore
 * 
 * Ecore_Drm provides a wrapper and functions for using libdrm
 * 
 * @li @ref Ecore_Drm_Init_Group
 * @li @ref Ecore_Drm_Device_Group
 * @li @ref Ecore_Drm_Tty_Group
 * @li @ref Ecore_Drm_Output_Group
 * @li @ref Ecore_Drm_Input_Group
 * @li @ref Ecore_Drm_Sprite_Group
 * @li @ref Ecore_Drm_Fb_Group
 * 
 */

EAPI int ecore_drm_init(void);
EAPI int ecore_drm_shutdown(void);

/**
 * Find a drm device in the system.
 *
 * @param name The name of the device to find. If NULL, this function will 
 *             search for the default drm device.
 * @param seat The name of the seat where this device may be found. If NULL, 
 *             this function will use a default seat name 'seat0'.
 * 
 * @return An opaque Ecore_Drm_Device structure representing the card.
 * 
 * @ingroup Ecore_Drm_Device_Group
 */
EAPI Ecore_Drm_Device *ecore_drm_device_find(const char *name, const char *seat);

/**
 * Free an Ecore_Drm_Device
 *
 * This function will cleanup and free any previously allocated Ecore_Drm_Device.
 * 
 * @param dev The Ecore_Drm_Device to free
 * 
 * @ingroup Ecore_Drm_Device_Group
 */
EAPI void ecore_drm_device_free(Ecore_Drm_Device *dev);

/**
 * Open an Ecore_Drm_Device
 *
 * This function will open an existing Ecore_Drm_Device for use.
 * 
 * @param dev The Ecore_Drm_Device to try and open
 * 
 * @return EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Device_Group
 */
EAPI Eina_Bool ecore_drm_device_open(Ecore_Drm_Device *dev);

/**
 * Close an Ecore_Drm_Device
 *
 * This function will close a previously opened Ecore_Drm_Device
 * 
 * @param dev The Ecore_Drm_Device to free
 * 
 * @return EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Device_Group
 */
EAPI Eina_Bool ecore_drm_device_close(Ecore_Drm_Device *dev);

/**
 * Get if a given Ecore_Drm_Device is master
 * 
 * This function will check if the given drm device is set to master
 * 
 * @param dev The Ecore_Drm_Device to check
 * 
 * @return EINA_TRUE if device is master, EINA_FALSE otherwise
 * 
 * @ingroup Ecore_Drm_Device_Group
 */
EAPI Eina_Bool ecore_drm_device_master_get(Ecore_Drm_Device *dev);

/**
 * Set a given Ecore_Drm_Device to master
 * 
 * This function will attempt to set a given drm device to be master
 * 
 * @param dev The Ecore_Drm_Device to set
 * 
 * @return EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Device_Group
 */
EAPI Eina_Bool ecore_drm_device_master_set(Ecore_Drm_Device *dev);

/**
 * Tell a given Ecore_Drm_Device to stop being master
 * 
 * This function will attempt to ask a drm device to stop being master
 * 
 * @param dev The Ecore_Drm_Device to set
 * 
 * @return EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Device_Group
 */
EAPI Eina_Bool ecore_drm_device_master_drop(Ecore_Drm_Device *dev);

/**
 * Get the file descriptor of Ecore_Drm_Device
 *
 * This function will get the file descriptor of drm device
 *
 * @param dev The Ecore_Drm_Device to get
 *
 * @return fd value on success, -1 on failure
 *
 * @ingroup Ecore_Drm_Device_Group
 *
 */
EAPI int ecore_drm_device_fd_get(Ecore_Drm_Device *dev);

/**
 * Set the window of Ecore_Drm_Device
 *
 * This function will set the window for given drm device
 *
 * @param dev The Ecore_Drm_Device for which window is set
 * @param window The window to set
 *
 * @ingroup Ecore_Drm_Device_Group
 *
 * @since 1.10
 */
EAPI void ecore_drm_device_window_set(Ecore_Drm_Device *dev, unsigned int window);

/**
 * Get the name of the Ecore_Drm_Device
 *
 * This function will return the name of Ecore_Drm_Device
 *
 * @param dev The Ecore_Drm_Device to get name
 *
 * @return device name on success, NULL on failure
 *
 * @ingroup Ecore_Drm_Device_Group
 *
 * @since 1.10
 */
EAPI const char *ecore_drm_device_name_get(Ecore_Drm_Device *dev);

/**
 * Setup an Ecore_Drm_Device for software rendering
 *
 * This function will setup an Ecore_Drm_Device for software rendering
 * which includes creating dumb buffers to render into
 *
 * @param dev The Ecore_Drm_Device to setup for software rendering
 * @return EINA_TRUE on success, EINA_FALSE on failure
 *
 * @ingroup Ecore_Drm_Device_Group
 * @since 1.15
 */
EAPI Eina_Bool ecore_drm_device_software_setup(Ecore_Drm_Device *dev);

/**
 * Open a tty for use
 * 
 * @param dev  The Ecore_Drm_Device that this tty will belong to.
 * @param name The name of the tty to try and open. 
 *             If NULL, /dev/tty0 will be used.
 * 
 * @return     EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Tty_Group
 */
EAPI Eina_Bool ecore_drm_tty_open(Ecore_Drm_Device *dev, const char *name);

/**
 * Close an already opened tty
 * 
 * @param dev The Ecore_Drm_Device which owns this tty.
 * 
 * @return    EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Tty_Group
 */
EAPI Eina_Bool ecore_drm_tty_close(Ecore_Drm_Device *dev);

/**
 * Release a virtual terminal
 * 
 * @param dev The Ecore_Drm_Device which owns this tty.
 * 
 * @return    EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Tty_Group
 */
EAPI Eina_Bool ecore_drm_tty_release(Ecore_Drm_Device *dev);

/**
 * Acquire a virtual terminal
 * 
 * @param dev The Ecore_Drm_Device which owns this tty.
 * 
 * @return    EINA_TRUE on success, EINA_FALSE on failure
 * 
 * @ingroup Ecore_Drm_Tty_Group
 */
EAPI Eina_Bool ecore_drm_tty_acquire(Ecore_Drm_Device *dev);

/**
 * Get the opened virtual terminal file descriptor
 * 
 * @param dev The Ecore_Drm_Device which owns this tty.
 * 
 * @return    The tty fd opened from previous call to ecore_drm_tty_open
 * 
 * @ingroup Ecore_Drm_Tty_Group
 * 
 * @since 1.10
 */
EAPI int ecore_drm_tty_get(Ecore_Drm_Device *dev);

/**
 * Create outputs for a drm device
 *
 * This function will create outputs for Ecore_Drm_Device
 *
 * @param dev The Ecore_Drm_Device device for which outputs
 *            needs to be created   
 * 
 * @return EINA_TRUE on success, EINA_FALSE on failure.
 *
 * @ingroup Ecore_Drm_Output_Group
 */
EAPI Eina_Bool ecore_drm_outputs_create(Ecore_Drm_Device *dev);

/**
 * Free an Ecore_Drm_Output 
 *
 * This function will cleanup and free any previously allocated Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to free
 * 
 * @ingroup Ecore_Drm_Output_Group
 */
EAPI void ecore_drm_output_free(Ecore_Drm_Output *output);

/**
 * Set the cursor size for Ecore_Drm_Output 
 *
 * This function will set the cursor size of Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to set
 * @param handle The Drm handle
 * @param w The width of cursor
 * @param h The height of cursor
 *
 * @ingroup Ecore_Drm_Output_Group
 */
EAPI void ecore_drm_output_cursor_size_set(Ecore_Drm_Output *output, int handle, int w, int h);

/* TODO: Doxy */
EAPI Eina_Bool ecore_drm_output_enable(Ecore_Drm_Output *output);

/**
 * Disable a Ecore_Drm_Output
 *
 * This function will disable rendering on an Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to disable
 *
 * @ingroup Ecore_Drm_Output_Group
 * @since 1.15
 */
EAPI void ecore_drm_output_disable(Ecore_Drm_Output *output);

/* TODO: Doxy */
EAPI void ecore_drm_output_fb_release(Ecore_Drm_Output *output, Ecore_Drm_Fb *fb);

/* TODO: Doxy */
EAPI void ecore_drm_output_repaint(Ecore_Drm_Output *output);

/**
 * Get the output size of Ecore_Drm_Device
 *
 * This function will give the output size of Ecore_Drm_Device
 *
 * @param dev The Ecore_Drm_Device to get output size
 * @param output The output id whose information needs to be retrived 
 * @param *w The parameter in which output width is stored
 * @param *h The parameter in which output height is stored
 *
 * @ingroup Ecore_Drm_Output_Group
 */
EAPI void ecore_drm_output_size_get(Ecore_Drm_Device *dev, int output, int *w, int *h);

/**
 * TODO: Doxy
 * @since 1.12
 */
EAPI void ecore_drm_outputs_geometry_get(Ecore_Drm_Device *dev, int *x, int *y, int *w, int *h);

/* TODO: Doxy */
EAPI Eina_Bool ecore_drm_inputs_create(Ecore_Drm_Device *dev);
EAPI void ecore_drm_inputs_destroy(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_inputs_enable(Ecore_Drm_Input *input);
EAPI void ecore_drm_inputs_disable(Ecore_Drm_Input *input);
EAPI void ecore_drm_inputs_device_axis_size_set(Ecore_Drm_Evdev *dev, int w, int h);

EAPI Eina_Bool ecore_drm_sprites_create(Ecore_Drm_Device *dev);
EAPI void ecore_drm_sprites_destroy(Ecore_Drm_Device *dev);
EAPI void ecore_drm_sprites_fb_set(Ecore_Drm_Sprite *sprite, int fb_id, int flags);
EAPI Eina_Bool ecore_drm_sprites_crtc_supported(Ecore_Drm_Output *output, unsigned int supported);

EAPI Ecore_Drm_Fb *ecore_drm_fb_create(Ecore_Drm_Device *dev, int width, int height);
EAPI void ecore_drm_fb_destroy(Ecore_Drm_Fb *fb);

/**
 * Mark an Ecore_Drm_Fb as dirty
 *
 * This function mark an Ecore_Drm_Fb as being dirty
 *
 * @param fb The Ecore_Drm_Fb to mark as dirty
 * @param rects The regions of the Ecore_Drm_Fb which are dirty
 * @param count The number of regions
 * 
 * @ingroup Ecore_Drm_Fb_Group
 * @since 1.15
 */
EAPI void ecore_drm_fb_dirty(Ecore_Drm_Fb *fb, Eina_Rectangle *rects, unsigned int count);

EAPI Eina_Bool ecore_drm_launcher_connect(Ecore_Drm_Device *dev);
EAPI void ecore_drm_launcher_disconnect(Ecore_Drm_Device *dev);

/**
 * Get the output position of Ecore_Drm_Output
 *
 * This function will give the output position of Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to get position for
 * @param *x The parameter in which output x co-ordinate is stored
 * @param *y The parameter in which output y co-ordinate is stored
 *
 * @ingroup Ecore_Drm_Output_Group
 * @since 1.14
 */
EAPI void ecore_drm_output_position_get(Ecore_Drm_Output *output, int *x, int *y);

/**
 * Get the current resolution of Ecore_Drm_Output
 *
 * This function will give the current resolution of Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to get resolution for
 * @param *w The parameter in which output width is stored
 * @param *h The parameter in which output height is stored
 * @param *refresh The parameter in which output refresh rate is stored
 *
 * @ingroup Ecore_Drm_Output_Group
 * @since 1.14
 */
EAPI void ecore_drm_output_current_resolution_get(Ecore_Drm_Output *output, int *w, int *h, unsigned int *refresh);

/**
 * Get the physical size of Ecore_Drm_Output
 *
 * This function will give the physical size (in mm) of Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to get physical size for
 * @param *w The parameter in which output physical width is stored
 * @param *h The parameter in which output physical height is stored
 *
 * @ingroup Ecore_Drm_Output_Group
 * @since 1.14
 */
EAPI void ecore_drm_output_physical_size_get(Ecore_Drm_Output *output, int *w, int *h);

/**
 * Get the subpixel order of Ecore_Drm_Output
 *
 * This function will give the subpixel order of Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to get subpixel order for
 * @return The output subpixel order
 *
 * @ingroup Ecore_Drm_Output_Group
 * @since 1.14
 */
EAPI unsigned int ecore_drm_output_subpixel_order_get(Ecore_Drm_Output *output);

/**
 * Get the model of Ecore_Drm_Output
 *
 * This function will give the model of Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to get model for
 * @return The model (do NOT eina_stringshare_del this return!)
 *
 * @ingroup Ecore_Drm_Output_Group
 * @since 1.14
 */
EAPI Eina_Stringshare *ecore_drm_output_model_get(Ecore_Drm_Output *output);

/**
 * Get the make of Ecore_Drm_Output
 *
 * This function will give the make of Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to get model for
 * @return The make (do NOT eina_stringshare_del this return!)
 *
 * @ingroup Ecore_Drm_Output_Group
 * @since 1.14
 */
EAPI Eina_Stringshare *ecore_drm_output_make_get(Ecore_Drm_Output *output);

/**
 * Set the dpms level of an Ecore_Drm_Output
 *
 * This function will set the DPMS level of an Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to set the dpms level on
 * @param level The level to set
 * 
 * @ingroup Ecore_Drm_Output_Group
 * @since 1.15
 */
EAPI void ecore_drm_output_dpms_set(Ecore_Drm_Output *output, int level);

/**
 * Set the gamma level of an Ecore_Drm_Output
 *
 * This function will set the gamma of an Ecore_Drm_Output
 *
 * @param output The Ecore_Drm_Output to set the gamma level on
 * @param size The gamma table size to set
 * @param r The amount to scale the red channel
 * @param g The amount to scale the green channel
 * @param b The amount to scale the blue channel
 * 
 * @ingroup Ecore_Drm_Output_Group
 * @since 1.15
 */
EAPI void ecore_drm_output_gamma_set(Ecore_Drm_Output *output, uint16_t size, uint16_t *r, uint16_t *g, uint16_t *b);

/**
 * Get the pointer position of Ecore_Drm_Device
 *
 * This function will give the pointer position of Ecore_Drm_Device
 *
 * @param dev The Ecore_Drm_Device to get pointer position for
 * @param *x The parameter in which output x co-ordinate is stored
 * @param *y The parameter in which output y co-ordinate is stored
 *
 * @ingroup Ecore_Drm_Device_Group
 * @since 1.14
 */
EAPI void ecore_drm_device_pointer_xy_get(Ecore_Drm_Device *dev, int *x, int *y);

/**
 * Get the list of drm devices which are allocated.
 *
 * @return Eina_List of drm devices, NULL otherwise
 *
 * @ingroup Ecore_Drm_Device_Group
 * @since 1.14
 */
EAPI Eina_List *ecore_drm_devices_get(void);

#endif

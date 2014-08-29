#ifndef _ECORE_DRM_H
# define _ECORE_DRM_H

# ifdef EAPI
#  undef EAPI
# endif

#ifdef _MSC_VER
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else // ifdef BUILDING_DLL
#  define EAPI __declspec(dllimport)
# endif // ifdef BUILDING_DLL
#else // ifdef _MSC_VER
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else // if __GNUC__ >= 4
#   define EAPI
#  endif // if __GNUC__ >= 4
# else // ifdef __GNUC__
#  define EAPI
# endif // ifdef __GNUC__
#endif // ifdef _MSC_VER

#include <Ecore.h>

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
/* #ifdef HAVE_GBM */
/*    struct gbm_bo *bo; */
/* #endif */
} Ecore_Drm_Fb;

struct _Ecore_Drm_Device
{
   int id;
   const char *seat;

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

   Eina_List *seats;
   Eina_List *inputs;
   Eina_List *outputs;
   Eina_List *sprites;

   struct
     {
        int fd;
        const char *name;
        Ecore_Event_Handler *event_hdlr;
     } tty;

   unsigned int format;
   Eina_Bool use_hw_accel : 1;
   Eina_Bool cursors_broken : 1;

   struct xkb_context *xkb_ctx;

   unsigned int window;

/* #ifdef HAVE_GBM */
/*    struct gbm_device *gbm; */
/*    struct */
/*      { */
/*         EGLDisplay disp; */
/*         EGLContext ctxt; */
/*         EGLConfig cfg; */
/*      } egl; */
/* #endif */
};

/* opaque structure to represent a drm device */
typedef struct _Ecore_Drm_Device Ecore_Drm_Device;

/* opaque structure to represent a drm output mode */
typedef struct _Ecore_Drm_Output_Mode Ecore_Drm_Output_Mode;

/* opaque structure to represent a drm output */
typedef struct _Ecore_Drm_Output Ecore_Drm_Output;

/* opaque structure to represent a drm udev input */
typedef struct _Ecore_Drm_Input Ecore_Drm_Input;

/* opaque structure to represent a drm evdev input */
typedef struct _Ecore_Drm_Evdev Ecore_Drm_Evdev;

/* opaque structure to represent a drm seat */
typedef struct _Ecore_Drm_Seat Ecore_Drm_Seat;

/* opaque structure to represent a drm sprite */
typedef struct _Ecore_Drm_Sprite Ecore_Drm_Sprite;

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
 * 
 */

EAPI int ecore_drm_init(void);
EAPI int ecore_drm_shutdown(void);

/* EAPI void *ecore_drm_gbm_get(Ecore_Drm_Device *dev); */
/* EAPI unsigned int ecore_drm_gbm_format_get(Ecore_Drm_Device *dev); */

EAPI Ecore_Drm_Device *ecore_drm_device_find(const char *name, const char *seat);
EAPI void ecore_drm_device_free(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_device_open(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_device_close(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_device_master_get(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_device_master_set(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_device_master_drop(Ecore_Drm_Device *dev);
EAPI int ecore_drm_device_fd_get(Ecore_Drm_Device *dev);
EAPI void ecore_drm_device_window_set(Ecore_Drm_Device *dev, unsigned int window);
EAPI const char *ecore_drm_device_name_get(Ecore_Drm_Device *dev);

EAPI Eina_Bool ecore_drm_tty_open(Ecore_Drm_Device *dev, const char *name);
EAPI Eina_Bool ecore_drm_tty_close(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_tty_release(Ecore_Drm_Device *dev);
EAPI Eina_Bool ecore_drm_tty_acquire(Ecore_Drm_Device *dev);
EAPI int ecore_drm_tty_get(Ecore_Drm_Device *dev);

EAPI Eina_Bool ecore_drm_outputs_create(Ecore_Drm_Device *dev);
EAPI void ecore_drm_output_free(Ecore_Drm_Output *output);
EAPI void ecore_drm_output_cursor_size_set(Ecore_Drm_Output *output, int handle, int w, int h);
EAPI Eina_Bool ecore_drm_output_enable(Ecore_Drm_Output *output);
EAPI void ecore_drm_output_fb_release(Ecore_Drm_Output *output, Ecore_Drm_Fb *fb);
EAPI void ecore_drm_output_repaint(Ecore_Drm_Output *output);
EAPI void ecore_drm_output_size_get(Ecore_Drm_Device *dev, int output, int *w, int *h);

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

#endif

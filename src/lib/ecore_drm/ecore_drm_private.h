#ifndef _ECORE_DRM_PRIVATE_H
# define _ECORE_DRM_PRIVATE_H

# include "Ecore.h"
# include "ecore_private.h"
# include "Ecore_Input.h"

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <errno.h>
# include <sys/mman.h>
# include <fcntl.h>

# include <libudev.h>
# include <linux/input.h>
//# include <libinput.h>
# include <xkbcommon/xkbcommon.h>

# include <xf86drm.h>
# include <xf86drmMode.h>
# include <drm_fourcc.h>

/* # ifdef HAVE_GBM */
/* #  include <gbm.h> */
/* #  include <EGL/egl.h> */
/* #  include <EGL/eglext.h> */
/* #  include <GLES2/gl2.h> */
/* #  include <GLES2/gl2ext.h> */
/* # endif */

# include <Ecore_Drm.h>

# define NUM_FRAME_BUFFERS 2

# ifndef DRM_MAJOR
#  define DRM_MAJOR 226
# endif

# ifndef DRM_CAP_TIMESTAMP_MONOTONIC
#  define DRM_CAP_TIMESTAMP_MONOTONIC 0x6
# endif

# ifdef ECORE_DRM_DEFAULT_LOG_COLOR
#  undef ECORE_DRM_DEFAULT_LOG_COLOR
# endif
# define ECORE_DRM_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

# define EVDEV_SEAT_POINTER (1 << 0)
# define EVDEV_SEAT_KEYBOARD (1 << 1)
# define EVDEV_SEAT_TOUCH (1 << 2)

# ifdef ERR
#  undef ERR
# endif
# ifdef DBG
#  undef DBG
# endif
# ifdef INF
#  undef INF
# endif
# ifdef WRN
#  undef WRN
# endif
# ifdef CRIT
#  undef CRIT
# endif

extern int _ecore_drm_log_dom;

/* undef this for non-testing builds */
# define LOG_TO_FILE

# ifdef LOG_TO_FILE
extern FILE *lg;

#  define ERR(...) \
   EINA_LOG_DOM_ERR(_ecore_drm_log_dom, __VA_ARGS__); \
   fflush(lg);

#  define DBG(...) \
   EINA_LOG_DOM_DBG(_ecore_drm_log_dom, __VA_ARGS__); \
   fflush(lg);

#  define INF(...) \
   EINA_LOG_DOM_INFO(_ecore_drm_log_dom, __VA_ARGS__); \
   fflush(lg);

#  define WRN(...) \
   EINA_LOG_DOM_WARN(_ecore_drm_log_dom, __VA_ARGS__); \
   fflush(lg);

#  define CRIT(...) \
   EINA_LOG_DOM_CRIT(_ecore_drm_log_dom, __VA_ARGS__); \
   fflush(lg);

# else
#  define ERR(...) EINA_LOG_DOM_ERR(_ecore_drm_log_dom, __VA_ARGS__)
#  define DBG(...) EINA_LOG_DOM_DBG(_ecore_drm_log_dom, __VA_ARGS__)
#  define INF(...) EINA_LOG_DOM_INFO(_ecore_drm_log_dom, __VA_ARGS__)
#  define WRN(...) EINA_LOG_DOM_WARN(_ecore_drm_log_dom, __VA_ARGS__)
#  define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_drm_log_dom, __VA_ARGS__)
# endif

extern struct udev *udev;

struct _Ecore_Drm_Output_Mode
{
   unsigned int flags;
   int width, height;
   unsigned int refresh;
   drmModeModeInfo info;
};

struct _Ecore_Drm_Output
{
   Ecore_Drm_Device *dev;
   unsigned int crtc_id;
   unsigned int conn_id;
   drmModeCrtcPtr crtc;

   int x, y;
   int drm_fd;

   Eina_Bool need_repaint : 1;
   Eina_Bool repaint_scheduled : 1;

   Eina_Bool pending_flip : 1;
   Eina_Bool pending_vblank : 1;

   const char *make, *model, *name;
   unsigned int subpixel;

   Ecore_Drm_Output_Mode *current_mode;
   Eina_List *modes;

   Ecore_Drm_Fb *current, *next;
   Ecore_Drm_Fb *dumb[NUM_FRAME_BUFFERS];

/* # ifdef HAVE_GBM */
/*    struct gbm_surface *surface; */
/*    struct gbm_bo *cursor[NUM_FRAME_BUFFERS]; */
/*    struct  */
/*      { */
/*         EGLSurface surface; */
/*      } egl; */
/* # endif */

   /* TODO: finish */
};

struct _Ecore_Drm_Seat
{
//   struct libinput_seat *seat;
   const char *name;
   Ecore_Drm_Input *input;
   Eina_List *devices;
};

struct _Ecore_Drm_Input
{
   int fd;
   const char *seat;
   struct udev_monitor *monitor;
   Ecore_Fd_Handler *hdlr;
   Ecore_Drm_Device *dev;

   Eina_Bool enabled : 1;
   Eina_Bool suspended : 1;
};

struct _Ecore_Drm_Evdev
{
   Ecore_Drm_Seat *seat;
   /* struct libinput *linput; */
   /* struct libinput_device *dev; */
   const char *name, *path;
   int fd;

   struct 
     {
        int min_x, min_y;
        int max_x, max_y;
        int x, y;
     } abs;

   struct 
     {
        struct xkb_keymap *keymap;
        struct xkb_state *state;
        xkb_mod_mask_t ctrl_mask;
        xkb_mod_mask_t alt_mask;
        xkb_mod_mask_t shift_mask;
        xkb_mod_mask_t win_mask;
        xkb_mod_mask_t scroll_mask;
        xkb_mod_mask_t num_mask;
        xkb_mod_mask_t caps_mask;
        xkb_mod_mask_t altgr_mask;
        unsigned int modifiers;
     } xkb;

   Ecore_Drm_Evdev_Event_Type pending_event;
   Ecore_Drm_Evdev_Capabilities caps;
   Ecore_Drm_Seat_Capabilities seat_caps;

   void (*event_process)(Ecore_Drm_Evdev *dev, struct input_event *event, int count);

   Ecore_Fd_Handler *hdlr;
};

struct _Ecore_Drm_Sprite
{
   Ecore_Drm_Fb *current_fb, *next_fb;
   Ecore_Drm_Output *output;

   int drm_fd;

   unsigned int crtcs;
   unsigned int plane_id;

   struct 
     {
        int x, y;
        unsigned int w, h;
     } src, dest;

   unsigned int num_formats;
   unsigned int formats[];
};

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

   void *window;

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

void _ecore_drm_message_send(int opcode, int fd, void *data, size_t bytes);
Eina_Bool _ecore_drm_message_receive(int opcode, int *fd, void **data, size_t bytes);

Ecore_Drm_Evdev *_ecore_drm_evdev_device_create(Ecore_Drm_Seat *seat, const char *path, int fd);
void _ecore_drm_evdev_device_destroy(Ecore_Drm_Evdev *evdev);
/* int _ecore_drm_evdev_event_process(struct libinput_event *event); */

Ecore_Drm_Fb *_ecore_drm_fb_create(Ecore_Drm_Device *dev, int width, int height);
void _ecore_drm_fb_destroy(Ecore_Drm_Fb *fb);

/* #ifdef HAVE_GBM */
/* Ecore_Drm_Fb *_ecore_drm_fb_bo_get(Ecore_Drm_Device *dev, struct gbm_bo *bo); */
/* #endif */

void _ecore_drm_output_fb_release(Ecore_Drm_Output *output, Ecore_Drm_Fb *fb);
void _ecore_drm_output_repaint_start(Ecore_Drm_Output *output);
void _ecore_drm_output_frame_finish(Ecore_Drm_Output *output);

#endif

#ifndef _ECORE_DRM_PRIVATE_H
# define _ECORE_DRM_PRIVATE_H

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include "Ecore.h"
# include "ecore_private.h"
# include "Ecore_Input.h"

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <errno.h>
# include <fcntl.h>
# include <sys/mman.h>
# include <sys/stat.h>
# include <sys/ioctl.h>

# include <linux/vt.h>
# include <linux/kd.h>
# include <linux/major.h>
# include <linux/input.h>
# include <libinput.h>
# include <xkbcommon/xkbcommon.h>

# include <xf86drm.h>
# include <xf86drmMode.h>
# include <drm_fourcc.h>

# ifdef HAVE_SYSTEMD_LOGIN
#  include <systemd/sd-login.h>
# endif

# include <Eeze.h>
# include <Eldbus.h>
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

#define EVDEV_MAX_SLOTS 32

#define ERR(...) EINA_LOG_DOM_ERR(_ecore_drm_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_drm_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_ecore_drm_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_drm_log_dom, __VA_ARGS__)
#define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_drm_log_dom, __VA_ARGS__)

struct _Ecore_Drm_Output_Mode
{
   unsigned int flags;
   int width, height;
   unsigned int refresh;
   drmModeModeInfo info;
};

typedef enum _Ecore_Drm_Backlight_Type
{
   ECORE_DRM_BACKLIGHT_RAW,
   ECORE_DRM_BACKLIGHT_PLATFORM,
   ECORE_DRM_BACKLIGHT_FIRMWARE
} Ecore_Drm_Backlight_Type;

typedef struct _Ecore_Drm_Backlight
{
   const char *device;
   double brightness_max;
   double brightness_actual;
   double brightness;

   Ecore_Drm_Backlight_Type type;
} Ecore_Drm_Backlight;

struct _Ecore_Drm_Output
{
   Ecore_Drm_Device *dev;
   unsigned int crtc_id;
   unsigned int conn_id;
   drmModeCrtcPtr crtc;
   Eeze_Udev_Watch *watch;

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
   Ecore_Drm_Backlight *backlight;   
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
   Ecore_Drm_Device *dev;
   struct libinput *libinput;

   Ecore_Fd_Handler *hdlr;

   Eina_Bool enabled : 1;
   Eina_Bool suspended : 1;
};

struct _Ecore_Drm_Evdev
{
   Ecore_Drm_Seat *seat;
   struct libinput_device *device;

   const char *path;
   int fd;

   int mt_slot;

   Ecore_Drm_Output *output;

   /* struct  */
   /*   { */
   /*      int min_x, min_y; */
   /*      int max_x, max_y; */
   /*      double rel_w, rel_h; */
   /*      struct */
   /*        { */
   /*          int x[2]; */
   /*          int y[2]; */
   /*          Eina_Bool down : 1; */
   /*        } pt[EVDEV_MAX_SLOTS]; */
   /*   } abs; */

   struct 
     {
        int x, y;
        unsigned int last, prev;
        double threshold;
        Eina_Bool did_double : 1;
        Eina_Bool did_triple : 1;
        uint32_t prev_button, last_button;
     } mouse;

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
        unsigned int depressed, latched, locked, group;
     } xkb;

   /* Ecore_Drm_Evdev_Capabilities caps; */
   Ecore_Drm_Seat_Capabilities seat_caps;
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

typedef void (*Ecore_Drm_Open_Cb)(void *data, int fd, Eina_Bool b);

void _ecore_drm_event_activate_send(Eina_Bool active);

Eina_Bool _ecore_drm_launcher_device_open(const char *device, Ecore_Drm_Open_Cb callback, void *data, int flags);
int _ecore_drm_launcher_device_open_no_pending(const char *device, int flags);
void _ecore_drm_launcher_device_close(const char *device, int fd);
int _ecore_drm_launcher_device_flags_set(int fd, int flags);

Eina_Bool _ecore_drm_tty_switch(Ecore_Drm_Device *dev, int activate_vt);

Ecore_Drm_Evdev *_ecore_drm_evdev_device_create(Ecore_Drm_Seat *seat, struct libinput_device *device);
void _ecore_drm_evdev_device_destroy(Ecore_Drm_Evdev *evdev);
Eina_Bool _ecore_drm_evdev_event_process(struct libinput_event *event);

Ecore_Drm_Fb *_ecore_drm_fb_create(Ecore_Drm_Device *dev, int width, int height);
void _ecore_drm_fb_destroy(Ecore_Drm_Fb *fb);

void _ecore_drm_output_fb_release(Ecore_Drm_Output *output, Ecore_Drm_Fb *fb);
void _ecore_drm_output_repaint_start(Ecore_Drm_Output *output);
void _ecore_drm_output_frame_finish(Ecore_Drm_Output *output);

Eina_Bool _ecore_drm_logind_connect(Ecore_Drm_Device *dev);
void _ecore_drm_logind_disconnect(Ecore_Drm_Device *dev);
void _ecore_drm_logind_restore(Ecore_Drm_Device *dev);
Eina_Bool _ecore_drm_logind_device_open(const char *device, Ecore_Drm_Open_Cb callback, void *data);
int _ecore_drm_logind_device_open_no_pending(const char *device);
void _ecore_drm_logind_device_close(const char *device);

int _ecore_drm_dbus_init(Ecore_Drm_Device *dev);
int _ecore_drm_dbus_shutdown(void);
int _ecore_drm_dbus_device_take(uint32_t major, uint32_t minor, Ecore_Drm_Open_Cb callback, void *data);
int _ecore_drm_dbus_device_take_no_pending(uint32_t major, uint32_t minor, Eina_Bool *paused_out, double timeout);
void _ecore_drm_dbus_device_release(uint32_t major, uint32_t minor);
Eina_Bool _ecore_drm_dbus_session_take(const char *session);
Eina_Bool _ecore_drm_dbus_session_release(const char *session);

#endif

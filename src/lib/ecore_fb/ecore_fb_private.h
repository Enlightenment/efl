#ifndef _ECORE_FB_PRIVATE_H
#define _ECORE_FB_PRIVATE_H

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Input.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/version.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/fb.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
 #define kernel_ulong_t unsigned long
 #define BITS_PER_LONG 32
 #include <linux/input.h>
 #undef kernel_ulong_t
 #undef BITS_PER_LONG
#else
 #include <linux/input.h>
#endif
#include <libinput.h>
#include <xkbcommon/xkbcommon.h>

#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include <Eeze.h>
#include <Ecore_Fb.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef __GNUC__
# if __GNUC__ >= 4
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#else
# define EAPI
#endif

# ifdef ECORE_FB_DEFAULT_LOG_COLOR
#  undef ECORE_FB_DEFAULT_LOG_COLOR
# endif
# define ECORE_FB_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

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

extern int _ecore_fb_log_dom;

# define ERR(...) EINA_LOG_DOM_ERR(_ecore_fb_log_dom, __VA_ARGS__)
# define DBG(...) EINA_LOG_DOM_DBG(_ecore_fb_log_dom, __VA_ARGS__)
# define INF(...) EINA_LOG_DOM_INFO(_ecore_fb_log_dom, __VA_ARGS__)
# define WRN(...) EINA_LOG_DOM_WARN(_ecore_fb_log_dom, __VA_ARGS__)
# define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_fb_log_dom, __VA_ARGS__)

typedef enum _Ecore_Fb_Seat_Capabilities
{
   EVDEV_SEAT_POINTER = (1 << 0),
   EVDEV_SEAT_KEYBOARD = (1 << 1),
   EVDEV_SEAT_TOUCH = (1 << 2),
} Ecore_Fb_Seat_Capabilities;

struct _Ecore_Fb_Output
{
   Ecore_Fb_Device *dev;
   int x, y, phys_width, phys_height;

   const char *make, *model, *name;
   Eina_List *modes;
};


struct _Ecore_Fb_Device
{
   Eina_List *inputs;
   Eina_List *seats;
   Eina_List *outputs;
   const char *seat;
   int window;

  struct xkb_context *xkb_ctx;
};

struct _Ecore_Fb_Seat
{
//   struct libinput_seat *seat;
   const char *name;
   Ecore_Fb_Input *input;
   Eina_List *devices;
};

struct _Ecore_Fb_Input
{
   int fd;
   Ecore_Fb_Device *dev;
   struct libinput *libinput;

   Ecore_Fd_Handler *hdlr;

   Eina_Bool enabled : 1;
   Eina_Bool suspended : 1;
};

struct _Ecore_Fb_Evdev
{
   Ecore_Fb_Seat *seat;
   struct libinput_device *device;

   const char *path;
   int fd;

   int mt_slot;

   Ecore_Fb_Output *output;

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
        int ix, iy;
        int minx, miny, maxw, maxh;
        double dx, dy;
        unsigned int last, prev;
        uint32_t threshold;
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

   /* Ecore_Fb_Evdev_Capabilities caps; */
   Ecore_Fb_Seat_Capabilities seat_caps;
};


/* ecore_fb_li.c */
struct _Ecore_Fb_Input_Device
{
   int fd;
   Ecore_Fd_Handler *handler;
   int listen;
   struct
     {
        Ecore_Fb_Input_Device_Cap cap;
        char *name;
        char *dev;
     } info;
   struct
     {
        /* common mouse */
        int x,y;
        int w,h;

        double last;
        double prev;
        double threshold;
        Eina_Bool did_double;
        Eina_Bool did_triple;
        /* absolute axis */
        int min_w, min_h;
        double rel_w, rel_h;
        int event;
        int prev_button;
        int last_button;
     } mouse;
   struct
     {
        int shift;
        int ctrl;
        int alt;
        int lock;
     } keyboard;
   void *window;
};

/* ecore_fb_ts.c */
EAPI int    ecore_fb_ts_init(void);
EAPI void   ecore_fb_ts_shutdown(void);
EAPI void   ecore_fb_ts_events_window_set(void *window);
EAPI void  *ecore_fb_ts_events_window_get(void);
EAPI void   ecore_fb_ts_event_window_set(void *window);

/* ecore_fb_vt.c */
int  ecore_fb_vt_init(void);
void ecore_fb_vt_shutdown(void);

/* ecore_fb_evdev.c */
Ecore_Fb_Evdev *_ecore_fb_evdev_device_create(Ecore_Fb_Seat *seat, struct libinput_device *device);
void _ecore_fb_evdev_device_destroy(Ecore_Fb_Evdev *evdev);
Eina_Bool _ecore_fb_evdev_event_process(struct libinput_event *event);

/* hacks to stop people NEEDING #include <linux/h3600_ts.h> */
#ifndef TS_SET_CAL
#define TS_SET_CAL 0x4014660b
#endif
#ifndef TS_GET_CAL
#define TS_GET_CAL 0x8014660a
#endif

#undef EAPI
#define EAPI

#endif

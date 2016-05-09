#ifndef _ELPUT_PRIVATE_H
# define _ELPUT_PRIVATE_H

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include "Ecore.h"
# include "ecore_private.h"
# include "Ecore_Input.h"
# include "Eeze.h"
# include "Eldbus.h"
# include <Elput.h>

# include <ctype.h>
# include <sys/mman.h>
# include <fcntl.h>
# include <unistd.h>
# include <linux/vt.h>
# include <linux/kd.h>
# include <linux/major.h>
# include <linux/input.h>
# include <libinput.h>
# include <xkbcommon/xkbcommon.h>

# ifdef HAVE_SYSTEMD
#  include <systemd/sd-login.h>
# endif

# ifdef ELPUT_DEFAULT_LOG_COLOR
#  undef ELPUT_DEFAULT_LOG_COLOR
# endif
# define ELPUT_DEFAULT_LOG_COLOR EINA_COLOR_GREEN

extern int _elput_log_dom;

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_elput_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_elput_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_elput_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_elput_log_dom, __VA_ARGS__)

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) EINA_LOG_DOM_CRIT(_elput_log_dom, __VA_ARGS__)

typedef enum _Elput_Device_Capability
{
   EVDEV_SEAT_POINTER = (1 << 0),
   EVDEV_SEAT_KEYBOARD = (1 << 1),
   EVDEV_SEAT_TOUCH = (1 << 2)
} Elput_Device_Capability;

typedef struct _Elput_Interface
{
   Eina_Bool (*connect)(Elput_Manager **manager, const char *seat, unsigned int tty, Eina_Bool sync);
   void (*disconnect)(Elput_Manager *manager);
   int (*open)(Elput_Manager *manager, const char *path, int flags);
   void (*close)(Elput_Manager *manager, int fd);
   Eina_Bool (*vt_set)(Elput_Manager *manager, int vt);
} Elput_Interface;

typedef struct _Elput_Input
{
   struct libinput *lib;

   Ecore_Fd_Handler *hdlr;

   Eina_List *seats;

   Eina_Bool suspended : 1;
} Elput_Input;

typedef struct _Elput_Keyboard_Info
{
   int refs;

   struct
     {
        int fd;
        size_t size;
        char *area;
        struct xkb_keymap *map;
     } keymap;

   struct
     {
        xkb_mod_index_t shift;
        xkb_mod_index_t caps;
        xkb_mod_index_t ctrl;
        xkb_mod_index_t alt;
        xkb_mod_index_t altgr;
        xkb_mod_index_t super;
     } mods;
} Elput_Keyboard_Info;

struct _Elput_Keyboard
{
   struct
     {
        unsigned int depressed;
        unsigned int latched;
        unsigned int locked;
        unsigned int group;
     } mods;

   struct
     {
        unsigned int key;
        unsigned int timestamp;
     } grab;

   Elput_Keyboard_Info *info;

   struct xkb_state *state;
   struct xkb_keymap *pending_map;
   struct xkb_context *context;
   struct xkb_rule_names names;

   Elput_Seat *seat;

   Eina_Bool external_map : 1;
};

struct _Elput_Pointer
{
   double x, y;
   int buttons;
   unsigned int timestamp;

   int minx, miny;
   int maxw, maxh;
   int hotx, hoty;

   struct
     {
        double x, y;
        unsigned int button;
        unsigned int timestamp;
     } grab;

   struct
     {
        unsigned int threshold;
        unsigned int last_button, prev_button;
        unsigned int last_time, prev_time;
        Eina_Bool double_click : 1;
        Eina_Bool triple_click : 1;
     } mouse;

   Elput_Seat *seat;
};

struct _Elput_Touch
{
   double x, y;
   int slot;
   unsigned int points;

   struct
     {
        int id;
        double x, y;
        unsigned int timestamp;
     } grab;

   Elput_Seat *seat;
};

struct _Elput_Seat
{
   const char *name;

   struct
     {
        int kbd, ptr, touch;
     } count;

   unsigned int modifiers;

   Elput_Keyboard *kbd;
   Elput_Pointer *ptr;
   Elput_Touch *touch;

   Eina_List *devices;
};

struct _Elput_Device
{
   Elput_Seat *seat;

   uint32_t window;
   uint32_t ow, oh;

   const char *path;
   const char *output_name;
   struct libinput_device *device;

   Elput_Device_Capability caps;
};

struct _Elput_Manager
{
   Elput_Interface *interface;

   int fd;
   char *sid;
   const char *seat;
   unsigned int vt_num;

   struct
     {
        char *path;
        Eldbus_Object *obj;
        Eldbus_Connection *conn;
     } dbus;

   Elput_Input input;

   Eina_Bool sync : 1;
};

int _evdev_event_process(struct libinput_event *event);
Elput_Device *_evdev_device_create(Elput_Seat *seat, struct libinput_device *device);
void _evdev_device_destroy(Elput_Device *edev);
void _evdev_keyboard_destroy(Elput_Keyboard *kbd);
void _evdev_pointer_destroy(Elput_Pointer *ptr);
void _evdev_touch_destroy(Elput_Touch *touch);
void _evdev_pointer_motion_send(Elput_Device *edev);

Elput_Pointer *_evdev_pointer_get(Elput_Seat *seat);
Elput_Keyboard *_evdev_keyboard_get(Elput_Seat *seat);
Elput_Touch *_evdev_touch_get(Elput_Seat *seat);

extern Elput_Interface _logind_interface;

#endif

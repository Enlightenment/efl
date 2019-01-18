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
# include <xkbcommon/xkbcommon-compose.h>

# ifdef HAVE_SYSTEMD
#  include <systemd/sd-login.h>
# endif

# ifdef HAVE_ELOGIND
#  include <elogind/sd-login.h>
# endif

#ifndef ELPUT_NODEFS
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
#endif
typedef struct _Elput_Interface
{
   Eina_Bool (*connect)(Elput_Manager **manager, const char *seat, unsigned int tty);
   void (*disconnect)(Elput_Manager *manager);
   int (*open)(Elput_Manager *manager, const char *path, int flags);
   void (*open_async)(Elput_Manager *manager, const char *path, int flags);
   void (*close)(Elput_Manager *manager, int fd);
   Eina_Bool (*vt_set)(Elput_Manager *manager, int vt);
} Elput_Interface;

typedef struct _Elput_Input
{
   struct libinput *lib;

   Ecore_Fd_Handler *hdlr;

   Eina_List *seats;
   Ecore_Thread *thread;
   Eldbus_Pending *current_pending;
   int pipe;
   int pointer_w, pointer_h;
   int rotation;

   Eina_Bool suspended : 1;
} Elput_Input;

typedef enum _Elput_Leds
{
   ELPUT_LED_NUM = (1 << 0),
   ELPUT_LED_CAPS = (1 << 1),
   ELPUT_LED_SCROLL = (1 << 2)
} Elput_Leds;

typedef struct _Elput_Keyboard_Info
{
   int refs;

   struct
     {
        struct xkb_keymap *map;
     } keymap;

   struct
     {
        xkb_mod_index_t shift;
        xkb_mod_index_t caps;
        xkb_mod_index_t num;
        xkb_mod_index_t ctrl;
        xkb_mod_index_t alt;
        xkb_mod_index_t altgr;
        xkb_mod_index_t super;
     } mods;

   struct
     {
        xkb_led_index_t num;
        xkb_led_index_t caps;
        xkb_led_index_t scroll;
     } leds;
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
   unsigned int key_count;

   struct xkb_state *state;
   struct xkb_state *maskless_state;
   struct xkb_context *context;
   struct xkb_rule_names names;
   struct xkb_compose_table *compose_table;
   struct xkb_compose_state *compose_state;

   Elput_Leds leds;

   Elput_Seat *seat;

   Eina_Bool pending_keymap : 1;
};

struct _Elput_Pointer
{
   int buttons;
   unsigned int timestamp;
   double pressure;

   int minx, miny;
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
   int slot;
   unsigned int points;
   unsigned int timestamp;
   double pressure;

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
   int refs; //for events
   const char *name;

   struct
     {
        int kbd, ptr, touch;
     } count;

   struct
     {
        double x, y;
     } pointer;

   unsigned int modifiers;

   Elput_Keyboard *kbd;
   Elput_Pointer *ptr;
   Elput_Touch *touch;

   Eina_List *devices;
   Elput_Manager *manager;
   Eina_Bool pending_motion : 1;
};

struct _Elput_Device
{
   Elput_Seat *seat;
   int refs; //for events

   uint32_t ow, oh;

   double absx, absy;

   const char *path;
   const char *output_name;
   struct libinput_device *device;

   Elput_Device_Caps caps;

   Eina_Hash *key_remap_hash;
   Eo *evas_device;

   Eina_Bool left_handed : 1;
   Eina_Bool key_remap : 1;
   Eina_Bool swap : 1;
   Eina_Bool invert_x : 1;
   Eina_Bool invert_y : 1;
};

struct _Elput_Manager
{
   Elput_Interface *interface;

   int fd;
   char *sid;
   const char *seat;
   unsigned int vt_num;
   int vt_fd;
   Ecore_Event_Handler *vt_hdlr;
   uint32_t window;

   int pending_ptr_x;
   int pending_ptr_y;

   struct
     {
        char *path;
        Eldbus_Object *obj;
        Eldbus_Connection *conn;
        Eldbus_Proxy *session;
        Eldbus_Proxy *manager;
     } dbus;

   struct
     {
        struct xkb_keymap *keymap;
        struct xkb_context *context;
        int group;
     } cached;
   int output_w, output_h;

   int drm_opens;

   Elput_Input input;
   Eina_Bool del : 1;
};

typedef struct _Elput_Async_Open
{
   Elput_Manager *manager;
   char *path;
   int flags;
} Elput_Async_Open;

void _elput_input_enable(Elput_Manager *manager);
void _elput_input_disable(Elput_Manager *manager);

int _evdev_event_process(struct libinput_event *event);
Elput_Device *_evdev_device_create(Elput_Seat *seat, struct libinput_device *device);
void _evdev_device_destroy(Elput_Device *edev);
void _evdev_keyboard_destroy(Elput_Keyboard *kbd);
void _evdev_pointer_destroy(Elput_Pointer *ptr);
void _evdev_touch_destroy(Elput_Touch *touch);
void _evdev_pointer_motion_send(Elput_Device *edev);
void _evdev_device_calibrate(Elput_Device *dev);

Elput_Pointer *_evdev_pointer_get(Elput_Seat *seat);
Elput_Keyboard *_evdev_keyboard_get(Elput_Seat *seat);
Elput_Touch *_evdev_touch_get(Elput_Seat *seat);

extern Elput_Interface _logind_interface;

void _keyboard_keymap_update(Elput_Seat *seat);
void _keyboard_group_update(Elput_Seat *seat);

void _udev_seat_destroy(Elput_Seat *eseat);

#endif

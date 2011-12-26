#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Input.h"
#include "ecore_wl_private.h"
#include "Ecore_Wayland.h"

/* FIXME: This gives BTN_LEFT/RIGHT/MIDDLE for linux systems ... 
 *        What about other OSs ?? */
#ifdef __linux__
# include <linux/input.h>
#else
# define BTN_LEFT 0x110
# define BTN_RIGHT 0x111
# define BTN_MIDDLE 0x112
# define BTN_SIDE 0x113
# define BTN_EXTRA 0x114
# define BTN_FORWARD 0x115
# define BTN_BACK 0x116
#endif

#include <X11/extensions/XKBcommon.h>

/* local function prototypes */
static Eina_Bool _ecore_wl_shutdown(Eina_Bool close_display);
static void _ecore_wl_cb_disp_handle_global(struct wl_display *disp, uint32_t id, const char *interface, uint32_t version __UNUSED__, void *data __UNUSED__);
static int _ecore_wl_cb_disp_event_mask_update(uint32_t mask, void *data __UNUSED__);
static void _ecore_wl_cb_shm_format_iterate(void *data __UNUSED__, struct wl_shm *shm __UNUSED__, uint32_t format);
static void _ecore_wl_cb_disp_handle_geometry(void *data __UNUSED__, struct wl_output *output __UNUSED__, int x, int y, int pw __UNUSED__, int ph __UNUSED__, int subpixel __UNUSED__, const char *make __UNUSED__, const char *model __UNUSED__);
static void _ecore_wl_cb_disp_handle_mode(void *data __UNUSED__, struct wl_output *output __UNUSED__, uint32_t flags, int w, int h, int refresh __UNUSED__);
static Eina_Bool _ecore_wl_cb_fd_handle(void *data, Ecore_Fd_Handler *hdl __UNUSED__);
static void _ecore_wl_cb_handle_motion(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t, int32_t x, int32_t y, int32_t sx, int32_t sy);
static void _ecore_wl_cb_handle_button(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t, uint32_t btn, uint32_t state);
static void _ecore_wl_cb_handle_key(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t __UNUSED__, uint32_t key, uint32_t state);
static void _ecore_wl_cb_handle_pointer_focus(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t __UNUSED__, struct wl_surface *surface, int32_t x, int32_t y, int32_t sx, int32_t sy);
static void _ecore_wl_cb_handle_keyboard_focus(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t __UNUSED__, struct wl_surface *surface, struct wl_array *keys);
static void _ecore_wl_mouse_out_send(void);
static void _ecore_wl_mouse_in_send(void);
static void _ecore_wl_focus_out_send(void);
static void _ecore_wl_focus_in_send(void);

/* local variables */
static int _ecore_wl_init_count = 0;
static struct wl_display *_ecore_wl_disp = NULL;
static uint32_t _ecore_wl_disp_mask = 0;
static uint32_t _ecore_wl_disp_format = WL_SHM_FORMAT_PREMULTIPLIED_ARGB32;
static Eina_Rectangle _ecore_wl_screen;
static Ecore_Fd_Handler *_ecore_wl_fd_hdl = NULL;
static int _ecore_wl_input_x = 0;
static int _ecore_wl_input_y = 0;
static int _ecore_wl_input_sx = 0;
static int _ecore_wl_input_sy = 0;
static int _ecore_wl_input_modifiers = 0;
static struct xkb_desc *_ecore_wl_xkb;

static struct wl_compositor *_ecore_wl_comp;
static struct wl_shm *_ecore_wl_shm;
static struct wl_shell *_ecore_wl_shell;
static struct wl_output *_ecore_wl_output;
static struct wl_input_device *_ecore_wl_input;
static struct wl_surface *_ecore_wl_input_surface;
static const struct wl_shm_listener _ecore_wl_shm_listener = 
{
   _ecore_wl_cb_shm_format_iterate
};
static const struct wl_output_listener _ecore_wl_output_listener = 
{
   _ecore_wl_cb_disp_handle_geometry, 
   _ecore_wl_cb_disp_handle_mode
};
static const struct wl_input_device_listener _ecore_wl_input_listener = 
{
   _ecore_wl_cb_handle_motion, 
   _ecore_wl_cb_handle_button, 
   _ecore_wl_cb_handle_key, 
   _ecore_wl_cb_handle_pointer_focus, 
   _ecore_wl_cb_handle_keyboard_focus, 
   NULL, // touch down
   NULL, // touch up
   NULL, // touch motion
   NULL, // touch frame
   NULL, // touch cancel
};

/* external variables */
int _ecore_wl_log_dom = -1;
EAPI int ECORE_WL_EVENT_MOUSE_IN = 0;
EAPI int ECORE_WL_EVENT_MOUSE_OUT = 0;
EAPI int ECORE_WL_EVENT_FOCUS_IN = 0;
EAPI int ECORE_WL_EVENT_FOCUS_OUT = 0;

EAPI int 
ecore_wl_init(const char *name) 
{
   struct xkb_rule_names xkb_names;
   int fd = 0;

   if (++_ecore_wl_init_count != 1)
     return _ecore_wl_init_count;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!eina_init()) return --_ecore_wl_init_count;

   _ecore_wl_log_dom = 
     eina_log_domain_register("ecore_wl", ECORE_WL_DEFAULT_LOG_COLOR);
   if (_ecore_wl_log_dom < 0) 
     {
        EINA_LOG_ERR("Cannot create a log domain for Ecore Wayland.");
        eina_shutdown();
        return --_ecore_wl_init_count;
     }

   if (!ecore_init()) 
     {
        eina_log_domain_unregister(_ecore_wl_log_dom);
        _ecore_wl_log_dom = -1;
        eina_shutdown();
        return --_ecore_wl_init_count;
     }

   if (!ecore_event_init()) 
     {
        eina_log_domain_unregister(_ecore_wl_log_dom);
        _ecore_wl_log_dom = -1;
        ecore_shutdown();
        eina_shutdown();
        return --_ecore_wl_init_count;
     }

   if (!ECORE_WL_EVENT_MOUSE_IN) 
     {
        ECORE_WL_EVENT_MOUSE_IN = ecore_event_type_new();
        ECORE_WL_EVENT_MOUSE_OUT = ecore_event_type_new();
        ECORE_WL_EVENT_FOCUS_IN = ecore_event_type_new();
        ECORE_WL_EVENT_FOCUS_OUT = ecore_event_type_new();
     }

   /* init xkb */
   /* FIXME: Somehow make this portable to other languages/countries */
   xkb_names.rules = "evdev";
   xkb_names.model = "evdev";
   xkb_names.layout = "us";
   xkb_names.variant = "";
   xkb_names.options = "";
   if (!(_ecore_wl_xkb = xkb_compile_keymap_from_rules(&xkb_names))) 
     {
        ERR("Could not compile keymap");
        eina_log_domain_unregister(_ecore_wl_log_dom);
        _ecore_wl_log_dom = -1;
        ecore_event_shutdown();
        ecore_shutdown();
        eina_shutdown();
        return --_ecore_wl_init_count;
     }

   /* connect to the wayland display */
   if (!(_ecore_wl_disp = wl_display_connect(name))) 
     {
        eina_log_domain_unregister(_ecore_wl_log_dom);
        _ecore_wl_log_dom = -1;
        ecore_event_shutdown();
        ecore_shutdown();
        eina_shutdown();
        return --_ecore_wl_init_count;
     }

   /* setup handler for wayland interfaces */
   wl_display_add_global_listener(_ecore_wl_disp, 
                                  _ecore_wl_cb_disp_handle_global, NULL);

   /* process connection events */
   wl_display_iterate(_ecore_wl_disp, WL_DISPLAY_READABLE);

   fd = wl_display_get_fd(_ecore_wl_disp, 
                          _ecore_wl_cb_disp_event_mask_update, NULL);

   /* NB: DO NOT TOUCH THIS OR YOU WILL PAY THE PRICE OF FAILURE !! */
   /* Without a ECORE_FD_WRITE, then animators/timers break */
   _ecore_wl_fd_hdl = 
     ecore_main_fd_handler_add(fd, ECORE_FD_READ | ECORE_FD_WRITE, 
                               _ecore_wl_cb_fd_handle, _ecore_wl_disp, 
                               NULL, NULL);
   if (!_ecore_wl_fd_hdl) 
     {
        wl_display_destroy(_ecore_wl_disp);
        _ecore_wl_disp = NULL;
        eina_log_domain_unregister(_ecore_wl_log_dom);
        _ecore_wl_log_dom = -1;
        ecore_event_shutdown();
        ecore_shutdown();
        eina_shutdown();
        return --_ecore_wl_init_count;
     }

   return _ecore_wl_init_count;
}

EAPI int 
ecore_wl_shutdown(void) 
{
   return _ecore_wl_shutdown(EINA_TRUE);
}

EAPI struct wl_display *
ecore_wl_display_get(void) 
{
   return _ecore_wl_disp;
}

EAPI struct wl_shm *
ecore_wl_shm_get(void) 
{
   return _ecore_wl_shm;
}

EAPI struct wl_compositor *
ecore_wl_compositor_get(void) 
{
   return _ecore_wl_comp;
}

EAPI struct wl_shell *
ecore_wl_shell_get(void) 
{
   return _ecore_wl_shell;
}

EAPI struct wl_input_device *
ecore_wl_input_device_get(void) 
{
   return _ecore_wl_input;
}

EAPI void 
ecore_wl_screen_size_get(int *w, int *h) 
{
   if (w) *w = _ecore_wl_screen.w;
   if (h) *h = _ecore_wl_screen.h;
}

EAPI unsigned int 
ecore_wl_format_get(void) 
{
   return _ecore_wl_disp_format;
}

EAPI void 
ecore_wl_flush(void) 
{
   wl_display_flush(_ecore_wl_disp);
   /* if (_ecore_wl_disp_mask & WL_DISPLAY_WRITABLE) */
   /*   wl_display_iterate(_ecore_wl_disp, WL_DISPLAY_WRITABLE); */
}

EAPI void 
ecore_wl_sync(void) 
{
   wl_display_iterate(_ecore_wl_disp, WL_DISPLAY_READABLE);
}

/* local functions */
static Eina_Bool 
_ecore_wl_shutdown(Eina_Bool close_display) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (--_ecore_wl_init_count != 0)
     return _ecore_wl_init_count;

   if (!_ecore_wl_disp) return _ecore_wl_init_count;

   if (_ecore_wl_xkb) free(_ecore_wl_xkb);

   if (_ecore_wl_fd_hdl) ecore_main_fd_handler_del(_ecore_wl_fd_hdl);
   _ecore_wl_fd_hdl = NULL;

   if (close_display) 
     {
        if (_ecore_wl_shm) wl_shm_destroy(_ecore_wl_shm);
        if (_ecore_wl_comp) wl_compositor_destroy(_ecore_wl_comp);
        if (_ecore_wl_disp) wl_display_destroy(_ecore_wl_disp);
        _ecore_wl_disp = NULL;
     }

   eina_log_domain_unregister(_ecore_wl_log_dom);
   _ecore_wl_log_dom = -1;

   ecore_event_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return _ecore_wl_init_count;
}

static void 
_ecore_wl_cb_disp_handle_global(struct wl_display *disp, uint32_t id, const char *interface, uint32_t version __UNUSED__, void *data __UNUSED__) 
{
//   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (disp != _ecore_wl_disp) return;
   if (!strcmp(interface, "wl_compositor")) 
     {
        _ecore_wl_comp = 
          wl_display_bind(_ecore_wl_disp, id, &wl_compositor_interface);
     }
   else if (!strcmp(interface, "wl_shm")) 
     {
        _ecore_wl_shm = 
          wl_display_bind(_ecore_wl_disp, id, &wl_shm_interface);
        wl_shm_add_listener(_ecore_wl_shm, &_ecore_wl_shm_listener, NULL);
     }
   else if (!strcmp(interface, "wl_output")) 
     {
        _ecore_wl_output = 
          wl_display_bind(_ecore_wl_disp, id, &wl_output_interface);
        wl_output_add_listener(_ecore_wl_output, 
                               &_ecore_wl_output_listener, NULL);
     }
   else if (!strcmp(interface, "wl_shell")) 
     {
        _ecore_wl_shell = 
          wl_display_bind(_ecore_wl_disp, id, &wl_shell_interface);
     }
   else if (!strcmp(interface, "wl_input_device")) 
     {
        _ecore_wl_input = 
          wl_display_bind(_ecore_wl_disp, id, &wl_input_device_interface);
        wl_input_device_add_listener(_ecore_wl_input, 
                                     &_ecore_wl_input_listener, NULL);
     }
}

static int 
_ecore_wl_cb_disp_event_mask_update(uint32_t mask, void *data __UNUSED__) 
{
//   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_disp_mask = mask;
   return 0;
}

static void 
_ecore_wl_cb_shm_format_iterate(void *data __UNUSED__, struct wl_shm *shm __UNUSED__, uint32_t format) 
{
//   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (_ecore_wl_disp_format < 2) return;
   switch (format) 
     {
      case WL_SHM_FORMAT_ARGB32:
        /* NB: Ignore argb32. We prefer premul */
        break;
      case WL_SHM_FORMAT_PREMULTIPLIED_ARGB32:
        _ecore_wl_disp_format = format;
        break;
      case WL_SHM_FORMAT_XRGB32:
        _ecore_wl_disp_format = format;
        break;
      default:
        break;
     }
}

static void 
_ecore_wl_cb_disp_handle_geometry(void *data __UNUSED__, struct wl_output *output __UNUSED__, int x, int y, int pw __UNUSED__, int ph __UNUSED__, int subpixel __UNUSED__, const char *make __UNUSED__, const char *model __UNUSED__) 
{
   _ecore_wl_screen.x = x;
   _ecore_wl_screen.y = y;
}

static void 
_ecore_wl_cb_disp_handle_mode(void *data __UNUSED__, struct wl_output *output __UNUSED__, uint32_t flags, int w, int h, int refresh __UNUSED__) 
{
   if (flags & WL_OUTPUT_MODE_CURRENT) 
     {
        _ecore_wl_screen.w = w;
        _ecore_wl_screen.h = h;
     }
}

static Eina_Bool 
_ecore_wl_cb_fd_handle(void *data, Ecore_Fd_Handler *hdl __UNUSED__) 
{
   struct wl_display *disp;

   if (!(disp = data)) return ECORE_CALLBACK_RENEW;
   if (disp != _ecore_wl_disp) return ECORE_CALLBACK_RENEW;

//   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* NB: This handles iterate for writable AND readable.
    * DO NOT TOUCH THIS OR YOU WILL PAY THE PRICE OF FAILURE !!
    * Without this, animators/timers die */
   if (_ecore_wl_disp_mask & (WL_DISPLAY_READABLE | WL_DISPLAY_WRITABLE))
     wl_display_roundtrip(_ecore_wl_disp);

   return ECORE_CALLBACK_RENEW;
}

static void 
_ecore_wl_cb_handle_motion(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t, int32_t x, int32_t y, int32_t sx, int32_t sy) 
{
   Ecore_Event_Mouse_Move *ev;

   if (dev != _ecore_wl_input) return;
   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Move)))) return;

   _ecore_wl_input_x = x;
   _ecore_wl_input_y = y;
   _ecore_wl_input_sx = sx;
   _ecore_wl_input_sy = sy;

   ev->timestamp = t;
   ev->x = sx;
   ev->y = sy;
   ev->root.x = x;
   ev->root.y = y;

   if (_ecore_wl_input_surface) 
     {
        unsigned int id = 0;

        if ((id = (unsigned int)wl_surface_get_user_data(_ecore_wl_input_surface)))
          {
             ev->window = id;
             ev->event_window = id;
          }
     }

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
}

static void 
_ecore_wl_cb_handle_button(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t, uint32_t btn, uint32_t state) 
{
   if (dev != _ecore_wl_input) return;

   if ((btn >= BTN_SIDE) && (btn <= BTN_BACK))
     {
        Ecore_Event_Mouse_Wheel *ev;

        if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Wheel)))) return;

        ev->timestamp = t;
        ev->x = _ecore_wl_input_sx;
        ev->y = _ecore_wl_input_sy;
        ev->root.x = _ecore_wl_input_x;
        ev->root.y = _ecore_wl_input_y;
        ev->modifiers = _ecore_wl_input_modifiers;
        ev->direction = 0;

        if (_ecore_wl_input_surface) 
          {
             unsigned int id = 0;

             if ((id = (unsigned int)wl_surface_get_user_data(_ecore_wl_input_surface)))
               {
                  ev->window = id;
                  ev->event_window = id;
               }
          }

        /* NB: (FIXME) Currently Wayland provides no measure of how much the 
         * wheel has scrolled (read: delta of movement). So for now, we will 
         * just assume that the amount scrolled is 1 */
        if ((btn == BTN_EXTRA) || (btn == BTN_FORWARD)) // down
          ev->z = 1;
        else if ((btn == BTN_SIDE) || (btn == BTN_BACK)) // up
          ev->z = -1;

        ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
     }
   else 
     {
        Ecore_Event_Mouse_Button *ev;

        if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Button)))) return;

        if (btn == BTN_LEFT)
          ev->buttons = 1;
        else if (btn == BTN_MIDDLE)
          ev->buttons = 2;
        else if (btn == BTN_RIGHT)
          ev->buttons = 3;

        ev->timestamp = t;
        ev->x = _ecore_wl_input_sx;
        ev->y = _ecore_wl_input_sy;
        ev->root.x = _ecore_wl_input_x;
        ev->root.y = _ecore_wl_input_y;
        ev->modifiers = _ecore_wl_input_modifiers;

        if (_ecore_wl_input_surface) 
          {
             unsigned int id = 0;

             if ((id = (unsigned int)wl_surface_get_user_data(_ecore_wl_input_surface))) 
               {
                  ev->window = id;
                  ev->event_window = id;
               }
          }

        if (state)
          ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
        else
          ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
     }
}

static void 
_ecore_wl_cb_handle_key(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t __UNUSED__, uint32_t key, uint32_t state) 
{
   unsigned int keycode = 0;

   if (dev != _ecore_wl_input) return;

   keycode = key + _ecore_wl_xkb->min_key_code;

   if (state)
     _ecore_wl_input_modifiers |= _ecore_wl_xkb->map->modmap[keycode];
   else
     _ecore_wl_input_modifiers &= ~_ecore_wl_xkb->map->modmap[keycode];
}

static void 
_ecore_wl_cb_handle_pointer_focus(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t __UNUSED__, struct wl_surface *surface, int32_t x, int32_t y, int32_t sx, int32_t sy) 
{
   if (dev != _ecore_wl_input) return;

   _ecore_wl_input_x = x;
   _ecore_wl_input_y = y;
   _ecore_wl_input_sx = sx;
   _ecore_wl_input_sy = sy;

   if (surface) 
     {
        if (_ecore_wl_input_surface) 
          {
             if (_ecore_wl_input_surface != surface) 
               {
                  /* NB: Pointer focus in different window. Send mouse & focus 
                   * out events for previous window */
                  _ecore_wl_mouse_out_send();
                  _ecore_wl_focus_out_send();

                  /* NB: Send mouse & focus in events for new window */
                  _ecore_wl_input_surface = surface;
                  _ecore_wl_mouse_in_send();
                  _ecore_wl_focus_in_send();
               }
          }
        else 
          {
             _ecore_wl_input_surface = surface;
             _ecore_wl_mouse_in_send();
             _ecore_wl_focus_in_send();
             /* printf("\tPointer Focus In New Window\n"); */
          }
     }
   else 
     {
        if (_ecore_wl_input_surface) 
          {
             _ecore_wl_mouse_out_send();
             _ecore_wl_focus_out_send();
             /* printf("\tPointer Focus Not On a Window\n"); */
          }
        else
          printf("\tUnhandled Pointer Focus Case !!!\n");
        _ecore_wl_input_surface = NULL;
     }
}

static void 
_ecore_wl_cb_handle_keyboard_focus(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t __UNUSED__, struct wl_surface *surface, struct wl_array *keys) 
{
   unsigned int *keyend = 0, *i = 0;

   if (dev != _ecore_wl_input) return;

   if ((surface) && (surface != _ecore_wl_input_surface)) 
     _ecore_wl_input_surface = surface;
   else if (!surface) 
     _ecore_wl_input_surface = NULL;

   keyend = keys->data + keys->size;
   _ecore_wl_input_modifiers = 0;
   for (i = keys->data; i < keyend; i++)
     _ecore_wl_input_modifiers |= _ecore_wl_xkb->map->modmap[*i];
}

static void 
_ecore_wl_mouse_out_send(void) 
{
   Ecore_Wl_Event_Mouse_Out *ev;

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Mouse_Out)))) return;

   ev->x = _ecore_wl_input_sx;
   ev->y = _ecore_wl_input_sy;
   ev->root.x = _ecore_wl_input_x;
   ev->root.y = _ecore_wl_input_y;
   ev->modifiers = _ecore_wl_input_modifiers;
   ev->time = ecore_time_get();

   if (_ecore_wl_input_surface) 
     {
        unsigned int id = 0;

        if ((id = (unsigned int)wl_surface_get_user_data(_ecore_wl_input_surface))) 
          ev->window = id;
     }

   ecore_event_add(ECORE_WL_EVENT_MOUSE_OUT, ev, NULL, NULL);
}

static void 
_ecore_wl_mouse_in_send(void) 
{
   Ecore_Wl_Event_Mouse_In *ev;

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Mouse_In)))) return;

   ev->x = _ecore_wl_input_sx;
   ev->y = _ecore_wl_input_sy;
   ev->root.x = _ecore_wl_input_x;
   ev->root.y = _ecore_wl_input_y;
   ev->modifiers = _ecore_wl_input_modifiers;
   ev->time = ecore_time_get();

   if (_ecore_wl_input_surface) 
     {
        unsigned int id = 0;

        if ((id = (unsigned int)wl_surface_get_user_data(_ecore_wl_input_surface))) 
          ev->window = id;
     }

   ecore_event_add(ECORE_WL_EVENT_MOUSE_IN, ev, NULL, NULL);
}

static void 
_ecore_wl_focus_out_send(void) 
{
   Ecore_Wl_Event_Focus_Out *ev;

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Focus_Out)))) return;
   ev->time = ecore_time_get();
   if (_ecore_wl_input_surface) 
     {
        unsigned int id = 0;

        if ((id = (unsigned int)wl_surface_get_user_data(_ecore_wl_input_surface))) 
          ev->window = id;
     }
   ecore_event_add(ECORE_WL_EVENT_FOCUS_OUT, ev, NULL, NULL);
}

static void 
_ecore_wl_focus_in_send(void) 
{
   Ecore_Wl_Event_Focus_In *ev;

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Focus_In)))) return;
   ev->time = ecore_time_get();
   if (_ecore_wl_input_surface) 
     {
        unsigned int id = 0;

        if ((id = (unsigned int)wl_surface_get_user_data(_ecore_wl_input_surface))) 
          ev->window = id;
     }
   ecore_event_add(ECORE_WL_EVENT_FOCUS_IN, ev, NULL, NULL);
}

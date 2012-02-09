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
#include <fcntl.h>
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
static void _ecore_wl_cb_disp_handle_geometry(void *data __UNUSED__, struct wl_output *output __UNUSED__, int x, int y, int pw __UNUSED__, int ph __UNUSED__, int subpixel __UNUSED__, const char *make __UNUSED__, const char *model __UNUSED__);
static void _ecore_wl_cb_disp_handle_mode(void *data __UNUSED__, struct wl_output *output __UNUSED__, uint32_t flags, int w, int h, int refresh __UNUSED__);
static Eina_Bool _ecore_wl_cb_fd_handle(void *data, Ecore_Fd_Handler *hdl __UNUSED__);
static void _ecore_wl_cb_handle_motion(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t, int32_t x, int32_t y, int32_t sx, int32_t sy);
static void _ecore_wl_cb_handle_button(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t, uint32_t btn, uint32_t state);
static void _ecore_wl_cb_handle_key(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t __UNUSED__, uint32_t key, uint32_t state);
static void _ecore_wl_cb_handle_pointer_focus(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t, struct wl_surface *surface, int32_t x, int32_t y, int32_t sx, int32_t sy);
static void _ecore_wl_cb_handle_keyboard_focus(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t __UNUSED__, struct wl_surface *surface, struct wl_array *keys);
static void _ecore_wl_cb_handle_touch_down(void *data __UNUSED__, struct wl_input_device *dev __UNUSED__, uint32_t timestamp, struct wl_surface *surface, int32_t id, int32_t x, int32_t y);
static void _ecore_wl_cb_handle_touch_up(void *data __UNUSED__, struct wl_input_device *dev __UNUSED__, uint32_t timestamp, int32_t id);
static void _ecore_wl_cb_handle_touch_motion(void *data __UNUSED__, struct wl_input_device *dev __UNUSED__, uint32_t timestamp, int32_t id, int32_t x, int32_t y);
static void _ecore_wl_cb_handle_touch_frame(void *data __UNUSED__, struct wl_input_device *dev __UNUSED__);
static void _ecore_wl_cb_handle_touch_cancel(void *data __UNUSED__, struct wl_input_device *dev __UNUSED__);
static void _ecore_wl_cb_source_target(void *data, struct wl_data_source *source __UNUSED__, const char *mime_type __UNUSED__);
static void _ecore_wl_cb_source_send(void *data, struct wl_data_source *source, const char *mime_type, int32_t fd);
static void _ecore_wl_cb_source_cancelled(void *data, struct wl_data_source *source __UNUSED__);
static void _ecore_wl_cb_source_offer(void *data, struct wl_data_offer *offer __UNUSED__, const char *type);
static void _ecore_wl_cb_data_offer(void *data, struct wl_data_device *data_dev, uint32_t id);
static void _ecore_wl_cb_data_enter(void *data __UNUSED__, struct wl_data_device *data_dev, uint32_t timestamp, struct wl_surface *surface, int32_t x, int32_t y, struct wl_data_offer *offer);
static void _ecore_wl_cb_data_leave(void *data __UNUSED__, struct wl_data_device *data_dev);
static void _ecore_wl_cb_data_motion(void *data __UNUSED__, struct wl_data_device *data_dev, uint32_t timestamp, int32_t x, int32_t y);
static void _ecore_wl_cb_data_drop(void *data __UNUSED__, struct wl_data_device *data_dev);
static void _ecore_wl_cb_data_selection(void *data, struct wl_data_device *data_dev, struct wl_data_offer *offer);
static void _ecore_wl_cb_mouse_move_free(void *data __UNUSED__, void *event);

static void _ecore_wl_mouse_move_send(uint32_t timestamp);
static void _ecore_wl_mouse_out_send(struct wl_surface *surface, uint32_t timestamp);
static void _ecore_wl_mouse_in_send(struct wl_surface *surface, uint32_t timestamp);
static void _ecore_wl_mouse_up_send(struct wl_surface *surface, uint32_t button, uint32_t timestamp);
static void _ecore_wl_mouse_down_send(struct wl_surface *surface, uint32_t button, uint32_t timestamp);
static void _ecore_wl_focus_out_send(struct wl_surface *surface, uint32_t timestamp);
static void _ecore_wl_focus_in_send(struct wl_surface *surface, uint32_t timestamp);

/* local variables */
static int _ecore_wl_init_count = 0;
static struct wl_display *_ecore_wl_disp = NULL;
static uint32_t _ecore_wl_disp_mask = 0;
static uint32_t _ecore_wl_disp_format = WL_SHM_FORMAT_ARGB8888;
static Eina_Rectangle _ecore_wl_screen;
static Ecore_Fd_Handler *_ecore_wl_fd_hdl = NULL;
static int _ecore_wl_screen_x = 0;
static int _ecore_wl_screen_y = 0;
static int _ecore_wl_surface_x = 0;
static int _ecore_wl_surface_y = 0;
static int _ecore_wl_touch_x = 0;
static int _ecore_wl_touch_y = 0;
static int _ecore_wl_input_modifiers = 0;
static struct xkb_desc *_ecore_wl_xkb = NULL;
static uint32_t _ecore_wl_input_button = 0;
static uint32_t _ecore_wl_input_timestamp = 0;

static struct wl_compositor *_ecore_wl_comp = NULL;
static struct wl_shm *_ecore_wl_shm = NULL;
static struct wl_shell *_ecore_wl_shell = NULL;
static struct wl_output *_ecore_wl_output = NULL;
static struct wl_input_device *_ecore_wl_input_dev = NULL;
static struct wl_surface *_ecore_wl_input_surface = NULL;
static struct wl_surface *_ecore_wl_touch_surface = NULL;
static struct wl_data_device_manager *_ecore_wl_data_manager = NULL;
static struct wl_data_device *_ecore_wl_data_dev = NULL;

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
   _ecore_wl_cb_handle_touch_down, 
   _ecore_wl_cb_handle_touch_up, 
   _ecore_wl_cb_handle_touch_motion, 
   _ecore_wl_cb_handle_touch_frame, 
   _ecore_wl_cb_handle_touch_cancel,
};
static const struct wl_data_source_listener _ecore_wl_source_listener = 
{
   _ecore_wl_cb_source_target, 
   _ecore_wl_cb_source_send, 
   _ecore_wl_cb_source_cancelled
};
static const struct wl_data_device_listener _ecore_wl_data_listener = 
{
   _ecore_wl_cb_data_offer, 
   _ecore_wl_cb_data_enter, 
   _ecore_wl_cb_data_leave, 
   _ecore_wl_cb_data_motion, 
   _ecore_wl_cb_data_drop, 
   _ecore_wl_cb_data_selection
};
static const struct wl_data_offer_listener _ecore_wl_offer_listener = 
{
   _ecore_wl_cb_source_offer,
};

/* external variables */
int _ecore_wl_log_dom = -1;
EAPI int ECORE_WL_EVENT_MOUSE_IN = 0;
EAPI int ECORE_WL_EVENT_MOUSE_OUT = 0;
EAPI int ECORE_WL_EVENT_FOCUS_IN = 0;
EAPI int ECORE_WL_EVENT_FOCUS_OUT = 0;
EAPI int ECORE_WL_EVENT_DRAG_START = 0;
EAPI int ECORE_WL_EVENT_DRAG_STOP = 0;

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
        ECORE_WL_EVENT_DRAG_START = ecore_event_type_new();
        ECORE_WL_EVENT_DRAG_STOP = ecore_event_type_new();
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
        ERR("Could not connect to display");
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

   _ecore_wl_fd_hdl = 
     ecore_main_fd_handler_add(fd, ECORE_FD_READ, _ecore_wl_cb_fd_handle, 
                               _ecore_wl_disp, NULL, NULL);
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
   return _ecore_wl_input_dev;
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
}

EAPI void 
ecore_wl_sync(void) 
{
   wl_display_iterate(_ecore_wl_disp, WL_DISPLAY_READABLE);
}

EAPI void 
ecore_wl_pointer_xy_get(int *x, int *y) 
{
   if (x) *x = _ecore_wl_screen_x;
   if (y) *y = _ecore_wl_screen_y;
}

EAPI unsigned int 
ecore_wl_input_timestamp_get(void)
{
   return _ecore_wl_input_timestamp;
}

EAPI Ecore_Wl_Drag_Source *
ecore_wl_drag_source_create(int hotspot_x, int hotspot_y, int offset_x, int offset_y, const char *mimetype, unsigned int timestamp, void *data)
{
   Ecore_Wl_Drag_Source *source;

   if (!(source = calloc(1, sizeof(Ecore_Wl_Drag_Source)))) return NULL;

   source->data_dev = _ecore_wl_data_dev;
   source->hotspot_x = hotspot_x;
   source->hotspot_y = hotspot_y;
   source->offset_x = offset_x;
   source->offset_y = offset_y;
   source->mimetype = mimetype;
   source->timestamp = timestamp;
   source->data = data;

   source->data_source = 
     wl_data_device_manager_create_data_source(_ecore_wl_data_manager);

   wl_data_source_add_listener(source->data_source, 
                               &_ecore_wl_source_listener, source);

   wl_data_source_offer(source->data_source, source->mimetype);

   /* NB: Do we add some default mimetypes here ?? */
   /* text/plain, etc */

   return source;
}

EAPI void 
ecore_wl_drag_start(Ecore_Wl_Drag_Source *source, struct wl_surface *surface, struct wl_buffer *buffer)
{
   source->buffer = buffer;

   wl_data_device_start_drag(source->data_dev, source->data_source, 
                             surface, source->timestamp);
}

EAPI void 
ecore_wl_drag_stop(void)
{

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
        if (_ecore_wl_data_dev) wl_data_device_destroy(_ecore_wl_data_dev);
        if (_ecore_wl_input_dev) wl_input_device_destroy(_ecore_wl_input_dev);
        if (_ecore_wl_data_manager) 
          wl_data_device_manager_destroy(_ecore_wl_data_manager);
        if (_ecore_wl_shell) wl_shell_destroy(_ecore_wl_shell);
        if (_ecore_wl_shm) wl_shm_destroy(_ecore_wl_shm);
        if (_ecore_wl_comp) wl_compositor_destroy(_ecore_wl_comp);
        if (_ecore_wl_disp)
          {
             wl_display_flush(_ecore_wl_disp);
             wl_display_destroy(_ecore_wl_disp);
          }
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
        _ecore_wl_input_dev = 
          wl_display_bind(_ecore_wl_disp, id, &wl_input_device_interface);
        wl_input_device_add_listener(_ecore_wl_input_dev, 
                                     &_ecore_wl_input_listener, NULL);
     }
   else if (!strcmp(interface, "wl_data_device_manager")) 
     {
        _ecore_wl_data_manager = 
          wl_display_bind(_ecore_wl_disp, id, 
                          &wl_data_device_manager_interface);
     }

   if ((_ecore_wl_input_dev) && (_ecore_wl_data_manager) && (!_ecore_wl_data_dev))
     {
        _ecore_wl_data_dev = 
          wl_data_device_manager_get_data_device(_ecore_wl_data_manager, 
                                                 _ecore_wl_input_dev);
        wl_data_device_add_listener(_ecore_wl_data_dev, 
                                    &_ecore_wl_data_listener, NULL);
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

//   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(disp = data)) return ECORE_CALLBACK_RENEW;
   if (disp != _ecore_wl_disp) return ECORE_CALLBACK_RENEW;

   if (_ecore_wl_disp_mask & WL_DISPLAY_WRITABLE)
     wl_display_iterate(_ecore_wl_disp, WL_DISPLAY_WRITABLE);

   if (_ecore_wl_disp_mask & WL_DISPLAY_READABLE)
     wl_display_iterate(_ecore_wl_disp, WL_DISPLAY_READABLE);

   return ECORE_CALLBACK_RENEW;
}

static void 
_ecore_wl_cb_handle_motion(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t, int32_t x, int32_t y, int32_t sx, int32_t sy) 
{
//   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (dev != _ecore_wl_input_dev) return;

   _ecore_wl_screen_x = x;
   _ecore_wl_screen_y = y;
   _ecore_wl_surface_x = sx;
   _ecore_wl_surface_y = sy;

   _ecore_wl_mouse_move_send(t);
}

static void 
_ecore_wl_cb_handle_button(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t, uint32_t btn, uint32_t state) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (dev != _ecore_wl_input_dev) return;

   if ((btn >= BTN_SIDE) && (btn <= BTN_BACK))
     {
        Ecore_Event_Mouse_Wheel *ev;

        if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Wheel)))) return;

        ev->timestamp = t;
        ev->x = _ecore_wl_surface_x;
        ev->y = _ecore_wl_surface_y;
        ev->root.x = _ecore_wl_screen_x;
        ev->root.y = _ecore_wl_screen_y;
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
        _ecore_wl_mouse_move_send(t);

        if (state)
          {
             _ecore_wl_input_button = btn;
             if (_ecore_wl_input_surface) 
               _ecore_wl_mouse_down_send(_ecore_wl_input_surface, btn, t);

             /* NB: Ideally, this is not the place to check for drags.
              * IMO, drags should be handled by the client. EG: we raise the 
              * mouse_down to the client, and the client can 'request' a 
              * drag_start from ecore_wl */
             if ((_ecore_wl_input_surface) || (_ecore_wl_touch_surface))
               {
                  /* record item which was grabbed. 
                   * create drag source. start drag */
               }
          }
        else
          {
             if ((_ecore_wl_input_surface) || (_ecore_wl_touch_surface))
               {
                  /* release grabbed button and finish drag */
                  if ((_ecore_wl_input_button) && 
                      (_ecore_wl_input_button == btn))
                    {
                       
                    }
               }
             _ecore_wl_input_button = 0;
             if (_ecore_wl_input_surface) 
               _ecore_wl_mouse_up_send(_ecore_wl_input_surface, btn, t);
          }
     }
}

static void 
_ecore_wl_cb_handle_key(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t __UNUSED__, uint32_t key, uint32_t state) 
{
   unsigned int keycode = 0;

   if ((!_ecore_wl_input_surface) || (dev != _ecore_wl_input_dev)) return;

   keycode = key + _ecore_wl_xkb->min_key_code;

   if (state)
     _ecore_wl_input_modifiers |= _ecore_wl_xkb->map->modmap[keycode];
   else
     _ecore_wl_input_modifiers &= ~_ecore_wl_xkb->map->modmap[keycode];
}

static void 
_ecore_wl_cb_handle_pointer_focus(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t, struct wl_surface *surface, int32_t x, int32_t y, int32_t sx, int32_t sy) 
{
   if (dev != _ecore_wl_input_dev) return;

   /* NB: Wayland pointer focus is weird. It's not pointer focus in the normal 
    * sense...Wayland 'moving/resizing' (and maybe other stuff) has a habit 
    * of stealing the pointer focus and thus this cannot be used to control 
    * normal pointer focus. On mouse down, the 'active' surface is stolen 
    * by Wayland for the grab, so 'surface' here ends up being NULL. When a 
    * move or resize is finished, we get this event again, but this time 
    * with an active surface */

   _ecore_wl_screen_x = x;
   _ecore_wl_screen_y = y;
   _ecore_wl_surface_x = sx;
   _ecore_wl_surface_y = sy;

   if ((_ecore_wl_input_surface) && (_ecore_wl_input_surface != surface)) 
     {
        if (!_ecore_wl_input_button)
          _ecore_wl_mouse_out_send(_ecore_wl_input_surface, t);
     }

   if (surface) 
     {
        if (_ecore_wl_input_button)
          {
             _ecore_wl_mouse_up_send(surface, _ecore_wl_input_button, t);
             _ecore_wl_input_button = 0;
          }
        else
          _ecore_wl_mouse_in_send(surface, t);
     }
}

static void 
_ecore_wl_cb_handle_keyboard_focus(void *data __UNUSED__, struct wl_input_device *dev, uint32_t t, struct wl_surface *surface, struct wl_array *keys) 
{
   unsigned int *keyend = 0, *i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (dev != _ecore_wl_input_dev) return;

   /* NB: Remove old keyboard focus */
   if ((_ecore_wl_input_surface) && (_ecore_wl_input_surface != surface))
     _ecore_wl_focus_out_send(_ecore_wl_input_surface, t);

   _ecore_wl_input_surface = NULL;

   keyend = keys->data + keys->size;
   _ecore_wl_input_modifiers = 0;
   for (i = keys->data; i < keyend; i++)
     _ecore_wl_input_modifiers |= _ecore_wl_xkb->map->modmap[*i];

   if (surface) 
     {
        /* set new input surface */
        _ecore_wl_input_surface = surface;

        /* send mouse in to new surface */
        /* _ecore_wl_mouse_in_send(surface, t); */

        /* send focus to new surface */
        _ecore_wl_focus_in_send(surface, t);
     }
}

static void 
_ecore_wl_cb_handle_touch_down(void *data __UNUSED__, struct wl_input_device *dev __UNUSED__, uint32_t timestamp, struct wl_surface *surface, int32_t id, int32_t x, int32_t y)
{
   Ecore_Event_Mouse_Button *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_touch_surface = surface;
   _ecore_wl_touch_x = x;
   _ecore_wl_touch_y = y;

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Button)))) return;

   ev->timestamp = timestamp;

   /* NB: Need to verify using x,y for these */
   ev->x = x;
   ev->y = y;
   ev->root.x = x;
   ev->root.y = y;
   ev->modifiers = 0;
   ev->buttons = 0;
   ev->same_screen = 1;

   /* FIXME: Need to get these from Wayland somehow */
   ev->double_click = 0;
   ev->triple_click = 0;

   ev->multi.device = id;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   /* NB: Need to verify using x,y for these */
   ev->multi.x = x;
   ev->multi.y = y;
   ev->multi.root.x = x;
   ev->multi.root.y = y;

     {
        unsigned int id = 0;

        if ((id = (unsigned int)wl_surface_get_user_data(surface))) 
          {
             ev->window = id;
             ev->event_window = id;
          }
     }

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
}

static void 
_ecore_wl_cb_handle_touch_up(void *data __UNUSED__, struct wl_input_device *dev __UNUSED__, uint32_t timestamp, int32_t id)
{
   Ecore_Event_Mouse_Button *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Button)))) return;

   ev->timestamp = timestamp;

   /* TODO: Need to verify using x,y for these */
   ev->x = _ecore_wl_touch_x;
   ev->y = _ecore_wl_touch_y;
   ev->root.x = _ecore_wl_touch_x;
   ev->root.y = _ecore_wl_touch_y;
   ev->modifiers = 0;
   ev->buttons = 0;
   ev->same_screen = 1;

   /* FIXME: Need to get these from Wayland somehow */
   ev->double_click = 0;
   ev->triple_click = 0;

   ev->multi.device = id;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;

   /* TODO: Need to verify using x,y for these */
   ev->multi.x = _ecore_wl_touch_x;
   ev->multi.y = _ecore_wl_touch_y;
   ev->multi.root.x = _ecore_wl_touch_x;
   ev->multi.root.y = _ecore_wl_touch_y;

     {
        unsigned int id = 0;

        if ((id = (unsigned int)wl_surface_get_user_data(_ecore_wl_touch_surface))) 
          {
             ev->window = id;
             ev->event_window = id;
          }
     }

   _ecore_wl_touch_surface = NULL;

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
}

static void 
_ecore_wl_cb_handle_touch_motion(void *data __UNUSED__, struct wl_input_device *dev __UNUSED__, uint32_t timestamp, int32_t id, int32_t x, int32_t y)
{
   Ecore_Event_Mouse_Move *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!_ecore_wl_touch_surface) return;

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Move)))) return;

   ev->timestamp = timestamp;
   /* TODO: Need to verify using x,y for these */
   ev->x = x;
   ev->y = y;
   ev->root.x = x;
   ev->root.y = y;
   ev->modifiers = 0; //_ecore_wl_input_modifiers;
   ev->same_screen = 1;

   ev->multi.device = id;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;

   /* TODO: Need to verify using x,y for these */
   ev->multi.x = x;
   ev->multi.y = y;
   ev->multi.root.x = x;
   ev->multi.root.y = y;

     {
        unsigned int id = 0;

        if ((id = (unsigned int)wl_surface_get_user_data(_ecore_wl_touch_surface)))
          {
             ev->window = id;
             ev->event_window = id;
          }
     }

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
}

static void 
_ecore_wl_cb_handle_touch_frame(void *data __UNUSED__, struct wl_input_device *dev __UNUSED__)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* FIXME: Need to get a device and actually test what happens here */
}

static void 
_ecore_wl_cb_handle_touch_cancel(void *data __UNUSED__, struct wl_input_device *dev __UNUSED__)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* FIXME: Need to get a device and actually test what happens here */
   _ecore_wl_touch_surface = NULL;
}

static void 
_ecore_wl_cb_source_target(void *data, struct wl_data_source *source __UNUSED__, const char *mime_type __UNUSED__)
{
   Ecore_Wl_Drag_Source *s;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   printf("Ecore_Wl Source Target\n");
   if (!(s = data)) return;
   printf("\tHave Drag Source\n");

   /* FIXME: buffer here should really be the mouse cursor buffer */
   wl_data_device_attach(s->data_dev, s->timestamp, s->buffer, 
                         s->hotspot_x, s->hotspot_y);
}

static void 
_ecore_wl_cb_source_send(void *data, struct wl_data_source *source, const char *mime_type, int32_t fd)
{
   Ecore_Wl_Drag_Source *s;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   printf("Ecore_Wl Source Send\n");
   if (!(s = data)) return;
   printf("\tHave Drag Source\n");

   /* FIXME: write message to fd */

   /* NB: Wayland really sucks in this regard. Why should selection stuff 
    * require an 'fd' ?? */
}

static void 
_ecore_wl_cb_source_cancelled(void *data, struct wl_data_source *source __UNUSED__)
{
   Ecore_Wl_Drag_Source *s;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* The cancelled event usually means source is no longer in use by 
    * the drag (or selection). */

   printf("Ecore_Wl Source Cancel\n");
   if (!(s = data)) return;
   printf("\tHave Drag Source\n");

   /* FIXME: raise this to ecore_evas so the surface/buffer 
    * of the drag can be destroyed */

   if (s->data_source) wl_data_source_destroy(s->data_source);
   s->data_source = NULL;

   free(s);
}

static void 
_ecore_wl_cb_source_offer(void *data, struct wl_data_offer *offer __UNUSED__, const char *type)
{
   Ecore_Wl_Dnd_Source *s;

   if (!(s = data)) return;
   eina_array_push(s->types, strdup(type));
}

static void 
_ecore_wl_cb_data_offer(void *data, struct wl_data_device *data_dev, uint32_t id)
{
   Ecore_Wl_Dnd_Source *source;

   /* create a new 'data offer' structure and setup a listener for it */
   if (!(source = calloc(1, sizeof(Ecore_Wl_Dnd_Source)))) return;

   source->types = eina_array_new(1);
   source->data = data;
   source->refs = 1;

   /* FIXME: This will need to change when Wayland has typesafe wrappers for this */
   source->offer = (struct wl_data_offer *)
     wl_proxy_create_for_id((struct wl_proxy *)data_dev, 
                            id, &wl_data_offer_interface);

   wl_data_device_set_user_data(data_dev, source);
   wl_data_offer_add_listener(source->offer, &_ecore_wl_offer_listener, source);
}

static void 
_ecore_wl_cb_data_enter(void *data __UNUSED__, struct wl_data_device *data_dev, uint32_t timestamp, struct wl_surface *surface, int32_t x, int32_t y, struct wl_data_offer *offer)
{
   Ecore_Wl_Dnd_Source *source;

   if (!(source = wl_data_device_get_user_data(data_dev))) return;

   /* TODO: maybe set pointer focus here ?? */

   source->timestamp = timestamp;
}

static void 
_ecore_wl_cb_data_leave(void *data __UNUSED__, struct wl_data_device *data_dev)
{
   Ecore_Wl_Dnd_Source *source;

   if (!(source = wl_data_device_get_user_data(data_dev))) return;

   /* destroy drag offer */
   wl_data_offer_destroy(source->offer);

   while (eina_array_count(source->types))
     free(eina_array_pop(source->types));

   eina_array_free(source->types);
   free(source);

   wl_data_device_set_user_data(data_dev, NULL);
}

static void 
_ecore_wl_cb_data_motion(void *data __UNUSED__, struct wl_data_device *data_dev, uint32_t timestamp, int32_t x, int32_t y)
{
   Ecore_Wl_Dnd_Source *source;

   if (!(source = wl_data_device_get_user_data(data_dev))) return;
   /* TODO: Here we should raise motion events for dragging */
}

static void 
_ecore_wl_cb_data_drop(void *data __UNUSED__, struct wl_data_device *data_dev)
{
   Ecore_Wl_Dnd_Source *source;

   if (!(source = wl_data_device_get_user_data(data_dev))) return;

   /* TODO: Raise event for drop */

   wl_data_offer_accept(source->offer, source->timestamp, NULL);
//                        eina_array_data_get(source->types, 0));
}

static void 
_ecore_wl_cb_data_selection(void *data, struct wl_data_device *data_dev, struct wl_data_offer *offer)
{
   Ecore_Wl_Dnd_Source *source;

   printf("Ecore_Wl Data Selection\n");
   if ((source = wl_data_device_get_user_data(data_dev)))
     {
        /* destroy old source */
        wl_data_offer_destroy(source->offer);

        while (eina_array_count(source->types))
          free(eina_array_pop(source->types));

        eina_array_free(source->types);
        free(source);

        wl_data_device_set_user_data(data_dev, NULL);
     }
}

static void 
_ecore_wl_cb_mouse_move_free(void *data __UNUSED__, void *event)
{
   Ecore_Event_Mouse_Move *ev;

   if ((ev = event)) free(ev);
}

static void 
_ecore_wl_mouse_move_send(uint32_t timestamp)
{
   Ecore_Event_Mouse_Move *ev;
   Ecore_Event *event;

//   if (!_ecore_wl_input_surface) return;

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Move)))) return;

   ev->timestamp = timestamp;

   ev->x = _ecore_wl_surface_x;
   ev->y = _ecore_wl_surface_y;
   ev->root.x = _ecore_wl_screen_x;
   ev->root.y = _ecore_wl_screen_y;
   ev->modifiers = _ecore_wl_input_modifiers;

   ev->multi.device = 0;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = _ecore_wl_surface_x;
   ev->multi.y = _ecore_wl_surface_y;
   ev->multi.root.x = _ecore_wl_screen_x;
   ev->multi.root.y = _ecore_wl_screen_y;

     {
        unsigned int id = 0;

        if (_ecore_wl_input_surface) 
          {
             LOGFN(__FILE__, __LINE__, __FUNCTION__);

             if ((id = (unsigned int)wl_surface_get_user_data(_ecore_wl_input_surface)))
               {
                  ev->window = id;
                  ev->event_window = id;
               }
          }
     }

   event = ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, 
                           _ecore_wl_cb_mouse_move_free, NULL);
}

static void 
_ecore_wl_mouse_out_send(struct wl_surface *surface, uint32_t timestamp) 
{
   Ecore_Wl_Event_Mouse_Out *ev;

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Mouse_Out)))) return;

   ev->x = _ecore_wl_surface_x;
   ev->y = _ecore_wl_surface_y;
   ev->root.x = _ecore_wl_screen_x;
   ev->root.y = _ecore_wl_screen_y;
   ev->modifiers = _ecore_wl_input_modifiers;
   ev->time = timestamp;

   if (surface) 
     {
        unsigned int id = 0;

        if ((id = (unsigned int)wl_surface_get_user_data(surface))) 
          ev->window = id;
     }

   ecore_event_add(ECORE_WL_EVENT_MOUSE_OUT, ev, NULL, NULL);
}

static void 
_ecore_wl_mouse_in_send(struct wl_surface *surface, uint32_t timestamp) 
{
   Ecore_Wl_Event_Mouse_In *ev;

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Mouse_In)))) return;

   ev->x = _ecore_wl_surface_x;
   ev->y = _ecore_wl_surface_y;
   ev->root.x = _ecore_wl_screen_x;
   ev->root.y = _ecore_wl_screen_y;
   ev->modifiers = _ecore_wl_input_modifiers;
   ev->time = timestamp;

   if (surface) 
     {
        unsigned int id = 0;

        if ((id = (unsigned int)wl_surface_get_user_data(surface))) 
          ev->window = id;
     }

   ecore_event_add(ECORE_WL_EVENT_MOUSE_IN, ev, NULL, NULL);
}

static void 
_ecore_wl_mouse_up_send(struct wl_surface *surface, uint32_t button, uint32_t timestamp)
{
   Ecore_Event_Mouse_Button *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Button)))) return;

   if (button == BTN_LEFT)
     ev->buttons = 1;
   else if (button == BTN_MIDDLE)
     ev->buttons = 2;
   else if (button == BTN_RIGHT)
     ev->buttons = 3;

   ev->timestamp = timestamp;
   ev->x = _ecore_wl_surface_x;
   ev->y = _ecore_wl_surface_y;
   ev->root.x = _ecore_wl_screen_x;
   ev->root.y = _ecore_wl_screen_y;
   ev->modifiers = _ecore_wl_input_modifiers;

   /* FIXME: Need to get these from Wayland somehow */
   ev->double_click = 0;
   ev->triple_click = 0;

   ev->multi.device = 0;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = _ecore_wl_surface_x;
   ev->multi.y = _ecore_wl_surface_y;
   ev->multi.root.x = _ecore_wl_screen_x;
   ev->multi.root.y = _ecore_wl_screen_y;

     {
        unsigned int id = 0;

        if ((id = (unsigned int)wl_surface_get_user_data(surface))) 
          {
             ev->window = id;
             ev->event_window = id;
          }
     }

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
}

static void 
_ecore_wl_mouse_down_send(struct wl_surface *surface, uint32_t button, uint32_t timestamp)
{
   Ecore_Event_Mouse_Button *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Button)))) return;

   if (button == BTN_LEFT)
     ev->buttons = 1;
   else if (button == BTN_MIDDLE)
     ev->buttons = 2;
   else if (button == BTN_RIGHT)
     ev->buttons = 3;

   _ecore_wl_input_timestamp = timestamp;
   ev->timestamp = timestamp;
   ev->x = _ecore_wl_surface_x;
   ev->y = _ecore_wl_surface_y;
   ev->root.x = _ecore_wl_screen_x;
   ev->root.y = _ecore_wl_screen_y;
   ev->modifiers = _ecore_wl_input_modifiers;

   /* FIXME: Need to get these from Wayland somehow */
   ev->double_click = 0;
   ev->triple_click = 0;

   ev->multi.device = 0;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = _ecore_wl_surface_x;
   ev->multi.y = _ecore_wl_surface_y;
   ev->multi.root.x = _ecore_wl_screen_x;
   ev->multi.root.y = _ecore_wl_screen_y;

     {
        unsigned int id = 0;

        if ((id = (unsigned int)wl_surface_get_user_data(surface))) 
          {
             ev->window = id;
             ev->event_window = id;
          }
     }

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
}

static void 
_ecore_wl_focus_out_send(struct wl_surface *surface, uint32_t timestamp) 
{
   Ecore_Wl_Event_Focus_Out *ev;

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Focus_Out)))) return;
   ev->time = timestamp;
   if (surface) 
     {
        unsigned int id = 0;

        if ((id = (unsigned int)wl_surface_get_user_data(surface))) 
          ev->window = id;
     }
   ecore_event_add(ECORE_WL_EVENT_FOCUS_OUT, ev, NULL, NULL);
}

static void 
_ecore_wl_focus_in_send(struct wl_surface *surface, uint32_t timestamp) 
{
   Ecore_Wl_Event_Focus_In *ev;

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Focus_In)))) return;
   ev->time = timestamp;
   if (surface) 
     {
        unsigned int id = 0;

        if ((id = (unsigned int)wl_surface_get_user_data(surface))) 
          ev->window = id;
     }
   ecore_event_add(ECORE_WL_EVENT_FOCUS_IN, ev, NULL, NULL);
}

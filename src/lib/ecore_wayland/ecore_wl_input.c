#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*
 * NB: Events that receive a 'serial' instead of timestamp
 * 
 * input_device_attach (for pointer image)
 * input_device_button_event (button press/release)
 * input_device_key_press
 * input_device_pointer_enter
 * input_device_pointer_leave
 * input_device_keyboard_enter
 * input_device_keyboard_leave
 * input_device_touch_down
 * input_device_touch_up
 * 
 **/

#include "ecore_wl_private.h"
#include <sys/mman.h>
#include <ctype.h>

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

typedef struct _Ecore_Wl_Mouse_Down_Info
{
   EINA_INLIST;
   int dev;
   int last_win;
   int last_last_win;
   int last_event_win;
   int last_last_event_win;
   int sx, sy;
   unsigned int last_time;
   unsigned int last_last_time;
   Eina_Bool did_double : 1;
   Eina_Bool did_triple : 1;
} Ecore_Wl_Mouse_Down_Info;

/* FIXME: This should be a global setting, used by wayland and X */
static double _ecore_wl_double_click_time = 0.25;
static Eina_Inlist *_ecore_wl_mouse_down_info_list = NULL;

/* local function prototypes */
static void _ecore_wl_input_seat_handle_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps);

static void _ecore_wl_input_cb_pointer_enter(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int serial, struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy);
static void _ecore_wl_input_cb_pointer_leave(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int serial, struct wl_surface *surface);
static void _ecore_wl_input_cb_pointer_motion(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int timestamp, wl_fixed_t sx, wl_fixed_t sy);
static void _ecore_wl_input_cb_pointer_button(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int serial, unsigned int timestamp, unsigned int button, unsigned int state);
static void _ecore_wl_input_cb_pointer_axis(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int timestamp, unsigned int axis, wl_fixed_t value);
static void _ecore_wl_input_cb_pointer_frame(void *data, struct wl_callback *callback, unsigned int timestamp EINA_UNUSED);
static void _ecore_wl_input_cb_keyboard_keymap(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int format, int fd, unsigned int size);
static void _ecore_wl_input_cb_keyboard_enter(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int serial, struct wl_surface *surface, struct wl_array *keys EINA_UNUSED);
static void _ecore_wl_input_cb_keyboard_leave(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int serial, struct wl_surface *surface);
static void _ecore_wl_input_cb_keyboard_key(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int serial, unsigned int timestamp, unsigned int key, unsigned int state);
static void _ecore_wl_input_cb_keyboard_modifiers(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int serial EINA_UNUSED, unsigned int depressed, unsigned int latched, unsigned int locked, unsigned int group);
static Eina_Bool _ecore_wl_input_cb_keyboard_repeat(void *data);
static void _ecore_wl_input_cb_touch_down(void *data, struct wl_touch *touch, unsigned int serial, unsigned int timestamp, struct wl_surface *surface EINA_UNUSED, int id EINA_UNUSED, wl_fixed_t x, wl_fixed_t y);
static void _ecore_wl_input_cb_touch_up(void *data, struct wl_touch *touch, unsigned int serial, unsigned int timestamp, int id EINA_UNUSED);
static void _ecore_wl_input_cb_touch_motion(void *data, struct wl_touch *touch EINA_UNUSED, unsigned int timestamp, int id, wl_fixed_t x, wl_fixed_t y);
static void _ecore_wl_input_cb_touch_frame(void *data EINA_UNUSED, struct wl_touch *touch EINA_UNUSED);
static void _ecore_wl_input_cb_touch_cancel(void *data EINA_UNUSED, struct wl_touch *touch EINA_UNUSED);
static void _ecore_wl_input_cb_data_offer(void *data, struct wl_data_device *data_device, struct wl_data_offer *offer);
static void _ecore_wl_input_cb_data_enter(void *data, struct wl_data_device *data_device, unsigned int timestamp, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y, struct wl_data_offer *offer);
static void _ecore_wl_input_cb_data_leave(void *data, struct wl_data_device *data_device);
static void _ecore_wl_input_cb_data_motion(void *data, struct wl_data_device *data_device, unsigned int timestamp, wl_fixed_t x, wl_fixed_t y);
static void _ecore_wl_input_cb_data_drop(void *data, struct wl_data_device *data_device);
static void _ecore_wl_input_cb_data_selection(void *data, struct wl_data_device *data_device, struct wl_data_offer *offer);

static void _ecore_wl_input_mouse_move_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int timestamp, int device);
static void _ecore_wl_input_mouse_in_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int timestamp);
static void _ecore_wl_input_mouse_out_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int timestamp);
static void _ecore_wl_input_focus_in_send(Ecore_Wl_Input *input EINA_UNUSED, Ecore_Wl_Window *win, unsigned int timestamp);
static void _ecore_wl_input_focus_out_send(Ecore_Wl_Input *input EINA_UNUSED, Ecore_Wl_Window *win, unsigned int timestamp);
static void _ecore_wl_input_mouse_down_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, int device, unsigned int button, unsigned int timestamp);
static void _ecore_wl_input_mouse_up_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, int device, unsigned int button, unsigned int timestamp);
static void _ecore_wl_input_mouse_wheel_send(Ecore_Wl_Input *input, unsigned int axis, int value, unsigned int timestamp);
static Ecore_Wl_Mouse_Down_Info *_ecore_wl_mouse_down_info_get(int dev);

/* static int _ecore_wl_input_keysym_to_string(unsigned int symbol, char *buffer, int len); */

/* wayland interfaces */
static const struct wl_pointer_listener pointer_listener = 
{
   _ecore_wl_input_cb_pointer_enter,
   _ecore_wl_input_cb_pointer_leave,
   _ecore_wl_input_cb_pointer_motion,
   _ecore_wl_input_cb_pointer_button,
   _ecore_wl_input_cb_pointer_axis,
};

static const struct wl_keyboard_listener keyboard_listener = 
{
   _ecore_wl_input_cb_keyboard_keymap,
   _ecore_wl_input_cb_keyboard_enter,
   _ecore_wl_input_cb_keyboard_leave,
   _ecore_wl_input_cb_keyboard_key,
   _ecore_wl_input_cb_keyboard_modifiers,
};

static const struct wl_touch_listener touch_listener = 
{
   _ecore_wl_input_cb_touch_down,
   _ecore_wl_input_cb_touch_up,
   _ecore_wl_input_cb_touch_motion,
   _ecore_wl_input_cb_touch_frame,
   _ecore_wl_input_cb_touch_cancel
};

static const struct wl_seat_listener _ecore_wl_seat_listener = 
{
   _ecore_wl_input_seat_handle_capabilities,
   NULL // _ecore_wl_input_seat_handle_name
};

static const struct wl_data_device_listener _ecore_wl_data_listener = 
{
   _ecore_wl_input_cb_data_offer,
   _ecore_wl_input_cb_data_enter,
   _ecore_wl_input_cb_data_leave,
   _ecore_wl_input_cb_data_motion,
   _ecore_wl_input_cb_data_drop,
   _ecore_wl_input_cb_data_selection
};

static const struct wl_callback_listener _ecore_wl_pointer_surface_listener = 
{
   _ecore_wl_input_cb_pointer_frame
};

/* local variables */
static int _pointer_x, _pointer_y;

EAPI void 
ecore_wl_input_grab(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int button)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!input) return;
   input->grab = win;
   input->grab_button = button;
}

EAPI void 
ecore_wl_input_ungrab(Ecore_Wl_Input *input)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!input) return;
   input->grab = NULL;
   input->grab_button = 0;
}

/* NB: This function should be called just before shell move and shell resize
 * functions. Those requests will trigger a mouse/touch implicit grab on the
 * compositor that will prevent the respective mouse/touch up events being
 * released after the end of the operation. This function checks if such grab
 * is in place for those windows and, if so, emit the respective mouse up
 * event. It's a workaround to the fact that wayland doesn't inform the
 * application about this move or resize grab being finished.
 */
void
_ecore_wl_input_grab_release(Ecore_Wl_Input *input, Ecore_Wl_Window *win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!input) return;
   if (input->grab != win) return;

   _ecore_wl_input_mouse_up_send(input, input->grab,
                                 0, input->grab_button, input->grab_timestamp);

   ecore_wl_input_ungrab(input);
}

static void
_pointer_update_stop(Ecore_Wl_Input *input)
{
   if (!input->cursor_timer) return;

   ecore_timer_del(input->cursor_timer);
   input->cursor_timer = NULL;
}

EAPI void
ecore_wl_input_pointer_set(Ecore_Wl_Input *input, struct wl_surface *surface, int hot_x, int hot_y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!input) return;

   _pointer_update_stop(input);
   if (input->pointer)
     wl_pointer_set_cursor(input->pointer, input->pointer_enter_serial,
                           surface, hot_x, hot_y);
}

EAPI void
ecore_wl_input_cursor_size_set(Ecore_Wl_Input *input, const int size)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!input) return;

   input->cursor_size = size;
   input->display->cursor_theme = 
     wl_cursor_theme_load(NULL, input->cursor_size, input->display->wl.shm);
}

static Eina_Bool
_ecore_wl_input_cursor_update(void *data)
{
   struct wl_cursor_image *cursor_image;
   struct wl_buffer *buffer;
   Ecore_Wl_Input *input = data;
   unsigned int delay;

   cursor_image = input->cursor->images[input->cursor_current_index];
   if (!cursor_image) return ECORE_CALLBACK_RENEW;

   if ((buffer = wl_cursor_image_get_buffer(cursor_image)))
     {
        ecore_wl_input_pointer_set(input, input->cursor_surface,
                                   cursor_image->hotspot_x,
                                   cursor_image->hotspot_y);
        wl_surface_attach(input->cursor_surface, buffer, 0, 0);
        wl_surface_damage(input->cursor_surface, 0, 0,
                          cursor_image->width, cursor_image->height);
        wl_surface_commit(input->cursor_surface);

        if ((input->cursor->image_count > 1) && (!input->cursor_frame_cb))
          _ecore_wl_input_cb_pointer_frame(input, NULL, 0);
     }

   if (input->cursor->image_count <= 1)
     return ECORE_CALLBACK_CANCEL;

   delay = cursor_image->delay;
   input->cursor_current_index =
      (input->cursor_current_index + 1) % input->cursor->image_count;

   if (!input->cursor_timer)
     input->cursor_timer =
        ecore_timer_loop_add(delay / 1000.0,
                             _ecore_wl_input_cursor_update, input);
   else
     ecore_timer_interval_set(input->cursor_timer, delay / 1000.0);

   return ECORE_CALLBACK_RENEW;
}

EAPI void
ecore_wl_input_cursor_from_name_set(Ecore_Wl_Input *input, const char *cursor_name)
{
   struct wl_cursor *cursor;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!input) return;

   _pointer_update_stop(input);

   eina_stringshare_replace(&input->cursor_name, cursor_name);

   /* No cursor. Set to default Left Pointer */
   if (!cursor_name) 
     eina_stringshare_replace(&input->cursor_name, "left_ptr");

   /* try to get this cursor from the theme */
   if (!(cursor = ecore_wl_cursor_get(input->cursor_name)))
     {
        /* if the theme does not have this cursor, default to left pointer */
        if (!(cursor = ecore_wl_cursor_get("left_ptr")))
          return;
     }

   input->cursor = cursor;

   if ((!cursor->images) || (!cursor->images[0]))
     {
        ecore_wl_input_pointer_set(input, NULL, 0, 0);
        return;
     }

   input->cursor_current_index = 0;

   _ecore_wl_input_cursor_update(input);
}

EAPI void
ecore_wl_input_cursor_default_restore(Ecore_Wl_Input *input)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!input) return;

   /* Restore to default wayland cursor */
   ecore_wl_input_cursor_from_name_set(input, "left_ptr");
}

/**
 * @since 1.8
 */
EAPI Ecore_Wl_Input *
ecore_wl_input_get(void)
{
   return _ecore_wl_disp->input;
}

/**
 * @since 1.8
 */
EAPI struct wl_seat *
ecore_wl_input_seat_get(Ecore_Wl_Input *input)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!input) return NULL;

   return input->seat;
}

/* local functions */
void 
_ecore_wl_input_add(Ecore_Wl_Display *ewd, unsigned int id)
{
   Ecore_Wl_Input *input;
   char *temp;
   unsigned int cursor_size;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = malloc(sizeof(Ecore_Wl_Input)))) return;

   memset(input, 0, sizeof(Ecore_Wl_Input));

   input->display = ewd;
   input->pointer_focus = NULL;
   input->keyboard_focus = NULL;
   input->touch_focus = NULL;

   temp = getenv("ECORE_WL_CURSOR_SIZE");
   if (temp)
     cursor_size = atoi(temp);
   else
     cursor_size = ECORE_WL_DEFAULT_CURSOR_SIZE;
   ecore_wl_input_cursor_size_set(input, cursor_size);

   input->seat = 
     wl_registry_bind(ewd->wl.registry, id, &wl_seat_interface, 1);
   ewd->inputs = eina_inlist_append(ewd->inputs, EINA_INLIST_GET(input));

   wl_seat_add_listener(input->seat, 
                        &_ecore_wl_seat_listener, input);
   wl_seat_set_user_data(input->seat, input);

   wl_array_init(&input->data_types);

   if (ewd->wl.data_device_manager)
     {
        input->data_device = 
          wl_data_device_manager_get_data_device(ewd->wl.data_device_manager, 
                                                 input->seat);
        wl_data_device_add_listener(input->data_device, 
                                    &_ecore_wl_data_listener, input);
     }

   ewd->input = input;
}

void 
_ecore_wl_input_del(Ecore_Wl_Input *input)
{
   if (!input) return;

   _pointer_update_stop(input);

   if (input->cursor_name) eina_stringshare_del(input->cursor_name);
   input->cursor_name = NULL;

   if (input->touch_focus)
     {
        input->touch_focus = NULL;
     }

   if (input->pointer_focus)
     {
        Ecore_Wl_Window *win = NULL;

        if ((win = input->pointer_focus))
          win->pointer_device = NULL;

        input->pointer_focus = NULL;
     }

   if (input->keyboard_focus)
     {
        Ecore_Wl_Window *win = NULL;

        if ((win = input->keyboard_focus))
          win->keyboard_device = NULL;

        input->keyboard_focus = NULL;
     }

   if (input->data_types.data)
     {
        char **t;

        wl_array_for_each(t, &input->data_types)
          free(*t);
        wl_array_release(&input->data_types);
     }

   if (input->data_source) wl_data_source_destroy(input->data_source);
   input->data_source = NULL;

   if (input->drag_source) _ecore_wl_dnd_del(input->drag_source);
   input->drag_source = NULL;

   if (input->selection_source) _ecore_wl_dnd_del(input->selection_source);
   input->selection_source = NULL;

   if (input->data_device) wl_data_device_destroy(input->data_device);

   if (input->xkb.state)
     xkb_state_unref(input->xkb.state);
   if (input->xkb.keymap)
     xkb_map_unref(input->xkb.keymap);

   if (input->cursor_surface)
     wl_surface_destroy(input->cursor_surface);

   _ecore_wl_disp->inputs = eina_inlist_remove
      (_ecore_wl_disp->inputs, EINA_INLIST_GET(input));
   if (input->seat) wl_seat_destroy(input->seat);

   if (input->repeat.tmr) ecore_timer_del(input->repeat.tmr);
   input->repeat.tmr = NULL;

   free(input);
}

void 
_ecore_wl_input_pointer_xy_get(int *x, int *y)
{
   if (x) *x = _pointer_x;
   if (y) *y = _pointer_y;
}

static void 
_ecore_wl_input_seat_handle_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps)
{
   Ecore_Wl_Input *input;

   if (!(input = data)) return;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((caps & WL_SEAT_CAPABILITY_POINTER) && (!input->pointer))
     {
        input->pointer = wl_seat_get_pointer(seat);
        wl_pointer_set_user_data(input->pointer, input);
        wl_pointer_add_listener(input->pointer, &pointer_listener, input);

        if (!input->cursor_surface)
          {
             input->cursor_surface = 
               wl_compositor_create_surface(_ecore_wl_disp->wl.compositor);
          }
     }
   else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && (input->pointer))
     {
        if (input->cursor_surface) wl_surface_destroy(input->cursor_surface);
        input->cursor_surface = NULL;
        wl_pointer_destroy(input->pointer);
        input->pointer = NULL;
     }

   if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && (!input->keyboard))
     {
        input->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_set_user_data(input->keyboard, input);
        wl_keyboard_add_listener(input->keyboard, &keyboard_listener, input);
     }
   else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && (input->keyboard))
     {
        wl_keyboard_destroy(input->keyboard);
        input->keyboard = NULL;
     }

   if ((caps & WL_SEAT_CAPABILITY_TOUCH) && (!input->touch))
     {
        input->touch = wl_seat_get_touch(seat);
        wl_touch_set_user_data(input->touch, input);
        wl_touch_add_listener(input->touch, &touch_listener, input);
     }
   else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && (input->touch))
     {
        wl_touch_destroy(input->touch);
        input->touch = NULL;
     }
}

static void 
_ecore_wl_input_cb_pointer_motion(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int timestamp, wl_fixed_t sx, wl_fixed_t sy)
{
   Ecore_Wl_Input *input;

   /* LOGFN(__FILE__, __LINE__, __FUNCTION__); */

   if (!(input = data)) return;

   _pointer_x = input->sx = wl_fixed_to_int(sx);
   _pointer_y = input->sy = wl_fixed_to_int(sy);

   input->timestamp = timestamp;

   if (input->pointer_focus)
     _ecore_wl_input_mouse_move_send(input, input->pointer_focus, timestamp, 0);
}

static void 
_ecore_wl_input_cb_pointer_button(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int serial, unsigned int timestamp, unsigned int button, unsigned int state)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   input->timestamp = timestamp;
   input->display->serial = serial;

//   _ecore_wl_input_mouse_move_send(input, input->pointer_focus, timestamp);

   if (state)
     {
        if ((input->pointer_focus) && (!input->grab))
          {
             ecore_wl_input_grab(input, input->pointer_focus, button);
             input->grab_timestamp = timestamp;
          }

        if (input->pointer_focus)
          _ecore_wl_input_mouse_down_send(input, input->pointer_focus,
                                          0, button, timestamp);
     }
   else
     {
        if (input->pointer_focus)
          _ecore_wl_input_mouse_up_send(input, input->pointer_focus,
                                        0, button, timestamp);

        if ((input->grab) && (input->grab_button == button) && (!state))
          ecore_wl_input_ungrab(input);
     }

//   _ecore_wl_input_mouse_move_send(input, timestamp);
}

static void 
_ecore_wl_input_cb_pointer_axis(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int timestamp, unsigned int axis, wl_fixed_t value)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;
   _ecore_wl_input_mouse_wheel_send(input, axis, wl_fixed_to_int(value), 
                                    timestamp);
}

static void 
_ecore_wl_input_cb_pointer_frame(void *data, struct wl_callback *callback, unsigned int timestamp EINA_UNUSED)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   if (callback)
     {
        if (callback != input->cursor_frame_cb) return;
        wl_callback_destroy(callback);
        input->cursor_frame_cb = NULL;
     }

   if (!input->cursor_name)
     {
        ecore_wl_input_pointer_set(input, NULL, 0, 0);
        return;
     }

   if ((input->cursor->image_count > 1) && (!input->cursor_frame_cb))
     {
        input->cursor_frame_cb = wl_surface_frame(input->cursor_surface);
        wl_callback_add_listener(input->cursor_frame_cb, 
                                 &_ecore_wl_pointer_surface_listener, input);
     }
}

static void 
_ecore_wl_input_cb_keyboard_keymap(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int format, int fd, unsigned int size)
{
   Ecore_Wl_Input *input;
   char *map = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data))
     {
        close(fd);
        return;
     }

   if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
     {
        close(fd);
        return;
     }

   map = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
   if (map == MAP_FAILED)
     {
        close(fd);
        return;
     }

   input->xkb.keymap = 
     xkb_map_new_from_string(input->display->xkb.context, map, 
                             XKB_KEYMAP_FORMAT_TEXT_V1, 0);

   munmap(map, size);
   close(fd);

   if (!(input->xkb.keymap)) return;
   if (!(input->xkb.state = xkb_state_new(input->xkb.keymap)))
     {
        xkb_map_unref(input->xkb.keymap);
        input->xkb.keymap = NULL;
        return;
     }

   input->xkb.control_mask = 
     1 << xkb_map_mod_get_index(input->xkb.keymap, XKB_MOD_NAME_CTRL);
   input->xkb.alt_mask = 
     1 << xkb_map_mod_get_index(input->xkb.keymap, XKB_MOD_NAME_ALT);
   input->xkb.shift_mask = 
     1 << xkb_map_mod_get_index(input->xkb.keymap, XKB_MOD_NAME_SHIFT);
   input->xkb.win_mask = 
     1 << xkb_map_mod_get_index(input->xkb.keymap, XKB_MOD_NAME_LOGO);
   input->xkb.scroll_mask = 
     1 << xkb_map_mod_get_index(input->xkb.keymap, XKB_LED_NAME_SCROLL);
   input->xkb.num_mask = 
     1 << xkb_map_mod_get_index(input->xkb.keymap, XKB_LED_NAME_NUM);
   input->xkb.caps_mask = 
     1 << xkb_map_mod_get_index(input->xkb.keymap, XKB_MOD_NAME_CAPS);
   input->xkb.altgr_mask = 
     1 << xkb_map_mod_get_index(input->xkb.keymap, "ISO_Level3_Shift");
}

static int
_ecore_wl_input_keymap_translate_keysym(xkb_keysym_t keysym, unsigned int modifiers, char *buffer, int bytes)
{
   unsigned long hbytes = 0;
   unsigned char c;

   if (!keysym) return 0;
   hbytes = (keysym >> 8);

   if (!(bytes &&
         ((hbytes == 0) ||
          ((hbytes == 0xFF) &&
           (((keysym >= XKB_KEY_BackSpace) && (keysym <= XKB_KEY_Clear)) ||
            (keysym == XKB_KEY_Return) || (keysym == XKB_KEY_Escape) ||
            (keysym == XKB_KEY_KP_Space) || (keysym == XKB_KEY_KP_Tab) ||
            (keysym == XKB_KEY_KP_Enter) ||
            ((keysym >= XKB_KEY_KP_Multiply) && (keysym <= XKB_KEY_KP_9)) ||
            (keysym == XKB_KEY_KP_Equal) || (keysym == XKB_KEY_Delete))))))
     return 0;

   if (keysym == XKB_KEY_KP_Space)
     c = (XKB_KEY_space & 0x7F);
   else if (hbytes == 0xFF)
     c = (keysym & 0x7F);
   else
     c = (keysym & 0xFF);

   if (modifiers & ECORE_EVENT_MODIFIER_CTRL)
     {
        if (((c >= '@') && (c < '\177')) || c == ' ')
          c &= 0x1F;
        else if (c == '2')
          c = '\000';
        else if ((c >= '3') && (c <= '7'))
          c -= ('3' - '\033');
        else if (c == '8')
          c = '\177';
        else if (c == '/')
          c = '_' & 0x1F;
     }
   buffer[0] = c;
   return 1;
}

static void 
_ecore_wl_input_cb_keyboard_key(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int serial, unsigned int timestamp, unsigned int keycode, unsigned int state)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win;
   unsigned int code, nsyms;
   const xkb_keysym_t *syms;
   xkb_keysym_t sym = XKB_KEY_NoSymbol;
   char key[256], keyname[256], compose[256];
   Ecore_Event_Key *e;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   win = input->keyboard_focus;
   if ((!win) || (win->keyboard_device != input) || (!input->xkb.state)) 
     return;

   input->display->serial = serial;

   /* xkb rules reflect X broken keycodes, so offset by 8 */
   code = keycode + 8;

   /* get the keysym for this key code */
   nsyms = xkb_key_get_syms(input->xkb.state, code, &syms);
   if (nsyms == 1) sym = syms[0];

   /* get the name of this keysym */
   memset(key, 0, sizeof(key));
   xkb_keysym_get_name(sym, key, sizeof(key));

   memset(keyname, 0, sizeof(keyname));
   memcpy(keyname, key, sizeof(keyname));

   if (keyname[0] == '\0')
     snprintf(keyname, sizeof(keyname), "Keycode-%u", code);

   /* if shift is active, we need to transform the key to lower */
   if (xkb_state_mod_index_is_active(input->xkb.state, 
                                     xkb_map_mod_get_index(input->xkb.keymap, 
                                                           XKB_MOD_NAME_SHIFT),
                                     XKB_STATE_MODS_EFFECTIVE))
     {
        if (keyname[0] != '\0')
          keyname[0] = tolower(keyname[0]);
     }

   memset(compose, 0, sizeof(compose));
   _ecore_wl_input_keymap_translate_keysym(sym, input->modifiers, 
                                           compose, sizeof(compose));

   e = malloc(sizeof(Ecore_Event_Key) + strlen(key) + strlen(keyname) +
              ((compose[0] != '\0') ? strlen(compose) : 0) + 3);
   if (!e) return;

   e->keyname = (char *)(e + 1);
   e->key = e->keyname + strlen(keyname) + 1;
   e->compose = strlen(compose) ? e->key + strlen(key) + 1 : NULL;
   e->string = e->compose;

   strcpy((char *)e->keyname, keyname);
   strcpy((char *)e->key, key);
   if (strlen(compose)) strcpy((char *)e->compose, compose);

   e->window = win->id;
   e->event_window = win->id;
   e->timestamp = timestamp;

   e->modifiers = input->modifiers;

   if (state)
     ecore_event_add(ECORE_EVENT_KEY_DOWN, e, NULL, NULL);
   else
     ecore_event_add(ECORE_EVENT_KEY_UP, e, NULL, NULL);

   if ((!state) && (keycode == input->repeat.key))
     {
        input->repeat.sym = 0;
        input->repeat.key = 0;
        input->repeat.time = 0;

        if (input->repeat.tmr) ecore_timer_del(input->repeat.tmr);
        input->repeat.tmr = NULL;
     }
   else if ((state) && (keycode != input->repeat.key))
     {
        input->repeat.sym = sym;
        input->repeat.key = keycode;
        input->repeat.time = timestamp;

        if (!input->repeat.tmr)
          {
             input->repeat.tmr = 
               ecore_timer_add(0.025, _ecore_wl_input_cb_keyboard_repeat, input);
          }
        ecore_timer_delay(input->repeat.tmr, 0.4);
     }
}

static void 
_ecore_wl_input_cb_keyboard_modifiers(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int serial EINA_UNUSED, unsigned int depressed, unsigned int latched, unsigned int locked, unsigned int group)
{
   Ecore_Wl_Input *input;
   xkb_mod_mask_t mask;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   input->modifiers = 0;
   if (!input->xkb.state) return;

   xkb_state_update_mask(input->xkb.state, 
                         depressed, latched, locked, 0, 0, group);
   mask = xkb_state_serialize_mods(input->xkb.state, 
                                   (XKB_STATE_DEPRESSED | XKB_STATE_LATCHED));
   if (mask & input->xkb.control_mask)
     input->modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if (mask & input->xkb.alt_mask)
     input->modifiers |= ECORE_EVENT_MODIFIER_ALT;
   if (mask & input->xkb.shift_mask)
     input->modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if (mask & input->xkb.win_mask)
     input->modifiers |= ECORE_EVENT_MODIFIER_WIN;
   if (mask & input->xkb.scroll_mask)
     input->modifiers |= ECORE_EVENT_LOCK_SCROLL;
   if (mask & input->xkb.num_mask)
     input->modifiers |= ECORE_EVENT_LOCK_NUM;
   if (mask & input->xkb.caps_mask)
     input->modifiers |= ECORE_EVENT_LOCK_CAPS;
   if (mask & input->xkb.altgr_mask)
     input->modifiers |= ECORE_EVENT_MODIFIER_ALTGR;
}

static Eina_Bool 
_ecore_wl_input_cb_keyboard_repeat(void *data)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return ECORE_CALLBACK_RENEW;

   if ((win = input->keyboard_focus))
     _ecore_wl_input_cb_keyboard_key(input, NULL, input->display->serial, 
                                     input->repeat.time, 
                                     input->repeat.key, EINA_TRUE);

   return ECORE_CALLBACK_RENEW;
}

static void 
_ecore_wl_input_cb_pointer_enter(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int serial, struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!surface) return;
   if (!(input = data)) return;

   if (!input->timestamp)
     {
        struct timeval tv;

        gettimeofday(&tv, NULL);
        input->timestamp = (tv.tv_sec * 1000 + tv.tv_usec / 1000);
     }

   input->sx = wl_fixed_to_double(sx);
   input->sy = wl_fixed_to_double(sy);
   input->display->serial = serial;
   input->pointer_enter_serial = serial;

   /* The cursor on the surface is undefined until we set it */
   ecore_wl_input_cursor_from_name_set(input, "left_ptr");

   if ((win = ecore_wl_window_surface_find(surface)))
     {
        win->pointer_device = input;
        input->pointer_focus = win;

        if (win->pointer.set)
          {
             ecore_wl_input_pointer_set(input, win->pointer.surface, 
                                        win->pointer.hot_x, win->pointer.hot_y);
          }
        /* NB: Commented out for now. Not needed in most circumstances, 
         * but left here for any corner-cases */
        /* else */
        /*   { */
        /*      _ecore_wl_input_cursor_update(input); */
        /*   } */

        _ecore_wl_input_mouse_in_send(input, win, input->timestamp);
     }
}

static void 
_ecore_wl_input_cb_pointer_leave(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int serial, struct wl_surface *surface)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!surface) return;
   if (!(input = data)) return;

   input->display->serial = serial;

   /* NB: Commented out for now. Not needed in most circumstances, but left 
    * here for any corner-cases */
   /* _ecore_wl_input_cursor_update(input); */

   if (!surface) return;
   if (!(win = ecore_wl_window_surface_find(surface))) return;

   win->pointer_device = NULL;
   input->pointer_focus = NULL;

   /* _ecore_wl_input_mouse_move_send(input, win, input->timestamp); */
   _ecore_wl_input_mouse_out_send(input, win, input->timestamp);

   if (input->grab)
     {
        /* move or resize started */

        /* printf("Pointer Leave WITH a Grab\n"); */
     }
}

static void 
_ecore_wl_input_cb_keyboard_enter(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int serial, struct wl_surface *surface, struct wl_array *keys EINA_UNUSED)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!surface) return;
   if (!(input = data)) return;

   if (!input->timestamp)
     {
        struct timeval tv;

        gettimeofday(&tv, NULL);
        input->timestamp = (tv.tv_sec * 1000 + tv.tv_usec / 1000);
     }

   input->display->serial = serial;

   if (!(win = ecore_wl_window_surface_find(surface))) return;

   win->keyboard_device = input;
   input->keyboard_focus = win;

   _ecore_wl_input_focus_in_send(input, win, input->timestamp);
}

static void 
_ecore_wl_input_cb_keyboard_leave(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int serial, struct wl_surface *surface)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!surface) return;
   if (!(input = data)) return;

   if (input->repeat.tmr) ecore_timer_del(input->repeat.tmr);
   input->repeat.tmr = NULL;

   if (!input->timestamp)
     {
        struct timeval tv;

        gettimeofday(&tv, NULL);
        input->timestamp = (tv.tv_sec * 1000 + tv.tv_usec / 1000);
     }

   input->display->serial = serial;

   if (!surface) return;
   if (!(win = ecore_wl_window_surface_find(surface))) return;

   win->keyboard_device = NULL;
   _ecore_wl_input_focus_out_send(input, win, input->timestamp);

   input->keyboard_focus = NULL;
}

static void 
_ecore_wl_input_cb_touch_down(void *data, struct wl_touch *touch EINA_UNUSED, unsigned int serial, unsigned int timestamp, struct wl_surface *surface, int id, wl_fixed_t x, wl_fixed_t y)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!surface) return;
   if (!(input = data)) return;

   if (!(win = ecore_wl_window_surface_find(surface))) return;

   input->touch_focus = win;
   input->timestamp = timestamp;
   input->display->serial = serial;
   input->sx = wl_fixed_to_int(x);
   input->sy = wl_fixed_to_int(y);

   _ecore_wl_input_mouse_move_send(input, input->touch_focus, timestamp, id);
   _ecore_wl_input_cb_pointer_enter(data, NULL, serial, surface, x, y);
   if ((input->touch_focus) && (!input->grab))
     {
        ecore_wl_input_grab(input, input->touch_focus, BTN_LEFT);
        input->grab_timestamp = timestamp;
     }

   _ecore_wl_input_mouse_down_send(input, input->touch_focus,
                                   id, BTN_LEFT, timestamp);
}

static void 
_ecore_wl_input_cb_touch_up(void *data, struct wl_touch *touch EINA_UNUSED, unsigned int serial, unsigned int timestamp, int id)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;
   if (!input->touch_focus) return;

   input->timestamp = timestamp;
   input->display->serial = serial;

   _ecore_wl_input_mouse_up_send(input, input->touch_focus, id, BTN_LEFT, timestamp);
   if ((input->grab) && (input->grab_button == BTN_LEFT))
     ecore_wl_input_ungrab(input);
}

static void 
_ecore_wl_input_cb_touch_motion(void *data, struct wl_touch *touch EINA_UNUSED, unsigned int timestamp, int id, wl_fixed_t x, wl_fixed_t y)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;
   if (!input->touch_focus) return;

   input->timestamp = timestamp;
   input->sx = wl_fixed_to_int(x);
   input->sy = wl_fixed_to_int(y);

   _ecore_wl_input_mouse_move_send(input, input->touch_focus, timestamp, id);
}

static void 
_ecore_wl_input_cb_touch_frame(void *data EINA_UNUSED, struct wl_touch *touch EINA_UNUSED)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
}

static void 
_ecore_wl_input_cb_touch_cancel(void *data EINA_UNUSED, struct wl_touch *touch EINA_UNUSED)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
}

static void 
_ecore_wl_input_cb_data_offer(void *data, struct wl_data_device *data_device, struct wl_data_offer *offer)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_dnd_add(data, data_device, offer);
}

static void 
_ecore_wl_input_cb_data_enter(void *data, struct wl_data_device *data_device, unsigned int timestamp, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y, struct wl_data_offer *offer)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!surface) return;

   _ecore_wl_dnd_enter(data, data_device, timestamp, surface, x, y, offer);
}

static void 
_ecore_wl_input_cb_data_leave(void *data, struct wl_data_device *data_device)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_dnd_leave(data, data_device);
}

static void 
_ecore_wl_input_cb_data_motion(void *data, struct wl_data_device *data_device, unsigned int timestamp, wl_fixed_t x, wl_fixed_t y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_dnd_motion(data, data_device, timestamp, x, y);
}

static void 
_ecore_wl_input_cb_data_drop(void *data, struct wl_data_device *data_device)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_dnd_drop(data, data_device);
}

static void 
_ecore_wl_input_cb_data_selection(void *data, struct wl_data_device *data_device, struct wl_data_offer *offer)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_dnd_selection(data, data_device, offer);
}

static void 
_ecore_wl_input_mouse_move_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int timestamp, int device)
{
   Ecore_Event_Mouse_Move *ev;
   Ecore_Wl_Mouse_Down_Info *down_info;

   /* LOGFN(__FILE__, __LINE__, __FUNCTION__); */

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Move)))) return;

   ev->timestamp = timestamp;
   ev->x = input->sx;
   ev->y = input->sy;
   ev->root.x = input->sx;
   ev->root.y = input->sy;
   ev->modifiers = input->modifiers;
   ev->multi.device = device;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = input->sx;
   ev->multi.y = input->sy;
   ev->multi.root.x = input->sx;
   ev->multi.root.y = input->sy;

   if ((down_info = _ecore_wl_mouse_down_info_get(device)))
     {
        down_info->sx = input->sx;
        down_info->sy = input->sy;
     }

   if (win)
     {
        ev->window = win->id;
        ev->event_window = win->id;
     }

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
}

static void 
_ecore_wl_input_mouse_in_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int timestamp)
{
   Ecore_Wl_Event_Mouse_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Mouse_In)))) return;

   ev->x = input->sx;
   ev->y = input->sy;
   ev->root.x = input->sx;
   ev->root.y = input->sy;
   ev->modifiers = input->modifiers;
   ev->timestamp = timestamp;

   if (win)
     {
        ev->window = win->id;
        ev->event_window = win->id;
     }

   ecore_event_add(ECORE_WL_EVENT_MOUSE_IN, ev, NULL, NULL);
}

static void 
_ecore_wl_input_mouse_out_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int timestamp)
{
   Ecore_Wl_Event_Mouse_Out *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Mouse_Out)))) return;

   ev->x = input->sx;
   ev->y = input->sy;
   ev->root.x = input->sx;
   ev->root.y = input->sy;
   ev->modifiers = input->modifiers;
   ev->timestamp = timestamp;

   if (win)
     {
        ev->window = win->id;
        ev->event_window = win->id;
     }

   ecore_event_add(ECORE_WL_EVENT_MOUSE_OUT, ev, NULL, NULL);
}

static void 
_ecore_wl_input_focus_in_send(Ecore_Wl_Input *input EINA_UNUSED, Ecore_Wl_Window *win, unsigned int timestamp)
{
   Ecore_Wl_Event_Focus_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Focus_In)))) return;
   ev->timestamp = timestamp;
   if (win) ev->win = win->id;
   ecore_event_add(ECORE_WL_EVENT_FOCUS_IN, ev, NULL, NULL);
}

static void 
_ecore_wl_input_focus_out_send(Ecore_Wl_Input *input EINA_UNUSED, Ecore_Wl_Window *win, unsigned int timestamp)
{
   Ecore_Wl_Event_Focus_Out *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Focus_Out)))) return;
   ev->timestamp = timestamp;
   if (win) ev->win = win->id;
   ecore_event_add(ECORE_WL_EVENT_FOCUS_OUT, ev, NULL, NULL);
}

static void 
_ecore_wl_input_mouse_down_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, int device, unsigned int button, unsigned int timestamp)
{
   Ecore_Event_Mouse_Button *ev;
   Ecore_Wl_Mouse_Down_Info *down_info;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Button)))) return;

   if (button == BTN_LEFT)
     ev->buttons = 1;
   else if (button == BTN_MIDDLE)
     ev->buttons = 2;
   else if (button == BTN_RIGHT)
     ev->buttons = 3;
   else
     ev->buttons = button;

   ev->timestamp = timestamp;
   ev->x = input->sx;
   ev->y = input->sy;
   ev->root.x = input->sx;
   ev->root.y = input->sy;
   ev->modifiers = input->modifiers;

   ev->double_click = 0;
   ev->triple_click = 0;

   /* handling double and triple click, taking into account multiple input
    * devices */
   if ((down_info = _ecore_wl_mouse_down_info_get(device)))
     {
        down_info->sx = input->sx;
        down_info->sy = input->sy;
        if (down_info->did_triple)
          {
             down_info->last_win = 0;
             down_info->last_last_win = 0;
             down_info->last_event_win = 0;
             down_info->last_last_event_win = 0;
             down_info->last_time = 0;
             down_info->last_last_time = 0;
          }
        //Check Double Clicked
        if (((int)(timestamp - down_info->last_time) <=
             (int)(1000 * _ecore_wl_double_click_time)) &&
            ((win) && 
                (win->id == down_info->last_win) &&
                (win->id == down_info->last_event_win)))
          {
             ev->double_click = 1;
             down_info->did_double = EINA_TRUE;
          }
        else
          {
             down_info->did_double = EINA_FALSE;
             down_info->did_triple = EINA_FALSE;
          }

        //Check Triple Clicked
        if (((int)(timestamp - down_info->last_last_time) <=
             (int)(2 * 1000 * _ecore_wl_double_click_time)) &&
            ((win) && 
                (win->id == down_info->last_win) &&
                (win->id == down_info->last_last_win) &&
                (win->id == down_info->last_event_win) &&
                (win->id == down_info->last_last_event_win)))
          {
             ev->triple_click = 1;
             down_info->did_triple = EINA_TRUE;
          }
        else
          {
             down_info->did_triple = EINA_FALSE;
          }
     }

   ev->multi.device = device;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = input->sx;
   ev->multi.y = input->sy;
   ev->multi.root.x = input->sx;
   ev->multi.root.y = input->sy;

   if (win)
     {
        ev->window = win->id;
        ev->event_window = win->id;
     }

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);

   if ((down_info) &&
       (!down_info->did_triple))
     {
        down_info->last_last_win = down_info->last_win;
        down_info->last_win = ev->window;
        down_info->last_last_event_win = down_info->last_event_win;
        down_info->last_event_win = ev->window;
        down_info->last_last_time = down_info->last_time;
        down_info->last_time = timestamp;
     }
}

static void 
_ecore_wl_input_mouse_up_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, int device, unsigned int button, unsigned int timestamp)
{
   Ecore_Event_Mouse_Button *ev;
   Ecore_Wl_Mouse_Down_Info *down_info;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Button)))) return;

   if (button == BTN_LEFT)
     ev->buttons = 1;
   else if (button == BTN_MIDDLE)
     ev->buttons = 2;
   else if (button == BTN_RIGHT)
     ev->buttons = 3;
   else
     ev->buttons = button;

   ev->timestamp = timestamp;
   ev->root.x = input->sx;
   ev->root.y = input->sy;
   ev->modifiers = input->modifiers;

   ev->double_click = 0;
   ev->triple_click = 0;

   if ((down_info = _ecore_wl_mouse_down_info_get(device)))
     {
        if (down_info->did_double)
          ev->double_click = 1;
        if (down_info->did_triple)
          ev->triple_click = 1;
        ev->x = down_info->sx;
        ev->y = down_info->sy;
        ev->multi.x = down_info->sx;
        ev->multi.y = down_info->sy;
     }
   else
     {
        ev->x = input->sx;
        ev->y = input->sy;
        ev->multi.x = input->sx;
        ev->multi.y = input->sy;
     }

   ev->multi.device = device;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.root.x = input->sx;
   ev->multi.root.y = input->sy;

   if (win)
     {
        ev->window = win->id;
        ev->event_window = win->id;
     }

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
}

static void 
_ecore_wl_input_mouse_wheel_send(Ecore_Wl_Input *input, unsigned int axis, int value, unsigned int timestamp)
{
   Ecore_Event_Mouse_Wheel *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Wheel)))) return;

   ev->timestamp = timestamp;
   ev->modifiers = input->modifiers;
   ev->x = input->sx;
   ev->y = input->sy;
   /* ev->root.x = input->sx; */
   /* ev->root.y = input->sy; */

   if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
     {
        ev->direction = 0;
        ev->z = value;
     }
   else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
     {
        ev->direction = 1;
        ev->z = value;
     }

   if (input->grab)
     {
        ev->window = input->grab->id;
        ev->event_window = input->grab->id;
     }
   else if (input->pointer_focus)
     {
        ev->window = input->pointer_focus->id;
        ev->event_window = input->pointer_focus->id;
     }

   ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
}

static void
_ecore_wl_mouse_down_info_clear(void)
{
   Eina_Inlist *l;
   Ecore_Wl_Mouse_Down_Info *info = NULL;

   l = _ecore_wl_mouse_down_info_list;
   while (l)
     {
        info = EINA_INLIST_CONTAINER_GET(l, Ecore_Wl_Mouse_Down_Info);
        l = eina_inlist_remove(l, l);
        free(info);
     }
   _ecore_wl_mouse_down_info_list = NULL;
}

static Ecore_Wl_Mouse_Down_Info *
_ecore_wl_mouse_down_info_get(int dev)
{
   Eina_Inlist *l = NULL;
   Ecore_Wl_Mouse_Down_Info *info = NULL;

   //Return the exist info
   l = _ecore_wl_mouse_down_info_list;
   EINA_INLIST_FOREACH(l, info)
     if (info->dev == dev) return info;

   //New Device. Add it.
   info = calloc(1, sizeof(Ecore_Wl_Mouse_Down_Info));
   if (!info) return NULL;

   info->dev = dev;
   l = eina_inlist_append(l, (Eina_Inlist *)info);
   _ecore_wl_mouse_down_info_list = l;
   return info;
}

void
_ecore_wl_events_init(void)
{
}

void
_ecore_wl_events_shutdown(void)
{
   _ecore_wl_mouse_down_info_clear();
}

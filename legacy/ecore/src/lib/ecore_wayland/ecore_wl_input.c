#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/**
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
#include <sys/timerfd.h>

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

#define MOD_SHIFT_MASK 0x01
#define MOD_ALT_MASK 0x02
#define MOD_CONTROL_MASK 0x04

Ecore_Wl_Dnd *glb_dnd = NULL;

/* local function prototypes */
static void _ecore_wl_input_seat_handle_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps);

static void _ecore_wl_input_cb_pointer_enter(void *data, struct wl_pointer *pointer __UNUSED__, unsigned int serial, struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy);
static void _ecore_wl_input_cb_pointer_leave(void *data, struct wl_pointer *pointer __UNUSED__, unsigned int serial, struct wl_surface *surface);
static void _ecore_wl_input_cb_pointer_motion(void *data, struct wl_pointer *pointer __UNUSED__, unsigned int timestamp, wl_fixed_t sx, wl_fixed_t sy);
static void _ecore_wl_input_cb_pointer_button(void *data, struct wl_pointer *pointer __UNUSED__, unsigned int serial, unsigned int timestamp, unsigned int button, unsigned int state);
static void _ecore_wl_input_cb_pointer_axis(void *data, struct wl_pointer *pointer __UNUSED__, unsigned int timestamp, unsigned int axis, wl_fixed_t value);
static void _ecore_wl_input_cb_pointer_frame(void *data, struct wl_callback *callback, unsigned int timestamp __UNUSED__);
static void _ecore_wl_input_cb_keyboard_keymap(void *data, struct wl_keyboard *keyboard __UNUSED__, unsigned int format, int fd, unsigned int size);
static void _ecore_wl_input_cb_keyboard_enter(void *data, struct wl_keyboard *keyboard __UNUSED__, unsigned int serial, struct wl_surface *surface, struct wl_array *keys __UNUSED__);
static void _ecore_wl_input_cb_keyboard_leave(void *data, struct wl_keyboard *keyboard __UNUSED__, unsigned int serial, struct wl_surface *surface);
static void _ecore_wl_input_cb_keyboard_key(void *data, struct wl_keyboard *keyboard __UNUSED__, unsigned int serial, unsigned int timestamp, unsigned int key, unsigned int state);
static void _ecore_wl_input_cb_keyboard_modifiers(void *data, struct wl_keyboard *keyboard __UNUSED__, unsigned int serial __UNUSED__, unsigned int depressed, unsigned int latched, unsigned int locked, unsigned int group);
static Eina_Bool _ecore_wl_input_cb_keyboard_repeat(void *data, Ecore_Fd_Handler *handler __UNUSED__);
static void _ecore_wl_input_cb_touch_down(void *data, struct wl_touch *touch __UNUSED__, unsigned int serial, unsigned int timestamp, struct wl_surface *surface __UNUSED__, int id __UNUSED__, wl_fixed_t x, wl_fixed_t y);
static void _ecore_wl_input_cb_touch_up(void *data, struct wl_touch *touch __UNUSED__, unsigned int serial, unsigned int timestamp, int id __UNUSED__);
static void _ecore_wl_input_cb_touch_motion(void *data, struct wl_touch *touch __UNUSED__, unsigned int timestamp, int id __UNUSED__, wl_fixed_t x, wl_fixed_t y);
static void _ecore_wl_input_cb_touch_frame(void *data __UNUSED__, struct wl_touch *touch __UNUSED__);
static void _ecore_wl_input_cb_touch_cancel(void *data __UNUSED__, struct wl_touch *touch __UNUSED__);
static void _ecore_wl_input_cb_data_offer(void *data, struct wl_data_device *data_device, struct wl_data_offer *offer);
static void _ecore_wl_input_cb_data_enter(void *data, struct wl_data_device *data_device, unsigned int timestamp, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y, struct wl_data_offer *offer);
static void _ecore_wl_input_cb_data_leave(void *data, struct wl_data_device *data_device);
static void _ecore_wl_input_cb_data_motion(void *data, struct wl_data_device *data_device, unsigned int timestamp, wl_fixed_t x, wl_fixed_t y);
static void _ecore_wl_input_cb_data_drop(void *data, struct wl_data_device *data_device);
static void _ecore_wl_input_cb_data_selection(void *data, struct wl_data_device *data_device, struct wl_data_offer *offer);

static void _ecore_wl_input_mouse_move_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int timestamp);
static void _ecore_wl_input_mouse_in_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int timestamp);
static void _ecore_wl_input_mouse_out_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int timestamp);
static void _ecore_wl_input_focus_in_send(Ecore_Wl_Input *input __UNUSED__, Ecore_Wl_Window *win, unsigned int timestamp);
static void _ecore_wl_input_focus_out_send(Ecore_Wl_Input *input __UNUSED__, Ecore_Wl_Window *win, unsigned int timestamp);
static void _ecore_wl_input_mouse_down_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int timestamp);
static void _ecore_wl_input_mouse_up_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int timestamp);
static void _ecore_wl_input_mouse_wheel_send(Ecore_Wl_Input *input, unsigned int axis, int value, unsigned int timestamp);

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

EAPI void
ecore_wl_input_pointer_set(Ecore_Wl_Input *input, struct wl_surface *surface, int hot_x, int hot_y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (input)
     wl_pointer_set_cursor(input->pointer, input->pointer_enter_serial, 
                           surface, hot_x, hot_y);
}

EAPI void
ecore_wl_input_cursor_from_name_set(Ecore_Wl_Input *input, const char *cursor_name)
{
   struct wl_cursor_image *cursor_image;
   struct wl_buffer *buffer;
   struct wl_cursor *cursor;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!input) return;

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

   if ((!cursor->images) || (!cursor->images[0]))
     {
        ecore_wl_input_pointer_set(input, NULL, 0, 0);
        return;
     }

   cursor_image = cursor->images[0];
   if ((buffer = wl_cursor_image_get_buffer(cursor_image)))
     {
        ecore_wl_input_pointer_set(input, input->cursor_surface, 
                                   cursor_image->hotspot_x, 
                                   cursor_image->hotspot_y);
        wl_surface_attach(input->cursor_surface, buffer, 0, 0);
        wl_surface_damage(input->cursor_surface, 0, 0, 
                          cursor_image->width, cursor_image->height);

        if (!input->cursor_frame_cb)
          _ecore_wl_input_cb_pointer_frame(input, NULL, 0);
     }
}

EAPI void
ecore_wl_input_cursor_default_restore(Ecore_Wl_Input *input)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!input) return;

   /* Restore to default wayland cursor */
   ecore_wl_input_cursor_from_name_set(input, "left_ptr");
}

/* local functions */
void 
_ecore_wl_input_add(Ecore_Wl_Display *ewd, unsigned int id)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = malloc(sizeof(Ecore_Wl_Input)))) return;

   memset(input, 0, sizeof(Ecore_Wl_Input));

   input->display = ewd;
   input->pointer_focus = NULL;
   input->keyboard_focus = NULL;

   input->seat = 
     wl_display_bind(ewd->wl.display, id, &wl_seat_interface);
   wl_list_insert(ewd->inputs.prev, &input->link);
   wl_seat_add_listener(input->seat, 
                        &_ecore_wl_seat_listener, input);
   wl_seat_set_user_data(input->seat, input);

   input->data_device = 
     wl_data_device_manager_get_data_device(ewd->wl.data_device_manager, 
                                            input->seat);
   wl_data_device_add_listener(input->data_device, 
                               &_ecore_wl_data_listener, input);
   input->cursor_surface = 
     wl_compositor_create_surface(_ecore_wl_disp->wl.compositor);

   input->repeat.timerfd = 
     timerfd_create(CLOCK_MONOTONIC, (TFD_CLOEXEC | TFD_NONBLOCK));

   input->repeat.hdlr = 
     ecore_main_fd_handler_add(input->repeat.timerfd, ECORE_FD_READ, 
                               _ecore_wl_input_cb_keyboard_repeat, input, 
                               NULL, NULL);

   ewd->input = input;

   /* create Ecore_Wl_Dnd */
   if (!glb_dnd)
     if (!(glb_dnd = calloc(1, sizeof(Ecore_Wl_Dnd)))) return;
   glb_dnd->ewd = ewd;
   glb_dnd->input = input;
   input->dnd = glb_dnd;
   wl_array_init(&glb_dnd->types_offered);
}

void 
_ecore_wl_input_del(Ecore_Wl_Input *input)
{
   if (!input) return;

   if (input->cursor_name) eina_stringshare_del(input->cursor_name);
   input->cursor_name = NULL;

   if (input->keyboard_focus)
     {
        Ecore_Wl_Window *win = NULL;

        if ((win = input->keyboard_focus))
          win->keyboard_device = NULL;

        input->keyboard_focus = NULL;
     }

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

   wl_list_remove(&input->link);
   if (input->seat) wl_seat_destroy(input->seat);

   if (input->repeat.hdlr) ecore_main_fd_handler_del(input->repeat.hdlr);
   input->repeat.hdlr = NULL;

   if (input->repeat.timerfd) close(input->repeat.timerfd);
   input->repeat.timerfd = 0;

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
   if ((caps & WL_SEAT_CAPABILITY_POINTER) && (!input->pointer))
     {
        input->pointer = wl_seat_get_pointer(seat);
        wl_pointer_set_user_data(input->pointer, input);
        wl_pointer_add_listener(input->pointer, &pointer_listener, input);
     }
   else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && (input->pointer))
     {
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
_ecore_wl_input_cb_pointer_motion(void *data, struct wl_pointer *pointer __UNUSED__, unsigned int timestamp, wl_fixed_t sx, wl_fixed_t sy)
{
   Ecore_Wl_Input *input;

   /* LOGFN(__FILE__, __LINE__, __FUNCTION__); */

   if (!(input = data)) return;

   _pointer_x = input->sx = wl_fixed_to_int(sx);
   _pointer_y = input->sy = wl_fixed_to_int(sy);

   input->timestamp = timestamp;

   if (input->pointer_focus)
     _ecore_wl_input_mouse_move_send(input, input->pointer_focus, timestamp);

   ecore_wl_input_cursor_from_name_set(input, input->cursor_name);
}

static void 
_ecore_wl_input_cb_pointer_button(void *data, struct wl_pointer *pointer __UNUSED__, unsigned int serial, unsigned int timestamp, unsigned int button, unsigned int state)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   input->timestamp = timestamp;
   input->display->serial = serial;

//   _ecore_wl_input_mouse_move_send(input, input->pointer_focus, timestamp);

   if (state)
     {
        if ((input->pointer_focus) && (!input->grab) && (state))
          ecore_wl_input_grab(input, input->pointer_focus, button);

        input->button = button;
        _ecore_wl_input_mouse_down_send(input, input->pointer_focus, 
                                        timestamp);
     }
   else
     {
        _ecore_wl_input_mouse_up_send(input, input->pointer_focus, 
                                      timestamp);
        input->button = 0;

        if ((input->grab) && (input->grab_button == button) && (!state))
          ecore_wl_input_ungrab(input);
     }

//   _ecore_wl_input_mouse_move_send(input, timestamp);
}

static void 
_ecore_wl_input_cb_pointer_axis(void *data, struct wl_pointer *pointer __UNUSED__, unsigned int timestamp, unsigned int axis, wl_fixed_t value)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;
   _ecore_wl_input_mouse_wheel_send(input, axis, wl_fixed_to_int(value), 
                                    timestamp);
}

static void 
_ecore_wl_input_cb_pointer_frame(void *data, struct wl_callback *callback, unsigned int timestamp __UNUSED__)
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

   if (!input->cursor_frame_cb)
     {
        input->cursor_frame_cb = wl_surface_frame(input->cursor_surface);
        wl_callback_add_listener(input->cursor_frame_cb, 
                                 &_ecore_wl_pointer_surface_listener, input);
     }
}

static void 
_ecore_wl_input_cb_keyboard_keymap(void *data, struct wl_keyboard *keyboard __UNUSED__, unsigned int format, int fd, unsigned int size)
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
     1 << xkb_map_mod_get_index(input->xkb.keymap, "Control");
   input->xkb.alt_mask = 
     1 << xkb_map_mod_get_index(input->xkb.keymap, "Mod1");
   input->xkb.shift_mask = 
     1 << xkb_map_mod_get_index(input->xkb.keymap, "Shift");
}

static void 
_ecore_wl_input_cb_keyboard_key(void *data, struct wl_keyboard *keyboard __UNUSED__, unsigned int serial, unsigned int timestamp, unsigned int keycode, unsigned int state)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win;
   unsigned int code, num;
   const xkb_keysym_t *syms;
   xkb_keysym_t sym = XKB_KEY_NoSymbol;
   xkb_mod_mask_t mask;
   char string[32], key[32], keyname[32];// compose[32];
   Ecore_Event_Key *e;
   struct itimerspec ts;
   int len = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;
   input->display->serial = serial;

   /* xkb rules reflect X broken keycodes, so offset by 8 */
   code = keycode + 8;

   win = input->keyboard_focus;
   if ((!win) || (win->keyboard_device != input) || (!input->xkb.state)) 
     return;

   mask = xkb_state_serialize_mods(input->xkb.state, 
                                   XKB_STATE_DEPRESSED | XKB_STATE_LATCHED);

   input->modifiers = 0;

   /* The Ecore_Event_Modifiers don't quite match the X mask bits */
   if (mask & input->xkb.control_mask)
     input->modifiers |= MOD_CONTROL_MASK;
   if (mask & input->xkb.alt_mask)
     input->modifiers |= MOD_ALT_MASK;
   if (mask & input->xkb.shift_mask)
     input->modifiers |= MOD_SHIFT_MASK;

   num = xkb_key_get_syms(input->xkb.state, code, &syms);
   if (num == 1) sym = syms[0];

   memset(key, 0, sizeof(key));
   xkb_keysym_get_name(sym, key, sizeof(key));

   memset(keyname, 0, sizeof(keyname));
   xkb_keysym_get_name(sym, keyname, sizeof(keyname));
   if (keyname[0] == '\0')
     snprintf(keyname, sizeof(keyname), "Keycode-%i", code);

   memset(string, 0, sizeof(string));
   if (xkb_keysym_to_utf8(sym, string, 32) <= 0)
     {
        /* FIXME: NB: We may need to add more checks here for other 
         * non-printable characters */
        if ((sym == XKB_KEY_Tab) || (sym == XKB_KEY_ISO_Left_Tab)) 
          string[len++] = '\t';
     }

   /* FIXME: NB: Start hacking on compose key support */
   /* memset(compose, 0, sizeof(compose)); */
   /* if (sym == XKB_KEY_Multi_key) */
   /*   { */
   /*      if (xkb_keysym_to_utf8(sym, compose, 32) <= 0) */
   /*        compose[0] = '\0'; */
   /*   } */

   e = malloc(sizeof(Ecore_Event_Key) + strlen(key) + strlen(keyname) +
              ((string[0] != '\0') ? strlen(string) : 0) + 3);
   if (!e) return;

   e->keyname = (char *)(e + 1);
   e->key = e->keyname + strlen(keyname) + 1;
   e->string = strlen(string) ? e->key + strlen(key) + 1 : NULL;
   e->compose = e->string;

   strcpy((char *)e->keyname, keyname);
   strcpy((char *)e->key, key);
   if (strlen(string)) strcpy((char *)e->string, string);

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

        ts.it_interval.tv_sec = 0;
        ts.it_interval.tv_nsec = 0;
        ts.it_value.tv_sec = 0;
        ts.it_value.tv_nsec = 0;

        timerfd_settime(input->repeat.timerfd, 0, &ts, NULL);
     }
   else if ((state) && 
            ((!input->repeat.key) || 
                ((keycode) && (keycode != input->repeat.key))))
     {
        input->repeat.sym = sym;
        input->repeat.key = keycode;
        input->repeat.time = timestamp;

        /* interval after expires */
        ts.it_interval.tv_sec = 0;
        ts.it_interval.tv_nsec = 35 * 1000 * 1000;

        /* initial expiration */
        ts.it_value.tv_sec = 0;
        ts.it_value.tv_nsec = 500 * 1000 * 1000;

        timerfd_settime(input->repeat.timerfd, 0, &ts, NULL);
     }
}

static void 
_ecore_wl_input_cb_keyboard_modifiers(void *data, struct wl_keyboard *keyboard __UNUSED__, unsigned int serial __UNUSED__, unsigned int depressed, unsigned int latched, unsigned int locked, unsigned int group)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;
   xkb_state_update_mask(input->xkb.state, depressed, latched, 
                         locked, 0, 0, group);
}

static Eina_Bool 
_ecore_wl_input_cb_keyboard_repeat(void *data, Ecore_Fd_Handler *handler __UNUSED__)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win = NULL;
   unsigned long long int xp;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return ECORE_CALLBACK_RENEW;

   /* Trap for EAGAIN */
   if (read(input->repeat.timerfd, &xp, sizeof(xp)) != sizeof(xp))
     return ECORE_CALLBACK_RENEW;

   if ((win = input->keyboard_focus))
     _ecore_wl_input_cb_keyboard_key(input, NULL, input->display->serial, 
                                     input->repeat.time, 
                                     input->repeat.key, EINA_TRUE);

   return ECORE_CALLBACK_RENEW;
}

static void 
_ecore_wl_input_cb_pointer_enter(void *data, struct wl_pointer *pointer __UNUSED__, unsigned int serial, struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy)
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

   if (!(win = wl_surface_get_user_data(surface))) return;

   win->pointer_device = input;
   input->pointer_focus = win;

   /* _ecore_wl_input_mouse_move_send(input, win, input->timestamp); */
   _ecore_wl_input_mouse_in_send(input, win, input->timestamp);

   /* The cursor on the surface is undefined until we set it */
   ecore_wl_input_cursor_from_name_set(input, "left_ptr");

   /* NB: This whole 'if' below is a major HACK due to wayland's stupidness 
    * of not sending a mouse_up (or any notification at all for that matter) 
    * when a move or resize grab is finished */
   if (input->grab)
     {
        /* NB: This COULD mean a move has finished, or it could mean that 
         * a 'drag' is being done to a different surface */

        if ((input->grab == win) && (win->moving))
          {
             /* NB: 'Fake' a mouse_up for move finished */
             win->moving = EINA_FALSE;
             _ecore_wl_input_mouse_up_send(input, win, input->timestamp);

             input->button = 0;

             if ((input->grab) && (input->grab_button == BTN_LEFT))
               ecore_wl_input_ungrab(input);
          }
        else if ((input->grab == win) && (win->resizing))
          {
             /* NB: 'Fake' a mouse_up for resize finished */
             win->resizing = EINA_FALSE;
             _ecore_wl_input_mouse_up_send(input, win, input->timestamp);

             input->button = 0;

             if ((input->grab) && (input->grab_button == BTN_LEFT))
               ecore_wl_input_ungrab(input);
          }
        /* FIXME: Test d-n-d and potentially add needed case here */
     }
}

static void 
_ecore_wl_input_cb_pointer_leave(void *data, struct wl_pointer *pointer __UNUSED__, unsigned int serial, struct wl_surface *surface)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!surface) return;
   if (!(input = data)) return;

   input->display->serial = serial;

   if (!surface) return;
   if (!(win = wl_surface_get_user_data(surface))) return;

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
_ecore_wl_input_cb_keyboard_enter(void *data, struct wl_keyboard *keyboard __UNUSED__, unsigned int serial, struct wl_surface *surface, struct wl_array *keys __UNUSED__)
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

   if (!(win = wl_surface_get_user_data(surface))) return;

   win->keyboard_device = input;
   input->keyboard_focus = win;

   _ecore_wl_input_focus_in_send(input, win, input->timestamp);
}

static void 
_ecore_wl_input_cb_keyboard_leave(void *data, struct wl_keyboard *keyboard __UNUSED__, unsigned int serial, struct wl_surface *surface)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!surface) return;
   if (!(input = data)) return;

   if (input->repeat.timerfd)
     {
        struct itimerspec ts;
        
        ts.it_interval.tv_sec = 0;
        ts.it_interval.tv_nsec = 0;
        ts.it_value.tv_sec = 0;
        ts.it_value.tv_nsec = 0;

        timerfd_settime(input->repeat.timerfd, 0, &ts, NULL);
     }

   if (!input->timestamp)
     {
        struct timeval tv;

        gettimeofday(&tv, NULL);
        input->timestamp = (tv.tv_sec * 1000 + tv.tv_usec / 1000);
     }

   input->display->serial = serial;

   if (!surface) return;
   if (!(win = wl_surface_get_user_data(surface))) return;

   win->keyboard_device = NULL;
   _ecore_wl_input_focus_out_send(input, win, input->timestamp);

   input->keyboard_focus = NULL;
}

static void 
_ecore_wl_input_cb_touch_down(void *data, struct wl_touch *touch __UNUSED__, unsigned int serial, unsigned int timestamp, struct wl_surface *surface __UNUSED__, int id __UNUSED__, wl_fixed_t x, wl_fixed_t y)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!surface) return;
   if (!(input = data)) return;

   /* FIXME: NB: Not sure yet if input->timestamp should be set here. 
    * This needs to be tested with an actual touch device */
   /* input->timestamp = timestamp; */
   input->display->serial = serial;
   input->button = BTN_LEFT;
   input->sx = wl_fixed_to_int(x);
   input->sy = wl_fixed_to_int(y);
   _ecore_wl_input_cb_pointer_enter(data, NULL, serial, surface, x, y);
   _ecore_wl_input_mouse_down_send(input, input->pointer_focus, timestamp);
}

static void 
_ecore_wl_input_cb_touch_up(void *data, struct wl_touch *touch __UNUSED__, unsigned int serial, unsigned int timestamp, int id __UNUSED__)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   /* FIXME: NB: Not sure yet if input->timestamp should be set here. 
    * This needs to be tested with an actual touch device */
   /* input->timestamp = timestamp; */
   input->button = BTN_LEFT;
   input->display->serial = serial;
   _ecore_wl_input_mouse_up_send(input, input->pointer_focus, timestamp);
   input->button = 0;
}

static void 
_ecore_wl_input_cb_touch_motion(void *data, struct wl_touch *touch __UNUSED__, unsigned int timestamp, int id __UNUSED__, wl_fixed_t x, wl_fixed_t y)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   /* FIXME: NB: Not sure yet if input->timestamp should be set here. 
    * This needs to be tested with an actual touch device */
   /* input->timestamp = timestamp; */
   input->sx = wl_fixed_to_int(x);
   input->sy = wl_fixed_to_int(y);

   _ecore_wl_input_mouse_move_send(input, input->pointer_focus, timestamp);
}

static void 
_ecore_wl_input_cb_touch_frame(void *data __UNUSED__, struct wl_touch *touch __UNUSED__)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
}

static void 
_ecore_wl_input_cb_touch_cancel(void *data __UNUSED__, struct wl_touch *touch __UNUSED__)
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
_ecore_wl_input_mouse_move_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int timestamp)
{
   Ecore_Event_Mouse_Move *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Move)))) return;

   ev->timestamp = timestamp;
   ev->x = input->sx;
   ev->y = input->sy;
   /* ev->root.x = input->sx; */
   /* ev->root.y = input->sy; */
   ev->modifiers = input->modifiers;
   ev->multi.device = 0;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = input->sx;
   ev->multi.y = input->sy;

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
   /* ev->root.x = input->sx; */
   /* ev->root.y = input->sy; */
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
   /* ev->root.x = input->sx; */
   /* ev->root.y = input->sy; */
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
_ecore_wl_input_focus_in_send(Ecore_Wl_Input *input __UNUSED__, Ecore_Wl_Window *win, unsigned int timestamp)
{
   Ecore_Wl_Event_Focus_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Focus_In)))) return;
   ev->timestamp = timestamp;
   if (win) ev->win = win->id;
   ecore_event_add(ECORE_WL_EVENT_FOCUS_IN, ev, NULL, NULL);
}

static void 
_ecore_wl_input_focus_out_send(Ecore_Wl_Input *input __UNUSED__, Ecore_Wl_Window *win, unsigned int timestamp)
{
   Ecore_Wl_Event_Focus_Out *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Focus_Out)))) return;
   ev->timestamp = timestamp;
   if (win) ev->win = win->id;
   ecore_event_add(ECORE_WL_EVENT_FOCUS_OUT, ev, NULL, NULL);
}

static void 
_ecore_wl_input_mouse_down_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int timestamp)
{
   Ecore_Event_Mouse_Button *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Button)))) return;

   if (input->button == BTN_LEFT)
     ev->buttons = 1;
   else if (input->button == BTN_MIDDLE)
     ev->buttons = 2;
   else if (input->button == BTN_RIGHT)
     ev->buttons = 3;
   else
     ev->buttons = input->button;

   ev->timestamp = timestamp;
   ev->x = input->sx;
   ev->y = input->sy;
   /* ev->root.x = input->sx; */
   /* ev->root.y = input->sy; */
   ev->modifiers = input->modifiers;

   /* FIXME: Need to get these from wayland somehow */
   ev->double_click = 0;
   ev->triple_click = 0;

   ev->multi.device = 0;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = input->sx;
   ev->multi.y = input->sy;

   if (win)
     {
        ev->window = win->id;
        ev->event_window = win->id;
     }

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
}

static void 
_ecore_wl_input_mouse_up_send(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int timestamp)
{
   Ecore_Event_Mouse_Button *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Button)))) return;

   if (input->button == BTN_LEFT)
     ev->buttons = 1;
   else if (input->button == BTN_MIDDLE)
     ev->buttons = 2;
   else if (input->button == BTN_RIGHT)
     ev->buttons = 3;
   else
     ev->buttons = input->button;

   ev->timestamp = timestamp;
   ev->x = input->sx;
   ev->y = input->sy;
   /* ev->root.x = input->sx; */
   /* ev->root.y = input->sy; */
   ev->modifiers = input->modifiers;

   /* FIXME: Need to get these from wayland somehow */
   ev->double_click = 0;
   ev->triple_click = 0;

   ev->multi.device = 0;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = input->sx;
   ev->multi.y = input->sy;

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
        ev->z = -value;
     }
   else if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
     {
        ev->direction = 1;
        ev->z = -value;
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

void
_ecore_wl_input_set_selection(Ecore_Wl_Input *input, struct wl_data_source *source)
{
   wl_data_device_set_selection(input->data_device, source, input->display->serial);
}

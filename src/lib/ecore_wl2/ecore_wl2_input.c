#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

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

#include <unistd.h>
#include <sys/mman.h>
#include "ecore_wl2_private.h"

static void
_ecore_wl2_input_mouse_in_send(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window)
{
   Ecore_Event_Mouse_IO *ev;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_IO));
   if (!ev) return;

   ev->x = input->pointer.sx;
   ev->y = input->pointer.sy;
   ev->window = window->id;
   ev->event_window = window->id;
   ev->timestamp = input->timestamp;
   ev->modifiers = input->keyboard.modifiers;

   ecore_event_add(ECORE_EVENT_MOUSE_IN, ev, NULL, NULL);
}

static void
_ecore_wl2_input_mouse_out_send(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window)
{
   Ecore_Event_Mouse_IO *ev;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_IO));
   if (!ev) return;

   ev->x = input->pointer.sx;
   ev->y = input->pointer.sy;
   ev->window = window->id;
   ev->event_window = window->id;
   ev->timestamp = input->timestamp;
   ev->modifiers = input->keyboard.modifiers;

   ecore_event_add(ECORE_EVENT_MOUSE_OUT, ev, NULL, NULL);
}

static void
_ecore_wl2_input_mouse_move_send(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, int device)
{
   Ecore_Event_Mouse_Move *ev;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Move));
   if (!ev) return;

   ev->window = window->id;
   ev->event_window = window->id;
   ev->timestamp = input->timestamp;
   ev->x = input->pointer.sx;
   ev->y = input->pointer.sy;
   ev->root.x = input->pointer.sx;
   ev->root.y = input->pointer.sy;
   ev->modifiers = input->keyboard.modifiers;
   ev->multi.device = device;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = input->pointer.sx;
   ev->multi.y = input->pointer.sy;
   ev->multi.root.x = input->pointer.sx;
   ev->multi.root.y = input->pointer.sy;

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
}

static void
_ecore_wl2_input_mouse_wheel_send(Ecore_Wl2_Input *input, unsigned int axis, int value, unsigned int timestamp)
{
   Ecore_Event_Mouse_Wheel *ev;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Wheel));
   if (!ev) return;

   ev->timestamp = timestamp;
   ev->modifiers = input->keyboard.modifiers;
   ev->x = input->pointer.sx;
   ev->y = input->pointer.sy;

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

   if (input->grab.window)
     {
        ev->window = input->grab.window->id;
        ev->event_window = input->grab.window->id;
     }
   else if (input->focus.pointer)
     {
        ev->window = input->focus.pointer->id;
        ev->event_window = input->focus.pointer->id;
     }

   ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
}

static void
_ecore_wl2_input_mouse_down_send(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, int device, unsigned int button, unsigned int timestamp)
{
   Ecore_Event_Mouse_Button *ev;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
   if (!ev) return;

   if (button == BTN_LEFT)
     ev->buttons = 1;
   else if (button == BTN_MIDDLE)
     ev->buttons = 2;
   else if (button == BTN_RIGHT)
     ev->buttons = 3;
   else
     ev->buttons = button;

   ev->timestamp = timestamp;
   ev->x = input->pointer.sx;
   ev->y = input->pointer.sy;
   ev->root.x = input->pointer.sx;
   ev->root.y = input->pointer.sy;
   ev->modifiers = input->keyboard.modifiers;

   ev->double_click = 0;
   ev->triple_click = 0;

   /* TODO: handle double/triple click */

   ev->multi.device = device;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = input->pointer.sx;
   ev->multi.y = input->pointer.sy;
   ev->multi.root.x = input->pointer.sx;
   ev->multi.root.y = input->pointer.sy;

   ev->window = window->id;
   ev->event_window = window->id;

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
}

static void
_ecore_wl2_input_mouse_up_send(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, int device, unsigned int button, unsigned int timestamp)
{
   Ecore_Event_Mouse_Button *ev;

   ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
   if (!ev) return;

   if (button == BTN_LEFT)
     ev->buttons = 1;
   else if (button == BTN_MIDDLE)
     ev->buttons = 2;
   else if (button == BTN_RIGHT)
     ev->buttons = 3;
   else
     ev->buttons = button;

   ev->timestamp = timestamp;
   ev->x = input->pointer.sx;
   ev->y = input->pointer.sy;
   ev->root.x = input->pointer.sx;
   ev->root.y = input->pointer.sy;
   ev->modifiers = input->keyboard.modifiers;

   ev->double_click = 0;
   ev->triple_click = 0;

   /* TODO: handle double/triple click */

   ev->multi.device = device;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = input->pointer.sx;
   ev->multi.y = input->pointer.sy;
   ev->multi.root.x = input->pointer.sx;
   ev->multi.root.y = input->pointer.sy;

   ev->window = window->id;
   ev->event_window = window->id;

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
}

static void
_ecore_wl2_input_grab(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, unsigned int button)
{
   input->grab.window = window;
   input->grab.button = button;
}

static void
_ecore_wl2_input_ungrab(Ecore_Wl2_Input *input)
{
   if ((input->grab.window) && (input->grab.button))
     {
        /* TODO: send a mouse up here */
     }

   input->grab.window = NULL;
   input->grab.button = 0;
}

static void
_pointer_cb_enter(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int serial, struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Window *window;

   input = data;
   if (!input) return;

   /* trap for a surface that was just destroyed */
   if (!surface) return;

   if (!input->timestamp)
     {
        struct timeval tv;

        gettimeofday(&tv, NULL);
        input->timestamp = (tv.tv_sec * 1000 + tv.tv_usec / 1000);
     }

   input->display->serial = serial;
   input->pointer.enter_serial = serial;
   input->pointer.sx = wl_fixed_to_double(sx);
   input->pointer.sy = wl_fixed_to_double(sy);

   /* find the window which this surface belongs to */
   window = _ecore_wl2_display_window_surface_find(input->display, surface);
   if (!window) return;

   input->focus.pointer = window;

   _ecore_wl2_input_mouse_in_send(input, window);
}

static void
_pointer_cb_leave(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int serial, struct wl_surface *surface)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Window *window;

   input = data;
   if (!input) return;

   input->display->serial = serial;
   input->focus.pointer = NULL;

   /* trap for a surface that was just destroyed */
   if (!surface) return;

   /* find the window which this surface belongs to */
   window = _ecore_wl2_display_window_surface_find(input->display, surface);
   if (!window) return;

   _ecore_wl2_input_mouse_out_send(input, window);
}

static void
_pointer_cb_motion(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int timestamp, wl_fixed_t sx, wl_fixed_t sy)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Window *window;

   input = data;
   if (!input) return;

   /* get currently focused window */
   window = input->focus.pointer;
   if (!window) return;

   input->timestamp = timestamp;
   input->pointer.sx = wl_fixed_to_double(sx);
   input->pointer.sy = wl_fixed_to_double(sy);

   /* NB: Unsure if we need this just yet, so commented out for now */
   /* if ((input->pointer.sx > window->geometry.w) || */
   /*     (input->pointer.sy > window->geometry.h)) */
   /*   return; */

   _ecore_wl2_input_mouse_move_send(input, window, 0);
}

static void
_pointer_cb_button(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int serial EINA_UNUSED, unsigned int timestamp, unsigned int button, unsigned int state)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;

   if (state == WL_POINTER_BUTTON_STATE_PRESSED)
     {
        if ((input->focus.pointer) && (!input->grab.window))
          {
             _ecore_wl2_input_grab(input, input->focus.pointer, button);
             input->grab.timestamp = timestamp;
          }

        if (input->focus.pointer)
          _ecore_wl2_input_mouse_down_send(input, input->focus.pointer,
                                           0, button, timestamp);
     }
   else
     {
        if (input->focus.pointer)
          _ecore_wl2_input_mouse_up_send(input, input->focus.pointer,
                                         0, button, timestamp);

        if ((input->grab.window) && (input->grab.button == button))
          _ecore_wl2_input_ungrab(input);
     }
}

static void
_pointer_cb_axis(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int timestamp, unsigned int axis, wl_fixed_t value)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;

   _ecore_wl2_input_mouse_wheel_send(input, axis, wl_fixed_to_int(value),
                                     timestamp);
}

static const struct wl_pointer_listener _pointer_listener =
{
   _pointer_cb_enter,
   _pointer_cb_leave,
   _pointer_cb_motion,
   _pointer_cb_button,
   _pointer_cb_axis,
};

static void
_keyboard_cb_keymap(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int format, int fd, unsigned int size)
{
   Ecore_Wl2_Input *input;
   char *map = NULL;

   input = data;
   if (!input)
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

   /* free any existing keymap and state */
   if (input->xkb.keymap) xkb_map_unref(input->xkb.keymap);
   if (input->xkb.state) xkb_state_unref(input->xkb.state);

   input->xkb.keymap =
     xkb_map_new_from_string(input->display->xkb_context, map,
                             XKB_KEYMAP_FORMAT_TEXT_V1, 0);

   munmap(map, size);
   close(fd);

   if (!input->xkb.keymap)
     {
        ERR("Failed to compile keymap");
        return;
     }

   input->xkb.state = xkb_state_new(input->xkb.keymap);
   if (!input->xkb.state)
     {
        ERR("Failed to create keymap state: %m");
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

static void
_keyboard_cb_enter(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int serial, struct wl_surface *surface, struct wl_array *keys EINA_UNUSED)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Window *window;

   input = data;
   if (!input) return;

   input->display->serial = serial;

   if (!input->timestamp)
     {
        struct timeval tv;

        gettimeofday(&tv, NULL);
        input->timestamp = (tv.tv_sec * 1000 + tv.tv_usec / 1000);
     }

   /* find the window which this surface belongs to */
   window = _ecore_wl2_display_window_surface_find(input->display, surface);
   if (!window) return;

   input->focus.keyboard = window;

   /* TODO: Send keyboard enter event (focus in) */
}

static void
_keyboard_cb_leave(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int serial, struct wl_surface *surface)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Window *window;

   input = data;
   if (!input) return;

   input->display->serial = serial;

   input->repeat.sym = 0;
   input->repeat.key = 0;
   input->repeat.time = 0;
   if (input->repeat.timer) ecore_timer_del(input->repeat.timer);
   input->repeat.timer = NULL;

   /* find the window which this surface belongs to */
   window = _ecore_wl2_display_window_surface_find(input->display, surface);
   if (!window) return;

   /* TODO: Send keyboard leave event (focus out) */

   input->focus.keyboard = NULL;
}

static Eina_Bool
_keyboard_cb_repeat(void *data)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Window *window;

   input = data;
   if (!input) return ECORE_CALLBACK_RENEW;

   window = input->focus.keyboard;
   if (!window) goto out;

   /* TODO: Send ecore key event for preseed */

   return ECORE_CALLBACK_RENEW;

out:
   input->repeat.sym = 0;
   input->repeat.key = 0;
   input->repeat.time = 0;
   return ECORE_CALLBACK_CANCEL;
}

static void
_keyboard_cb_key(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int serial, unsigned int timestamp, unsigned int keycode, unsigned int state)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Window *window;
   unsigned int code, nsyms;
   const xkb_keysym_t *syms;
   xkb_keysym_t sym = XKB_KEY_NoSymbol;

   input = data;
   if (!input) return;

   /* try to get the window which has keyboard focus */
   window = input->focus.keyboard;
   if (!window) return;

   input->display->serial = serial;

   /* xkb rules reflect X broken keycodes, so offset by 8 */
   code = keycode + 8;

   nsyms = xkb_state_key_get_syms(input->xkb.state, code, &syms);
   if (nsyms == 1) sym = syms[0];

   /* TODO: Send ecore key event */

   if ((state == WL_KEYBOARD_KEY_STATE_RELEASED) &&
       (keycode == input->repeat.key))
     {
        input->repeat.sym = 0;
        input->repeat.key = 0;
        input->repeat.time = 0;
        if (input->repeat.timer) ecore_timer_del(input->repeat.timer);
        input->repeat.timer = NULL;
     }
   else if ((state == WL_KEYBOARD_KEY_STATE_PRESSED) &&
            (xkb_keymap_key_repeats(input->xkb.keymap, code)))
     {
        /* don't setup key repeat timer if not enabled */
        if (!input->repeat.enabled) return;

        input->repeat.sym = sym;
        input->repeat.key = keycode;
        input->repeat.time = timestamp;

        if (!input->repeat.timer)
          {
             input->repeat.timer =
               ecore_timer_add(input->repeat.rate, _keyboard_cb_repeat, input);
          }

        ecore_timer_delay(input->repeat.timer, input->repeat.delay);
     }
}

static void
_keyboard_cb_modifiers(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int serial EINA_UNUSED, unsigned int depressed, unsigned int latched, unsigned int locked, unsigned int group)
{
   Ecore_Wl2_Input *input;
   xkb_mod_mask_t mask;

   input = data;
   if (!input) return;

   /* skip PC style modifiers if we have no keymap */
   if (!input->xkb.keymap) return;

   xkb_state_update_mask(input->xkb.state,
                         depressed, latched, locked, 0, 0, group);

   mask = xkb_state_serialize_mods(input->xkb.state,
                                   XKB_STATE_MODS_DEPRESSED | XKB_STATE_MODS_LATCHED);

   /* reset modifiers to default */
   input->keyboard.modifiers = 0;

   if (mask & input->xkb.control_mask)
     input->keyboard.modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if (mask & input->xkb.alt_mask)
     input->keyboard.modifiers |= ECORE_EVENT_MODIFIER_ALT;
   if (mask & input->xkb.shift_mask)
     input->keyboard.modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if (mask & input->xkb.win_mask)
     input->keyboard.modifiers |= ECORE_EVENT_MODIFIER_WIN;
   if (mask & input->xkb.scroll_mask)
     input->keyboard.modifiers |= ECORE_EVENT_LOCK_SCROLL;
   if (mask & input->xkb.num_mask)
     input->keyboard.modifiers |= ECORE_EVENT_LOCK_NUM;
   if (mask & input->xkb.caps_mask)
     input->keyboard.modifiers |= ECORE_EVENT_LOCK_CAPS;
   if (mask & input->xkb.altgr_mask)
     input->keyboard.modifiers |= ECORE_EVENT_MODIFIER_ALTGR;
}

static void
_keyboard_cb_repeat_setup(void *data, struct wl_keyboard *keyboard EINA_UNUSED, int32_t rate, int32_t delay)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;

   if (rate == 0)
     {
        input->repeat.enabled = EINA_FALSE;
        return;
     }

   input->repeat.enabled = EINA_TRUE;
   input->repeat.rate = (rate / 1000);
   input->repeat.delay = (delay / 100);
}

static const struct wl_keyboard_listener _keyboard_listener =
{
   _keyboard_cb_keymap,
   _keyboard_cb_enter,
   _keyboard_cb_leave,
   _keyboard_cb_key,
   _keyboard_cb_modifiers,
   _keyboard_cb_repeat_setup
};

static void
_touch_cb_down(void *data, struct wl_touch *touch EINA_UNUSED, unsigned int serial, unsigned int timestamp, struct wl_surface *surface, int id, wl_fixed_t x, wl_fixed_t y)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Window *window;

   input = data;
   if (!input) return;

   /* find the window which this surface belongs to */
   window = _ecore_wl2_display_window_surface_find(input->display, surface);
   if (!window) return;

   input->focus.touch = window;

   /* TODO: Finish sending ecore events */
}

static void
_touch_cb_up(void *data, struct wl_touch *touch EINA_UNUSED, unsigned int serial, unsigned int timestamp, int id)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;
   if (!input->focus.touch) return;

   /* TODO: Send ecore mouse up event and do input ungrab */
}

static void
_touch_cb_motion(void *data, struct wl_touch *touch EINA_UNUSED, unsigned int timestamp, int id, wl_fixed_t x, wl_fixed_t y)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;
   if (!input->focus.touch) return;

   /* TODO: Send ecore mouse move event */
}

static void
_touch_cb_frame(void *data EINA_UNUSED, struct wl_touch *touch EINA_UNUSED)
{

}

static void
_touch_cb_cancel(void *data EINA_UNUSED, struct wl_touch *tough EINA_UNUSED)
{

}

static const struct wl_touch_listener _touch_listener =
{
   _touch_cb_down,
   _touch_cb_up,
   _touch_cb_motion,
   _touch_cb_frame,
   _touch_cb_cancel
};

static void
_data_cb_offer(void *data, struct wl_data_device *data_device, struct wl_data_offer *offer)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;
}

static void
_data_cb_enter(void *data, struct wl_data_device *data_device, unsigned int timestmap, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y, struct wl_data_offer *offer)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;
}

static void
_data_cb_leave(void *data, struct wl_data_device *data_device)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;
}

static void
_data_cb_motion(void *data, struct wl_data_device *data_device, unsigned int timestamp, wl_fixed_t x, wl_fixed_t y)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;
}

static void
_data_cb_drop(void *data, struct wl_data_device *data_device)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;
}

static void
_data_cb_selection(void *data, struct wl_data_device *data_device, struct wl_data_offer *offer)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;
}

static const struct wl_data_device_listener _data_listener =
{
   _data_cb_offer,
   _data_cb_enter,
   _data_cb_leave,
   _data_cb_motion,
   _data_cb_drop,
   _data_cb_selection
};

static void
_seat_cb_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;

   if ((caps & WL_SEAT_CAPABILITY_POINTER) && (!input->wl.pointer))
     {
        input->wl.pointer = wl_seat_get_pointer(seat);
        wl_pointer_set_user_data(input->wl.pointer, input);
        wl_pointer_add_listener(input->wl.pointer, &_pointer_listener, input);

        if (!input->cursor.surface)
          {
             input->cursor.surface =
               wl_compositor_create_surface(input->display->wl.compositor);
          }
     }
   else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && (input->wl.pointer))
     {
        if (input->cursor.surface) wl_surface_destroy(input->cursor.surface);
        input->cursor.surface = NULL;

        wl_pointer_destroy(input->wl.pointer);
        input->wl.pointer = NULL;
     }
   else if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && (!input->wl.keyboard))
     {
        input->wl.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_set_user_data(input->wl.keyboard, input);
        wl_keyboard_add_listener(input->wl.keyboard, &_keyboard_listener, input);
     }
   else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && (input->wl.keyboard))
     {
        wl_keyboard_destroy(input->wl.keyboard);
        input->wl.keyboard = NULL;
     }
   else if ((caps & WL_SEAT_CAPABILITY_TOUCH) && (!input->wl.touch))
     {
        input->wl.touch = wl_seat_get_touch(seat);
        wl_touch_set_user_data(input->wl.touch, input);
        wl_touch_add_listener(input->wl.touch, &_touch_listener, input);
     }
   else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && (input->wl.touch))
     {
        wl_touch_destroy(input->wl.touch);
        input->wl.touch = NULL;
     }
}

static const struct wl_seat_listener _seat_listener =
{
   _seat_cb_capabilities,
   NULL
};

static void
_ecore_wl2_input_cursor_setup(Ecore_Wl2_Input *input)
{
   char *tmp;

   input->cursor.size = 32;
   tmp = getenv("ECORE_WL_CURSOR_SIZE");
   if (tmp) input->cursor.size = atoi(tmp);

   tmp = getenv("ECORE_WL_CURSOR_THEME_NAME");
   eina_stringshare_replace(&input->cursor.theme_name, tmp);

   if (!input->cursor.name)
     input->cursor.name = eina_stringshare_add("left_ptr");

   if (input->display->wl.shm)
     {
        input->cursor.theme =
          wl_cursor_theme_load(input->cursor.theme_name, input->cursor.size,
                               input->display->wl.shm);
     }
}

void
_ecore_wl2_input_add(Ecore_Wl2_Display *display, unsigned int id)
{
   Ecore_Wl2_Input *input;

   input = calloc(1, sizeof(Ecore_Wl2_Input));
   if (!input) return;

   input->display = display;

   input->repeat.rate = 0.025;
   input->repeat.delay = 0.4;
   input->repeat.enabled = EINA_TRUE;

   wl_array_init(&input->data.types);

   /* setup cursor size and theme */
   _ecore_wl2_input_cursor_setup(input);

   input->wl.seat =
     wl_registry_bind(wl_display_get_registry(display->wl.display),
                      id, &wl_seat_interface, 1);

   display->inputs =
     eina_inlist_append(display->inputs, EINA_INLIST_GET(input));

   wl_seat_add_listener(input->wl.seat, &_seat_listener, input);
   wl_seat_set_user_data(input->wl.seat, input);

   if (!display->wl.data_device_manager) return;

   input->data.device =
     wl_data_device_manager_get_data_device(display->wl.data_device_manager,
                                            input->wl.seat);
   wl_data_device_add_listener(input->data.device, &_data_listener, input);
}

void
_ecore_wl2_input_del(Ecore_Wl2_Input *input)
{
   Ecore_Wl2_Display *display;

   if (!input) return;

   display = input->display;

   if (input->repeat.timer) ecore_timer_del(input->repeat.timer);

   if (input->cursor.theme) wl_cursor_theme_destroy(input->cursor.theme);
   if (input->cursor.surface) wl_surface_destroy(input->cursor.surface);
   if (input->cursor.name) eina_stringshare_del(input->cursor.name);
   if (input->cursor.theme_name)
     eina_stringshare_del(input->cursor.theme_name);

   if (input->data.types.data)
     {
        char **t;

        wl_array_for_each(t, &input->data.types)
          free(*t);

        wl_array_release(&input->data.types);
     }

   if (input->data.source) wl_data_source_destroy(input->data.source);
   /* TODO: cleanup dnd */
   /* TODO: cleanup selection */
   if (input->data.device) wl_data_device_destroy(input->data.device);

   if (input->xkb.state) xkb_state_unref(input->xkb.state);
   if (input->xkb.keymap) xkb_map_unref(input->xkb.keymap);

   if (input->wl.seat) wl_seat_destroy(input->wl.seat);

   display->inputs =
     eina_inlist_remove(display->inputs, EINA_INLIST_GET(input));

   free(input);
}

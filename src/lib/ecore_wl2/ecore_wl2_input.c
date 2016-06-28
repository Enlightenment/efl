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

typedef struct _Ecore_Wl2_Mouse_Down_Info
{
   EINA_INLIST;
   int device, sx, sy;
   int last_win;
   int last_last_win;
   int last_event_win;
   int last_last_event_win;
   unsigned int last_time;
   unsigned int last_last_time;
   Eina_Bool double_click : 1;
   Eina_Bool triple_click : 1;
} Ecore_Wl2_Mouse_Down_Info;

static Eina_Inlist *_ecore_wl2_mouse_down_info_list = NULL;

static void _keyboard_cb_key(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int serial, unsigned int timestamp, unsigned int keycode, unsigned int state);
static void _pointer_cb_frame(void *data, struct wl_callback *callback, unsigned int timestamp EINA_UNUSED);

static Ecore_Wl2_Mouse_Down_Info *
_ecore_wl2_input_mouse_down_info_get(int device)
{
   Eina_Inlist *l = NULL;
   Ecore_Wl2_Mouse_Down_Info *info = NULL;

   l = _ecore_wl2_mouse_down_info_list;
   EINA_INLIST_FOREACH(l, info)
     if (info->device == device) return info;

   info = calloc(1, sizeof(Ecore_Wl2_Mouse_Down_Info));
   if (!info) return NULL;

   info->device = device;
   l = eina_inlist_append(l, (Eina_Inlist *)info);
   _ecore_wl2_mouse_down_info_list = l;

   return info;
}

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
   Ecore_Wl2_Mouse_Down_Info *info;

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

   info = _ecore_wl2_input_mouse_down_info_get(device);
   if (info)
     {
        info->sx = input->pointer.sx;
        info->sy = input->pointer.sy;
     }

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
   Ecore_Wl2_Mouse_Down_Info *info;

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

   info = _ecore_wl2_input_mouse_down_info_get(device);
   if (info)
     {
        info->sx = input->pointer.sx;
        info->sy = input->pointer.sy;
        if (info->triple_click)
          {
             info->last_win = 0;
             info->last_last_win = 0;
             info->last_event_win = 0;
             info->last_last_event_win = 0;
             info->last_time = 0;
             info->last_last_time = 0;
          }

        if (((int)(timestamp - info->last_time) <= (int)(1000 * 0.25)) &&
            ((window) && (window->id == info->last_win) &&
                (window->id == info->last_event_win)))
          {
             ev->double_click = 1;
             info->double_click = EINA_TRUE;
          }
        else
          {
             info->double_click = EINA_FALSE;
             info->triple_click = EINA_FALSE;
          }

        if (((int)(timestamp - info->last_last_time) <=
             (int)(2 * 1000 * 0.25)) &&
            ((window) && (window->id == info->last_win) &&
                (window->id == info->last_last_win) &&
                (window->id == info->last_event_win) &&
                (window->id == info->last_last_event_win)))
          {
             ev->triple_click = 1;
             info->triple_click = EINA_TRUE;
          }
        else
          info->triple_click = EINA_FALSE;
     }

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

   if ((info) && (!info->triple_click))
     {
        info->last_last_win = info->last_win;
        info->last_win = ev->window;
        info->last_last_event_win = info->last_event_win;
        info->last_event_win = ev->window;
        info->last_last_time = info->last_time;
        info->last_time = timestamp;
     }
}

static void
_ecore_wl2_input_mouse_up_send(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, int device, unsigned int button, unsigned int timestamp)
{
   Ecore_Event_Mouse_Button *ev;
   Ecore_Wl2_Mouse_Down_Info *info;

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

   info = _ecore_wl2_input_mouse_down_info_get(device);
   if (info)
     {
        ev->double_click = info->double_click;
        ev->triple_click = info->triple_click;
        ev->x = info->sx;
        ev->y = info->sy;
        ev->multi.x = info->sx;
        ev->multi.y = info->sy;
     }
   else
     {
        ev->multi.x = input->pointer.sx;
        ev->multi.y = input->pointer.sy;
     }

   ev->multi.device = device;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.root.x = input->pointer.sx;
   ev->multi.root.y = input->pointer.sy;

   ev->window = window->id;
   ev->event_window = window->id;

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
}

static void
_ecore_wl2_input_focus_in_send(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Event_Focus_In *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Focus_In));
   if (!ev) return;

   ev->timestamp = input->timestamp;
   ev->window = window->id;
   ecore_event_add(ECORE_WL2_EVENT_FOCUS_IN, ev, NULL, NULL);
}

static void
_ecore_wl2_input_focus_out_send(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Event_Focus_Out *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Focus_Out));
   if (!ev) return;

   ev->timestamp = input->timestamp;
   ev->window = window->id;
   ecore_event_add(ECORE_WL2_EVENT_FOCUS_OUT, ev, NULL, NULL);
}

static int
_ecore_wl2_input_key_translate(xkb_keysym_t keysym, unsigned int modifiers, char *buffer, int bytes)
{
   if (!keysym) return 0;

   /* check for possible control codes */
   if (modifiers & ECORE_EVENT_MODIFIER_CTRL)
     {
        Eina_Bool valid_control_code = EINA_TRUE;
        unsigned long hbytes = 0;
        unsigned char c;

        hbytes = (keysym >> 8);

        if (keysym == XKB_KEY_KP_Space)
          c = (XKB_KEY_space & 0x7F);
        else if (hbytes == 0xFF)
          c = (keysym & 0x7F);
        else
          c = (keysym & 0xFF);

        /* We are building here a control code
           for more details, read:
           https://en.wikipedia.org/wiki/C0_and_C1_control_codes#C0_.28ASCII_and_derivatives.29
         */

        if (((c >= '@') && (c <= '_')) || /* those are the one defined in C0 with capital letters */
             ((c >= 'a') && (c <= 'z')) ||  /* the lowercase symbols (not part of the standard, but usefull */
              c == ' ')
          c &= 0x1F;
        else if (c == '\x7f')
          c = '\177';
        /* following codes are alternatives, they are longer here, i dont want to change them */
        else if (c == '2')
          c = '\000'; /* 0 code */
        else if ((c >= '3') && (c <= '7'))
          c -= ('3' - '\033'); /* from escape to unitseperator code*/
        else if (c == '8')
          c = '\177'; /* delete code */
        else if (c == '/')
          c = '_' & 0x1F; /* unit seperator code */
        else
          valid_control_code = EINA_FALSE;

        if (valid_control_code)
          buffer[0] = c;
        else
          return 0;
     }
   else
     {
        /* if its not a control code, try to produce a usefull output */
        if (!xkb_keysym_to_utf8(keysym, buffer, bytes))
          return 0;
     }

   return 1;
}

static void
_ecore_wl2_input_key_send(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, xkb_keysym_t sym, unsigned int code, unsigned int state, unsigned int timestamp)
{
   Ecore_Event_Key *ev;
   char key[256], keyname[256], compose[256];

   memset(key, 0, sizeof(key));
   memset(keyname, 0, sizeof(keyname));

   /*try to get a name or utf char of the given symbol */
   xkb_keysym_to_utf8(sym, keyname, sizeof(keyname));
   xkb_keysym_get_name(sym, key, sizeof(key));

   if (keyname[0] == '\0' && key[0] != '\0')
     {
        memcpy(keyname, key, sizeof(key));
     }
   else if (keyname[0] != '\0' && key[0] == '\0')
     {
        memcpy(key, keyname, sizeof(keyname));
     }
   else if (keyname[0] == '\0' && key[0] == '\0')
     {
        snprintf(keyname, sizeof(keyname), "Keycode-%u", code);
        memcpy(key, keyname, sizeof(keyname));
     }

   memset(compose, 0, sizeof(compose));
   _ecore_wl2_input_key_translate(sym, input->keyboard.modifiers,
                                  compose, sizeof(compose));

   ev = calloc(1, sizeof(Ecore_Event_Key) + strlen(key) + strlen(keyname) +
               ((compose[0] != '\0') ? strlen(compose) : 0) + 3);
   if (!ev) return;

   ev->keyname = (char *)(ev + 1);
   ev->key = ev->keyname + strlen(keyname) + 1;
   ev->compose = strlen(compose) ? ev->key + strlen(key) + 1 : NULL;
   ev->string = ev->compose;

   strcpy((char *)ev->keyname, keyname);
   strcpy((char *)ev->key, key);
   if (strlen(compose)) strcpy((char *)ev->compose, compose);

   ev->window = window->id;
   ev->event_window = window->id;
   ev->timestamp = timestamp;
   ev->modifiers = input->keyboard.modifiers;
   ev->keycode = code;

   if (state)
     ecore_event_add(ECORE_EVENT_KEY_DOWN, ev, NULL, NULL);
   else
     ecore_event_add(ECORE_EVENT_KEY_UP, ev, NULL, NULL);
}

void
_ecore_wl2_input_grab(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, unsigned int button)
{
   input->grab.window = window;
   input->grab.button = button;
}

void
_ecore_wl2_input_ungrab(Ecore_Wl2_Input *input)
{
   if ((input->grab.window) && (input->grab.button) && (input->grab.count))
     _ecore_wl2_input_mouse_up_send(input, input->grab.window, 0,
                                    input->grab.button, input->grab.timestamp);

   input->grab.window = NULL;
   input->grab.button = 0;
   input->grab.count = 0;
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

   window->input = input;
   input->focus.prev_pointer = NULL;
   input->focus.pointer = window;

   _ecore_wl2_input_mouse_in_send(input, window);

   if ((window->moving) && (input->grab.window == window))
     {
        _ecore_wl2_input_mouse_up_send(input, window, 0, input->grab.button,
                                       input->grab.timestamp);
        window->moving = EINA_FALSE;
     }
}

static void
_pointer_cb_leave(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int serial, struct wl_surface *surface)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Window *window;

   input = data;
   if (!input) return;

   input->display->serial = serial;
   input->focus.prev_pointer = input->focus.pointer;
   input->focus.pointer = NULL;

   /* trap for a surface that was just destroyed */
   if (!surface) return;

   /* find the window which this surface belongs to */
   window = _ecore_wl2_display_window_surface_find(input->display, surface);
   if (!window) return;

   /* NB: Don't send a mouse out if we grabbed this window for moving */
   if ((window->moving) && (input->grab.window == window)) return;

   _ecore_wl2_input_mouse_out_send(input, window);
}

static void
_pointer_cb_motion(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int timestamp, wl_fixed_t sx, wl_fixed_t sy)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Window *window;

   input = data;
   if (!input) return;

   input->timestamp = timestamp;
   input->pointer.sx = wl_fixed_to_double(sx);
   input->pointer.sy = wl_fixed_to_double(sy);

   /* get currently focused window */
   window = input->focus.pointer;
   if (!window) return;

   /* NB: Unsure if we need this just yet, so commented out for now */
   /* if ((input->pointer.sx > window->geometry.w) || */
   /*     (input->pointer.sy > window->geometry.h)) */
   /*   return; */

   _ecore_wl2_input_mouse_move_send(input, window, 0);
}

static void
_pointer_cb_button(void *data, struct wl_pointer *pointer EINA_UNUSED, unsigned int serial, unsigned int timestamp, unsigned int button, unsigned int state)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;

   input->display->serial = serial;

   if (state == WL_POINTER_BUTTON_STATE_PRESSED)
     {
        if ((input->focus.pointer) &&
            (!input->grab.window) && (!input->grab.count))
          {
             _ecore_wl2_input_grab(input, input->focus.pointer, button);
             input->grab.timestamp = timestamp;
          }

        if (input->focus.pointer)
          _ecore_wl2_input_mouse_down_send(input, input->focus.pointer,
                                           0, button, timestamp);

        input->grab.count++;
     }
   else
     {
        if (input->focus.pointer)
          _ecore_wl2_input_mouse_up_send(input, input->focus.pointer,
                                         0, button, timestamp);

        if (input->grab.count) input->grab.count--;
        if ((input->grab.window) && (input->grab.button == button) &&
            (!input->grab.count))
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
   NULL, /* frame */
   NULL, /* axis_source */
   NULL, /* axis_stop */
   NULL, /* axis_discrete */
};

static const struct wl_callback_listener _pointer_surface_listener =
{
   _pointer_cb_frame
};

static void
_pointer_cb_frame(void *data, struct wl_callback *callback, unsigned int timestamp EINA_UNUSED)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;

   if (callback)
     {
        if (callback != input->cursor.frame_cb) return;
        wl_callback_destroy(callback);
        input->cursor.frame_cb = NULL;
     }

   if (!input->cursor.name)
     {
        _ecore_wl2_input_cursor_set(input, NULL);
        return;
     }

   if ((input->cursor.wl_cursor->image_count > 1) &&
       (!input->cursor.frame_cb))
     {
        input->cursor.frame_cb = wl_surface_frame(input->cursor.surface);
        wl_callback_add_listener(input->cursor.frame_cb,
                                 &_pointer_surface_listener, input);
     }
}

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
        ERR("Failed to create keymap state");
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
   window->input = input;

   _ecore_wl2_input_focus_in_send(input, window);
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

   _ecore_wl2_input_focus_out_send(input, window);

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

   _ecore_wl2_input_key_send(input, input->focus.keyboard, input->repeat.sym, input->repeat.key + 8, WL_KEYBOARD_KEY_STATE_PRESSED,  input->repeat.time);

   if (!input->repeat.repeating)
     {
        ecore_timer_interval_set(input->repeat.timer, input->repeat.rate);
        input->repeat.repeating = EINA_TRUE;
     }
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

   _ecore_wl2_input_key_send(input, window, sym, code, state, timestamp);

   if (!xkb_keymap_key_repeats(input->xkb.keymap, code)) return;

   if ((state == WL_KEYBOARD_KEY_STATE_RELEASED) &&
       (keycode == input->repeat.key))
     {
        input->repeat.sym = 0;
        input->repeat.key = 0;
        input->repeat.time = 0;
        if (input->repeat.timer) ecore_timer_del(input->repeat.timer);
        input->repeat.timer = NULL;
     }
   else if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
     {
        /* don't setup key repeat timer if not enabled */
        if (!input->repeat.enabled) return;

        input->repeat.sym = sym;
        input->repeat.key = keycode;
        input->repeat.time = timestamp;

        /* Delete this timer if there is still one */
        if (input->repeat.timer) ecore_timer_del(input->repeat.timer);
        input->repeat.timer = NULL;

        if (!input->repeat.timer)
          {
             input->repeat.repeating = EINA_FALSE;
             input->repeat.timer =
               ecore_timer_add(input->repeat.delay, _keyboard_cb_repeat, input);
          }
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

   mask =
     xkb_state_serialize_mods(input->xkb.state,
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
   input->repeat.rate = (1.0 / rate);
   input->repeat.delay = (delay / 1000.0);
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

   _pointer_cb_enter(data, NULL, serial, surface, x, y);

   if ((!input->grab.window) && (input->focus.touch))
     {
        _ecore_wl2_input_grab(input, input->focus.touch, BTN_LEFT);
        input->grab.timestamp = timestamp;
     }

   _ecore_wl2_input_mouse_down_send(input, input->focus.touch, id,
                                    BTN_LEFT, timestamp);
}

static void
_touch_cb_up(void *data, struct wl_touch *touch EINA_UNUSED, unsigned int serial, unsigned int timestamp, int id)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;
   if (!input->focus.touch) return;

   input->timestamp = timestamp;
   input->display->serial = serial;

   _ecore_wl2_input_mouse_up_send(input, input->focus.touch, id,
                                  BTN_LEFT, timestamp);

   if (input->grab.count) input->grab.count--;
   if ((input->grab.window) && (input->grab.button == BTN_LEFT) &&
       (!input->grab.count))
     _ecore_wl2_input_ungrab(input);
}

static void
_touch_cb_motion(void *data, struct wl_touch *touch EINA_UNUSED, unsigned int timestamp, int id, wl_fixed_t x, wl_fixed_t y)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;
   if (!input->focus.touch) return;

   input->timestamp = timestamp;
   input->pointer.sx = wl_fixed_to_int(x);
   input->pointer.sy = wl_fixed_to_int(y);

   _ecore_wl2_input_mouse_move_send(input, input->focus.touch, id);
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
_data_cb_offer(void *data, struct wl_data_device *data_device EINA_UNUSED, struct wl_data_offer *offer)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;

   _ecore_wl2_dnd_add(input, offer);
}

static void
_data_cb_enter(void *data, struct wl_data_device *data_device EINA_UNUSED, uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y, struct wl_data_offer *offer)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;

   _ecore_wl2_dnd_enter(input, offer, surface,
                        wl_fixed_to_int(x), wl_fixed_to_int(y), serial);
}

static void
_data_cb_leave(void *data, struct wl_data_device *data_device EINA_UNUSED)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;

   _ecore_wl2_dnd_leave(input);
}

static void
_data_cb_motion(void *data, struct wl_data_device *data_device EINA_UNUSED, uint32_t serial, wl_fixed_t x, wl_fixed_t y)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;

   _ecore_wl2_dnd_motion(input, wl_fixed_to_int(x),
                         wl_fixed_to_int(y), serial);
}

static void
_data_cb_drop(void *data, struct wl_data_device *data_device EINA_UNUSED)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;

   _ecore_wl2_dnd_drop(input);
}

static void
_data_cb_selection(void *data, struct wl_data_device *data_device EINA_UNUSED, struct wl_data_offer *offer)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;

   _ecore_wl2_dnd_selection(input, offer);
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

#ifdef WL_POINTER_RELEASE_SINCE_VERSION
        if (input->seat_version >= WL_POINTER_RELEASE_SINCE_VERSION)
          wl_pointer_release(input->wl.pointer);
        else
#endif
          wl_pointer_destroy(input->wl.pointer);
        input->wl.pointer = NULL;
     }

   if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && (!input->wl.keyboard))
     {
        input->wl.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_set_user_data(input->wl.keyboard, input);
        wl_keyboard_add_listener(input->wl.keyboard, &_keyboard_listener, input);
     }
   else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && (input->wl.keyboard))
     {
#ifdef WL_KEYBOARD_RELEASE_SINCE_VERSION
        if (input->seat_version >= WL_KEYBOARD_RELEASE_SINCE_VERSION)
          wl_keyboard_release(input->wl.keyboard);
        else
#endif
          wl_keyboard_destroy(input->wl.keyboard);
        input->wl.keyboard = NULL;
     }

   if ((caps & WL_SEAT_CAPABILITY_TOUCH) && (!input->wl.touch))
     {
        input->wl.touch = wl_seat_get_touch(seat);
        wl_touch_set_user_data(input->wl.touch, input);
        wl_touch_add_listener(input->wl.touch, &_touch_listener, input);
     }
   else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && (input->wl.touch))
     {
#ifdef WL_TOUCH_RELEASE_SINCE_VERSION
        if (input->seat_version >= WL_TOUCH_RELEASE_SINCE_VERSION)
          wl_touch_release(input->wl.touch);
        else
#endif
          wl_touch_destroy(input->wl.touch);
        input->wl.touch = NULL;
     }
}

static void
_seat_cb_name(void *data EINA_UNUSED, struct wl_seat *seat EINA_UNUSED, const char *name EINA_UNUSED)
{
   /* NB: No-Op as we don't care about seat name right now.
    *
    * This will likely change as we hash out remaining multi-seat issues */
}

static const struct wl_seat_listener _seat_listener =
{
   _seat_cb_capabilities,
   _seat_cb_name,
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

static Eina_Bool
_ecore_wl2_input_cursor_update(void *data)
{
   Ecore_Wl2_Input *input;
   struct wl_cursor_image *image;
   struct wl_buffer *buffer;
   unsigned int delay = 0;

   input = data;
   if (!input) return EINA_FALSE;

   image = input->cursor.wl_cursor->images[input->cursor.index];
   if (!image) return EINA_FALSE;

   buffer = wl_cursor_image_get_buffer(image);
   if (buffer)
     {
        if (input->wl.pointer)
          wl_pointer_set_cursor(input->wl.pointer, input->pointer.enter_serial,
                                input->cursor.surface,
                                image->hotspot_x, image->hotspot_y);

        wl_surface_attach(input->cursor.surface, buffer, 0, 0);
#ifdef WL_SURFACE_DAMAGE_BUFFER_SINCE_VERSION
        if (input->display->wl.compositor_version >= WL_SURFACE_DAMAGE_BUFFER_SINCE_VERSION)
           wl_surface_damage_buffer(input->cursor.surface,
                                    0, 0, image->width, image->height);
        else
#endif
           wl_surface_damage(input->cursor.surface,
                             0, 0, image->width, image->height);
        wl_surface_commit(input->cursor.surface);

        if ((input->cursor.wl_cursor->image_count > 1) &&
            (!input->cursor.frame_cb))
          _pointer_cb_frame(input, NULL, 0);
     }

   if (input->cursor.wl_cursor->image_count <= 1)
     return ECORE_CALLBACK_CANCEL;

   delay = image->delay;
   input->cursor.index =
     (input->cursor.index + 1) % input->cursor.wl_cursor->image_count;

   if (!input->cursor.timer)
     {
        input->cursor.timer =
          ecore_timer_loop_add(delay / 1000.0,
                               _ecore_wl2_input_cursor_update, input);
     }
   else
     ecore_timer_interval_set(input->cursor.timer, delay / 1000.0);

   return ECORE_CALLBACK_RENEW;
}

void
_ecore_wl2_input_add(Ecore_Wl2_Display *display, unsigned int id, unsigned int version)
{
   Ecore_Wl2_Input *input;

   input = calloc(1, sizeof(Ecore_Wl2_Input));
   if (!input) return;

   input->display = display;
   input->seat_version = version;
   input->repeat.rate = 0.025;
   input->repeat.delay = 0.4;
   input->repeat.enabled = EINA_TRUE;

   wl_array_init(&input->data.types);

   /* setup cursor size and theme */
   _ecore_wl2_input_cursor_setup(input);

   input->wl.seat =
     wl_registry_bind(display->wl.registry, id, &wl_seat_interface, 4);

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
   Eina_Inlist *l = NULL;
   Ecore_Wl2_Mouse_Down_Info *info = NULL;

   if (!input) return;

   display = input->display;

   l = _ecore_wl2_mouse_down_info_list;
   while (l)
     {
        info = EINA_INLIST_CONTAINER_GET(l, Ecore_Wl2_Mouse_Down_Info);
        l = eina_inlist_remove(l, l);
        free(info);
     }
   _ecore_wl2_mouse_down_info_list = NULL;

   if (input->repeat.timer) ecore_timer_del(input->repeat.timer);

   _ecore_wl2_input_cursor_update_stop(input);

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
   if (input->drag.source) _ecore_wl2_dnd_del(input->drag.source);
   if (input->selection.source) _ecore_wl2_dnd_del(input->selection.source);
   if (input->data.device) wl_data_device_destroy(input->data.device);

   if (input->xkb.state) xkb_state_unref(input->xkb.state);
   if (input->xkb.keymap) xkb_map_unref(input->xkb.keymap);

   if (input->wl.seat) wl_seat_destroy(input->wl.seat);

   display->inputs =
     eina_inlist_remove(display->inputs, EINA_INLIST_GET(input));

   free(input);
}

void
_ecore_wl2_input_cursor_set(Ecore_Wl2_Input *input, const char *cursor)
{
   struct wl_cursor *wl_cursor;

   _ecore_wl2_input_cursor_update_stop(input);

   eina_stringshare_replace(&input->cursor.name, cursor);
   if (!cursor) eina_stringshare_replace(&input->cursor.name, "left_ptr");

   wl_cursor =
     wl_cursor_theme_get_cursor(input->cursor.theme, input->cursor.name);
   if (!wl_cursor)
     {
        wl_cursor =
          wl_cursor_theme_get_cursor(input->cursor.theme, "left_ptr");
     }

   if (!wl_cursor)
     {
        ERR("Could not get Wayland Cursor from Cursor Theme: %s",
            input->cursor.theme_name);
        return;
     }

   input->cursor.wl_cursor = wl_cursor;

   if ((!wl_cursor->images) || (!wl_cursor->images[0]))
     {
        if (input->wl.pointer)
          wl_pointer_set_cursor(input->wl.pointer, input->pointer.enter_serial,
                                NULL, 0, 0);
        return;
     }

   input->cursor.index = 0;

   _ecore_wl2_input_cursor_update(input);
}

void
_ecore_wl2_input_cursor_update_stop(Ecore_Wl2_Input *input)
{
   if (!input->cursor.timer) return;
   ecore_timer_del(input->cursor.timer);
   input->cursor.timer = NULL;
}

EAPI void
ecore_wl2_input_grab(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, unsigned int button)
{
   EINA_SAFETY_ON_NULL_RETURN(input);

   _ecore_wl2_input_grab(input, window, button);
}

EAPI void
ecore_wl2_input_ungrab(Ecore_Wl2_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN(input);

   _ecore_wl2_input_ungrab(input);
}

EAPI struct wl_seat *
ecore_wl2_input_seat_get(Ecore_Wl2_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, NULL);

   return input->wl.seat;
}

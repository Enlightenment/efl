/* this file contains code copied from weston; the copyright notice is below */
/*
 * Copyright © 2008 Kristian Høgsberg
 * Copyright © 2012-2013 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

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

static Ecore_Wl2_Input_Devices *
_ecore_wl2_devices_get(const Ecore_Wl2_Input *input, int window_id)
{
   Ecore_Wl2_Input_Devices *devices;
   Eina_List *l;

   EINA_LIST_FOREACH(input->devices_list, l, devices)
     {
        if (devices->window_id == window_id)
          return devices;
     }

   return NULL;
}

static Eo *
_ecore_wl2_mouse_dev_get(Ecore_Wl2_Input *input, int window_id)
{
   Ecore_Wl2_Input_Devices *devices;

   devices = _ecore_wl2_devices_get(input, window_id);
   if (devices && devices->pointer_dev)
     return efl_ref(devices->pointer_dev);

   return NULL;
}

static Eo *
_ecore_wl2_touch_dev_get(Ecore_Wl2_Input *input, int window_id)
{
   Ecore_Wl2_Input_Devices *devices;

   devices = _ecore_wl2_devices_get(input, window_id);
   if (devices && devices->touch_dev)
     return efl_ref(devices->touch_dev);

   return NULL;
}

static Eo *
_ecore_wl2_seat_dev_get(Ecore_Wl2_Input *input, int window_id)
{
   Ecore_Wl2_Input_Devices *devices;

   devices = _ecore_wl2_devices_get(input, window_id);
   if (devices)
     return efl_ref(devices->seat_dev);

   return NULL;
}

static void
_input_event_cb_free(void *data, void *event)
{
   if (data)
     efl_unref(data);
   free(event);
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
   ev->dev = _ecore_wl2_mouse_dev_get(input, window->id);

   ecore_event_add(ECORE_EVENT_MOUSE_IN, ev, _input_event_cb_free, ev->dev);
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
   ev->dev = _ecore_wl2_mouse_dev_get(input, window->id);

   ecore_event_add(ECORE_EVENT_MOUSE_OUT, ev, _input_event_cb_free, ev->dev);
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

   if ((input->focus.touch) && (input->focus.touch == window))
     ev->dev = _ecore_wl2_touch_dev_get(input, window->id);
   else if ((input->focus.pointer) && (input->focus.pointer == window))
     ev->dev = _ecore_wl2_mouse_dev_get(input, window->id);

   info = _ecore_wl2_input_mouse_down_info_get(device);
   if (info)
     {
        info->sx = input->pointer.sx;
        info->sy = input->pointer.sy;
     }

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, _input_event_cb_free, ev->dev);
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
   value /= 10;

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
        ev->dev = _ecore_wl2_mouse_dev_get(input, input->focus.pointer->id);
     }
   else if (input->focus.touch)
     {
        ev->window = input->focus.touch->id;
        ev->event_window = input->focus.touch->id;
        ev->dev = _ecore_wl2_touch_dev_get(input, input->focus.touch->id);
     }

   if (!ev->dev)
     {
        ev->dev = _ecore_wl2_mouse_dev_get(input, ev->window);
        if (!ev->dev)
          ev->dev = _ecore_wl2_touch_dev_get(input, ev->window);
     }

   ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, _input_event_cb_free, ev->dev);
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

   if (window)
     {
        ev->window = window->id;
        ev->event_window = window->id;

        if ((input->focus.touch) && (input->focus.touch == window))
          ev->dev = _ecore_wl2_touch_dev_get(input, window->id);
        else if ((input->focus.pointer) && (input->focus.pointer == window))
          ev->dev = _ecore_wl2_mouse_dev_get(input, window->id);
     }

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev,
                   _input_event_cb_free, ev->dev);

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

   if ((input->focus.touch) && (input->focus.touch == window))
     ev->dev = _ecore_wl2_touch_dev_get(input, window->id);
   else if ((input->focus.pointer) && (input->focus.pointer == window))
     ev->dev = _ecore_wl2_mouse_dev_get(input, window->id);

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev,
                   _input_event_cb_free, ev->dev);
}

static void
_input_event_focus_cb_free(void *data, void *event)
{
   Ecore_Wl2_Event_Focus_In *ev = event;
   if (data)
     efl_unref(data);
   ecore_wl2_display_disconnect(ev->display);
   free(event);
}

static void
_ecore_wl2_input_focus_in_send(Ecore_Wl2_Window *window, Ecore_Wl2_Input *input)
{
   Ecore_Wl2_Event_Focus_In *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Focus_In));
   if (!ev) return;

   ev->timestamp = input->timestamp;
   ev->window = window->id;
   ev->dev = _ecore_wl2_seat_dev_get(input, window->id);
   ev->display = input->display;
   ev->display->refs++;
   ecore_event_add(ECORE_WL2_EVENT_FOCUS_IN, ev, _input_event_focus_cb_free,
                   ev->dev);
}

static void
_ecore_wl2_input_focus_out_send(Ecore_Wl2_Window *window, Ecore_Wl2_Input *input)
{
   Ecore_Wl2_Event_Focus_Out *ev;

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Focus_Out));
   if (!ev) return;

   ev->timestamp = input->timestamp;
   ev->window = window->id;
   ev->dev = _ecore_wl2_seat_dev_get(input, window->id);
   ev->display = input->display;
   ev->display->refs++;
   ecore_event_add(ECORE_WL2_EVENT_FOCUS_OUT, ev, _input_event_focus_cb_free,
                   ev->dev);
}

static int
_ecore_wl2_input_key_translate(xkb_keysym_t keysym, unsigned int modifiers, char *buffer, int bytes)
{
/* this function is copied, with slight changes in variable names, from KeyBind.c in libX11
 * the license from that file can be found below:
 */
/*

Copyright 1985, 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/
   if (!keysym) return 0;

   /* check for possible control codes */
   if (modifiers & ECORE_EVENT_MODIFIER_CTRL)
     {
        Eina_Bool valid_control_code = EINA_TRUE;
        unsigned long hbytes = 0;
        unsigned char c;

        hbytes = (keysym >> 8);
        if (!(bytes &&
        ((hbytes == 0) ||
        ((hbytes == 0xFF) &&
        (((keysym >= XKB_KEY_BackSpace) && (keysym <= XKB_KEY_Clear)) ||
        (keysym == XKB_KEY_Return) ||
        (keysym == XKB_KEY_Escape) ||
        (keysym == XKB_KEY_KP_Space) ||
        (keysym == XKB_KEY_KP_Tab) ||
        (keysym == XKB_KEY_KP_Enter) ||
        ((keysym >= XKB_KEY_KP_Multiply) && (keysym <= XKB_KEY_KP_9)) ||
        (keysym == XKB_KEY_KP_Equal) ||
        (keysym == XKB_KEY_Delete))))))
          return 0;

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
             ((c >= 'a') && (c <= 'z')) ||  /* the lowercase symbols (not part of the standard, but useful) */
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
        /* if its not a control code, try to produce useful output */
        if (!xkb_keysym_to_utf8(keysym, buffer, bytes))
          return 0;
     }

   return 1;
}

static void
_ecore_wl2_input_symbol_rep_find(xkb_keysym_t keysym, char *buffer, int size, unsigned int code)
{
    int n = 0;

    n = xkb_keysym_to_utf8(keysym, buffer, size);

    /* check if we are a control code */
    if (n > 0 && !(
        (buffer[0] > 0x0 && buffer[0] <= 0x20) || /* others 0x0 to 0x20 control codes */
        buffer[0] == 0x7F)) /*delete control code */
      return;

    if (xkb_keysym_get_name(keysym, buffer, size) != 0)
      return;

    snprintf(buffer, size, "Keycode-%u", code);
}

static Eo *
_ecore_wl2_keyboard_dev_get(Ecore_Wl2_Input *input, int window_id)
{
   Ecore_Wl2_Input_Devices *devices;

   devices = _ecore_wl2_devices_get(input, window_id);
   if (devices && devices->keyboard_dev)
     return efl_ref(devices->keyboard_dev);

   return NULL;
}

static void
_ecore_wl2_input_key_send(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, xkb_keysym_t sym, xkb_keysym_t sym_name, unsigned int code, unsigned int state, unsigned int timestamp)
{
   Ecore_Event_Key *ev;
   char key[256] = "", keyname[256] = "", compose[256] = "";
   int name_len, key_len, comp_len;

   /*try to get a name or utf char of the given symbol */
   _ecore_wl2_input_symbol_rep_find(sym, key, sizeof(key), code);
   _ecore_wl2_input_symbol_rep_find(sym_name, keyname, sizeof(keyname), code);
   _ecore_wl2_input_key_translate(sym, input->keyboard.modifiers,
                                  compose, sizeof(compose));

   name_len = strlen(keyname);
   key_len = strlen(key);
   comp_len = strlen(compose);

   ev = calloc(1, sizeof(Ecore_Event_Key) + key_len + name_len + comp_len + 3);
   if (!ev) return;

   ev->keyname = (char *)(ev + 1);
   ev->key = ev->keyname + name_len + 1;
   ev->compose = comp_len ? ev->key + key_len + 1 : NULL;
   ev->string = ev->compose;

   strcpy((char *)ev->keyname, keyname);
   strcpy((char *)ev->key, key);
   if (comp_len) strcpy((char *)ev->compose, compose);

   ev->window = window->id;
   ev->event_window = window->id;
   ev->timestamp = timestamp;
   ev->modifiers = input->keyboard.modifiers;
   ev->keycode = code;
   ev->dev = _ecore_wl2_keyboard_dev_get(input, window->id);

   /* DBG("Emitting Key event (%s,%s,%s,%s)\n", ev->keyname, ev->key, ev->compose, ev->string); */

   if (state)
     ecore_event_add(ECORE_EVENT_KEY_DOWN, ev, _input_event_cb_free, ev->dev);
   else
     ecore_event_add(ECORE_EVENT_KEY_UP, ev, _input_event_cb_free, ev->dev);
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

   input->focus.prev_pointer = NULL;
   input->focus.pointer = window;

   _ecore_wl2_input_mouse_in_send(input, window);

   if ((window->moving || window->resizing) && (input->grab.window == window))
     {
        _ecore_wl2_input_mouse_up_send(input, window, 0, input->grab.button,
                                       input->grab.timestamp);
        window->resizing = window->moving = EINA_FALSE;
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

static void
_keyboard_cb_keymap(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int format, int fd, unsigned int size)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Event_Seat_Keymap_Changed *ev;
   char *map = NULL;
   const char *locale;

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
   if (input->xkb.maskless_state) xkb_state_unref(input->xkb.maskless_state);

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
   input->xkb.maskless_state = xkb_state_new(input->xkb.keymap);

   if (!input->xkb.state || !input->xkb.maskless_state)
     {
        ERR("Failed to create keymap state");
        xkb_map_unref(input->xkb.keymap);
        input->xkb.keymap = NULL;
        return;
     }

   if (!(locale = getenv("LC_ALL")))
     if (!(locale = getenv("LC_CTYPE")))
       if (!(locale = getenv("LANG")))
         locale = "C";

   if (input->xkb.compose_table)
     xkb_compose_table_unref(input->xkb.compose_table);

   input->xkb.compose_table =
     xkb_compose_table_new_from_locale(input->display->xkb_context,
                                       locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
   if (input->xkb.compose_state)
     xkb_compose_state_unref(input->xkb.compose_state);
   input->xkb.compose_state = NULL;

   if (input->xkb.compose_table)
     {
        input->xkb.compose_state =
          xkb_compose_state_new(input->xkb.compose_table,
                                XKB_COMPOSE_STATE_NO_FLAGS);
     }

   ev = malloc(sizeof(Ecore_Wl2_Event_Seat_Keymap_Changed));
   if (ev)
     {
        ev->id = input->id;
        ev->display = input->display;
        input->display->refs++;
        ecore_event_add(ECORE_WL2_EVENT_SEAT_KEYMAP_CHANGED, ev,
                        _display_event_free, ev->display);
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
   _ecore_wl2_input_focus_in_send(window, input);
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
   window = _ecore_wl2_display_window_surface_find(input->display, surface);
   if (window)
     {
        if (input->focus.keyboard != window)
          ERR("Received keyboard.leave when keyboard did not have enter");
     }
   input->focus.keyboard = NULL;
   if (window)
     _ecore_wl2_input_focus_out_send(window, input);
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

   _ecore_wl2_input_key_send(input, input->focus.keyboard,
                             input->repeat.sym, input->repeat.sym_name,
                             input->repeat.key + 8,
                             WL_KEYBOARD_KEY_STATE_PRESSED,
                             input->repeat.time);

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

/* from weston/clients/window.c */
/* Translate symbols appropriately if a compose sequence is being entered */
static xkb_keysym_t
process_key_press(xkb_keysym_t sym, Ecore_Wl2_Input *input)
{
   if (!input->xkb.compose_state)
     return sym;
   if (sym == XKB_KEY_NoSymbol)
     return sym;
   if (xkb_compose_state_feed(input->xkb.compose_state, sym) !=
       XKB_COMPOSE_FEED_ACCEPTED)
     return sym;

   switch (xkb_compose_state_get_status(input->xkb.compose_state))
     {
      case XKB_COMPOSE_COMPOSING:
        return XKB_KEY_NoSymbol;
      case XKB_COMPOSE_COMPOSED:
        return xkb_compose_state_get_one_sym(input->xkb.compose_state);
      case XKB_COMPOSE_CANCELLED:
        return XKB_KEY_NoSymbol;
      case XKB_COMPOSE_NOTHING:
      default: break;
     }
   return sym;
}

static void
_keyboard_cb_key(void *data, struct wl_keyboard *keyboard EINA_UNUSED, unsigned int serial, unsigned int timestamp, unsigned int keycode, unsigned int state)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Window *window;
   unsigned int code;
   xkb_keysym_t sym = XKB_KEY_NoSymbol, sym_name = XKB_KEY_NoSymbol;
   const xkb_keysym_t *syms;

   input = data;
   if (!input) return;

   /* try to get the window which has keyboard focus */
   window = input->focus.keyboard;
   if (!window) return;

   input->display->serial = serial;

   /* xkb rules reflect X broken keycodes, so offset by 8 */
   code = keycode + 8;

   if (xkb_state_key_get_syms(input->xkb.state, code, &syms) == 1)
     sym = syms[0];
   if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
     sym = process_key_press(sym, input);
   sym_name = xkb_state_key_get_one_sym(input->xkb.maskless_state, code);

   _ecore_wl2_input_key_send(input, window, sym, sym_name, code,
                             state, timestamp);

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
        input->repeat.sym_name = sym;
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
   if (mask & input->xkb.altgr_mask)
     input->keyboard.modifiers |= ECORE_EVENT_MODIFIER_ALTGR;
   if (mask & input->xkb.scroll_mask)
     input->keyboard.modifiers |= ECORE_EVENT_MODIFIER_SCROLL;
   if (mask & input->xkb.num_mask)
     input->keyboard.modifiers |= ECORE_EVENT_MODIFIER_NUM;
   if (mask & input->xkb.caps_mask)
     input->keyboard.modifiers |= ECORE_EVENT_MODIFIER_CAPS;


   mask = xkb_state_serialize_mods(input->xkb.state, XKB_STATE_MODS_LOCKED);
   if (mask & input->xkb.scroll_mask)
     input->keyboard.modifiers |= ECORE_EVENT_LOCK_SCROLL;
   if (mask & input->xkb.num_mask)
     input->keyboard.modifiers |= ECORE_EVENT_LOCK_NUM;
   if (mask & input->xkb.caps_mask)
     input->keyboard.modifiers |= ECORE_EVENT_LOCK_CAPS;
}

static void
_keyboard_cb_repeat_setup(void *data, struct wl_keyboard *keyboard EINA_UNUSED, int32_t rate, int32_t delay)
{
   Ecore_Wl2_Input *input;
   Ecore_Wl2_Event_Seat_Keyboard_Repeat_Changed *ev;

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
   ev = malloc(sizeof(Ecore_Wl2_Event_Seat_Keymap_Changed));
   if (ev)
     {
        ev->id = input->id;
        ev->display = input->display;
        ev->display->refs++;
        ecore_event_add(ECORE_WL2_EVENT_SEAT_KEYBOARD_REPEAT_CHANGED, ev,
                        _display_event_free, ev->display);
     }
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
   Ecore_Wl2_Event_Seat_Capabilities *ev;
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return;

   if ((caps & WL_SEAT_CAPABILITY_POINTER) && (!input->wl.pointer))
     {
        input->wl.pointer = wl_seat_get_pointer(seat);
        wl_pointer_set_user_data(input->wl.pointer, input);
        wl_pointer_add_listener(input->wl.pointer, &_pointer_listener, input);
     }
   else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && (input->wl.pointer))
     {
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
   ecore_wl2_display_flush(input->display);

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Seat_Capabilities));
   EINA_SAFETY_ON_NULL_RETURN(ev);

   ev->id = input->id;
   ev->pointer_enabled = !!(caps & WL_SEAT_CAPABILITY_POINTER);
   ev->keyboard_enabled = !!(caps & WL_SEAT_CAPABILITY_KEYBOARD);
   ev->touch_enabled = !!(caps & WL_SEAT_CAPABILITY_TOUCH);
   ev->display = input->display;
   ev->display->refs++;

   ecore_event_add(ECORE_WL2_EVENT_SEAT_CAPABILITIES_CHANGED, ev,
                   _display_event_free, ev->display);
}

static void
_cb_seat_event_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Wl2_Event_Seat_Name *ev;

   ev = event;
   eina_stringshare_del(ev->name);
   ecore_wl2_display_disconnect(ev->display);
   free(ev);
}

static void
_seat_cb_name(void *data, struct wl_seat *seat EINA_UNUSED, const char *name)
{
   Ecore_Wl2_Event_Seat_Name *ev;
   Ecore_Wl2_Input *input;

   input = data;
   eina_stringshare_replace(&input->name, name);

   ev = calloc(1, sizeof(Ecore_Wl2_Event_Seat_Name));
   EINA_SAFETY_ON_NULL_RETURN(ev);

   ev->id = input->id;
   ev->name = eina_stringshare_add(name);
   ev->display = input->display;
   ev->display->refs++;

   ecore_event_add(ECORE_WL2_EVENT_SEAT_NAME_CHANGED, ev,
                   _cb_seat_event_free, NULL);
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

   if (!input->cursor.name)
     input->cursor.name = eina_stringshare_add("left_ptr");
}

Eina_Bool
_ecore_wl2_input_cursor_update(void *data)
{
   Ecore_Wl2_Input *input;

   input = data;
   if (!input) return EINA_FALSE;

   if (input->wl.pointer)
     wl_pointer_set_cursor(input->wl.pointer, input->pointer.enter_serial,
                           input->cursor.surface,
                           input->cursor.hot_x, input->cursor.hot_y);
   ecore_wl2_display_flush(input->display);

   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_wl2_devices_free(Ecore_Wl2_Input_Devices *devices)
{
   if (devices->seat_dev)
     efl_unref(devices->seat_dev);
   if (devices->pointer_dev)
     efl_unref(devices->pointer_dev);
   if (devices->keyboard_dev)
     efl_unref(devices->keyboard_dev);
   if (devices->touch_dev)
     efl_unref(devices->touch_dev);

   free(devices);
}

static Eina_Bool
_ecore_wl2_cb_device_event(void *data, int type, void *event)
{
   Ecore_Wl2_Input_Devices *devs, *devices = NULL;;
   Ecore_Wl2_Event_Device *ev = event;
   Ecore_Wl2_Input *input = data;
   Eina_List *l;

   if (input->id != ev->seat_id)
     return ECORE_CALLBACK_PASS_ON;

   EINA_LIST_FOREACH(input->devices_list, l, devs)
     {
        if (devs->window_id == ev->window_id)
          {
             devices = devs;
             break;
          }
     }

   if (type == ECORE_WL2_EVENT_DEVICE_ADDED)
     {
        if (!devices)
          {
             devices = calloc(1, sizeof(Ecore_Wl2_Input_Devices));
             EINA_SAFETY_ON_NULL_RETURN_VAL(devices, ECORE_CALLBACK_PASS_ON);
             input->devices_list =
               eina_list_append(input->devices_list, devices);
             devices->window_id = ev->window_id;
          }

        if (ev->type == ECORE_WL2_DEVICE_TYPE_POINTER)
          devices->pointer_dev = efl_ref(ev->dev);
        else if (ev->type == ECORE_WL2_DEVICE_TYPE_KEYBOARD)
          devices->keyboard_dev = efl_ref(ev->dev);
        else if (ev->type == ECORE_WL2_DEVICE_TYPE_TOUCH)
          devices->touch_dev = efl_ref(ev->dev);
        else if (ev->type == ECORE_WL2_DEVICE_TYPE_SEAT)
          devices->seat_dev = efl_ref(ev->dev);

        return ECORE_CALLBACK_PASS_ON;
     }

   if (!devices)
     return ECORE_CALLBACK_PASS_ON;

   if (ev->type == ECORE_WL2_DEVICE_TYPE_SEAT)
     {
        input->devices_list =
          eina_list_remove(input->devices_list, devices);
        _ecore_wl2_devices_free(devices);
        return ECORE_CALLBACK_PASS_ON;
     }

   if ((ev->type == ECORE_WL2_DEVICE_TYPE_POINTER) &&
       (devices->pointer_dev == ev->dev))
     {
        efl_unref(devices->pointer_dev);
        devices->pointer_dev = NULL;
     }
   else if ((ev->type == ECORE_WL2_DEVICE_TYPE_KEYBOARD) &&
            (devices->keyboard_dev == ev->dev))
     {
        efl_unref(devices->keyboard_dev);
        devices->keyboard_dev = NULL;
     }
   else if ((ev->type == ECORE_WL2_DEVICE_TYPE_TOUCH) &&
            (devices->touch_dev == ev->dev))
     {
        efl_unref(devices->touch_dev);
        devices->touch_dev = NULL;
     }

   return ECORE_CALLBACK_PASS_ON;
}

void
_ecore_wl2_input_add(Ecore_Wl2_Display *display, unsigned int id, unsigned int version)
{
   Ecore_Wl2_Input *input;

   input = calloc(1, sizeof(Ecore_Wl2_Input));
   if (!input) return;

   input->id = id;
   input->display = display;
   input->seat_version = version;
   input->repeat.rate = 0.025;
   input->repeat.delay = 0.4;
   input->repeat.enabled = EINA_TRUE;

   wl_array_init(&input->data.selection.types);
   wl_array_init(&input->data.drag.types);

   /* setup cursor size and theme */
   _ecore_wl2_input_cursor_setup(input);

   input->wl.seat =
     wl_registry_bind(display->wl.registry, id, &wl_seat_interface, 4);

   display->inputs =
     eina_inlist_append(display->inputs, EINA_INLIST_GET(input));

   wl_seat_add_listener(input->wl.seat, &_seat_listener, input);
   wl_seat_set_user_data(input->wl.seat, input);

   input->dev_add_handler =
     ecore_event_handler_add(ECORE_WL2_EVENT_DEVICE_ADDED,
                             _ecore_wl2_cb_device_event, input);

   input->dev_remove_handler =
     ecore_event_handler_add(ECORE_WL2_EVENT_DEVICE_REMOVED,
                             _ecore_wl2_cb_device_event, input);

   if (!display->wl.data_device_manager) return;

   input->data.device =
     wl_data_device_manager_get_data_device(display->wl.data_device_manager,
                                            input->wl.seat);
   wl_data_device_add_listener(input->data.device, &_data_listener, input);
}

void
_ecore_wl2_input_del(Ecore_Wl2_Input *input)
{
   Ecore_Wl2_Input_Devices *devices;
   Ecore_Wl2_Display *display;
   Eina_Inlist *l = NULL;
   Ecore_Wl2_Mouse_Down_Info *info = NULL;
   Ecore_Wl2_Window *window;

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

   if (input->cursor.name) eina_stringshare_del(input->cursor.name);

   if (input->data.selection.types.data)
     {
        char **t;

        wl_array_for_each(t, &input->data.selection.types)
          free(*t);

        wl_array_release(&input->data.selection.types);
     }
   if (input->data.drag.types.data)
     {
        char **t;

        wl_array_for_each(t, &input->data.drag.types)
          free(*t);

        wl_array_release(&input->data.drag.types);
     }

   if (input->data.selection.source)
     wl_data_source_destroy(input->data.selection.source);
   if (input->data.drag.source)
     wl_data_source_destroy(input->data.drag.source);
   if (input->drag.offer) _ecore_wl2_offer_unref(input->drag.offer);
   if (input->selection.offer) _ecore_wl2_offer_unref(input->selection.offer);
   if (input->data.device) wl_data_device_destroy(input->data.device);

   if (input->xkb.state) xkb_state_unref(input->xkb.state);
   if (input->xkb.maskless_state) xkb_state_unref(input->xkb.maskless_state);
   if (input->xkb.keymap) xkb_map_unref(input->xkb.keymap);
   if (input->xkb.compose_table)
     xkb_compose_table_unref(input->xkb.compose_table);
   if (input->xkb.compose_state)
     xkb_compose_state_unref(input->xkb.compose_state);

   if (input->wl.seat) wl_seat_destroy(input->wl.seat);

   ecore_event_handler_del(input->dev_add_handler);
   ecore_event_handler_del(input->dev_remove_handler);
   EINA_LIST_FREE(input->devices_list, devices)
      _ecore_wl2_devices_free(devices);

   display->inputs =
     eina_inlist_remove(display->inputs, EINA_INLIST_GET(input));

   EINA_INLIST_FOREACH(display->windows, window)
     if (window->grab == input) window->grab = NULL;

   eina_stringshare_replace(&input->name, NULL);
   free(input);
}

void
_ecore_wl2_input_cursor_set(Ecore_Wl2_Input *input, const char *cursor)
{
   eina_stringshare_replace(&input->cursor.name, cursor);
   if (!cursor) eina_stringshare_replace(&input->cursor.name, "left_ptr");
}

void
_ecore_wl2_input_window_remove(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Input_Devices *devices;
   Eina_List *l, *l_next;

   if ((input->focus.pointer) &&
       (input->focus.pointer == window))
     input->focus.pointer = NULL;
   if ((input->focus.keyboard) &&
       (input->focus.keyboard == window))
     {
        input->focus.keyboard = NULL;
        ecore_timer_del(input->repeat.timer);
        input->repeat.timer = NULL;
     }

   EINA_LIST_FOREACH_SAFE(input->devices_list, l, l_next, devices)
      if (devices->window_id == window->id)
        {
           _ecore_wl2_devices_free(devices);
           input->devices_list = eina_list_remove_list(input->devices_list, l);
        }
}

EAPI void
ecore_wl2_input_grab(Ecore_Wl2_Input *input, Ecore_Wl2_Window *window, unsigned int button)
{
   EINA_SAFETY_ON_NULL_RETURN(input);
   EINA_SAFETY_ON_NULL_RETURN(input->display);
   _ecore_wl2_input_grab(input, window, button);
}

EAPI void
ecore_wl2_input_ungrab(Ecore_Wl2_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN(input);
   EINA_SAFETY_ON_NULL_RETURN(input->display);
   _ecore_wl2_input_ungrab(input);
}

EAPI struct wl_seat *
ecore_wl2_input_seat_get(Ecore_Wl2_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input->display, NULL);
   return input->wl.seat;
}

EAPI Ecore_Wl2_Seat_Capabilities
ecore_wl2_input_seat_capabilities_get(Ecore_Wl2_Input *input)
{
   Ecore_Wl2_Seat_Capabilities cap = ECORE_WL2_SEAT_CAPABILITIES_NONE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(input, cap);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input->display, 0);
   if (input->wl.keyboard)
     cap |= ECORE_WL2_SEAT_CAPABILITIES_KEYBOARD;
   if (input->wl.pointer)
     cap |= ECORE_WL2_SEAT_CAPABILITIES_POINTER;
   if (input->wl.touch)
     cap |= ECORE_WL2_SEAT_CAPABILITIES_TOUCH;
   return cap;
}

EAPI Eina_Stringshare *
ecore_wl2_input_name_get(Ecore_Wl2_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, NULL);
   return input->name;
}

EAPI unsigned int
ecore_wl2_input_seat_id_get(Ecore_Wl2_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input->display, 0);
   return input->id;
}

EAPI Ecore_Wl2_Display *
ecore_wl2_input_display_get(const Ecore_Wl2_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input->display, NULL);
   return input->display;
}

EAPI struct xkb_keymap *
ecore_wl2_input_keymap_get(const Ecore_Wl2_Input *input)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input->display, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(input->wl.keyboard, NULL);
   return input->xkb.keymap;
}

EAPI Eina_Bool
ecore_wl2_input_keyboard_repeat_get(const Ecore_Wl2_Input *input, double *rate, double *delay)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(input->display, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(input->wl.keyboard, EINA_FALSE);
   if (rate) *rate = input->repeat.rate;
   if (delay) *delay = input->repeat.delay;
   return input->repeat.enabled;
}

EAPI Eo *
ecore_wl2_input_seat_device_get(const Ecore_Wl2_Input *input, const Ecore_Wl2_Window *window)
{
   Ecore_Wl2_Input_Devices *devices;
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(window, NULL);

   devices = _ecore_wl2_devices_get(input, window->id);
   return devices ?  devices->seat_dev : NULL;
}

EAPI void
ecore_wl2_input_pointer_set(Ecore_Wl2_Input *input, struct wl_surface *surface, int hot_x, int hot_y)
{
   EINA_SAFETY_ON_NULL_RETURN(input);

   input->cursor.surface = surface;
   input->cursor.hot_x = hot_x;
   input->cursor.hot_y = hot_y;

   _ecore_wl2_input_cursor_update(input);
}

EAPI void
ecore_wl2_input_cursor_from_name_set(Ecore_Wl2_Input *input, const char *cursor)
{
   EINA_SAFETY_ON_NULL_RETURN(input);
   _ecore_wl2_input_cursor_set(input, cursor);
}

EAPI Eina_Bool
ecore_wl2_input_pointer_xy_get(const Ecore_Wl2_Input *input, int *x, int *y)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(input, EINA_FALSE);

   if (x) *x = 0;
   if (y) *y = 0;
   if (!input->wl.pointer) return EINA_FALSE;
   if (x) *x = input->pointer.sx;
   if (y) *y = input->pointer.sy;
   return EINA_TRUE;
}

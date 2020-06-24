/* Portions of this code have been derived from Weston
 *
 * Copyright © 2008-2012 Kristian Høgsberg
 * Copyright © 2010-2012 Intel Corporation
 * Copyright © 2010-2011 Benjamin Franzke
 * Copyright © 2011-2012 Collabora, Ltd.
 * Copyright © 2010 Red Hat <mjg@redhat.com>
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

#include "ecore_drm_private.h"
#include <ctype.h>
#include <math.h>

static void  _device_modifiers_update(Ecore_Drm_Evdev *edev);

static void
_device_calibration_set(Ecore_Drm_Evdev *edev)
{
   const char *sysname;
   float cal[6];
   const char *device;
   Eina_List *devices;
   const char *vals;
   enum libinput_config_status status;

   if ((!libinput_device_config_calibration_has_matrix(edev->device)) ||
       (libinput_device_config_calibration_get_default_matrix(edev->device, cal) != 0))
     return;

   sysname = libinput_device_get_sysname(edev->device);

   devices = eeze_udev_find_by_subsystem_sysname("input", sysname);
   if (eina_list_count(devices) < 1) return;

   EINA_LIST_FREE(devices, device)
     {
        vals = eeze_udev_syspath_get_property(device, "WL_CALIBRATION");
	if ((!vals) ||
            (sscanf(vals, "%f %f %f %f %f %f",
                    &cal[0], &cal[1], &cal[2], &cal[3], &cal[4], &cal[5]) != 6))
          goto cont;

        cal[2] /= edev->output->current_mode->width;
        cal[5] /= edev->output->current_mode->height;

        status =
          libinput_device_config_calibration_set_matrix(edev->device, cal);

        if (status != LIBINPUT_CONFIG_STATUS_SUCCESS)
          ERR("Failed to apply calibration");

cont:
        eina_stringshare_del(device);
        continue;
     }
}

static void
_device_output_set(Ecore_Drm_Evdev *edev)
{
   Ecore_Drm_Input *input;
   Ecore_Drm_Output *output = NULL;
   const char *oname;

   if (!edev->seat) return;
   if (!(input = edev->seat->input)) return;

   oname = libinput_device_get_output_name(edev->device);
   if (oname)
     {
        Eina_List *l;

        DBG("Device Has Output Name: %s", oname);

        EINA_LIST_FOREACH(input->dev->outputs, l, output)
          if ((output->name) && (!strcmp(output->name, oname))) break;
     }

   if (!output)
     output = eina_list_nth(input->dev->outputs, 0);

   if (!output) return;

   edev->output = output;

   if (libinput_device_has_capability(edev->device,
                                      LIBINPUT_DEVICE_CAP_POINTER))
     {
        edev->seat->ptr.ix = edev->seat->ptr.dx = edev->output->current_mode->width / 2;
        edev->seat->ptr.iy = edev->seat->ptr.dy = edev->output->current_mode->height / 2;
        edev->mouse.dx = edev->seat->ptr.dx;
        edev->mouse.dy = edev->seat->ptr.dy;
     }
}

static void
_device_configure(Ecore_Drm_Evdev *edev)
{
   if (libinput_device_config_tap_get_finger_count(edev->device) > 0)
     {
        Eina_Bool tap = EINA_FALSE;

        tap = libinput_device_config_tap_get_default_enabled(edev->device);
        libinput_device_config_tap_set_enabled(edev->device, tap);
     }

   ecore_drm_outputs_geometry_get(edev->seat->input->dev,
                                  &edev->mouse.minx, &edev->mouse.miny,
                                  &edev->mouse.maxw, &edev->mouse.maxh);

   _device_output_set(edev);
   _device_calibration_set(edev);
}

static void
_device_keyboard_setup(Ecore_Drm_Evdev *edev)
{
   Ecore_Drm_Input *input;

   if ((!edev) || (!edev->seat)) return;
   if (!(input = edev->seat->input)) return;
   if (!input->dev->xkb_ctx) return;

   /* create keymap from xkb context */
   edev->xkb.keymap = _ecore_drm_device_cached_keymap_get(input->dev->xkb_ctx, NULL, 0);
   if (!edev->xkb.keymap)
     {
        ERR("Failed to create keymap");
        return;
     }

   /* create xkb state */
   if (!(edev->xkb.state = xkb_state_new(edev->xkb.keymap)))
     {
        ERR("Failed to create xkb state");
        return;
     }

   edev->xkb.ctrl_mask =
     1 << xkb_map_mod_get_index(edev->xkb.keymap, XKB_MOD_NAME_CTRL);
   edev->xkb.alt_mask =
     1 << xkb_map_mod_get_index(edev->xkb.keymap, XKB_MOD_NAME_ALT);
   edev->xkb.shift_mask =
     1 << xkb_map_mod_get_index(edev->xkb.keymap, XKB_MOD_NAME_SHIFT);
   edev->xkb.win_mask =
     1 << xkb_map_mod_get_index(edev->xkb.keymap, XKB_MOD_NAME_LOGO);
   edev->xkb.scroll_mask =
     1 << xkb_map_mod_get_index(edev->xkb.keymap, XKB_LED_NAME_SCROLL);
   edev->xkb.num_mask =
     1 << xkb_map_mod_get_index(edev->xkb.keymap, XKB_LED_NAME_NUM);
   edev->xkb.caps_mask =
     1 << xkb_map_mod_get_index(edev->xkb.keymap, XKB_MOD_NAME_CAPS);
   edev->xkb.altgr_mask =
     1 << xkb_map_mod_get_index(edev->xkb.keymap, "ISO_Level3_Shift");
}

static int
_device_keysym_translate(xkb_keysym_t keysym, unsigned int modifiers, char *buffer, int bytes)
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
_device_modifiers_update_device(Ecore_Drm_Evdev *edev, Ecore_Drm_Evdev *from)
{
   xkb_mod_mask_t mask;

   edev->xkb.depressed =
     xkb_state_serialize_mods(from->xkb.state, XKB_STATE_DEPRESSED);
   edev->xkb.latched =
     xkb_state_serialize_mods(from->xkb.state, XKB_STATE_LATCHED);
   edev->xkb.locked =
     xkb_state_serialize_mods(from->xkb.state, XKB_STATE_LOCKED);
   edev->xkb.group =
     xkb_state_serialize_mods(from->xkb.state, XKB_STATE_EFFECTIVE);

   mask = (edev->xkb.depressed | edev->xkb.latched);

   if (mask & from->xkb.ctrl_mask)
     edev->xkb.modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if (mask & from->xkb.alt_mask)
     edev->xkb.modifiers |= ECORE_EVENT_MODIFIER_ALT;
   if (mask & from->xkb.shift_mask)
     edev->xkb.modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if (mask & from->xkb.win_mask)
     edev->xkb.modifiers |= ECORE_EVENT_MODIFIER_WIN;
   if (mask & from->xkb.scroll_mask)
     edev->xkb.modifiers |= ECORE_EVENT_LOCK_SCROLL;
   if (mask & from->xkb.num_mask)
     edev->xkb.modifiers |= ECORE_EVENT_LOCK_NUM;
   if (mask & from->xkb.caps_mask)
     edev->xkb.modifiers |= ECORE_EVENT_LOCK_CAPS;
   if (mask & from->xkb.altgr_mask)
     edev->xkb.modifiers |= ECORE_EVENT_MODIFIER_ALTGR;
}

static void
_device_modifiers_update(Ecore_Drm_Evdev *edev)
{
   edev->xkb.modifiers = 0;

   if (edev->seat_caps & EVDEV_SEAT_KEYBOARD)
     _device_modifiers_update_device(edev, edev);
   else
     {
        Eina_List *l;
        Ecore_Drm_Evdev *ed;

        EINA_LIST_FOREACH(edev->seat->devices, l, ed)
          {
             if (!(ed->seat_caps & EVDEV_SEAT_KEYBOARD)) continue;
             _device_modifiers_update_device(edev, ed);
          }
     }

}

static int
_device_remapped_key_get(Ecore_Drm_Evdev *edev, int code)
{
   void *ret = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(edev, code);
   if (!edev->key_remap_enabled) return code;
   EINA_SAFETY_ON_NULL_RETURN_VAL(edev->key_remap_hash, code);

   ret = eina_hash_find(edev->key_remap_hash, &code);

   if (ret) code = (int)(intptr_t)ret;

   return code;
}

static void
_device_handle_key(struct libinput_device *device, struct libinput_event_keyboard *event)
{
   Ecore_Drm_Evdev *edev;
   Ecore_Drm_Input *input;
   uint32_t timestamp, nsyms;
   uint32_t code = 0;
   const xkb_keysym_t *syms;
   enum libinput_key_state state;
   int key_count;
   xkb_keysym_t sym = XKB_KEY_NoSymbol;
   char key[256], keyname[256], compose_buffer[256];
   Ecore_Event_Key *e;
   char *tmp = NULL, *compose = NULL;

   if (!(edev = libinput_device_get_user_data(device))) return;

   if (!(input = edev->seat->input)) return;

   timestamp = libinput_event_keyboard_get_time(event);
   code = libinput_event_keyboard_get_key(event);

   if (!code) return;

   code = _device_remapped_key_get(edev, code) + 8;

   state = libinput_event_keyboard_get_key_state(event);
   key_count = libinput_event_keyboard_get_seat_key_count(event);

   /* ignore key events that are not seat wide state changes */
   if (((state == LIBINPUT_KEY_STATE_PRESSED) && (key_count != 1)) ||
       ((state == LIBINPUT_KEY_STATE_RELEASED) && (key_count != 0)))
     return;

   xkb_state_update_key(edev->xkb.state, code,
                        (state ? XKB_KEY_DOWN : XKB_KEY_UP));

   /* get the keysym for this code */
   nsyms = xkb_key_get_syms(edev->xkb.state, code, &syms);
   if (nsyms == 1) sym = syms[0];

   /* get the keyname for this sym */
   memset(key, 0, sizeof(key));
   xkb_keysym_get_name(sym, key, sizeof(key));

   memset(keyname, 0, sizeof(keyname));
   memcpy(keyname, key, sizeof(keyname));

   if (keyname[0] == '\0')
     snprintf(keyname, sizeof(keyname), "Keycode-%u", code);

   /* if shift is active, we need to transform the key to lower */
   if (xkb_state_mod_index_is_active(edev->xkb.state,
                                     xkb_map_mod_get_index(edev->xkb.keymap,
                                                           XKB_MOD_NAME_SHIFT),
                                     XKB_STATE_MODS_EFFECTIVE))
     {
        if (keyname[0] != '\0')
          keyname[0] = tolower(keyname[0]);
     }

   memset(compose_buffer, 0, sizeof(compose_buffer));
   if (_device_keysym_translate(sym, edev->xkb.modifiers,
                                compose_buffer, sizeof(compose_buffer)))
     {
        compose = eina_str_convert("ISO8859-1", "UTF-8", compose_buffer);
        if (!compose)
          {
             ERR("Ecore_DRM cannot convert input key string '%s' to UTF-8. "
                 "Is Eina built with iconv support?", compose_buffer);
          }
        else
          tmp = compose;
     }

   if (!compose) compose = compose_buffer;

   e = calloc(1, sizeof(Ecore_Event_Key) + strlen(key) + strlen(keyname) +
              ((compose[0] != '\0') ? strlen(compose) : 0) + 3);
   if (!e) goto err;

   e->keyname = (char *)(e + 1);
   e->key = e->keyname + strlen(keyname) + 1;
   e->compose = strlen(compose) ? e->key + strlen(key) + 1 : NULL;
   e->string = e->compose;

   strcpy((char *)e->keyname, keyname);
   strcpy((char *)e->key, key);
   if (strlen(compose)) strcpy((char *)e->compose, compose);

   e->window = (Ecore_Window)input->dev->window;
   e->event_window = (Ecore_Window)input->dev->window;
   e->root_window = (Ecore_Window)input->dev->window;
   e->timestamp = timestamp;
   e->same_screen = 1;
   e->keycode = code;

   _device_modifiers_update(edev);

   e->modifiers = edev->xkb.modifiers;

   if (state)
     ecore_event_add(ECORE_EVENT_KEY_DOWN, e, NULL, NULL);
   else
     ecore_event_add(ECORE_EVENT_KEY_UP, e, NULL, NULL);

err:
   if (tmp) free(tmp);
}

static void
_device_pointer_motion(Ecore_Drm_Evdev *edev, struct libinput_event_pointer *event)
{
   Ecore_Drm_Input *input;
   Ecore_Event_Mouse_Move *ev;

   if (!(input = edev->seat->input)) return;

   if (!(ev = calloc(1, sizeof(Ecore_Event_Mouse_Move)))) return;

   if (edev->seat->ptr.ix < edev->mouse.minx)
     edev->seat->ptr.dx = edev->seat->ptr.ix = edev->mouse.minx;
   else if (edev->seat->ptr.ix >= (edev->mouse.minx + edev->mouse.maxw))
     edev->seat->ptr.dx = edev->seat->ptr.ix = (edev->mouse.minx + edev->mouse.maxw - 1);

   if (edev->seat->ptr.iy < edev->mouse.miny)
     edev->seat->ptr.dy = edev->seat->ptr.iy = edev->mouse.miny;
   else if (edev->seat->ptr.iy >= (edev->mouse.miny + edev->mouse.maxh))
     edev->seat->ptr.dy = edev->seat->ptr.iy = (edev->mouse.miny + edev->mouse.maxh - 1);

   edev->mouse.dx = edev->seat->ptr.dx;
   edev->mouse.dy = edev->seat->ptr.dy;

   ev->window = (Ecore_Window)input->dev->window;
   ev->event_window = (Ecore_Window)input->dev->window;
   ev->root_window = (Ecore_Window)input->dev->window;
   if (event) ev->timestamp = libinput_event_pointer_get_time(event);
   ev->same_screen = 1;

   _device_modifiers_update(edev);
   ev->modifiers = edev->xkb.modifiers;

   ev->x = edev->seat->ptr.ix;
   ev->y = edev->seat->ptr.iy;
   ev->root.x = ev->x;
   ev->root.y = ev->y;

   ev->multi.device = edev->mt_slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = ev->x;
   ev->multi.y = ev->y;
   ev->multi.root.x = ev->x;
   ev->multi.root.y = ev->y;

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
}

void
_ecore_drm_pointer_motion_post(Ecore_Drm_Evdev *edev)
{
   _device_pointer_motion(edev, NULL);
}

static void
_device_handle_pointer_motion(struct libinput_device *device, struct libinput_event_pointer *event)
{
   Ecore_Drm_Evdev *edev;

   if (!(edev = libinput_device_get_user_data(device))) return;

   edev->seat->ptr.dx += libinput_event_pointer_get_dx(event);
   edev->seat->ptr.dy += libinput_event_pointer_get_dy(event);

   edev->mouse.dx = edev->seat->ptr.dx;
   edev->mouse.dy = edev->seat->ptr.dy;

   if (floor(edev->seat->ptr.dx) == edev->seat->ptr.ix &&
       floor(edev->seat->ptr.dy) == edev->seat->ptr.iy) return;

   edev->seat->ptr.ix = edev->seat->ptr.dx;
   edev->seat->ptr.iy = edev->seat->ptr.dy;
  _device_pointer_motion(edev, event);
}

static void
_device_handle_pointer_motion_absolute(struct libinput_device *device, struct libinput_event_pointer *event)
{
   Ecore_Drm_Evdev *edev;

   if (!(edev = libinput_device_get_user_data(device))) return;

   edev->mouse.dx = edev->seat->ptr.dx =
     libinput_event_pointer_get_absolute_x_transformed(event,
                                                       edev->output->current_mode->width);
   edev->mouse.dy = edev->seat->ptr.dy =
     libinput_event_pointer_get_absolute_y_transformed(event,
                                                       edev->output->current_mode->height);

   if (floor(edev->seat->ptr.dx) == edev->seat->ptr.ix &&
       floor(edev->seat->ptr.dy) == edev->seat->ptr.iy) return;

   edev->seat->ptr.ix = edev->seat->ptr.dx;
   edev->seat->ptr.iy = edev->seat->ptr.dy;
   _device_pointer_motion(edev, event);
}

static void
_device_handle_button(struct libinput_device *device, struct libinput_event_pointer *event)
{
   Ecore_Drm_Evdev *edev;
   Ecore_Drm_Input *input;
   Ecore_Event_Mouse_Button *ev;
   enum libinput_button_state state;
   uint32_t button, timestamp;

   if (!(edev = libinput_device_get_user_data(device))) return;
   if (!(input = edev->seat->input)) return;

   if (!(ev = calloc(1, sizeof(Ecore_Event_Mouse_Button)))) return;

   state = libinput_event_pointer_get_button_state(event);
   button = libinput_event_pointer_get_button(event);
   timestamp = libinput_event_pointer_get_time(event);

   button = ((button & 0x00F) + 1);
   if (button == 3) button = 2;
   else if (button == 2) button = 3;

   ev->window = (Ecore_Window)input->dev->window;
   ev->event_window = (Ecore_Window)input->dev->window;
   ev->root_window = (Ecore_Window)input->dev->window;
   ev->timestamp = timestamp;
   ev->same_screen = 1;

   _device_modifiers_update(edev);
   ev->modifiers = edev->xkb.modifiers;

   ev->x = edev->seat->ptr.ix;
   ev->y = edev->seat->ptr.iy;
   ev->root.x = ev->x;
   ev->root.y = ev->y;

   ev->multi.device = edev->mt_slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = ev->x;
   ev->multi.y = ev->y;
   ev->multi.root.x = ev->x;
   ev->multi.root.y = ev->y;

   if (state)
     {
        unsigned int current;

        current = timestamp;
        edev->mouse.did_double = EINA_FALSE;
        edev->mouse.did_triple = EINA_FALSE;

        if (((current - edev->mouse.prev) <= edev->mouse.threshold) &&
            (button == edev->mouse.prev_button))
          {
             edev->mouse.did_double = EINA_TRUE;
             if (((current - edev->mouse.last) <= (2 * edev->mouse.threshold)) &&
                 (button == edev->mouse.last_button))
               {
                  edev->mouse.did_triple = EINA_TRUE;
                  edev->mouse.prev = 0;
                  edev->mouse.last = 0;
                  current = 0;
               }
          }

        edev->mouse.last = edev->mouse.prev;
        edev->mouse.prev = current;
        edev->mouse.last_button = edev->mouse.prev_button;
        edev->mouse.prev_button = button;
     }

   ev->buttons = button;

   if (edev->mouse.did_double)
     ev->double_click = 1;
   if (edev->mouse.did_triple)
     ev->triple_click = 1;

   if (state)
     ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
   else
     ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
}

static double
_event_scroll_get(struct libinput_event_pointer *pe, enum libinput_pointer_axis axis)
{
   switch (libinput_event_pointer_get_axis_source(pe))
     {
      case LIBINPUT_POINTER_AXIS_SOURCE_WHEEL:
         return libinput_event_pointer_get_axis_value_discrete(pe, axis);
      case LIBINPUT_POINTER_AXIS_SOURCE_FINGER:
      case LIBINPUT_POINTER_AXIS_SOURCE_CONTINUOUS:
         return libinput_event_pointer_get_axis_value(pe, axis);
     }
   return 0.0;
}

static void
_device_handle_axis(struct libinput_device *device, struct libinput_event_pointer *event)
{
   Ecore_Drm_Evdev *edev;
   Ecore_Drm_Input *input;
   Ecore_Event_Mouse_Wheel *ev;
   uint32_t timestamp;
   enum libinput_pointer_axis axis;

   if (!(edev = libinput_device_get_user_data(device))) return;
   if (!(input = edev->seat->input)) return;

   if (!(ev = calloc(1, sizeof(Ecore_Event_Mouse_Wheel)))) return;

   timestamp = libinput_event_pointer_get_time(event);

   ev->window = (Ecore_Window)input->dev->window;
   ev->event_window = (Ecore_Window)input->dev->window;
   ev->root_window = (Ecore_Window)input->dev->window;
   ev->timestamp = timestamp;
   ev->same_screen = 1;

   _device_modifiers_update(edev);
   ev->modifiers = edev->xkb.modifiers;

   ev->x = edev->seat->ptr.ix;
   ev->y = edev->seat->ptr.iy;
   ev->root.x = ev->x;
   ev->root.y = ev->y;

   axis = LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL;
   if (libinput_event_pointer_has_axis(event, axis))
     ev->z = _event_scroll_get(event, axis);

   axis = LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL;
   if (libinput_event_pointer_has_axis(event, axis))
     {
        ev->direction = 1;
        ev->z = _event_scroll_get(event, axis);
     }

   ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
}

Ecore_Drm_Evdev *
_ecore_drm_evdev_device_create(Ecore_Drm_Seat *seat, struct libinput_device *device)
{
   Ecore_Drm_Evdev *edev;
   Eina_List *devices;

   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, NULL);

   /* try to allocate space for new evdev */
   if (!(edev = calloc(1, sizeof(Ecore_Drm_Evdev)))) return NULL;

   edev->seat = seat;
   edev->device = device;
   edev->path = eina_stringshare_add(libinput_device_get_sysname(device));

   devices = eeze_udev_find_by_filter("input", NULL, edev->path);
   if (eina_list_count(devices) >= 1)
     {
        Eina_List *l;
        const char *dev, *name;

        EINA_LIST_FOREACH(devices, l, dev)
          {
             name = eeze_udev_syspath_get_devname(dev);
             if (strstr(name, edev->path))
               {
                  eina_stringshare_replace(&edev->path, eeze_udev_syspath_get_devpath(dev));
                  break;
               }
          }

        EINA_LIST_FREE(devices, dev)
          eina_stringshare_del(dev);
     }

   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_KEYBOARD))
     {
        edev->seat_caps |= EVDEV_SEAT_KEYBOARD;
        _device_keyboard_setup(edev);
     }

   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_POINTER))
     {
        Ecore_Drm_Device *dev;

        edev->seat_caps |= EVDEV_SEAT_POINTER;

        /* TODO: make this configurable */
        edev->mouse.threshold = 250;

        dev = seat->input->dev;
        if (dev->left_handed == EINA_TRUE)
          {
             if (libinput_device_config_left_handed_set(device, 1) !=
                 LIBINPUT_CONFIG_STATUS_SUCCESS)
               {
                  WRN("Failed to set left hand mode about device: %s\n",
                      libinput_device_get_name(device));
               }
          }
     }

   if (libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_TOUCH))
     {
        edev->seat_caps |= EVDEV_SEAT_TOUCH;
     }

   libinput_device_set_user_data(device, edev);
   libinput_device_ref(device);

   /* configure device */
   _device_configure(edev);

   return edev;
}

static void
_device_handle_touch_event_send(Ecore_Drm_Evdev *edev, struct libinput_event_touch *event, int state)
{
   Ecore_Drm_Input *input;
   Ecore_Event_Mouse_Button *ev;
   uint32_t timestamp, button = 0;

   if (!edev) return;
   if (!(input = edev->seat->input)) return;

   if (!(ev = calloc(1, sizeof(Ecore_Event_Mouse_Button)))) return;

   timestamp = libinput_event_touch_get_time(event);

   ev->window = (Ecore_Window)input->dev->window;
   ev->event_window = (Ecore_Window)input->dev->window;
   ev->root_window = (Ecore_Window)input->dev->window;
   ev->timestamp = timestamp;
   ev->same_screen = 1;

   _device_modifiers_update(edev);
   ev->modifiers = edev->xkb.modifiers;

   ev->x = edev->seat->ptr.ix;
   ev->y = edev->seat->ptr.iy;
   ev->root.x = ev->x;
   ev->root.y = ev->y;

   ev->multi.device = edev->mt_slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = ev->x;
   ev->multi.y = ev->y;
   ev->multi.root.x = ev->x;
   ev->multi.root.y = ev->y;

   if (state == ECORE_EVENT_MOUSE_BUTTON_DOWN)
     {
        unsigned int current;

        current = timestamp;
        edev->mouse.did_double = EINA_FALSE;
        edev->mouse.did_triple = EINA_FALSE;

        if (((current - edev->mouse.prev) <= edev->mouse.threshold) &&
            (button == edev->mouse.prev_button))
          {
             edev->mouse.did_double = EINA_TRUE;
             if (((current - edev->mouse.last) <= (2 * edev->mouse.threshold)) &&
                 (button == edev->mouse.last_button))
               {
                  edev->mouse.did_triple = EINA_TRUE;
                  edev->mouse.prev = 0;
                  edev->mouse.last = 0;
                  current = 0;
               }
          }

        edev->mouse.last = edev->mouse.prev;
        edev->mouse.prev = current;
        edev->mouse.last_button = edev->mouse.prev_button;
        edev->mouse.prev_button = button;
     }

   ev->buttons = ((button & 0x00F) + 1);

   if (edev->mouse.did_double)
     ev->double_click = 1;
   if (edev->mouse.did_triple)
     ev->triple_click = 1;

   ecore_event_add(state, ev, NULL, NULL);
}

static void
_device_handle_touch_motion_send(Ecore_Drm_Evdev *edev, struct libinput_event_touch *event)
{
   Ecore_Drm_Input *input;
   Ecore_Event_Mouse_Move *ev;

   if (!edev) return;
   if (!(input = edev->seat->input)) return;

   if (!(ev = calloc(1, sizeof(Ecore_Event_Mouse_Move)))) return;

   ev->window = (Ecore_Window)input->dev->window;
   ev->event_window = (Ecore_Window)input->dev->window;
   ev->root_window = (Ecore_Window)input->dev->window;
   ev->timestamp = libinput_event_touch_get_time(event);
   ev->same_screen = 1;

   _device_modifiers_update(edev);
   ev->modifiers = edev->xkb.modifiers;

   ev->x = edev->seat->ptr.ix;
   ev->y = edev->seat->ptr.iy;
   ev->root.x = ev->x;
   ev->root.y = ev->y;

   ev->multi.device = edev->mt_slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = ev->x;
   ev->multi.y = ev->y;
   ev->multi.root.x = ev->x;
   ev->multi.root.y = ev->y;

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
}

static void
_device_handle_touch_down(struct libinput_device *device, struct libinput_event_touch *event)
{
   Ecore_Drm_Evdev *edev;

   if (!(edev = libinput_device_get_user_data(device))) return;

   edev->mouse.dx = edev->seat->ptr.ix = edev->seat->ptr.dx =
     libinput_event_touch_get_x_transformed(event, edev->output->current_mode->width);
   edev->mouse.dy = edev->seat->ptr.iy = edev->seat->ptr.dy =
     libinput_event_touch_get_y_transformed(event, edev->output->current_mode->height);

   edev->mt_slot = libinput_event_touch_get_seat_slot(event);

   _device_handle_touch_motion_send(edev, event);
   _device_handle_touch_event_send(edev, event, ECORE_EVENT_MOUSE_BUTTON_DOWN);
}

static void
_device_handle_touch_motion(struct libinput_device *device, struct libinput_event_touch *event)
{
   Ecore_Drm_Evdev *edev;

   if (!(edev = libinput_device_get_user_data(device))) return;

   edev->mouse.dx = edev->seat->ptr.dx =
     libinput_event_touch_get_x_transformed(event, edev->output->current_mode->width);
   edev->mouse.dy = edev->seat->ptr.dy =
     libinput_event_touch_get_y_transformed(event, edev->output->current_mode->height);

   if (floor(edev->seat->ptr.dx) == edev->seat->ptr.ix &&
       floor(edev->seat->ptr.dy) == edev->seat->ptr.iy) return;

   edev->seat->ptr.ix = edev->seat->ptr.dx;
   edev->seat->ptr.iy = edev->seat->ptr.dy;

   edev->mt_slot = libinput_event_touch_get_seat_slot(event);

   _device_handle_touch_motion_send(edev, event);
}

static void
_device_handle_touch_up(struct libinput_device *device, struct libinput_event_touch *event)
{
   Ecore_Drm_Evdev *edev;

   if (!(edev = libinput_device_get_user_data(device))) return;

   edev->mt_slot = libinput_event_touch_get_seat_slot(event);

   _device_handle_touch_event_send(edev, event, ECORE_EVENT_MOUSE_BUTTON_UP);
}

static void
_device_handle_touch_frame(struct libinput_device *device EINA_UNUSED, struct libinput_event_touch *event EINA_UNUSED)
{
   /* DBG("Unhandled Touch Frame Event"); */
}

void
_ecore_drm_evdev_device_destroy(Ecore_Drm_Evdev *edev)
{
   EINA_SAFETY_ON_NULL_RETURN(edev);

   if (edev->seat_caps & EVDEV_SEAT_KEYBOARD)
     {
        if (edev->xkb.state) xkb_state_unref(edev->xkb.state);
        if (edev->xkb.keymap) xkb_map_unref(edev->xkb.keymap);
     }

   if (edev->path) eina_stringshare_del(edev->path);
   if (edev->device) libinput_device_unref(edev->device);
   if (edev->key_remap_hash) eina_hash_free(edev->key_remap_hash);

   free(edev);
}

Eina_Bool
_ecore_drm_evdev_event_process(struct libinput_event *event)
{
   struct libinput_device *device;
   Eina_Bool ret = EINA_TRUE;

   device = libinput_event_get_device(event);
   switch (libinput_event_get_type(event))
     {
      case LIBINPUT_EVENT_KEYBOARD_KEY:
        _device_handle_key(device, libinput_event_get_keyboard_event(event));
        break;
      case LIBINPUT_EVENT_POINTER_MOTION:
        _device_handle_pointer_motion(device,
                                      libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
        _device_handle_pointer_motion_absolute(device,
                                               libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_POINTER_BUTTON:
        _device_handle_button(device, libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_POINTER_AXIS:
        _device_handle_axis(device, libinput_event_get_pointer_event(event));
        break;
      case LIBINPUT_EVENT_TOUCH_DOWN:
        _device_handle_touch_down(device, libinput_event_get_touch_event(event));
        break;
      case LIBINPUT_EVENT_TOUCH_MOTION:
        _device_handle_touch_motion(device,
                                    libinput_event_get_touch_event(event));
        break;
      case LIBINPUT_EVENT_TOUCH_UP:
        _device_handle_touch_up(device, libinput_event_get_touch_event(event));
        break;
      case LIBINPUT_EVENT_TOUCH_FRAME:
        _device_handle_touch_frame(device, libinput_event_get_touch_event(event));
        break;
      default:
        ret = EINA_FALSE;
        break;
     }

   return ret;
}

/**
 * @brief Set the axis size of the given device.
 *
 * @param edev The device to set the axis size to.
 * @param w The width of the axis.
 * @param h The height of the axis.
 *
 * This function sets set the width @p w and height @p h of the axis
 * of device @p dev. If @p dev is a relative input device, a width and
 * height must set for it. If its absolute set the ioctl correctly, if
 * not, unsupported device.
 */
EAPI void
ecore_drm_inputs_device_axis_size_set(Ecore_Drm_Evdev *edev, int w, int h)
{
   const char *sysname;
   float cal[6];
   const char *device;
   Eina_List *devices;
   const char *vals;
   enum libinput_config_status status;

   EINA_SAFETY_ON_NULL_RETURN(edev);
   EINA_SAFETY_ON_TRUE_RETURN((w == 0) || (h == 0));

   if ((!libinput_device_config_calibration_has_matrix(edev->device)) ||
       (libinput_device_config_calibration_get_default_matrix(edev->device, cal) != 0))
     return;

   sysname = libinput_device_get_sysname(edev->device);

   devices = eeze_udev_find_by_subsystem_sysname("input", sysname);
   if (eina_list_count(devices) < 1) return;

   EINA_LIST_FREE(devices, device)
     {
        vals = eeze_udev_syspath_get_property(device, "WL_CALIBRATION");
	if ((!vals) ||
            (sscanf(vals, "%f %f %f %f %f %f",
                    &cal[0], &cal[1], &cal[2], &cal[3], &cal[4], &cal[5]) != 6))
          goto cont;

        cal[2] /= w;
        cal[5] /= h;

        status =
          libinput_device_config_calibration_set_matrix(edev->device, cal);

        if (status != LIBINPUT_CONFIG_STATUS_SUCCESS)
          ERR("Failed to apply calibration");

cont:
        eina_stringshare_del(device);
        continue;
     }
}

/**
 * @brief Enable key remap functionality on the given device
 *
 * @param edev The Ecore_Drm_Evdev to enable the key remap on.
 * @param enable An Eina_Bool value to enable or disable the key remap on the device.
 * @return EINA_FALSE is returned if the Ecore_Drm_Evdev is not valid, or if no libinput device has been
 * assigned to it yet. EINA_TRUE will be returned if enabling key remap for this device succeeded.
 *
 * This function enables/disables key remap functionality with the given enable value.
 * If the given enable value is EINA_FALSE, the key remap functionality wil be disable and
 * the existing hash table for remapping keys will be freed.
 */
EAPI Eina_Bool
ecore_drm_evdev_key_remap_enable(Ecore_Drm_Evdev *edev, Eina_Bool enable)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(edev, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(edev->device, EINA_FALSE);

   edev->key_remap_enabled = enable;

   if (enable == EINA_FALSE && edev->key_remap_hash)
     {
        eina_hash_free(edev->key_remap_hash);
        edev->key_remap_hash = NULL;
     }

   return EINA_TRUE;
}

/**
 * @brief Set a set of keys as remapping keys on the given device.
 *
 * @param edev The Ecore_Drm_Evdev to set the remapping keys on
 * @param from_keys A set of keys which contains the original keycodes
 * @param to_keys A set of keys which contains the keycodes to be remapped
 * @param num The number of keys to be applied
 * @return EINA_FALSE is returned if the Ecore_Drm_Evdev is not valid, if no libinput device has been
 * assigned to it yet, if key remap is not enabled yet, or the some of the given parameters such as
 * from_keys, to_keys, num are not valid. EINA_TRUE will be returned if setting key remap for this device succeeded.
 *
 * This function will create a hash table of remapping keys as a member of the given device.
 * This hash table will be used in _device_remapped_key_get() later on.
 * Whenever a key event is coming from the the backend of ecore drm layer
 * the keycode of it can be replaced with the key found in the hash table.
 * If there is no key found, the coming keycode will be used.
 */
EAPI Eina_Bool
ecore_drm_evdev_key_remap_set(Ecore_Drm_Evdev *edev, int *from_keys, int *to_keys, int num)
{
   int i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(edev, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(edev->device, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(from_keys, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(to_keys, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(num <= 0, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!edev->key_remap_enabled, EINA_FALSE);

   if (!edev->key_remap_hash) edev->key_remap_hash = eina_hash_int32_new(NULL);

   if (!edev->key_remap_hash)
     {
        ERR("Failed to set remap key information : creating a hash is failed.");
        return EINA_FALSE;
     }

   for (i = 0; i < num ; i++)
     {
        if ((!from_keys[i]) || (!to_keys[i]))
          {
             ERR("Failed to set remap key information : given arguments are invalid.");
             return EINA_FALSE;
          }
     }

   for (i = 0; i < num ; i++)
     eina_hash_add(edev->key_remap_hash, &from_keys[i], (void *)(intptr_t)to_keys[i]);

   return EINA_TRUE;
}

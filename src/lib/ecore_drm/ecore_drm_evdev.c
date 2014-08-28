#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* copied from udev/extras/input_id/input_id.c */
/* we must use this kernel-compatible implementation */
#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define TEST_BIT(array, bit)    ((array[LONG(bit)] >> OFF(bit)) & 1)
/* end copied */

#include "ecore_drm_private.h"
#include <sys/ioctl.h>
#include <linux/input.h>
#include <ctype.h>

/* local functions */
static void 
_device_keyboard_setup(Ecore_Drm_Evdev *edev)
{
   Ecore_Drm_Input *input;

   if ((!edev) || (!edev->seat)) return;
   if (!(input = edev->seat->input)) return;
   if (!input->dev->xkb_ctx) return;

   /* create keymap from xkb context */
   edev->xkb.keymap = xkb_map_new_from_names(input->dev->xkb_ctx, NULL, 0);
   if (!edev->xkb.keymap)
     {
        ERR("Failed to create keymap: %m");
        return;
     }

   /* create xkb state */
   if (!(edev->xkb.state = xkb_state_new(edev->xkb.keymap)))
     {
        ERR("Failed to create xkb state: %m");
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

static Eina_Bool 
_device_configure(Ecore_Drm_Evdev *edev)
{
   Eina_Bool ret = EINA_FALSE;

   if (!edev) return EINA_FALSE;

   if ((edev->caps & (EVDEV_MOTION_ABS | EVDEV_MOTION_REL)) && 
       (edev->caps & EVDEV_BUTTON))
     {
        DBG("Input device %s is a pointer", edev->name);
        edev->seat_caps |= EVDEV_SEAT_POINTER;

        /* FIXME: make this configurable */
        edev->mouse.threshold = 0.25;

        ret = EINA_TRUE;
     }

   if (edev->caps & EVDEV_KEYBOARD)
     {
        DBG("Input device %s is a keyboard", edev->name);
        edev->seat_caps |= EVDEV_SEAT_KEYBOARD;
        _device_keyboard_setup(edev);
        ret = EINA_TRUE;
     }

   if (edev->caps & EVDEV_TOUCH)
     {
        DBG("Input device %s is a touchpad", edev->name);
        edev->seat_caps |= EVDEV_SEAT_TOUCH;
        ret = EINA_TRUE;
     }

   return ret;
}

static void
_device_axis_update(Ecore_Drm_Evdev *dev)
{
   if (!dev) return;

   if ((dev->abs.rel_w < 0) || (dev->abs.rel_h < 0))
     {
        int w = 0, h = 0;

        ecore_drm_output_size_get(dev->seat->input->dev, 
                                  dev->seat->input->dev->window, &w, &h);
        if ((w) && (h)) 
          ecore_drm_inputs_device_axis_size_set(dev, w, h);
     }
}

static Eina_Bool 
_device_handle(Ecore_Drm_Evdev *edev)
{
   struct input_absinfo absinfo;
   unsigned long dev_bits[NBITS(EV_MAX)];
   unsigned long abs_bits[NBITS(ABS_MAX)];
   unsigned long rel_bits[NBITS(REL_MAX)];
   unsigned long key_bits[NBITS(KEY_MAX)];
   /* Eina_Bool have_key = EINA_FALSE; */
   Eina_Bool have_abs = EINA_FALSE;

   if (!edev) return EINA_FALSE;

   ioctl(edev->fd, EVIOCGBIT(0, sizeof(dev_bits)), dev_bits);
   if (TEST_BIT(dev_bits, EV_ABS))
     {
        have_abs = EINA_TRUE;

        ioctl(edev->fd, EVIOCGBIT(EV_ABS, sizeof(abs_bits)), abs_bits);

        if ((TEST_BIT(abs_bits, ABS_WHEEL)) || 
            (TEST_BIT(abs_bits, ABS_GAS)) || 
            (TEST_BIT(abs_bits, ABS_BRAKE)) || 
            (TEST_BIT(abs_bits, ABS_HAT0X)))
          {
             /* ignore joystick */
             return EINA_FALSE;
          }

        if (TEST_BIT(abs_bits, ABS_X))
          {
             ioctl(edev->fd, EVIOCGABS(ABS_X), &absinfo);
             edev->abs.min_x = absinfo.minimum;
             edev->abs.max_x = absinfo.maximum;
             edev->abs.rel_w = -1;
             edev->mouse.x = -1;
             edev->caps |= EVDEV_MOTION_ABS;
          }

        if (TEST_BIT(abs_bits, ABS_Y))
          {
             ioctl(edev->fd, EVIOCGABS(ABS_Y), &absinfo);
             edev->abs.min_y = absinfo.minimum;
             edev->abs.max_y = absinfo.maximum;
             edev->abs.rel_h = -1;
             edev->mouse.y = -1;
             edev->caps |= EVDEV_MOTION_ABS;
          }

        if ((TEST_BIT(abs_bits, ABS_MT_POSITION_X)) && 
            (TEST_BIT(abs_bits, ABS_MT_POSITION_Y)))
          {
             DBG("Handle MultiTouch Device: %s", edev->path);
          }
     }

   if (TEST_BIT(dev_bits, EV_REL))
     {
        ioctl(edev->fd, EVIOCGBIT(EV_REL, sizeof(rel_bits)), rel_bits);

        if ((TEST_BIT(rel_bits, REL_X)) || (TEST_BIT(rel_bits, REL_Y)))
          edev->caps |= EVDEV_MOTION_REL;
     }

   if (TEST_BIT(dev_bits, EV_KEY))
     {
        unsigned int i = 0;

        /* have_key = EINA_TRUE; */

        ioctl(edev->fd, EVIOCGBIT(EV_KEY, sizeof(key_bits)), key_bits);

        if (have_abs)
          {
             if ((TEST_BIT(key_bits, BTN_TOOL_FINGER)) && 
                 (!TEST_BIT(key_bits, BTN_TOOL_PEN)))
               {
                  DBG("Device Is Touchpad: %s", edev->path);
               }
          }

        for (i = KEY_ESC; i < KEY_MAX; i++)
          {
             if ((i >= BTN_MISC) && (i < KEY_OK)) continue;
             if (TEST_BIT(key_bits, i))
               {
                  edev->caps |= EVDEV_KEYBOARD;
                  break;
               }
          }

        if (TEST_BIT(key_bits, BTN_TOUCH))
          edev->caps |= EVDEV_TOUCH;

        for (i = BTN_MISC; i < BTN_JOYSTICK; i++)
          {
             if (TEST_BIT(key_bits, i))
               {
                  edev->caps |= EVDEV_BUTTON;
                  edev->caps &= ~EVDEV_TOUCH;
                  break;
               }
          }
     }

   if (TEST_BIT(dev_bits, EV_LED)) edev->caps |= EVDEV_KEYBOARD;

   return EINA_TRUE;
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
_device_modifiers_update(Ecore_Drm_Evdev *edev)
{
   xkb_mod_mask_t mask;

   edev->xkb.modifiers = 0;

   edev->xkb.depressed = 
     xkb_state_serialize_mods(edev->xkb.state, XKB_STATE_DEPRESSED);
   edev->xkb.latched = 
     xkb_state_serialize_mods(edev->xkb.state, XKB_STATE_LATCHED);
   edev->xkb.locked = 
     xkb_state_serialize_mods(edev->xkb.state, XKB_STATE_LOCKED);
   edev->xkb.group = 
     xkb_state_serialize_mods(edev->xkb.state, XKB_STATE_EFFECTIVE);

   mask = (edev->xkb.depressed | edev->xkb.latched);

   if (mask & edev->xkb.ctrl_mask)
     edev->xkb.modifiers |= ECORE_EVENT_MODIFIER_CTRL;
   if (mask & edev->xkb.alt_mask)
     edev->xkb.modifiers |= ECORE_EVENT_MODIFIER_ALT;
   if (mask & edev->xkb.shift_mask)
     edev->xkb.modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
   if (mask & edev->xkb.win_mask)
     edev->xkb.modifiers |= ECORE_EVENT_MODIFIER_WIN;
   if (mask & edev->xkb.scroll_mask)
     edev->xkb.modifiers |= ECORE_EVENT_LOCK_SCROLL;
   if (mask & edev->xkb.num_mask)
     edev->xkb.modifiers |= ECORE_EVENT_LOCK_NUM;
   if (mask & edev->xkb.caps_mask)
     edev->xkb.modifiers |= ECORE_EVENT_LOCK_CAPS;
   if (mask & edev->xkb.altgr_mask)
     edev->xkb.modifiers |= ECORE_EVENT_MODIFIER_ALTGR;
}

static void 
_device_notify_key(Ecore_Drm_Evdev *dev, struct input_event *event, unsigned int timestamp)
{
   unsigned int code, nsyms;
   /* unsigned int *keycode; */
   const xkb_keysym_t *syms;
   xkb_keysym_t sym = XKB_KEY_NoSymbol;
   char key[256], keyname[256], compose_buffer[256];
   Ecore_Event_Key *e;
   Ecore_Drm_Input *input;
   char *tmp = NULL;
   char *compose = NULL;

   if (!(input = dev->seat->input)) return;

   /* xkb rules reflect X broken keycodes, so offset by 8 */
   code = event->code + 8;

   xkb_state_update_key(dev->xkb.state, code, 
                        (event->value ? XKB_KEY_DOWN : XKB_KEY_UP));

   /* get the keysym for this code */
   nsyms = xkb_key_get_syms(dev->xkb.state, code, &syms);
   if (nsyms == 1) sym = syms[0];

   /* get the keyname for this sym */
   memset(key, 0, sizeof(key));
   xkb_keysym_get_name(sym, key, sizeof(key));

   memset(keyname, 0, sizeof(keyname));
   memcpy(keyname, key, sizeof(keyname));

   if (keyname[0] == '\0')
     snprintf(keyname, sizeof(keyname), "Keycode-%u", code);

   /* if shift is active, we need to transform the key to lower */
   if (xkb_state_mod_index_is_active(dev->xkb.state, 
                                     xkb_map_mod_get_index(dev->xkb.keymap, 
                                                           XKB_MOD_NAME_SHIFT),
                                     XKB_STATE_MODS_EFFECTIVE))
     {
        if (keyname[0] != '\0')
          keyname[0] = tolower(keyname[0]);
     }

   memset(compose_buffer, 0, sizeof(compose_buffer));
   if (_device_keysym_translate(sym, dev->xkb.modifiers, compose_buffer, sizeof(compose_buffer)))
     {
        compose = eina_str_convert("ISO8859-1", "UTF-8",
                                   compose_buffer);
        if (!compose)
          {
             ERR("Ecore_DRM cannot convert input key string '%s' to UTF-8. "
                 "Is Eina built with iconv support?", compose_buffer);
          }
        else
          {
             tmp = compose;
          }
     }

   if (!compose)
     compose = compose_buffer;

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

   e->window = (Ecore_Window)input->dev->window;
   e->event_window = (Ecore_Window)input->dev->window;
   e->root_window = (Ecore_Window)input->dev->window;
   e->timestamp = timestamp;
   e->same_screen = 1;
   e->keycode = code;
   e->data = NULL;

   _device_modifiers_update(dev);
   e->modifiers = dev->xkb.modifiers;

   if (event->value)
     ecore_event_add(ECORE_EVENT_KEY_DOWN, e, NULL, NULL);
   else
     ecore_event_add(ECORE_EVENT_KEY_UP, e, NULL, NULL);

   if (tmp)
     free(tmp);
}

static void 
_device_notify_motion(Ecore_Drm_Evdev *dev, unsigned int timestamp)
{
   Ecore_Drm_Input *input;
   Ecore_Event_Mouse_Move *ev;

   if (!(input = dev->seat->input)) return;
   if (!(ev = calloc(1, sizeof(Ecore_Event_Mouse_Move)))) return;

   ev->window = (Ecore_Window)input->dev->window;
   ev->event_window = (Ecore_Window)input->dev->window;
   ev->root_window = (Ecore_Window)input->dev->window;
   ev->timestamp = timestamp;
   ev->same_screen = 1;

   /* NB: Commented out. This borks mouse movement if no key has been 
    * pressed yet due to 'state' not being set */
//   _device_modifiers_update(dev);
   ev->modifiers = dev->xkb.modifiers;

   ev->x = dev->mouse.x;
   ev->y = dev->mouse.y;
   ev->root.x = ev->x;
   ev->root.y = ev->y;

   ev->multi.device = dev->mt_slot;
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
_device_notify_wheel(Ecore_Drm_Evdev *dev, struct input_event *event, unsigned int timestamp)
{
   Ecore_Drm_Input *input;
   Ecore_Event_Mouse_Wheel *ev;

   if (!(input = dev->seat->input)) return;
   if (!(ev = calloc(1, sizeof(Ecore_Event_Mouse_Wheel)))) return;

   ev->window = (Ecore_Window)input->dev->window;
   ev->event_window = (Ecore_Window)input->dev->window;
   ev->root_window = (Ecore_Window)input->dev->window;
   ev->timestamp = timestamp;
   ev->same_screen = 1;

   /* NB: Commented out. This borks mouse wheel if no key has been 
    * pressed yet due to 'state' not being set */
//   _device_modifiers_update(dev);
   ev->modifiers = dev->xkb.modifiers;

   ev->x = dev->mouse.x;
   ev->y = dev->mouse.y;
   ev->root.x = ev->x;
   ev->root.y = ev->y;
   if (event->value == REL_HWHEEL) ev->direction = 1;
   ev->z = -event->value;

   ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
}

static void 
_device_notify_button(Ecore_Drm_Evdev *dev, struct input_event *event, unsigned int timestamp)
{
   Ecore_Drm_Input *input;
   Ecore_Event_Mouse_Button *ev;
   int button;

   if (!(input = dev->seat->input)) return;
   if (!(ev = calloc(1, sizeof(Ecore_Event_Mouse_Button)))) return;

   ev->window = (Ecore_Window)input->dev->window;
   ev->event_window = (Ecore_Window)input->dev->window;
   ev->root_window = (Ecore_Window)input->dev->window;
   ev->timestamp = timestamp;
   ev->same_screen = 1;

   /* NB: Commented out. This borks mouse button if no key has been 
    * pressed yet due to 'state' not being set */
//   _device_modifiers_update(dev);
   ev->modifiers = dev->xkb.modifiers;

   ev->x = dev->mouse.x;
   ev->y = dev->mouse.y;
   ev->root.x = ev->x;
   ev->root.y = ev->y;

   ev->multi.device = dev->mt_slot;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = ev->x;
   ev->multi.y = ev->y;
   ev->multi.root.x = ev->x;
   ev->multi.root.y = ev->y;

   button = ((event->code & 0x00F) + 1);

   /* swap buttons 2 & 3 so behaviour is like X */
   if (button == 3) button = 2;
   else if (button == 2) button = 3;

   if (event->value)
     {
        unsigned int current;

        current = timestamp;
        dev->mouse.did_double = EINA_FALSE;
        dev->mouse.did_triple = EINA_FALSE;

        if (((current - dev->mouse.prev) <= dev->mouse.threshold) &&
            (button == dev->mouse.prev_button))
          {
             dev->mouse.did_double = EINA_TRUE;
             if (((current - dev->mouse.last) <= (2 * dev->mouse.threshold)) && 
                 (button == dev->mouse.last_button))
               {
                  dev->mouse.did_triple = EINA_TRUE;
                  dev->mouse.prev = 0;
                  dev->mouse.last = 0;
                  current = 0;
               }
          }

        dev->mouse.last = dev->mouse.prev;
        dev->mouse.prev = current;
        dev->mouse.last_button = dev->mouse.prev_button;
        dev->mouse.prev_button = button;
     }

   ev->buttons = button;
   if (dev->mouse.did_double)
     ev->double_click = 1;
   if (dev->mouse.did_triple)
     ev->triple_click = 1;

   if (event->value)
     ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
   else
     ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
}

static void 
_device_process_flush(Ecore_Drm_Evdev *dev, unsigned int timestamp)
{
   switch (dev->pending_event)
     {
      case EVDEV_NONE:
        return;
      case EVDEV_RELATIVE_MOTION:
        _device_notify_motion(dev, timestamp);
        /* dev->mouse.x = 0; */
        /* dev->mouse.y = 0; */
        goto out;
        break;
      case EVDEV_ABSOLUTE_MT_DOWN:
        goto out;
        break;
      case EVDEV_ABSOLUTE_MT_MOTION:
        goto out;
        break;
      case EVDEV_ABSOLUTE_MT_UP:
        goto out;
        break;
      case EVDEV_ABSOLUTE_TOUCH_DOWN:
      case EVDEV_ABSOLUTE_TOUCH_UP:
          {
             struct input_event event;

             event.code = 0;
             event.value = dev->abs.pt[dev->mt_slot].down;

             dev->mouse.x = dev->abs.pt[dev->mt_slot].x[0];
             dev->mouse.y = dev->abs.pt[dev->mt_slot].y[0];

             _device_notify_motion(dev, timestamp);
             _device_notify_button(dev, &event, timestamp);

             break;
          }
      case EVDEV_ABSOLUTE_MOTION:
        dev->mouse.x = dev->abs.pt[dev->mt_slot].x[0];
        dev->mouse.y = dev->abs.pt[dev->mt_slot].y[0];

        _device_notify_motion(dev, timestamp);
        break;
     }

out:
   dev->pending_event = EVDEV_NONE;
}

static void 
_device_process_key(Ecore_Drm_Evdev *dev, struct input_event *event, unsigned int timestamp)
{
   /* ignore key repeat */
   if (event->value == 2) return;

   _device_process_flush(dev, timestamp);

   if ((event->code >= BTN_LEFT) && (event->code <= BTN_TASK))
     _device_notify_button(dev, event, timestamp);
   else if ((event->code >= KEY_ESC) && (event->code <= KEY_MICMUTE))
     _device_notify_key(dev, event, timestamp);
   else if ((event->code == BTN_TOUCH) && (dev->caps & EVDEV_MOTION_ABS))
     dev->abs.pt[dev->mt_slot].down = event->value;
}

static void 
_device_process_absolute(Ecore_Drm_Evdev *dev, struct input_event *event, unsigned int timestamp EINA_UNUSED)
{
   _device_axis_update(dev);

   switch (event->code)
     {
      case ABS_X:
        if (dev->abs.pt[dev->mt_slot].down == 0)
          return;
      case ABS_MT_POSITION_X:
        dev->abs.pt[dev->mt_slot].x[0] = 
          (int)((double)(event->value - dev->abs.min_x) / dev->abs.rel_w);

        if (dev->pending_event != EVDEV_ABSOLUTE_TOUCH_DOWN)
          if (dev->pending_event != EVDEV_ABSOLUTE_TOUCH_UP)
            dev->pending_event = EVDEV_ABSOLUTE_MOTION;

        break;
      case ABS_Y:
        if (dev->abs.pt[dev->mt_slot].down == 0)
          return;
      case ABS_MT_POSITION_Y:
        dev->abs.pt[dev->mt_slot].y[0] = 
          (int)((double)(event->value - dev->abs.min_y) / dev->abs.rel_h);

        if (dev->pending_event != EVDEV_ABSOLUTE_TOUCH_DOWN)
          if (dev->pending_event != EVDEV_ABSOLUTE_TOUCH_UP)
            dev->pending_event = EVDEV_ABSOLUTE_MOTION;

        break;
      case ABS_MT_SLOT:
        if ((event->value >= 0) && (event->value < EVDEV_MAX_SLOTS))
          dev->mt_slot = event->value;

        break;
      case ABS_MT_TRACKING_ID:
        if (event->value < 0)
          {
             dev->abs.pt[dev->mt_slot].down = 0;
             dev->pending_event = EVDEV_ABSOLUTE_TOUCH_UP;
          }
        else
          {
             dev->abs.pt[dev->mt_slot].down = 1;
             dev->pending_event = EVDEV_ABSOLUTE_TOUCH_DOWN;
          }
        break;
     }
}

static void
_device_process_relative(Ecore_Drm_Evdev *dev, struct input_event *event, unsigned int timestamp)
{
   switch (event->code)
     {
      case REL_X:
        if (dev->pending_event != EVDEV_RELATIVE_MOTION)
          _device_process_flush(dev, timestamp);
        dev->mouse.x += event->value;
        dev->pending_event = EVDEV_RELATIVE_MOTION;
        break;
      case REL_Y:
        if (dev->pending_event != EVDEV_RELATIVE_MOTION)
          _device_process_flush(dev, timestamp);
        dev->mouse.y += event->value;
        dev->pending_event = EVDEV_RELATIVE_MOTION;
        break;
      case REL_WHEEL:
      case REL_HWHEEL:
        _device_process_flush(dev, timestamp);
        _device_notify_wheel(dev, event, timestamp);
        break;
     }
}

static void 
_device_process(Ecore_Drm_Evdev *dev, struct input_event *event, int count)
{
   struct input_event *ev, *end;
   unsigned int timestamp = 0;

   ev = event;
   end = ev + count;
   for (ev = event; ev < end; ev++)
     {
        timestamp = (ev->time.tv_sec * 1000) + (ev->time.tv_usec / 1000);

        switch (ev->type)
          {
           case EV_KEY:
             _device_process_key(dev, ev, timestamp);
             break;
           case EV_REL:
             _device_process_relative(dev, ev, timestamp);
             break;
           case EV_ABS:
             _device_process_absolute(dev, ev, timestamp);
             break;
           case EV_SYN:
             _device_process_flush(dev, timestamp);
             break;
           default:
             break;
          }
     }
}

static Eina_Bool 
_cb_device_data(void *data, Ecore_Fd_Handler *hdlr EINA_UNUSED)
{
   Ecore_Drm_Evdev *edev;
   struct input_event ev[32];
   int len = 0;

   if (!(edev = data)) return EINA_TRUE;

   do
     {
        len = read(edev->fd, &ev, sizeof(ev));

        if ((len < 0) || ((len % sizeof(ev[0])) != 0))
          {
             if ((len < 0) && (errno != EAGAIN) && (errno != EINTR))
               {
                  ERR("Device Died");
               }

             return EINA_TRUE;
          }

        edev->event_process(edev, ev, (len / sizeof(ev[0])));

     } while (len > 0);

   return EINA_TRUE;
}

/* external functions */
Ecore_Drm_Evdev *
_ecore_drm_evdev_device_create(Ecore_Drm_Seat *seat, const char *path, int fd)
{
   Ecore_Drm_Evdev *edev;
   char name[256] = "unknown";

   if (!(edev = calloc(1, sizeof(Ecore_Drm_Evdev))))
     return NULL;

   edev->seat = seat;
   edev->path = eina_stringshare_add(path);
   edev->fd = fd;
   edev->mt_slot = 0;

   if (ioctl(edev->fd, EVIOCGNAME(sizeof(name)), name) < 0)
     ERR("Error getting device name: %m");

   name[sizeof(name) - 1] = '\0';
   edev->name = eina_stringshare_add(name);

   if (!_device_handle(edev))
     {
        ERR("Unhandled Input Device: %s", name);
        _ecore_drm_evdev_device_destroy(edev);
        return NULL;
     }

   if (!_device_configure(edev))
     {
        ERR("Could not configure input device: %s", name);
        _ecore_drm_evdev_device_destroy(edev);
        return NULL;
     }

   edev->event_process = _device_process;

   edev->hdlr = 
     ecore_main_fd_handler_add(edev->fd, ECORE_FD_READ, 
                               _cb_device_data, edev, NULL, NULL);
   if (!edev->hdlr)
     {
        ERR("Could not create fd handler");
        _ecore_drm_evdev_device_destroy(edev);
        return NULL;
     }

   return edev;
}

void 
_ecore_drm_evdev_device_destroy(Ecore_Drm_Evdev *dev)
{
   if (!dev) return;

   if (dev->xkb.state) xkb_state_unref(dev->xkb.state);
   if (dev->xkb.keymap) xkb_map_unref(dev->xkb.keymap);

   if (dev->path) eina_stringshare_del(dev->path);
   if (dev->name) eina_stringshare_del(dev->name);
   if (dev->hdlr) ecore_main_fd_handler_del(dev->hdlr);

   close(dev->fd);

   free(dev);
}


/**
 * @brief Set the axis size of the given device.
 *
 * @param dev The device to set the axis size to.
 * @param w The width of the axis.
 * @param h The height of the axis.
 *
 * This function sets set the width @p w and height @p h of the axis
 * of device @p dev. If @p dev is a relative input device, a width and
 * height must set for it. If its absolute set the ioctl correctly, if
 * not, unsupported device.
 */
EAPI void
ecore_drm_inputs_device_axis_size_set(Ecore_Drm_Evdev *dev, int w, int h)
{
   if (!dev) return;
   if ((w < 0) || (h < 0)) return;

   if (dev->caps & EVDEV_MOTION_ABS)
     {
        /* FIXME looks like some kernels dont include this struct */
        struct input_absinfo abs_features;

        ioctl(dev->fd, EVIOCGABS(ABS_X), &abs_features);
        dev->abs.rel_w = 
          (double)(abs_features.maximum - abs_features.minimum)/(double)(w);
        dev->abs.min_x = abs_features.minimum;

        ioctl(dev->fd, EVIOCGABS(ABS_Y), &abs_features);
        dev->abs.rel_h = 
          (double)(abs_features.maximum - abs_features.minimum)/(double)(h);
        dev->abs.min_y = abs_features.minimum;
     }
   else if (!(dev->caps & EVDEV_MOTION_REL))
     return;

   /* update the local values */
   if (dev->mouse.x > w - 1) dev->mouse.x = w -1;
   if (dev->mouse.y > h - 1) dev->mouse.y = h -1;
}

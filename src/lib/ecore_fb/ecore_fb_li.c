#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "Ecore_Fb.h"
#include "ecore_fb_private.h"

#define CLICK_THRESHOLD_DEFAULT 0.25

static Eina_List *_ecore_fb_li_devices = NULL;

static const char *_ecore_fb_li_kbd_syms[128 * 7] =
{
#include "ecore_fb_keytable.h"
};

/* Initial Copyright (C) Brad Hards (1999-2002),
 * this function is used to tell if "bit" is set in "array"
 * it selects a byte from the array, and does a boolean AND
 * operation with a byte that only has the relevant bit set.
 * eg. to check for the 12th bit, we do (array[1] & 1<<4).
 * Moved to static inline in order to force compiler to otimized
 * the unsued part away or force a link error if long has an unexpected
 * size.
 *                                                - bigeasy
 */
extern int long_has_neither_32_nor_64_bits(void);
static inline int
test_bit(int bit, unsigned long *array)
{
   if (sizeof(long) == 4)
      return array[bit / 32] & (1 << (bit % 32));
   else if (sizeof(long) == 8)
      return array[bit / 64] & (1 << (bit % 64));
   else long_has_neither_32_nor_64_bits();
}

static void
_ecore_fb_li_device_event_key(Ecore_Fb_Input_Device *dev, struct input_event *iev)
{
   if (!dev->listen) return;

   /* check for basic keyboard keys */
   if ((iev->code >= KEY_ESC) && (iev->code <= KEY_COMPOSE))
     {
        int offset = 0;
        const char *keyname = _ecore_fb_li_kbd_syms[iev->code * 7];
        
        /* check the key table */
        if (iev->value)
          {
             /* its a repeated key, dont increment */
             if (iev->value != 2)
               {
                  if (!strcmp(keyname, "Control_L"))
                    dev->keyboard.ctrl++;
                  else if (!strcmp(keyname, "Control_R"))
                    dev->keyboard.ctrl++;
                  else if (!strcmp(keyname, "Alt_L"))
                    dev->keyboard.alt++;
                  else if (!strcmp(keyname, "Alt_R"))
                    dev->keyboard.alt++;
                  else if (!strcmp(keyname, "Shift_L"))
                    dev->keyboard.shift++;
                  else if (!strcmp(keyname, "Shift_R"))
                    dev->keyboard.shift++;
                  else if (!strcmp(keyname, "Caps_Lock"))
                    dev->keyboard.lock = !dev->keyboard.lock;
                  if (dev->keyboard.ctrl > 2) dev->keyboard.ctrl = 2;
                  if (dev->keyboard.alt > 2) dev->keyboard.alt = 2;
                  if (dev->keyboard.shift > 2) dev->keyboard.shift = 2;
                  if (dev->keyboard.lock > 1) dev->keyboard.lock = 1;
               }
          }
        else
          {
             if (!strcmp(keyname, "Control_L"))
                dev->keyboard.ctrl--;
             else if (!strcmp(keyname, "Control_R"))
                dev->keyboard.ctrl--;
             else if (!strcmp(keyname, "Alt_L"))
                dev->keyboard.alt--;
             else if (!strcmp(keyname, "Alt_R"))
                dev->keyboard.alt--;
             else if (!strcmp(keyname, "Shift_L"))
                dev->keyboard.shift--;
             else if (!strcmp(keyname, "Shift_R"))
                dev->keyboard.shift--;
             if (dev->keyboard.ctrl < 0) dev->keyboard.ctrl = 0;
             if (dev->keyboard.alt < 0) dev->keyboard.alt = 0;
             if (dev->keyboard.shift < 0) dev->keyboard.shift = 0;
             if (dev->keyboard.lock < 0) dev->keyboard.lock = 0;
          }

        /* sending ecore_input_evas events */
        Ecore_Event_Key *e;

        if (dev->keyboard.shift) offset = 1;
        else if (dev->keyboard.lock) offset = 2;

        const char *key = _ecore_fb_li_kbd_syms[(iev->code * 7) + offset];
        const char *compose = _ecore_fb_li_kbd_syms[(iev->code * 7) + 3 + offset];
        
        if (dev->keyboard.ctrl)
          {
             const char *ts = _ecore_fb_li_kbd_syms[(iev->code * 7) + 3 + 3];
             
             if (ts) compose = ts;
          }

        e = calloc(1, sizeof(Ecore_Event_Key) + strlen(key) +
                   strlen(keyname) + (compose ? strlen(compose) : 0) + 3);
        e->keyname = (char *)(e + 1);
        e->key = e->keyname + strlen(keyname) + 1;
        e->compose = (compose) ? e->key + strlen(key) + 1 : NULL;
        e->string = e->compose;

        strcpy((char *)e->keyname, keyname);
        strcpy((char *)e->key, key);
        if (compose)
          strcpy((char *)e->compose, compose);

        e->modifiers = 0;
        if (dev->keyboard.shift) e->modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
        if (dev->keyboard.ctrl) e->modifiers |= ECORE_EVENT_MODIFIER_CTRL;
        if (dev->keyboard.alt) e->modifiers |= ECORE_EVENT_MODIFIER_ALT;
        if (dev->keyboard.lock) e->modifiers |= ECORE_EVENT_LOCK_CAPS;

        e->timestamp = ecore_loop_time_get() * 1000.0;
        e->window = (Ecore_Window)dev->window;
        e->event_window = (Ecore_Window)dev->window;
        e->root_window = (Ecore_Window)dev->window;
        e->same_screen = 1;

        if (iev->value)
          ecore_event_add(ECORE_EVENT_KEY_DOWN, e, NULL, NULL);
        else
          ecore_event_add(ECORE_EVENT_KEY_UP, e, NULL, NULL);
     }
   /* check for mouse button events */
   else if ((iev->code >= BTN_MOUSE) && (iev->code < BTN_JOYSTICK))
     {
        int button;
        Ecore_Event_Mouse_Button *e;
        double current = ecore_loop_time_get();

        button = ((iev->code & 0x00F) + 1);
        // swap 2 and 3 to make middle and right butotn work right.
        if (button == 3) button = 2;
        else if (button == 2) button = 3;
        if (iev->value)
          {
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
                       /* reset */
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

        e = calloc(1, sizeof(Ecore_Event_Mouse_Button));
        if (!e)
          return;

        e->timestamp = current * 1000.0;
        e->window = (Ecore_Window)dev->window;
        e->event_window = (Ecore_Window)dev->window;
        e->root_window = (Ecore_Window)dev->window;
        e->same_screen = 1;

        e->modifiers = 0;
        if (dev->keyboard.shift) e->modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
        if (dev->keyboard.ctrl) e->modifiers |= ECORE_EVENT_MODIFIER_CTRL;
        if (dev->keyboard.alt) e->modifiers |= ECORE_EVENT_MODIFIER_ALT;
        if (dev->keyboard.lock) e->modifiers |= ECORE_EVENT_LOCK_CAPS;

        e->x = dev->mouse.x;
        e->y = dev->mouse.y;
        e->root.x = e->x;
        e->root.y = e->y;
        e->buttons = button;

        if (dev->mouse.did_double)
          e->double_click = 1;
        if (dev->mouse.did_triple)
          e->triple_click = 1;

        if (iev->value)
          ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, e, NULL, NULL);
        else
          ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, e, NULL, NULL);
     }
}

static void
_ecore_fb_li_device_event_rel(Ecore_Fb_Input_Device *dev, struct input_event *iev)
{
   if (!dev->listen) return;
   /* dispatch the button events if they are queued */
   switch (iev->code)
     {
     case REL_X:
     case REL_Y:
          {
             Ecore_Event_Mouse_Move *e;
             if (iev->code == REL_X)
               {
                  dev->mouse.x += iev->value;
                  if (dev->mouse.x > dev->mouse.w - 1)
                    dev->mouse.x = dev->mouse.w;
                  else if(dev->mouse.x < 0)
                    dev->mouse.x = 0;
               }
             else
               {
                  dev->mouse.y += iev->value;
                  if (dev->mouse.y > dev->mouse.h - 1)
                    dev->mouse.y = dev->mouse.h;
                  else if(dev->mouse.y < 0)
                    dev->mouse.y = 0;
               }

             e = calloc(1, sizeof(Ecore_Event_Mouse_Move));
             if (!e)
               return;

             e->window = (Ecore_Window)dev->window;
             e->event_window = (Ecore_Window)dev->window;
             e->root_window = (Ecore_Window)dev->window;
             e->same_screen = 1;

             e->modifiers = 0;
             if (dev->keyboard.shift) e->modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
             if (dev->keyboard.ctrl) e->modifiers |= ECORE_EVENT_MODIFIER_CTRL;
             if (dev->keyboard.alt) e->modifiers |= ECORE_EVENT_MODIFIER_ALT;
             if (dev->keyboard.lock) e->modifiers |= ECORE_EVENT_LOCK_CAPS;

             e->x = dev->mouse.x;
             e->y = dev->mouse.y;
             e->root.x = e->x;
             e->root.y = e->y;

             e->timestamp = ecore_loop_time_get() * 1000.0;

             ecore_event_add(ECORE_EVENT_MOUSE_MOVE, e, NULL, NULL);

             break;
          }
     case REL_WHEEL:
     case REL_HWHEEL:
          {
             Ecore_Event_Mouse_Wheel *e;

             e = calloc(1, sizeof(Ecore_Event_Mouse_Wheel));
             if (!e)
               return;

             e->x = dev->mouse.x;
             e->y = dev->mouse.y;
             if (iev->code == REL_HWHEEL) e->direction = 1;
             e->z = -iev->value;
             e->root.x = dev->mouse.x;
             e->root.y = dev->mouse.y;

             e->window = (Ecore_Window)dev->window;
             e->event_window = (Ecore_Window)dev->window;
             e->root_window = (Ecore_Window)dev->window;
             e->same_screen = 1;

             e->modifiers = 0;
             if (dev->keyboard.shift) e->modifiers |= ECORE_EVENT_MODIFIER_SHIFT;
             if (dev->keyboard.ctrl) e->modifiers |= ECORE_EVENT_MODIFIER_CTRL;
             if (dev->keyboard.alt) e->modifiers |= ECORE_EVENT_MODIFIER_ALT;
             if (dev->keyboard.lock) e->modifiers |= ECORE_EVENT_LOCK_CAPS;

             e->timestamp = ecore_loop_time_get() * 1000.0;

             ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, e, NULL, NULL);

             break;
          }
     default:
        break;
     }
}

static void
_ecore_fb_li_device_event_abs(Ecore_Fb_Input_Device *dev, struct input_event *iev)
{
   static int prev_pressure = 0;
   int pressure;

   if (!dev->listen) return;
   switch (iev->code)
     {
     case ABS_X:
        if (dev->mouse.w != 0)
          {
             int tmp;

             tmp = (int)((double)(iev->value - dev->mouse.min_w) / dev->mouse.rel_w);
             if (tmp < 0) dev->mouse.x = 0;
             else if (tmp > dev->mouse.w) dev->mouse.x = dev->mouse.w;
             else dev->mouse.x = tmp;
             dev->mouse.event = ECORE_EVENT_MOUSE_MOVE;
          }
        break;

     case ABS_Y:
        if (dev->mouse.h != 0)
          {
             int tmp;

             tmp = (int)((double)(iev->value - dev->mouse.min_h) / dev->mouse.rel_h);
             if (tmp < 0) dev->mouse.y = 0;
             else if (tmp > dev->mouse.h) dev->mouse.y = dev->mouse.h;
             else dev->mouse.y = tmp;
             dev->mouse.event = ECORE_EVENT_MOUSE_MOVE;
          }
        break;

     case ABS_PRESSURE:
        pressure = iev->value;
        if ((pressure) && (!prev_pressure))
          {
             /* DOWN: mouse is down, but was not before */
             dev->mouse.event = ECORE_EVENT_MOUSE_BUTTON_DOWN;
          }
        else if ((!pressure) && (prev_pressure))
          {
             /* UP: mouse was down, but is not now */
             dev->mouse.event = ECORE_EVENT_MOUSE_BUTTON_UP;
          }
        prev_pressure = pressure;
        break;
     }
}

static void
_ecore_fb_li_device_event_syn(Ecore_Fb_Input_Device *dev, struct input_event *iev EINA_UNUSED)
{
   if (!dev->listen) return;

   if (dev->mouse.event == ECORE_EVENT_MOUSE_MOVE)
     {
        Ecore_Event_Mouse_Move *ev;
        ev = calloc(1,sizeof(Ecore_Event_Mouse_Move));
        ev->x = dev->mouse.x;
        ev->y = dev->mouse.y;
        ev->root.x = ev->x;
        ev->root.y = ev->y;
        ev->timestamp = ecore_loop_time_get() * 1000.0;
        ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
     }
   else if (dev->mouse.event == ECORE_EVENT_MOUSE_BUTTON_DOWN)
     {
        Ecore_Event_Mouse_Button *ev;
        ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
        ev->x = dev->mouse.x;
        ev->y = dev->mouse.y;
        ev->root.x = ev->x;
        ev->root.y = ev->y;
        ev->buttons = 1;
        ev->timestamp = ecore_loop_time_get() * 1000.0;
        ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
     }
   else if (dev->mouse.event == ECORE_EVENT_MOUSE_BUTTON_UP)
     {
        Ecore_Event_Mouse_Button *ev;
        ev = calloc(1, sizeof(Ecore_Event_Mouse_Button));
        ev->x = dev->mouse.x;
        ev->y = dev->mouse.y;
        ev->root.x = ev->x;
        ev->root.y = ev->y;
        ev->buttons = 1;
        ev->timestamp = ecore_loop_time_get() * 1000.0;
        ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
     }
}

static Eina_Bool
_ecore_fb_li_device_fd_callback(void *data, Ecore_Fd_Handler *fdh EINA_UNUSED)
{
   Ecore_Fb_Input_Device *dev;
   struct input_event ev[64];
   int len;
   int i;

   dev = (Ecore_Fb_Input_Device*)data;
   /* read up to 64 events at once */
   len = read(dev->fd, &ev, sizeof(ev));
   for(i = 0; i < (int)(len / sizeof(ev[0])); i++)
     {
        switch(ev[i].type)
          {
          case EV_SYN:
             _ecore_fb_li_device_event_syn(dev, &ev[i]);
             break;
          case EV_ABS:
             _ecore_fb_li_device_event_abs(dev, &ev[i]);
             break;
          case EV_REL:
             _ecore_fb_li_device_event_rel(dev, &ev[i]);
             break;
          case EV_KEY:
             _ecore_fb_li_device_event_key(dev, &ev[i]);
             break;
          default:
             break;
          }
     }
   return EINA_TRUE;
}

/**
 * @addtogroup Ecore_FB_Group Ecore_FB - Frame buffer convenience functions.
 *
 * @{
 */

/**
 * @brief Set the listen mode for an input device .
 *
 * @param dev The device to set the mode of.
 * @param listen @c EINA_FALSE to disable listening mode, @c EINA_TRUE to
 * enable it.
 *
 * This function enables or disables listening on the input device @p dev.
 * If @p listen is @c EINA_FALSE, listening mode is disabled, if it is
 * @c EINA_TRUE, it is enabled.
 */
EAPI void
ecore_fb_input_device_listen(Ecore_Fb_Input_Device *dev, Eina_Bool listen)
{
   if (!dev) return;
   if ((listen && dev->listen) || (!listen && !dev->listen)) return;
   if (listen)
     {
        /* if the device already had a handler */
        if (!dev->handler)
           dev->handler = ecore_main_fd_handler_add(dev->fd, ECORE_FD_READ, _ecore_fb_li_device_fd_callback, dev, NULL, NULL);

     }
   dev->listen = listen;
}

#ifndef EV_CNT
# define EV_CNT (EV_MAX+1)
#endif

/**
 * @brief Associates an input device with the given @ref Ecore_Evas_Group.
 *
 * @param dev The input being associated with an @ref Ecore_Evas_Group (not @c NULL).
 * @param window The window which this input is being associated to.
 * @c NULL will remove any previous association.
 *
 * Events generated by this device will have a pointer to @p window. If this @p
 * window is registered with ecore_event_window_register() or
 * ecore_evas_input_event_register(), respective evas events will be delivered
 * by the ecore_input_evas system. An example can be seen in the following code:
 *
 * @code
 * Ecore_Evas *ee = ecore_evas_new(NULL, 0, 0, 800, 600, NULL);
 *
 * ecore_evas_input_event_register(ee);
 *
 * device = ecore_fb_input_device_open(device_path);
 * if (device)
 *   ecore_fb_input_device_window_set(device, ee);
 *
 * @endcode
 *
 * On the previous code, all input captured on the mentioned device will be
 * delivered to the @c Ecore_Evas @c ee.
 *
 * @since 1.1
 */
EAPI void
ecore_fb_input_device_window_set(Ecore_Fb_Input_Device *dev, void *window)
{
   if (!dev) return;

   dev->window = window;
}

/**
 * @brief Open an input device.
 *
 * @param dev The device to open.
 * @return The @ref Ecore_Fb_Input_Device object that has been opened.
 *
 * This function opens the input device named @p dev and returns the
 * object for it, or returns @c NULL on failure.
 */
EAPI Ecore_Fb_Input_Device *
ecore_fb_input_device_open(const char *dev)
{
   Ecore_Fb_Input_Device *device;
   unsigned long event_type_bitmask[EV_CNT / 32 + 1];
   int event_type;
   int fd;

   if (!dev) return NULL;
   device = calloc(1, sizeof(Ecore_Fb_Input_Device));
   if (!device) return NULL;

   if ((fd = open(dev, O_RDONLY, O_NONBLOCK)) < 0)
     {
        fprintf(stderr, "[ecore_fb_li:device_open] %s %s", dev, strerror(errno));
        goto error_open;
     }
   /* query capabilities */
   if (ioctl(fd, EVIOCGBIT(0, EV_MAX), event_type_bitmask) < 0)
     {
        fprintf(stderr,"[ecore_fb_li:device_open] query capabilities %s %s", dev, strerror(errno));
        goto error_caps;
     }
   /* query name */
   device->info.name = calloc(256, sizeof(char));
   if (ioctl(fd, EVIOCGNAME(sizeof(char) * 256), device->info.name) < 0)
     {
        fprintf(stderr, "[ecore_fb_li:device_open] get name %s %s", dev, strerror(errno));
        strcpy(device->info.name, "Unknown");
     }
   device->fd = fd;
   device->info.dev = strdup(dev);
   /* common */
   device->mouse.threshold = CLICK_THRESHOLD_DEFAULT;

   /* set info */
   for (event_type = 0; event_type < EV_MAX; event_type++)
     {
        if (!test_bit(event_type, event_type_bitmask))
          continue;
        switch (event_type)
          {
          case EV_SYN:
             break;
          case EV_KEY:
             device->info.cap |= ECORE_FB_INPUT_DEVICE_CAP_KEYS_OR_BUTTONS;
             break;
          case EV_REL:
             device->info.cap |= ECORE_FB_INPUT_DEVICE_CAP_RELATIVE;
             break;
          case EV_ABS:
             device->info.cap |= ECORE_FB_INPUT_DEVICE_CAP_ABSOLUTE;
             break;
          case EV_MSC:
          case EV_LED:
          case EV_SND:
          case EV_REP:
          case EV_FF :
          case EV_FF_STATUS:
          case EV_PWR:
          default:
                break;
          }
     }

   _ecore_fb_li_devices = eina_list_append(_ecore_fb_li_devices, device);
   return device;

error_caps:
   close(fd);
error_open:
   free(device);
   return NULL;
}

/**
 * @brief Close the given device.
 *
 * @param dev The device to close
 *
 * This function closes the device @p dev. If @p dev is @c NULL, this
 * function does nothing.
 */
EAPI void
ecore_fb_input_device_close(Ecore_Fb_Input_Device *dev)
{
   if (!dev || dev->fd < 0) return;
   /* close the fd */
   close(dev->fd);
   /* remove the element from the list */
   _ecore_fb_li_devices = eina_list_remove(_ecore_fb_li_devices, dev);
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
ecore_fb_input_device_axis_size_set(Ecore_Fb_Input_Device *dev, int w, int h)
{
   if (!dev) return;
   if ((w < 0) || (h < 0)) return;
   /* FIXME
    * this code is for a touchscreen device,
    * make it configurable (ABSOLUTE | RELATIVE)
    */
   if (dev->info.cap & ECORE_FB_INPUT_DEVICE_CAP_ABSOLUTE)
     {
        /* FIXME looks like some kernels dont include this struct */
        struct input_absinfo abs_features;

        ioctl(dev->fd, EVIOCGABS(ABS_X), &abs_features);
        dev->mouse.min_w = abs_features.minimum;
        dev->mouse.rel_w = (double)(abs_features.maximum - abs_features.minimum)/(double)(w);

        ioctl(dev->fd, EVIOCGABS(ABS_Y), &abs_features);
        dev->mouse.min_h = abs_features.minimum;
        dev->mouse.rel_h = (double)(abs_features.maximum - abs_features.minimum)/(double)(h);
     }
   else if (!(dev->info.cap & ECORE_FB_INPUT_DEVICE_CAP_RELATIVE))
      return;

   /* update the local values */
   if (dev->mouse.x > w - 1) dev->mouse.x = w -1;
   if (dev->mouse.y > h - 1) dev->mouse.y = h -1;
   dev->mouse.w = w;
   dev->mouse.h = h;
}

/**
 * @brief Retrieve the name of the given device.
 *
 * @param dev The device to get the name from.
 * @return The name of the device.
 *
 * This function returns the name of the device @p dev. If @p dev is
 * @c NULL, this function returns @c NULL.
 */
EAPI const char *
ecore_fb_input_device_name_get(Ecore_Fb_Input_Device *dev)
{
   if (!dev) return NULL;
   return dev->info.name;
}

/**
 * @brief Retrieve the capability of the given device.
 *
 * @param dev The device to get the name from.
 * @return The capability of the device.
 *
 * This function returns the capability of the device @p dev. If @p dev is
 * @c NULL, this function returns ECORE_FB_INPUT_DEVICE_CAP_NONE.
 */
EAPI Ecore_Fb_Input_Device_Cap
ecore_fb_input_device_cap_get(Ecore_Fb_Input_Device *dev)
{
   if (!dev) return ECORE_FB_INPUT_DEVICE_CAP_NONE;
   return dev->info.cap;
}

/**
 * @brief Set the threshold of mouse clicks of the given device.
 *
 * @param dev The device to set the threshodl mouse click to.
 * @param threshold The threshold value.
 *
 * This function sets the threshold of mouse clicks of the device
 * @p dev to @p threshold. If @p dev is @c NULL, this function does
 * nothing.
 */
EAPI void
ecore_fb_input_device_threshold_click_set(Ecore_Fb_Input_Device *dev, double threshold)
{
   if (!dev) return;
   if ((threshold == dev->mouse.threshold) || (threshold == 0)) return;
   dev->mouse.threshold = threshold;
}

/**
 * @brief Get the threshold of mouse clicks of the given device.
 *
 * @param dev The device to set the threshodl mouse click from.
 * @return The threshold value.
 *
 * This function returns the threshold of mouse clicks of the device
 * @p dev. If @p dev is @c NULL, this function returns 0.0.
 */
EAPI double
ecore_fb_input_device_threshold_click_get(Ecore_Fb_Input_Device *dev)
{
   if (!dev) return 0;
   return dev->mouse.threshold;
}

/**
 * @}
 */

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

/* local functions */
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
        ret = EINA_TRUE;
     }

   if (edev->caps & EVDEV_KEYBOARD)
     {
        DBG("Input device %s is a keyboard", edev->name);
        edev->seat_caps |= EVDEV_SEAT_KEYBOARD;
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
             edev->caps |= EVDEV_MOTION_ABS;
          }

        if (TEST_BIT(abs_bits, ABS_Y))
          {
             ioctl(edev->fd, EVIOCGABS(ABS_Y), &absinfo);
             edev->abs.min_y = absinfo.minimum;
             edev->abs.max_y = absinfo.maximum;
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

static void 
_device_notify_key(Ecore_Drm_Evdev *dev, struct input_event *event, unsigned int timestamp)
{
   DBG("Key Event");
   DBG("\tCode: %d", event->code);
   DBG("\tValue: %d", event->value);

   if ((event->code >= KEY_ESC) && (event->code <= KEY_COMPOSE))
     {
        /* ignore key repeat */
        if (event->value == 2)
          {
             DBG("\tKey Repeat");
          }
     }
}

static void 
_device_process_flush(Ecore_Drm_Evdev *dev, unsigned int timestamp)
{
   switch (dev->pending_event)
     {
      case EVDEV_NONE:
        return;
      case EVDEV_RELATIVE_MOTION:
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
        goto out;
        break;
      case EVDEV_ABSOLUTE_MOTION:
        goto out;
        break;
      case EVDEV_ABSOLUTE_TOUCH_UP:
        goto out;
        break;
     }

out:
   dev->pending_event = EVDEV_NONE;
}

static void 
_device_process_key(Ecore_Drm_Evdev *dev, struct input_event *event, unsigned int timestamp)
{
   if (event->code == BTN_TOUCH)
     {
        /* TODO: check for mt device */
        return;
     }

   _device_process_flush(dev, timestamp);

   switch (event->code)
     {
      case BTN_LEFT:
      case BTN_RIGHT:
      case BTN_MIDDLE:
      case BTN_SIDE:
      case BTN_EXTRA:
      case BTN_FORWARD:
      case BTN_BACK:
      case BTN_TASK:
        /* TODO: notify button */
        break;
      default:
        _device_notify_key(dev, event, timestamp);
        break;
     }
}

static void 
_device_process(Ecore_Drm_Evdev *dev, struct input_event *event, int count)
{
   struct input_event *ev, *end;
   unsigned int timestamp = 0;

   DBG("Evdev Device Process");

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
             DBG("\tRelative Event");
             break;
           case EV_ABS:
             DBG("\tAbsolute Event");
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

   if (ioctl(edev->fd, EVIOCGNAME(sizeof(name)), name) < 0)
     DBG("Error getting device name: %m");

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

   if (dev->path) eina_stringshare_del(dev->path);
   if (dev->name) eina_stringshare_del(dev->name);
   if (dev->hdlr) ecore_main_fd_handler_del(dev->hdlr);

   free(dev);
}

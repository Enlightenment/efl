#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_drm_private.h"

Ecore_Drm_Evdev *
_ecore_drm_evdev_device_create(struct libinput *linput, struct libinput_device *ldevice)
{
   Ecore_Drm_Evdev *evdev;
   struct libinput_seat *lseat;

   if (!(evdev = calloc(1, sizeof(Ecore_Drm_Evdev))))
     return NULL;

   lseat = libinput_device_get_seat(ldevice);

   evdev->linput = linput;
   evdev->seat = libinput_seat_get_user_data(lseat);
   evdev->dev = ldevice;

   libinput_device_set_user_data(ldevice, evdev);
   libinput_device_ref(ldevice);

   return evdev;
}

void 
_ecore_drm_evdev_device_destroy(Ecore_Drm_Evdev *evdev)
{
   if (!evdev) return;
   libinput_device_unref(evdev->dev);
   if (evdev->name) eina_stringshare_del(evdev->name);
   free(evdev);
}

int 
_ecore_drm_evdev_event_process(struct libinput_event *event)
{
   /* struct libinput_device *device; */
   int ret = 1;

   /* device = libinput_event_get_target(event).device; */
   switch (libinput_event_get_type(event))
     {
      case LIBINPUT_EVENT_DEVICE_REGISTER_CAPABILITY:
        DBG("Evdev Register Capability");
        break;
      case LIBINPUT_EVENT_DEVICE_UNREGISTER_CAPABILITY:
        DBG("Evdev Unregister Capability");
        break;
      case LIBINPUT_EVENT_KEYBOARD_KEY:
        DBG("Evdev Event Key");
        break;
      case LIBINPUT_EVENT_POINTER_MOTION:
        DBG("Evdev Event Pointer Motion");
        break;
      case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
        DBG("Evdev Event Pointer Motion Absolute");
        break;
      case LIBINPUT_EVENT_POINTER_BUTTON:
        DBG("Evdev Event Pointer Button");
        break;
      case LIBINPUT_EVENT_POINTER_AXIS:
        DBG("Evdev Event Pointer Axis");
        break;
      case LIBINPUT_EVENT_TOUCH_TOUCH:
        break;
      default:
        ret = 0;
        break;
     }

   return ret;
}

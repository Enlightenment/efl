#include <elput_private.h>

#define ACCESSOR(STRUCT_NAME, FIELD_NAME, TYPE, FALLBACK) \
EAPI TYPE \
elput_##STRUCT_NAME##_##FIELD_NAME##_get(Elput_Swipe_Gesture *gesture) \
{ \
   EINA_SAFETY_ON_NULL_RETURN_VAL(gesture, FALLBACK); \
   return gesture->FIELD_NAME; \
}

ACCESSOR(swipe, dy, double, 0.0f)
ACCESSOR(swipe, dx, double, 0.0f)
ACCESSOR(swipe, finger_count, int, 0)
ACCESSOR(swipe, window, int, 0)
ACCESSOR(swipe, device, Elput_Device*, NULL)

EAPI Elput_Manager*
elput_manager_connect_gestures(const char *seat, unsigned int tty)
{
   Elput_Manager *em;

   em = elput_manager_connect(seat, tty);
   if (em)
     em->only_gesture_events = EINA_TRUE;

   return em;
}

static void
_free_and_unref_device(void *userdata EINA_UNUSED, void *data)
{
   Elput_Swipe_Gesture *gesture;

   gesture = data;
   libinput_device_unref(gesture->device->device);
   free(gesture);
}

static void
_eval_callback(int event, struct libinput_device *device, struct libinput_event_gesture *gesture)
{
   Elput_Device *dev;
   Elput_Swipe_Gesture *elput_gesture;

   dev = libinput_device_get_user_data(device);
   elput_gesture = calloc(1, sizeof(Elput_Swipe_Gesture));

   elput_gesture->dx = libinput_event_gesture_get_dx(gesture);
   elput_gesture->dy = libinput_event_gesture_get_dy(gesture);
   elput_gesture->finger_count =
     libinput_event_gesture_get_finger_count(gesture);
   elput_gesture->window = dev->seat->manager->window;
   elput_gesture->device = dev;

   libinput_device_ref(device);
   ecore_event_add(event, elput_gesture, _free_and_unref_device, NULL);
}

int
_gesture_event_process(struct libinput_event *event)
{
   struct libinput_device *dev;
   int ret = 1;

   dev = libinput_event_get_device(event);

   switch (libinput_event_get_type(event))
     {
      case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
        _eval_callback(ELPUT_EVENT_SWIPE_BEGIN, dev,
                       libinput_event_get_gesture_event(event));
        break;
      case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
        _eval_callback(ELPUT_EVENT_SWIPE_UPDATE, dev,
                       libinput_event_get_gesture_event(event));
        break;
      case LIBINPUT_EVENT_GESTURE_SWIPE_END:
        _eval_callback(ELPUT_EVENT_SWIPE_END, dev,
                       libinput_event_get_gesture_event(event));
        break;
      default:
        ret = 0;
        break;
     }

   return ret;
}

#include <elput_private.h>


EAPI double
elput_swipe_dx_get(Elput_Swipe_Gesture *gesture)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(gesture, 0.0f);
   return gesture->dx;
}

EAPI double
elput_swipe_dy_get(Elput_Swipe_Gesture *gesture)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(gesture, 0.0f);
   return gesture->dy;
}

EAPI int
elput_swipe_finger_count_get(Elput_Swipe_Gesture *gesture)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(gesture, 0);
   return gesture->finger_count;
}

EAPI void
elput_manager_swipe_gesture_listen(Elput_Manager *em,
                                   Elput_Swipe_Gesture_Callback begin, void *begin_data,
                                   Elput_Swipe_Gesture_Callback update, void *update_data,
                                   Elput_Swipe_Gesture_Callback end, void *end_data)
{
   EINA_SAFETY_ON_NULL_RETURN(em);
   em->swipe_callback.begin.cb = begin;
   em->swipe_callback.begin.data = begin_data;
   em->swipe_callback.end.cb = end;
   em->swipe_callback.end.data = end_data;
   em->swipe_callback.update.cb = update;
   em->swipe_callback.update.data = update_data;
}

EAPI Elput_Manager*
elput_manager_connect_gestures(const char *seat, unsigned int tty)
{
   Elput_Manager *em = elput_manager_connect(seat, tty);

   if (em)
     {
        em->only_gesture_events = EINA_TRUE;
     }

   return em;
}

static void
_eval_callback(Elput_Gesture_Swipe_Callback *callback, struct libinput_device *device, struct libinput_event_gesture *gesture)
{
   Elput_Device *dev;
   Elput_Swipe_Gesture elput_gesture = {
      libinput_event_gesture_get_dx(gesture),
      libinput_event_gesture_get_dy(gesture),
      libinput_event_gesture_get_finger_count(gesture),
   };

   if (!callback->cb) return;

   dev = libinput_device_get_user_data(device);

   callback->cb(callback->data, dev, &elput_gesture);
}

int
_gesture_event_process(struct libinput_event *event)
{
   Elput_Manager *em;
   struct libinput *lib;
   struct libinput_device *dev;
   int ret = 1;

   lib = libinput_event_get_context(event);
   dev = libinput_event_get_device(event);
   em = libinput_get_user_data(lib);

   switch (libinput_event_get_type(event))
     {
      case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
        _eval_callback(&em->swipe_callback.begin, dev, libinput_event_get_gesture_event(event));
        break;
      case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
        _eval_callback(&em->swipe_callback.update, dev, libinput_event_get_gesture_event(event));
        break;
      case LIBINPUT_EVENT_GESTURE_SWIPE_END:
        _eval_callback(&em->swipe_callback.end, dev, libinput_event_get_gesture_event(event));
        break;
      default:
        ret = 0;
        break;
     }

   return ret;
}

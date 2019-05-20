#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_RECOGNIZER_TAP_CLASS

//FIXME: It doesnt have matched config value.
//       may using dobule tap timeout value?
#define EFL_GESTURE_TAP_TIME_OUT 0.33

EOLIAN static Efl_Canvas_Gesture *
_efl_canvas_gesture_recognizer_tap_efl_canvas_gesture_recognizer_add(Eo *obj,
                                                                     Efl_Canvas_Gesture_Recognizer_Tap_Data *pd EINA_UNUSED,
                                                                     Efl_Object *target EINA_UNUSED)
{
   return efl_add(EFL_CANVAS_GESTURE_TAP_CLASS, obj);
}

static Eina_Bool
_tap_timeout_cb(void *data)
{
   Efl_Canvas_Gesture_Recognizer_Tap_Data *pd = data;

   efl_gesture_state_set(pd->gesture, EFL_GESTURE_CANCELED);
   efl_event_callback_call(pd->target, EFL_EVENT_GESTURE_TAP, pd->gesture);

   return ECORE_CALLBACK_CANCEL;
}


EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_efl_canvas_gesture_recognizer_tap_efl_canvas_gesture_recognizer_recognize(Eo *obj,
                                                                           Efl_Canvas_Gesture_Recognizer_Tap_Data *pd,
                                                                           Efl_Canvas_Gesture *gesture,
                                                                           Efl_Object *watched,
                                                                           Efl_Canvas_Gesture_Touch *event)
{
   double length;
   Eina_Position2D pos;
   Eina_Vector2 dist;
   Efl_Canvas_Gesture_Recognizer_Result result = EFL_GESTURE_CANCEL;
   Efl_Canvas_Gesture_Recognizer_Data *rd = efl_data_scope_get(obj, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);

   pd->target = watched;
   pd->gesture = gesture;

   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_BEGIN:
        {
           pos = efl_gesture_touch_start_point_get(event);
           efl_gesture_hotspot_set(gesture, pos);

           if (pd->timeout)
             ecore_timer_del(pd->timeout);
           pd->timeout = ecore_timer_add(EFL_GESTURE_TAP_TIME_OUT, _tap_timeout_cb, pd);

           result = EFL_GESTURE_TRIGGER;

           break;
        }

      case EFL_GESTURE_TOUCH_UPDATE:
      case EFL_GESTURE_TOUCH_END:
        {
           if (pd->timeout)
             {
                ecore_timer_del(pd->timeout);
                pd->timeout = NULL;
             }

           if (efl_gesture_state_get(gesture) != EFL_GESTURE_NONE &&
               !efl_gesture_touch_multi_touch_get(event))
             {
                dist = efl_gesture_touch_distance(event, 0);
                length = fabs(dist.x) + fabs(dist.y);
                if (length <= rd->finger_size)
                  {
                     if (efl_gesture_touch_state_get(event) == EFL_GESTURE_TOUCH_END)
                       result = EFL_GESTURE_FINISH;
                     else
                       result = EFL_GESTURE_TRIGGER;
                  }
             }

           break;
        }

      default:

        break;
     }

   return result;
}

#include "efl_canvas_gesture_recognizer_tap.eo.c"

#include "efl_gesture_private.h"

#define MY_CLASS EFL_GESTURE_RECOGNIZER_TAP_CLASS

EOLIAN static Efl_Gesture *
_efl_gesture_recognizer_tap_efl_gesture_recognizer_create(Eo *obj, void *pd EINA_UNUSED,
                                                          Efl_Object *target EINA_UNUSED)
{
   return efl_add(EFL_GESTURE_TAP_CLASS, obj);
}

EOLIAN static Efl_Gesture_Recognizer_Result
_efl_gesture_recognizer_tap_efl_gesture_recognizer_recognize(Eo *obj EINA_UNUSED,
                                                             void *pd EINA_UNUSED,
                                                             Efl_Gesture *gesture, Efl_Object *watched EINA_UNUSED,
                                                             Efl_Gesture_Touch *event EINA_UNUSED)
{
   double length;
   Eina_Vector2 pos, dist;
   Efl_Gesture_Recognizer_Result result = EFL_GESTURE_CANCEL;

   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_BEGIN:
        {
           pos = efl_gesture_touch_start_point_get(event);
           efl_gesture_hotspot_set(gesture, pos);
           result = EFL_GESTURE_TRIGGER;
           break;
        }
      case EFL_GESTURE_TOUCH_UPDATE:
      case EFL_GESTURE_TOUCH_END:
        {
           if (efl_gesture_state_get(gesture) != EFL_GESTURE_NONE &&
               !efl_gesture_touch_multi_touch_get(event))
             {
                dist = efl_gesture_touch_distance(event, 0);
                length = fabs(dist.x) + fabs(dist.y);
                if (length <= 50) // FIXME config!
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

#include "efl_gesture_recognizer_tap.eo.c"

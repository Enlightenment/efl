#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_RECOGNIZER_LONG_TAP_CLASS

#define LONG_TAP_TIME_OUT 0.2

EOLIAN static Efl_Canvas_Gesture *
_efl_canvas_gesture_recognizer_long_tap_efl_canvas_gesture_recognizer_create(Eo *obj, void *pd EINA_UNUSED,
                                                               Efl_Object *target EINA_UNUSED)
{
   return efl_add(EFL_CANVAS_GESTURE_LONG_TAP_CLASS, obj);
}

static Eina_Bool
_long_tap_timeout_cb(void *data)
{
   Efl_Canvas_Gesture_Long_Tap_Data *ltp = data;

   /* FIXME: Needs to propagate this event back to evas! */
   ltp->is_timeout = EINA_TRUE;

   return ECORE_CALLBACK_RENEW;
}

EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_efl_canvas_gesture_recognizer_long_tap_efl_canvas_gesture_recognizer_recognize(Eo *obj EINA_UNUSED,
                                                                  void *pd EINA_UNUSED,
                                                                  Efl_Canvas_Gesture *gesture, Efl_Object *watched EINA_UNUSED,
                                                                  Efl_Canvas_Gesture_Touch *event)
{
   double length; // Manhattan distance
   Eina_Vector2 pos, dist;
   Efl_Canvas_Gesture_Recognizer_Result result = EFL_GESTURE_CANCEL;
   Efl_Canvas_Gesture_Long_Tap_Data *ltp = efl_data_scope_get(gesture, EFL_CANVAS_GESTURE_LONG_TAP_CLASS);

   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_BEGIN:
        {
           pos = efl_gesture_touch_start_point_get(event);
           efl_gesture_hotspot_set(gesture, pos);
           if (ltp->timeout)
             ecore_timer_del(ltp->timeout);
           ltp->timeout = ecore_timer_add(LONG_TAP_TIME_OUT,
                                          _long_tap_timeout_cb, ltp);
           result = EFL_GESTURE_MAYBE;
           break;
        }
      case EFL_GESTURE_TOUCH_UPDATE:
        {
           if (!efl_gesture_touch_multi_touch_get(event))
             {
                dist = efl_gesture_touch_distance(event, 0);
                length = fabs(dist.x) + fabs(dist.y);
                if (length <= 50) // FIXME config!
                  {
                     if (ltp->is_timeout)
                       {
                          ltp->is_timeout = EINA_FALSE;
                          result = EFL_GESTURE_TRIGGER;
                       }
                     else
                       {
                          result = EFL_GESTURE_MAYBE;
                       }
                  }
                else
                  {
                     result = EFL_GESTURE_CANCEL;
                  }
             }
           break;
        }
      case EFL_GESTURE_TOUCH_END:
        {
           if (ltp->timeout)
             ecore_timer_del(ltp->timeout);
           ltp->timeout = NULL;
           if (efl_gesture_state_get(gesture) != EFL_GESTURE_NONE &&
               !efl_gesture_touch_multi_touch_get(event))
             {
                dist = efl_gesture_touch_distance(event, 0);
                length = fabs(dist.x) + fabs(dist.y);
                if (length <= 50 && ltp->is_timeout) // FIXME config!
                  {
                     result = EFL_GESTURE_FINISH;
                  }
                else
                  {
                     result = EFL_GESTURE_CANCEL;
                  }
             }
           break;
        }
      default:
        break;
     }
   return result;
}

EOLIAN static void
_efl_canvas_gesture_recognizer_long_tap_efl_canvas_gesture_recognizer_reset(Eo *obj,
                                                              void *pd EINA_UNUSED,
                                                              Efl_Canvas_Gesture *gesture)
{
   Efl_Canvas_Gesture_Long_Tap_Data *ltp;
   ltp = efl_data_scope_get(gesture, EFL_CANVAS_GESTURE_LONG_TAP_CLASS);
   if (ltp->timeout)
     ecore_timer_del(ltp->timeout);
   ltp->timeout = NULL;
   ltp->is_timeout = EINA_FALSE;
   efl_gesture_recognizer_reset(efl_super(obj, MY_CLASS), gesture);
}

#include "efl_canvas_gesture_recognizer_long_tap.eo.c"

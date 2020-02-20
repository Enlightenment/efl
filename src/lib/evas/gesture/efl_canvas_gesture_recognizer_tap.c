#include "efl_canvas_gesture_private.h"

#define MY_CLASS                                 EFL_CANVAS_GESTURE_RECOGNIZER_TAP_CLASS

//FIXME: It doesnt have matched config value.
//       may using dobule tap timeout value?
#define EFL_GESTURE_RECOGNIZER_TYPE_TAP_TIME_OUT 0.33

EOLIAN static const Efl_Class *
_efl_canvas_gesture_recognizer_tap_efl_canvas_gesture_recognizer_type_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Tap_Data *pd EINA_UNUSED)
{
   return EFL_CANVAS_GESTURE_TAP_CLASS;
}

static Eina_Bool
_tap_timeout_cb(void *data)
{
   Efl_Canvas_Gesture_Recognizer_Tap_Data *pd = data;
   pd->timeout = NULL;

   efl_gesture_state_set(pd->gesture, EFL_GESTURE_STATE_CANCELED);
   efl_event_callback_call(pd->target, EFL_EVENT_GESTURE_TAP, pd->gesture);

   return ECORE_CALLBACK_CANCEL;
}

EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_efl_canvas_gesture_recognizer_tap_efl_canvas_gesture_recognizer_recognize(Eo *obj EINA_UNUSED,
                                                                           Efl_Canvas_Gesture_Recognizer_Tap_Data *pd,
                                                                           Efl_Canvas_Gesture *gesture,
                                                                           Efl_Object *watched,
                                                                           Efl_Canvas_Gesture_Touch *event)
{
   double length;
   Eina_Position2D pos;
   Eina_Vector2 dist;
   Efl_Canvas_Gesture_Recognizer_Result result = EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;

   pd->target = watched;
   pd->gesture = gesture;

   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_STATE_BEGIN:
      {
new_tap:
         pos = efl_gesture_touch_start_point_get(event);
         efl_gesture_hotspot_set(gesture, pos);

         if (pd->timeout)
           ecore_timer_del(pd->timeout);
         pd->timeout = ecore_timer_add(EFL_GESTURE_RECOGNIZER_TYPE_TAP_TIME_OUT, _tap_timeout_cb, pd);

         result = EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;

         break;
      }

      case EFL_GESTURE_TOUCH_STATE_UPDATE:
        /* multi-touch */
        if (efl_gesture_touch_current_data_get(event)->action == EFL_POINTER_ACTION_DOWN)
          {
             /* a second finger was pressed at the same time-ish as the first: combine into same event */
             if (efl_gesture_touch_current_timestamp_get(event) - efl_gesture_timestamp_get(gesture) < TAP_TOUCH_TIME_THRESHOLD)
               {
                  result = EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
                  break;
               }
             /* another distinct touch occurred, treat this as a new touch */
             goto new_tap;
          }
        EINA_FALLTHROUGH;
      case EFL_GESTURE_TOUCH_STATE_END:
      {
         if (pd->timeout)
           {
              ecore_timer_del(pd->timeout);
              pd->timeout = NULL;
           }
         if (_event_multi_touch_get(event)) return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
         if (efl_gesture_state_get(gesture) != EFL_GESTURE_STATE_NONE)
           {
              dist = efl_gesture_touch_distance(event, efl_gesture_touch_current_data_get(event)->id);
              length = fabs(dist.x) + fabs(dist.y);
              if (length <= pd->finger_size)
                {
                   if (efl_gesture_touch_state_get(event) == EFL_GESTURE_TOUCH_STATE_END)
                     result = EFL_GESTURE_RECOGNIZER_RESULT_FINISH;
                   else
                     result = EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;
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

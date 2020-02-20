#include "efl_canvas_gesture_private.h"

#define MY_CLASS                      EFL_CANVAS_GESTURE_RECOGNIZER_LONG_PRESS_CLASS

#define EFL_GESTURE_LONG_PRESS_TIME_OUT 1.2

EOLIAN static const Efl_Class *
_efl_canvas_gesture_recognizer_long_press_efl_canvas_gesture_recognizer_type_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Long_Press_Data *pd EINA_UNUSED)
{
   return EFL_CANVAS_GESTURE_LONG_PRESS_CLASS;
}

EOLIAN static void
_efl_canvas_gesture_recognizer_long_press_efl_object_destructor(Eo *obj,
                                                              Efl_Canvas_Gesture_Recognizer_Long_Press_Data *pd)
{
   if (pd->timeout)
     ecore_timer_del(pd->timeout);

   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Bool
_long_press_timeout_cb(void *data)
{
   Efl_Canvas_Gesture_Recognizer_Long_Press_Data *pd = data;

   /* FIXME: Needs to propagate this event back to evas! */
   pd->is_timeout = EINA_TRUE;

   efl_gesture_state_set(pd->gesture, EFL_GESTURE_STATE_UPDATED);
   efl_event_callback_call(pd->target, EFL_EVENT_GESTURE_LONG_PRESS, pd->gesture);

   return ECORE_CALLBACK_RENEW;
}

EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_efl_canvas_gesture_recognizer_long_press_efl_canvas_gesture_recognizer_recognize(Eo *obj,
                                                                                Efl_Canvas_Gesture_Recognizer_Long_Press_Data *pd,
                                                                                Efl_Canvas_Gesture *gesture,
                                                                                Efl_Object *watched,
                                                                                Efl_Canvas_Gesture_Touch *event)
{
   double length, start_timeout = pd->start_timeout; // Manhattan distance
   double timeout = EFL_GESTURE_LONG_PRESS_TIME_OUT;
   Eina_Position2D pos;
   Eina_Vector2 dist;
   Efl_Canvas_Gesture_Recognizer_Result result = EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;

   pd->target = watched;
   pd->gesture = gesture;

   if (!EINA_DBL_NONZERO(start_timeout))
     {
        double time;
        Eina_Value *val = _recognizer_config_get(obj, "glayer_long_tap_start_timeout");

        if (val)
          {
             eina_value_get(val, &time);
             pd->start_timeout = timeout = time;
          }
     }
   else
     timeout = start_timeout;

   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_STATE_BEGIN:
      {
         pos = efl_gesture_touch_start_point_get(event);
         efl_gesture_hotspot_set(gesture, pos);

         if (pd->timeout)
           {
              ecore_timer_del(pd->timeout);
           }
         pd->timeout = ecore_timer_add(timeout,
                                       _long_press_timeout_cb, pd);

         result = EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;

         break;
      }

      case EFL_GESTURE_TOUCH_STATE_UPDATE:
      {
         dist = efl_gesture_touch_distance(event, efl_gesture_touch_current_data_get(event)->id);
         length = fabs(dist.x) + fabs(dist.y);

         if ((_event_multi_touch_get(event)) || (length > pd->finger_size))
           {
              if (pd->timeout)
                {
                   ecore_timer_del(pd->timeout);
                   pd->timeout = NULL;
                }

              result = EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
           }
         else
           {
              result = EFL_GESTURE_RECOGNIZER_RESULT_MAYBE;
           }

         break;
      }

      case EFL_GESTURE_TOUCH_STATE_END:
      {
         if (pd->timeout)
           {
              ecore_timer_del(pd->timeout);
              pd->timeout = NULL;
           }

         if (efl_gesture_state_get(gesture) != EFL_GESTURE_STATE_NONE &&
             !_event_multi_touch_get(event))
           {
              dist = efl_gesture_touch_distance(event, efl_gesture_touch_current_data_get(event)->id);
              length = fabs(dist.x) + fabs(dist.y);
              if (length <= pd->finger_size && pd->is_timeout)
                {
                   result = EFL_GESTURE_RECOGNIZER_RESULT_FINISH;
                }
              else
                {
                   result = EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
                }
           }

         break;
      }

      default:

        break;
     }

   return result;
}

#include "efl_canvas_gesture_recognizer_long_press.eo.c"

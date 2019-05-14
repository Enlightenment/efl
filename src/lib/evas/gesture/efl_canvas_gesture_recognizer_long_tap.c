#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_RECOGNIZER_LONG_TAP_CLASS

#define EFL_GESTURE_LONG_TAP_TIME_OUT 1.2

EOLIAN static Efl_Canvas_Gesture *
_efl_canvas_gesture_recognizer_long_tap_efl_canvas_gesture_recognizer_add(Eo *obj,
                                                                          Efl_Canvas_Gesture_Recognizer_Long_Tap_Data *pd EINA_UNUSED,
                                                                          Efl_Object *target EINA_UNUSED)
{
   return efl_add(EFL_CANVAS_GESTURE_LONG_TAP_CLASS, obj);
}

EOLIAN static void
_efl_canvas_gesture_recognizer_long_tap_efl_object_destructor(Eo *obj,
                                                              Efl_Canvas_Gesture_Recognizer_Long_Tap_Data *pd)
{
   if (pd->timeout)
     ecore_timer_del(pd->timeout);

   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Bool
_long_tap_timeout_cb(void *data)
{
   Efl_Canvas_Gesture_Recognizer_Long_Tap_Data *pd = data;

   /* FIXME: Needs to propagate this event back to evas! */
   pd->is_timeout = EINA_TRUE;

   efl_gesture_state_set(pd->gesture, EFL_GESTURE_UPDATED);
   efl_event_callback_call(pd->target, EFL_EVENT_GESTURE_LONG_TAP, pd->gesture);

   return ECORE_CALLBACK_RENEW;
}

EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_efl_canvas_gesture_recognizer_long_tap_efl_canvas_gesture_recognizer_recognize(Eo *obj,
                                                                                Efl_Canvas_Gesture_Recognizer_Long_Tap_Data *pd,
                                                                                Efl_Canvas_Gesture *gesture,
                                                                                Efl_Object *watched,
                                                                                Efl_Canvas_Gesture_Touch *event)
{
   double length; // Manhattan distance
   double timeout = EFL_GESTURE_LONG_TAP_TIME_OUT;
   Eina_Position2D pos;
   Eina_Vector2 dist;
   Efl_Canvas_Gesture_Recognizer_Result result = EFL_GESTURE_CANCEL;
   Efl_Canvas_Gesture_Recognizer_Data *rd = efl_data_scope_get(obj, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);

   pd->target = watched;
   pd->gesture = gesture;

   if (!pd->start_timeout)
     {
        double time;
        Eina_Value *val =  efl_gesture_recognizer_config_get(obj, "glayer_long_tap_start_timeout");

        if (val)
          {
             eina_value_get(val, &time);
             pd->start_timeout = timeout = time;
          }
     }
   else
     timeout = pd->start_timeout;


   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_BEGIN:
        {
           pos = efl_gesture_touch_start_point_get(event);
           efl_gesture_hotspot_set(gesture, pos);

           if (pd->timeout)
             {
                ecore_timer_del(pd->timeout);
             }
           pd->timeout = ecore_timer_add(timeout,
                                         _long_tap_timeout_cb, pd);

           result = EFL_GESTURE_TRIGGER;

           break;
        }

      case EFL_GESTURE_TOUCH_UPDATE:
        {
           dist = efl_gesture_touch_distance(event, 0);
           length = fabs(dist.x) + fabs(dist.y);

           if ((efl_gesture_touch_multi_touch_get(event)) || (length > rd->finger_size))
             {
                if (pd->timeout)
                  {
                     ecore_timer_del(pd->timeout);
                     pd->timeout = NULL;
                  }

                result = EFL_GESTURE_CANCEL;
             }
           else
             {
                result = EFL_GESTURE_MAYBE;
             }

           break;
        }

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
                if (length <= rd->finger_size && pd->is_timeout)
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
                                                                            Efl_Canvas_Gesture_Recognizer_Long_Tap_Data *pd,
                                                                            Efl_Canvas_Gesture *gesture)
{
   if (pd->timeout)
     {
        ecore_timer_del(pd->timeout);
        pd->timeout = NULL;
     }
   pd->is_timeout = EINA_FALSE;
   efl_gesture_recognizer_reset(efl_super(obj, MY_CLASS), gesture);
}

EOLIAN static double
_efl_canvas_gesture_recognizer_long_tap_timeout_get(const Eo *obj EINA_UNUSED,
                                                    Efl_Canvas_Gesture_Recognizer_Long_Tap_Data *pd)
{
   return pd->start_timeout;
}

EOLIAN static void
_efl_canvas_gesture_recognizer_long_tap_timeout_set(Eo *obj EINA_UNUSED,
                                                    Efl_Canvas_Gesture_Recognizer_Long_Tap_Data *pd,
                                                    double time)
{
   pd->start_timeout = time;
}

#include "efl_canvas_gesture_recognizer_long_tap.eo.c"

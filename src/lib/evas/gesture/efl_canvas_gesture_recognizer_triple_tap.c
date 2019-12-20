#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_RECOGNIZER_TRIPLE_TAP_CLASS

#define TAP_TIME_OUT 0.33

EOLIAN static Efl_Canvas_Gesture *
_efl_canvas_gesture_recognizer_triple_tap_efl_canvas_gesture_recognizer_add(Eo *obj, Efl_Canvas_Gesture_Recognizer_Triple_Tap_Data *pd EINA_UNUSED, Efl_Object *target EINA_UNUSED)
{
   return efl_add(EFL_CANVAS_GESTURE_TRIPLE_TAP_CLASS, obj);
}

EOLIAN static void
_efl_canvas_gesture_recognizer_triple_tap_efl_object_destructor(Eo *obj,
                                                                Efl_Canvas_Gesture_Recognizer_Triple_Tap_Data *pd)
{
   if (pd->timeout)
     ecore_timer_del(pd->timeout);

   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Bool
_tap_timeout_cb(void *data)
{
   Efl_Canvas_Gesture_Recognizer_Data *rd;
   Efl_Canvas_Gesture_Recognizer_Triple_Tap_Data *pd;

   rd = efl_data_scope_get(data, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);
   pd = efl_data_scope_get(data, EFL_CANVAS_GESTURE_RECOGNIZER_TRIPLE_TAP_CLASS);

   efl_gesture_state_set(pd->gesture, EFL_GESTURE_CANCELED);
   efl_event_callback_call(pd->target, EFL_EVENT_GESTURE_TRIPLE_TAP, pd->gesture);

   efl_gesture_manager_gesture_clean_up(rd->manager, pd->target, EFL_EVENT_GESTURE_TRIPLE_TAP);

   pd->timeout = NULL;
   pd->tap_count = 0;

   return ECORE_CALLBACK_CANCEL;
}

EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_efl_canvas_gesture_recognizer_triple_tap_efl_canvas_gesture_recognizer_recognize(Eo *obj,
                                                                                  Efl_Canvas_Gesture_Recognizer_Triple_Tap_Data *pd,
                                                                                  Efl_Canvas_Gesture *gesture, Efl_Object *watched,
                                                                                  Efl_Canvas_Gesture_Touch *event)
{
   double length;
   double timeout = TAP_TIME_OUT;
   Eina_Position2D pos;
   Eina_Vector2 dist;
   Efl_Canvas_Gesture_Recognizer_Result result = EFL_GESTURE_CANCEL;
   Efl_Canvas_Gesture_Recognizer_Data *rd = efl_data_scope_get(obj, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);

   pd->target = watched;
   pd->gesture = gesture;

   if (!pd->start_timeout)
     {
        double time;
        Eina_Value *val =  efl_gesture_recognizer_config_get(obj, "glayer_doublee_tap_timeout");

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
                ecore_timer_reset(pd->timeout);
              else
                pd->timeout = ecore_timer_add(timeout, _tap_timeout_cb, obj);

              result = EFL_GESTURE_TRIGGER;

              break;
           }

      case EFL_GESTURE_TOUCH_UPDATE:
           {
              result = EFL_GESTURE_IGNORE;

              if (efl_gesture_state_get(gesture) != EFL_GESTURE_NONE &&
                  !efl_gesture_touch_multi_touch_get(event))
                {
                   dist = efl_gesture_touch_distance(event, 0);
                   length = fabs(dist.x) + fabs(dist.y);

                   if (length > rd->finger_size)
                     {
                        if (pd->timeout)
                          {
                             ecore_timer_del(pd->timeout);
                             pd->timeout = NULL;
                          }

                        result = EFL_GESTURE_CANCEL;

                        pd->tap_count = 0;
                     }
                }

              break;
           }
      case EFL_GESTURE_TOUCH_END:
           {

              if (efl_gesture_state_get(gesture) != EFL_GESTURE_NONE &&
                  !efl_gesture_touch_multi_touch_get(event))
                {
                   dist = efl_gesture_touch_distance(event, 0);
                   length = fabs(dist.x) + fabs(dist.y);

                   if (length <= rd->finger_size)
                     {
                        pd->tap_count++;
                        if (pd->tap_count < 3)
                          {
                             if (pd->timeout)
                               ecore_timer_reset(pd->timeout);

                             result = EFL_GESTURE_TRIGGER;
                          }
                        else
                          {
                             if (pd->timeout)
                               {
                                  ecore_timer_del(pd->timeout);
                                  pd->timeout = NULL;
                               }

                             if (efl_gesture_touch_state_get(event) == EFL_GESTURE_TOUCH_END)
                               result = EFL_GESTURE_FINISH;
                             else
                               result = EFL_GESTURE_TRIGGER;

                             pd->tap_count = 0;
                          }
                     }
                   else
                     {
                        if (pd->timeout)
                          {
                             ecore_timer_del(pd->timeout);
                             pd->timeout = NULL;
                          }

                        result = EFL_GESTURE_CANCEL;

                        pd->tap_count = 0;
                     }
                }

              break;
           }

      default:

        break;
     }

   return result;
}

EOLIAN static double
_efl_canvas_gesture_recognizer_triple_tap_timeout_get(const Eo *obj EINA_UNUSED,
                                                      Efl_Canvas_Gesture_Recognizer_Triple_Tap_Data *pd)
{
   return pd->start_timeout;
}

EOLIAN static void
_efl_canvas_gesture_recognizer_triple_tap_timeout_set(Eo *obj EINA_UNUSED,
                                                      Efl_Canvas_Gesture_Recognizer_Triple_Tap_Data *pd,
                                                      double time)
{
   pd->start_timeout = time;
}

#include "efl_canvas_gesture_recognizer_triple_tap.eo.c"

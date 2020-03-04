#include "efl_canvas_gesture_private.h"

#define MY_CLASS     EFL_CANVAS_GESTURE_RECOGNIZER_TRIPLE_TAP_CLASS

#define TAP_TIME_OUT 0.33

EOLIAN static const Efl_Class *
_efl_canvas_gesture_recognizer_triple_tap_efl_canvas_gesture_recognizer_type_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Triple_Tap_Data *pd EINA_UNUSED)
{
   return EFL_CANVAS_GESTURE_TRIPLE_TAP_CLASS;
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
   Efl_Canvas_Gesture_Recognizer_Triple_Tap_Data *pd;

   pd = efl_data_scope_get(data, EFL_CANVAS_GESTURE_RECOGNIZER_TRIPLE_TAP_CLASS);

   efl_gesture_state_set(pd->gesture, EFL_GESTURE_STATE_CANCELED);
   efl_event_callback_call(pd->target, EFL_EVENT_GESTURE_TRIPLE_TAP, pd->gesture);

   efl_gesture_manager_recognizer_cleanup(efl_provider_find(data, EFL_CANVAS_GESTURE_MANAGER_CLASS), data, pd->target);

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
   double length, start_timeout = pd->start_timeout;
   double timeout = TAP_TIME_OUT;
   Eina_Position2D pos;
   Eina_Vector2 dist;
   Efl_Canvas_Gesture_Recognizer_Result result = EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;

   pd->target = watched;
   pd->gesture = gesture;

   if (!EINA_DBL_NONZERO(start_timeout))
     {
        double time;
        Eina_Value *val = _recognizer_config_get(obj, "glayer_double_tap_timeout");

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
           ecore_timer_reset(pd->timeout);
         else
           pd->timeout = ecore_timer_add(timeout, _tap_timeout_cb, obj);

         result = EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;

         break;
      }

      case EFL_GESTURE_TOUCH_STATE_UPDATE:
      {
        /* multi-touch */
        if (efl_gesture_touch_current_data_get(event)->action == EFL_POINTER_ACTION_DOWN)
          {
             /* a second finger was pressed at the same time-ish as the first: combine into same event */
             if (efl_gesture_touch_current_timestamp_get(event) - efl_gesture_timestamp_get(gesture) < TAP_TOUCH_TIME_THRESHOLD)
               {
                  result = EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
                  break;
               }
          }
         result = EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;

         if (efl_gesture_state_get(gesture) != EFL_GESTURE_STATE_NONE &&
             !_event_multi_touch_get(event))
           {
              dist = efl_gesture_touch_distance(event, efl_gesture_touch_current_data_get(event)->id);
              length = fabs(dist.x) + fabs(dist.y);

              if (length > pd->finger_size)
                {
                   if (pd->timeout)
                     {
                        ecore_timer_del(pd->timeout);
                        pd->timeout = NULL;
                     }

                   result = EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;

                   pd->tap_count = 0;
                }
           }

         break;
      }

      case EFL_GESTURE_TOUCH_STATE_END:
      {
         if (efl_gesture_state_get(gesture) != EFL_GESTURE_STATE_NONE &&
             !_event_multi_touch_get(event))
           {
              if (efl_gesture_touch_previous_data_get(event))
                {
                   Efl_Pointer_Action prev_act = efl_gesture_touch_previous_data_get(event)->action;
                   /* multi-touch */
                   if ((prev_act == EFL_POINTER_ACTION_UP) || (prev_act == EFL_POINTER_ACTION_CANCEL))
                     {
                        /* a second finger was pressed at the same time-ish as the first: combine into same event */
                        if (efl_gesture_touch_current_timestamp_get(event) - efl_gesture_timestamp_get(gesture) < TAP_TOUCH_TIME_THRESHOLD)
                          {
                             result = EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;
                             break;
                          }
                     }
                }
              dist = efl_gesture_touch_distance(event, efl_gesture_touch_current_data_get(event)->id);
              length = fabs(dist.x) + fabs(dist.y);

              if (length <= pd->finger_size)
                {
                   pd->tap_count++;
                   if (pd->tap_count < 3)
                     {
                        if (pd->timeout)
                          ecore_timer_reset(pd->timeout);

                        result = EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;
                     }
                   else
                     {
                        if (pd->timeout)
                          {
                             ecore_timer_del(pd->timeout);
                             pd->timeout = NULL;
                          }

                        if (efl_gesture_touch_state_get(event) == EFL_GESTURE_TOUCH_STATE_END)
                          result = EFL_GESTURE_RECOGNIZER_RESULT_FINISH;
                        else
                          result = EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;

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

                   result = EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;

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

#include "efl_canvas_gesture_recognizer_triple_tap.eo.c"

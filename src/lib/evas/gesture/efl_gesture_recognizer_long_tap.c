#include "efl_gesture_private.h"

#define MY_CLASS EFL_GESTURE_RECOGNIZER_LONG_TAP_CLASS

#define LONG_TAP_TIME_OUT 0.2

typedef struct _Efl_Gesture_Recognizer_Long_Tap_Data
{

} Efl_Gesture_Recognizer_Long_Tap_Data;

EOLIAN static Efl_Object *
_efl_gesture_recognizer_long_tap_efl_object_constructor(Eo *obj, Efl_Gesture_Recognizer_Long_Tap_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   return obj;
}

EOLIAN static void
_efl_gesture_recognizer_long_tap_efl_object_destructor(Eo *obj EINA_UNUSED, Efl_Gesture_Recognizer_Long_Tap_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Gesture *
_efl_gesture_recognizer_long_tap_efl_gesture_recognizer_create(Eo *obj, Efl_Gesture_Recognizer_Long_Tap_Data *pd EINA_UNUSED,
                                                               Efl_Object *target EINA_UNUSED)
{
   return efl_add(EFL_GESTURE_LONG_TAP_CLASS, obj);
}

static Eina_Bool
_long_tap_timeout_cb(void *data)
{
   Efl_Gesture_Long_Tap_Data *ltp = data;

   ltp->is_timeout = EINA_TRUE;
   return ECORE_CALLBACK_RENEW;
}

EOLIAN static Efl_Gesture_Recognizer_Result
_efl_gesture_recognizer_long_tap_efl_gesture_recognizer_recognize(Eo *obj EINA_UNUSED,
                                                                   Efl_Gesture_Recognizer_Long_Tap_Data *pd EINA_UNUSED,
                                                                   Efl_Gesture *gesture, Efl_Object *watched EINA_UNUSED,
                                                                   Efl_Gesture_Touch *event)
{
    double dist_x, dist_y, length, x, y;
    Eina_Vector2 pos;
    Efl_Gesture_Recognizer_Result result = EFL_GESTURE_CANCEL;
    Efl_Gesture_Long_Tap_Data *ltp = efl_data_scope_get(gesture, EFL_GESTURE_LONG_TAP_CLASS);

    switch (efl_gesture_touch_state_get(event))
      {
         case EFL_GESTURE_TOUCH_BEGIN:
           {
              efl_gesture_touch_start_point(event, &x, &y);
              eina_vector2_set(&pos, x, y);
              efl_gesture_long_tap_position_set(gesture, pos);
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
                   efl_gesture_touch_distance(event, 0, &dist_x, &dist_y);
                   length = abs(dist_x) + abs(dist_y);
                   if (length <= 50)
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
                   efl_gesture_touch_distance(event, 0, &dist_x, &dist_y);
                   length = abs(dist_x) + abs(dist_y);
                   if (length <= 50 && ltp->is_timeout)
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
_efl_gesture_recognizer_long_tap_efl_gesture_recognizer_reset(Eo *obj,
                                                              Efl_Gesture_Recognizer_Long_Tap_Data *pd EINA_UNUSED,
                                                              Efl_Gesture *gesture)
{
   Efl_Gesture_Long_Tap_Data *ltp;
   ltp = efl_data_scope_get(gesture, EFL_GESTURE_LONG_TAP_CLASS);
   eina_vector2_set(&ltp->pos, 0, 0);
   if (ltp->timeout)
     ecore_timer_del(ltp->timeout);
   ltp->timeout = NULL;
   ltp->is_timeout = EINA_FALSE;
   efl_gesture_recognizer_reset(efl_super(obj, MY_CLASS), gesture);
}

#include "efl_gesture_recognizer_long_tap.eo.c"
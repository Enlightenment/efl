#include "efl_gesture_private.h"

#define MY_CLASS EFL_GESTURE_RECOGNIZER_TAP_CLASS

typedef struct _Efl_Gesture_Recognizer_Tap_Data
{

} Efl_Gesture_Recognizer_Tap_Data;

EOLIAN static Efl_Object *
_efl_gesture_recognizer_tap_efl_object_constructor(Eo *obj, Efl_Gesture_Recognizer_Tap_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   return obj;
}

EOLIAN static void
_efl_gesture_recognizer_tap_efl_object_destructor(Eo *obj EINA_UNUSED, Efl_Gesture_Recognizer_Tap_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Gesture *
_efl_gesture_recognizer_tap_efl_gesture_recognizer_create(Eo *obj EINA_UNUSED, Efl_Gesture_Recognizer_Tap_Data *pd EINA_UNUSED,
                                                                Efl_Object *target EINA_UNUSED)
{
   return efl_add(EFL_GESTURE_TAP_CLASS, NULL);
}

EOLIAN static Efl_Gesture_Recognizer_Result
_efl_gesture_recognizer_tap_efl_gesture_recognizer_recognize(Eo *obj EINA_UNUSED,
                                                                   Efl_Gesture_Recognizer_Tap_Data *pd EINA_UNUSED,
                                                                   Efl_Gesture *gesture, Efl_Object *watched EINA_UNUSED,
                                                                   Efl_Gesture_Touch *event EINA_UNUSED)
{
    double dist_x, dist_y, length, x, y;
    Efl_Gesture_Recognizer_Result result = EFL_GESTURE_CANCEL;

    switch (efl_gesture_touch_state_get(event))
      {
         case EFL_GESTURE_TOUCH_BEGIN:
           {
              efl_gesture_touch_start_point(event, &x, &y);
              efl_gesture_tap_position_set(gesture, EINA_POSITION2D(x, y));
              efl_gesture_hotspot_set(gesture, EINA_POSITION2D(x, y));
              result = EFL_GESTURE_TRIGGER;
              break;
           }
         case EFL_GESTURE_TOUCH_UPDATE:
         case EFL_GESTURE_TOUCH_END:
           {
              if (efl_gesture_state_get(gesture) != EFL_GESTURE_NONE &&
                  !efl_gesture_touch_multi_touch_get(event))
                {
                   efl_gesture_touch_distance(event, 0, &dist_x, &dist_y);
                   length = abs(dist_x) + abs(dist_y);
                   if (length <= 50)
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

EOLIAN static void
_efl_gesture_recognizer_tap_efl_gesture_recognizer_reset(Eo *obj,
                                                              Efl_Gesture_Recognizer_Tap_Data *pd EINA_UNUSED,
                                                              Efl_Gesture *gesture)
{
   Efl_Gesture_Tap_Data *tap;
   tap = efl_data_scope_get(gesture, EFL_GESTURE_TAP_CLASS);
   tap->pos = EINA_POSITION2D(0, 0);
   efl_gesture_recognizer_reset(efl_super(obj, MY_CLASS), gesture);
}

#include "efl_gesture_recognizer_tap.eo.c"
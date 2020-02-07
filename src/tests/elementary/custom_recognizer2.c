#include <Efl_Ui.h>
#include "custom_recognizer2.eo.h"
#define MY_CLASS                       CUSTOM_RECOGNIZER2_CLASS

typedef struct Custom_Recognizer2_Data
{
} Custom_Recognizer2_Data;

EOLIAN static Eo *
_custom_recognizer2_efl_object_finalize(Eo *obj, Custom_Recognizer2_Data *pd EINA_UNUSED)
{
   efl_gesture_recognizer_custom_gesture_name_set(obj, "custom_gesture2");
   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_custom_recognizer2_efl_canvas_gesture_recognizer_recognize(Eo *obj EINA_UNUSED, Custom_Recognizer2_Data *pd EINA_UNUSED,
                                                           Efl_Canvas_Gesture *gesture EINA_UNUSED, Efl_Object *watched EINA_UNUSED,
                                                           Efl_Canvas_Gesture_Touch *event)
{
   Eina_Position2D start = efl_gesture_touch_start_point_get(event);
   Eina_Position2D cur = efl_gesture_touch_current_point_get(event);
   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_STATE_BEGIN:
        return EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;
      case EFL_GESTURE_TOUCH_STATE_UPDATE:
        if (EINA_POSITION2D_EQ(start, cur))
          return EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;
        if ((cur.x - start.x == 1) && (cur.y == start.y))
          return EFL_GESTURE_RECOGNIZER_RESULT_FINISH;
      case EFL_GESTURE_TOUCH_STATE_END:
      default:

        break;
     }

   return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
}

#include "custom_recognizer2.eo.c"

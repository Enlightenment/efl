#include <Efl_Ui.h>
#include "custom_recognizer.eo.h"
#define MY_CLASS                       CUSTOM_RECOGNIZER_CLASS

typedef struct Custom_Recognizer_Data
{

} Custom_Recognizer_Data;

EOLIAN static Eo *
_custom_recognizer_efl_object_finalize(Eo *obj, Custom_Recognizer_Data *pd EINA_UNUSED)
{
   efl_gesture_recognizer_custom_gesture_name_set(obj, "custom_gesture");
   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Canvas_Gesture *
_custom_recognizer_efl_canvas_gesture_recognizer_add(Eo *obj, Custom_Recognizer_Data *pd EINA_UNUSED, Efl_Object *target EINA_UNUSED)
{
   return efl_add(EFL_CANVAS_GESTURE_CUSTOM_CLASS, obj);
}

EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_custom_recognizer_efl_canvas_gesture_recognizer_recognize(Eo *obj EINA_UNUSED, Custom_Recognizer_Data *pd EINA_UNUSED,
                                                           Efl_Canvas_Gesture *gesture EINA_UNUSED, Efl_Object *watched EINA_UNUSED,
                                                           Efl_Canvas_Gesture_Touch *event)
{
   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_STATE_BEGIN:
        return EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;
      case EFL_GESTURE_TOUCH_STATE_UPDATE:
        return EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;
      case EFL_GESTURE_TOUCH_STATE_END:
        return EFL_GESTURE_RECOGNIZER_RESULT_FINISH;
      default:

        break;
     }

   return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
}

#include "custom_recognizer.eo.c"

#include <Efl_Ui.h>
#include "custom_recognizer.eo.h"
#include "custom_gesture.eo.h"
#include "custom_gesture.h"

#define MY_CLASS                       CUSTOM_RECOGNIZER_CLASS

typedef struct Custom_Recognizer_Data
{

} Custom_Recognizer_Data;

EOLIAN static const Efl_Class *
_custom_recognizer_efl_canvas_gesture_recognizer_type_get(const Eo *obj EINA_UNUSED, Custom_Recognizer_Data *pd EINA_UNUSED)
{
   return CUSTOM_GESTURE_CLASS;
}

EOLIAN static Eo *
_custom_recognizer_efl_object_finalize(Eo *obj, Custom_Recognizer_Data *pd EINA_UNUSED)
{
   efl_gesture_recognizer_custom_gesture_name_set(obj, "custom_gesture");
   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Canvas_Gesture_Recognizer_Result
_custom_recognizer_efl_canvas_gesture_recognizer_recognize(Eo *obj EINA_UNUSED, Custom_Recognizer_Data *pd EINA_UNUSED,
                                                           Efl_Canvas_Gesture *gesture, Efl_Object *watched EINA_UNUSED,
                                                           Efl_Canvas_Gesture_Touch *event)
{
   const Efl_Gesture_Touch_Point_Data *data = efl_gesture_touch_current_data_get(event);
   Custom_Gesture_Data *gd;

   /* ignore multi-touch */
   if (data->id) return EFL_GESTURE_RECOGNIZER_RESULT_IGNORE;

   switch (efl_gesture_touch_state_get(event))
     {
      case EFL_GESTURE_TOUCH_STATE_BEGIN:
        return EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;
      case EFL_GESTURE_TOUCH_STATE_UPDATE:
        gd = efl_data_scope_get(gesture, CUSTOM_GESTURE_CLASS);
        gd->x_delta += data->cur.pos.x - data->prev.pos.x;
        gd->y_delta += data->cur.pos.y - data->prev.pos.y;
        return EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER;
      case EFL_GESTURE_TOUCH_STATE_END:
        return EFL_GESTURE_RECOGNIZER_RESULT_FINISH;
      default:

        break;
     }

   return EFL_GESTURE_RECOGNIZER_RESULT_CANCEL;
}

#include "custom_recognizer.eo.c"

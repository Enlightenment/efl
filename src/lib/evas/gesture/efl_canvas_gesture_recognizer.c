#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_RECOGNIZER_CLASS

#define EFL_GESTURE_TAP_FINGER_SIZE 10
EOLIAN static Eina_Value *
_efl_canvas_gesture_recognizer_config_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Data *pd, const char *name)
{
   return efl_gesture_manager_config_get(pd->manager, name);
}

EOLIAN static void
_efl_canvas_gesture_recognizer_reset(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Data *pd EINA_UNUSED,
                              Efl_Canvas_Gesture *gesture EINA_UNUSED)
{

}

EOLIAN static Efl_Object *
_efl_canvas_gesture_recognizer_efl_object_constructor(Eo *obj, Efl_Canvas_Gesture_Recognizer_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->finger_size = EFL_GESTURE_TAP_FINGER_SIZE;

   return obj;
}
#include "efl_canvas_gesture_recognizer.eo.c"

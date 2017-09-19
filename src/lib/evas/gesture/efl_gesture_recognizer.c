#include "efl_gesture_private.h"

#define MY_CLASS EFL_GESTURE_RECOGNIZER_CLASS


EOLIAN static Eina_Value *
_efl_gesture_recognizer_config_get(Eo *obj EINA_UNUSED, Efl_Gesture_Recognizer_Data *pd, const char *name)
{
   return efl_gesture_manager_config_get(pd->manager, name);
}


EOLIAN static void
_efl_gesture_recognizer_reset(Eo *obj EINA_UNUSED, Efl_Gesture_Recognizer_Data *pd EINA_UNUSED,
                                 Efl_Gesture *gesture EINA_UNUSED)
{

}

#include "efl_gesture_recognizer.eo.c"



#define EFL_CANVAS_GESTURE_RECOGNIZER_PROTECTED
#include "efl_canvas_gesture_private.h"

#define MY_CLASS                                    EFL_CANVAS_GESTURE_RECOGNIZER_CLASS
#include "efl_canvas_gesture_recognizer.eo.h"

Eina_Value *
_recognizer_config_get(const Eo *obj, const char *name)
{
   Eo *manager = efl_provider_find(obj, EFL_CANVAS_GESTURE_MANAGER_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(manager, NULL);
   return efl_gesture_manager_config_get(manager, name);
}

EOLIAN static void
_efl_canvas_gesture_recognizer_reset(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Data *pd EINA_UNUSED,
                                     Efl_Canvas_Gesture *gesture EINA_UNUSED)
{
}


EOLIAN static Eina_Bool
_efl_canvas_gesture_recognizer_continues_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Data *pd)
{
   return pd->continues;
}

EOLIAN static void
_efl_canvas_gesture_recognizer_continues_set(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Data *pd, Eina_Bool value)
{
   pd->continues = !!value;
}

int
_direction_get(Evas_Coord xx1, Evas_Coord xx2)
{
   if (xx2 < xx1) return -1;
   if (xx2 > xx1) return 1;

   return 0;
}

#include "efl_canvas_gesture_recognizer.eo.c"

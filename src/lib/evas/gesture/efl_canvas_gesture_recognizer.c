#include "efl_canvas_gesture_private.h"

#define MY_CLASS                                    EFL_CANVAS_GESTURE_RECOGNIZER_CLASS

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

int
_direction_get(Evas_Coord xx1, Evas_Coord xx2)
{
   if (xx2 < xx1) return -1;
   if (xx2 > xx1) return 1;

   return 0;
}

#include "efl_canvas_gesture_recognizer.eo.c"

#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_FLICK_CLASS

EOLIAN static Eina_Vector2
_efl_canvas_gesture_flick_momentum_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Flick_Data *pd)
{
   return pd->momentum;
}

EOLIAN static double
_efl_canvas_gesture_flick_angle_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Flick_Data *pd)
{
   return pd->angle;
}

#include "efl_canvas_gesture_flick.eo.c"

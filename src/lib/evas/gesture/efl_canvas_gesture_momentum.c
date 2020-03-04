#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_MOMENTUM_CLASS

EOLIAN static Eina_Vector2
_efl_canvas_gesture_momentum_momentum_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Momentum_Data *pd)
{
   return pd->momentum;
}

#include "efl_canvas_gesture_momentum.eo.c"

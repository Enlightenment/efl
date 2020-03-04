#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_ROTATE_CLASS

EOLIAN static unsigned int
_efl_canvas_gesture_rotate_radius_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Rotate_Data *pd)
{
   return pd->radius;
}

EOLIAN static double
_efl_canvas_gesture_rotate_angle_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Rotate_Data *pd)
{
   return pd->angle;
}

#include "efl_canvas_gesture_rotate.eo.c"

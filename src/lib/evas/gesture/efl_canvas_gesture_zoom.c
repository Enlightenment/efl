#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_ZOOM_CLASS

EOLIAN static double
_efl_canvas_gesture_zoom_radius_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Zoom_Data *pd)
{
   return pd->radius;
}

EOLIAN static double
_efl_canvas_gesture_zoom_zoom_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Zoom_Data *pd)
{
   return pd->zoom;
}

#include "efl_canvas_gesture_zoom.eo.c"

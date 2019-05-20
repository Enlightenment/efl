#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_ZOOM_CLASS

EOLIAN static Efl_Object *
_efl_canvas_gesture_zoom_efl_object_constructor(Eo *obj, Efl_Canvas_Gesture_Zoom_Data *pd EINA_UNUSED)
{
   Efl_Canvas_Gesture_Data *gd;

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   gd = efl_data_scope_get(obj, EFL_CANVAS_GESTURE_CLASS);
   gd->type = EFL_EVENT_GESTURE_ZOOM;

   return obj;
}

EOLIAN static void
_efl_canvas_gesture_zoom_efl_object_destructor(Eo *obj, Efl_Canvas_Gesture_Zoom_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static double
_efl_canvas_gesture_zoom_radius_get(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Zoom_Data *pd)
{
   return pd->radius;
}

EOLIAN static double
_efl_canvas_gesture_zoom_zoom_get(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Zoom_Data *pd)
{
   return pd->zoom;
}

#include "efl_canvas_gesture_zoom.eo.c"

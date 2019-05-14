#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_DOUBLE_TAP_CLASS

EOLIAN static Efl_Object *
_efl_canvas_gesture_double_tap_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   Efl_Canvas_Gesture_Data *gd;

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   gd = efl_data_scope_get(obj, EFL_CANVAS_GESTURE_CLASS);
   gd->type = EFL_EVENT_GESTURE_DOUBLE_TAP;

   return obj;
}

#include "efl_canvas_gesture_double_tap.eo.c"

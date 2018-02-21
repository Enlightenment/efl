#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_LONG_TAP_CLASS


EOLIAN static Efl_Object *
_efl_canvas_gesture_long_tap_efl_object_constructor(Eo *obj, Efl_Canvas_Gesture_Long_Tap_Data *pd EINA_UNUSED)
{
   Efl_Canvas_Gesture_Data *gd;

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   gd = efl_data_scope_get(obj, EFL_CANVAS_GESTURE_CLASS);
   gd->type = EFL_EVENT_GESTURE_LONG_TAP;

   return obj;
}

EOLIAN static void
_efl_canvas_gesture_long_tap_efl_object_destructor(Eo *obj, Efl_Canvas_Gesture_Long_Tap_Data *pd)
{
   if (pd->timeout)
     ecore_timer_del(pd->timeout);

   efl_destructor(efl_super(obj, MY_CLASS));
}

#include "efl_canvas_gesture_long_tap.eo.c"

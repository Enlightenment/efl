#include "efl_gesture_private.h"

#define MY_CLASS EFL_GESTURE_TAP_CLASS


EOLIAN static Efl_Object *
_efl_gesture_tap_efl_object_constructor(Eo *obj, Efl_Gesture_Tap_Data *pd EINA_UNUSED)
{
   Efl_Gesture_Data *gd;

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   gd = efl_data_scope_get(obj, EFL_GESTURE_CLASS);
   gd->type = EFL_EVENT_GESTURE_TAP;

   return obj;
}

#include "efl_gesture_tap.eo.c"

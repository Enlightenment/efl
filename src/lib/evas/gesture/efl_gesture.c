#include "efl_gesture_private.h"

#define MY_CLASS EFL_GESTURE_CLASS

EOLIAN static const Efl_Event_Description *
 _efl_gesture_type_get(Eo *obj EINA_UNUSED, Efl_Gesture_Data *pd)
{
   return pd->type;
}

EOLIAN static Efl_Gesture_State
_efl_gesture_state_get(Eo *obj EINA_UNUSED, Efl_Gesture_Data *pd)
{
   return pd->state;
}

EOLIAN static void
_efl_gesture_state_set(Eo *obj EINA_UNUSED, Efl_Gesture_Data *pd, Efl_Gesture_State state)
{
   pd->state = state;
}

EOLIAN static void
_efl_gesture_hotspot_set(Eo *obj EINA_UNUSED, Efl_Gesture_Data *pd, Eina_Vector2 hotspot)
{
   pd->hotspot = hotspot;
}


EOLIAN static Eina_Vector2
_efl_gesture_hotspot_get(Eo *obj EINA_UNUSED, Efl_Gesture_Data *pd)
{
   return pd->hotspot;
}

#include "efl_gesture.eo.c"
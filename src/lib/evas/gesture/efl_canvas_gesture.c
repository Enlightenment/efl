#define EFL_CANVAS_GESTURE_PROTECTED
#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_CLASS

EOLIAN static Efl_Canvas_Gesture_State
_efl_canvas_gesture_state_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Data *pd)
{
   return pd->state;
}

EOLIAN static void
_efl_canvas_gesture_state_set(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Data *pd, Efl_Canvas_Gesture_State state)
{
   pd->state = state;
}

EOLIAN static void
_efl_canvas_gesture_hotspot_set(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Data *pd, Eina_Position2D hotspot)
{
   pd->hotspot = hotspot;
}

EOLIAN static Eina_Position2D
_efl_canvas_gesture_hotspot_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Data *pd)
{
   return pd->hotspot;
}

EOLIAN static void
_efl_canvas_gesture_timestamp_set(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Data *pd, unsigned int timestamp)
{
   pd->timestamp = timestamp;
}

EOLIAN static unsigned int
_efl_canvas_gesture_timestamp_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Data *pd)
{
   return pd->timestamp;
}

EOLIAN static void
_efl_canvas_gesture_touch_count_set(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Data *pd, unsigned int touch_count)
{
   pd->touch_count = touch_count;
}

EOLIAN static unsigned int
_efl_canvas_gesture_touch_count_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Data *pd)
{
   return pd->touch_count;
}

#include "efl_canvas_gesture.eo.c"
#include "efl_canvas_gesture_events.eo.c"

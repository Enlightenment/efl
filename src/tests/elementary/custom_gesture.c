#include <Efl_Ui.h>
#include "custom_gesture.eo.h"
#include "custom_gesture.h"
#define MY_CLASS                       CUSTOM_GESTURE_CLASS


EOLIAN static int
_custom_gesture_x_delta_get(const Eo *obj EINA_UNUSED, Custom_Gesture_Data *pd)
{
   return pd->x_delta;
}

EOLIAN static int
_custom_gesture_y_delta_get(const Eo *obj EINA_UNUSED, Custom_Gesture_Data *pd)
{
   return pd->y_delta;
}

#include "custom_gesture.eo.c"

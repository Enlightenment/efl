#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

#define EFL_INTERNAL_UNSTABLE
#include "efl_common_internal.h"

#define MY_CLASS EFL_POINTER_EVENT_CLASS


/* Pointer Event
 *
 * This is a storage class only, should not require ANY knowledge about
 * Ecore, Evas or anything else.
 *
 * This is intended to replace Ecore and Evas structs for mouse events.
 *
 * Do not add any logic here.
 */

EOLIAN static Eo_Base *
_efl_pointer_event_eo_base_constructor(Eo *obj, Efl_Pointer_Event_Data *pd)
{
   eo_constructor(eo_super(obj, MY_CLASS));
   pd->eo = obj;
   return obj;
}

EOLIAN static void
_efl_pointer_event_action_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, Efl_Pointer_Action act)
{
   pd->action = act;
}

EOLIAN static Efl_Pointer_Action
_efl_pointer_event_action_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd)
{
   return pd->action;
}

EOLIAN static void
_efl_pointer_event_button_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, int but)
{
   pd->button = but;
}

EOLIAN static int
_efl_pointer_event_button_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd)
{
   return pd->button;
}

EOLIAN static void
_efl_pointer_event_button_pressed_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, int button, Eina_Bool pressed)
{
   if (button < 0) return;
   if (button > 31) return;
   if (pressed)
     pd->button_flags |= (1 << button);
   else
     pd->button_flags &= ~(1 << button);
}

EOLIAN static Eina_Bool
_efl_pointer_event_button_pressed_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, int button)
{
   if (button < 0) return EINA_FALSE;
   if (button > 31) return EINA_FALSE;
   return (pd->button_flags & (1 << button)) != 0;
}

EOLIAN static void
_efl_pointer_event_position_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, int x, int y)
{
   pd->cur.canvas.x = x;
   pd->cur.canvas.y = y;
   /* FIXME: What is the difference??? */
   pd->cur.output.x = x;
   pd->cur.output.y = y;
}

EOLIAN static void
_efl_pointer_event_position_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, int *x, int *y)
{
   if (x) *x = pd->cur.canvas.x;
   if (y) *y = pd->cur.canvas.y;
}

EOLIAN static void
_efl_pointer_event_previous_position_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, int x, int y)
{
   pd->prev.canvas.x = x;
   pd->prev.canvas.y = y;
   /* FIXME: What is the difference??? */
   pd->prev.output.x = x;
   pd->prev.output.y = y;
}

EOLIAN static void
_efl_pointer_event_previous_position_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, int *x, int *y)
{
   if (x) *x = pd->prev.canvas.x;
   if (y) *y = pd->prev.canvas.y;
}


/* FIXME: implement input state with eo*/

/*
EOLIAN static void
_efl_pointer_event_input_state_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, Efl_Input_State *modifiers)
{

}
EOLIAN static Efl_Input_State *
_efl_pointer_event_input_state_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd)
{

}
*/

EOLIAN static void
_efl_pointer_event_device_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, Efl_Input_Device *dev)
{
   /* ref? */
   pd->device = dev;
}

EOLIAN static Efl_Input_Device *
_efl_pointer_event_device_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd)
{
   return pd->device;
}

EOLIAN static void
_efl_pointer_event_source_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, Efl_Gfx *src)
{
   /* ref? */
   pd->source = src;
}

EOLIAN static Efl_Gfx *
_efl_pointer_event_source_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd)
{
   return pd->source;
}

EOLIAN static void
_efl_pointer_event_button_flags_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, Efl_Pointer_Button_Flags flags)
{
   pd->button_flags = flags;
}

EOLIAN static Efl_Pointer_Button_Flags
_efl_pointer_event_button_flags_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd)
{
   return pd->button_flags;
}

EOLIAN static void
_efl_pointer_event_event_flags_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, Efl_Pointer_Event_Flags flags)
{
   pd->event_flags = flags;
}

EOLIAN static Efl_Pointer_Event_Flags
_efl_pointer_event_event_flags_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd)
{
   return pd->event_flags;
}

EOLIAN static void
_efl_pointer_event_efl_event_timestamp_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, double ms)
{
   pd->timestamp = (unsigned int) ms;
}

EOLIAN static double
_efl_pointer_event_efl_event_timestamp_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd)
{
   return (double) pd->timestamp;
}

EOLIAN static void
_efl_pointer_event_efl_event_event_type_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, const Eo_Event_Description *type)
{
   pd->event_desc = type;
}

EOLIAN static const Eo_Event_Description *
_efl_pointer_event_efl_event_event_type_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd)
{
   return pd->event_desc;
}

EOLIAN static void
_efl_pointer_event_wheel_direction_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, Efl_Orient dir)
{
   pd->wheel.dir = dir;
}

EOLIAN static Efl_Orient
_efl_pointer_event_wheel_direction_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd)
{
   return pd->wheel.dir;
}

EOLIAN static void
_efl_pointer_event_wheel_distance_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, int dist)
{
   pd->wheel.z = dist;
}

EOLIAN static int
_efl_pointer_event_wheel_distance_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd)
{
   return pd->wheel.z;
}

#include "interfaces/efl_pointer_event.eo.c"

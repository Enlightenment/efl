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

static Efl_Pointer_Event *s_cached_event = NULL;

static void
_del_hook(Eo *evt)
{
   if (!s_cached_event)
     {
        if (eo_parent_get(evt))
          {
             eo_ref(evt);
             eo_parent_set(evt, NULL);
          }
        s_cached_event = evt;
     }
   else
     {
        eo_del_intercept_set(evt, NULL);
        eo_unref(evt);
     }
}

EOLIAN static Efl_Pointer_Event *
_efl_pointer_event_instance_get(Eo_Class *klass EINA_UNUSED, void *pd EINA_UNUSED,
                                Eo *owner, void **priv)
{
   Efl_Pointer_Event *evt;

   if (s_cached_event)
     {
        evt = s_cached_event;
        s_cached_event = NULL;
        efl_event_reset(evt);
        eo_parent_set(evt, owner);
     }
   else
     {
        evt = eo_add(EFL_POINTER_EVENT_CLASS, owner);
        eo_del_intercept_set(evt, _del_hook);
     }

   if (priv)
     *priv = eo_data_scope_get(evt, EFL_POINTER_EVENT_CLASS);

   return evt;
}

EOLIAN static void
_efl_pointer_event_class_destructor(Eo_Class *klass EINA_UNUSED)
{
   // this is a strange situation...
   eo_unref(s_cached_event);
   s_cached_event = NULL;
}

EOLIAN static Eo_Base *
_efl_pointer_event_eo_base_constructor(Eo *obj, Efl_Pointer_Event_Data *pd EINA_UNUSED)
{
   eo_constructor(eo_super(obj, MY_CLASS));
   efl_event_reset(obj);
   return obj;
}

EOLIAN static void
_efl_pointer_event_efl_event_reset(Eo *obj, Efl_Pointer_Event_Data *pd)
{
   free(pd->legacy);
   memset(pd, 0, sizeof(*pd));
   pd->eo = obj;
   pd->wheel.dir = EFL_ORIENT_VERTICAL;
}

EOLIAN static Efl_Event *
_efl_pointer_event_efl_event_dup(Eo *obj, Efl_Pointer_Event_Data *pd)
{
   Efl_Pointer_Event_Data *ev;
   Efl_Pointer_Event *evt;

   evt = _efl_pointer_event_instance_get(EFL_POINTER_EVENT_CLASS, NULL, obj, &ev);
   if (!evt) return NULL;

   memcpy(ev, pd, sizeof(*ev));
   ev->eo = evt;

   return evt;
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
_efl_pointer_event_position_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, int x, int y, double xsub, double ysub)
{
   pd->cur.x = x;
   pd->cur.y = y;
   pd->cur.xsub = xsub;
   pd->cur.ysub = ysub;
}

EOLIAN static void
_efl_pointer_event_position_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, int *x, int *y, double *xsub, double *ysub)
{
   if (x) *x = pd->cur.x;
   if (y) *y = pd->cur.y;
   if (xsub) *xsub = pd->cur.xsub;
   if (ysub) *ysub = pd->cur.ysub;
}

EOLIAN static void
_efl_pointer_event_previous_position_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, int x, int y)
{
   pd->prev.x = x;
   pd->prev.y = y;
}

EOLIAN static void
_efl_pointer_event_previous_position_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, int *x, int *y)
{
   if (x) *x = pd->prev.x;
   if (y) *y = pd->prev.y;
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

EOLIAN static int
_efl_pointer_event_finger_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd)
{
   return pd->finger;
}

EOLIAN static void
_efl_pointer_event_finger_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, int id)
{
   pd->finger = id;
}

EOLIAN static void
_efl_pointer_event_touch_get(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, double *r, double *rx, double *ry, double *press, double *angle)
{
   if (r) *r = pd->radius;
   if (rx) *rx = pd->radius_x;
   if (ry) *ry = pd->radius_y;
   if (press) *press = pd->pressure;
   if (angle) *angle = pd->angle;
}


EOLIAN static void
_efl_pointer_event_touch_set(Eo *obj EINA_UNUSED, Efl_Pointer_Event_Data *pd, double r, double rx, double ry, double press, double angle)
{
   pd->radius = r;
   pd->radius_x = rx;
   pd->radius_y = ry;
   pd->pressure = press;
   pd->angle = angle;
}

#include "interfaces/efl_pointer_event.eo.c"

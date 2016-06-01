#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Evas.h>

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define MY_CLASS EFL_EVENT_POINTER_CLASS


/* Pointer Event
 *
 * This is a storage class only, should not require ANY knowledge about
 * Ecore, Evas or anything else. Note: locks & modifiers require evas. :(
 *
 * This is intended to replace Ecore and Evas structs for mouse events.
 *
 * Do not add any logic here.
 */

static Efl_Event_Pointer *s_cached_event = NULL;

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
        eo_del(evt);
     }
}

EOLIAN static Efl_Event_Pointer *
_efl_event_pointer_instance_get(Eo_Class *klass EINA_UNUSED, void *pd EINA_UNUSED,
                                Eo *owner, void **priv)
{
   Efl_Event_Pointer *evt;

   if (s_cached_event)
     {
        evt = s_cached_event;
        s_cached_event = NULL;
        efl_event_reset(evt);
        eo_parent_set(evt, owner);
     }
   else
     {
        evt = eo_add(EFL_EVENT_POINTER_CLASS, owner);
        eo_del_intercept_set(evt, _del_hook);
     }

   if (priv)
     *priv = eo_data_scope_get(evt, EFL_EVENT_POINTER_CLASS);

   return evt;
}

EOLIAN static void
_efl_event_pointer_class_destructor(Eo_Class *klass EINA_UNUSED)
{
   // this is a strange situation...
   eo_unref(s_cached_event);
   s_cached_event = NULL;
}

EOLIAN static Eo_Base *
_efl_event_pointer_eo_base_constructor(Eo *obj, Efl_Event_Pointer_Data *pd EINA_UNUSED)
{
   eo_constructor(eo_super(obj, MY_CLASS));
   efl_event_reset(obj);
   return obj;
}

EOLIAN static void
_efl_event_pointer_efl_event_reset(Eo *obj, Efl_Event_Pointer_Data *pd)
{
   memset(pd, 0, sizeof(*pd));
   pd->eo = obj;
   pd->wheel.dir = EFL_ORIENT_VERTICAL;
}

EOLIAN static Efl_Event *
_efl_event_pointer_efl_event_dup(Eo *obj, Efl_Event_Pointer_Data *pd)
{
   Efl_Event_Pointer_Data *ev;
   Efl_Event_Pointer *evt;

   evt = _efl_event_pointer_instance_get((Eo_Class *) EFL_EVENT_POINTER_CLASS,
                                         NULL, obj, (void **) &ev);
   if (!evt) return NULL;

   memcpy(ev, pd, sizeof(*ev));
   ev->eo = evt;

   return evt;
}

EOLIAN static void
_efl_event_pointer_action_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, Efl_Pointer_Action act)
{
   pd->action = act;
}

EOLIAN static Efl_Pointer_Action
_efl_event_pointer_action_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd)
{
   return pd->action;
}

EOLIAN static void
_efl_event_pointer_button_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, int but)
{
   pd->button = but;
}

EOLIAN static int
_efl_event_pointer_button_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd)
{
   return pd->button;
}

EOLIAN static void
_efl_event_pointer_button_pressed_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, int button, Eina_Bool pressed)
{
   if (button < 0) return;
   if (button > 31) return;
   if (pressed)
     pd->button_flags |= (1 << button);
   else
     pd->button_flags &= ~(1 << button);
}

EOLIAN static Eina_Bool
_efl_event_pointer_button_pressed_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, int button)
{
   if (button < 0) return EINA_FALSE;
   if (button > 31) return EINA_FALSE;
   return (pd->button_flags & (1 << button)) != 0;
}

EOLIAN static void
_efl_event_pointer_position_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, int x, int y)
{
   pd->cur.x = (double) x;
   pd->cur.y = (double) y;
}

EOLIAN static void
_efl_event_pointer_position_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, int *x, int *y)
{
   if (x) *x = (int) pd->cur.x;
   if (y) *y = (int) pd->cur.y;
}

EOLIAN static void
_efl_event_pointer_position_precise_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, double x, double y)
{
   pd->cur.x = x;
   pd->cur.y = y;
}

EOLIAN static void
_efl_event_pointer_position_precise_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, double *x, double *y)
{
   if (x) *x = pd->cur.x;
   if (y) *y = pd->cur.y;
}

EOLIAN static void
_efl_event_pointer_previous_position_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, int x, int y)
{
   pd->prev.x = (double) x;
   pd->prev.y = (double) y;
}

EOLIAN static void
_efl_event_pointer_previous_position_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, int *x, int *y)
{
   if (x) *x = (int) pd->prev.x;
   if (y) *y = (int) pd->prev.y;
}

EOLIAN static void
_efl_event_pointer_previous_position_precise_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, double x, double y)
{
   pd->prev.x = x;
   pd->prev.y = y;
}

EOLIAN static void
_efl_event_pointer_previous_position_precise_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, double *x, double *y)
{
   if (x) *x = pd->prev.x;
   if (y) *y = pd->prev.y;
}

EOLIAN static void
_efl_event_pointer_efl_event_input_device_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, Efl_Input_Device *dev)
{
   /* ref? */
   pd->device = dev;
}

EOLIAN static Efl_Input_Device *
_efl_event_pointer_efl_event_input_device_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd)
{
   return pd->device;
}

EOLIAN static void
_efl_event_pointer_source_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, Efl_Gfx *src)
{
   /* ref? */
   pd->source = src;
}

EOLIAN static Efl_Gfx *
_efl_event_pointer_source_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd)
{
   return pd->source;
}

EOLIAN static void
_efl_event_pointer_button_flags_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, Efl_Pointer_Flags flags)
{
   pd->button_flags = flags;
}

EOLIAN static Efl_Pointer_Flags
_efl_event_pointer_button_flags_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd)
{
   return pd->button_flags;
}

EOLIAN static void
_efl_event_pointer_efl_event_input_event_flags_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, Efl_Event_Flags flags)
{
   pd->event_flags = flags;
}

EOLIAN static Efl_Event_Flags
_efl_event_pointer_efl_event_input_event_flags_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd)
{
   return pd->event_flags;
}

EOLIAN static void
_efl_event_pointer_efl_event_timestamp_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, double ms)
{
   pd->timestamp = (unsigned int) ms;
}

EOLIAN static double
_efl_event_pointer_efl_event_timestamp_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd)
{
   return (double) pd->timestamp;
}

EOLIAN static void
_efl_event_pointer_wheel_direction_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, Efl_Orient dir)
{
   pd->wheel.dir = dir;
}

EOLIAN static Efl_Orient
_efl_event_pointer_wheel_direction_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd)
{
   return pd->wheel.dir;
}

EOLIAN static void
_efl_event_pointer_wheel_distance_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, int dist)
{
   pd->wheel.z = dist;
}

EOLIAN static int
_efl_event_pointer_wheel_distance_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd)
{
   return pd->wheel.z;
}

EOLIAN static int
_efl_event_pointer_finger_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd)
{
   return pd->finger;
}

EOLIAN static void
_efl_event_pointer_finger_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, int id)
{
   pd->finger = id;
}

EOLIAN static void
_efl_event_pointer_touch_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, double *r, double *rx, double *ry, double *press, double *angle)
{
   if (r) *r = pd->radius;
   if (rx) *rx = pd->radius_x;
   if (ry) *ry = pd->radius_y;
   if (press) *press = pd->pressure;
   if (angle) *angle = pd->angle;
}

EOLIAN static void
_efl_event_pointer_touch_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, double r, double rx, double ry, double press, double angle)
{
   pd->radius = r;
   pd->radius_x = rx;
   pd->radius_y = ry;
   pd->pressure = press;
   pd->angle = angle;
}

EOLIAN static Eina_Bool
_efl_event_pointer_efl_input_state_modifier_enabled_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, const char *name)
{
   if (!pd->modifiers) return EINA_FALSE;
   return evas_key_modifier_is_set(pd->modifiers, name);
}

EOLIAN static Eina_Bool
_efl_event_pointer_efl_input_state_lock_enabled_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, const char *name)
{
   if (!pd->locks) return EINA_FALSE;
   return evas_key_lock_is_set(pd->locks, name);
}

EOLIAN static void
_efl_event_pointer_double_click_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, Eina_Bool val)
{
   if (val)
     pd->button_flags |= EFL_POINTER_FLAGS_DOUBLE_CLICK;
   else
     pd->button_flags &= ~EFL_POINTER_FLAGS_DOUBLE_CLICK;
}

EOLIAN static Eina_Bool
_efl_event_pointer_double_click_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd)
{
   return !!(pd->button_flags & EFL_POINTER_FLAGS_DOUBLE_CLICK);
}

EOLIAN static void
_efl_event_pointer_triple_click_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, Eina_Bool val)
{
   if (val)
     pd->button_flags |= EFL_POINTER_FLAGS_TRIPLE_CLICK;
   else
     pd->button_flags &= ~EFL_POINTER_FLAGS_TRIPLE_CLICK;
}

EOLIAN static Eina_Bool
_efl_event_pointer_triple_click_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd)
{
   return !!(pd->button_flags & EFL_POINTER_FLAGS_TRIPLE_CLICK);
}

EOLIAN static void
_efl_event_pointer_efl_event_input_on_hold_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, Eina_Bool val)
{
   if (val)
     pd->event_flags |= EFL_EVENT_FLAGS_ON_HOLD;
   else
     pd->event_flags &= ~EFL_EVENT_FLAGS_ON_HOLD;
}

EOLIAN static Eina_Bool
_efl_event_pointer_efl_event_input_on_hold_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd)
{
   return !!(pd->event_flags & EFL_EVENT_FLAGS_ON_HOLD);
}

EOLIAN static void
_efl_event_pointer_efl_event_input_on_scroll_set(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd, Eina_Bool val)
{
   if (val)
     pd->event_flags |= EFL_EVENT_FLAGS_ON_SCROLL;
   else
     pd->event_flags &= ~EFL_EVENT_FLAGS_ON_SCROLL;
}

EOLIAN static Eina_Bool
_efl_event_pointer_efl_event_input_on_scroll_get(Eo *obj EINA_UNUSED, Efl_Event_Pointer_Data *pd)
{
   return !!(pd->event_flags & EFL_EVENT_FLAGS_ON_SCROLL);
}

#include "efl_event_pointer.eo.c"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_EVENT_PROTECTED

#include <Evas.h>

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define MY_CLASS EFL_EVENT_HOLD_CLASS

EOLIAN static void
_efl_event_hold_hold_set(Eo *obj EINA_UNUSED, Efl_Event_Hold_Data *pd, Eina_Bool val)
{
   pd->hold = !!val;
}

EOLIAN static Eina_Bool
_efl_event_hold_hold_get(Eo *obj EINA_UNUSED, Efl_Event_Hold_Data *pd)
{
   return pd->hold;
}

EOLIAN static void
_efl_event_hold_efl_event_input_device_set(Eo *obj EINA_UNUSED, Efl_Event_Hold_Data *pd, Efl_Input_Device *dev)
{
   pd->device = dev;
}

EOLIAN static Efl_Input_Device *
_efl_event_hold_efl_event_input_device_get(Eo *obj EINA_UNUSED, Efl_Event_Hold_Data *pd)
{
   return pd->device;
}

EOLIAN static double
_efl_event_hold_efl_event_timestamp_get(Eo *obj EINA_UNUSED, Efl_Event_Hold_Data *pd)
{
   return pd->timestamp;
}

EOLIAN static void
_efl_event_hold_efl_event_timestamp_set(Eo *obj EINA_UNUSED, Efl_Event_Hold_Data *pd, double ms)
{
   pd->timestamp = ms;
}

EOLIAN static void
_efl_event_hold_efl_event_input_event_flags_set(Eo *obj EINA_UNUSED, Efl_Event_Hold_Data *pd EINA_UNUSED, Efl_Event_Flags flags EINA_UNUSED)
{
   // ignore
}

EOLIAN static Efl_Event_Flags
_efl_event_hold_efl_event_input_event_flags_get(Eo *obj EINA_UNUSED, Efl_Event_Hold_Data *pd EINA_UNUSED)
{
   // ignore
   return 0;
}

EOLIAN static Eo *
_efl_event_hold_eo_base_constructor(Eo *obj, Efl_Event_Hold_Data *pd)
{
   obj = eo_constructor(eo_super(obj, MY_CLASS));
   pd->eo = obj;
   return obj;
}

EOLIAN static Efl_Event *
_efl_event_hold_efl_event_instance_get(Eo *klass EINA_UNUSED, void *_pd EINA_UNUSED,
                                       Eo_Base *owner, void **priv)
{
   // TODO: Implement a cache. Depends only on how many hold events we trigger.
   Efl_Event *evt = eo_add(MY_CLASS, owner);
   if (priv) *priv = eo_data_scope_get(evt, MY_CLASS);
   return evt;
}

EOLIAN static void
_efl_event_hold_efl_event_reset(Eo *obj, Efl_Event_Hold_Data *pd)
{
   memset(pd, 0, sizeof(*pd));
   pd->eo = obj;
}

EOLIAN static Efl_Event *
_efl_event_hold_efl_event_dup(Eo *obj, Efl_Event_Hold_Data *pd)
{
   Efl_Event_Hold_Data *ev;
   Efl_Event *evt = eo_add(EFL_EVENT_HOLD_CLASS, eo_parent_get(obj));
   ev = eo_data_scope_get(evt, MY_CLASS);
   if (ev)
     {
        memcpy(ev, pd, sizeof(*ev));
        ev->eo = evt;
     }
   return evt;
}

#include "efl_event_hold.eo.c"

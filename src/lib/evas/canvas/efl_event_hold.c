#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

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
_efl_event_hold_device_set(Eo *obj EINA_UNUSED, Efl_Event_Hold_Data *pd, Efl_Input_Device *dev)
{
   pd->device = dev;
}

EOLIAN static Efl_Input_Device *
_efl_event_hold_device_get(Eo *obj EINA_UNUSED, Efl_Event_Hold_Data *pd)
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

EOLIAN static Eo *
_efl_event_hold_eo_base_constructor(Eo *obj, Efl_Event_Hold_Data *pd)
{
   obj = eo_constructor(eo_super(obj, MY_CLASS));
   pd->eo = obj;
   return obj;
}

#include "efl_event_hold.eo.c"

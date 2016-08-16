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
_efl_event_hold_efl_object_constructor(Eo *obj, Efl_Event_Hold_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_event_reset(obj);
   return obj;
}

static inline void
_efl_event_hold_free(Efl_Event_Hold_Data *pd)
{
   free(pd->legacy);
}

EOLIAN static void
_efl_event_hold_efl_object_destructor(Eo *obj, Efl_Event_Hold_Data *pd)
{
   _efl_event_hold_free(pd);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Event *
_efl_event_hold_efl_event_instance_get(Eo *klass EINA_UNUSED, void *_pd EINA_UNUSED,
                                       Efl_Object *owner, void **priv)
{
   // TODO: Implement a cache. Depends only on how many hold events we trigger.
   Efl_Event *evt = efl_add(MY_CLASS, owner);
   if (priv) *priv = efl_data_scope_get(evt, MY_CLASS);
   return evt;
}

EOLIAN static void
_efl_event_hold_efl_event_reset(Eo *obj, Efl_Event_Hold_Data *pd)
{
   _efl_event_hold_free(pd);
   memset(pd, 0, sizeof(*pd));
   pd->eo = obj;
}

EOLIAN static Efl_Event *
_efl_event_hold_efl_event_dup(Eo *obj, Efl_Event_Hold_Data *pd)
{
   Efl_Event_Hold_Data *ev;
   Efl_Event *evt = efl_add(EFL_EVENT_HOLD_CLASS, efl_parent_get(obj));
   ev = efl_data_scope_get(evt, MY_CLASS);
   if (!ev) return NULL;

   ev->eo = evt;
   ev->timestamp = pd->timestamp;
   ev->data = pd->data;
   ev->hold = pd->hold;
   ev->device = pd->device; // lacks a proper ref :(

   return evt;
}

#include "efl_event_hold.eo.c"

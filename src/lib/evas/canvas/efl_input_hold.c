#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_INPUT_EVENT_PROTECTED

#include <Evas.h>

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define MY_CLASS EFL_INPUT_HOLD_CLASS

EOLIAN static void
_efl_input_hold_hold_set(Eo *obj EINA_UNUSED, Efl_Input_Hold_Data *pd, Eina_Bool val)
{
   pd->hold = !!val;
}

EOLIAN static Eina_Bool
_efl_input_hold_hold_get(Eo *obj EINA_UNUSED, Efl_Input_Hold_Data *pd)
{
   return pd->hold;
}

EOLIAN static void
_efl_input_hold_efl_input_event_device_set(Eo *obj EINA_UNUSED, Efl_Input_Hold_Data *pd, Efl_Input_Device *dev)
{
   pd->device = efl_ref(dev);
}

EOLIAN static Efl_Input_Device *
_efl_input_hold_efl_input_event_device_get(Eo *obj EINA_UNUSED, Efl_Input_Hold_Data *pd)
{
   return pd->device;
}

EOLIAN static double
_efl_input_hold_efl_input_event_timestamp_get(Eo *obj EINA_UNUSED, Efl_Input_Hold_Data *pd)
{
   return pd->timestamp;
}

EOLIAN static void
_efl_input_hold_efl_input_event_timestamp_set(Eo *obj EINA_UNUSED, Efl_Input_Hold_Data *pd, double ms)
{
   pd->timestamp = ms;
}

EOLIAN static void
_efl_input_hold_efl_input_event_event_flags_set(Eo *obj EINA_UNUSED, Efl_Input_Hold_Data *pd, Efl_Input_Flags flags)
{
   pd->event_flags = flags;
}

EOLIAN static Efl_Input_Flags
_efl_input_hold_efl_input_event_event_flags_get(Eo *obj EINA_UNUSED, Efl_Input_Hold_Data *pd)
{
   return pd->event_flags;
}

EOLIAN static Eo *
_efl_input_hold_efl_object_constructor(Eo *obj, Efl_Input_Hold_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_input_reset(obj);
   return obj;
}

static inline void
_efl_input_hold_free(Efl_Input_Hold_Data *pd)
{
   efl_unref(pd->device);
   free(pd->legacy);
}

EOLIAN static void
_efl_input_hold_efl_object_destructor(Eo *obj, Efl_Input_Hold_Data *pd)
{
   _efl_input_hold_free(pd);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Input_Event *
_efl_input_hold_efl_input_event_instance_get(Eo *klass EINA_UNUSED, void *_pd EINA_UNUSED,
                                             Efl_Object *owner, void **priv)
{
   // TODO: Implement a cache. Depends only on how many hold events we trigger.
   Efl_Input_Event *evt = efl_add(MY_CLASS, owner);
   if (priv) *priv = efl_data_scope_get(evt, MY_CLASS);
   return evt;
}

EOLIAN static void
_efl_input_hold_efl_input_event_reset(Eo *obj, Efl_Input_Hold_Data *pd)
{
   _efl_input_hold_free(pd);
   memset(pd, 0, sizeof(*pd));
   pd->eo = obj;
}

EOLIAN static Efl_Input_Event *
_efl_input_hold_efl_input_event_dup(Eo *obj, Efl_Input_Hold_Data *pd)
{
   Efl_Input_Hold_Data *ev;
   Efl_Input_Event *evt = efl_add(EFL_INPUT_HOLD_CLASS, efl_parent_get(obj));
   ev = efl_data_scope_get(evt, MY_CLASS);
   if (!ev) return NULL;

   ev->eo = evt;
   ev->timestamp = pd->timestamp;
   ev->data = pd->data;
   ev->hold = pd->hold;
   ev->device = efl_ref(pd->device);

   return evt;
}

#include "efl_input_hold.eo.c"

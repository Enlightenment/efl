#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"

#define EFL_INPUT_EVENT_PROTECTED

#include <Evas.h>

#include <Evas_Internal.h>

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define MY_CLASS EFL_INPUT_FOCUS_CLASS

static void
_efl_input_focus_free(Efl_Input_Focus_Data *pd)
{
   efl_wref_del_safe(&pd->object_wref);
   efl_unref(pd->device);
}

EOLIAN static Efl_Object *
_efl_input_focus_efl_object_constructor(Eo *obj,
                                        Efl_Input_Focus_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   return obj;
}

EOLIAN static void
_efl_input_focus_efl_object_destructor(Eo *obj,
                                       Efl_Input_Focus_Data *pd)
{
   _efl_input_focus_free(pd);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_input_focus_object_set(Eo *obj EINA_UNUSED, Efl_Input_Focus_Data *pd,
                            Efl_Object *object)
{
   pd->object_wref = object;
}

EOLIAN static Efl_Object *
_efl_input_focus_object_get(const Eo *obj EINA_UNUSED, Efl_Input_Focus_Data *pd)
{
   return pd->object_wref;
}

EOLIAN static void
_efl_input_focus_efl_input_event_device_set(Eo *obj EINA_UNUSED,
                                            Efl_Input_Focus_Data *pd,
                                            Efl_Input_Device *device)
{
   efl_replace(&pd->device, device);
}

EOLIAN static Efl_Input_Device *
_efl_input_focus_efl_input_event_device_get(const Eo *obj EINA_UNUSED,
                                            Efl_Input_Focus_Data *pd)
{
   return pd->device;
}

EOLIAN static void
_efl_input_focus_efl_input_event_timestamp_set(Eo *obj EINA_UNUSED,
                                               Efl_Input_Focus_Data *pd,
                                               double ms)
{
   pd->timestamp = ms;
}

EOLIAN static double
_efl_input_focus_efl_input_event_timestamp_get(const Eo *obj EINA_UNUSED,
                                               Efl_Input_Focus_Data *pd)
{
   return pd->timestamp;
}

EOLIAN static Efl_Input_Focus *
_efl_input_focus_efl_duplicate_duplicate(const Eo *obj, Efl_Input_Focus_Data *pd)
{
   Efl_Input_Focus_Data *ev;
   Efl_Input_Focus *evt;

   evt = efl_add(MY_CLASS, efl_parent_get(obj),
                 efl_allow_parent_unref_set(efl_added, EINA_TRUE));
   ev = efl_data_scope_get(evt, MY_CLASS);
   if (!ev) return NULL;

   memcpy(ev, pd, sizeof(*ev));
   ev->eo        = evt;
   ev->device    = efl_ref(pd->device);
   efl_wref_add(ev->object_wref, &ev->object_wref);

   return evt;
}

EOLIAN static Efl_Input_Focus *
_efl_input_focus_efl_input_event_instance_get(Eo *klass, void *_pd EINA_UNUSED,
                                              Eo *owner, void **priv)
{
   Efl_Input_Focus_Data *ev;
   Efl_Input_Focus *evt;

   evt = efl_input_event_instance_get(klass, owner);
   if (!evt) return NULL;

   ev = efl_data_scope_get(evt, MY_CLASS);
   if (priv) *priv = ev;
   return evt;
}

EOLIAN static void
_efl_input_focus_class_destructor(Efl_Class *klass)
{
   efl_input_event_instance_clean(klass);
}

EOLIAN static void
_efl_input_focus_efl_input_event_reset(Eo *obj, Efl_Input_Focus_Data *pd)
{
   _efl_input_focus_free(pd);
   memset(pd, 0, sizeof(Efl_Input_Focus_Data));
   pd->eo = obj;
}

EOLIAN static void
_efl_input_focus_efl_input_event_event_flags_set(Eo *obj EINA_UNUSED, Efl_Input_Focus_Data *pd, Efl_Input_Flags flags)
{
   if (flags == EFL_INPUT_FLAGS_SCROLLING)
     ERR("A focus event cannot be created based on scrolling");
   else
     pd->event_flags |= flags;
}

EOLIAN static Efl_Input_Flags
_efl_input_focus_efl_input_event_event_flags_get(const Eo *obj EINA_UNUSED, Efl_Input_Focus_Data *pd)
{
   return pd->event_flags;
}


/* Internal EO APIs */

#define EFL_INPUT_FOCUS_EXTRA_CLASS_OPS \
   EFL_OBJECT_OP_FUNC(efl_input_instance_get, _efl_input_focus_efl_input_event_instance_get)

#include "efl_input_focus.eo.c"

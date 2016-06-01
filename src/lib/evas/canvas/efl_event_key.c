#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Evas.h>

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define MY_CLASS EFL_EVENT_KEY_CLASS

EOLIAN static Eo_Base *
_efl_event_key_eo_base_constructor(Eo *obj, Efl_Event_Key_Data *pd EINA_UNUSED)
{
   obj = eo_constructor(eo_super(obj, MY_CLASS));
   efl_event_reset(obj);
   return obj;
}

EOLIAN static void
_efl_event_key_pressed_set(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, Eina_Bool val)
{
   pd->pressed = !!val;
}

EOLIAN static Eina_Bool
_efl_event_key_pressed_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   return pd->pressed;
}

EOLIAN static void
_efl_event_key_key_name_set(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->keyname, val);
}

EOLIAN static const char *
_efl_event_key_key_name_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   return pd->keyname;
}

EOLIAN static void
_efl_event_key_key_set(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->key, val);
}

EOLIAN static const char *
_efl_event_key_key_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   return pd->key;
}

EOLIAN static void
_efl_event_key_string_set(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->string, val);
}

EOLIAN static const char *
_efl_event_key_string_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   return pd->string;
}

EOLIAN static void
_efl_event_key_compose_set(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->compose, val);
}

EOLIAN static const char *
_efl_event_key_compose_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   return pd->compose;
}

EOLIAN static void
_efl_event_key_key_code_set(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, int val)
{
   pd->keycode = val;
}

EOLIAN static int
_efl_event_key_key_code_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   return pd->keycode;
}

EOLIAN static void
_efl_event_key_efl_event_reset(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   eina_stringshare_del(pd->key);
   eina_stringshare_del(pd->keyname);
   eina_stringshare_del(pd->string);
   eina_stringshare_del(pd->compose);
   memset(pd, 0, sizeof(*pd));
   pd->eo = obj;
}

EOLIAN static Efl_Event *
_efl_event_key_efl_event_dup(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   Efl_Event_Key_Data *pd2;
   Efl_Event *ev;

   ev = eo_add(MY_CLASS, NULL); // no parent
   pd2 = eo_data_scope_get(ev, MY_CLASS);
   if (pd2)
     {
        memcpy(pd2, pd, sizeof(*pd));
        pd2->eo        = ev;
        pd2->key       = eina_stringshare_add(pd->key);
        pd2->keyname   = eina_stringshare_add(pd->keyname);
        pd2->string    = eina_stringshare_add(pd->string);
        pd2->compose   = eina_stringshare_add(pd->compose);
        pd2->evas_done = 0;
     }

   return ev;
}

EOLIAN static void
_efl_event_key_efl_event_timestamp_set(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, double ms)
{
   pd->timestamp = ms;
}

EOLIAN static double
_efl_event_key_efl_event_timestamp_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   return pd->timestamp;
}

EOLIAN static void
_efl_event_key_efl_event_input_event_flags_set(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, Efl_Event_Flags flags)
{
   pd->event_flags = flags;
}

EOLIAN static Efl_Event_Flags
_efl_event_key_efl_event_input_event_flags_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   return pd->event_flags;
}

EOLIAN static void
_efl_event_key_efl_event_input_on_hold_set(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, Eina_Bool val)
{
   if (val)
     pd->event_flags |= EFL_EVENT_FLAGS_ON_HOLD;
   else
     pd->event_flags &= ~EFL_EVENT_FLAGS_ON_HOLD;
}

EOLIAN static Eina_Bool
_efl_event_key_efl_event_input_on_hold_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   return !!(pd->event_flags & EFL_EVENT_FLAGS_ON_HOLD);
}

EOLIAN static void
_efl_event_key_efl_event_input_on_scroll_set(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, Eina_Bool val)
{
   if (val)
     pd->event_flags |= EFL_EVENT_FLAGS_ON_SCROLL;
   else
     pd->event_flags &= ~EFL_EVENT_FLAGS_ON_SCROLL;
}

EOLIAN static Eina_Bool
_efl_event_key_efl_event_input_on_scroll_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   return !!(pd->event_flags & EFL_EVENT_FLAGS_ON_SCROLL);
}

EOLIAN static void
_efl_event_key_efl_event_input_device_set(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, Efl_Input_Device *dev)
{
   pd->device = dev;
}

EOLIAN static Efl_Input_Device *
_efl_event_key_efl_event_input_device_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   return pd->device;
}

EOLIAN static Eina_Bool
_efl_event_key_efl_input_state_modifier_enabled_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, const char * name)
{
   if (!pd->modifiers) return EINA_FALSE;
   return evas_key_modifier_is_set(pd->modifiers, name);
}

EOLIAN static Eina_Bool
_efl_event_key_efl_input_state_lock_enabled_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, const char * name)
{
   if (!pd->locks) return EINA_FALSE;
   return evas_key_lock_is_set(pd->locks, name);
}

#include "efl_event_key.eo.c"

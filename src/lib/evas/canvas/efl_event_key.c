#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_EVENT_PROTECTED

#include <Evas.h>

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define MY_CLASS EFL_EVENT_KEY_CLASS

static Efl_Event_Key *s_cached_event = NULL;

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

EOLIAN static Efl_Event_Key *
_efl_event_key_efl_event_instance_get(Eo *klass EINA_UNUSED, void *_pd EINA_UNUSED,
                                      Eo *owner, void **priv)
{
   Efl_Event_Key_Data *ev;
   Efl_Event_Key *evt;

   if (s_cached_event)
     {
        evt = s_cached_event;
        s_cached_event = NULL;
        efl_event_reset(evt);
        eo_parent_set(evt, owner);
     }
   else
     {
        evt = eo_add(EFL_EVENT_KEY_CLASS, owner);
        eo_del_intercept_set(evt, _del_hook);
     }

   ev = eo_data_scope_get(evt, EFL_EVENT_KEY_CLASS);
   ev->fake = EINA_FALSE;
   if (priv) *priv = ev;

   return evt;
}

EOLIAN static void
_efl_event_key_class_destructor(Eo_Class *klass EINA_UNUSED)
{
   // this is a strange situation...
   eo_del(s_cached_event);
   s_cached_event = NULL;
}

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
   pd->fake = EINA_TRUE;
}

EOLIAN static Efl_Event *
_efl_event_key_efl_event_dup(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   Efl_Event_Key_Data *ev;
   Efl_Event_Key *evt;

   // no parent
   evt = efl_event_instance_get(EFL_EVENT_KEY_CLASS, NULL, (void **) &ev);
   if (!evt || !ev) return NULL;

   memcpy(ev, pd, sizeof(*ev));
   ev->eo        = evt;
   ev->key       = eina_stringshare_add(pd->key);
   ev->keyname   = eina_stringshare_add(pd->keyname);
   ev->string    = eina_stringshare_add(pd->string);
   ev->compose   = eina_stringshare_add(pd->compose);
   ev->evas_done = 0;
   ev->win_fed   = 0;
   ev->fake      = 1;

   return evt;
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
_efl_event_key_efl_event_input_processed_set(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, Eina_Bool val)
{
   if (val)
     pd->event_flags |= EFL_EVENT_FLAGS_PROCESSED;
   else
     pd->event_flags &= ~EFL_EVENT_FLAGS_PROCESSED;
}

EOLIAN static Eina_Bool
_efl_event_key_efl_event_input_processed_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   return !!(pd->event_flags & EFL_EVENT_FLAGS_PROCESSED);
}

EOLIAN static void
_efl_event_key_efl_event_input_scrolling_set(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd, Eina_Bool val)
{
   if (val)
     pd->event_flags |= EFL_EVENT_FLAGS_SCROLLING;
   else
     pd->event_flags &= ~EFL_EVENT_FLAGS_SCROLLING;
}

EOLIAN static Eina_Bool
_efl_event_key_efl_event_input_scrolling_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   return !!(pd->event_flags & EFL_EVENT_FLAGS_SCROLLING);
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

EOLIAN static Eina_Bool
_efl_event_key_efl_event_input_fake_get(Eo *obj EINA_UNUSED, Efl_Event_Key_Data *pd)
{
   return pd->fake;
}

#include "efl_event_key.eo.c"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_INPUT_EVENT_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define MY_CLASS EFL_INPUT_KEY_CLASS

EOAPI Eo*
efl_input_key_instance_get(Efl_Object *owner, void **priv)
{
   Efl_Input_Key_Data *ev;
   Efl_Input_Key *evt;
   Evas *evas;

   evt = efl_input_event_instance_get(EFL_INPUT_KEY_CLASS, owner);
   if (!evt) return NULL;

   ev = efl_data_scope_get(evt, EFL_INPUT_KEY_CLASS);
   ev->fake = EINA_FALSE;
   if (priv) *priv = ev;

   evas = efl_provider_find(owner, EVAS_CANVAS_CLASS);
   if (evas)
     {
        Evas_Public_Data *e = efl_data_scope_get(evas, EVAS_CANVAS_CLASS);
        ev->modifiers = &e->modifiers;
        ev->locks = &e->locks;
     }

   return evt;
}

EOLIAN static void
_efl_input_key_class_destructor(Efl_Class *klass)
{
   efl_input_event_instance_clean(klass);
}

EOLIAN static Efl_Object *
_efl_input_key_efl_object_constructor(Eo *obj, Efl_Input_Key_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_input_reset(obj);
   return obj;
}

static inline void
_efl_input_key_free(Efl_Input_Key_Data *pd)
{
   free(pd->legacy);
   efl_unref(pd->device);
   if (pd->no_stringshare) return;
   eina_stringshare_del(pd->key);
   eina_stringshare_del(pd->keyname);
   eina_stringshare_del(pd->string);
   eina_stringshare_del(pd->compose);
}

EOLIAN static void
_efl_input_key_efl_object_destructor(Eo *obj, Efl_Input_Key_Data *pd)
{
   _efl_input_key_free(pd);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_input_key_pressed_set(Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd, Eina_Bool val)
{
   pd->pressed = !!val;
}

EOLIAN static Eina_Bool
_efl_input_key_pressed_get(const Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd)
{
   return pd->pressed;
}

EOLIAN static void
_efl_input_key_key_name_set(Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->keyname, val);
}

EOLIAN static const char *
_efl_input_key_key_name_get(const Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd)
{
   return pd->keyname;
}

EOLIAN static void
_efl_input_key_key_set(Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->key, val);
}

EOLIAN static const char *
_efl_input_key_key_get(const Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd)
{
   return pd->key;
}

EOLIAN static void
_efl_input_key_string_set(Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->string, val);
}

EOLIAN static const char *
_efl_input_key_string_get(const Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd)
{
   return pd->string;
}

EOLIAN static void
_efl_input_key_compose_set(Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->compose, val);
}

EOLIAN static const char *
_efl_input_key_compose_get(const Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd)
{
   return pd->compose;
}

EOLIAN static void
_efl_input_key_key_code_set(Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd, int val)
{
   pd->keycode = val;
}

EOLIAN static int
_efl_input_key_key_code_get(const Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd)
{
   return pd->keycode;
}

EOLIAN static void
_efl_input_key_efl_input_event_reset(Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd)
{
   _efl_input_key_free(pd);
   memset(pd, 0, sizeof(*pd));
   pd->eo = obj;
   pd->fake = EINA_TRUE;
}

EOLIAN static Efl_Input_Event *
_efl_input_key_efl_duplicate_duplicate(const Eo *obj, Efl_Input_Key_Data *pd)
{
   Efl_Input_Key_Data *ev;
   Efl_Input_Key *evt;

   evt = efl_add(MY_CLASS, efl_parent_get(obj),
                 efl_allow_parent_unref_set(efl_added, EINA_TRUE));
   ev = efl_data_scope_get(evt, MY_CLASS);
   if (!ev) return NULL;

   memcpy(ev, pd, sizeof(*ev));
   ev->eo        = evt;
   ev->legacy    = NULL;
   ev->key       = eina_stringshare_add(pd->key);
   ev->keyname   = eina_stringshare_add(pd->keyname);
   ev->string    = eina_stringshare_add(pd->string);
   ev->compose   = eina_stringshare_add(pd->compose);
   ev->evas_done = 0;
   ev->win_fed   = 0;
   ev->fake      = 1;
   ev->legacy    = NULL;
   ev->device    = efl_ref(pd->device);

   return evt;
}

EOLIAN static void
_efl_input_key_efl_input_event_timestamp_set(Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd, double ms)
{
   pd->timestamp = ms;
}

EOLIAN static double
_efl_input_key_efl_input_event_timestamp_get(const Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd)
{
   return pd->timestamp;
}

EOLIAN static void
_efl_input_key_efl_input_event_event_flags_set(Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd, Efl_Input_Flags flags)
{
   pd->event_flags = flags;
}

EOLIAN static Efl_Input_Flags
_efl_input_key_efl_input_event_event_flags_get(const Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd)
{
   return pd->event_flags;
}

EOLIAN static void
_efl_input_key_efl_input_event_device_set(Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd, Efl_Input_Device *dev)
{
   efl_replace(&pd->device, dev);
}

EOLIAN static Efl_Input_Device *
_efl_input_key_efl_input_event_device_get(const Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd)
{
   return pd->device;
}

EOLIAN static Eina_Bool
_efl_input_key_efl_input_state_modifier_enabled_get(const Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd,
                                                    Efl_Input_Modifier mod, const Efl_Input_Device *seat)
{
   const char *name;

   if (!pd->modifiers) return EINA_FALSE;
   if (!seat)
     {
        seat = efl_input_device_seat_get(pd->device);
        if (!seat) return EINA_FALSE;
     }
   name = _efl_input_modifier_to_string(mod);
   if (!name) return EINA_FALSE;
   return evas_seat_key_modifier_is_set(pd->modifiers, name, seat);
}

EOLIAN static Eina_Bool
_efl_input_key_efl_input_state_lock_enabled_get(const Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd,
                                                Efl_Input_Lock lock, const Efl_Input_Device *seat)
{
   const char *name;

   if (!pd->locks) return EINA_FALSE;
   if (!seat)
     {
        seat = efl_input_device_seat_get(pd->device);
        if (!seat) return EINA_FALSE;
     }
   name = _efl_input_lock_to_string(lock);
   if (!name) return EINA_FALSE;
   return evas_seat_key_lock_is_set(pd->locks, name, seat);
}

EOLIAN static Eina_Bool
_efl_input_key_efl_input_event_fake_get(const Eo *obj EINA_UNUSED, Efl_Input_Key_Data *pd)
{
   return pd->fake;
}

EOLIAN static void *
_efl_input_key_efl_input_event_legacy_info_get(Eo *obj, Efl_Input_Key_Data *pd)
{
   if (pd->legacy) return pd->legacy;
   return efl_input_key_legacy_info_fill(obj, NULL);
}

/* Internal EO APIs */

#define EFL_INPUT_KEY_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_input_legacy_info_get, _efl_input_key_efl_input_event_legacy_info_get)

#include "efl_input_key.eo.c"

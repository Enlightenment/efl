#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_INPUT_EVENT_PROTECTED

#include <Evas.h>
#include <Evas_Internal.h>
#include "canvas/evas_canvas_eo.h"

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define MY_CLASS EFL_INPUT_EVENT_MIXIN

EOLIAN static void
_efl_input_event_processed_set(Eo *obj, void *pd EINA_UNUSED, Eina_Bool val)
{
   if (val)
     efl_input_event_flags_set(obj, efl_input_event_flags_get(obj) | EFL_INPUT_FLAGS_PROCESSED);
   else
     efl_input_event_flags_set(obj, efl_input_event_flags_get(obj) & ~EFL_INPUT_FLAGS_PROCESSED);
}

EOLIAN static Eina_Bool
_efl_input_event_processed_get(const Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return !!(efl_input_event_flags_get(obj) & EFL_INPUT_FLAGS_PROCESSED);
}

EOLIAN static void
_efl_input_event_scrolling_set(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Eina_Bool val)
{
   if (val)
     efl_input_event_flags_set(obj, efl_input_event_flags_get(obj) | EFL_INPUT_FLAGS_SCROLLING);
   else
     efl_input_event_flags_set(obj, efl_input_event_flags_get(obj) & ~EFL_INPUT_FLAGS_SCROLLING);
}

EOLIAN static Eina_Bool
_efl_input_event_scrolling_get(const Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return !!(efl_input_event_flags_get(obj) & EFL_INPUT_FLAGS_SCROLLING);
}

EOLIAN static Eina_Bool
_efl_input_event_fake_get(const Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Efl_Object *
_efl_input_event_efl_object_provider_find(const Eo *obj, void *pd EINA_UNUSED, const Efl_Object *klass)
{
   // Note: provider_find should probably return self if self is a klass object
   if (klass == EVAS_CANVAS_CLASS)
     {
        for (Eo *parent = efl_parent_get(obj); parent; parent = efl_parent_get(parent))
          {
             if (efl_isa(parent, klass))
               return parent;
          }
     }
   return efl_provider_find(efl_super(obj, MY_CLASS), klass);
}

static Eina_Hash *_cached_events = NULL;

static void
_del_hook(Eo *evt)
{
   Efl_Input_Event *cached;
   const Eo *klass = efl_class_get(evt);

   efl_del_intercept_set(evt, NULL);

   cached = eina_hash_find(_cached_events, &klass);
   if (!cached)
     {
        efl_reuse(evt);
        eina_hash_add(_cached_events, &klass, evt);
        efl_input_reset(evt);
     }
   else
     {
        efl_unref(evt);
     }
}

static void
_noref_death(void *data EINA_UNUSED, const Efl_Event *event)
{
   efl_event_callback_del(event->object, EFL_EVENT_NOREF, _noref_death, NULL);
   efl_del(event->object);
}

Efl_Input_Event *
efl_input_event_instance_get(const Eo *klass, Eo *owner)
{
   Efl_Input_Event *evt;

   if (efl_invalidated_get(owner)) return NULL;

   if (!_cached_events)
     _cached_events = eina_hash_pointer_new(EINA_FREE_CB(efl_unref));

   evt = eina_hash_find(_cached_events, &klass);
   if (evt)
     {
        // eina_hash_del will call efl_unref, so prevent the destruction of the object
        evt = efl_ref(evt);
        eina_hash_del(_cached_events, &klass, evt);
        efl_parent_set(evt, owner);
        efl_unref(evt); // Remove reference before turning on self destruction
        efl_del_intercept_set(evt, _del_hook);
        efl_event_callback_add(evt, EFL_EVENT_NOREF, _noref_death, NULL);
     }
   else
     {
        evt = efl_add(klass, owner);
        efl_event_callback_add(evt, EFL_EVENT_NOREF, _noref_death, NULL);
        efl_del_intercept_set(evt, _del_hook);
     }

   return efl_ref(evt);
}

void
efl_input_event_instance_clean(Eo *klass)
{
   if (!_cached_events) return ;

   eina_hash_del(_cached_events, &klass, NULL);
}

/* Internal EO APIs */

EOAPI EFL_FUNC_BODY_CONST(efl_input_legacy_info_get, void *, NULL)

#define EFL_INPUT_EVENT_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_input_legacy_info_get, NULL)

#include "efl_input_event.eo.c"
#include "efl_input_state.eo.c"
#include "efl_input_interface.eo.c"

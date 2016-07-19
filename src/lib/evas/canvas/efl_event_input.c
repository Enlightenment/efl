#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Evas.h>

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define MY_CLASS EFL_EVENT_INPUT_MIXIN

EOLIAN static void
_efl_event_input_processed_set(Eo *obj, void *pd EINA_UNUSED, Eina_Bool val)
{
   if (val)
     efl_event_flags_set(obj, efl_event_flags_get(obj) | EFL_EVENT_FLAGS_PROCESSED);
   else
     efl_event_flags_set(obj, efl_event_flags_get(obj) & ~EFL_EVENT_FLAGS_PROCESSED);
}

EOLIAN static Eina_Bool
_efl_event_input_processed_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return !!(efl_event_flags_get(obj) & EFL_EVENT_FLAGS_PROCESSED);
}

EOLIAN static void
_efl_event_input_scrolling_set(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Eina_Bool val)
{
   if (val)
     efl_event_flags_set(obj, efl_event_flags_get(obj) | EFL_EVENT_FLAGS_SCROLLING);
   else
     efl_event_flags_set(obj, efl_event_flags_get(obj) & ~EFL_EVENT_FLAGS_SCROLLING);
}

EOLIAN static Eina_Bool
_efl_event_input_scrolling_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return !!(efl_event_flags_get(obj) & EFL_EVENT_FLAGS_SCROLLING);
}

EOLIAN static Eina_Bool
_efl_event_input_fake_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

#include "efl_event_input.eo.c"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_INPUT_EVENT_PROTECTED

#include <Evas.h>

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
_efl_input_event_processed_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
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
_efl_input_event_scrolling_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return !!(efl_input_event_flags_get(obj) & EFL_INPUT_FLAGS_SCROLLING);
}

EOLIAN static Eina_Bool
_efl_input_event_fake_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

#include "efl_input_event.eo.c"
#include "efl_input_state.eo.c"
#include "efl_input_interface.eo.c"

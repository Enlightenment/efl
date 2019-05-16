#include "libefl_mono_native_test.h"

typedef struct Dummy_Event_Manager_Data
{
   Eo* emitter;
} Dummy_Event_Manager_Data;

static Efl_Object*
_dummy_event_manager_efl_object_constructor(Eo *obj, EINA_UNUSED Dummy_Event_Manager_Data *pd)
{
   efl_constructor(efl_super(obj, DUMMY_EVENT_MANAGER_CLASS));

   return obj;
}

static void
_dummy_event_manager_efl_object_destructor(Eo *obj, Dummy_Event_Manager_Data *pd)
{
   if (pd->emitter != 0)
     efl_unref(pd->emitter);

   efl_destructor(efl_super(obj, DUMMY_EVENT_MANAGER_CLASS));
}

static void
_dummy_event_manager_emitter_set(EINA_UNUSED Eo *obj, Dummy_Event_Manager_Data *pd, Eo *emitter)
{
   pd->emitter = emitter;
}

static Eina_Bool
_dummy_event_manager_emit_with_int(EINA_UNUSED Eo *obj, Dummy_Event_Manager_Data *pd, int data)
{
   if (pd->emitter)
     efl_event_callback_call(pd->emitter, DUMMY_TEST_OBJECT_EVENT_EVT_WITH_INT, &data);
   else
     {
        EINA_LOG_ERR("Trying to emit event without an emitter.");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_dummy_event_manager_release(EINA_UNUSED Eo *obj, Dummy_Event_Manager_Data *pd)
{
   if (!pd->emitter)
     return;

   efl_unref(pd->emitter);
}


#include "dummy_event_manager.eo.c"

#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define EFL_ACCESS_BRIDGE_PROTECTED

#include <Eo.h>
#include <Elementary.h>
#include "elm_priv.h"

static Eina_Inarray bridges;

EOLIAN void
_efl_access_bridge_access_event_emit(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Access_Object *object, const Efl_Event_Description *event, void *data)
{
   Efl_Access_Bridge **bridge;

   EINA_INARRAY_FOREACH(&bridges, bridge)
      efl_access_bridge_access_event_handle(*bridge, object, event, data);
}

static int
_eo_pointer_compare(const void *a, const void *b)
{
   return memcmp(a, b, sizeof(Eo*));
}

EOLIAN Eo*
_efl_access_bridge_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, EFL_ACCESS_BRIDGE_CLASS));
   if (!obj) return NULL;

   if (eina_inarray_search(&bridges, &obj, _eo_pointer_compare) == -1)
      eina_inarray_push(&bridges, &obj);

   return obj;
}

EOLIAN void
_efl_access_bridge_efl_object_destructor(Eo *obj, void *pd EINA_UNUSED)
{
   eina_inarray_remove(&bridges, &obj);
   efl_destructor(efl_super(obj, EFL_ACCESS_BRIDGE_CLASS));
}

EOLIAN void
_efl_access_bridge_class_constructor(Efl_Class *cls EINA_UNUSED)
{
   eina_inarray_step_set(&bridges, sizeof(Eina_Inarray), sizeof(Eo*), 0);
}

#include "efl_access_bridge.eo.c"

#include <Eina.h>
#include "eolian_database.h"

EAPI Eina_Stringshare *
eolian_event_name_get(const Eolian_Event *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, NULL);
   return event->name;
}

EAPI const Eolian_Type *
eolian_event_type_get(const Eolian_Event *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, NULL);
   return event->type;
}

EAPI Eina_Stringshare *
eolian_event_description_get(const Eolian_Event *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, NULL);
   return event->comment;
}

EAPI Eolian_Object_Scope
eolian_event_scope_get(const Eolian_Event *event)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, EOLIAN_SCOPE_PUBLIC);
   return event->scope;
}

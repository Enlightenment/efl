#include <Eina.h>
#include "eolian_database.h"

EAPI Eina_Bool
eolian_class_event_information_get(const Eolian_Event *event, const char **event_name, const Eolian_Type **event_type, const char **event_comment)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, EINA_FALSE);
   if (event_name) *event_name = event->name;
   if (event_type) *event_type = event->type;
   if (event_comment) *event_comment = event->comment;
   return EINA_TRUE;
}

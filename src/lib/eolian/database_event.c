#include <Eina.h>
#include "eolian_database.h"

Eolian_Event *
database_event_new(const char *event_name, const char *event_type, const char *event_comment)
{
   if (!event_name) return NULL;
   Eolian_Event *event_desc = calloc(1, sizeof(Eolian_Event));
   if (!event_desc) return NULL;
   event_desc->name = eina_stringshare_add(event_name);
   if (event_type) event_desc->type = eina_stringshare_add(event_type);
   event_desc->comment = eina_stringshare_add(event_comment);
   return event_desc;
}

void
database_event_free(Eolian_Event *event)
{
   eina_stringshare_del(event->name);
   eina_stringshare_del(event->comment);
   free(event);
}

EAPI Eina_Bool
eolian_class_event_information_get(const Eolian_Event *event, const char **event_name, const char **event_type, const char **event_comment)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(event, EINA_FALSE);
   if (event_name) *event_name = event->name;
   if (event_type) *event_type = event->type;
   if (event_comment) *event_comment = event->comment;
   return EINA_TRUE;
}

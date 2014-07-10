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

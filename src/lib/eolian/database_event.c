#include <Eina.h>
#include "eolian_database.h"

void
database_event_del(Eolian_Event *event)
{
   if (!event) return;
   if (event->name) eina_stringshare_del(event->name);
   if (event->comment) eina_stringshare_del(event->comment);
   database_type_del(event->type);
   free(event);
}

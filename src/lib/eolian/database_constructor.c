#include <Eina.h>
#include "eolian_database.h"

void
database_constructor_del(Eolian_Constructor *ctor)
{
   if (!ctor) return;
   if (ctor->base.file) eina_stringshare_del(ctor->base.file);
   if (ctor->full_name) eina_stringshare_del(ctor->full_name);
   free(ctor);
}

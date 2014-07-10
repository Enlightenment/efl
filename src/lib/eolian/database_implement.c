#include <Eina.h>
#include "eolian_database.h"

void
database_implement_del(Eolian_Implement *impl)
{
   if (!impl) return;
   if (impl->full_name) eina_stringshare_del(impl->full_name);
   free(impl);
}

#include <Eina.h>
#include "eolian_database.h"

Eolian_Implement *
database_implement_new(const char *impl_name)
{
   Eolian_Implement *impl_desc = calloc(1, sizeof(Eolian_Implement));
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl_desc, NULL);
   impl_desc->full_name = eina_stringshare_add(impl_name);
   return impl_desc;
}

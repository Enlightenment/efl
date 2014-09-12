#include <Eina.h>
#include "eolian_database.h"

void
database_implement_del(Eolian_Implement *impl)
{
   if (!impl) return;
   if (impl->base.file) eina_stringshare_del(impl->base.file);
   if (impl->full_name) eina_stringshare_del(impl->full_name);
   free(impl);
}

void
database_implement_constructor_add(Eolian_Implement *impl, const Eolian_Class *klass)
{
   if (eolian_function_is_constructor(impl->foo_id, impl->klass))
     database_function_constructor_add((Eolian_Function*)impl->foo_id, klass);
}

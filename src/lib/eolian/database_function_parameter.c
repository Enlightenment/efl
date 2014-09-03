#include <Eina.h>
#include "eolian_database.h"

Eolian_Function_Parameter *
database_parameter_add(Eolian_Type *type, Eolian_Expression *value,
                       const char *name, const char *description)
{
   Eolian_Function_Parameter *param = NULL;
   param = calloc(1, sizeof(*param));
   param->name = eina_stringshare_add(name);
   param->type = type;
   param->value = value;
   param->description = eina_stringshare_add(description);
   return param;
}

void
database_parameter_del(Eolian_Function_Parameter *pdesc)
{
   if (pdesc->base.file) eina_stringshare_del(pdesc->base.file);
   eina_stringshare_del(pdesc->name);

   database_type_del(pdesc->type);
   eina_stringshare_del(pdesc->description);
   free(pdesc);
}

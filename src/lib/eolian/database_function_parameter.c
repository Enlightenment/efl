#include <Eina.h>
#include "eolian_database.h"

Eolian_Function_Parameter *
database_parameter_add(Eolian_Type *type, const char *name, const char *description)
{
   Eolian_Function_Parameter *param = NULL;
   param = calloc(1, sizeof(*param));
   param->name = eina_stringshare_add(name);
   param->type = type;
   param->description = eina_stringshare_add(description);
   return param;
}

void
database_parameter_del(Eolian_Function_Parameter *pdesc)
{
   eina_stringshare_del(pdesc->name);

   database_type_del(pdesc->type);
   eina_stringshare_del(pdesc->description);
   free(pdesc);
}

void
database_parameter_const_attribute_set(Eolian_Function_Parameter *param, Eina_Bool is_get, Eina_Bool is_const)
{
   EINA_SAFETY_ON_NULL_RETURN(param);
   if (is_get)
      param->is_const_on_get = is_const;
   else
      param->is_const_on_set = is_const;
}

void
database_parameter_type_set(Eolian_Function_Parameter *param, Eolian_Type *types)
{
   EINA_SAFETY_ON_NULL_RETURN(param);
   param->type = types;
}

void
database_parameter_nonull_set(Eolian_Function_Parameter *param, Eina_Bool nonull)
{
   EINA_SAFETY_ON_NULL_RETURN(param);
   param->nonull = nonull;
}

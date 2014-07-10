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

EAPI const Eolian_Type *
eolian_parameter_type_get(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, NULL);
   return param->type;
}

EAPI Eina_Stringshare *
eolian_parameter_name_get(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, NULL);
   eina_stringshare_ref(param->name);
   return param->name;
}

/* Get parameter information */
EAPI void
eolian_parameter_information_get(const Eolian_Function_Parameter *param, Eolian_Parameter_Dir *param_dir, const Eolian_Type **type, const char **name, const char **description)
{
   EINA_SAFETY_ON_NULL_RETURN(param);
   if (param_dir) *param_dir = param->param_dir;
   if (type) *type = param->type;
   if (name) *name = param->name;
   if (description) *description = param->description;
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

EAPI Eina_Bool
eolian_parameter_const_attribute_get(const Eolian_Function_Parameter *param, Eina_Bool is_get)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, EINA_FALSE);
   if (is_get)
      return param->is_const_on_get;
   else
      return param->is_const_on_set;
}

void
database_parameter_nonull_set(Eolian_Function_Parameter *param, Eina_Bool nonull)
{
   EINA_SAFETY_ON_NULL_RETURN(param);
   param->nonull = nonull;
}

EAPI Eina_Bool
eolian_parameter_is_nonull(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, EINA_FALSE);
   return param->nonull;
}

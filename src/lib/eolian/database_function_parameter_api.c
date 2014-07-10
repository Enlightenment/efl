#include <Eina.h>
#include "eolian_database.h"

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

EAPI void
eolian_parameter_information_get(const Eolian_Function_Parameter *param, Eolian_Parameter_Dir *param_dir, const Eolian_Type **type, const char **name, const char **description)
{
   EINA_SAFETY_ON_NULL_RETURN(param);
   if (param_dir) *param_dir = param->param_dir;
   if (type) *type = param->type;
   if (name) *name = param->name;
   if (description) *description = param->description;
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

EAPI Eina_Bool
eolian_parameter_is_nonull(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, EINA_FALSE);
   return param->nonull;
}

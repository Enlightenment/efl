#include <Eina.h>
#include "eolian_database.h"

EAPI Eolian_Parameter_Dir
eolian_parameter_direction_get(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, EOLIAN_IN_PARAM);
   return param->param_dir;
}

EAPI const Eolian_Type *
eolian_parameter_type_get(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, NULL);
   return param->type;
}

EAPI const Eolian_Expression *
eolian_parameter_default_value_get(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, NULL);
   return param->value;
}

EAPI Eina_Stringshare *
eolian_parameter_name_get(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, NULL);
   return param->name;
}

EAPI Eina_Stringshare *
eolian_parameter_description_get(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, NULL);
   return param->description;
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

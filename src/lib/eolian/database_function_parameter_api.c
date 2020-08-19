#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"

EOLIAN_API Eolian_Parameter_Direction
eolian_parameter_direction_get(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, EOLIAN_PARAMETER_UNKNOWN);
   return param->param_dir;
}

EOLIAN_API const Eolian_Type *
eolian_parameter_type_get(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, NULL);
   return param->type;
}

EOLIAN_API const Eolian_Expression *
eolian_parameter_default_value_get(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, NULL);
   return param->value;
}

EOLIAN_API const Eolian_Documentation *
eolian_parameter_documentation_get(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, NULL);
   return param->doc;
}

EOLIAN_API Eina_Bool
eolian_parameter_is_optional(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, EINA_FALSE);
   return param->optional;
}

EOLIAN_API Eina_Bool
eolian_parameter_is_move(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, EINA_FALSE);
   return param->move;
}

EOLIAN_API Eina_Bool
eolian_parameter_is_by_ref(const Eolian_Function_Parameter *param)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param, EINA_FALSE);
   return param->by_ref;
}

EOLIAN_API Eina_Stringshare *
eolian_parameter_c_type_get(const Eolian_Function_Parameter *param_desc,
                            Eina_Bool as_return)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(param_desc, NULL);
   Eina_Strbuf *buf = eina_strbuf_new();
   database_type_to_str(param_desc->type, buf, NULL,
                        as_return ? EOLIAN_C_TYPE_RETURN : EOLIAN_C_TYPE_PARAM,
                        param_desc->by_ref);
   Eina_Stringshare *ret = eina_stringshare_add(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   return ret;
}
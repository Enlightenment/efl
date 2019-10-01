#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"

EAPI const Eolian_Documentation *
eolian_constant_documentation_get(const Eolian_Constant *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   return var->doc;
}

EAPI const Eolian_Type *
eolian_constant_type_get(const Eolian_Constant *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   return var->base_type;
}

EAPI const Eolian_Expression *
eolian_constant_value_get(const Eolian_Constant *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   return var->value;
}

EAPI Eina_Bool
eolian_constant_is_extern(const Eolian_Constant *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, EINA_FALSE);
   return var->is_extern;
}

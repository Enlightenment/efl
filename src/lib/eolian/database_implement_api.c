#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"

EAPI Eina_Stringshare *
eolian_implement_full_name_get(const Eolian_Implement *impl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, NULL);
   return impl->full_name;
}

EAPI const Eolian_Class *
eolian_implement_class_get(const Eolian_Implement *impl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, NULL);
   return impl->klass;
}

EAPI const Eolian_Function *
eolian_implement_function_get(const Eolian_Implement *impl,
                              Eolian_Function_Type   *func_type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, NULL);

   if (!impl->foo_id)
     return NULL; /* normally unreachable */

   if (!func_type)
     return impl->foo_id;

   if (impl->is_prop_get && impl->is_prop_set)
     *func_type = EOLIAN_PROPERTY;
   else if (impl->is_prop_get)
     *func_type = EOLIAN_PROP_GET;
   else if (impl->is_prop_set)
     *func_type = EOLIAN_PROP_SET;
   else
     *func_type = eolian_function_type_get(impl->foo_id);

   return impl->foo_id;
}

EAPI Eina_Bool
eolian_implement_is_auto(const Eolian_Implement *impl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, EINA_FALSE);
   return impl->is_auto;
}

EAPI Eina_Bool
eolian_implement_is_empty(const Eolian_Implement *impl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, EINA_FALSE);
   return impl->is_empty;
}

EAPI Eina_Bool
eolian_implement_is_virtual(const Eolian_Implement *impl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, EINA_FALSE);
   return impl->is_virtual;
}

EAPI Eina_Bool
eolian_implement_is_prop_get(const Eolian_Implement *impl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, EINA_FALSE);
   return impl->is_prop_get;
}

EAPI Eina_Bool
eolian_implement_is_prop_set(const Eolian_Implement *impl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, EINA_FALSE);
   return impl->is_prop_set;
}

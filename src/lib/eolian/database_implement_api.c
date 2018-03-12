#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"

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

EAPI const Eolian_Documentation *
eolian_implement_documentation_get(const Eolian_Implement *impl,
                                   Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, NULL);
   switch (ftype)
     {
      case EOLIAN_PROP_GET: return impl->get_doc; break;
      case EOLIAN_PROP_SET: return impl->set_doc; break;
      default: return impl->common_doc;
     }
}

EAPI Eina_Bool
eolian_implement_is_auto(const Eolian_Implement *impl, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_UNRESOLVED, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_PROPERTY, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_METHOD:
        return impl->get_auto && !impl->is_prop_get && !impl->is_prop_set;
      case EOLIAN_PROP_GET:
        return impl->get_auto && impl->is_prop_get;
      case EOLIAN_PROP_SET:
        return impl->set_auto && impl->is_prop_set;
      default:
        return EINA_FALSE;
     }
}

EAPI Eina_Bool
eolian_implement_is_empty(const Eolian_Implement *impl, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_UNRESOLVED, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_PROPERTY, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_METHOD:
        return impl->get_empty && !impl->is_prop_get && !impl->is_prop_set;
      case EOLIAN_PROP_GET:
        return impl->get_empty && impl->is_prop_get;
      case EOLIAN_PROP_SET:
        return impl->set_empty && impl->is_prop_set;
      default:
        return EINA_FALSE;
     }
}

EAPI Eina_Bool
eolian_implement_is_pure_virtual(const Eolian_Implement *impl, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_UNRESOLVED, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_PROPERTY, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_METHOD:
        return impl->get_pure_virtual && !impl->is_prop_get && !impl->is_prop_set;
      case EOLIAN_PROP_GET:
        return impl->get_pure_virtual && impl->is_prop_get;
      case EOLIAN_PROP_SET:
        return impl->set_pure_virtual && impl->is_prop_set;
      default:
        return EINA_FALSE;
     }
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

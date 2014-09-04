#include <Eina.h>
#include "eolian_database.h"

EAPI Eina_Stringshare *
eolian_implement_full_name_get(const Eolian_Implement *impl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, NULL);
   return impl->full_name;
}

static Eina_Bool
_fill_class(Eolian_Implement *impl)
{
   const Eolian_Class *class = NULL;
   if (impl->klass)
     return EINA_TRUE;
   if (!database_class_name_validate(impl->full_name, &class) || !class)
     return EINA_FALSE;
   impl->klass = class;
   return EINA_TRUE;
}

EAPI const Eolian_Class *
eolian_implement_class_get(const Eolian_Implement *impl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, NULL);
   if (!_fill_class((Eolian_Implement*)impl))
     return NULL;
   return impl->klass;
}

EAPI const Eolian_Function *
eolian_implement_function_get(const Eolian_Implement *impl,
                              Eolian_Function_Type   *func_type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, NULL);

   if (impl->foo_id)
     {
        if (!func_type)
          return impl->foo_id;

        if (impl->is_prop_get)
          *func_type = EOLIAN_PROP_GET;
        else if (impl->is_prop_set)
          *func_type = EOLIAN_PROP_SET;
        else
          *func_type = eolian_function_type_get(impl->foo_id);

        return impl->foo_id;
     }

   const Eolian_Class *klass = eolian_implement_class_get(impl);
   if (!klass)
     return NULL;

   const char *func_name = impl->full_name + strlen(klass->full_name) + 1;

   Eolian_Function_Type tp = EOLIAN_UNRESOLVED;

   if (impl->is_prop_get)
     tp = EOLIAN_PROP_GET;
   else if (impl->is_prop_set)
     tp = EOLIAN_PROP_SET;

   const Eolian_Function *fid = eolian_class_function_get_by_name(klass,
                                                                  func_name,
                                                                  tp);

   if (fid && tp == EOLIAN_UNRESOLVED && (fid->type == EOLIAN_PROP_GET
                                       || fid->type == EOLIAN_PROP_SET))
     {
        eina_log_print(_eolian_log_dom, EINA_LOG_LEVEL_ERR,
            impl->base.file, "", impl->base.line, "both get and set required "
              "for property '%s' at column %d", func_name, impl->base.column);
        return NULL;
     }

   if (func_type)
     {
        if (tp == EOLIAN_UNRESOLVED)
          *func_type = eolian_function_type_get(fid);
        else
          *func_type = tp;
     }

   ((Eolian_Implement*)impl)->foo_id = fid;

   return fid;
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

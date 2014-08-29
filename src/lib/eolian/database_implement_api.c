#include <Eina.h>
#include "eolian_database.h"

EAPI Eina_Stringshare *
eolian_implement_full_name_get(const Eolian_Implement *impl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl, NULL);
   return impl->full_name;
}

/*
 * ret false -> clash, class = NULL
 * ret true && class -> only one class corresponding
 * ret true && !class -> no class corresponding
 */
static Eina_Bool
_class_name_validate(const char *class_name, const Eolian_Class **cl)
{
   char *name = strdup(class_name);
   char *colon = name + 1;
   const Eolian_Class *found_class = NULL;
   const Eolian_Class *candidate;
   if (cl) *cl = NULL;
   do
     {
        colon = strchr(colon, '.');
        if (colon) *colon = '\0';
        candidate = eolian_class_get_by_name(name);
        if (candidate)
          {
             if (found_class)
               {
                  ERR("Name clash between class %s and class %s",
                        candidate->full_name,
                        found_class->full_name);
                  free(name);
                  return EINA_FALSE; // Names clash
               }
             found_class = candidate;
          }
        if (colon) *colon++ = '.';
     }
   while(colon);
   if (cl) *cl = found_class;
   free(name);
   return EINA_TRUE;
}

static Eina_Bool
_fill_class(Eolian_Implement *impl)
{
   const Eolian_Class *class = NULL;
   if (impl->klass)
     return EINA_TRUE;
   if (!_class_name_validate(impl->full_name, &class) || !class)
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
   const Eolian_Class *klass = eolian_implement_class_get(impl);
   if (!klass)
     return NULL;

   char *func_name = strdup(impl->full_name + strlen(klass->full_name) + 1);
   char *dot = strchr(func_name, '.');

   Eolian_Function_Type tp = EOLIAN_UNRESOLVED;
   if (dot)
     {
        *dot = '\0';
        if      (!strcmp(dot + 1, "set")) tp = EOLIAN_PROP_SET;
        else if (!strcmp(dot + 1, "get")) tp = EOLIAN_PROP_GET;
     }

   const Eolian_Function *fid = eolian_class_function_get_by_name(klass,
                                                                  func_name,
                                                                  tp);

   if (fid && tp == EOLIAN_UNRESOLVED && (fid->type == EOLIAN_PROP_GET
                                       || fid->type == EOLIAN_PROP_SET))
     {
        eina_log_print(_eolian_log_dom, EINA_LOG_LEVEL_ERR,
            impl->base.file, "", impl->base.line, "both get and set required "
              "for property '%s' at column %d", func_name, impl->base.column);
        free(func_name);
        return NULL;
     }

   free(func_name);

   if (func_type)
     {
        if (tp == EOLIAN_UNRESOLVED)
          *func_type = eolian_function_type_get(fid);
        else
          *func_type = tp;
     }
   return fid;
}

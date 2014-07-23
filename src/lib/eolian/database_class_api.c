#include <Eina.h>
#include "eolian_database.h"

EAPI Eina_Stringshare *
eolian_class_file_get(const Eolian_Class *cl)
{
   return cl ? cl->file : NULL;
}

EAPI Eina_Stringshare *
eolian_class_full_name_get(const Eolian_Class *cl)
{
   return cl ? cl->full_name : NULL;
}

EAPI Eina_Stringshare *
eolian_class_name_get(const Eolian_Class *cl)
{
   return cl ? cl->name : NULL;
}

EAPI Eina_Iterator *
eolian_class_namespaces_list_get(const Eolian_Class *cl)
{
   return ((cl && cl->namespaces) ? eina_list_iterator_new(cl->namespaces)
                                  : NULL);
}

EAPI const Eolian_Class *
eolian_class_find_by_name(const char *class_name)
{
   Eina_List *itr;
   Eolian_Class *cl;
   Eina_Stringshare *shr_name = eina_stringshare_add(class_name);
   EINA_LIST_FOREACH(_classes, itr, cl)
      if (cl->full_name == shr_name) goto end;
   cl = NULL;
end:
   eina_stringshare_del(shr_name);
   return cl;
}

EAPI const Eolian_Class *
eolian_class_find_by_file(const char *file_name)
{
   Eina_List *itr;
   Eolian_Class *cl;
   Eina_Stringshare *shr_file = eina_stringshare_add(file_name);
   EINA_LIST_FOREACH(_classes, itr, cl)
      if (cl->file == shr_file) goto end;
   cl = NULL;
end:
   eina_stringshare_del(shr_file);
   return cl;
}

EAPI Eolian_Class_Type
eolian_class_type_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EOLIAN_CLASS_UNKNOWN_TYPE);
   return cl->type;
}

EAPI Eina_Iterator *
eolian_all_classes_list_get(void)
{
   return (_classes ? eina_list_iterator_new(_classes) : NULL);
}

EAPI Eina_Stringshare *
eolian_class_description_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->description;
}

EAPI Eina_Stringshare*
eolian_class_legacy_prefix_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->legacy_prefix;
}

EAPI Eina_Stringshare*
eolian_class_eo_prefix_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->eo_prefix;
}

EAPI Eina_Stringshare*
eolian_class_data_type_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->data_type;
}

EAPI Eina_Iterator *
eolian_class_inherits_list_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return (cl->inherits ? eina_list_iterator_new(cl->inherits) : NULL);
}

EAPI Eina_Iterator*
eolian_class_implements_list_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return (cl->implements ? eina_list_iterator_new(cl->implements) : NULL);
}

EAPI const Eolian_Function *
eolian_class_function_find_by_name(const Eolian_Class *cl, const char *func_name, Eolian_Function_Type f_type)
{
   Eina_List *itr;
   Eolian_Function *fid;
   if (!cl) return NULL;

   if (f_type == EOLIAN_UNRESOLVED || f_type == EOLIAN_METHOD)
      EINA_LIST_FOREACH(cl->methods, itr, fid)
        {
           if (!strcmp(fid->name, func_name))
              return fid;
        }

   if (f_type == EOLIAN_UNRESOLVED || f_type == EOLIAN_PROPERTY ||
         f_type == EOLIAN_PROP_SET || f_type == EOLIAN_PROP_GET)
     {
        EINA_LIST_FOREACH(cl->properties, itr, fid)
          {
             if (!strcmp(fid->name, func_name))
                return fid;
          }
     }

   if (f_type == EOLIAN_UNRESOLVED || f_type == EOLIAN_CTOR)
     {
        EINA_LIST_FOREACH(cl->constructors, itr, fid)
          {
             if (!strcmp(fid->name, func_name))
                return fid;
          }
     }

   ERR("Function %s not found in class %s", func_name, cl->name);
   return NULL;
}

EAPI Eina_Iterator *
eolian_class_functions_list_get(const Eolian_Class *cl, Eolian_Function_Type foo_type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   switch (foo_type)
     {
      case EOLIAN_PROPERTY:
         return (cl->properties ? eina_list_iterator_new(cl->properties) : NULL);
      case EOLIAN_METHOD:
         return (cl->methods ? eina_list_iterator_new(cl->methods) : NULL);
      case EOLIAN_CTOR:
         return (cl->constructors ? eina_list_iterator_new(cl->constructors) : NULL);
      default: return NULL;
     }
}

EAPI Eina_Iterator *
eolian_class_events_list_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return (cl->events ? eina_list_iterator_new(cl->events) : NULL);
}

EAPI Eina_Bool
eolian_class_ctor_enable_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   return cl->class_ctor_enable;
}

EAPI Eina_Bool
eolian_class_dtor_enable_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   return cl->class_dtor_enable;
}

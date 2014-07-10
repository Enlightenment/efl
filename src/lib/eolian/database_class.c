#include <Eina.h>
#include "eolian_database.h"

void
database_class_del(Eolian_Class *class)
{
   Eina_Stringshare *inherit_name;
   Eina_List *inherits = class->inherits;
   Eolian_Function *fid;
   Eolian_Event *ev;

   EINA_LIST_FREE(inherits, inherit_name)
     eina_stringshare_del(inherit_name);

   Eolian_Implement *impl;
   Eina_List *implements = class->implements;
   EINA_LIST_FREE(implements, impl)
     {
        eina_stringshare_del(impl->full_name);
        free(impl);
     }

   EINA_LIST_FREE(class->constructors, fid) database_function_del(fid);
   EINA_LIST_FREE(class->methods, fid) database_function_del(fid);
   EINA_LIST_FREE(class->properties, fid) database_function_del(fid);
   EINA_LIST_FREE(class->events, ev) database_event_free(ev);

   eina_stringshare_del(class->name);
   eina_stringshare_del(class->full_name);
   eina_stringshare_del(class->file);
   eina_stringshare_del(class->description);
   eina_stringshare_del(class->legacy_prefix);
   eina_stringshare_del(class->eo_prefix);
   eina_stringshare_del(class->data_type);

   free(class);
}

Eolian_Class *
database_class_add(const char *class_name, Eolian_Class_Type type)
{
   char *full_name = strdup(class_name);
   char *name = full_name;
   char *colon = full_name;
   Eolian_Class *cl = calloc(1, sizeof(*cl));
   cl->full_name = eina_stringshare_add(class_name);
   cl->type = type;
   do
     {
        colon = strchr(colon, '.');
        if (colon)
          {
             *colon = '\0';
             cl->namespaces = eina_list_append(cl->namespaces,
                                               eina_stringshare_add(name));
             colon += 1;
             name = colon;
          }
     }
   while(colon);
   cl->name = eina_stringshare_add(name);
   _classes = eina_list_append(_classes, cl);
   free(full_name);
   return cl;
}

Eina_Bool
database_class_file_set(Eolian_Class *class, const char *file_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(class, EINA_FALSE);
   class->file = eina_stringshare_add(file_name);
   return EINA_TRUE;
}

EAPI const char *
eolian_class_file_get(const Eolian_Class *cl)
{
   return cl ? cl->file : NULL;
}

EAPI const char *
eolian_class_full_name_get(const Eolian_Class *cl)
{
   return cl ? cl->full_name : NULL;
}

EAPI const char *
eolian_class_name_get(const Eolian_Class *cl)
{
   return cl ? cl->name : NULL;
}

EAPI const Eina_List *
eolian_class_namespaces_list_get(const Eolian_Class *cl)
{
   return cl ? cl->namespaces : NULL;
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

/*
 * ret false -> clash, class = NULL
 * ret true && class -> only one class corresponding
 * ret true && !class -> no class corresponding
 */
Eina_Bool database_class_name_validate(const char *class_name, const Eolian_Class **class)
{
   char *name = strdup(class_name);
   char *colon = name + 1;
   const Eolian_Class *found_class = NULL;
   const Eolian_Class *candidate;
   if (class) *class = NULL;
   do
     {
        colon = strchr(colon, '.');
        if (colon) *colon = '\0';
        candidate = eolian_class_find_by_name(name);
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
   if (class) *class = found_class;
   free(name);
   return EINA_TRUE;
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

EAPI const Eina_List *
eolian_all_classes_list_get(void)
{
   return _classes;
}

Eina_Bool
database_class_inherit_add(Eolian_Class *cl, const char *inherit_class_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   cl->inherits = eina_list_append(cl->inherits, eina_stringshare_add(inherit_class_name));
   return EINA_TRUE;
}

EAPI const char *
eolian_class_description_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->description;
}

void
database_class_description_set(Eolian_Class *cl, const char *description)
{
   EINA_SAFETY_ON_NULL_RETURN(cl);
   cl->description = eina_stringshare_add(description);
}

EAPI const char*
eolian_class_legacy_prefix_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->legacy_prefix;
}

void
database_class_legacy_prefix_set(Eolian_Class *cl, const char *legacy_prefix)
{
   EINA_SAFETY_ON_NULL_RETURN(cl);
   cl->legacy_prefix = eina_stringshare_add(legacy_prefix);
}

EAPI const char*
eolian_class_eo_prefix_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->eo_prefix;
}

void
database_class_eo_prefix_set(Eolian_Class *cl, const char *eo_prefix)
{
   EINA_SAFETY_ON_NULL_RETURN(cl);
   cl->eo_prefix = eina_stringshare_add(eo_prefix);
}

EAPI const char*
eolian_class_data_type_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->data_type;
}

void
database_class_data_type_set(Eolian_Class *cl, const char *data_type)
{
   EINA_SAFETY_ON_NULL_RETURN(cl);
   cl->data_type = eina_stringshare_add(data_type);
}

EAPI const Eina_List *
eolian_class_inherits_list_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   //FIXME: create list here
   return cl->inherits;
}

EAPI const Eina_List*
eolian_class_implements_list_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->implements;
}

Eina_Bool database_class_function_add(Eolian_Class *cl, Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(fid && cl, EINA_FALSE);
   switch (fid->type)
     {
      case EOLIAN_PROPERTY:
      case EOLIAN_PROP_SET:
      case EOLIAN_PROP_GET:
         cl->properties = eina_list_append(cl->properties, fid);
         break;
      case EOLIAN_METHOD:
         cl->methods = eina_list_append(cl->methods, fid);
         break;
      case EOLIAN_CTOR:
         cl->constructors = eina_list_append(cl->constructors, fid);
         break;
      default:
         ERR("Bad function type %d.", fid->type);
         return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool
database_class_implement_add(Eolian_Class *cl, Eolian_Implement *impl_desc)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(impl_desc, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   cl->implements = eina_list_append(cl->implements, impl_desc);
   return EINA_TRUE;
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

EAPI const Eina_List *
eolian_class_functions_list_get(const Eolian_Class *cl, Eolian_Function_Type foo_type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   switch (foo_type)
     {
      case EOLIAN_PROPERTY:
         return cl->properties;
      case EOLIAN_METHOD:
         return cl->methods;
      case EOLIAN_CTOR:
         return cl->constructors;
      default: return NULL;
     }
}

Eina_Bool
database_class_event_add(Eolian_Class *cl, Eolian_Event *event_desc)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(event_desc && cl, EINA_FALSE);
   cl->events = eina_list_append(cl->events, event_desc);
   return EINA_TRUE;
}

EAPI const Eina_List*
eolian_class_events_list_get(const Eolian_Class *cl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   return cl->events;
}

Eina_Bool
database_class_ctor_enable_set(Eolian_Class *cl, Eina_Bool enable)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   cl->class_ctor_enable = enable;
   return EINA_TRUE;
}

Eina_Bool
database_class_dtor_enable_set(Eolian_Class *cl, Eina_Bool enable)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   cl->class_dtor_enable = enable;
   return EINA_TRUE;
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

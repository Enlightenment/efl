#include <Eina.h>
#include "eolian_database.h"

void
database_class_del(Eolian_Class *cl)
{
   Eina_Stringshare *inherit_name;
   Eina_List *inherits = cl->inherits;
   Eolian_Function *fid;
   Eolian_Event *ev;
   const char *sp;

   EINA_LIST_FREE(inherits, inherit_name)
     eina_stringshare_del(inherit_name);

   Eolian_Implement *impl;
   Eina_List *implements = cl->implements;
   EINA_LIST_FREE(implements, impl)
     {
        eina_stringshare_del(impl->full_name);
        free(impl);
     }

   EINA_LIST_FREE(cl->constructors, fid) database_function_del(fid);
   EINA_LIST_FREE(cl->methods, fid) database_function_del(fid);
   EINA_LIST_FREE(cl->properties, fid) database_function_del(fid);
   EINA_LIST_FREE(cl->events, ev) database_event_del(ev);

   eina_stringshare_del(cl->name);
   eina_stringshare_del(cl->full_name);
   eina_stringshare_del(cl->file);
   eina_stringshare_del(cl->description);
   eina_stringshare_del(cl->legacy_prefix);
   eina_stringshare_del(cl->eo_prefix);
   eina_stringshare_del(cl->data_type);

   if (cl->namespaces) EINA_LIST_FREE(cl->namespaces, sp)
      eina_stringshare_del(sp);

   free(cl);
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

void
database_class_file_set(Eolian_Class *cl, const char *file_name)
{
   EINA_SAFETY_ON_NULL_RETURN(cl);
   cl->file = eina_stringshare_add(file_name);
}

/*
 * ret false -> clash, class = NULL
 * ret true && class -> only one class corresponding
 * ret true && !class -> no class corresponding
 */
Eina_Bool database_class_name_validate(const char *class_name, const Eolian_Class **cl)
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

Eina_Bool
database_class_inherit_add(Eolian_Class *cl, const char *inherit_class_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, EINA_FALSE);
   cl->inherits = eina_list_append(cl->inherits, eina_stringshare_add(inherit_class_name));
   return EINA_TRUE;
}

void
database_class_description_set(Eolian_Class *cl, const char *description)
{
   EINA_SAFETY_ON_NULL_RETURN(cl);
   cl->description = eina_stringshare_add(description);
}

void
database_class_legacy_prefix_set(Eolian_Class *cl, const char *legacy_prefix)
{
   EINA_SAFETY_ON_NULL_RETURN(cl);
   cl->legacy_prefix = eina_stringshare_add(legacy_prefix);
}

void
database_class_eo_prefix_set(Eolian_Class *cl, const char *eo_prefix)
{
   EINA_SAFETY_ON_NULL_RETURN(cl);
   cl->eo_prefix = eina_stringshare_add(eo_prefix);
}

void
database_class_data_type_set(Eolian_Class *cl, const char *data_type)
{
   EINA_SAFETY_ON_NULL_RETURN(cl);
   cl->data_type = eina_stringshare_add(data_type);
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

Eina_Bool
database_class_event_add(Eolian_Class *cl, Eolian_Event *event_desc)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(event_desc && cl, EINA_FALSE);
   cl->events = eina_list_append(cl->events, event_desc);
   return EINA_TRUE;
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

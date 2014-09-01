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

   if (cl->base.file) eina_stringshare_del(cl->base.file);

   EINA_LIST_FREE(inherits, inherit_name)
     eina_stringshare_del(inherit_name);

   Eolian_Implement *impl;
   Eina_List *implements = cl->implements;
   EINA_LIST_FREE(implements, impl)
     {
        eina_stringshare_del(impl->full_name);
        free(impl);
     }

   Eolian_Constructor *ctor;
   Eina_List *constructors = cl->constructors;
   EINA_LIST_FREE(constructors, ctor)
     {
        eina_stringshare_del(ctor->full_name);
        free(ctor);
     }

   EINA_LIST_FREE(cl->methods, fid) database_function_del(fid);
   EINA_LIST_FREE(cl->properties, fid) database_function_del(fid);
   EINA_LIST_FREE(cl->events, ev) database_event_del(ev);

   eina_stringshare_del(cl->name);
   eina_stringshare_del(cl->full_name);
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
   eina_hash_set(_classes, cl->full_name, cl);
   free(full_name);
   return cl;
}

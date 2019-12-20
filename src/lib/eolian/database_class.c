#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"

void
database_class_del(Eolian_Class *cl)
{
   Eolian_Function *fid;
   Eolian_Event *ev;
   Eolian_Part *pt;
   Eolian_Implement *impl;
   Eolian_Constructor *ctor;

   if (!cl || eolian_object_unref(&cl->base)) return;

   eina_stringshare_del(cl->base.file);
   eina_stringshare_del(cl->base.name);
   eina_stringshare_del(cl->base.c_name);

   EINA_LIST_FREE(cl->implements, impl)
     database_implement_del(impl);

   EINA_LIST_FREE(cl->constructors, ctor)
     database_constructor_del(ctor);

   EINA_LIST_FREE(cl->methods, fid) database_function_del(fid);
   EINA_LIST_FREE(cl->properties, fid) database_function_del(fid);
   EINA_LIST_FREE(cl->events, ev) database_event_del(ev);
   EINA_LIST_FREE(cl->parts, pt) database_part_del(pt);
   eina_list_free(cl->requires);
   eina_list_free(cl->callables);
   eina_list_free(cl->composite);

   if (cl->c_prefix)  eina_stringshare_del(cl->c_prefix);
   if (cl->ev_prefix) eina_stringshare_del(cl->ev_prefix);
   if (cl->data_type) eina_stringshare_del(cl->data_type);

   database_doc_del(cl->doc);

   free(cl);
}

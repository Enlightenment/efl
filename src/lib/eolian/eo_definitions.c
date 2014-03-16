#include <stdio.h>
#include <stdlib.h>

#include "eo_definitions.h"

static void
eo_definitions_ret_free(Eo_Ret_Def *ret)
{
   if (ret->type) eina_stringshare_del(ret->type);
   if (ret->comment) eina_stringshare_del(ret->comment);
   if (ret->dflt_ret_val) free(ret->dflt_ret_val);
   free(ret);
}

static void
eo_definitions_param_free(Eo_Param_Def *param)
{
   if (param->type) eina_stringshare_del(param->type);
   if (param->name) eina_stringshare_del(param->name);
   if (param->comment) eina_stringshare_del(param->comment);
   free(param);
}

static void
eo_definitions_accessor_param_free(Eo_Accessor_Param *param)
{
   if (param->name) eina_stringshare_del(param->name);
   if (param->attrs) eina_stringshare_del(param->attrs);
   free(param);
}

static void
eo_definitions_accessor_free(Eo_Accessor_Def *accessor)
{
   if (accessor->comment)
     eina_stringshare_del(accessor->comment);

   if (accessor->legacy)
     eina_stringshare_del(accessor->legacy);

   Eo_Accessor_Param *param;
   EINA_LIST_FREE(accessor->params, param)
      eo_definitions_accessor_param_free(param);

   if (accessor->ret)
      eo_definitions_ret_free(accessor->ret);

   free(accessor);
}

static void
eo_definitions_property_def_free(Eo_Property_Def *prop)
{
   Eo_Param_Def *param;
   Eo_Accessor_Def *accessor;

   if (prop->name)
     eina_stringshare_del(prop->name);

   EINA_LIST_FREE(prop->keys, param)
      eo_definitions_param_free(param);

   EINA_LIST_FREE(prop->values, param)
      eo_definitions_param_free(param);

   EINA_LIST_FREE(prop->accessors, accessor)
      eo_definitions_accessor_free(accessor);

   free(prop);
}

static void
eo_definitions_method_def_free(Eo_Method_Def *meth)
{
   Eo_Param_Def *param;

   if (meth->ret)
      eo_definitions_ret_free(meth->ret);

   if (meth->name)
     eina_stringshare_del(meth->name);
   if (meth->comment)
     eina_stringshare_del(meth->comment);
   if (meth->legacy)
     eina_stringshare_del(meth->legacy);

   EINA_LIST_FREE(meth->params, param)
      eo_definitions_param_free(param);

   free(meth);
}

static void
eo_definitions_event_def_free(Eo_Event_Def *sgn)
{
   if (sgn->name)
     eina_stringshare_del(sgn->name);
   if (sgn->comment)
     eina_stringshare_del(sgn->comment);

   free(sgn);
}

static void
eo_definitions_impl_def_free(Eo_Implement_Def *impl)
{
   if (impl->meth_name)
     eina_stringshare_del(impl->meth_name);
   if (impl->legacy)
      free(impl->legacy);

   free(impl);
}

void
eo_definitions_class_def_free(Eo_Class_Def *kls)
{
   const char *s;
   Eina_List *l;
   Eo_Property_Def *prop;
   Eo_Method_Def *meth;
   Eo_Event_Def *sgn;
   Eo_Implement_Def *impl;

   if (kls->name)
     eina_stringshare_del(kls->name);
   if (kls->comment)
     eina_stringshare_del(kls->comment);
   if (kls->legacy_prefix)
     eina_stringshare_del(kls->legacy_prefix);
   if (kls->eo_prefix)
     eina_stringshare_del(kls->eo_prefix);
   if (kls->data_type)
     eina_stringshare_del(kls->data_type);

   EINA_LIST_FOREACH(kls->inherits, l, s)
      if (s) eina_stringshare_del(s);

   EINA_LIST_FREE(kls->implements, impl)
      eo_definitions_impl_def_free(impl);

   EINA_LIST_FREE(kls->constructors, meth)
      eo_definitions_method_def_free(meth);

   EINA_LIST_FREE(kls->destructors, meth)
      eo_definitions_method_def_free(meth);

   EINA_LIST_FREE(kls->properties, prop)
      eo_definitions_property_def_free(prop);

   EINA_LIST_FREE(kls->methods, meth)
      eo_definitions_method_def_free(meth);

   EINA_LIST_FREE(kls->events, sgn)
      eo_definitions_event_def_free(sgn);

   free(kls);
}


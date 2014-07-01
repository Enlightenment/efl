#include <stdio.h>
#include <stdlib.h>

#include "eo_definitions.h"

static void
eo_definitions_type_free(Eo_Type_Def *tp)
{
   Eo_Type_Def *stp;
   if (tp->name) eina_stringshare_del(tp->name);
   /* for function types, this will map to arguments and ret_type */
   if (tp->subtypes) EINA_LIST_FREE(tp->subtypes, stp)
     eo_definitions_type_free(stp);
   if (tp->base_type)
     eo_definitions_type_free(tp->base_type);
   free(tp);
}

static void
eo_definitions_ret_free(Eo_Ret_Def *ret)
{
   if (ret->type) eo_definitions_type_free(ret->type);
   if (ret->comment) eina_stringshare_del(ret->comment);
   if (ret->dflt_ret_val) eina_stringshare_del(ret->dflt_ret_val);
   free(ret);
}

static void
eo_definitions_param_free(Eo_Param_Def *param)
{
   if (param->type) eo_definitions_type_free(param->type);
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
   if (sgn->type)
     eina_stringshare_del(sgn->type);
   if (sgn->comment)
     eina_stringshare_del(sgn->comment);

   free(sgn);
}

static void
eo_definitions_impl_def_free(Eo_Implement_Def *impl)
{
   if (impl->meth_name)
     eina_stringshare_del(impl->meth_name);

   free(impl);
}

void
eo_definitions_typedef_def_free(Eo_Typedef_Def *type)
{
   if (type->alias)
     eina_stringshare_del(type->alias);

   if (type->type)
     eo_definitions_type_free(type->type);

   free(type);
}

void
eo_definitions_class_def_free(Eo_Class_Def *kls)
{
   const char *s;
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

   EINA_LIST_FREE(kls->inherits, s)
     if (s) eina_stringshare_del(s);

   EINA_LIST_FREE(kls->implements, impl)
     eo_definitions_impl_def_free(impl);

   EINA_LIST_FREE(kls->constructors, meth)
     eo_definitions_method_def_free(meth);

   EINA_LIST_FREE(kls->properties, prop)
     eo_definitions_property_def_free(prop);

   EINA_LIST_FREE(kls->methods, meth)
     eo_definitions_method_def_free(meth);

   EINA_LIST_FREE(kls->events, sgn)
     eo_definitions_event_def_free(sgn);

   free(kls);
}

void
eo_definitions_temps_free(Eo_Lexer_Temps *tmp)
{
   Eina_Strbuf *buf;
   Eo_Param_Def *par;
   const char *s;

   EINA_LIST_FREE(tmp->str_bufs, buf)
     eina_strbuf_free(buf);

   EINA_LIST_FREE(tmp->params, par)
     eo_definitions_param_free(par);

   if (tmp->legacy_def)
     eina_stringshare_del(tmp->legacy_def);

   if (tmp->kls)
     eo_definitions_class_def_free(tmp->kls);

   if (tmp->ret_def)
     eo_definitions_ret_free(tmp->ret_def);

   if (tmp->typedef_def)
     eo_definitions_typedef_def_free(tmp->typedef_def);

   if (tmp->type_def)
     eo_definitions_type_free(tmp->type_def);

   if (tmp->prop)
     eo_definitions_property_def_free(tmp->prop);

   if (tmp->meth)
     eo_definitions_method_def_free(tmp->meth);

   if (tmp->param)
     eo_definitions_param_free(tmp->param);

   if (tmp->accessor)
     eo_definitions_accessor_free(tmp->accessor);

   if (tmp->accessor_param)
     eo_definitions_accessor_param_free(tmp->accessor_param);

   EINA_LIST_FREE(tmp->str_items, s)
     if (s) eina_stringshare_del(s);

   if (tmp->event)
     eo_definitions_event_def_free(tmp->event);

   if (tmp->impl)
     eo_definitions_impl_def_free(tmp->impl);
}

#include <stdio.h>
#include <stdlib.h>

#include "eo_definitions.h"

static void
eo_definitions_ret_free(Eo_Ret_Def *ret)
{
   if (ret->type) database_type_del(ret->type);
   if (ret->comment) eina_stringshare_del(ret->comment);
   database_expr_del(ret->default_ret_val);
   free(ret);
}

static void
eo_definitions_param_free(Eo_Param_Def *param)
{
   if (param->base.file) eina_stringshare_del(param->base.file);
   if (param->type) database_type_del(param->type);
   if (param->name) eina_stringshare_del(param->name);
   if (param->comment) eina_stringshare_del(param->comment);
   free(param);
}

static void
eo_definitions_accessor_param_free(Eo_Accessor_Param *param)
{
   if (param->name) eina_stringshare_del(param->name);
   free(param);
}

static void
eo_definitions_accessor_free(Eo_Accessor_Def *accessor)
{
   if (accessor->base.file)
     eina_stringshare_del(accessor->base.file);

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

   if (prop->base.file)
     eina_stringshare_del(prop->base.file);

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

   if (meth->base.file)
     eina_stringshare_del(meth->base.file);

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

void
eo_definitions_class_def_free(Eo_Class_Def *kls)
{
   const char *s;
   Eo_Property_Def *prop;
   Eo_Method_Def *meth;
   Eolian_Event *sgn;
   Eolian_Implement *impl;
   Eolian_Constructor *ctor;

   if (kls->base.file)
     eina_stringshare_del(kls->base.file);

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
     database_implement_del(impl);

   EINA_LIST_FREE(kls->constructors, ctor)
     database_constructor_del(ctor);

   EINA_LIST_FREE(kls->properties, prop)
     eo_definitions_property_def_free(prop);

   EINA_LIST_FREE(kls->methods, meth)
     eo_definitions_method_def_free(meth);

   EINA_LIST_FREE(kls->events, sgn)
     database_event_del(sgn);

   free(kls);
}

void
eo_definitions_temps_free(Eo_Lexer_Temps *tmp)
{
   Eina_Strbuf *buf;
   Eo_Param_Def *par;
   Eolian_Type *tp;
   Eolian_Variable *var;
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

   EINA_LIST_FREE(tmp->type_defs, tp)
     database_type_del(tp);

   EINA_LIST_FREE(tmp->var_defs, var)
     database_var_del(var);

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
     database_event_del(tmp->event);

   if (tmp->impl)
     database_implement_del(tmp->impl);

   if (tmp->ctor)
     database_constructor_del(tmp->ctor);

   EINA_LIST_FREE(tmp->strs, s)
     if (s) eina_stringshare_del(s);
}

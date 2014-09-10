#include <stdio.h>
#include <stdlib.h>

#include "eo_definitions.h"

static void
eo_definitions_property_def_free(Eo_Property_Def *prop)
{
   Eolian_Function_Parameter *param;

   if (prop->base.file)
     eina_stringshare_del(prop->base.file);

   if (prop->name)
     eina_stringshare_del(prop->name);

   EINA_LIST_FREE(prop->keys, param)
     database_parameter_del(param);

   EINA_LIST_FREE(prop->params, param)
     database_parameter_del(param);

   if (prop->get_description)
     eina_stringshare_del(prop->get_description);

   if (prop->set_description)
     eina_stringshare_del(prop->set_description);

   if (prop->get_legacy)
     eina_stringshare_del(prop->get_legacy);

   if (prop->set_legacy)
     eina_stringshare_del(prop->set_legacy);

   if (prop->get_ret_type)
     database_type_del(prop->get_ret_type);

   if (prop->set_ret_type)
     database_type_del(prop->set_ret_type);

   if (prop->get_return_comment)
     eina_stringshare_del(prop->get_return_comment);

   if (prop->set_return_comment)
     eina_stringshare_del(prop->set_return_comment);

   database_expr_del(prop->get_ret_val);
   database_expr_del(prop->set_ret_val);

   free(prop);
}

static void
eo_definitions_method_def_free(Eo_Method_Def *meth)
{
   Eolian_Function_Parameter *param;

   if (meth->base.file)
     eina_stringshare_del(meth->base.file);

   if (meth->ret_type)
     database_type_del(meth->ret_type);

   if (meth->ret_comment)
     eina_stringshare_del(meth->ret_comment);

   database_expr_del(meth->default_ret_val);

   if (meth->name)
     eina_stringshare_del(meth->name);
   if (meth->comment)
     eina_stringshare_del(meth->comment);
   if (meth->legacy)
     eina_stringshare_del(meth->legacy);

   EINA_LIST_FREE(meth->params, param)
     database_parameter_del(param);

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
   Eolian_Function_Parameter *par;
   Eolian_Type *tp;
   Eolian_Variable *var;
   const char *s;

   EINA_LIST_FREE(tmp->str_bufs, buf)
     eina_strbuf_free(buf);

   EINA_LIST_FREE(tmp->params, par)
     database_parameter_del(par);

   if (tmp->legacy_def)
     eina_stringshare_del(tmp->legacy_def);

   if (tmp->kls)
     eo_definitions_class_def_free(tmp->kls);

   EINA_LIST_FREE(tmp->type_defs, tp)
     database_type_del(tp);

   EINA_LIST_FREE(tmp->var_defs, var)
     database_var_del(var);

   if (tmp->prop)
     eo_definitions_property_def_free(tmp->prop);

   if (tmp->meth)
     eo_definitions_method_def_free(tmp->meth);

   if (tmp->param)
     database_parameter_del(tmp->param);

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

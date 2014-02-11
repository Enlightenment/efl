#include <stdio.h>
#include <stdlib.h>

#include "eo_definitions.h"

void
eo_definitions_ret_free(Eo_Ret_Def *ret)
{
   if (ret->type) eina_stringshare_del(ret->type);
   if (ret->comment) eina_stringshare_del(ret->comment);
   /* do not free */
}

void
eo_definitions_param_free(Eo_Param_Def *param)
{
   if (param->type) eina_stringshare_del(param->type);
   if (param->name) eina_stringshare_del(param->name);
   if (param->comment) eina_stringshare_del(param->comment);
   free(param);
}

void
eo_definitions_accessor_free(Eo_Accessor_Def *accessor)
{
   if (accessor->comment)
     eina_stringshare_del(accessor->comment);

   if (accessor->legacy)
     eina_stringshare_del(accessor->legacy);

   eo_definitions_ret_free(&accessor->ret);

   free(accessor);
}

void
eo_definitions_property_def_free(Eo_Property_Def *prop)
{
   Eo_Param_Def *param;
   Eo_Accessor_Def *accessor;

   if (prop->name)
     eina_stringshare_del(prop->name);

   EINA_LIST_FREE(prop->params, param)
      eo_definitions_param_free(param);

   EINA_LIST_FREE(prop->accessors, accessor)
      eo_definitions_accessor_free(accessor);

   free(prop);
}

void
eo_definitions_method_def_free(Eo_Method_Def *meth)
{
   Eo_Param_Def *param;

   eo_definitions_ret_free(&meth->ret);

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
eo_definitions_signal_def_free(Eo_Signal_Def *sgn)
{
   if (sgn->name)
     eina_stringshare_del(sgn->name);
   if (sgn->comment)
     eina_stringshare_del(sgn->comment);

   free(sgn);
}

void
eo_definitions_dfltctor_def_free(Eo_DfltCtor_Def *ctor)
{
   if (ctor->name)
     eina_stringshare_del(ctor->name);
   if (ctor->comment)
     eina_stringshare_del(ctor->comment);

   free(ctor);
}

void
eo_definitions_dfltdtor_def_free(Eo_DfltDtor_Def *dtor)
{
   if (dtor->name)
     eina_stringshare_del(dtor->name);
   if (dtor->comment)
     eina_stringshare_del(dtor->comment);

   free(dtor);
}

void
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
   Eo_Signal_Def *sgn;
   Eo_Implement_Def *impl;

   if (kls->name)
     eina_stringshare_del(kls->name);
   if (kls->comment)
     eina_stringshare_del(kls->comment);
   if (kls->legacy_prefix)
     eina_stringshare_del(kls->legacy_prefix);

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

   EINA_LIST_FREE(kls->signals, sgn)
      eo_definitions_signal_def_free(sgn);

   if (kls->dflt_ctor) eo_definitions_dfltctor_def_free(kls->dflt_ctor);
   if (kls->dflt_dtor) eo_definitions_dfltdtor_def_free(kls->dflt_dtor);

   free(kls);
}


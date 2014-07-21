#include "eo_parser.h"

static Eina_Bool
_db_fill_ctor(Eolian_Class *cl, Eo_Method_Def *meth)
{
   Eo_Param_Def *param;
   Eina_List *l;

   Eolian_Function *foo_id = database_function_new(meth->name, EOLIAN_CTOR);

   database_class_function_add(cl, foo_id);

   if (meth->ret)
     {
        database_function_return_comment_set(foo_id, EOLIAN_METHOD,
                                             meth->ret->comment);
     }
 
   database_function_data_set(foo_id, EOLIAN_LEGACY, meth->legacy);

   EINA_LIST_FOREACH(meth->params, l, param)
     {
        database_method_parameter_add(foo_id, (Eolian_Parameter_Dir)param->way,
                                      param->type, param->name, param->comment);
        param->type = NULL;
     }

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_ctors(Eolian_Class *cl, Eo_Class_Def *kls)
{
   Eo_Method_Def *meth;
   Eina_List *l;

   EINA_LIST_FOREACH(kls->constructors, l, meth)
     if (!_db_fill_ctor(cl, meth)) return EINA_FALSE;

   return EINA_TRUE;
}


static Eina_Bool
_db_fill_key(Eolian_Function *foo_id, Eo_Param_Def *param)
{
   Eolian_Function_Parameter *p = database_property_key_add(foo_id,
                                                            param->type,
                                                            param->name,
                                                            param->comment);
   database_parameter_nonull_set(p, param->nonull);
   param->type = NULL;

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_keys(Eolian_Function *foo_id, Eo_Property_Def *prop)
{
   Eo_Param_Def *param;
   Eina_List *l;

   EINA_LIST_FOREACH(prop->keys, l, param)
     if (!_db_fill_key(foo_id, param)) return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_value(Eolian_Function *foo_id, Eo_Param_Def *param)
{
   Eolian_Function_Parameter *p = database_property_value_add(foo_id,
                                                              param->type,
                                                              param->name,
                                                              param->comment);
   database_parameter_nonull_set(p, param->nonull);
   param->type = NULL;

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_values(Eolian_Function *foo_id, Eo_Property_Def *prop)
{
   Eo_Param_Def *param;
   Eina_List *l;

   EINA_LIST_FOREACH(prop->values, l, param)
     if (!_db_fill_value(foo_id, param)) return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_param(Eolian_Function *foo_id, Eo_Param_Def *param)
{
   Eolian_Function_Parameter *p = database_method_parameter_add(foo_id,
                                                                param->way,
                                                                param->type,
                                                                param->name,
                                                                param->comment);
   database_parameter_nonull_set(p, param->nonull);
   param->type = NULL;

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_params(Eolian_Function *foo_id, Eo_Method_Def *meth)
{
   Eo_Param_Def *param;
   Eina_List *l;

   EINA_LIST_FOREACH(meth->params, l, param)
     if (!_db_fill_param(foo_id, param)) return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_accessor(Eolian_Function *foo_id, Eo_Class_Def *kls,
                  Eo_Property_Def *prop, Eo_Accessor_Def *accessor)
{
   Eo_Accessor_Param *acc_param;
   Eina_List *l;

   database_function_type_set(foo_id, ((accessor->type == SETTER)
                                        ? EOLIAN_PROP_SET : EOLIAN_PROP_GET));

   Eolian_Function_Type ftype = (accessor->type == SETTER)
                                 ? EOLIAN_PROP_SET : EOLIAN_PROP_GET;

   if (accessor->ret && accessor->ret->type)
     {
        database_function_return_type_set(foo_id, ftype, accessor->ret->type);
        database_function_return_comment_set(foo_id, ftype,
                                             accessor->ret->comment);
        database_function_return_flag_set_as_warn_unused(foo_id, ftype,
                                             accessor->ret->warn_unused);
        database_function_return_default_val_set(foo_id, ftype,
                                             accessor->ret->default_ret_val);
        accessor->ret->type = NULL;
     }

   if (accessor->legacy)
     {
        database_function_data_set(foo_id, ((accessor->type == SETTER)
                                            ? EOLIAN_LEGACY_SET
                                            : EOLIAN_LEGACY_GET),
                                   accessor->legacy);
     }

   database_function_description_set(foo_id, ((accessor->type == SETTER)
                                              ? EOLIAN_COMMENT_SET
                                              : EOLIAN_COMMENT_GET),
                                     accessor->comment);

   EINA_LIST_FOREACH(accessor->params, l, acc_param)
     {
        Eolian_Function_Parameter *desc = (Eolian_Function_Parameter*)
            eolian_function_parameter_get(foo_id, acc_param->name);

        if (!desc)
          {
             ERR("Error - %s not known as parameter of property %s\n",
                 acc_param->name, prop->name);
             return EINA_FALSE;
          }
        else if (acc_param->is_const)
          {
             database_parameter_const_attribute_set(desc,
                                                    accessor->type == GETTER,
                                                    EINA_TRUE);
          }
     }

   if (kls->type == EOLIAN_CLASS_INTERFACE)
      database_function_set_as_virtual_pure(foo_id, ftype);

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_accessors(Eolian_Function *foo_id, Eo_Class_Def *kls,
                   Eo_Property_Def *prop)
{
   Eo_Accessor_Def *accessor;
   Eina_List *l;

   EINA_LIST_FOREACH(prop->accessors, l, accessor)
     if (!_db_fill_accessor(foo_id, kls, prop, accessor)) return EINA_FALSE;

   return EINA_TRUE;
}


static Eina_Bool
_db_fill_property(Eolian_Class *cl, Eo_Class_Def *kls, Eo_Property_Def *prop)
{
   Eolian_Function *foo_id = database_function_new(prop->name,
                                                   EOLIAN_UNRESOLVED);

   database_function_scope_set(foo_id, prop->scope);

   if (!_db_fill_keys     (foo_id,      prop)) goto failure;
   if (!_db_fill_values   (foo_id,      prop)) goto failure;
   if (!_db_fill_accessors(foo_id, kls, prop)) goto failure;

   if (!prop->accessors)
     {
        database_function_type_set(foo_id, EOLIAN_PROPERTY);
        if (kls->type == EOLIAN_CLASS_INTERFACE)
          database_function_set_as_virtual_pure(foo_id, EOLIAN_UNRESOLVED);
     }

   database_class_function_add(cl, foo_id);

   return EINA_TRUE;

failure:
   database_function_del(foo_id);
   return EINA_FALSE;
}

static Eina_Bool
_db_fill_properties(Eolian_Class *cl, Eo_Class_Def *kls)
{
   Eo_Property_Def *prop;
   Eina_List *l;

   EINA_LIST_FOREACH(kls->properties, l, prop)
     if (!_db_fill_property(cl, kls, prop)) return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_method(Eolian_Class *cl, Eo_Class_Def *kls, Eo_Method_Def *meth)
{
   Eolian_Function *foo_id = database_function_new(meth->name, EOLIAN_METHOD);

   database_function_scope_set(foo_id, meth->scope);

   database_class_function_add(cl, foo_id);

   if (meth->ret)
     {
        database_function_return_type_set(foo_id, EOLIAN_METHOD,
                                          meth->ret->type);
        database_function_return_comment_set(foo_id, EOLIAN_METHOD,
                                          meth->ret->comment);
        database_function_return_flag_set_as_warn_unused(foo_id, EOLIAN_METHOD,
                                          meth->ret->warn_unused);
        database_function_return_default_val_set(foo_id, EOLIAN_METHOD,
                                          meth->ret->default_ret_val);
        meth->ret->type = NULL;
     }

   database_function_description_set(foo_id, EOLIAN_COMMENT, meth->comment);
   database_function_data_set(foo_id, EOLIAN_LEGACY, meth->legacy);
   database_function_object_set_as_const(foo_id, meth->obj_const);

   _db_fill_params(foo_id, meth);

   if (kls->type == EOLIAN_CLASS_INTERFACE)
     database_function_set_as_virtual_pure(foo_id, EOLIAN_METHOD);

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_methods(Eolian_Class *cl, Eo_Class_Def *kls)
{
   Eo_Method_Def *meth;
   Eina_List *l;

   EINA_LIST_FOREACH(kls->methods, l, meth)
     if (!_db_fill_method(cl, kls, meth)) return EINA_FALSE;

   return EINA_TRUE;
}

static int
_db_fill_implement(Eolian_Class *cl, Eolian_Implement *impl)
{
   const char *impl_name = impl->full_name;

   if (!strcmp(impl_name, "class.constructor"))
     {
        database_class_ctor_enable_set(cl, EINA_TRUE);
        return 1;
     }

   if (!strcmp(impl_name, "class.destructor"))
     {
        database_class_dtor_enable_set(cl, EINA_TRUE);
        return 1;
     }

   if (!strncmp(impl_name, "virtual.", 8))
     {
        Eolian_Function_Type ftype = EOLIAN_UNRESOLVED;
        char *type_as_str  = NULL;
        char *virtual_name = strdup(impl_name);
        char *func         = strchr(virtual_name, '.');

        if (func) *func = '\0';
        func += 1;

        if ((type_as_str = strchr(func, '.')))
          {
             *type_as_str = '\0';

             if (!strcmp(type_as_str+1, "set"))
               ftype = EOLIAN_PROP_SET;
             else if (!strcmp(type_as_str+1, "get"))
               ftype = EOLIAN_PROP_GET;
          }

        Eolian_Function *foo_id = (Eolian_Function*)
                                   eolian_class_function_find_by_name(cl,
                                                                      func,
                                                                      ftype);

        free(virtual_name);

        if (!foo_id)
          {
             ERR("Error - %s not known in class %s", impl_name + 8,
                 eolian_class_name_get(cl));
             return -1;
          }
        database_function_set_as_virtual_pure(foo_id, ftype);
        return 1;
     }
   database_class_implement_add(cl, impl);
   return 0;
}

static Eina_Bool
_db_fill_implements(Eolian_Class *cl, Eo_Class_Def *kls)
{
   Eolian_Implement *impl;
   Eina_List *l;

   EINA_LIST_FOREACH(kls->implements, l, impl)
     {
        int ret = _db_fill_implement(cl, impl);
        if (ret < 0) return EINA_FALSE;
        if (ret > 0) continue;
        eina_list_data_set(l, NULL); /* prevent double free */
     }
 
   return EINA_TRUE;
}

static Eina_Bool
_db_fill_events(Eolian_Class *cl, Eo_Class_Def *kls)
{
   Eolian_Event *event;
   Eina_List *l;

   EINA_LIST_FOREACH(kls->events, l, event)
     {
        database_class_event_add(cl, event);
        eina_list_data_set(l, NULL); /* prevent double free */
     }

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_class(Eo_Class_Def *kls, const char *filename)
{
   Eolian_Class *cl = database_class_add(kls->name, kls->type);
   const char *s;
   Eina_List *l;

   database_class_file_set(cl, filename);

   if (kls->comment)
     {
        database_class_description_set(cl, kls->comment);
     }

   EINA_LIST_FOREACH(kls->inherits, l, s)
     {
        database_class_inherit_add(cl, s);
     }

   if (kls->legacy_prefix)
     {
        database_class_legacy_prefix_set(cl, kls->legacy_prefix);
     }
   if (kls->eo_prefix)
     {
        database_class_eo_prefix_set(cl, kls->eo_prefix);
     }
   if (kls->data_type)
     {
        database_class_data_type_set(cl, kls->data_type);
     }

   if (!_db_fill_ctors     (cl, kls)) return EINA_FALSE;
   if (!_db_fill_properties(cl, kls)) return EINA_FALSE;
   if (!_db_fill_methods   (cl, kls)) return EINA_FALSE;
   if (!_db_fill_implements(cl, kls)) return EINA_FALSE;
   if (!_db_fill_events    (cl, kls)) return EINA_FALSE;

   return EINA_TRUE;
}

Eina_Bool
eo_parser_database_fill(const char *filename, Eina_Bool eot)
{
   Eina_List *k;
   Eo_Node *nd;
   Eina_Bool has_class = EINA_FALSE;

   Eo_Lexer *ls = eo_lexer_new(filename);
   if (!ls)
     {
        ERR("unable to create lexer");
        return EINA_FALSE;
     }

   /* read first token */
   eo_lexer_get(ls);

   if (!eo_parser_walk(ls, eot))
     {
        eo_lexer_free(ls);
        return EINA_FALSE;
     }

   if (eot) goto done;

   EINA_LIST_FOREACH(ls->nodes, k, nd) if (nd->type == NODE_CLASS)
     {
        has_class = EINA_TRUE;
        break;
     }

   if (!has_class)
     {
        ERR("No classes for file %s", filename);
        eo_lexer_free(ls);
        return EINA_FALSE;
     }

   EINA_LIST_FOREACH(ls->nodes, k, nd)
     {
        switch (nd->type)
          {
           case NODE_CLASS:
             if (!_db_fill_class(nd->def_class, filename))
               goto error;
             break;
           default:
             break;
          }
     }

done:
   eo_lexer_free(ls);
   return EINA_TRUE;

error:
   eo_lexer_free(ls);
   return EINA_FALSE;
}

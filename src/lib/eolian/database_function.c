#include <Eina.h>
#include "eolian_database.h"

void
database_function_del(Eolian_Function *fid)
{
   Eolian_Function_Parameter *param;
   if (!fid) return;

   if (fid->base.file) eina_stringshare_del(fid->base.file);
   eina_stringshare_del(fid->name);
   EINA_LIST_FREE(fid->keys, param) database_parameter_del(param);
   EINA_LIST_FREE(fid->params, param) database_parameter_del(param);
   database_type_del(fid->get_ret_type);
   database_type_del(fid->set_ret_type);
   database_expr_del(fid->get_ret_val);
   database_expr_del(fid->set_ret_val);
   if (fid->get_legacy) eina_stringshare_del(fid->get_legacy);
   if (fid->set_legacy) eina_stringshare_del(fid->set_legacy);
   if (fid->get_description) eina_stringshare_del(fid->get_description);
   if (fid->set_description) eina_stringshare_del(fid->set_description);
   if (fid->get_return_comment) eina_stringshare_del(fid->get_return_comment);
   if (fid->set_return_comment) eina_stringshare_del(fid->set_return_comment);
   free(fid);
}

Eolian_Function *
database_function_new(const char *function_name, Eolian_Function_Type foo_type)
{
   Eolian_Function *fid = calloc(1, sizeof(*fid));
   fid->name = eina_stringshare_add(function_name);
   fid->type = foo_type;
   return fid;
}

void
database_function_scope_set(Eolian_Function *fid, Eolian_Object_Scope scope)
{
   EINA_SAFETY_ON_NULL_RETURN(fid);
   fid->scope = scope;
}

void
database_function_type_set(Eolian_Function *fid, Eolian_Function_Type foo_type)
{
   EINA_SAFETY_ON_NULL_RETURN(fid);
   switch (foo_type)
     {
      case EOLIAN_PROP_SET:
         if (fid->type == EOLIAN_PROP_GET) foo_type = EOLIAN_PROPERTY;
         break;
      case EOLIAN_PROP_GET:
         if (fid->type == EOLIAN_PROP_SET) foo_type = EOLIAN_PROPERTY;
         break;
      default:
         break;
     }
   fid->type = foo_type;
}

Eina_Bool
database_function_set_as_virtual_pure(Eolian_Function *fid, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROP_GET: fid->get_virtual_pure = EINA_TRUE; break;
      case EOLIAN_PROP_SET: fid->set_virtual_pure = EINA_TRUE; break;
      default: return EINA_FALSE;
     }
   return EINA_TRUE;
}

void
database_function_set_as_class(Eolian_Function *fid, Eina_Bool is_class)
{
   EINA_SAFETY_ON_NULL_RETURN(fid);
   fid->is_class = is_class;
}

Eolian_Function_Parameter *
database_property_key_add(Eolian_Function *fid, Eolian_Type *type, const char *name, const char *description)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   Eolian_Function_Parameter *param = database_parameter_add(type, name, description);
   fid->keys = eina_list_append(fid->keys, param);
   return param;
}

Eolian_Function_Parameter *
database_property_value_add(Eolian_Function *fid, Eolian_Type *type, const char *name, const char *description)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   Eolian_Function_Parameter *param = database_parameter_add(type, name, description);
   fid->params = eina_list_append(fid->params, param);
   return param;
}

Eolian_Function_Parameter *
database_method_parameter_add(Eolian_Function *fid, Eolian_Parameter_Dir param_dir, Eolian_Type *type, const char *name, const char *description)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   Eolian_Function_Parameter *param = database_parameter_add(type, name, description);
   param->param_dir = param_dir;
   fid->params = eina_list_append(fid->params, param);
   return param;
}

void database_function_return_type_set(Eolian_Function *fid, Eolian_Function_Type ftype, Eolian_Type *ret_type)
{
   switch (ftype)
     {
      case EOLIAN_PROP_SET: fid->set_ret_type = ret_type; break;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROP_GET: fid->get_ret_type = ret_type; break;
      default: return;
     }
}

void database_function_return_default_val_set(Eolian_Function *fid, Eolian_Function_Type ftype, Eolian_Expression *ret_default_value)
{
   switch (ftype)
     {
      case EOLIAN_PROP_SET: fid->set_ret_val = ret_default_value; break;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROP_GET: fid->get_ret_val = ret_default_value; break;
      default: return;
     }
}

void database_function_return_comment_set(Eolian_Function *fid, Eolian_Function_Type ftype, const char *ret_comment)
{
   switch (ftype)
     {
      case EOLIAN_PROP_SET: fid->set_return_comment = eina_stringshare_ref(ret_comment); break;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROP_GET: fid->get_return_comment = eina_stringshare_ref(ret_comment); break;
      default: return;
     }
}

void database_function_return_flag_set_as_warn_unused(Eolian_Function *fid,
      Eolian_Function_Type ftype, Eina_Bool warn_unused)
{
   EINA_SAFETY_ON_NULL_RETURN(fid);
   switch (ftype)
     {
      case EOLIAN_METHOD: case EOLIAN_PROP_GET: fid->get_return_warn_unused = warn_unused; break;
      case EOLIAN_PROP_SET: fid->set_return_warn_unused = warn_unused; break;
      default: return;
     }
}

void
database_function_object_set_as_const(Eolian_Function *fid, Eina_Bool is_const)
{
   EINA_SAFETY_ON_NULL_RETURN(fid);
   fid->obj_is_const = is_const;
}

void database_function_legacy_set(Eolian_Function *fid, Eolian_Function_Type ftype, const char *legacy)
{
   switch (ftype)
     {
      case EOLIAN_PROP_SET: fid->set_legacy = eina_stringshare_ref(legacy); break;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_CTOR: case EOLIAN_PROP_GET: fid->get_legacy = eina_stringshare_ref(legacy); break;
      default: return;
     }
}

void database_function_description_set(Eolian_Function *fid, Eolian_Function_Type ftype, const char *desc)
{
   switch (ftype)
     {
      case EOLIAN_PROP_SET: fid->set_description = eina_stringshare_ref(desc); break;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_CTOR: case EOLIAN_PROP_GET: fid->get_description = eina_stringshare_ref(desc); break;
      default: return;
     }
}

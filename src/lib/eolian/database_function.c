#include <Eina.h>
#include "eolian_database.h"

void
database_function_del(Eolian_Function *fid)
{
   Eolian_Function_Parameter *param;
   if (!fid) return;
   eina_stringshare_del(fid->name);
   eina_hash_free(fid->data);
   EINA_LIST_FREE(fid->keys, param) database_parameter_del(param);
   EINA_LIST_FREE(fid->params, param) database_parameter_del(param);
   database_type_del(fid->get_ret_type);
   database_type_del(fid->set_ret_type);
   free(fid);
}

Eolian_Function *
database_function_new(const char *function_name, Eolian_Function_Type foo_type)
{
   Eolian_Function *fid = calloc(1, sizeof(*fid));
   fid->name = eina_stringshare_add(function_name);
   fid->type = foo_type;
   fid->data = eina_hash_string_superfast_new(free);
   return fid;
}

void
database_function_scope_set(Eolian_Function *fid, Eolian_Function_Scope scope)
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

void
database_function_data_set(Eolian_Function *fid, const char *key, const char *data)
{
   EINA_SAFETY_ON_NULL_RETURN(key);
   EINA_SAFETY_ON_NULL_RETURN(fid);
   if (data)
     {
        if (!eina_hash_find(fid->data, key))
          eina_hash_set(fid->data, key, strdup(data));
     }
   else
     {
        eina_hash_del(fid->data, key, NULL);
     }
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

void database_function_return_default_val_set(Eolian_Function *fid, Eolian_Function_Type ftype, const char *ret_default_value)
{
   const char *key = NULL;
   switch (ftype)
     {
      case EOLIAN_PROP_SET: key = PROP_SET_RETURN_DEFAULT_VAL; break;
      case EOLIAN_PROP_GET: key = PROP_GET_RETURN_DEFAULT_VAL; break;
      case EOLIAN_METHOD: key = METHOD_RETURN_DEFAULT_VAL; break;
      default: return;
     }
   database_function_data_set(fid, key, ret_default_value);
}

void database_function_return_comment_set(Eolian_Function *fid, Eolian_Function_Type ftype, const char *ret_comment)
{
   const char *key = NULL;
   switch (ftype)
     {
      case EOLIAN_PROP_SET: key = EOLIAN_PROP_SET_RETURN_COMMENT; break;
      case EOLIAN_PROP_GET: key = EOLIAN_PROP_GET_RETURN_COMMENT; break;
      case EOLIAN_METHOD: key = EOLIAN_METHOD_RETURN_COMMENT; break;
      default: return;
     }
   database_function_data_set(fid, key, ret_comment);
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

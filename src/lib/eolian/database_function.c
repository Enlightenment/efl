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

EAPI Eolian_Function_Scope
eolian_function_scope_get(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EOLIAN_SCOPE_PUBLIC);
   return fid->scope;
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

EAPI Eolian_Function_Type
eolian_function_type_get(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EOLIAN_UNRESOLVED);
   return fid->type;
}

EAPI const char *
eolian_function_name_get(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   return fid->name;
}

EAPI const char *
eolian_function_full_c_name_get(const Eolian_Function *foo_id, const char *prefix)
{
   const char  *funcn = eolian_function_name_get(foo_id);
   const char  *last_p = strrchr(prefix, '_');
   const char  *func_p = strchr(funcn, '_');
   Eina_Strbuf *buf = eina_strbuf_new();
   Eina_Stringshare *ret;
   int   len;

   if (!last_p) last_p = prefix;
   else last_p++;
   if (!func_p) len = strlen(funcn);
   else len = func_p - funcn;

   if ((int)strlen(last_p) != len || strncmp(last_p, funcn, len))
     {
        eina_strbuf_append(buf, prefix);
        eina_strbuf_append_char(buf, '_');
        eina_strbuf_append(buf, funcn);
        ret = eina_stringshare_add(eina_strbuf_string_get(buf));
        eina_strbuf_free(buf);
        return ret;
     }

   if (last_p != prefix)
      eina_strbuf_append_n(buf, prefix, last_p - prefix); /* includes _ */

   eina_strbuf_append(buf, funcn);
   ret = eina_stringshare_add(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   return ret;
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

EAPI Eina_Bool
eolian_function_is_virtual_pure(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROP_GET: return fid->get_virtual_pure; break;
      case EOLIAN_PROP_SET: return fid->set_virtual_pure; break;
      default: return EINA_FALSE;
     }
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

EAPI const char *
eolian_function_data_get(const Eolian_Function *fid, const char *key)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   return eina_hash_find(fid->data, key);
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

EAPI const Eolian_Function_Parameter *
eolian_function_parameter_get(const Eolian_Function *fid, const char *param_name)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   Eina_List *itr;
   Eolian_Function_Parameter *param;
   EINA_LIST_FOREACH(fid->keys, itr, param)
      if (!strcmp(param->name, param_name)) return param;
   EINA_LIST_FOREACH(fid->params, itr, param)
      if (!strcmp(param->name, param_name)) return param;
   return NULL;
}

EAPI const Eina_List *
eolian_property_keys_list_get(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   return fid->keys;
}

EAPI const Eina_List *
eolian_property_values_list_get(const Eolian_Function *fid)
{
   return eolian_parameters_list_get(fid);
}

EAPI const Eina_List *
eolian_parameters_list_get(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   return fid->params;
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

EAPI const Eolian_Type *
eolian_function_return_type_get(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   switch (ftype)
     {
      case EOLIAN_PROP_SET: return fid->set_ret_type;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROP_GET: return fid->get_ret_type;
      default: return NULL;
     }
}

void database_function_return_dflt_val_set(Eolian_Function *fid, Eolian_Function_Type ftype, const char *ret_dflt_value)
{
   const char *key = NULL;
   switch (ftype)
     {
      case EOLIAN_PROP_SET: key = PROP_SET_RETURN_DFLT_VAL; break;
      case EOLIAN_PROP_GET: key = PROP_GET_RETURN_DFLT_VAL; break;
      case EOLIAN_METHOD: key = METHOD_RETURN_DFLT_VAL; break;
      default: return;
     }
   database_function_data_set(fid, key, ret_dflt_value);
}

EAPI const char *
eolian_function_return_dflt_value_get(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   const char *key = NULL;
   switch (ftype)
     {
      case EOLIAN_PROP_SET: key = PROP_SET_RETURN_DFLT_VAL; break;
      case EOLIAN_PROP_GET: key = PROP_GET_RETURN_DFLT_VAL; break;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: key = METHOD_RETURN_DFLT_VAL; break;
      default: return NULL;
     }
   return eolian_function_data_get(fid, key);
}

EAPI const char *
eolian_function_return_comment_get(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   const char *key = NULL;
   switch (ftype)
     {
      case EOLIAN_PROP_SET: key = EOLIAN_PROP_SET_RETURN_COMMENT; break;
      case EOLIAN_PROP_GET: key = EOLIAN_PROP_GET_RETURN_COMMENT; break;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: key = EOLIAN_METHOD_RETURN_COMMENT; break;
      default: return NULL;
     }
   return eolian_function_data_get(fid, key);
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

EAPI Eina_Bool
eolian_function_return_is_warn_unused(const Eolian_Function *fid,
      Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_METHOD: case EOLIAN_PROP_GET: return fid->get_return_warn_unused;
      case EOLIAN_PROP_SET: return fid->set_return_warn_unused;
      default: return EINA_FALSE;
     }
}

void
database_function_object_set_as_const(Eolian_Function *fid, Eina_Bool is_const)
{
   EINA_SAFETY_ON_NULL_RETURN(fid);
   fid->obj_is_const = is_const;
}

EAPI Eina_Bool
eolian_function_object_is_const(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   return fid->obj_is_const;
}

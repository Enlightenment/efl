#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"

void
database_function_del(Eolian_Function *fid)
{
   Eolian_Function_Parameter *param;
   Eina_Stringshare *cls_name;
   if (!fid) return;

   eina_stringshare_del(fid->base.file);
   eina_stringshare_del(fid->base.name);
   EINA_LIST_FREE(fid->prop_values, param) database_parameter_del(param);
   EINA_LIST_FREE(fid->prop_values_get, param) database_parameter_del(param);
   EINA_LIST_FREE(fid->prop_values_set, param) database_parameter_del(param);
   EINA_LIST_FREE(fid->prop_keys, param) database_parameter_del(param);
   EINA_LIST_FREE(fid->prop_keys_get, param) database_parameter_del(param);
   EINA_LIST_FREE(fid->prop_keys_set, param) database_parameter_del(param);
   EINA_LIST_FREE(fid->ctor_of, cls_name) eina_stringshare_del(cls_name);
   database_type_del(fid->get_ret_type);
   database_type_del(fid->set_ret_type);
   database_expr_del(fid->get_ret_val);
   database_expr_del(fid->set_ret_val);
   if (fid->get_legacy) eina_stringshare_del(fid->get_legacy);
   if (fid->set_legacy) eina_stringshare_del(fid->set_legacy);
   database_doc_del(fid->get_return_doc);
   database_doc_del(fid->set_return_doc);
   free(fid);
}

static Eina_List*
_list_sorted_insert_no_dup(Eina_List *l, Eina_Compare_Cb func, const void *data)
{
   Eina_List *lnear;
   int cmp;

   if (!l)
     return eina_list_append(NULL, data);
   else
     lnear = eina_list_search_sorted_near_list(l, func, data, &cmp);

   if (cmp < 0)
     return eina_list_append_relative_list(l, data, lnear);
   else if (cmp > 0)
     return eina_list_prepend_relative_list(l, data, lnear);
   return l;
}

void
database_function_constructor_add(Eolian_Function *func, const Eolian_Class *cls)
{
   func->ctor_of = _list_sorted_insert_no_dup
     (func->ctor_of, EINA_COMPARE_CB(strcmp),
      eina_stringshare_ref(cls->base.name));
}

Eina_Bool
database_function_is_type(Eolian_Function *fid, Eolian_Function_Type ftype)
{
   if (ftype == EOLIAN_UNRESOLVED)
     return EINA_TRUE;
   else if (ftype == EOLIAN_PROP_GET)
     return (fid->type == EOLIAN_PROP_GET) || (fid->type == EOLIAN_PROPERTY);
   else if (ftype == EOLIAN_PROP_SET)
     return (fid->type == EOLIAN_PROP_SET) || (fid->type == EOLIAN_PROPERTY);
   return (fid->type == ftype);
}
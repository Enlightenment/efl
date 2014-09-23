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

   if (fid->base.file) eina_stringshare_del(fid->base.file);
   eina_stringshare_del(fid->name);
   EINA_LIST_FREE(fid->keys, param) database_parameter_del(param);
   EINA_LIST_FREE(fid->params, param) database_parameter_del(param);
   EINA_LIST_FREE(fid->ctor_of, cls_name) eina_stringshare_del(cls_name);
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
      eina_stringshare_ref(cls->full_name));
}

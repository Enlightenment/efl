#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"

EAPI Eolian_Object_Scope
eolian_function_scope_get(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EOLIAN_SCOPE_PUBLIC);
   return fid->scope;
}

EAPI Eolian_Function_Type
eolian_function_type_get(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EOLIAN_UNRESOLVED);
   return fid->type;
}

EAPI Eina_Stringshare *
eolian_function_name_get(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   return fid->name;
}

static const char *
_get_eo_prefix(const Eolian_Function *foo_id, char *buf, Eina_Bool use_legacy)
{
    char *tmp = buf;
    if (use_legacy)
      return foo_id->klass->legacy_prefix;
    else if (foo_id->klass->eo_prefix)
      return foo_id->klass->eo_prefix;
    strcpy(buf, foo_id->klass->full_name);
    eina_str_tolower(&buf);
    while ((tmp = strchr(tmp, '.'))) *tmp = '_';
    return buf;
}

EAPI Eina_Stringshare *
eolian_function_full_c_name_get(const Eolian_Function *foo_id,
                                Eolian_Function_Type ftype,
                                Eina_Bool use_legacy)
{
   switch (ftype)
     {
      case EOLIAN_UNRESOLVED:
      case EOLIAN_METHOD:
      case EOLIAN_PROPERTY:
      case EOLIAN_PROP_GET:
        if (foo_id->get_legacy && use_legacy)
          {
             if (!strcmp(foo_id->get_legacy, "null"))
               return NULL;
             return eina_stringshare_ref(foo_id->get_legacy);
          }
        break;
      case EOLIAN_PROP_SET:
        if (foo_id->set_legacy && use_legacy)
          {
             if (!strcmp(foo_id->set_legacy, "null"))
               return NULL;
             return eina_stringshare_ref(foo_id->set_legacy);
          }
        break;
     }

   char tbuf[512];
   const char *prefix = _get_eo_prefix(foo_id, tbuf, use_legacy);

   if (!prefix)
     return NULL;

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

   if (use_legacy || (int)strlen(last_p) != len || strncmp(last_p, funcn, len))
     {
        eina_strbuf_append(buf, prefix);
        eina_strbuf_append_char(buf, '_');
        eina_strbuf_append(buf, funcn);

        if ((ftype == EOLIAN_PROP_GET) || (ftype == EOLIAN_PROPERTY))
          eina_strbuf_append(buf, "_get");
        else if (ftype == EOLIAN_PROP_SET)
          eina_strbuf_append(buf, "_set");

        ret = eina_stringshare_add(eina_strbuf_string_get(buf));
        eina_strbuf_free(buf);
        return ret;
     }

   if (last_p != prefix)
      eina_strbuf_append_n(buf, prefix, last_p - prefix); /* includes _ */

   eina_strbuf_append(buf, funcn);

   if ((ftype == EOLIAN_PROP_GET) || (ftype == EOLIAN_PROPERTY))
     eina_strbuf_append(buf, "_get");
   else if (ftype == EOLIAN_PROP_SET)
     eina_strbuf_append(buf, "_set");

   ret = eina_stringshare_add(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   return ret;
}

EAPI Eina_Stringshare *
eolian_function_legacy_get(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   switch (ftype)
     {
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROPERTY: case EOLIAN_PROP_GET: return fid->get_legacy; break;
      case EOLIAN_PROP_SET: return fid->set_legacy; break;
      default: return NULL;
     }
}

EAPI const Eolian_Documentation *
eolian_function_documentation_get(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   switch (ftype)
     {
      case EOLIAN_PROP_GET: return fid->get_doc; break;
      case EOLIAN_PROP_SET: return fid->set_doc; break;
      default: return fid->common_doc;
     }
}

EAPI Eina_Bool
eolian_function_is_virtual_pure(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROPERTY: case EOLIAN_PROP_GET: return fid->get_virtual_pure; break;
      case EOLIAN_PROP_SET: return fid->set_virtual_pure; break;
      default: return EINA_FALSE;
     }
}

EAPI Eina_Bool
eolian_function_is_auto(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROPERTY: case EOLIAN_PROP_GET: return fid->get_auto; break;
      case EOLIAN_PROP_SET: return fid->set_auto; break;
      default: return EINA_FALSE;
     }
}

EAPI Eina_Bool
eolian_function_is_empty(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROPERTY: case EOLIAN_PROP_GET: return fid->get_empty; break;
      case EOLIAN_PROP_SET: return fid->set_empty; break;
      default: return EINA_FALSE;
     }
}

EAPI Eina_Bool
eolian_function_is_legacy_only(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROPERTY: case EOLIAN_PROP_GET: return fid->get_only_legacy; break;
      case EOLIAN_PROP_SET: return fid->set_only_legacy; break;
      default: return EINA_FALSE;
     }
}

EAPI Eina_Bool
eolian_function_is_class(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   return fid->is_class;
}

EAPI Eina_Bool
eolian_function_is_constructor(const Eolian_Function *fid, const Eolian_Class *klass)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   Eina_Stringshare *s = eina_stringshare_ref(klass->full_name);
   Eina_Bool r = !!eina_list_search_sorted_list
     (fid->ctor_of, EINA_COMPARE_CB(strcmp), s);
   eina_stringshare_del(s);
   return r;
}

static Eina_List *
_get_prop_keys(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   Eina_List *l = fid->prop_keys_get;
   if (ftype == EOLIAN_PROP_SET) l = fid->prop_keys_set;
   if (!l) return fid->prop_keys;
   return l;
}

static Eina_List *
_get_prop_values(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   Eina_List *l = fid->prop_values_get;
   if (ftype == EOLIAN_PROP_SET) l = fid->prop_values_set;
   if (!l) return fid->prop_values;
   return l;
}

EAPI Eina_Iterator *
eolian_property_keys_get(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   Eina_List *l = NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   if (ftype != EOLIAN_PROP_GET && ftype != EOLIAN_PROP_SET)
     return NULL;
   l = _get_prop_keys(fid, ftype);
   return (l ? eina_list_iterator_new(l) : NULL);
}

EAPI Eina_Iterator *
eolian_property_values_get(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   Eina_List *l = NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   if (ftype != EOLIAN_PROP_GET && ftype != EOLIAN_PROP_SET)
     return NULL;
   l = _get_prop_values(fid, ftype);
   return (l ? eina_list_iterator_new(l) : NULL);
}

EAPI Eina_Iterator *
eolian_function_parameters_get(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   if (fid->type != EOLIAN_METHOD)
     return NULL;
   return (fid->params ? eina_list_iterator_new(fid->params) : NULL);
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

EAPI const Eolian_Expression *
eolian_function_return_default_value_get(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   switch (ftype)
     {
      case EOLIAN_PROP_SET: return fid->set_ret_val;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROPERTY: case EOLIAN_PROP_GET: return fid->get_ret_val;
      default: return NULL;
     }
}

EAPI const Eolian_Documentation *
eolian_function_return_documentation_get(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   switch (ftype)
     {
      case EOLIAN_PROP_SET: return fid->set_return_doc; break;
      case EOLIAN_UNRESOLVED: case EOLIAN_METHOD: case EOLIAN_PROPERTY: case EOLIAN_PROP_GET: return fid->get_return_doc; break;
      default: return NULL;
     }
}

EAPI Eina_Bool
eolian_function_return_is_warn_unused(const Eolian_Function *fid,
      Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_METHOD: case EOLIAN_PROP_GET: case EOLIAN_PROPERTY: return fid->get_return_warn_unused;
      case EOLIAN_PROP_SET: return fid->set_return_warn_unused;
      default: return EINA_FALSE;
     }
}

EAPI Eina_Bool
eolian_function_object_is_const(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   return fid->obj_is_const;
}

EAPI const Eolian_Class *
eolian_function_class_get(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   return fid->klass;
}

EAPI Eina_Bool
eolian_function_is_c_only(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   return fid->is_c_only;
}

EAPI Eina_Bool
eolian_function_is_beta(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   return fid->is_beta;
}

EAPI Eina_Bool eolian_function_is_implemented(
      const Eolian_Function *function_id, Eolian_Function_Type func_type,
      const Eolian_Class *klass)
{
   Eina_Iterator *impl_itr = NULL;
   Eolian_Function_Type found_type = EOLIAN_UNRESOLVED;
   Eina_Bool found = EINA_TRUE;
   if (!function_id || !klass) return EINA_FALSE;
   Eina_List *list = eina_list_append(NULL, klass), *list2, *itr;
   EINA_LIST_FOREACH(list, itr, klass)
     {
        const char *inherit_name;
        const Eolian_Implement *impl;
        if (eolian_class_type_get(klass) == EOLIAN_CLASS_INTERFACE) continue;
        impl_itr = eolian_class_implements_get(klass);
        EINA_ITERATOR_FOREACH(impl_itr, impl)
          {
             if (eolian_implement_is_virtual(impl)) continue;
             Eolian_Function_Type impl_type = EOLIAN_UNRESOLVED;
             const Eolian_Function *impl_func = eolian_implement_function_get(impl, &impl_type);
             if (impl_func == function_id)
               {
                  /* The type matches the requested or is not important for the caller */
                  if (func_type == EOLIAN_UNRESOLVED || impl_type == func_type) goto end;
                  if (impl_type == EOLIAN_METHOD) continue;
                  /* In case we search for a property type */
                  if (impl_type == EOLIAN_PROPERTY &&
                        (func_type == EOLIAN_PROP_GET || func_type == EOLIAN_PROP_SET))
                     goto end;
                  /* Property may be splitted on multiple implements */
                  if (func_type == EOLIAN_PROPERTY)
                    {
                       if (found_type == EOLIAN_UNRESOLVED) found_type = impl_type;
                       if ((found_type == EOLIAN_PROP_SET && impl_type == EOLIAN_PROP_GET) ||
                             (found_type == EOLIAN_PROP_GET && impl_type == EOLIAN_PROP_SET))
                          goto end;
                    }
               }
          }
        eina_iterator_free(impl_itr);
        impl_itr = NULL;

        Eina_Iterator *inherits_itr = eolian_class_inherits_get(klass);
        EINA_ITERATOR_FOREACH(inherits_itr, inherit_name)
          {
             const Eolian_Class *inherit = eolian_class_get_by_name(inherit_name);
             /* Avoid duplicates. */
             if (!eina_list_data_find(list, inherit))
               {
                  list2 = eina_list_append(list, inherit);
               }
          }
        eina_iterator_free(inherits_itr);
     }
   (void) list2;
   found = EINA_FALSE;
end:
   if (impl_itr) eina_iterator_free(impl_itr);
   eina_list_free(list);
   return found;
}


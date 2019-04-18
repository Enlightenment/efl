#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eolian_database.h"

EAPI Eolian_Object_Scope
eolian_function_scope_get(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EOLIAN_SCOPE_UNKNOWN);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_UNRESOLVED, EOLIAN_SCOPE_UNKNOWN);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_PROPERTY, EOLIAN_SCOPE_UNKNOWN);
   switch (ftype)
     {
      case EOLIAN_METHOD:
        if (fid->type != EOLIAN_METHOD)
          return EOLIAN_SCOPE_UNKNOWN;
        return fid->get_scope;
      case EOLIAN_PROP_GET:
        if ((fid->type != EOLIAN_PROP_GET) && (fid->type != EOLIAN_PROPERTY))
          return EOLIAN_SCOPE_UNKNOWN;
        return fid->get_scope;
      case EOLIAN_PROP_SET:
        if ((fid->type != EOLIAN_PROP_SET) && (fid->type != EOLIAN_PROPERTY))
          return EOLIAN_SCOPE_UNKNOWN;
        return fid->set_scope;
      default:
        return EOLIAN_SCOPE_UNKNOWN;
     }
}

EAPI Eolian_Function_Type
eolian_function_type_get(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EOLIAN_UNRESOLVED);
   return fid->type;
}

static const char *
_get_eo_prefix(const Eolian_Function *foo_id, char *buf)
{
    char *tmp = buf;
    if (foo_id->klass->eo_prefix)
      return foo_id->klass->eo_prefix;
    strcpy(buf, foo_id->klass->base.name);
    eina_str_tolower(&buf);
    while ((tmp = strchr(tmp, '.'))) *tmp = '_';
    return buf;
}

static char *
_get_abbreviated_name(const char *prefix, const char *fname)
{
   Eina_Strbuf *buf = eina_strbuf_new();

   const char *last_p = strrchr(prefix, '_');
   last_p = (last_p) ? (last_p + 1) : prefix;

   const char *tmp = strstr(fname, last_p);
   int len = strlen(last_p);

   if ((tmp) &&
       ((tmp == fname) || (*(tmp - 1) == '_')) &&
       ((*(tmp + len) == '\0') || (*(tmp + len) == '_')))
     {
        int plen = strlen(prefix);
        len += (tmp - fname);

        if ((plen >= len) && !strncmp(prefix + plen - len, fname, len))
          {
             eina_strbuf_append_n(buf, prefix, plen - len);
          }
     }

   if (eina_strbuf_length_get(buf) == 0)
     eina_strbuf_append_printf(buf, "%s_", prefix);
   eina_strbuf_append(buf, fname);

   char *ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

EAPI Eina_Stringshare *
eolian_function_full_c_name_get(const Eolian_Function *foo_id,
                                Eolian_Function_Type ftype)
{
   char tbuf[512];
   tbuf[0] = '\0';
   const char *prefix = (ftype != EOLIAN_FUNCTION_POINTER) ? _get_eo_prefix(foo_id, tbuf): tbuf;

   if (!prefix)
     return NULL;

   const char  *funcn = eolian_function_name_get(foo_id);
   Eina_Strbuf *buf = eina_strbuf_new();
   Eina_Stringshare *ret;

   char *abbr = _get_abbreviated_name(prefix, funcn);
   eina_strbuf_append(buf, abbr);
   free(abbr);

   if ((ftype == EOLIAN_PROP_GET) || (ftype == EOLIAN_PROPERTY))
     eina_strbuf_append(buf, "_get");
   else if (ftype == EOLIAN_PROP_SET)
     eina_strbuf_append(buf, "_set");

   ret = eina_stringshare_add(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   return ret;
}

EAPI const Eolian_Implement *
eolian_function_implement_get(const Eolian_Function *fid)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   return fid->impl;
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
   Eina_Stringshare *s = eina_stringshare_ref(klass->base.name);
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
   if (fid->type != EOLIAN_METHOD && fid->type != EOLIAN_FUNCTION_POINTER)
     return NULL;
   return (fid->params ? eina_list_iterator_new(fid->params) : NULL);
}

EAPI const Eolian_Type *
eolian_function_return_type_get(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_UNRESOLVED, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_PROPERTY, NULL);
   switch (ftype)
     {
      case EOLIAN_METHOD:
        if (fid->type != EOLIAN_METHOD)
          return NULL;
        return fid->get_ret_type;
      case EOLIAN_PROP_GET:
        if ((fid->type != EOLIAN_PROP_GET) && (fid->type != EOLIAN_PROPERTY))
          return NULL;
        return fid->get_ret_type;
      case EOLIAN_PROP_SET:
        if ((fid->type != EOLIAN_PROP_SET) && (fid->type != EOLIAN_PROPERTY))
          return NULL;
        return fid->set_ret_type;
      case EOLIAN_FUNCTION_POINTER:
        return (fid->type != EOLIAN_FUNCTION_POINTER) ? NULL : fid->get_ret_type;
      default:
        return NULL;
     }
}

EAPI const Eolian_Expression *
eolian_function_return_default_value_get(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_UNRESOLVED, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_PROPERTY, NULL);
   switch (ftype)
     {
      case EOLIAN_METHOD:
        if (fid->type != EOLIAN_METHOD)
          return NULL;
        return fid->get_ret_val;
      case EOLIAN_PROP_GET:
        if ((fid->type != EOLIAN_PROP_GET) && (fid->type != EOLIAN_PROPERTY))
          return NULL;
        return fid->get_ret_val;
      case EOLIAN_PROP_SET:
        if ((fid->type != EOLIAN_PROP_SET) && (fid->type != EOLIAN_PROPERTY))
          return NULL;
        return fid->set_ret_val;
      default:
        return NULL;
     }
}

EAPI const Eolian_Documentation *
eolian_function_return_documentation_get(const Eolian_Function *fid, Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_UNRESOLVED, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_PROPERTY, NULL);
   switch (ftype)
     {
      case EOLIAN_METHOD:
        if (fid->type != EOLIAN_METHOD)
          return NULL;
        return fid->get_return_doc;
      case EOLIAN_PROP_GET:
        if ((fid->type != EOLIAN_PROP_GET) && (fid->type != EOLIAN_PROPERTY))
          return NULL;
        return fid->get_return_doc;
      case EOLIAN_PROP_SET:
        if ((fid->type != EOLIAN_PROP_SET) && (fid->type != EOLIAN_PROPERTY))
          return NULL;
        return fid->set_return_doc;
      default:
        return NULL;
     }
}

EAPI Eina_Bool
eolian_function_return_is_warn_unused(const Eolian_Function *fid,
      Eolian_Function_Type ftype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fid, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_UNRESOLVED, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ftype != EOLIAN_PROPERTY, EINA_FALSE);
   switch (ftype)
     {
      case EOLIAN_METHOD:
        if (fid->type != EOLIAN_METHOD)
          return EINA_FALSE;
        return fid->get_return_warn_unused;
      case EOLIAN_PROP_GET:
        if ((fid->type != EOLIAN_PROP_GET) && (fid->type != EOLIAN_PROPERTY))
          return EINA_FALSE;
        return fid->get_return_warn_unused;
      case EOLIAN_PROP_SET:
        if ((fid->type != EOLIAN_PROP_SET) && (fid->type != EOLIAN_PROPERTY))
          return EINA_FALSE;
        return fid->set_return_warn_unused;
      default:
        return EINA_FALSE;
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

#include <ctype.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eo_lexer.h"

static Eina_Bool
_validate_ref(const char *ref, const Eolian_Object *info)
{
   if (eolian_declaration_get_by_name(ref))
     return EINA_TRUE;

   const char *suffix = strrchr(ref, '.');
   if (!suffix) goto failed;

   Eina_Stringshare *base = eina_stringshare_add_length(ref, suffix - ref);

   const Eolian_Type *tp = eolian_type_struct_get_by_name(base);
   if (tp)
     {
        eina_stringshare_del(base);
        if (!eolian_type_struct_field_get(tp, suffix + 1))
          goto failed;
        return EINA_TRUE;
     }

   tp = eolian_type_enum_get_by_name(base);
   if (tp)
     {
        eina_stringshare_del(base);
        if (!eolian_type_enum_field_get(tp, suffix + 1))
          goto failed;
        return EINA_TRUE;
     }

   const Eolian_Class *cl = eolian_class_get_by_name(base);
   if (cl)
     {
        eina_stringshare_del(base);
        if (!eolian_class_function_get_by_name(cl, suffix + 1, EOLIAN_UNRESOLVED))
          goto failed;
        return EINA_TRUE;
     }

   Eolian_Function_Type ftype = EOLIAN_UNRESOLVED;
   if (!strcmp(suffix, ".get"))
     ftype = EOLIAN_PROP_GET;
   else if (!strcmp(suffix, ".set"))
     ftype = EOLIAN_PROP_SET;

   const char *meth;
   if (ftype != EOLIAN_UNRESOLVED)
     {
        eina_stringshare_del(base);
        meth = suffix - 1;
        while ((meth != ref) && (*meth != '.')) --meth;
        if (meth == ref) goto failed;
        base = eina_stringshare_add_length(ref, meth - ref);
        cl = eolian_class_get_by_name(base);
        eina_stringshare_del(base);
     }

   if (!cl) goto failed;

   char *ameth = strndup(meth + 1, suffix - meth - 1);
   const Eolian_Function *fn = eolian_class_function_get_by_name(cl, ameth, ftype);
   free(ameth);

   if (!fn) goto failed;
   return EINA_TRUE;

failed:
   fprintf(stderr, "eolian:%s:%d:%d: failed validating reference '%s'\n",
           info->file, info->line, info->column, ref);
   return EINA_FALSE;
}

static Eina_Bool
_validate_docstr(Eina_Stringshare *str, const Eolian_Object *info)
{
   if (!str) return EINA_TRUE;

   const char *p;
   for (p = strchr(str, '@'); p; p = strchr(p, '@'))
     {
        ++p;
        /* escaped refs */
        if ((p > (str + 1)) && (*(p - 2) == '\\'))
          continue;
        /* are we starting a reference? */
        const char *ref = p;
        if (!isalpha(*p) && (*p != '_'))
          continue;
        ++p;
        /* check the rest of the reference */
        while (isalnum(*p) || (*p == '.') || (*p == '_'))
          ++p;
        if (*(p - 1) == '.') --p;
        Eina_Stringshare *refs = eina_stringshare_add_length(ref, (p - ref));
        if (!_validate_ref(refs, info))
          {
             eina_stringshare_del(refs);
             return EINA_FALSE;
          }
        eina_stringshare_del(refs);
     }

   return EINA_TRUE;
}

static Eina_Bool
_validate_doc(const Eolian_Documentation *doc)
{
   if (!doc) return EINA_TRUE;

   if (!_validate_docstr(doc->summary, &doc->base))
     return EINA_FALSE;
   if (!_validate_docstr(doc->description, &doc->base))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool _validate_type(const Eolian_Type *tp);
static Eina_Bool _validate_expr(const Eolian_Expression *expr,
                                const Eolian_Type *tp,
                                Eolian_Expression_Mask msk);

static Eina_Bool
_sf_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
           const Eolian_Struct_Type_Field *sf, Eina_Bool *success)
{
   *success = _validate_type(sf->type);

   if (!*success)
     return EINA_FALSE;

   *success = _validate_doc(sf->doc);

   return *success;
}

static Eina_Bool
_ef_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
           const Eolian_Enum_Type_Field *ef, Eina_Bool *success)
{
   if (ef->value)
     *success = _validate_expr(ef->value, NULL, EOLIAN_MASK_INT);
   else
     *success = EINA_TRUE;

   if (!*success)
     return EINA_FALSE;

   *success = _validate_doc(ef->doc);

   return *success;
}

static Eina_Bool
_type_error(const Eolian_Type *tp, const char *msg)
{
   eina_log_print(_eolian_log_dom, EINA_LOG_LEVEL_WARN, tp->base.file, "",
                  tp->base.line, "%s at column %d", msg, tp->base.column);
   return EINA_FALSE;
}

static Eina_Bool
_validate_type(const Eolian_Type *tp)
{
   if (!_validate_doc(tp->doc))
     return EINA_FALSE;

   switch (tp->type)
     {
      case EOLIAN_TYPE_VOID:
      case EOLIAN_TYPE_UNDEFINED:
      case EOLIAN_TYPE_COMPLEX:
        return EINA_TRUE;
      case EOLIAN_TYPE_REGULAR:
        {
           const Eolian_Type *tpp;
           /* builtins */
           int id = eo_lexer_keyword_str_to_id(tp->full_name);
           if (id)
             return eo_lexer_is_type_keyword(id);
           /* user defined */
           tpp = eolian_type_base_type_get(tp);
           if (!tpp)
             {
                char buf[256];
                snprintf(buf, sizeof(buf), "undefined type %s", tp->full_name);
                _type_error(tp, buf);
                return EINA_TRUE; /* for now only warn */
             }
           return _validate_type(tpp);
        }
      case EOLIAN_TYPE_POINTER:
      case EOLIAN_TYPE_ALIAS:
        return _validate_type(tp->base_type);
      case EOLIAN_TYPE_STRUCT:
        {
           Eina_Bool succ = EINA_TRUE;
           eina_hash_foreach(tp->fields, (Eina_Hash_Foreach)_sf_map_cb, &succ);
           return succ;
        }
      case EOLIAN_TYPE_STRUCT_OPAQUE:
        return EINA_TRUE;
      case EOLIAN_TYPE_ENUM:
        {
           Eina_Bool succ = EINA_TRUE;
           eina_hash_foreach(tp->fields, (Eina_Hash_Foreach)_ef_map_cb, &succ);
           return succ;
        }
      case EOLIAN_TYPE_CLASS:
        {
           if (!eolian_type_class_get(tp))
             {
                /* for now only warn */
                char buf[256];
                snprintf(buf, sizeof(buf), "undefined class %s "
                         "(likely wrong namespacing)", tp->full_name);
                _type_error(tp, buf);
             }
           return EINA_TRUE;
        }
      default:
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_validate_expr(const Eolian_Expression *expr, const Eolian_Type *tp,
               Eolian_Expression_Mask msk)
{
   Eolian_Value val;
   /* TODO: enable later, for now we can't (unfinished interfaces */
   if (tp)
     {
#if 0
        val = eolian_expression_eval_type(expr, tp);
#else
        return EINA_TRUE;
#endif
     }
   else
      val = eolian_expression_eval(expr, msk);
   return (val.type != EOLIAN_EXPR_UNKNOWN);
}

static Eina_Bool
_validate_param(const Eolian_Function_Parameter *param)
{
   if (!_validate_type(param->type))
     return EINA_FALSE;

   if (!_validate_doc(param->doc))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_validate_function(const Eolian_Function *func)
{
   Eina_List *l;
   const Eolian_Function_Parameter *param;

   if (func->get_ret_type && !_validate_type(func->get_ret_type))
     return EINA_FALSE;

   if (func->set_ret_type && !_validate_type(func->set_ret_type))
     return EINA_FALSE;

   if (func->get_ret_val && !_validate_expr(func->get_ret_val,
                                            func->get_ret_type, 0))
     return EINA_FALSE;

   if (func->set_ret_val && !_validate_expr(func->set_ret_val,
                                            func->set_ret_type, 0))
     return EINA_FALSE;

#define EOLIAN_PARAMS_VALIDATE(params) \
   EINA_LIST_FOREACH(params, l, param) \
     if (!_validate_param(param)) \
       return EINA_FALSE;

   EOLIAN_PARAMS_VALIDATE(func->prop_values);
   EOLIAN_PARAMS_VALIDATE(func->prop_values_get);
   EOLIAN_PARAMS_VALIDATE(func->prop_values_set);
   EOLIAN_PARAMS_VALIDATE(func->prop_keys);
   EOLIAN_PARAMS_VALIDATE(func->prop_keys_get);
   EOLIAN_PARAMS_VALIDATE(func->prop_keys_set);

#undef EOLIAN_PARAMS_VALIDATE

   if (!_validate_doc(func->common_doc))
     return EINA_FALSE;
   if (!_validate_doc(func->get_doc))
     return EINA_FALSE;
   if (!_validate_doc(func->set_doc))
     return EINA_FALSE;
   if (!_validate_doc(func->get_return_doc))
     return EINA_FALSE;
   if (!_validate_doc(func->set_return_doc))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_validate_event(const Eolian_Event *event)
{
   if (event->type && !_validate_type(event->type))
     return EINA_FALSE;

   if (!_validate_doc(event->doc))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_validate_class(const Eolian_Class *cl)
{
   Eina_List *l;
   const Eolian_Function *func;
   const Eolian_Event *event;

   EINA_LIST_FOREACH(cl->properties, l, func)
     if (!_validate_function(func))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->methods, l, func)
     if (!_validate_function(func))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->events, l, event)
     if (!_validate_event(event))
       return EINA_FALSE;

   if (!_validate_doc(cl->doc))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_validate_variable(const Eolian_Variable *var)
{
   if (!_validate_type(var->base_type))
     return EINA_FALSE;

   if (var->value && !_validate_expr(var->value, var->base_type, 0))
     return EINA_FALSE;

   if (!_validate_doc(var->doc))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_type_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
             const Eolian_Type *tp, Eina_Bool *success)
{
   *success = _validate_type(tp);
   return *success;
}

static Eina_Bool
_var_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
             const Eolian_Variable *var, Eina_Bool *success)
{
   *success = _validate_variable(var);
   return *success;
}

Eina_Bool
database_validate(void)
{
   const Eolian_Class *cl;

   Eina_Iterator *iter = eolian_all_classes_get();
   EINA_ITERATOR_FOREACH(iter, cl)
     if (!_validate_class(cl))
       {
          eina_iterator_free(iter);
          return EINA_FALSE;
       }
   eina_iterator_free(iter);

   Eina_Bool succ = EINA_TRUE;

   eina_hash_foreach(_aliases, (Eina_Hash_Foreach)_type_map_cb, &succ);
   if (!succ)
     return EINA_FALSE;

   eina_hash_foreach(_structs, (Eina_Hash_Foreach)_type_map_cb, &succ);
   if (!succ)
     return EINA_FALSE;

   eina_hash_foreach(_enums, (Eina_Hash_Foreach)_type_map_cb, &succ);
   if (!succ)
     return EINA_FALSE;

   eina_hash_foreach(_globals, (Eina_Hash_Foreach)_var_map_cb, &succ);
   if (!succ)
     return EINA_FALSE;

   eina_hash_foreach(_constants, (Eina_Hash_Foreach)_var_map_cb, &succ);
   if (!succ)
     return EINA_FALSE;

   return EINA_TRUE;
}

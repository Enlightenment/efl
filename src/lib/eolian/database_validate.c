#include <ctype.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eo_lexer.h"

static Eina_Bool
_validate_docstr(Eina_Stringshare *str, const Eolian_Object *info)
{
   if (!str || !str[0]) return EINA_TRUE;

   Eina_Bool ret = EINA_TRUE;
   Eina_List *pl = eolian_documentation_string_split(str);
   char *par;
   EINA_LIST_FREE(pl, par)
     {
        const char *doc = par;
        Eolian_Doc_Token tok;
        eolian_doc_token_init(&tok);
        while (ret && (doc = eolian_documentation_tokenize(doc, &tok)))
          if (eolian_doc_token_type_get(&tok) == EOLIAN_DOC_TOKEN_REF)
            /* FIXME: pass unit properly */
            if (eolian_doc_token_ref_get(NULL, &tok, NULL, NULL) == EOLIAN_DOC_REF_INVALID)
              {
                 char *refn = eolian_doc_token_text_get(&tok);
                 fprintf(stderr, "eolian:%s:%d:%d: failed validating reference '%s'\n",
                         info->file, info->line, info->column, refn);
                 free(refn);
                 ret = EINA_FALSE;
                 break;
              }
        free(par);
     }

   return ret;
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
           const Eolian_Struct_Type_Field *sf, Eina_Bool *sc)
{
   *sc = _validate_type(sf->type);

   if (!*sc)
     return EINA_FALSE;

   *sc = _validate_doc(sf->doc);

   return *sc;
}

static Eina_Bool
_ef_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
           const Eolian_Enum_Type_Field *ef, Eina_Bool *sc)
{
   if (ef->value)
     *sc = _validate_expr(ef->value, NULL, EOLIAN_MASK_INT);
   else
     *sc = EINA_TRUE;

   if (!*sc)
     return EINA_FALSE;

   *sc = _validate_doc(ef->doc);

   return *sc;
}

static Eina_Bool
_type_error(const Eolian_Type *tp, const char *msg)
{
   fprintf(stderr, "eolian:%s:%d:%d: %s\n", tp->base.file, tp->base.line,
           tp->base.column, msg);
   return EINA_FALSE;
}

static Eina_Bool
_validate_typedecl(const Eolian_Typedecl *tp)
{
   if (!_validate_doc(tp->doc))
     return EINA_FALSE;

   switch (tp->type)
     {
      case EOLIAN_TYPEDECL_ALIAS:
        return _validate_type(tp->base_type);
      case EOLIAN_TYPEDECL_STRUCT:
        {
           Eina_Bool succ = EINA_TRUE;
           eina_hash_foreach(tp->fields, (Eina_Hash_Foreach)_sf_map_cb, &succ);
           return succ;
        }
      case EOLIAN_TYPEDECL_STRUCT_OPAQUE:
        return EINA_TRUE;
      case EOLIAN_TYPEDECL_ENUM:
        {
           Eina_Bool succ = EINA_TRUE;
           eina_hash_foreach(tp->fields, (Eina_Hash_Foreach)_ef_map_cb, &succ);
           return succ;
        }
      case EOLIAN_TYPEDECL_FUNCTION_POINTER:
        // FIXME validate functions here
        return EINA_TRUE;
      default:
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_validate_type(const Eolian_Type *tp)
{
   switch (tp->type)
     {
      case EOLIAN_TYPE_VOID:
      case EOLIAN_TYPE_UNDEFINED:
      case EOLIAN_TYPE_COMPLEX:
        return EINA_TRUE;
      case EOLIAN_TYPE_REGULAR:
        {
           const Eolian_Typedecl *tpp;
           /* builtins */
           int id = eo_lexer_keyword_str_to_id(tp->full_name);
           if (id)
             return eo_lexer_is_type_keyword(id);
           /* user defined */
           tpp = eolian_type_typedecl_get(tp);
           if (!tpp)
             {
                char buf[256];
                snprintf(buf, sizeof(buf), "undefined type %s", tp->full_name);
                return _type_error(tp, buf);
             }
           return _validate_typedecl(tpp);
        }
      case EOLIAN_TYPE_STATIC_ARRAY:
      case EOLIAN_TYPE_TERMINATED_ARRAY:
        return _validate_type(tp->base_type);
      case EOLIAN_TYPE_CLASS:
        {
           /* FIXME: pass unit properly */
           if (!eolian_type_class_get(NULL, tp))
             {
                char buf[256];
                snprintf(buf, sizeof(buf), "undefined class %s "
                         "(likely wrong namespacing)", tp->full_name);
                return _type_error(tp, buf);
             }
           return EINA_TRUE;
        }
      default:
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_validate_expr(const Eolian_Expression *expr,
               const Eolian_Type *tp, Eolian_Expression_Mask msk)
{
   Eolian_Value val;
   /* FIXME: pass unit properly */
   if (tp)
     val = eolian_expression_eval_type(NULL, expr, tp);
   else
     val = eolian_expression_eval(NULL, expr, msk);
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
_validate_implement(const Eolian_Implement *impl)
{
   if (!_validate_doc(impl->common_doc))
     return EINA_FALSE;
   if (!_validate_doc(impl->get_doc))
     return EINA_FALSE;
   if (!_validate_doc(impl->set_doc))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_validate_class(const Eolian_Class *cl)
{
   Eina_List *l;
   const Eolian_Function *func;
   const Eolian_Event *event;
   const Eolian_Implement *impl;

   EINA_LIST_FOREACH(cl->properties, l, func)
     if (!_validate_function(func))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->methods, l, func)
     if (!_validate_function(func))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->events, l, event)
     if (!_validate_event(event))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->implements, l, impl)
     if (!_validate_implement(impl))
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
_typedecl_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
                 const Eolian_Typedecl *tp, Eina_Bool *sc)
{
   return (*sc = _validate_typedecl(tp));
}

static Eina_Bool
_var_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
             const Eolian_Variable *var, Eina_Bool *sc)
{
   return (*sc = _validate_variable(var));
}

Eina_Bool
database_validate()
{
   const Eolian_Class *cl;

   /* FIXME: pass unit properly */
   Eina_Iterator *iter = eolian_all_classes_get(NULL);
   EINA_ITERATOR_FOREACH(iter, cl)
     if (!_validate_class(cl))
       {
          eina_iterator_free(iter);
          return EINA_FALSE;
       }
   eina_iterator_free(iter);

   Eina_Bool succ = EINA_TRUE;

   eina_hash_foreach(_aliases, (Eina_Hash_Foreach)_typedecl_map_cb, &succ);
   if (!succ)
     return EINA_FALSE;

   eina_hash_foreach(_structs, (Eina_Hash_Foreach)_typedecl_map_cb, &succ);
   if (!succ)
     return EINA_FALSE;

   eina_hash_foreach(_enums, (Eina_Hash_Foreach)_typedecl_map_cb, &succ);
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

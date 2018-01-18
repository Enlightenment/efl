#include <ctype.h>
#include <assert.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eo_lexer.h"
#include "eolian_priv.h"

static Eina_Bool
_validate(Eolian_Object *obj)
{
   obj->validated = EINA_TRUE;
   return EINA_TRUE;
}

static Eina_Bool
_validate_docstr(const Eolian_Unit *src, Eina_Stringshare *str, const Eolian_Object *info)
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
            if (eolian_doc_token_ref_get(src, &tok, NULL, NULL) == EOLIAN_DOC_REF_INVALID)
              {
                 char *refn = eolian_doc_token_text_get(&tok);
                 _eolian_log_line(info->file, info->line, info->column,
                                  "failed validating reference '%s'", refn);
                 free(refn);
                 ret = EINA_FALSE;
                 break;
              }
        free(par);
     }

   return ret;
}

static Eina_Bool
_validate_doc(const Eolian_Unit *src, Eolian_Documentation *doc)
{
   if (!doc)
     return EINA_TRUE;

   if (!_validate_docstr(src, doc->summary, &doc->base))
     return EINA_FALSE;
   if (!_validate_docstr(src, doc->description, &doc->base))
     return EINA_FALSE;

   return _validate(&doc->base);
}

static Eina_Bool _validate_type(const Eolian_Unit *src, Eolian_Type *tp);
static Eina_Bool _validate_expr(const Eolian_Unit *src,
                                Eolian_Expression *expr,
                                const Eolian_Type *tp,
                                Eolian_Expression_Mask msk);
static Eina_Bool _validate_function(const Eolian_Unit *src,
                                    Eolian_Function *func,
                                    Eina_Hash *nhash);

typedef struct _Cb_Ret
{
   const Eolian_Unit *unit;
   Eina_Bool succ;
} Cb_Ret;

static Eina_Bool
_sf_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
           const Eolian_Struct_Type_Field *sf, Cb_Ret *sc)
{
   sc->succ = _validate_type(sc->unit, sf->type);

   if (!sc->succ)
     return EINA_FALSE;

   sc->succ = _validate_doc(sc->unit, sf->doc);

   return sc->succ;
}

static Eina_Bool
_ef_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
           const Eolian_Enum_Type_Field *ef, Cb_Ret *sc)
{
   if (ef->value)
     sc->succ = _validate_expr(sc->unit, ef->value, NULL, EOLIAN_MASK_INT);
   else
     sc->succ = EINA_TRUE;

   if (!sc->succ)
     return EINA_FALSE;

   sc->succ = _validate_doc(sc->unit, ef->doc);

   return sc->succ;
}

static Eina_Bool
_obj_error(const Eolian_Object *o, const char *msg)
{
   _eolian_log_line(o->file, o->line, o->column, "%s", msg);
   return EINA_FALSE;
}

static Eina_Bool
_validate_typedecl(const Eolian_Unit *src, Eolian_Typedecl *tp)
{
   if (tp->base.validated)
     return EINA_TRUE;

   if (!_validate_doc(src, tp->doc))
     return EINA_FALSE;

   switch (tp->type)
     {
      case EOLIAN_TYPEDECL_ALIAS:
        if (!_validate_type(src, tp->base_type))
          return EINA_FALSE;
        if (!tp->freefunc && tp->base_type->freefunc)
          tp->freefunc = eina_stringshare_ref(tp->base_type->freefunc);
        return _validate(&tp->base);
      case EOLIAN_TYPEDECL_STRUCT:
        {
           Cb_Ret rt = { src, EINA_TRUE };
           eina_hash_foreach(tp->fields, (Eina_Hash_Foreach)_sf_map_cb, &rt);
           if (!rt.succ)
             return EINA_FALSE;
           return _validate(&tp->base);
        }
      case EOLIAN_TYPEDECL_STRUCT_OPAQUE:
        return _validate(&tp->base);
      case EOLIAN_TYPEDECL_ENUM:
        {
           Cb_Ret rt = { src, EINA_TRUE };
           eina_hash_foreach(tp->fields, (Eina_Hash_Foreach)_ef_map_cb, &rt);
           if (!rt.succ)
             return EINA_FALSE;
           return _validate(&tp->base);
        }
      case EOLIAN_TYPEDECL_FUNCTION_POINTER:
        if (!_validate_function(src, tp->function_pointer, NULL))
          return EINA_FALSE;
        return _validate(&tp->base);
      default:
        return EINA_FALSE;
     }
   return _validate(&tp->base);
}

static const char * const eo_complex_frees[] =
{
   "eina_accessor_free", "eina_array_free",
   "eina_iterator_free", "eina_hash_free",
   "eina_list_free", "eina_inarray_free", "eina_inlist_free",

   "efl_del" /* future */
};

static const char *eo_obj_free = "efl_del";
static const char *eo_str_free = "free";
static const char *eo_strshare_free = "eina_stringshare_del";
static const char *eo_value_free = "eina_value_flush";
static const char *eo_value_ptr_free = "eina_value_free";

static Eina_Bool
_validate_type(const Eolian_Unit *src, Eolian_Type *tp)
{
   char buf[256];
   if (tp->owned && !database_type_is_ownable(src, tp))
     {
        snprintf(buf, sizeof(buf), "type '%s' is not ownable", tp->full_name);
        return _obj_error(&tp->base, buf);
     }

   if (tp->is_ptr && !tp->legacy)
     {
        tp->is_ptr = EINA_FALSE;
        Eina_Bool still_ownable = database_type_is_ownable(src, tp);
        tp->is_ptr = EINA_TRUE;
        if (still_ownable)
          {
             return _obj_error(&tp->base, "cannot take a pointer to pointer type");
          }
     }

   switch (tp->type)
     {
      case EOLIAN_TYPE_VOID:
      case EOLIAN_TYPE_UNDEFINED:
        return _validate(&tp->base);
      case EOLIAN_TYPE_REGULAR:
        {
           if (tp->base_type)
             {
                int kwid = eo_lexer_keyword_str_to_id(tp->full_name);
                if (!tp->freefunc)
                  {
                     tp->freefunc = eina_stringshare_add(eo_complex_frees[
                       kwid - KW_accessor]);
                  }
                Eolian_Type *itp = tp->base_type;
                /* validate types in brackets so freefuncs get written... */
                while (itp)
                  {
                     if (!_validate_type(src, itp))
                       return EINA_FALSE;
                     if ((kwid >= KW_accessor) && (kwid <= KW_list))
                       {
                          if (!database_type_is_ownable(src, itp))
                            {
                               snprintf(buf, sizeof(buf),
                                        "%s cannot contain value types (%s)",
                                        tp->full_name, itp->full_name);
                               return _obj_error(&itp->base, buf);
                            }
                       }
                     itp = itp->next_type;
                  }
                return _validate(&tp->base);
             }
           /* builtins */
           int id = eo_lexer_keyword_str_to_id(tp->full_name);
           if (id)
             {
                if (!eo_lexer_is_type_keyword(id))
                  return EINA_FALSE;
                if (!tp->freefunc)
                  switch (id)
                    {
                     case KW_string:
                       tp->freefunc = eina_stringshare_add(eo_str_free);
                       break;
                     case KW_stringshare:
                       tp->freefunc = eina_stringshare_add(eo_strshare_free);
                       break;
                     case KW_any_value:
                       tp->freefunc = eina_stringshare_add(eo_value_free);
                       break;
                     case KW_any_value_ptr:
                       tp->freefunc = eina_stringshare_add(eo_value_ptr_free);
                       break;
                     default:
                       break;
                    }
                return _validate(&tp->base);
             }
           /* user defined */
           tp->tdecl = database_type_decl_find(src, tp);
           if (!tp->tdecl)
             {
                snprintf(buf, sizeof(buf), "undefined type %s", tp->full_name);
                return _obj_error(&tp->base, buf);
             }
           if (!_validate_typedecl(src, tp->tdecl))
             return EINA_FALSE;
           if (tp->tdecl->freefunc && !tp->freefunc)
             tp->freefunc = eina_stringshare_ref(tp->tdecl->freefunc);
           return _validate(&tp->base);
        }
      case EOLIAN_TYPE_CLASS:
        {
           tp->klass = (Eolian_Class *)eolian_class_get_by_name(src, tp->full_name);
           if (!tp->klass)
             {
                snprintf(buf, sizeof(buf), "undefined class %s "
                         "(likely wrong namespacing)", tp->full_name);
                return _obj_error(&tp->base, buf);
             }
           if (!tp->freefunc)
             tp->freefunc = eina_stringshare_add(eo_obj_free);
           return _validate(&tp->base);
        }
      default:
        return EINA_FALSE;
     }
   return _validate(&tp->base);
}

static Eina_Bool
_validate_expr(const Eolian_Unit *src, Eolian_Expression *expr,
               const Eolian_Type *tp, Eolian_Expression_Mask msk)
{
   Eolian_Value val;
   if (tp)
     val = database_expr_eval_type(src, expr, tp);
   else
     val = database_expr_eval(src, expr, msk);

   if (val.type == EOLIAN_EXPR_UNKNOWN)
     return EINA_FALSE;

   return _validate(&expr->base);
}

static Eina_Bool
_validate_param(const Eolian_Unit *src, Eolian_Function_Parameter *param)
{
   if (!_validate_type(src, param->type))
     return EINA_FALSE;

   if (param->value && !_validate_expr(src, param->value, param->type, 0))
     return EINA_FALSE;

   if (!_validate_doc(src, param->doc))
     return EINA_FALSE;

   return _validate(&param->base);
}

static Eina_Bool
_validate_function(const Eolian_Unit *src, Eolian_Function *func, Eina_Hash *nhash)
{
   Eina_List *l;
   Eolian_Function_Parameter *param;
   char buf[512];

   static int _duplicates_warn = -1;
   if (EINA_UNLIKELY(_duplicates_warn < 0))
     {
        const char *s = getenv("EOLIAN_WARN_FUNC_DUPLICATES");
        if (!s) _duplicates_warn = 0;
        else _duplicates_warn = atoi(s);
     }

   const Eolian_Function *ofunc = nhash ? eina_hash_find(nhash, func->name) : NULL;
   if (EINA_UNLIKELY(ofunc && (ofunc != func) && (_duplicates_warn > 0)))
     {
        snprintf(buf, sizeof(buf),
                 "%sfunction '%s' redefined (originally at %s:%d:%d)",
                 func->is_beta ? "beta " : "", func->name, ofunc->base.file,
                 ofunc->base.line, ofunc->base.column);
        if ((!func->is_beta && !ofunc->is_beta) || (_duplicates_warn > 1))
          _obj_error(&func->base, buf);
     }

   /* if already validated, no need to perform the other checks...
    * but duplicate checks need to be performed every time
    */
   if (func->base.validated)
     {
        /* it might be validated, but need to add it anyway */
        if (!ofunc && nhash)
          eina_hash_add(nhash, func->name, func);
        return EINA_TRUE;
     }

   if (func->get_ret_type && !_validate_type(src, func->get_ret_type))
     return EINA_FALSE;

   if (func->set_ret_type && !_validate_type(src, func->set_ret_type))
     return EINA_FALSE;

   if (func->get_ret_val && !_validate_expr(src, func->get_ret_val,
                                            func->get_ret_type, 0))
     return EINA_FALSE;

   if (func->set_ret_val && !_validate_expr(src, func->set_ret_val,
                                            func->set_ret_type, 0))
     return EINA_FALSE;

#define EOLIAN_PARAMS_VALIDATE(params) \
   EINA_LIST_FOREACH(params, l, param) \
     if (!_validate_param(src, param)) \
       return EINA_FALSE;

   EOLIAN_PARAMS_VALIDATE(func->prop_values);
   EOLIAN_PARAMS_VALIDATE(func->prop_values_get);
   EOLIAN_PARAMS_VALIDATE(func->prop_values_set);
   EOLIAN_PARAMS_VALIDATE(func->prop_keys);
   EOLIAN_PARAMS_VALIDATE(func->prop_keys_get);
   EOLIAN_PARAMS_VALIDATE(func->prop_keys_set);

#undef EOLIAN_PARAMS_VALIDATE

   if (!_validate_doc(src, func->get_return_doc))
     return EINA_FALSE;
   if (!_validate_doc(src, func->set_return_doc))
     return EINA_FALSE;

   /* just for now, when dups become errors there will be no need to check */
   if (!ofunc && nhash)
     eina_hash_add(nhash, func->name, func);

   return _validate(&func->base);
}

static Eina_Bool
_validate_part(const Eolian_Unit *src, Eolian_Part *part, Eina_Hash *nhash)
{
   const Eolian_Function *ofunc = eina_hash_find(nhash, part->name);
   if (ofunc)
     {
        char buf[512];
        snprintf(buf, sizeof(buf),
                 "part '%s' conflicts with a function (defined at %s:%d:%d)",
                 part->name, ofunc->base.file,
                 ofunc->base.line, ofunc->base.column);
        _obj_error(&part->base, buf);
     }

   /* see _validate_function above */
   if (part->base.validated)
     return EINA_TRUE;

   if (!_validate_doc(src, part->doc))
     return EINA_FALSE;

   return _validate(&part->base);
}

static Eina_Bool
_validate_event(const Eolian_Unit *src, Eolian_Event *event)
{
   if (event->base.validated)
     return EINA_TRUE;

   if (event->type && !_validate_type(src, event->type))
     return EINA_FALSE;

   if (!_validate_doc(src, event->doc))
     return EINA_FALSE;

   return _validate(&event->base);
}

static Eina_Bool
_validate_implement(const Eolian_Unit *src, Eolian_Implement *impl)
{
   if (impl->base.validated)
     return EINA_TRUE;

   if (!_validate_doc(src, impl->common_doc))
     return EINA_FALSE;
   if (!_validate_doc(src, impl->get_doc))
     return EINA_FALSE;
   if (!_validate_doc(src, impl->set_doc))
     return EINA_FALSE;

   return _validate(&impl->base);
}

static Eina_Bool
_validate_class(const Eolian_Unit *src, Eolian_Class *cl, Eina_Hash *nhash)
{
   Eina_List *l;
   Eolian_Function *func;
   Eolian_Event *event;
   Eolian_Part *part;
   Eolian_Implement *impl;
   Eolian_Class *icl;

   if (!cl)
     return EINA_FALSE; /* if this happens something is very wrong though */

   Eina_Bool valid = cl->base.validated;

   EINA_LIST_FOREACH(cl->inherits, l, icl)
     {
        /* first inherit needs some checking done on it */
        if (!valid && (l == cl->inherits)) switch (cl->type)
          {
           case EOLIAN_CLASS_REGULAR:
           case EOLIAN_CLASS_ABSTRACT:
             if (icl->type != EOLIAN_CLASS_REGULAR && icl->type != EOLIAN_CLASS_ABSTRACT)
               {
                  char buf[PATH_MAX];
                  snprintf(buf, sizeof(buf), "regular classes ('%s') cannot inherit from non-regular classes ('%s')",
                           cl->full_name, icl->full_name);
                  return _obj_error(&cl->base, buf);
               }
             break;
           case EOLIAN_CLASS_MIXIN:
           case EOLIAN_CLASS_INTERFACE:
             if (icl->type != EOLIAN_CLASS_MIXIN && icl->type != EOLIAN_CLASS_INTERFACE)
               {
                  char buf[PATH_MAX];
                  snprintf(buf, sizeof(buf), "non-regular classes ('%s') cannot inherit from regular classes ('%s')",
                           cl->full_name, icl->full_name);
                  return _obj_error(&cl->base, buf);
               }
             break;
           default:
             break;
          }
        if (!_validate_class(src, icl, nhash))
          return EINA_FALSE;
     }

   EINA_LIST_FOREACH(cl->properties, l, func)
     if (!_validate_function(src, func, nhash))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->methods, l, func)
     if (!_validate_function(src, func, nhash))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->events, l, event)
     if (!_validate_event(src, event))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->parts, l, part)
     if (!_validate_part(src, part, nhash))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->implements, l, impl)
     if (!_validate_implement(src, impl))
       return EINA_FALSE;

   /* all the checks that need to be done every time are performed now */
   if (valid)
     return EINA_TRUE;

   if (!_validate_doc(src, cl->doc))
     return EINA_FALSE;

   return _validate(&cl->base);
}

static Eina_Bool
_validate_variable(const Eolian_Unit *src, Eolian_Variable *var)
{
   if (var->base.validated)
     return EINA_TRUE;

   if (!_validate_type(src, var->base_type))
     return EINA_FALSE;

   if (var->value && !_validate_expr(src, var->value, var->base_type, 0))
     return EINA_FALSE;

   if (!_validate_doc(src, var->doc))
     return EINA_FALSE;

   return _validate(&var->base);
}

static Eina_Bool
_typedecl_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
                 Eolian_Typedecl *tp, Cb_Ret *sc)
{
   return (sc->succ = _validate_typedecl(sc->unit, tp));
}

static Eina_Bool
_var_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
             Eolian_Variable *var, Cb_Ret *sc)
{
   return (sc->succ = _validate_variable(sc->unit, var));
}

Eina_Bool
database_validate(Eolian *state, const Eolian_Unit *src)
{
   Eolian_Class *cl;

   Eina_Iterator *iter = eolian_all_classes_get(src);
   Eina_Hash *nhash = eina_hash_string_small_new(NULL);
   EINA_ITERATOR_FOREACH(iter, cl)
     {
        eina_hash_free_buckets(nhash);
        if (!_validate_class(src, cl, nhash))
          {
             eina_iterator_free(iter);
             eina_hash_free(nhash);
             return EINA_FALSE;
          }
     }
   eina_hash_free(nhash);
   eina_iterator_free(iter);

   Cb_Ret rt = { src, EINA_TRUE };

   eina_hash_foreach(state->unit.aliases, (Eina_Hash_Foreach)_typedecl_map_cb, &rt);
   if (!rt.succ)
     return EINA_FALSE;

   eina_hash_foreach(state->unit.structs, (Eina_Hash_Foreach)_typedecl_map_cb, &rt);
   if (!rt.succ)
     return EINA_FALSE;

   eina_hash_foreach(state->unit.enums, (Eina_Hash_Foreach)_typedecl_map_cb, &rt);
   if (!rt.succ)
     return EINA_FALSE;

   eina_hash_foreach(state->unit.globals, (Eina_Hash_Foreach)_var_map_cb, &rt);
   if (!rt.succ)
     return EINA_FALSE;

   eina_hash_foreach(state->unit.constants, (Eina_Hash_Foreach)_var_map_cb, &rt);
   if (!rt.succ)
     return EINA_FALSE;

   return EINA_TRUE;
}

#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eo_lexer.h"
#include "eolian_priv.h"

typedef struct _Validate_State
{
   Eina_Bool warned;
   Eina_Bool stable;
   Eina_Bool in_tree;
   Eina_Bool unimplemented_beta;
   Eina_Bool verify_since;
   const char *since_ver;
} Validate_State;

static Eina_Bool
_set_stable(Validate_State *vals, Eina_Bool newval)
{
   Eina_Bool ret = vals->stable;
   vals->stable = newval;
   return ret;
}

static Eina_Bool
_reset_stable(Validate_State *vals, Eina_Bool oldval, Eina_Bool ret)
{
   vals->stable = oldval;
   return ret;
}

static Eina_Bool
_validate(Eolian_Object *obj)
{
   obj->validated = EINA_TRUE;
   return EINA_TRUE;
}

#define _eo_parser_log(_base, ...) \
   eolian_state_log_obj((_base)->unit->state, (_base), __VA_ARGS__)

static Eina_Bool
_validate_docstr(Eina_Stringshare *str, const Eolian_Object *info, Eina_List **rdbg)
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
          {
             if (eolian_doc_token_type_get(&tok) == EOLIAN_DOC_TOKEN_REF)
               {
                  /* check staging first, then main */
                  Eolian_Object_Type tp = database_doc_token_ref_resolve(&tok,
                    &info->unit->state->staging.unit,
                    &info->unit->state->main.unit,
                    NULL, NULL);
                  if (tp == EOLIAN_OBJECT_UNKNOWN)
                    {
                       size_t dbgn = (size_t)eina_list_data_get(*rdbg);
                       char *refn = eolian_doc_token_text_get(&tok);
                       Eolian_Object tmp;
                       memset(&tmp, 0, sizeof(Eolian_Object));
                       tmp.unit = info->unit;
                       tmp.file = info->file;
                       tmp.line = (int)(dbgn & 0xFFFFF);
                       tmp.column = (int)(dbgn >> 20);
                       eolian_state_log_obj(info->unit->state, &tmp,
                                            "failed validating reference '%s'", refn);
                       free(refn);
                       ret = EINA_FALSE;
                       break;
                    }
                  *rdbg = eina_list_next(*rdbg);
               }
          }
        free(par);
     }

   return ret;
}

static Eina_Bool
_validate_doc_since(Validate_State *vals, Eolian_Documentation *doc)
{
   if (!doc || !vals->stable || !vals->verify_since)
     return EINA_TRUE;

   if (doc->since)
     {
        if (!doc->since[0])
          {
             /* this should not really happen */
             _eo_parser_log(&doc->base, "empty @since tag");
             return EINA_FALSE;
          }
        /* this is EFL; check the numbers */
        if (vals->in_tree)
          {
             const char *snum = doc->since;
             if (strncmp(snum, "1.", 2))
               {
                  _eo_parser_log(&doc->base, "invalid EFL version in @since");
                  return EINA_FALSE;
               }
             snum += 2;
             unsigned long min = strtoul(snum, NULL, 10);
             if (min < 22)
               {
                  _eo_parser_log(&doc->base, "stable APIs must be 1.22 or higher");
                  return EINA_FALSE;
               }
          }
        vals->since_ver = doc->since;
     }
   else if (!vals->since_ver)
     {
        _eo_parser_log(&doc->base, "missing @since tag");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_validate_doc_since_reset(Validate_State *vals, Eolian_Documentation *doc)
{
   if (!doc || !doc->since)
     return EINA_TRUE;

   const char *old_since = vals->since_ver;
   Eina_Bool ret = _validate_doc_since(vals, doc);
   vals->since_ver = old_since;
   return ret;
}

static Eina_Bool
_validate_doc(Eolian_Documentation *doc)
{
   if (!doc)
     return EINA_TRUE;

   Eina_List *rdbg = doc->ref_dbg;

   if (!_validate_docstr(doc->summary, &doc->base, &rdbg))
     return EINA_FALSE;
   if (!_validate_docstr(doc->description, &doc->base, &rdbg))
     return EINA_FALSE;

   return _validate(&doc->base);
}

static Eina_Bool _validate_type(Validate_State *vals, Eolian_Type *tp,
                                Eina_Bool by_ref, Eina_Bool is_ret);
static Eina_Bool _validate_type_by_ref(Validate_State *vals, Eolian_Type *tp,
                                       Eina_Bool by_ref, Eina_Bool move,
                                       Eina_Bool is_ret);
static Eina_Bool _validate_expr(Eolian_Expression *expr,
                                const Eolian_Type *tp,
                                Eolian_Expression_Mask msk,
                                Eina_Bool by_ref);
static Eina_Bool _validate_function(Validate_State *vals,
                                    Eolian_Function *func,
                                    Eina_Hash *nhash);

typedef struct _Cb_Ret
{
   Validate_State *vals;
   Eina_Bool succ;
} Cb_Ret;

static Eina_Bool
_sf_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
           const Eolian_Struct_Type_Field *sf, Cb_Ret *sc)
{
   sc->succ = _validate_type_by_ref(sc->vals, sf->type, sf->by_ref,
                                    sf->move, EINA_FALSE);

   if (!sc->succ)
     return EINA_FALSE;

   sc->succ = _validate_doc(sf->doc);
   if (sc->succ) sc->succ = _validate_doc_since_reset(sc->vals, sf->doc);

   return sc->succ;
}

static Eina_Bool
_ef_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
           const Eolian_Enum_Type_Field *ef, Cb_Ret *sc)
{
   if (ef->value)
     sc->succ = _validate_expr(ef->value, NULL, EOLIAN_MASK_INT, EINA_FALSE);
   else
     sc->succ = EINA_TRUE;

   if (!sc->succ)
     return EINA_FALSE;

   sc->succ = _validate_doc(ef->doc);
   if (sc->succ) sc->succ = _validate_doc_since_reset(sc->vals, ef->doc);

   return sc->succ;
}

static Eina_Bool
_validate_typedecl(Validate_State *vals, Eolian_Typedecl *tp)
{
   if (tp->base.validated)
     return EINA_TRUE;

   const char *old_since = vals->since_ver;
   vals->since_ver = NULL;

   /* for the time being assume all typedecls are beta unless overridden */
   Eina_Bool was_stable = _set_stable(vals, !tp->base.is_beta);

   if (!_validate_doc(tp->doc))
     return EINA_FALSE;

   if (!_validate_doc_since(vals, tp->doc))
     return EINA_FALSE;

   switch (tp->type)
     {
      case EOLIAN_TYPEDECL_ALIAS:
        if (!_validate_type(vals, tp->base_type, EINA_FALSE, EINA_FALSE))
          return _reset_stable(vals, was_stable, EINA_FALSE);
        if (tp->base_type->ownable)
          tp->ownable = EINA_TRUE;
        _reset_stable(vals, was_stable, EINA_TRUE);
        vals->since_ver = old_since;
        return _validate(&tp->base);
      case EOLIAN_TYPEDECL_STRUCT:
        {
           Cb_Ret rt = { vals, EINA_TRUE };
           eina_hash_foreach(tp->fields, (Eina_Hash_Foreach)_sf_map_cb, &rt);
           if (!rt.succ)
             return _reset_stable(vals, was_stable, EINA_FALSE);
           _reset_stable(vals, was_stable, EINA_TRUE);
           vals->since_ver = old_since;
           return _validate(&tp->base);
        }
      case EOLIAN_TYPEDECL_STRUCT_OPAQUE:
        _reset_stable(vals, was_stable, EINA_TRUE);
        vals->since_ver = old_since;
        return _validate(&tp->base);
      case EOLIAN_TYPEDECL_ENUM:
        {
           if (vals->stable && tp->legacy)
             {
                _eo_parser_log(&tp->base, "legacy field not allowed in stable enums");
                return EINA_FALSE;
             }
           Cb_Ret rt = { vals, EINA_TRUE };
           eina_hash_foreach(tp->fields, (Eina_Hash_Foreach)_ef_map_cb, &rt);
           if (!rt.succ)
             return _reset_stable(vals, was_stable, EINA_FALSE);
           _reset_stable(vals, was_stable, EINA_TRUE);
           vals->since_ver = old_since;
           return _validate(&tp->base);
        }
      case EOLIAN_TYPEDECL_FUNCTION_POINTER:
        if (!_validate_function(vals, tp->function_pointer, NULL))
          return _reset_stable(vals, was_stable, EINA_FALSE);
        _reset_stable(vals, was_stable, EINA_TRUE);
        vals->since_ver = old_since;
        return _validate(&tp->base);
      default:
        return _reset_stable(vals, was_stable, EINA_FALSE);
     }

   //Unexpected case?
   return EINA_FALSE;
}

static Eina_Bool
_validate_by_ref(Eolian_Type *tp, Eina_Bool by_ref, Eina_Bool move)
{
   Eina_Bool maybe_ownable =
     database_type_is_ownable(tp->base.unit, tp, EINA_FALSE, NULL);

   /* only allow value types when @by_ref */
   if (by_ref && maybe_ownable)
     {
        _eo_parser_log(&tp->base, "@by_ref is only allowed for value types");
        return EINA_FALSE;
     }

   /* futures can be whatever... */
   if (tp->btype == EOLIAN_TYPE_BUILTIN_FUTURE)
     return EINA_TRUE;

   /* not marked @move, or marked @by_ref; just validate */
   if (!move || by_ref)
      return EINA_TRUE;

   /* marked @move, not pointer-like or otherwise ownable, error */
   if (!maybe_ownable || !tp->ownable)
     {
        _eo_parser_log(&tp->base, "type '%s' is not ownable", tp->base.name);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_validate_type_by_ref(Validate_State *vals, Eolian_Type *tp,
                      Eina_Bool by_ref, Eina_Bool move, Eina_Bool is_ret)
{
   if (!_validate_type(vals, tp, by_ref, is_ret))
     return EINA_FALSE;

   return _validate_by_ref(tp, by_ref, move);
}

static Eina_Bool
_validate_type(Validate_State *vals, Eolian_Type *tp, Eina_Bool by_ref,
               Eina_Bool is_ret)
{
   const Eolian_Unit *src = tp->base.unit;

   if (tp->is_ptr)
     {
        if (vals->stable)
          {
             _eo_parser_log(&tp->base, "ptr() used in stable API");
             return EINA_FALSE;
          }
        tp->is_ptr = EINA_FALSE;
        Eina_Bool still_ownable = database_type_is_ownable(src, tp, EINA_FALSE, NULL);
        tp->is_ptr = EINA_TRUE;
        if (still_ownable)
          {
             _eo_parser_log(&tp->base, "cannot take a pointer to pointer type");
             return EINA_FALSE;
          }
     }

   if (tp->is_const && !tp->is_ptr)
     {
        if (database_type_is_ownable(src, tp, EINA_FALSE, NULL))
          {
             int kw = eo_lexer_keyword_str_to_id(tp->base.name);
             switch (kw)
               {
                case KW_string:
                case KW_mstring:
                case KW_stringshare:
                  _eo_parser_log(&tp->base, "spurious explicit const on string type");
                  return EINA_FALSE;
                default:
                  {
                     if (!is_ret)
                       break;
                     const char *ct = eo_lexer_get_c_type(kw);
                     if (ct && ct[strlen(ct) - 1] != '*')
                       {
                          _eo_parser_log(&tp->base, "return const requires a C pointer");
                          return EINA_FALSE;
                       }
                  }
               }
            }
        else if (!by_ref && is_ret)
          {
             _eo_parser_log(&tp->base, "value returns cannot be const");
             return EINA_FALSE;
          }
     }

   switch (tp->type)
     {
      case EOLIAN_TYPE_VOID:
        return _validate(&tp->base);
      case EOLIAN_TYPE_UNDEFINED:
        if (vals->stable)
          {
             _eo_parser_log(&tp->base,
               "__undefined_type not allowed in stable context");
             return EINA_FALSE;
          }
        return _validate(&tp->base);
      case EOLIAN_TYPE_REGULAR:
        {
           if (tp->base_type)
             {
                int kwid = eo_lexer_keyword_str_to_id(tp->base.name);
                if (kwid > KW_void)
                  tp->ownable = EINA_TRUE;
                if (kwid == KW_hash && vals->stable)
                  {
                     _eo_parser_log(&tp->base, "hashes not allowed in stable context");
                     return EINA_FALSE;
                  }
                Eolian_Type *itp = tp->base_type;
                /* validate types in brackets so transitive fields get written */
                while (itp)
                  {
                     if (vals->stable && itp->is_ptr)
                       {
                          _eo_parser_log(&itp->base,
                                         "pointer types not allowed in '%s' in stable context",
                                         tp->base.name);
                          return EINA_FALSE;
                       }
                     if (!_validate_type_by_ref(vals, itp, EINA_FALSE, itp->move, EINA_FALSE))
                       return EINA_FALSE;
                     itp = itp->next_type;
                  }
                return _validate(&tp->base);
             }
           /* builtins */
           int id = eo_lexer_keyword_str_to_id(tp->base.name);
           if (id)
             {
                if (!eo_lexer_is_type_keyword(id))
                  return EINA_FALSE;
                switch (id)
                  {
                   case KW_mstring:
                   case KW_stringshare:
                   case KW_any_value:
                   case KW_any_value_ref:
                   case KW_binbuf:
                   case KW_strbuf:
                     tp->ownable = EINA_TRUE;
                     break;
                   default:
                     break;
                  }
                if (id == KW_void_ptr && vals->stable)
                  {
                     _eo_parser_log(&tp->base,
                       "void pointers not allowed in stable context");
                     return EINA_FALSE;
                  }
                return _validate(&tp->base);
             }
           /* user defined */
           tp->tdecl = database_type_decl_find(src, tp);
           if (!tp->tdecl)
             {
                _eo_parser_log(&tp->base, "undefined type %s", tp->base.name);
                return EINA_FALSE;
             }
           else if (vals->stable && tp->tdecl->base.is_beta)
             {
                /* we should enable this by default, but can't for now */
                _eo_parser_log(&tp->base, "beta type declaration '%s' used in stable context",
                               tp->tdecl->base.name);
                return EINA_FALSE;
             }
           if (!_validate_typedecl(vals, tp->tdecl))
             return EINA_FALSE;
           if (tp->tdecl->ownable)
             tp->ownable = EINA_TRUE;
           tp->base.c_name = eina_stringshare_ref(tp->tdecl->base.c_name);
           return _validate(&tp->base);
        }
      case EOLIAN_TYPE_CLASS:
        {
           tp->klass = (Eolian_Class *)eolian_unit_class_by_name_get(src, tp->base.name);
           if (!tp->klass)
             {
                _eo_parser_log(&tp->base, "undefined class %s "
                         "(likely wrong namespacing)", tp->base.name);
                return EINA_FALSE;
             }
           else if (vals->stable && tp->klass->base.is_beta)
             {
                _eo_parser_log(&tp->base, "beta class '%s' used in stable context",
                               tp->klass->base.name);
                return EINA_FALSE;
             }
           tp->ownable = EINA_TRUE;
           tp->base.c_name = eina_stringshare_ref(tp->klass->base.c_name);
           return _validate(&tp->base);
        }
      case EOLIAN_TYPE_ERROR:
        {
           tp->error = (Eolian_Error *)eolian_unit_error_by_name_get(src, tp->base.name);
           if (!tp->error)
             {
                _eo_parser_log(&tp->base, "undefined error %s "
                         "(likely wrong namespacing)", tp->base.name);
                return EINA_FALSE;
             }
           else if (vals->stable && tp->error->base.is_beta)
             {
                _eo_parser_log(&tp->base, "beta error '%s' used in stable context",
                               tp->error->base.name);
                return EINA_FALSE;
             }
           tp->base.c_name = eina_stringshare_ref(tp->error->base.c_name);
           if (tp->next_type && !_validate_type(vals, tp->next_type, EINA_FALSE, EINA_FALSE))
             return EINA_FALSE;
           return _validate(&tp->base);
        }
      default:
        break;
     }
   return EINA_FALSE;
}

static Eina_Bool
_validate_expr(Eolian_Expression *expr, const Eolian_Type *tp,
               Eolian_Expression_Mask msk, Eina_Bool by_ref)
{
   Eolian_Value val;
   if (by_ref)
     val = database_expr_eval(expr->base.unit, expr, EOLIAN_MASK_NULL, NULL, NULL);
   else if (tp)
     val = database_expr_eval_type(expr->base.unit, expr, tp, NULL, NULL);
   else
     val = database_expr_eval(expr->base.unit, expr, msk, NULL, NULL);

   if (val.type == EOLIAN_EXPR_UNKNOWN)
     return EINA_FALSE;

   return _validate(&expr->base);
}

static Eina_Bool
_validate_param(Validate_State *vals, Eolian_Function_Parameter *param)
{
   if (!_validate_type_by_ref(vals, param->type, param->by_ref, param->move, EINA_FALSE))
     return EINA_FALSE;

   if (param->value && !_validate_expr(param->value, param->type, 0, param->by_ref))
     return EINA_FALSE;

   if (!_validate_doc(param->doc))
     return EINA_FALSE;

   return _validate(&param->base);
}

static Eina_Bool
_validate_function(Validate_State *vals, Eolian_Function *func, Eina_Hash *nhash)
{
   Eina_List *l;
   Eolian_Function_Parameter *param;

   const Eolian_Object *oobj = nhash ? eina_hash_find(nhash, &func->base.name) : NULL;
   if (EINA_UNLIKELY(oobj && (oobj != &func->base)))
     {
        _eo_parser_log(&func->base,
                 "%sfunction '%s' conflicts with another symbol (at %s:%d:%d)",
                 func->base.is_beta ? "beta " : "", func->base.name, oobj->file,
                 oobj->line, oobj->column);
        vals->warned = EINA_TRUE;
     }

   /* if already validated, no need to perform the other checks...
    * but duplicate checks need to be performed every time
    */
   if (func->base.validated)
     {
        /* it might be validated, but need to add it anyway */
        if (!oobj && nhash)
          eina_hash_add(nhash, &func->base.name, &func->base);
        return EINA_TRUE;
     }

   /* need to preserve stable flag set from the class */
   Eina_Bool was_stable = _set_stable(vals, !func->base.is_beta && vals->stable);

   if (func->get_ret_type && !_validate_type_by_ref(vals, func->get_ret_type,
       func->get_return_by_ref, func->get_return_move, EINA_TRUE))
     return _reset_stable(vals, was_stable, EINA_FALSE);

   if (func->set_ret_type && !_validate_type_by_ref(vals, func->set_ret_type,
       func->set_return_by_ref, func->set_return_move, EINA_TRUE))
     return _reset_stable(vals, was_stable, EINA_FALSE);

   if (func->get_ret_val && !_validate_expr(func->get_ret_val,
                                            func->get_ret_type, 0,
                                            func->get_return_by_ref))
     return _reset_stable(vals, was_stable, EINA_FALSE);

   if (func->set_ret_val && !_validate_expr(func->set_ret_val,
                                            func->set_ret_type, 0,
                                            func->set_return_by_ref))
     return _reset_stable(vals, was_stable, EINA_FALSE);

#define EOLIAN_PARAMS_VALIDATE(params) \
   EINA_LIST_FOREACH(params, l, param) \
     if (!_validate_param(vals, param)) \
       return _reset_stable(vals, was_stable, EINA_FALSE);

   EOLIAN_PARAMS_VALIDATE(func->prop_values);
   EOLIAN_PARAMS_VALIDATE(func->prop_values_get);
   EOLIAN_PARAMS_VALIDATE(func->prop_values_set);
   EOLIAN_PARAMS_VALIDATE(func->prop_keys);
   EOLIAN_PARAMS_VALIDATE(func->prop_keys_get);
   EOLIAN_PARAMS_VALIDATE(func->prop_keys_set);

#undef EOLIAN_PARAMS_VALIDATE

   if (!_validate_doc(func->get_return_doc))
     return _reset_stable(vals, was_stable, EINA_FALSE);
   if (!_validate_doc(func->set_return_doc))
     return _reset_stable(vals, was_stable, EINA_FALSE);

   /* just for now, when dups become errors there will be no need to check */
   if (!oobj && nhash)
     eina_hash_add(nhash, &func->base.name, &func->base);

   _reset_stable(vals, was_stable, EINA_TRUE);
   return _validate(&func->base);
}

static Eina_Bool
_validate_part(Validate_State *vals, Eolian_Part *part, Eina_Hash *phash)
{
   const Eolian_Object *oobj = eina_hash_find(phash, &part->base.name);
   if (oobj)
     {
        _eo_parser_log(&part->base,
                 "part '%s' conflicts with another part (at %s:%d:%d)",
                 part->base.name, oobj->file, oobj->line, oobj->column);
        vals->warned = EINA_TRUE;
     }

   /* see _validate_function above */
   if (part->base.validated)
     {
        if (!oobj)
          eina_hash_add(phash, &part->base.name, &part->base);
        return EINA_TRUE;
     }

   Eina_Bool was_stable = _set_stable(vals, !part->base.is_beta && vals->stable);

   if (!_validate_doc(part->doc))
     return _reset_stable(vals, was_stable, EINA_FALSE);
   if (!_validate_doc_since_reset(vals, part->doc))
     return _reset_stable(vals, was_stable, EINA_FALSE);

   /* switch the class name for class */
   Eolian_Class *pcl = eina_hash_find(part->base.unit->classes, part->klass_name);
   if (!pcl)
     {
        _eo_parser_log(&part->base, "unknown part class '%s' (incorrect case?)",
                 part->klass_name);
        return _reset_stable(vals, was_stable, EINA_FALSE);
     }
   else if (vals->stable && pcl->base.is_beta)
     {
        _eo_parser_log(&part->base, "beta part class '%s' used in stable context",
                       pcl->base.name);
        return _reset_stable(vals, was_stable, EINA_FALSE);
     }
   eina_stringshare_del(part->klass_name);
   part->klass = pcl;

   if (!oobj)
     eina_hash_add(phash, &part->base.name, &part->base);

   _reset_stable(vals, was_stable, EINA_TRUE);
   return _validate(&part->base);
}

static Eina_Bool
_validate_event(Validate_State *vals, Eolian_Event *event, Eina_Hash *nhash)
{
   const Eolian_Object *oobj = NULL;

   oobj = eina_hash_find(nhash, &event->base.name);
   if (EINA_UNLIKELY(!!oobj))
     {
        _eo_parser_log(&event->base,
                 "event '%s' conflicts with another event (at %s:%d:%d)",
                 event->base.name, oobj->file, oobj->line, oobj->column);
        vals->warned = EINA_TRUE;
     }

   if (event->base.validated)
     {
        eina_hash_set(nhash, &event->base.name, &event->base);
        return EINA_TRUE;
     }

   Eina_Bool was_stable = _set_stable(vals, !event->base.is_beta && vals->stable);

   if (!_validate_type(vals, event->type, EINA_FALSE, EINA_FALSE))
     return _reset_stable(vals, was_stable, EINA_FALSE);

   /* if this is an alias we need the lowest type in the stack, this is
    * after validation so all the necessary fields are properly filled in
    */
   const Eolian_Type *tp = eolian_type_aliased_base_get(event->type);

   /* event types are specially restricted
    * but stuff like pointer-to-pointer is alrady handled by _validate_type
    */
   if (tp->type == EOLIAN_TYPE_REGULAR && vals->stable)
     {
        /* explicit pointers never allowed */
        if (tp->is_ptr)
          {
             _eo_parser_log(&tp->base, "pointers not allowed in events");
             return _reset_stable(vals, was_stable, EINA_FALSE);
          }
        int kwid = eo_lexer_keyword_str_to_id(tp->base.name);
        /* require containers to be const for now...
         *
         * this is FIXME, and decision wasn't reached before 1.22
         * it is a simple search-replace anyway
         */
        if (database_type_is_ownable(tp->base.unit, tp, EINA_FALSE, NULL))
          {
             switch (kwid)
               {
                case KW_string:
                case KW_stringshare:
                  break;
                default:
                  if (!tp->is_const)
                    {
                       _eo_parser_log(&tp->base, "event container types must be const");
                       return _reset_stable(vals, was_stable, EINA_FALSE);
                    }
               }
          }
        else if (tp->is_const)
          {
             _eo_parser_log(&tp->base, "event value types cannot be const");
             return _reset_stable(vals, was_stable, EINA_FALSE);
          }
        /* containers are allowed but not iterators/lists */
        if (kwid == KW_iterator || kwid == KW_list)
          {
             _eo_parser_log(&tp->base, "sequence containers not allowed in events");
             return _reset_stable(vals, was_stable, EINA_FALSE);
          }
        /* rw slices are not allowed as regular types are always immutable */
        if (kwid == KW_rw_slice)
          {
             _eo_parser_log(&tp->base, "mutable slices not allowed in events");
             return _reset_stable(vals, was_stable, EINA_FALSE);
          }
        /* any type past builtin value types and containers is not allowed,
         * any_value is allowed but passed as const reference, any_value_ref
         * is not; string and stringshare is allowed, but mutable strings are
         * not and neither are string buffers, the type is never owned by the
         * callee, so all strings passed in are unowned and read-only
         */
        if (kwid >= KW_any_value_ref && kwid != KW_string && kwid != KW_stringshare)
          {
             _eo_parser_log(&tp->base, "forbidden event type");
             return _reset_stable(vals, was_stable, EINA_FALSE);
          }
     }

   if (!_validate_doc(event->doc))
     return _reset_stable(vals, was_stable, EINA_FALSE);
   if (!_validate_doc_since_reset(vals, event->doc))
     return _reset_stable(vals, was_stable, EINA_FALSE);

   eina_hash_set(nhash, &event->base.name, &event->base);

   _reset_stable(vals, was_stable, EINA_TRUE);
   return _validate(&event->base);
}

const Eolian_Class *
_get_impl_class(const Eolian_Class *cl, const char *cln)
{
   if (!cl || !strcmp(cl->base.name, cln))
     return cl;
   Eina_List *l;
   Eolian_Class *icl = cl->parent;
   if (icl)
     {
        /* we can do a depth first search, it's easier and doesn't matter
         * which part of the inheritance tree we find the class in
         */
        const Eolian_Class *fcl = _get_impl_class(icl, cln);
        if (fcl)
          return fcl;
     }
   EINA_LIST_FOREACH(cl->requires, l, icl)
     {
        const Eolian_Class *fcl = _get_impl_class(icl, cln);
        if (fcl)
          return fcl;
     }
   EINA_LIST_FOREACH(cl->extends, l, icl)
     {
        const Eolian_Class *fcl = _get_impl_class(icl, cln);
        if (fcl)
          return fcl;
     }
   return NULL;
}

static Eina_Bool
_db_fill_implement(Eolian_Class *cl, Eolian_Implement *impl)
{
   Eolian_Function_Type ftype = EOLIAN_METHOD;

   if (impl->is_prop_get && impl->is_prop_set)
     ftype = EOLIAN_PROPERTY;
   else if (impl->is_prop_get)
     ftype = EOLIAN_PROP_GET;
   else if (impl->is_prop_set)
     ftype = EOLIAN_PROP_SET;

   size_t imlen = strlen(impl->base.name);
   char *clbuf = alloca(imlen + 1);
   memcpy(clbuf, impl->base.name, imlen + 1);

   char *ldot = strrchr(clbuf, '.');
   if (!ldot)
     return EINA_FALSE; /* unreachable in practice, for static analysis */

   *ldot = '\0'; /* split between class name and func name */
   const char *clname = clbuf;
   const char *fnname = ldot + 1;

   const Eolian_Class *tcl = _get_impl_class(cl, clname);
   if (!tcl)
     {
        _eo_parser_log(&impl->base, "class '%s' not found within the inheritance tree of '%s'",
                       clname, cl->base.name);
        return EINA_FALSE;
     }

   impl->klass = tcl;
   impl->implklass = cl;

   const Eolian_Function *fid = eolian_class_function_by_name_get(tcl, fnname, EOLIAN_UNRESOLVED);
   if (!fid)
     {
        _eo_parser_log(&impl->base, "function '%s' not known in class '%s'", fnname, clname);
        return EINA_FALSE;
     }

   Eolian_Function_Type aftype = eolian_function_type_get(fid);

   Eina_Bool auto_empty = (impl->get_auto || impl->get_empty);

   /* match implement type against function type */
   if (ftype == EOLIAN_PROPERTY)
     {
        /* property */
        if (aftype != EOLIAN_PROPERTY)
          {
             _eo_parser_log(&impl->base, "function '%s' is not a complete property", fnname);
             return EINA_FALSE;
          }
        auto_empty = auto_empty && (impl->set_auto || impl->set_empty);
     }
   else if (ftype == EOLIAN_PROP_SET)
     {
        /* setter */
        if ((aftype != EOLIAN_PROP_SET) && (aftype != EOLIAN_PROPERTY))
          {
             _eo_parser_log(&impl->base, "function '%s' doesn't have a setter", fnname);
             return EINA_FALSE;
          }
        auto_empty = (impl->set_auto || impl->set_empty);
     }
   else if (ftype == EOLIAN_PROP_GET)
     {
        /* getter */
        if ((aftype != EOLIAN_PROP_GET) && (aftype != EOLIAN_PROPERTY))
          {
             _eo_parser_log(&impl->base, "function '%s' doesn't have a getter", fnname);
             return EINA_FALSE;
          }
     }
   else if (aftype != EOLIAN_METHOD)
     {
        _eo_parser_log(&impl->base, "function '%s' is not a method", fnname);
        return EINA_FALSE;
     }

   if ((fid->klass == cl) && !auto_empty)
     {
        /* only allow explicit implements from other classes, besides auto and
         * empty... also prevents pure virtuals from being implemented
         */
        _eo_parser_log(&impl->base, "invalid implement '%s'", impl->base.name);
        return EINA_FALSE;
     }

   impl->foo_id = fid;

   return EINA_TRUE;
}

typedef enum
{
   IMPL_STATUS_NONE = 1,
   IMPL_STATUS_FULL,
   IMPL_STATUS_GET,
   IMPL_STATUS_SET
} Impl_Status;

static Eina_Bool
_extend_impl(Eina_Hash *fs, Eolian_Implement *impl, Eina_Bool as_iface)
{
   const Eolian_Function *fid = impl->foo_id;
   Impl_Status st = (Impl_Status)eina_hash_find(fs, &fid);
   if (st == IMPL_STATUS_FULL)
     return EINA_FALSE;
   if (!st)
     eina_hash_set(fs, &fid, (void *)IMPL_STATUS_NONE);
   if (as_iface || (impl->implklass->type == EOLIAN_CLASS_INTERFACE))
     return !st;
   /* impl covers entire declaration */
   if (fid->type == EOLIAN_METHOD ||
       ((st == IMPL_STATUS_GET || fid->type == EOLIAN_PROP_SET) && impl->is_prop_set) ||
       ((st == IMPL_STATUS_SET || fid->type == EOLIAN_PROP_GET) && impl->is_prop_get) ||
       (impl->is_prop_get && impl->is_prop_set))
     {
        /* different implementing class can only do a real implementation */
        if (impl->implklass != impl->klass)
          {
             eina_hash_set(fs, &fid, (void *)IMPL_STATUS_FULL);
             return (st != IMPL_STATUS_FULL);
          }
        /* entirely virtual, so bail out always */
        if (impl->get_pure_virtual && impl->set_pure_virtual)
          return !st;
        if (impl->get_pure_virtual)
          {
             if (fid->type == EOLIAN_METHOD || fid->type == EOLIAN_PROP_GET)
               return !st;
             if (st == IMPL_STATUS_GET)
               {
                  eina_hash_set(fs, &fid, (void *)IMPL_STATUS_FULL);
                  return (st != IMPL_STATUS_FULL);
               }
             else
               {
                  eina_hash_set(fs, &fid, (void *)IMPL_STATUS_SET);
                  return (st <= IMPL_STATUS_NONE);
               }
          }
        if (impl->set_pure_virtual)
          {
             if (fid->type == EOLIAN_PROP_SET)
               return !st;
             if (st == IMPL_STATUS_SET)
               {
                  eina_hash_set(fs, &fid, (void *)IMPL_STATUS_FULL);
                  return (st < IMPL_STATUS_FULL);
               }
             else
               {
                  eina_hash_set(fs, &fid, (void *)IMPL_STATUS_GET);
                  return (st <= IMPL_STATUS_NONE);
               }
          }
        eina_hash_set(fs, &fid, (void *)IMPL_STATUS_FULL);
        return (st != IMPL_STATUS_FULL);
     }
   if (impl->implklass != impl->klass ||
       (!impl->get_pure_virtual && !impl->set_pure_virtual))
     {
        if (impl->is_prop_get)
          {
             eina_hash_set(fs, &fid, (void *)IMPL_STATUS_GET);
             return (st <= IMPL_STATUS_NONE);
          }
        else if (impl->is_prop_set)
          {
             eina_hash_set(fs, &fid, (void *)IMPL_STATUS_SET);
             return (st <= IMPL_STATUS_NONE);
          }
     }
   return !st;
}

/* fills a complete set of stuff implemented or inherited on a class
 * this is used to check whether to auto-add composited interfaces into
 * implemented/extended list
 */
static void
_db_fill_ihash(Eolian_Class *icl, Eina_Hash *icls)
{
   if (icl->parent)
     _db_fill_ihash(icl->parent, icls);
   Eina_List *l;
   Eolian_Class *sicl;
   EINA_LIST_FOREACH(icl->extends, l, sicl)
     _db_fill_ihash(sicl, icls);
   eina_hash_set(icls, &icl, icl);
}

static void
_db_fill_callables(Eolian_Class *cl, Eolian_Class *icl, Eina_Hash *fs, Eina_Bool parent)
{
   Eina_List *l;
   Eolian_Implement *impl;
   Eina_Bool allow_impl = parent || (icl->type == EOLIAN_CLASS_MIXIN);
   EINA_LIST_FOREACH(icl->callables, l, impl)
     {
        Impl_Status ost = (Impl_Status)eina_hash_find(fs, &impl->foo_id);
        /* while regular classes are already fully checked and one may
         * assume that we could just make everything coming from regular
         * classes IMPL_STATUS_FULL, we still need to account for all of
         * the callables of the regular class, as the full implementation
         * may come from somewhere deeper in the inheritance tree and we
         * may not reach it first, so follow the same logic for all
         */
        if (_extend_impl(fs, impl, !allow_impl))
          {
             /* we had an unimplementation in the list, replace
              * instead of appending the new thing to callables
              * this is a corner case, it shouldn't happen much
              */
             if (ost == IMPL_STATUS_NONE)
               {
                  Eina_List *ll;
                  Eolian_Implement *old;
                  EINA_LIST_FOREACH(cl->callables, ll, old)
                    {
                       if (old->foo_id == impl->foo_id)
                         eina_list_data_set(ll, impl);
                    }
               }
             else
               cl->callables = eina_list_append(cl->callables, impl);
          }
     }
}

static Eina_Bool
_db_check_implemented(Validate_State *vals, Eolian_Class *cl, Eina_Hash *fs,
                      Eina_Hash *cs, Eina_Hash *errh)
{
   if (cl->type != EOLIAN_CLASS_REGULAR)
     return EINA_TRUE;

   Eina_Bool succ = EINA_TRUE;

   /* class is beta and we didn't enable unimplemented checking for those */
   if (!vals->unimplemented_beta && cl->base.is_beta)
     return EINA_TRUE;

   Eina_List *l;
   Eolian_Implement *impl;
   EINA_LIST_FOREACH(cl->callables, l, impl)
     {
        const Eolian_Function *fid = impl->foo_id;
        /* not checking beta and the function is beta: skip */
        if (!vals->unimplemented_beta && fid->base.is_beta)
          continue;
        /* not checking beta and the function's class is beta: skip */
        if (!vals->unimplemented_beta && fid->klass->base.is_beta)
          continue;
        Impl_Status st = (Impl_Status)eina_hash_find(fs, &fid);
        /* found an interface this func was originally defined in in the
         * composite list; in that case, ignore it and assume it will come
         * from a composite object later
         */
        if (eina_hash_find(cs, &fid->klass))
          continue;
        /* the error on this impl has already happened, which means it came
         * from another regular class; reduce verbosity by not repeating it
         */
        if (eina_hash_find(errh, &impl))
          continue;
        switch (st)
          {
           case IMPL_STATUS_NONE:
             _eo_parser_log(
               &cl->base, "unimplemented function '%s' (originally defined at %s:%d:%d)",
               fid->base.name, fid->base.file, fid->base.line, fid->base.column);
             succ = EINA_FALSE;
             eina_hash_set(errh, &impl, impl);
             continue;
           case IMPL_STATUS_GET:
           case IMPL_STATUS_SET:
             _eo_parser_log(
               &cl->base, "partially implemented function '%s' (originally defined at %s:%d:%d)",
               fid->base.name, fid->base.file, fid->base.line, fid->base.column);
             succ = EINA_FALSE;
             eina_hash_set(errh, &impl, impl);
             continue;
           case IMPL_STATUS_FULL:
             continue;
           default:
             _eo_parser_log(
               &cl->base, "internal error, unregistered function '%s' (originally defined at %s:%d:%d)",
               fid->base.name, fid->base.file, fid->base.line, fid->base.column);
             return EINA_FALSE;
          }
     }
   return succ;
}

static Eina_Bool
_db_fill_implements(Eolian_Class *cl, Eina_Hash *fs)
{
   Eolian_Implement *impl;
   Eina_List *l;

   Eina_Bool ret = EINA_TRUE;

   Eina_Hash *th = eina_hash_string_small_new(NULL),
             *pth = eina_hash_string_small_new(NULL);
   EINA_LIST_FOREACH(cl->implements, l, impl)
     {
        Eina_Bool prop = (impl->is_prop_get || impl->is_prop_set);
        if (eina_hash_find(prop ? pth : th, impl->base.name))
          {
             _eo_parser_log(&impl->base, "duplicate implement '%s'", impl->base.name);
             ret = EINA_FALSE;
             goto end;
          }
        if (impl->klass != cl)
          {
             if (!_db_fill_implement(cl, impl))
               {
                  ret = EINA_FALSE;
                  goto end;
               }
             if (eolian_function_is_constructor(impl->foo_id, impl->klass))
               database_function_constructor_add((Eolian_Function *)impl->foo_id, cl);
          }
        if ((impl->klass != cl) && !_db_fill_implement(cl, impl))
          {
             ret = EINA_FALSE;
             goto end;
          }
        cl->callables = eina_list_append(cl->callables, impl);
        eina_hash_add(prop ? pth : th, impl->base.name, impl->base.name);
        _extend_impl(fs, impl, cl->type == EOLIAN_CLASS_INTERFACE);
     }

end:
   eina_hash_free(th);
   eina_hash_free(pth);
   return ret;
}

static Eina_Bool
_db_fill_ctors(Eolian_Class *cl)
{
   Eolian_Constructor *ctor;
   Eina_List *l;

   Eina_Bool ret = EINA_TRUE;

   Eina_Hash *th = eina_hash_string_small_new(NULL);
   EINA_LIST_FOREACH(cl->constructors, l, ctor)
     {
        if (eina_hash_find(th, ctor->base.name))
          {
             _eo_parser_log(&ctor->base, "duplicate ctor '%s'", ctor->base.name);
             ret = EINA_FALSE;
             goto end;
          }
        const char *ldot = strrchr(ctor->base.name, '.');
        if (!ldot)
          {
             ret = EINA_FALSE;
             goto end;
          }
        char *cnbuf = alloca(ldot - ctor->base.name + 1);
        memcpy(cnbuf, ctor->base.name, ldot - ctor->base.name);
        cnbuf[ldot - ctor->base.name] = '\0';
        const Eolian_Class *tcl = _get_impl_class(cl, cnbuf);
        if (!tcl)
          {
             _eo_parser_log(&ctor->base, "class '%s' not found within the inheritance tree of '%s'",
                            cnbuf, cl->base.name);
             ret = EINA_FALSE;
             goto end;
          }
        ctor->klass = tcl;
        const Eolian_Function *cfunc = eolian_constructor_function_get(ctor);
        if (!cfunc)
          {
             _eo_parser_log(&ctor->base, "unable to find function '%s'", ctor->base.name);
             ret = EINA_FALSE;
             goto end;
          }
        database_function_constructor_add((Eolian_Function *)cfunc, tcl);
        eina_hash_add(th, ctor->base.name, ctor->base.name);
     }

end:
   eina_hash_free(th);
   return ret;
}

static Eina_Bool
_db_swap_inherit(Eolian_Class *cl, Eina_Bool succ, Eina_Stringshare *in_cl,
                 Eolian_Class **out_cl, Eina_Bool iface_only)
{
   if (!succ)
     {
        eina_stringshare_del(in_cl);
        return EINA_FALSE;
     }
   Eolian_Class *icl = eina_hash_find(cl->base.unit->classes, in_cl);
   if (!icl)
     {
        succ = EINA_FALSE;
        _eo_parser_log(&cl->base, "unknown inherit '%s' (incorrect case?)", in_cl);
     }
   else if (iface_only && (icl->type != EOLIAN_CLASS_INTERFACE))
     {
        succ = EINA_FALSE;
        _eo_parser_log(&cl->base, "non-interface class '%s' in composite list", icl->base.name);
     }
   else
     *out_cl = icl;
   eina_stringshare_del(in_cl);
   return succ;
}

/* this is so we can inherit composite lists into regular classes
 * it doesn't need to be recursive since the parent/extension already
 * has its composite stuff filled in from before
 */
static void
_add_composite(Eolian_Class *cl, const Eolian_Class *icl, Eina_Hash *ch)
{
   const Eolian_Class *ccl;
   Eina_List *l;
   EINA_LIST_FOREACH(icl->composite, l, ccl)
     {
        if (eina_hash_find(ch, &ccl))
          continue;
        cl->composite = eina_list_append(cl->composite, ccl);
        eina_hash_add(ch, &ccl, ccl);
     }
}

static Eina_Bool
_db_fill_inherits(Validate_State *vals, Eolian_Class *cl, Eina_Hash *fhash,
                  Eina_Hash *errh)
{
   if (eina_hash_find(fhash, &cl->base.name))
     return EINA_TRUE;

   /* already merged outside of staging, therefore validated, and skipped */
   if (eina_hash_find(cl->base.unit->state->main.unit.classes, cl->base.name))
     return EINA_TRUE;

   Eina_List *il = cl->extends, *rl = cl->requires;
   Eina_Stringshare *inn = NULL;
   cl->extends = NULL;
   cl->requires = NULL;
   Eina_Bool succ = EINA_TRUE;

   if (cl->parent_name)
     {
        succ = _db_swap_inherit(cl, succ, cl->parent_name, &cl->parent, EINA_FALSE);
        if (succ)
          {
             /* fill if not found, but do not return right away because
              * the rest of the list needs to be freed in order not to
              * leak any memory
              */
             succ = _db_fill_inherits(vals, cl->parent, fhash, errh);
          }
     }

   EINA_LIST_FREE(il, inn)
     {
        Eolian_Class *out_cl = NULL;
        succ = _db_swap_inherit(cl, succ, inn, &out_cl, EINA_FALSE);
        if (!succ)
          continue;
        cl->extends = eina_list_append(cl->extends, out_cl);
        succ = _db_fill_inherits(vals, out_cl, fhash, errh);
     }

   if (succ && cl->type == EOLIAN_CLASS_MIXIN)
     {
        EINA_LIST_FREE(rl, inn)
          {
             Eolian_Class *out_cl = NULL;
             succ = _db_swap_inherit(cl, succ, inn, &out_cl, EINA_FALSE);
             if (succ && !(out_cl->type == EOLIAN_CLASS_REGULAR || out_cl->type == EOLIAN_CLASS_ABSTRACT))
               {
                  _eo_parser_log(&cl->base, "requires only allows regulars or abstracts");
                  succ = EINA_FALSE;
               }
             if (succ)
               {
                 _db_fill_inherits(vals, out_cl, fhash, errh);
               }
             if (!succ)
               continue;
             if (!eina_list_data_find(cl->requires, out_cl))
               cl->requires = eina_list_append(cl->requires, out_cl);
          }
     }

   /* a set of interfaces for quick checks */
   Eina_Hash *ch = eina_hash_pointer_new(NULL);

   /* replace the composite list with real instances and initial-fill the hash */
   il = cl->composite;
   cl->composite = NULL;
   int ncomp = 0;
   EINA_LIST_FREE(il, inn)
     {
        Eolian_Class *out_cl = NULL;
        succ = _db_swap_inherit(cl, succ, inn, &out_cl, EINA_TRUE);
        if (!succ)
          continue;
        cl->composite = eina_list_append(cl->composite, out_cl);
        succ = _db_fill_inherits(vals, out_cl, fhash, errh);
        ++ncomp;
        eina_hash_set(ch, &out_cl, out_cl);
     }

   /* parent can be abstract, those are not checked for unimplemented,
    * plus later we'll be exposing composite as an API, so we need to add
    * all composite interfaces from everywhere in the inheritance tree anyway
    */
   if (cl->parent)
     _add_composite(cl, cl->parent, ch);

   /* iterate extensions, add any composite stuff into the hash as well */
   Eolian_Class *icl;
   EINA_LIST_FOREACH(cl->extends, il, icl)
     _add_composite(cl, icl, ch);

   /* failed on the way, no point in filling further
    * the failed stuff will get dropped so it's ok if it's inconsistent
    */
   if (!succ)
     {
        eina_hash_free(ch);
        return EINA_FALSE;
     }

   eina_hash_add(fhash, &cl->base.name, cl);

   /* there are more than zero of composites of its own */
   if (ncomp > 0)
     {
        /* this one stores what is already in inheritance tree */
        Eina_Hash *ih = eina_hash_pointer_new(NULL);

        /* fill a complete inheritance tree set */
        if (cl->parent)
          _db_fill_ihash(cl->parent, ih);

        EINA_LIST_FOREACH(cl->extends, il, icl)
          _db_fill_ihash(icl, ih);

        /* go through only the explicitly specified composite list part, as the
         * rest was to be handled in parents already... add what hasn't been
         * explicitly implemented so far into implements/extends
         */
        EINA_LIST_FOREACH(cl->composite, il, icl)
          {
             /* ran out of classes */
             if (!ncomp--)
               break;
             /* found in inheritance tree, skip */
             if (eina_hash_find(ih, &icl))
               continue;
             cl->extends = eina_list_append(cl->extends, icl);
          }

        eina_hash_free(ih);
     }

   /* stores mappings from function to Impl_Status */
   Eina_Hash *fh = eina_hash_pointer_new(NULL);

   /* make sure impls/ctors are filled first, but do it only once */
   if (!_db_fill_implements(cl, fh))
     {
        eina_hash_free(ch);
        eina_hash_free(fh);
        return EINA_FALSE;
     }

   if (!_db_fill_ctors(cl))
     {
        eina_hash_free(ch);
        eina_hash_free(fh);
        return EINA_FALSE;
     }

   /* fill callables list with stuff from inheritance tree, the current
    * class stuff is already filled in _db_fill_implements, this is needed
    * in order to make sure all methods are implemented
    */
   if (cl->parent)
     _db_fill_callables(cl, cl->parent, fh, EINA_TRUE);

   EINA_LIST_FOREACH(cl->extends, il, icl)
     _db_fill_callables(cl, icl, fh, EINA_FALSE);

   /* verify that all methods are implemented on the class */
   if (!_db_check_implemented(vals, cl, fh, ch, errh))
     vals->warned = EINA_TRUE;

   eina_hash_free(fh);
   eina_hash_free(ch);

   return EINA_TRUE;
}

static Eina_Bool
_validate_implement(Validate_State *vals, Eolian_Implement *impl)
{
   if (impl->base.validated)
     return EINA_TRUE;

   if (!_validate_doc(impl->common_doc))
     return EINA_FALSE;
   if (!_validate_doc(impl->get_doc))
     return EINA_FALSE;
   if (!_validate_doc(impl->set_doc))
     return EINA_FALSE;

   /* common doc inherits @since into get/set doc */
   const char *old_since = vals->since_ver;
   if (impl->common_doc && !_validate_doc_since(vals, impl->common_doc))
     return EINA_FALSE;
   if (!_validate_doc_since_reset(vals, impl->get_doc))
     return EINA_FALSE;
   if (!_validate_doc_since_reset(vals, impl->set_doc))
     return EINA_FALSE;
   vals->since_ver = old_since;

   return _validate(&impl->base);
}

static Eina_List*
_required_classes(Eolian_Class *mixin)
{
   Eina_List *result = NULL, *n;
   Eolian_Class *extension;


   result = eina_list_clone(mixin->requires);

   if (mixin->parent)
     result = eina_list_merge(result, _required_classes(mixin->parent));

   EINA_LIST_FOREACH(mixin->extends, n, extension)
     result = eina_list_merge(result, _required_classes(extension));

   return result;
}

static Eina_Bool
_validate_class(Validate_State *vals, Eolian_Class *cl,
                Eina_Hash *nhash, Eina_Hash *ehash, Eina_Hash *phash, Eina_Hash *chash)
{
   Eina_List *l;
   Eolian_Function *func;
   Eolian_Event *event;
   Eolian_Part *part;
   Eolian_Implement *impl;
   Eolian_Class *icl;
   Eina_List *required_classes = NULL;

   if (!cl)
     return EINA_FALSE; /* if this happens something is very wrong though */

   /* we've gone through this part */
   if (eina_hash_find(chash, &cl))
     return EINA_TRUE;

   Eina_Bool valid = cl->base.validated;

   if (cl->parent)
     {
        /* first inherit needs some checking done on it */
        if (!valid) switch (cl->type)
          {
           case EOLIAN_CLASS_REGULAR:
           case EOLIAN_CLASS_ABSTRACT:
             if (cl->parent->type != EOLIAN_CLASS_REGULAR && cl->parent->type != EOLIAN_CLASS_ABSTRACT)
               {
                  _eo_parser_log(&cl->base, "regular classes ('%s') cannot inherit from non-regular classes ('%s')",
                                 cl->base.name, cl->parent->base.name);
                  return EINA_FALSE;
               }
             break;
           default:
             break;
          }
        if (!cl->base.is_beta && cl->parent->base.is_beta)
          {
             _eo_parser_log(&cl->base, "non-beta class cannot have beta parent");
             return EINA_FALSE;
          }
        if (!_validate_class(vals, cl->parent, nhash, ehash, phash, chash))
          return EINA_FALSE;
     }

   EINA_LIST_FOREACH(cl->extends, l, icl)
     {
        if (icl->type == EOLIAN_CLASS_MIXIN)
          {
             Eina_List *res = _required_classes(icl);
             Eolian_Class *required_class;
             Eina_List *n;
             EINA_LIST_FOREACH(res, n, required_class)
               {
                 if (!eina_list_data_find(required_classes, required_class))
                   required_classes = eina_list_append(required_classes, required_class);
               }
          }
        if (!valid) switch (icl->type)
          {
           case EOLIAN_CLASS_REGULAR:
           case EOLIAN_CLASS_ABSTRACT:
             /* regular class in extensions list, forbidden */
             {
                _eo_parser_log(&cl->base, "regular classes ('%s') cannot appear in extensions list of '%s'",
                               icl->base.name, cl->base.name);
                vals->warned = EINA_TRUE;
                break;
             }
           default:
             /* it's ok, interfaces are allowed */
             break;
          }
        if (!_validate_class(vals, icl, nhash, ehash, phash, chash))
          return EINA_FALSE;
     }
   if (cl->type == EOLIAN_CLASS_ABSTRACT || cl->type == EOLIAN_CLASS_REGULAR)
     {
        //walk up the parent list and remove all classes from there
        icl = cl;
        while (icl)
          {
             required_classes = eina_list_remove(required_classes, icl);
             icl = icl->parent;
          }
        //if there are a few left, drop, and error
        if (required_classes)
          {
             Eina_Strbuf *classes = eina_strbuf_new();
             Eolian_Class *required_class;
             Eina_List *n;
             EINA_LIST_FOREACH(required_classes, n, required_class)
               {
                   eina_strbuf_append(classes, required_class->base.name);
                   eina_strbuf_append_char(classes, ' ');
               }
             _eo_parser_log(&cl->base, "required classes %sare not in the inherit chain of %s",
                            eina_strbuf_string_get(classes), cl->base.name);
             eina_strbuf_free(classes);
             return EINA_FALSE;
          }
     }

   _set_stable(vals, !cl->base.is_beta);
   vals->since_ver = NULL;

   if (!_validate_doc_since(vals, cl->doc))
     return EINA_FALSE;

   EINA_LIST_FOREACH(cl->properties, l, func)
     if (!_validate_function(vals, func, nhash))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->methods, l, func)
     if (!_validate_function(vals, func, nhash))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->events, l, event)
     if (!_validate_event(vals, event, ehash))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->parts, l, part)
     if (!_validate_part(vals, part, phash))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->implements, l, impl)
     if (!_validate_implement(vals, impl))
       return EINA_FALSE;

   /* all the checks that need to be done every time are performed now */
   if (valid)
     {
        /* no need to go through this next time */
        eina_hash_add(chash, &cl, cl);
        return EINA_TRUE;
     }

   if (!_validate_doc(cl->doc))
     return EINA_FALSE;

   /* also done */
   eina_hash_add(chash, &cl, cl);

   return _validate(&cl->base);
}

static Eina_Bool
_validate_constant(Validate_State *vals, Eolian_Constant *var)
{
   if (var->base.validated)
     return EINA_TRUE;

   const char *old_since = vals->since_ver;
   vals->since_ver = NULL;

   Eina_Bool was_stable = _set_stable(vals, !var->base.is_beta && vals->stable);

   if (!_validate_doc_since(vals, var->doc))
     return EINA_FALSE;

   if (!_validate_type(vals, var->base_type, EINA_FALSE, EINA_FALSE))
     return _reset_stable(vals, was_stable, EINA_FALSE);

   if (!_validate_expr(var->value, var->base_type, 0, EINA_FALSE))
     return _reset_stable(vals, was_stable, EINA_FALSE);

   if (!_validate_doc(var->doc))
     return _reset_stable(vals, was_stable, EINA_FALSE);

   _reset_stable(vals, was_stable, EINA_TRUE);
   vals->since_ver = old_since;
   return _validate(&var->base);
}

static Eina_Bool
_typedecl_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
                 Eolian_Typedecl *tp, Cb_Ret *sc)
{
   return (sc->succ = _validate_typedecl(sc->vals, tp));
}

static Eina_Bool
_constant_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
             Eolian_Constant *var, Cb_Ret *sc)
{
   return (sc->succ = _validate_constant(sc->vals, var));
}

Eina_Bool
database_validate(const Eolian_Unit *src)
{
   Eolian_Class *cl;

   Validate_State vals = {
      EINA_FALSE,
      EINA_TRUE,
      !!getenv("EFL_RUN_IN_TREE"),
      !!getenv("EOLIAN_CLASS_UNIMPLEMENTED_BETA_WARN"),
      !!getenv("EOLIAN_ENFORCE_SINCE")
   };

   /* do an initial pass to refill inherits */
   Eina_Iterator *iter = eolian_unit_classes_get(src);
   Eina_Hash *fhash = eina_hash_pointer_new(NULL);
   /* keeps track of impls we already errored on to reduce verbosity */
   Eina_Hash *errh = eina_hash_pointer_new(NULL);
   EINA_ITERATOR_FOREACH(iter, cl)
     {
        /* clear, because otherwise if unrelated classes A and B both
         * had interface C in extensions list without implementing it,
         * it would only get printed for A
         */
        eina_hash_free_buckets(errh);
        if (!_db_fill_inherits(&vals, cl, fhash, errh))
          {
             eina_hash_free(errh);
             eina_hash_free(fhash);
             return EINA_FALSE;
          }
     }
   eina_hash_free(errh);
   eina_hash_free(fhash);
   eina_iterator_free(iter);

   iter = eolian_unit_classes_get(src);
   Eina_Hash *nhash = eina_hash_pointer_new(NULL);
   Eina_Hash *ehash = eina_hash_pointer_new(NULL);
   Eina_Hash *phash = eina_hash_pointer_new(NULL);
   Eina_Hash *chash = eina_hash_pointer_new(NULL);
   EINA_ITERATOR_FOREACH(iter, cl)
     {
        eina_hash_free_buckets(nhash);
        eina_hash_free_buckets(ehash);
        eina_hash_free_buckets(phash);
        eina_hash_free_buckets(chash);
        if (!_validate_class(&vals, cl, nhash, ehash, phash, chash))
          {
             eina_iterator_free(iter);
             eina_hash_free(nhash);
             eina_hash_free(ehash);
             eina_hash_free(phash);
             eina_hash_free(chash);
             return EINA_FALSE;
          }
     }
   eina_hash_free(chash);
   eina_hash_free(phash);
   eina_hash_free(ehash);
   eina_hash_free(nhash);
   eina_iterator_free(iter);

   Cb_Ret rt = { &vals, EINA_TRUE };

   eina_hash_foreach(src->aliases, (Eina_Hash_Foreach)_typedecl_map_cb, &rt);
   if (!rt.succ)
     return EINA_FALSE;

   eina_hash_foreach(src->structs, (Eina_Hash_Foreach)_typedecl_map_cb, &rt);
   if (!rt.succ)
     return EINA_FALSE;

   eina_hash_foreach(src->enums, (Eina_Hash_Foreach)_typedecl_map_cb, &rt);
   if (!rt.succ)
     return EINA_FALSE;

   eina_hash_foreach(src->constants, (Eina_Hash_Foreach)_constant_map_cb, &rt);
   if (!rt.succ)
     return EINA_FALSE;

   if(vals.warned)
     return EINA_FALSE;

   return EINA_TRUE;
}

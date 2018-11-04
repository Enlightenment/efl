#include <ctype.h>
#include <assert.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eo_lexer.h"
#include "eolian_priv.h"

typedef struct _Validate_State
{
   Eina_Bool warned;
   Eina_Bool event_redef;
} Validate_State;

static Eina_Bool
_validate(Eolian_Object *obj)
{
   obj->validated = EINA_TRUE;
   return EINA_TRUE;
}

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

static Eina_Bool _validate_type(Validate_State *vals, Eolian_Type *tp);
static Eina_Bool _validate_expr(Eolian_Expression *expr,
                                const Eolian_Type *tp,
                                Eolian_Expression_Mask msk);
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
   sc->succ = _validate_type(sc->vals, sf->type);

   if (!sc->succ)
     return EINA_FALSE;

   sc->succ = _validate_doc(sf->doc);

   return sc->succ;
}

static Eina_Bool
_ef_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
           const Eolian_Enum_Type_Field *ef, Cb_Ret *sc)
{
   if (ef->value)
     sc->succ = _validate_expr(ef->value, NULL, EOLIAN_MASK_INT);
   else
     sc->succ = EINA_TRUE;

   if (!sc->succ)
     return EINA_FALSE;

   sc->succ = _validate_doc(ef->doc);

   return sc->succ;
}

static Eina_Bool
_obj_error(const Eolian_Object *o, const char *msg)
{
   eolian_state_log_obj(o->unit->state, o, "%s", msg);
   return EINA_FALSE;
}

static Eina_Bool
_validate_typedecl(Validate_State *vals, Eolian_Typedecl *tp)
{
   if (tp->base.validated)
     return EINA_TRUE;

   if (!_validate_doc(tp->doc))
     return EINA_FALSE;

   switch (tp->type)
     {
      case EOLIAN_TYPEDECL_ALIAS:
        if (!_validate_type(vals, tp->base_type))
          return EINA_FALSE;
        if (!tp->freefunc && tp->base_type->freefunc)
          tp->freefunc = eina_stringshare_ref(tp->base_type->freefunc);
        return _validate(&tp->base);
      case EOLIAN_TYPEDECL_STRUCT:
        {
           Cb_Ret rt = { vals, EINA_TRUE };
           eina_hash_foreach(tp->fields, (Eina_Hash_Foreach)_sf_map_cb, &rt);
           if (!rt.succ)
             return EINA_FALSE;
           return _validate(&tp->base);
        }
      case EOLIAN_TYPEDECL_STRUCT_OPAQUE:
        return _validate(&tp->base);
      case EOLIAN_TYPEDECL_ENUM:
        {
           Cb_Ret rt = { vals, EINA_TRUE };
           eina_hash_foreach(tp->fields, (Eina_Hash_Foreach)_ef_map_cb, &rt);
           if (!rt.succ)
             return EINA_FALSE;
           return _validate(&tp->base);
        }
      case EOLIAN_TYPEDECL_FUNCTION_POINTER:
        if (!_validate_function(vals, tp->function_pointer, NULL))
          return EINA_FALSE;
        return _validate(&tp->base);
      default:
        return EINA_FALSE;
     }
   return _validate(&tp->base);
}

static const char * const eo_complex_frees[] =
{
   "eina_accessor_free", "eina_array_free", NULL, /* future */
   "eina_iterator_free", "eina_hash_free",
   "eina_list_free", "eina_inarray_free", "eina_inlist_free"
};

static const char *eo_obj_free = "efl_del";
static const char *eo_str_free = "free";
static const char *eo_strshare_free = "eina_stringshare_del";
static const char *eo_value_free = "eina_value_flush";
static const char *eo_value_ptr_free = "eina_value_free";

static Eina_Bool
_validate_type(Validate_State *vals, Eolian_Type *tp)
{
   const Eolian_Unit *src = tp->base.unit;

   char buf[256];
   if (tp->owned && !database_type_is_ownable(src, tp, EINA_FALSE))
     {
        snprintf(buf, sizeof(buf), "type '%s' is not ownable", tp->base.name);
        return _obj_error(&tp->base, buf);
     }

   if (tp->is_ptr && !tp->legacy)
     {
        tp->is_ptr = EINA_FALSE;
        Eina_Bool still_ownable = database_type_is_ownable(src, tp, EINA_FALSE);
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
                int kwid = eo_lexer_keyword_str_to_id(tp->base.name);
                if (!tp->freefunc)
                  {
                     tp->freefunc = eina_stringshare_add(eo_complex_frees[
                       kwid - KW_accessor]);
                  }
                Eolian_Type *itp = tp->base_type;
                /* validate types in brackets so freefuncs get written... */
                while (itp)
                  {
                     if (!_validate_type(vals, itp))
                       return EINA_FALSE;
                     if ((kwid >= KW_accessor) && (kwid <= KW_list) && (kwid != KW_future))
                       {
                          if (!database_type_is_ownable(src, itp, EINA_TRUE))
                            {
                               snprintf(buf, sizeof(buf),
                                        "%s cannot contain value types (%s)",
                                        tp->base.name, itp->base.name);
                               return _obj_error(&itp->base, buf);
                            }
                       }
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
                snprintf(buf, sizeof(buf), "undefined type %s", tp->base.name);
                return _obj_error(&tp->base, buf);
             }
           if (!_validate_typedecl(vals, tp->tdecl))
             return EINA_FALSE;
           if (tp->tdecl->freefunc && !tp->freefunc)
             tp->freefunc = eina_stringshare_ref(tp->tdecl->freefunc);
           return _validate(&tp->base);
        }
      case EOLIAN_TYPE_CLASS:
        {
           tp->klass = (Eolian_Class *)eolian_unit_class_by_name_get(src, tp->base.name);
           if (!tp->klass)
             {
                snprintf(buf, sizeof(buf), "undefined class %s "
                         "(likely wrong namespacing)", tp->base.name);
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
_validate_expr(Eolian_Expression *expr, const Eolian_Type *tp,
               Eolian_Expression_Mask msk)
{
   Eolian_Value val;
   if (tp)
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
   if (!_validate_type(vals, param->type))
     return EINA_FALSE;

   if (param->value && !_validate_expr(param->value, param->type, 0))
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
   char buf[512];

   const Eolian_Object *oobj = nhash ? eina_hash_find(nhash, &func->base.name) : NULL;
   if (EINA_UNLIKELY(oobj && (oobj != &func->base)))
     {
        snprintf(buf, sizeof(buf),
                 "%sfunction '%s' conflicts with another symbol (at %s:%d:%d)",
                 func->is_beta ? "beta " : "", func->base.name, oobj->file,
                 oobj->line, oobj->column);
        _obj_error(&func->base, buf);
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

   if (func->get_ret_type && !_validate_type(vals, func->get_ret_type))
     return EINA_FALSE;

   if (func->set_ret_type && !_validate_type(vals, func->set_ret_type))
     return EINA_FALSE;

   if (func->get_ret_val && !_validate_expr(func->get_ret_val,
                                            func->get_ret_type, 0))
     return EINA_FALSE;

   if (func->set_ret_val && !_validate_expr(func->set_ret_val,
                                            func->set_ret_type, 0))
     return EINA_FALSE;

#define EOLIAN_PARAMS_VALIDATE(params) \
   EINA_LIST_FOREACH(params, l, param) \
     if (!_validate_param(vals, param)) \
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

   /* just for now, when dups become errors there will be no need to check */
   if (!oobj && nhash)
     eina_hash_add(nhash, &func->base.name, &func->base);

   return _validate(&func->base);
}

static Eina_Bool
_validate_part(Eolian_Part *part, Eina_Hash *nhash)
{
   const Eolian_Object *oobj = eina_hash_find(nhash, &part->base.name);
   if (oobj)
     {
        char buf[512];
        snprintf(buf, sizeof(buf),
                 "part '%s' conflicts with another symbol (at %s:%d:%d)",
                 part->base.name, oobj->file, oobj->line, oobj->column);
        _obj_error(&part->base, buf);
     }

   /* see _validate_function above */
   if (part->base.validated)
     {
        if (!oobj)
          eina_hash_add(nhash, &part->base.name, &part->base);
        return EINA_TRUE;
     }

   if (!_validate_doc(part->doc))
     return EINA_FALSE;

   /* switch the class name for class */
   Eolian_Class *pcl = eina_hash_find(part->base.unit->classes, part->klass_name);
   if (!pcl)
     {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "unknown part class '%s' (incorrect case?)",
                 part->klass_name);
        _obj_error(&part->base, buf);
        return EINA_FALSE;
     }
   eina_stringshare_del(part->klass_name);
   part->klass = pcl;

   if (!oobj)
     eina_hash_add(nhash, &part->base.name, &part->base);

   return _validate(&part->base);
}

static Eina_Bool
_validate_event(Validate_State *vals, Eolian_Event *event, Eina_Hash *nhash)
{
   char buf[512];
   const Eolian_Object *oobj = NULL;

   if (vals->event_redef)
     {
        oobj = eina_hash_find(nhash, &event->base.name);
        if (EINA_UNLIKELY(!!oobj))
          {
             snprintf(buf, sizeof(buf),
                      "event '%s' conflicts with another symbol (at %s:%d:%d)",
                      event->base.name, oobj->file, oobj->line, oobj->column);
             _obj_error(&event->base, buf);
             vals->warned = EINA_TRUE;
          }
     }

   if (event->base.validated)
     {
        if (vals->event_redef && !oobj)
          eina_hash_add(nhash, &event->base.name, &event->base);
        return EINA_TRUE;
     }

   if (!_validate_type(vals, event->type))
     return EINA_FALSE;

   if (!_validate_doc(event->doc))
     return EINA_FALSE;

   if (vals->event_redef && !oobj)
     eina_hash_add(nhash, &event->base.name, &event->base);

   return _validate(&event->base);
}

const Eolian_Class *
_get_impl_class(const Eolian_Class *cl, const char *cln)
{
   if (!cl || !strcmp(cl->base.name, cln))
     return cl;
   Eina_List *l;
   Eolian_Class *icl;
   EINA_LIST_FOREACH(cl->inherits, l, icl)
     {
        /* we can do a depth first search, it's easier and doesn't matter
         * which part of the inheritance tree we find the class in
         */
        const Eolian_Class *fcl = _get_impl_class(icl, cln);
        if (fcl)
          return fcl;
     }
   return NULL;
}

#define _eo_parser_log(_base, ...) \
   eolian_state_log_obj((_base)->unit->state, (_base), __VA_ARGS__)

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

static Eina_Bool
_db_fill_implements(Eolian_Class *cl)
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
        eina_hash_add(prop ? pth : th, impl->base.name, impl->base.name);
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
_db_fill_inherits(Eolian_Class *cl, Eina_Hash *fhash)
{
   if (eina_hash_find(fhash, &cl->base.name))
     return EINA_TRUE;

   /* already merged outside of staging, therefore validated, and skipped */
   if (eina_hash_find(cl->base.unit->state->main.unit.classes, cl->base.name))
     return EINA_TRUE;

   Eina_List *il = cl->inherits;
   Eina_Stringshare *inn = NULL;
   cl->inherits = NULL;
   Eina_Bool succ = EINA_TRUE;

   EINA_LIST_FREE(il, inn)
     {
        if (!succ)
          {
             eina_stringshare_del(inn);
             continue;
          }
        Eolian_Class *icl = eina_hash_find(cl->base.unit->classes, inn);
        if (!icl)
          {
             succ = EINA_FALSE;
             char buf[PATH_MAX];
             snprintf(buf, sizeof(buf), "unknown inherit '%s' (incorrect case?)", inn);
             _obj_error(&cl->base, buf);
          }
        else
          {
             cl->inherits = eina_list_append(cl->inherits, icl);
             /* fill if not found, but do not return right away because
              * the rest of the list needs to be freed in order not to
              * leak any memory
              */
             if (!_db_fill_inherits(icl, fhash))
               succ = EINA_FALSE;
          }
        eina_stringshare_del(inn);
     }

   /* failed on the way, no point in filling further
    * the failed stuff will get dropped so it's ok if it's inconsistent
    */
   if (!succ)
     return EINA_FALSE;

   eina_hash_add(fhash, &cl->base.name, cl);

   /* make sure impls/ctors are filled first, but do it only once */
   if (!_db_fill_implements(cl))
     return EINA_FALSE;

   if (!_db_fill_ctors(cl))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_validate_implement(Eolian_Implement *impl)
{
   if (impl->base.validated)
     return EINA_TRUE;

   if (!_validate_doc(impl->common_doc))
     return EINA_FALSE;
   if (!_validate_doc(impl->get_doc))
     return EINA_FALSE;
   if (!_validate_doc(impl->set_doc))
     return EINA_FALSE;

   return _validate(&impl->base);
}

static Eina_Bool
_validate_class(Validate_State *vals, Eolian_Class *cl,
                Eina_Hash *nhash, Eina_Hash *chash)
{
   Eina_List *l;
   Eolian_Function *func;
   Eolian_Event *event;
   Eolian_Part *part;
   Eolian_Implement *impl;
   Eolian_Class *icl;

   if (!cl)
     return EINA_FALSE; /* if this happens something is very wrong though */

   /* we've gone through this part */
   if (eina_hash_find(chash, &cl))
     return EINA_TRUE;

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
                           cl->base.name, icl->base.name);
                  return _obj_error(&cl->base, buf);
               }
             break;
           case EOLIAN_CLASS_MIXIN:
           case EOLIAN_CLASS_INTERFACE:
             if (icl->type != EOLIAN_CLASS_MIXIN && icl->type != EOLIAN_CLASS_INTERFACE)
               {
                  char buf[PATH_MAX];
                  snprintf(buf, sizeof(buf), "non-regular classes ('%s') cannot inherit from regular classes ('%s')",
                           cl->base.name, icl->base.name);
                  return _obj_error(&cl->base, buf);
               }
             break;
           default:
             break;
          }
        if (!_validate_class(vals, icl, nhash, chash))
          return EINA_FALSE;
     }

   EINA_LIST_FOREACH(cl->properties, l, func)
     if (!_validate_function(vals, func, nhash))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->methods, l, func)
     if (!_validate_function(vals, func, nhash))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->events, l, event)
     if (!_validate_event(vals, event, nhash))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->parts, l, part)
     if (!_validate_part(part, nhash))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->implements, l, impl)
     if (!_validate_implement(impl))
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
_validate_variable(Validate_State *vals, Eolian_Variable *var)
{
   if (var->base.validated)
     return EINA_TRUE;

   if (!_validate_type(vals, var->base_type))
     return EINA_FALSE;

   if (var->value && !_validate_expr(var->value, var->base_type, 0))
     return EINA_FALSE;

   if (!_validate_doc(var->doc))
     return EINA_FALSE;

   return _validate(&var->base);
}

static Eina_Bool
_typedecl_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
                 Eolian_Typedecl *tp, Cb_Ret *sc)
{
   return (sc->succ = _validate_typedecl(sc->vals, tp));
}

static Eina_Bool
_var_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
             Eolian_Variable *var, Cb_Ret *sc)
{
   return (sc->succ = _validate_variable(sc->vals, var));
}

Eina_Bool
database_validate(const Eolian_Unit *src)
{
   Eolian_Class *cl;

   Validate_State vals = {
      EINA_FALSE,
      !!getenv("EOLIAN_EVENT_REDEF_WARN")
   };

   /* do an initial pass to refill inherits */
   Eina_Iterator *iter = eolian_unit_classes_get(src);
   Eina_Hash *fhash = eina_hash_pointer_new(NULL);
   EINA_ITERATOR_FOREACH(iter, cl)
     {
        if (!_db_fill_inherits(cl, fhash))
          {
             eina_hash_free(fhash);
             return EINA_FALSE;
          }
     }
   eina_hash_free(fhash);
   eina_iterator_free(iter);

   iter = eolian_unit_classes_get(src);
   Eina_Hash *nhash = eina_hash_pointer_new(NULL);
   Eina_Hash *chash = eina_hash_pointer_new(NULL);
   EINA_ITERATOR_FOREACH(iter, cl)
     {
        eina_hash_free_buckets(nhash);
        eina_hash_free_buckets(chash);
        if (!_validate_class(&vals, cl, nhash, chash))
          {
             eina_iterator_free(iter);
             eina_hash_free(nhash);
             eina_hash_free(chash);
             return EINA_FALSE;
          }
     }
   eina_hash_free(chash);
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

   eina_hash_foreach(src->globals, (Eina_Hash_Foreach)_var_map_cb, &rt);
   if (!rt.succ)
     return EINA_FALSE;

   eina_hash_foreach(src->constants, (Eina_Hash_Foreach)_var_map_cb, &rt);
   if (!rt.succ)
     return EINA_FALSE;

   if(vals.warned)
     return EINA_FALSE;

   return EINA_TRUE;
}

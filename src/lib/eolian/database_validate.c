#include <ctype.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eo_lexer.h"

typedef struct _Validator {
   Eina_Bool silent_types;
} Validator;

typedef struct _Val_Success {
   const Validator *vs;
   Eina_Bool success;
} Val_Success;

static Eina_Bool
_validate_ref(const Validator *vs EINA_UNUSED, const char *ref,
              const Eolian_Object *info)
{
   if (eolian_declaration_get_by_name(ref))
     return EINA_TRUE;

   const char *suffix = strrchr(ref, '.');
   if (!suffix) goto failed;

   Eina_Stringshare *base = eina_stringshare_add_length(ref, suffix - ref);

   const Eolian_Typedecl *tpd = eolian_typedecl_struct_get_by_name(base);
   if (tpd)
     {
        eina_stringshare_del(base);
        if (!eolian_typedecl_struct_field_get(tpd, suffix + 1))
          goto failed;
        return EINA_TRUE;
     }

   tpd = eolian_typedecl_enum_get_by_name(base);
   if (tpd)
     {
        eina_stringshare_del(base);
        if (!eolian_typedecl_enum_field_get(tpd, suffix + 1))
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
_validate_docstr(const Validator *vs, Eina_Stringshare *str,
                 const Eolian_Object *info)
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
        if (!_validate_ref(vs, refs, info))
          {
             eina_stringshare_del(refs);
             return EINA_FALSE;
          }
        eina_stringshare_del(refs);
     }

   return EINA_TRUE;
}

static Eina_Bool
_validate_doc(const Validator *vs, const Eolian_Documentation *doc)
{
   if (!doc) return EINA_TRUE;

   if (!_validate_docstr(vs, doc->summary, &doc->base))
     return EINA_FALSE;
   if (!_validate_docstr(vs, doc->description, &doc->base))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool _validate_type(const Validator *vs, const Eolian_Type *tp);
static Eina_Bool _validate_expr(const Validator *vs,
                                const Eolian_Expression *expr,
                                const Eolian_Type *tp,
                                Eolian_Expression_Mask msk);

static Eina_Bool
_sf_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
           const Eolian_Struct_Type_Field *sf, Val_Success *sc)
{
   sc->success = _validate_type(sc->vs, sf->type);

   if (!sc->success)
     return EINA_FALSE;

   sc->success = _validate_doc(sc->vs, sf->doc);

   return sc->success;
}

static Eina_Bool
_ef_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
           const Eolian_Enum_Type_Field *ef, Val_Success *sc)
{
   if (ef->value)
     sc->success = _validate_expr(sc->vs, ef->value, NULL, EOLIAN_MASK_INT);
   else
     sc->success = EINA_TRUE;

   if (!sc->success)
     return EINA_FALSE;

   sc->success = _validate_doc(sc->vs, ef->doc);

   return sc->success;
}

static Eina_Bool
_type_error(const Validator *vs, const Eolian_Type *tp, const char *msg)
{
   if (vs->silent_types)
     return EINA_FALSE;
   fprintf(stderr, "eolian:%s:%d:%d: %s\n", tp->base.file, tp->base.line,
           tp->base.column, msg);
   return EINA_FALSE;
}

static Eina_Bool
_validate_typedecl(const Validator *vs, const Eolian_Typedecl *tp)
{
   if (!_validate_doc(vs, tp->doc))
     return EINA_FALSE;

   switch (tp->type)
     {
      case EOLIAN_TYPEDECL_ALIAS:
        return _validate_type(vs, tp->base_type);
      case EOLIAN_TYPEDECL_STRUCT:
        {
           Val_Success succ;
           succ.vs = vs;
           succ.success = EINA_TRUE;
           eina_hash_foreach(tp->fields, (Eina_Hash_Foreach)_sf_map_cb, &succ);
           return succ.success;
        }
      case EOLIAN_TYPEDECL_STRUCT_OPAQUE:
        return EINA_TRUE;
      case EOLIAN_TYPEDECL_ENUM:
        {
           Val_Success succ;
           succ.vs = vs;
           succ.success = EINA_TRUE;
           eina_hash_foreach(tp->fields, (Eina_Hash_Foreach)_ef_map_cb, &succ);
           return succ.success;
        }
      default:
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_validate_type(const Validator *vs, const Eolian_Type *tp)
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
                return _type_error(vs, tp, buf);
             }
           return _validate_typedecl(vs, tpp);
        }
      case EOLIAN_TYPE_POINTER:
      case EOLIAN_TYPE_STATIC_ARRAY:
      case EOLIAN_TYPE_TERMINATED_ARRAY:
        return _validate_type(vs, tp->base_type);
      case EOLIAN_TYPE_CLASS:
        {
           if (!eolian_type_class_get(tp))
             {
                /* for now only warn */
                char buf[256];
                snprintf(buf, sizeof(buf), "undefined class %s "
                         "(likely wrong namespacing)", tp->full_name);
                return _type_error(vs, tp, buf);
             }
           return EINA_TRUE;
        }
      default:
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_validate_expr(const Validator *vs EINA_UNUSED, const Eolian_Expression *expr,
               const Eolian_Type *tp, Eolian_Expression_Mask msk)
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
_validate_param(const Validator *vs, const Eolian_Function_Parameter *param)
{
   if (!_validate_type(vs, param->type))
     return EINA_FALSE;

   if (!_validate_doc(vs, param->doc))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_validate_function(const Validator *vs, const Eolian_Function *func)
{
   Eina_List *l;
   const Eolian_Function_Parameter *param;

   if (func->get_ret_type && !_validate_type(vs, func->get_ret_type))
     return EINA_FALSE;

   if (func->set_ret_type && !_validate_type(vs, func->set_ret_type))
     return EINA_FALSE;

   if (func->get_ret_val && !_validate_expr(vs, func->get_ret_val,
                                            func->get_ret_type, 0))
     return EINA_FALSE;

   if (func->set_ret_val && !_validate_expr(vs, func->set_ret_val,
                                            func->set_ret_type, 0))
     return EINA_FALSE;

#define EOLIAN_PARAMS_VALIDATE(params) \
   EINA_LIST_FOREACH(params, l, param) \
     if (!_validate_param(vs, param)) \
       return EINA_FALSE;

   EOLIAN_PARAMS_VALIDATE(func->prop_values);
   EOLIAN_PARAMS_VALIDATE(func->prop_values_get);
   EOLIAN_PARAMS_VALIDATE(func->prop_values_set);
   EOLIAN_PARAMS_VALIDATE(func->prop_keys);
   EOLIAN_PARAMS_VALIDATE(func->prop_keys_get);
   EOLIAN_PARAMS_VALIDATE(func->prop_keys_set);

#undef EOLIAN_PARAMS_VALIDATE

   if (!_validate_doc(vs, func->common_doc))
     return EINA_FALSE;
   if (!_validate_doc(vs, func->get_doc))
     return EINA_FALSE;
   if (!_validate_doc(vs, func->set_doc))
     return EINA_FALSE;
   if (!_validate_doc(vs, func->get_return_doc))
     return EINA_FALSE;
   if (!_validate_doc(vs, func->set_return_doc))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_validate_event(const Validator *vs, const Eolian_Event *event)
{
   if (event->type && !_validate_type(vs, event->type))
     return EINA_FALSE;

   if (!_validate_doc(vs, event->doc))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_validate_class(const Validator *vs, const Eolian_Class *cl)
{
   Eina_List *l;
   const Eolian_Function *func;
   const Eolian_Event *event;

   EINA_LIST_FOREACH(cl->properties, l, func)
     if (!_validate_function(vs, func))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->methods, l, func)
     if (!_validate_function(vs, func))
       return EINA_FALSE;

   EINA_LIST_FOREACH(cl->events, l, event)
     if (!_validate_event(vs, event))
       return EINA_FALSE;

   if (!_validate_doc(vs, cl->doc))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_validate_variable(const Validator *vs, const Eolian_Variable *var)
{
   if (!_validate_type(vs, var->base_type))
     return EINA_FALSE;

   if (var->value && !_validate_expr(vs, var->value, var->base_type, 0))
     return EINA_FALSE;

   if (!_validate_doc(vs, var->doc))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_typedecl_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
                 const Eolian_Typedecl *tp, Val_Success *sc)
{
   sc->success = _validate_typedecl(sc->vs, tp);
   return sc->success;
}

static Eina_Bool
_var_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
             const Eolian_Variable *var, Val_Success *sc)
{
   sc->success = _validate_variable(sc->vs, var);
   return sc->success;
}

Eina_Bool
database_validate(Eina_Bool silent_types)
{
   Validator vs;
   const Eolian_Class *cl;

   vs.silent_types = silent_types;

   Eina_Iterator *iter = eolian_all_classes_get();
   EINA_ITERATOR_FOREACH(iter, cl)
     if (!_validate_class(&vs, cl))
       {
          eina_iterator_free(iter);
          return EINA_FALSE;
       }
   eina_iterator_free(iter);

   Val_Success succ;
   succ.vs = &vs;
   succ.success = EINA_TRUE;

   eina_hash_foreach(_aliases, (Eina_Hash_Foreach)_typedecl_map_cb, &succ);
   if (!succ.success)
     return EINA_FALSE;

   eina_hash_foreach(_structs, (Eina_Hash_Foreach)_typedecl_map_cb, &succ);
   if (!succ.success)
     return EINA_FALSE;

   eina_hash_foreach(_enums, (Eina_Hash_Foreach)_typedecl_map_cb, &succ);
   if (!succ.success)
     return EINA_FALSE;

   eina_hash_foreach(_globals, (Eina_Hash_Foreach)_var_map_cb, &succ);
   if (!succ.success)
     return EINA_FALSE;

   eina_hash_foreach(_constants, (Eina_Hash_Foreach)_var_map_cb, &succ);
   if (!succ.success)
     return EINA_FALSE;

   return EINA_TRUE;
}

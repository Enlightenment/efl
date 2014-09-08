#include "eo_lexer.h"

static Eina_Bool _validate_type(const Eolian_Type *tp);
static Eina_Bool _validate_expr(const Eolian_Expression *expr,
                                const Eolian_Type *tp,
                                Eolian_Expression_Mask msk);

static Eina_Bool
_sf_map_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
           const Eolian_Struct_Type_Field *sf, Eina_Bool *success)
{
   *success = _validate_type(sf->type);
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
   switch (tp->type)
     {
      case EOLIAN_TYPE_VOID:
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
           tpp = eolian_type_alias_get_by_name(tp->full_name);
           if (!tpp)
             {
                char buf[256];
                snprintf(buf, sizeof(buf), "undefined alias %s", tp->full_name);
                _type_error(tp, buf);
                return EINA_TRUE; /* for now only warn */
             }
           return _validate_type(tpp);
        }
      case EOLIAN_TYPE_REGULAR_STRUCT:
        {
           const Eolian_Type *tpp;
           tpp = eolian_type_struct_get_by_name(tp->full_name);
           if (!tpp)
             {
                char buf[256];
                snprintf(buf, sizeof(buf), "undefined struct %s", tp->full_name);
                _type_error(tp, buf);
                return EINA_TRUE; /* for now only warn */
             }
           return _validate_type(tpp);
        }
      case EOLIAN_TYPE_REGULAR_ENUM:
        {
           const Eolian_Type *tpp;
           tpp = eolian_type_enum_get_by_name(tp->full_name);
           if (!tpp)
             {
                char buf[256];
                snprintf(buf, sizeof(buf), "undefined enum %s", tp->full_name);
                _type_error(tp, buf);
                return EINA_TRUE; /* for now only warn */
             }
           return _validate_type(tpp);
        }
      case EOLIAN_TYPE_POINTER:
      case EOLIAN_TYPE_ALIAS:
        return _validate_type(tp->base_type);
      case EOLIAN_TYPE_FUNCTION:
        {
           Eina_List *l;
           Eolian_Type *tpp;
           if (tp->ret_type && !_validate_type(tp->ret_type))
             return EINA_FALSE;
           EINA_LIST_FOREACH(tp->arguments, l, tpp)
             if (!_validate_type(tpp))
               return EINA_FALSE;
           return EINA_TRUE;
        }
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
   if (tp)
      val = eolian_expression_eval_type(expr, tp);
   else
      val = eolian_expression_eval(expr, msk);
   return (val.type != EOLIAN_EXPR_UNKNOWN);
}

static Eina_Bool
_validate_param(const Eolian_Function_Parameter *param)
{
   if (!_validate_type(param->type))
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

   EINA_LIST_FOREACH(func->keys, l, param)
     if (!_validate_param(param))
       return EINA_FALSE;

   EINA_LIST_FOREACH(func->params, l, param)
     if (!_validate_param(param))
       return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_validate_event(const Eolian_Event *event)
{
   if (event->type && !_validate_type(event->type))
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

   return EINA_TRUE;
}

static Eina_Bool
_validate_variable(const Eolian_Variable *var)
{
   if (!_validate_type(var->base_type))
     return EINA_FALSE;

   if (var->value && !_validate_expr(var->value, var->base_type, 0))
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

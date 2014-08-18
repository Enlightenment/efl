#include <Eina.h>
#include "eo_lexer.h"

EAPI Eolian_Value
eolian_expression_eval(const Eolian_Expression *expr, Eolian_Expression_Mask m)
{
   Eolian_Value err;
   err.type = EOLIAN_EXPR_UNKNOWN;
   EINA_SAFETY_ON_NULL_RETURN_VAL(expr, err);
   return database_expr_eval(expr, m);
}

static Eolian_Value
_eval_type(const Eolian_Expression *expr, const Eolian_Type *type)
{
   Eolian_Value err;
   err.type = EOLIAN_EXPR_UNKNOWN;
   if (!type)
     return err;
   switch (type->type)
     {
      case EOLIAN_TYPE_ALIAS:
        return _eval_type(expr, eolian_type_base_type_get(type));
      case EOLIAN_TYPE_POINTER:
        {
           int mask = EOLIAN_MASK_NULL;
           const Eolian_Type *base = eolian_type_base_type_get(type);
           int kw = base->name ? eo_lexer_keyword_str_to_id(base->name) : 0;
           if (kw == KW_char)
             mask |= EOLIAN_MASK_STRING;
           return database_expr_eval(expr, mask);
        }
      case EOLIAN_TYPE_CLASS:
        return database_expr_eval(expr, EOLIAN_MASK_NULL);
      case EOLIAN_TYPE_REGULAR:
        {
           int  kw = eo_lexer_keyword_str_to_id(type->name);
           if (!kw || kw < KW_byte || kw >= KW_void)
             return err;
           switch (kw)
             {
              case KW_byte:
              case KW_short:
              case KW_int:
              case KW_long:
              case KW_llong:
              case KW_int8:
              case KW_int16:
              case KW_int32:
              case KW_int64:
              case KW_int128:
              case KW_ssize:
              case KW_intptr:
              case KW_ptrdiff:
                return database_expr_eval(expr, EOLIAN_MASK_SINT);
              case KW_ubyte:
              case KW_ushort:
              case KW_uint:
              case KW_ulong:
              case KW_ullong:
              case KW_uint8:
              case KW_uint16:
              case KW_uint32:
              case KW_uint64:
              case KW_uint128:
              case KW_size:
              case KW_uintptr:
              case KW_time:
                return database_expr_eval(expr, EOLIAN_MASK_UINT);
              case KW_float:
              case KW_double:
                return database_expr_eval(expr, EOLIAN_MASK_FLOAT);
              case KW_bool:
                return database_expr_eval(expr, EOLIAN_MASK_BOOL);
              case KW_char:
                return database_expr_eval(expr, EOLIAN_MASK_CHAR);
              default:
                return err;
             }
        }
      default:
        return err;
     }
}

EAPI Eolian_Value
eolian_expression_eval_type(const Eolian_Expression *expr,
                            const Eolian_Type *type)
{
   Eolian_Value err;
   err.type = EOLIAN_EXPR_UNKNOWN;
   EINA_SAFETY_ON_NULL_RETURN_VAL(expr, err);
   return _eval_type(expr, type);
}

static void
_append_char_escaped(Eina_Strbuf *buf, char c)
{
   switch (c)
     {
      case '\'': eina_strbuf_append(buf, "\\\'"); break;
      case '\"': eina_strbuf_append(buf, "\\\""); break;
      case '\?': eina_strbuf_append(buf, "\\\?"); break;
      case '\\': eina_strbuf_append(buf, "\\\\"); break;
      case '\a': eina_strbuf_append(buf, "\\a"); break;
      case '\b': eina_strbuf_append(buf, "\\b"); break;
      case '\f': eina_strbuf_append(buf, "\\f"); break;
      case '\n': eina_strbuf_append(buf, "\\n"); break;
      case '\r': eina_strbuf_append(buf, "\\r"); break;
      case '\t': eina_strbuf_append(buf, "\\t"); break;
      case '\v': eina_strbuf_append(buf, "\\v"); break;
      default:
         if ((c < 32) || (c > 126))
           eina_strbuf_append_printf(buf, "\\x%X", (unsigned char)c);
         else
           eina_strbuf_append_char(buf, c);
         break;
     }
}

static void
_number_to_str(const Eolian_Value *v, Eina_Strbuf *buf)
{
   switch (v->type)
     {
      case EOLIAN_EXPR_INT:
        eina_strbuf_append_printf(buf, "%d", v->value.i); break;
      case EOLIAN_EXPR_UINT:
        eina_strbuf_append_printf(buf, "%uU", v->value.u); break;
      case EOLIAN_EXPR_LONG:
        eina_strbuf_append_printf(buf, "%ldL", v->value.l); break;
      case EOLIAN_EXPR_ULONG:
        eina_strbuf_append_printf(buf, "%luUL", v->value.ul); break;
      case EOLIAN_EXPR_LLONG:
        eina_strbuf_append_printf(buf, "%ldLL", (long)v->value.ll); break;
      case EOLIAN_EXPR_ULLONG:
        eina_strbuf_append_printf(buf, "%luULL", (unsigned long)v->value.ull);
        break;
      case EOLIAN_EXPR_FLOAT:
        eina_strbuf_append_printf(buf, "%ff", v->value.f); break;
      case EOLIAN_EXPR_DOUBLE:
        eina_strbuf_append_printf(buf, "%f", v->value.d); break;
      default:
        break;
     }
}

EAPI Eina_Stringshare *
eolian_expression_value_to_literal(const Eolian_Value *val)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(val, NULL);
   switch (val->type)
     {
      case EOLIAN_EXPR_BOOL:
        return eina_stringshare_add(val->value.b ? "EINA_TRUE"
                                                 : "EINA_FALSE");
      case EOLIAN_EXPR_NULL:
        return eina_stringshare_add("NULL");
      case EOLIAN_EXPR_CHAR:
        {
           char c = val->value.c;
           Eina_Strbuf *buf = eina_strbuf_new();
           const char *ret;
           eina_strbuf_append_char(buf, '\'');
           _append_char_escaped(buf, c);
           eina_strbuf_append_char(buf, '\'');
           ret = eina_stringshare_add(eina_strbuf_string_get(buf));
           eina_strbuf_free(buf);
           return ret;
        }
      case EOLIAN_EXPR_STRING:
        {
           const char *ret;
           char *c = (char*)val->value.s;
           Eina_Strbuf *buf = eina_strbuf_new();
           eina_strbuf_append_char(buf, '\"');
           while (*c) _append_char_escaped(buf, *(c++));
           eina_strbuf_append_char(buf, '\"');
           ret = eina_stringshare_add(eina_strbuf_string_get(buf));
           eina_strbuf_free(buf);
           return ret;
        }
      case EOLIAN_EXPR_INT:
      case EOLIAN_EXPR_UINT:
      case EOLIAN_EXPR_LONG:
      case EOLIAN_EXPR_ULONG:
      case EOLIAN_EXPR_LLONG:
      case EOLIAN_EXPR_ULLONG:
      case EOLIAN_EXPR_FLOAT:
      case EOLIAN_EXPR_DOUBLE:
        {
           const char *ret;
           Eina_Strbuf *buf = eina_strbuf_new();
           _number_to_str(val, buf);
           ret = eina_stringshare_add(eina_strbuf_string_get(buf));
           eina_strbuf_free(buf);
           return ret;
        }
      default:
        return NULL;
     }
}

static const char *_binops[] = {
    "+", "-", "*", "/", "%",
    "==", "!=", ">", "<", ">=", "<=",
    "&&", "||",
    "&", "|", "^", "<<", ">>"
};

static const char *_unops[] = {
    "+", "-", "!", "~"
};

static Eina_Bool
_expr_serialize(const Eolian_Expression *expr, Eina_Strbuf *buf, Eina_Bool outer)
{
   switch (expr->type)
     {
      case EOLIAN_EXPR_UNKNOWN:
        return EINA_FALSE;
      case EOLIAN_EXPR_INT:
      case EOLIAN_EXPR_UINT:
      case EOLIAN_EXPR_LONG:
      case EOLIAN_EXPR_ULONG:
      case EOLIAN_EXPR_LLONG:
      case EOLIAN_EXPR_ULLONG:
      case EOLIAN_EXPR_FLOAT:
      case EOLIAN_EXPR_DOUBLE:
      case EOLIAN_EXPR_STRING:
      case EOLIAN_EXPR_CHAR:
      case EOLIAN_EXPR_NULL:
      case EOLIAN_EXPR_BOOL:
        {
           Eolian_Value v = eolian_expression_eval(expr, EOLIAN_MASK_ALL);
           if (!v.type)
             return EINA_FALSE;
           const char *x = eolian_expression_value_to_literal(&v);
           if (!x)
             return EINA_FALSE;
           eina_strbuf_append(buf, x);
           eina_stringshare_del(x);
           break;
        }
      case EOLIAN_EXPR_NAME:
      case EOLIAN_EXPR_ENUM:
        {
           if (expr->type == EOLIAN_EXPR_ENUM)
             eina_strbuf_append(buf, "enum ");
           eina_strbuf_append(buf, expr->value.s);
           break;
        }
      case EOLIAN_EXPR_UNARY:
        eina_strbuf_append(buf, _unops[expr->unop]);
        _expr_serialize(expr->expr, buf, EINA_FALSE);
        break;
      case EOLIAN_EXPR_BINARY:
        if (!outer)
          eina_strbuf_append_char(buf, '(');
        _expr_serialize(expr->lhs, buf, EINA_FALSE);
        eina_strbuf_append_printf(buf, " %s ", _binops[expr->binop]);
        _expr_serialize(expr->rhs, buf, EINA_FALSE);
        if (!outer)
          eina_strbuf_append_char(buf, ')');
        break;
      default:
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EAPI Eina_Stringshare *
eolian_expression_serialize(const Eolian_Expression *expr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(expr, NULL);
   Eina_Strbuf *buf = eina_strbuf_new();
   if (!_expr_serialize(expr, buf, EINA_TRUE))
     {
        eina_strbuf_free(buf);
        return NULL;
     }
   const char *ret = eina_stringshare_add(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   return ret;
}

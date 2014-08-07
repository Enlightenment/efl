#include <assert.h>

#include <Eina.h>
#include "eolian_database.h"

static Eina_Bool
node_error(const Eolian_Object *obj, const char *msg)
{
   eina_log_print(_eolian_log_dom, EINA_LOG_LEVEL_ERR, obj->file, "",
                  obj->line, "%s at column %d", msg, obj->column);
   return EINA_FALSE;
}

static void
mask_to_str(int mask, char *buf)
{
#define APPEND_TP(str) \
   { \
      if (append_sep) *(buf++) = '|'; \
      memcpy(buf, str, sizeof(str) - 1); \
      buf += sizeof(str) - 1; \
      append_sep = EINA_TRUE; \
   }

   Eina_Bool append_sep = EINA_FALSE;

   if ((mask & EOLIAN_MASK_SINT) && (mask & EOLIAN_MASK_UINT))
     APPEND_TP("integer")
   else if (mask & EOLIAN_MASK_SINT)
     APPEND_TP("signed integer")
   else if (mask & EOLIAN_MASK_UINT)
     APPEND_TP("unsigned integer")

   if (mask & EOLIAN_MASK_FLOAT)
     APPEND_TP("float")
   if (mask & EOLIAN_MASK_BOOL)
     APPEND_TP("boolean")
   if (mask & EOLIAN_MASK_STRING)
     APPEND_TP("string")
   if (mask & EOLIAN_MASK_CHAR)
     APPEND_TP("char")
   if (mask & EOLIAN_MASK_NULL)
     APPEND_TP("null")

   *buf = '\0';
#undef APPEND_TP
}

static Eina_Bool
expr_type_error(const Eolian_Expression *expr, int type, int mask)
{
   char buf[512];
   char ebuf[256];
   char tbuf[128];
   mask_to_str(mask, ebuf);
   mask_to_str(type, tbuf);
   snprintf(buf, sizeof(buf), "invalid type (given %s, expected %s)",
       tbuf, ebuf);
   return node_error((const Eolian_Object*)expr, buf);
}

static int
expr_type_to_mask(const Eolian_Expression *expr)
{
   assert(expr->type);
   switch (expr->type)
     {
      case EOLIAN_EXPR_ULLONG:
      case EOLIAN_EXPR_ULONG:
      case EOLIAN_EXPR_UINT:
        return EOLIAN_MASK_UINT;
      case EOLIAN_EXPR_LLONG:
      case EOLIAN_EXPR_LONG:
      case EOLIAN_EXPR_INT:
        return EOLIAN_MASK_SINT;
      case EOLIAN_EXPR_LDOUBLE:
      case EOLIAN_EXPR_DOUBLE:
      case EOLIAN_EXPR_FLOAT:
        return EOLIAN_MASK_FLOAT;
      case EOLIAN_EXPR_BOOL:
        return EOLIAN_MASK_BOOL;
      case EOLIAN_EXPR_STRING:
        return EOLIAN_MASK_STRING;
      case EOLIAN_EXPR_NULL:
        return EOLIAN_MASK_NULL;
      case EOLIAN_EXPR_CHAR:
        return EOLIAN_MASK_CHAR;
      default:
        return 0;
     }
   return 0;
}

static Eina_Bool
expr_type_mismatch_error(const Eolian_Expression *lhs,
                         const Eolian_Expression *rhs)
{
   char buf[512];
   char tbuf[256];
   char ebuf[256];
   mask_to_str(expr_type_to_mask(lhs), tbuf);
   mask_to_str(expr_type_to_mask(rhs), ebuf);
   snprintf(buf, sizeof(buf), "mismatched types (%s vs %s)", tbuf, ebuf);
   return node_error((const Eolian_Object*)rhs, buf);
}

static Eina_Bool
promote(Eolian_Expression *a, Eolian_Expression *b)
{
#define CONVERT_CASE(id, dtp, expr, field, fnm) \
   case EOLIAN_EXPR_##id: \
     expr->value.field = (dtp)(expr->value.fnm); break;

#define CONVERT(dtp, expr, field) \
   switch (expr->type) \
     { \
      CONVERT_CASE(LDOUBLE, dtp, expr, field, ld ) \
      CONVERT_CASE(DOUBLE , dtp, expr, field, d  ) \
      CONVERT_CASE(FLOAT  , dtp, expr, field, f  ) \
      CONVERT_CASE(ULLONG , dtp, expr, field, ull) \
      CONVERT_CASE(LLONG  , dtp, expr, field, ll ) \
      CONVERT_CASE(ULONG  , dtp, expr, field, ul ) \
      CONVERT_CASE(LONG   , dtp, expr, field, l  ) \
      CONVERT_CASE(UINT   , dtp, expr, field, u  ) \
      CONVERT_CASE(INT    , dtp, expr, field, i  ) \
      default: \
        break; \
     }

#define PROMOTE(a, b, tp, dtp, field) \
   if (a->type == tp) \
     { \
        CONVERT(dtp, b, field) \
        b->type = a->type; \
        return EINA_TRUE; \
     } \
   else if (b->type == tp) \
     { \
        CONVERT(dtp, a, field) \
        a->type = b->type; \
        return EINA_TRUE; \
     }

   assert(a->type && b->type);
   /* not a number */
   if (a->type >= EOLIAN_EXPR_STRING)
     return expr_type_error(a, expr_type_to_mask(a), EOLIAN_MASK_NUMBER);
   if (b->type >= EOLIAN_EXPR_STRING)
     return expr_type_error(b, expr_type_to_mask(b), EOLIAN_MASK_NUMBER);
   /* no need for promotion */
   if (a->type == b->type) return EINA_TRUE;
   /* if either operand is floating point, everything has to be */
   PROMOTE(a, b, EOLIAN_EXPR_LDOUBLE, long double, ld)
   PROMOTE(a, b, EOLIAN_EXPR_DOUBLE,       double, d)
   PROMOTE(a, b, EOLIAN_EXPR_FLOAT,         float, f)
   /* if either operand is unsigned with rank >= the other one, convert to
    * unsigned; if either signed operand can represent all values of the
    * other signed or unsigned operand, convert to the larger one; our
    * ordering of types already guarantees this
    */
   PROMOTE(a, b, EOLIAN_EXPR_ULLONG, unsigned long long, ull)
   PROMOTE(a, b, EOLIAN_EXPR_LLONG,           long long, ll)
   PROMOTE(a, b, EOLIAN_EXPR_ULONG,  unsigned      long, ul)
   PROMOTE(a, b, EOLIAN_EXPR_LONG,                 long, l)
   PROMOTE(a, b, EOLIAN_EXPR_UINT,   unsigned       int, u)
   PROMOTE(a, b, EOLIAN_EXPR_INT,                   int, i)
   /* it never gets here - this is just so static analyzers don't yell at me */
   return EINA_TRUE;

#undef PROMOTE
#undef CONVERT
#undef CONVERT_CASE
}

static Eina_Bool eval_exp(const Eolian_Expression *expr, Eolian_Expression_Mask mask, Eolian_Expression *out);

static Eina_Bool
eval_unary(const Eolian_Expression *expr, Eolian_Expression_Mask mask,
           Eolian_Expression *out)
{
   switch (expr->unop)
     {
      case EOLIAN_UNOP_UNP:
        {
           /* no-op, but still typecheck */
           if (!(mask & EOLIAN_MASK_SINT))
             return expr_type_error(expr, EOLIAN_MASK_SINT, mask);

           return eval_exp(expr->expr, EOLIAN_MASK_SINT, out);
        }
      case EOLIAN_UNOP_UNM:
        {
           Eolian_Expression exp;

           if (!(mask & EOLIAN_MASK_SINT))
             return expr_type_error(expr, EOLIAN_MASK_SINT, mask);

           if (!eval_exp(expr->expr, EOLIAN_MASK_SINT, &exp))
             return EINA_FALSE;

           switch (exp.type)
             {
              case EOLIAN_EXPR_LLONG: exp.value.ll  = -(exp.value.ll); break;
              case EOLIAN_EXPR_LONG : exp.value.l   = -(exp.value.l ); break;
              case EOLIAN_EXPR_INT  : exp.value.i   = -(exp.value.i ); break;
              default: return EINA_FALSE;
             }

           *out = exp;
           return EINA_TRUE;
        }
      case EOLIAN_UNOP_NOT:
        {
           Eolian_Expression exp;

           if (!(mask & EOLIAN_MASK_BOOL))
             return expr_type_error(expr, EOLIAN_MASK_BOOL, mask);

           if (!eval_exp(expr->expr, EOLIAN_MASK_NUMBER | EOLIAN_MASK_BOOL, &exp))
             return EINA_FALSE;

           exp.value.b = !(exp.value.ull);
           exp.type = EOLIAN_EXPR_BOOL;

           *out = exp;
           return EINA_TRUE;
        }
      case EOLIAN_UNOP_BNOT:
        {
           Eolian_Expression exp;

           if (!(mask & EOLIAN_MASK_INT))
             return expr_type_error(expr, EOLIAN_MASK_INT, mask);

           if (!eval_exp(expr->expr, EOLIAN_MASK_INT, &exp))
             return EINA_FALSE;

           switch (exp.type)
             {
              case EOLIAN_EXPR_ULLONG: exp.value.ull  = ~(exp.value.ull); break;
              case EOLIAN_EXPR_LLONG : exp.value.ll   = ~(exp.value.ll ); break;
              case EOLIAN_EXPR_ULONG : exp.value.ul   = ~(exp.value.ul ); break;
              case EOLIAN_EXPR_LONG  : exp.value.l    = ~(exp.value.l  ); break;
              case EOLIAN_EXPR_UINT  : exp.value.u    = ~(exp.value.u  ); break;
              case EOLIAN_EXPR_INT   : exp.value.i    = ~(exp.value.i  ); break;
              default: return EINA_FALSE;
             }

           *out = exp;
           return EINA_TRUE;
        }

      default:
        assert(EINA_FALSE);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
eval_promote_num(const Eolian_Expression *expr, Eolian_Expression *lhs,
                 Eolian_Expression *rhs, int mask, int emask)
{
   /* make sure the output can be a number */
   if (!(mask & EOLIAN_MASK_NUMBER))
     return expr_type_error(expr, EOLIAN_MASK_NUMBER, mask);

   /* eval into primitive value */
   if (!eval_exp(expr->lhs, emask, lhs))
     return EINA_FALSE;

   if (!eval_exp(expr->rhs, emask, rhs))
     return EINA_FALSE;

   /* promote so both sides are of the same type */
   if (!promote(lhs, rhs))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
eval_binary(const Eolian_Expression *expr, Eolian_Expression_Mask mask,
            Eolian_Expression *out)
{
#define APPLY_CASE(id, expr, lhs, rhs, fnm, op) \
   case EOLIAN_EXPR_##id: \
     expr->value.fnm = lhs.value.fnm op rhs.value.fnm; break;

#define APPLY_CASE_INT(expr, lhs, rhs, op) \
   APPLY_CASE(ULLONG, expr, lhs, rhs, ull, op) \
   APPLY_CASE(LLONG , expr, lhs, rhs, ll , op) \
   APPLY_CASE(ULONG , expr, lhs, rhs, ul , op) \
   APPLY_CASE(LONG  , expr, lhs, rhs, l  , op) \
   APPLY_CASE(UINT  , expr, lhs, rhs, u  , op) \
   APPLY_CASE(INT   , expr, lhs, rhs, i  , op)

#define APPLY_CASE_FLOAT(expr, lhs, rhs, op) \
   APPLY_CASE(LDOUBLE, expr, lhs, rhs, ld, op) \
   APPLY_CASE(DOUBLE , expr, lhs, rhs, d , op) \
   APPLY_CASE(FLOAT  , expr, lhs, rhs, f , op)

#define APPLY_NUM(expr, lhs, rhs, op) \
   expr->type = lhs.type; \
   switch (lhs.type) \
     { \
      APPLY_CASE_INT(  expr, lhs, rhs, op) \
      APPLY_CASE_FLOAT(expr, lhs, rhs, op) \
      default: \
        return EINA_FALSE; \
     }

#define APPLY_INT(expr, lhs, rhs, op) \
   expr->type = lhs.type; \
   switch (lhs.type) \
     { \
      APPLY_CASE_INT(expr, lhs, rhs, op) \
      default: \
        return EINA_FALSE; \
     }

#define CASE_ARITH(id, op) \
   case EOLIAN_BINOP_##id: \
     { \
        Eolian_Expression lhs; \
        Eolian_Expression rhs; \
        int emask = (mask & EOLIAN_MASK_FLOAT) \
                    ? EOLIAN_MASK_NUMBER \
                    : EOLIAN_MASK_INT; \
        if (!eval_promote_num(expr, &lhs, &rhs, mask, emask)) \
          return EINA_FALSE; \
        APPLY_NUM(out, lhs, rhs, op) \
        return EINA_TRUE; \
     }

#define CASE_ARITH_INT(id, op) \
   case EOLIAN_BINOP_##id: \
     { \
        Eolian_Expression lhs; \
        Eolian_Expression rhs; \
        if (!eval_promote_num(expr, &lhs, &rhs, mask, EOLIAN_MASK_INT)) \
          return EINA_FALSE; \
        APPLY_INT(out, lhs, rhs, op) \
        return EINA_TRUE; \
     }

#define CASE_COMP(id, op, allowed) \
   case EOLIAN_BINOP_##id: \
     { \
        Eolian_Expression lhs; \
        Eolian_Expression rhs; \
        if (!(mask & EOLIAN_MASK_BOOL)) \
          return expr_type_error(expr, EOLIAN_MASK_BOOL, mask); \
        if (!eval_exp(expr->lhs, allowed, &lhs)) \
          return EINA_FALSE; \
        if (!eval_exp(expr->rhs, allowed, &rhs)) \
          return EINA_FALSE; \
        if (lhs.type >= EOLIAN_EXPR_STRING && rhs.type != lhs.type) \
          return expr_type_mismatch_error(&lhs, &rhs); \
        else if (rhs.type >= EOLIAN_EXPR_STRING && rhs.type != lhs.type) \
          return expr_type_mismatch_error(&lhs, &rhs); \
        out->type    = EOLIAN_EXPR_BOOL; \
        out->value.b = lhs.value.ull op rhs.value.ull; \
        return EINA_TRUE; \
     }

   switch (expr->binop)
     {
      CASE_ARITH(ADD, +)
      CASE_ARITH(SUB, -)
      CASE_ARITH(MUL, *)
      CASE_ARITH(DIV, /)

      CASE_ARITH_INT(MOD, %)

      CASE_COMP(EQ, ==, EOLIAN_MASK_ALL)
      CASE_COMP(NQ, !=, EOLIAN_MASK_ALL)
      CASE_COMP(GT, > , EOLIAN_MASK_NUMBER)
      CASE_COMP(LT, < , EOLIAN_MASK_NUMBER)
      CASE_COMP(GE, >=, EOLIAN_MASK_NUMBER)
      CASE_COMP(LE, <=, EOLIAN_MASK_NUMBER)

      CASE_COMP(AND, &&, EOLIAN_MASK_ALL)
      CASE_COMP(OR,  ||, EOLIAN_MASK_ALL)

      CASE_ARITH_INT(BAND, &)
      CASE_ARITH_INT(BOR , |)
      CASE_ARITH_INT(BXOR, ^)
      CASE_ARITH_INT(LSH , <<)
      CASE_ARITH_INT(RSH , >>)

      default:
        assert(EINA_FALSE);
        return EINA_FALSE;
     }
   return EINA_TRUE;

#undef CASE_ARITH
#undef CASE_ARITH_INT
#undef CASE_COMP
#undef APPLY_NUM
#undef APPLY_INT
#undef APPLY_CASE_FLOAT
#undef APPLY_CASE_INT
#undef APPLY_CASE
}

static Eina_Bool
eval_exp(const Eolian_Expression *expr, Eolian_Expression_Mask mask,
         Eolian_Expression *out)
{
   switch (expr->type)
     {
      case EOLIAN_EXPR_INT:
      case EOLIAN_EXPR_LONG:
      case EOLIAN_EXPR_LLONG:
        {
           if (!(mask & EOLIAN_MASK_SINT))
             return expr_type_error(expr, EOLIAN_MASK_SINT, mask);
           *out = *expr;
           return EINA_TRUE;
        }
      case EOLIAN_EXPR_UINT:
      case EOLIAN_EXPR_ULONG:
      case EOLIAN_EXPR_ULLONG:
        {
           if (!(mask & EOLIAN_MASK_UINT))
             return expr_type_error(expr, EOLIAN_MASK_UINT, mask);
           *out = *expr;
           return EINA_TRUE;
        }
      case EOLIAN_EXPR_FLOAT:
      case EOLIAN_EXPR_DOUBLE:
      case EOLIAN_EXPR_LDOUBLE:
        {
           if (!(mask & EOLIAN_MASK_FLOAT))
             return expr_type_error(expr, EOLIAN_MASK_FLOAT, mask);
           *out = *expr;
           return EINA_TRUE;
        }
      case EOLIAN_EXPR_STRING:
        {
           if (!(mask & EOLIAN_MASK_STRING))
             return expr_type_error(expr, EOLIAN_MASK_STRING, mask);
           *out = *expr;
           return EINA_TRUE;
        }
      case EOLIAN_EXPR_NULL:
        {
           if (!(mask & EOLIAN_MASK_NULL))
             return expr_type_error(expr, EOLIAN_MASK_NULL, mask);
           *out = *expr;
           return EINA_TRUE;
        }
      case EOLIAN_EXPR_CHAR:
        {
           if (!(mask & EOLIAN_MASK_CHAR))
             return expr_type_error(expr, EOLIAN_MASK_CHAR, mask);
           *out = *expr;
           return EINA_TRUE;
        }
      case EOLIAN_EXPR_BOOL:
        {
           if (!(mask & EOLIAN_MASK_BOOL))
             return expr_type_error(expr, EOLIAN_MASK_BOOL, mask);
           *out = *expr;
           return EINA_TRUE;
        }
      case EOLIAN_EXPR_UNARY:
        return eval_unary(expr, mask, out);
      case EOLIAN_EXPR_BINARY:
        return eval_binary(expr, mask, out);
      default:
        assert(EINA_FALSE);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eolian_Expression_Type
database_expr_eval(const Eolian_Expression *expr, Eolian_Expression_Mask mask,
                   Eina_Value **outval)
{
   Eolian_Expression out;
   if (!mask)
     return EOLIAN_EXPR_UNKNOWN;
   if (!eval_exp(expr, mask, &out))
     return EOLIAN_EXPR_UNKNOWN;
   if (!outval)
     return out.type;
   switch (out.type)
     {
      case EOLIAN_EXPR_INT:
        *outval = eina_value_new(EINA_VALUE_TYPE_INT);
        eina_value_set(*outval, out.value.i);
        break;
      case EOLIAN_EXPR_UINT:
        *outval = eina_value_new(EINA_VALUE_TYPE_UINT);
        eina_value_set(*outval, out.value.u);
        break;
      case EOLIAN_EXPR_LONG:
        *outval = eina_value_new(EINA_VALUE_TYPE_LONG);
        eina_value_set(*outval, out.value.l);
        break;
      case EOLIAN_EXPR_ULONG:
        *outval = eina_value_new(EINA_VALUE_TYPE_ULONG);
        eina_value_set(*outval, out.value.ul);
        break;
      case EOLIAN_EXPR_LLONG:
        *outval = eina_value_new(EINA_VALUE_TYPE_INT64);
        eina_value_set(*outval, out.value.ll);
        break;
      case EOLIAN_EXPR_ULLONG:
        *outval = eina_value_new(EINA_VALUE_TYPE_UINT64);
        eina_value_set(*outval, out.value.ull);
        break;
      case EOLIAN_EXPR_FLOAT:
        *outval = eina_value_new(EINA_VALUE_TYPE_FLOAT);
        eina_value_set(*outval, out.value.f);
        break;
      case EOLIAN_EXPR_DOUBLE:
        *outval = eina_value_new(EINA_VALUE_TYPE_DOUBLE);
        eina_value_set(*outval, out.value.d);
        break;
      case EOLIAN_EXPR_LDOUBLE:
        *outval = eina_value_new(EINA_VALUE_TYPE_DOUBLE);
        eina_value_set(*outval, (double)out.value.ld);
        break;
      case EOLIAN_EXPR_STRING:
        *outval = eina_value_new(EINA_VALUE_TYPE_STRINGSHARE);
        eina_value_set(*outval, eina_stringshare_ref(out.value.s));
        break;
      case EOLIAN_EXPR_CHAR:
        *outval = eina_value_new(EINA_VALUE_TYPE_CHAR);
        eina_value_set(*outval, out.value.c);
        break;
      case EOLIAN_EXPR_BOOL:
        *outval = eina_value_new(EINA_VALUE_TYPE_UCHAR);
        eina_value_set(*outval, out.value.b);
        break;
      default:
        return EOLIAN_EXPR_UNKNOWN;
     }
   return out.type;
}

void
database_expr_del(Eolian_Expression *expr)
{
   if (!expr) return;
   if (expr->base.file) eina_stringshare_del(expr->base.file);
   if (expr->type == EOLIAN_EXPR_BINARY)
     {
        database_expr_del(expr->lhs);
        database_expr_del(expr->rhs);
     }
   else if (expr->type == EOLIAN_EXPR_UNARY)
     {
        database_expr_del(expr->expr);
     }
   else if (expr->type == EOLIAN_EXPR_STRING)
     {
        eina_stringshare_del(expr->value.s);
     }
   free(expr);
}

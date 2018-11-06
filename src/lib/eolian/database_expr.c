#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>

#include <Eina.h>
#include "eolian_database.h"
#include "eolian_priv.h"
#include "eo_lexer.h"

static Eina_Bool
node_error(const Eolian_Object *obj, const char *msg)
{
   eolian_state_log_obj(obj->unit->state, obj, "%s", msg);
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

static Eina_Bool
expr_error(const Eolian_Expression *expr, const char *msg)
{
   char buf[512];
   snprintf(buf, sizeof(buf), "%s '%s'", msg, expr->value.s);
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
   char buf[512 + 128];
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
      CONVERT_CASE(DOUBLE, dtp, expr, field, d  ) \
      CONVERT_CASE(FLOAT , dtp, expr, field, f  ) \
      CONVERT_CASE(ULLONG, dtp, expr, field, ull) \
      CONVERT_CASE(LLONG , dtp, expr, field, ll ) \
      CONVERT_CASE(ULONG , dtp, expr, field, ul ) \
      CONVERT_CASE(LONG  , dtp, expr, field, l  ) \
      CONVERT_CASE(UINT  , dtp, expr, field, u  ) \
      CONVERT_CASE(INT   , dtp, expr, field, i  ) \
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
   PROMOTE(a, b, EOLIAN_EXPR_DOUBLE, double, d)
   PROMOTE(a, b, EOLIAN_EXPR_FLOAT,   float, f)
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

static Eina_Bool eval_exp(const Eolian_Unit *unit,
                          Eolian_Expression *expr,
                          Eolian_Expression_Mask mask, Eolian_Expression *out,
                          Expr_Obj_Cb cb, void *data);

static Eina_Bool
eval_unary(const Eolian_Unit *unit, Eolian_Expression *expr,
           Eolian_Expression_Mask mask, Eolian_Expression *out,
           Expr_Obj_Cb cb, void *data)
{
   switch (expr->unop)
     {
      case EOLIAN_UNOP_UNP:
        {
           /* no-op, but still typecheck */
           if (!(mask & EOLIAN_MASK_SIGNED))
             return expr_type_error(expr, EOLIAN_MASK_SIGNED, mask);

           return eval_exp(unit, expr->expr, EOLIAN_MASK_SIGNED, out, cb, data);
        }
      case EOLIAN_UNOP_UNM:
        {
           Eolian_Expression exp;

           if (!(mask & EOLIAN_MASK_SIGNED))
             return expr_type_error(expr, EOLIAN_MASK_SIGNED, mask);

           if (!eval_exp(unit, expr->expr, EOLIAN_MASK_SIGNED, &exp, cb, data))
             return EINA_FALSE;

           switch (exp.type)
             {
              case EOLIAN_EXPR_LLONG : exp.value.ll = -(exp.value.ll); break;
              case EOLIAN_EXPR_LONG  : exp.value.l  = -(exp.value.l ); break;
              case EOLIAN_EXPR_INT   : exp.value.i  = -(exp.value.i ); break;
              case EOLIAN_EXPR_FLOAT : exp.value.f  = -(exp.value.f ); break;
              case EOLIAN_EXPR_DOUBLE: exp.value.d  = -(exp.value.d ); break;
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

           if (!eval_exp(unit, expr->expr, EOLIAN_MASK_NUMBER | EOLIAN_MASK_BOOL, &exp, cb, data))
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

           if (!eval_exp(unit, expr->expr, EOLIAN_MASK_INT, &exp, cb, data))
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
eval_promote_num(const Eolian_Unit *unit, Eolian_Expression *expr,
                 Eolian_Expression *lhs, Eolian_Expression *rhs, int mask,
                 int emask, Expr_Obj_Cb cb, void *data)
{
   /* make sure the output can be a number */
   if (!(mask & EOLIAN_MASK_NUMBER))
     return expr_type_error(expr, EOLIAN_MASK_NUMBER, mask);

   /* eval into primitive value */
   if (!eval_exp(unit, expr->lhs, emask, lhs, cb, data))
     return EINA_FALSE;

   if (!eval_exp(unit, expr->rhs, emask, rhs, cb, data))
     return EINA_FALSE;

   /* promote so both sides are of the same type */
   if (!promote(lhs, rhs))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
eval_binary(const Eolian_Unit *unit, Eolian_Expression *expr,
            Eolian_Expression_Mask mask, Eolian_Expression *out,
            Expr_Obj_Cb cb, void *data)
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
   APPLY_CASE(DOUBLE, expr, lhs, rhs, d , op) \
   APPLY_CASE(FLOAT , expr, lhs, rhs, f , op)

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
        if (!eval_promote_num(unit, expr, &lhs, &rhs, mask, emask, cb, data)) \
          return EINA_FALSE; \
        APPLY_NUM(out, lhs, rhs, op) \
        return EINA_TRUE; \
     }

#define CASE_ARITH_INT(id, op) \
   case EOLIAN_BINOP_##id: \
     { \
        Eolian_Expression lhs; \
        Eolian_Expression rhs; \
        if (!eval_promote_num(unit, expr, &lhs, &rhs, mask, EOLIAN_MASK_INT, cb, data)) \
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
        if (!eval_exp(unit, expr->lhs, allowed, &lhs, cb, data)) \
          return EINA_FALSE; \
        if (!eval_exp(unit, expr->rhs, allowed, &rhs, cb, data)) \
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
split_enum_name(const char *str, char **ename, char **memb)
{
   char *fulln  = strdup(str);
   char *memb_s = strrchr(fulln, '.');
   if  (!memb_s)
     {
        free(fulln);
        return EINA_FALSE;
     }
   *(memb_s++) = '\0';
   *ename = fulln;
   *memb  = memb_s;
   return EINA_TRUE;
}

static Eina_Bool
eval_exp(const Eolian_Unit *unit, Eolian_Expression *expr,
         Eolian_Expression_Mask mask, Eolian_Expression *out,
         Expr_Obj_Cb cb, void *data)
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
        {
           if (!(mask & EOLIAN_MASK_FLOAT))
             return expr_type_error(expr, EOLIAN_MASK_FLOAT, mask);
           *out = *expr;
           return EINA_TRUE;
        }
      case EOLIAN_EXPR_STRING:
        {
           if (!(mask & EOLIAN_MASK_STRING) && !(mask & EOLIAN_MASK_NULL))
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
      case EOLIAN_EXPR_NAME:
        {
           if (!unit)
             {
                if (!expr->expr)
                  return expr_error(expr, "undefined value");
                return eval_exp(NULL, expr->expr, mask, out, cb, data);
             }

           const Eolian_Variable *var = eolian_unit_constant_by_name_get
             (unit, expr->value.s);
           Eolian_Expression *exp = NULL;

           if (!var)
             {
                const Eolian_Typedecl *etpd;
                const Eolian_Enum_Type_Field *fl;

                /* try aliases, hoping it'll be enum */
                char *fulln = NULL, *memb = NULL;

                if (!split_enum_name(expr->value.s, &fulln, &memb))
                  return expr_error(expr, "undefined variable");

                /* assert int here, as we're clearly dealing with enum */
                if (!(mask & EOLIAN_MASK_INT))
                  return expr_type_error(expr, EOLIAN_MASK_INT, mask);

                etpd = eolian_unit_alias_by_name_get(unit, fulln);
                if (etpd && cb) cb((const Eolian_Object *)etpd, data);
                while (etpd && etpd->type == EOLIAN_TYPEDECL_ALIAS)
                  {
                     const Eolian_Type *etp = eolian_typedecl_base_type_get(etpd);
                     if (!etp || etp->type != EOLIAN_TYPE_REGULAR)
                       break;
                     etpd = database_type_decl_find(unit, etp);
                  }

                if (!etpd)
                  {
                     etpd = eolian_unit_enum_by_name_get(unit, fulln);
                     if (etpd && cb) cb((const Eolian_Object *)etpd, data);
                  }
                if (!etpd || etpd->type != EOLIAN_TYPEDECL_ENUM)
                  {
                     free(fulln);
                     return expr_error(expr, "undefined variable");
                  }

                fl = eolian_typedecl_enum_field_get(etpd, memb);
                if (fl) exp = (Eolian_Expression *)eolian_typedecl_enum_field_value_get(fl, EINA_TRUE);
                free(fulln);

                if (!exp)
                  return expr_error(expr, "invalid enum field");
             }
           else
             {
                exp = (Eolian_Expression *)var->value;
                if (cb) cb((const Eolian_Object *)var, data);
             }

           if (!exp)
             return expr_error(expr, "undefined variable");

           expr->expr = exp;
           return eval_exp(unit, exp, mask, out, cb, data);
        }
      case EOLIAN_EXPR_UNARY:
        return eval_unary(unit, expr, mask, out, cb, data);
      case EOLIAN_EXPR_BINARY:
        return eval_binary(unit, expr, mask, out, cb, data);
      default:
        assert(EINA_FALSE);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eolian_Value
database_expr_eval(const Eolian_Unit *unit, Eolian_Expression *expr,
                   Eolian_Expression_Mask mask, Expr_Obj_Cb cb, void *data)
{
   Eolian_Expression out;
   Eolian_Value ret;
   ret.type = EOLIAN_EXPR_UNKNOWN;
   if (!mask)
     return ret;
   if (!eval_exp(unit, expr, mask, &out, cb, data))
     return ret;
   ret.type = out.type;
   ret.value = out.value;
   return ret;
}

Eolian_Value
database_expr_eval_type(const Eolian_Unit *unit, Eolian_Expression *expr,
                        const Eolian_Type *type, Expr_Obj_Cb cb, void *data)
{
    Eolian_Value err;
    err.type = EOLIAN_EXPR_UNKNOWN;
    if (!type)
      return err;
    switch (type->type)
      {
        case EOLIAN_TYPE_CLASS:
          return database_expr_eval(unit, expr, EOLIAN_MASK_NULL, cb, data);
        case EOLIAN_TYPE_REGULAR:
          {
              if (database_type_is_ownable(unit, type, EINA_FALSE))
                 return database_expr_eval(unit, expr, EOLIAN_MASK_NULL, cb, data);
              int  kw = eo_lexer_keyword_str_to_id(type->base.name);
              if (!kw || kw < KW_byte || kw >= KW_void)
                 {
                     const Eolian_Typedecl *base = database_type_decl_find(unit, type);
                     if (!base)
                        return err;
                     if (base->type == EOLIAN_TYPEDECL_ALIAS)
                        return database_expr_eval_type(unit, expr, eolian_typedecl_base_type_get(base), cb, data);
                     else if (base->type == EOLIAN_TYPEDECL_ENUM)
                        return database_expr_eval(unit, expr, EOLIAN_MASK_INT, cb, data);
                     return err;
                 }
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
                     return database_expr_eval(unit, expr, EOLIAN_MASK_SINT, cb, data);
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
                     return database_expr_eval(unit, expr, EOLIAN_MASK_UINT, cb, data);
                  case KW_float:
                  case KW_double:
                     return database_expr_eval(unit, expr, EOLIAN_MASK_FLOAT, cb, data);
                  case KW_bool:
                     return database_expr_eval(unit, expr, EOLIAN_MASK_BOOL, cb, data);
                  case KW_char:
                     return database_expr_eval(unit, expr, EOLIAN_MASK_CHAR, cb, data);
                  default:
                     return err;
                 }
          }
        default:
          return err;
      }
}

void
database_expr_del(Eolian_Expression *expr)
{
   if (!expr) return;
   if (expr->base.file) eina_stringshare_del(expr->base.file);
   if (expr->type == EOLIAN_EXPR_BINARY)
     {
        if (!expr->weak_lhs) database_expr_del(expr->lhs);
        if (!expr->weak_rhs) database_expr_del(expr->rhs);
     }
   else if (expr->type == EOLIAN_EXPR_UNARY)
     {
        if (!expr->weak_lhs) database_expr_del(expr->expr);
     }
   else if (expr->type == EOLIAN_EXPR_STRING)
     {
        eina_stringshare_del(expr->value.s);
     }
   free(expr);
}

#include <assert.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eo_parser.h"
#include "eolian_priv.h"

#define CASE_LOCK(ls, var, msg) \
   if (has_##var) \
     eo_lexer_syntax_error(ls, "double " msg); \
   has_##var = EINA_TRUE;

#define FILL_BASE(exp, ls, l, c, tp) \
   (exp).unit = ls->unit; \
   (exp).file = eina_stringshare_ref(ls->filename); \
   (exp).line = l; \
   (exp).column = c; \
   (exp).type = EOLIAN_OBJECT_##tp

#define FILL_DOC(ls, def, docf) \
   if (ls->t.token == TOK_DOC) \
     { \
        def->docf = ls->t.value.doc; \
        ls->t.value.doc = NULL; \
        eo_lexer_get(ls); \
     }

static void
error_expected(Eo_Lexer *ls, int token)
{
   char  buf[256 + 128];
   char tbuf[256];
   eo_lexer_token_to_str(token, tbuf);
   snprintf(buf, sizeof(buf), "'%s' expected", tbuf);
   eo_lexer_syntax_error(ls, buf);
}

static Eina_Bool
test_next(Eo_Lexer *ls, int token)
{
   if (ls->t.token == token)
     {
        eo_lexer_get(ls);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
check(Eo_Lexer *ls, int token)
{
   if (ls->t.token != token)
     error_expected(ls, token);
}

static void
check_kw(Eo_Lexer *ls, int kw)
{
   if (ls->t.kw != kw)
     error_expected(ls, TOK_VALUE + kw);
}

static void
check_next(Eo_Lexer *ls, int token)
{
   check(ls, token);
   eo_lexer_get(ls);
}

static void
check_kw_next(Eo_Lexer *ls, int kw)
{
   check_kw(ls, kw);
   eo_lexer_get(ls);
}

static void
check_match(Eo_Lexer *ls, int what, int who, int where, int col)
{
   if (!test_next(ls, what))
     {
        if (where == ls->line_number)
          error_expected(ls, what);
        else
          {
             char  buf[256 + 256 + 128];
             char tbuf[256];
             char vbuf[256];
             eo_lexer_token_to_str(what, tbuf);
             eo_lexer_token_to_str(who , vbuf);
             snprintf(buf, sizeof(buf),
                      "'%s' expected (to close '%s' at line %d, column %d)",
                      tbuf, vbuf, where, col);
             eo_lexer_syntax_error(ls, buf);
          }
     }
}

static Eina_Bool
compare_class_file(const char *fn1, const char *fn2)
{
   return !strcmp(fn1, fn2);
}


static Eolian_Object *
_eolian_decl_get(Eo_Lexer *ls, const char *name)
{
   Eolian_Object *obj = eina_hash_find(ls->state->main.unit.objects, name);
   if (!obj)
     obj = eina_hash_find(ls->state->staging.unit.objects, name);
   if (obj && ((obj->type == EOLIAN_OBJECT_CLASS) ||
               (obj->type == EOLIAN_OBJECT_TYPEDECL) ||
               (obj->type == EOLIAN_OBJECT_VARIABLE)))
     return obj;

   return NULL;
}

static const char *
_eolian_decl_name_get(Eolian_Object *obj)
{
   switch (obj->type)
     {
      case EOLIAN_OBJECT_CLASS:
        return "class";
      case EOLIAN_OBJECT_TYPEDECL:
        switch (((Eolian_Typedecl *)obj)->type)
          {
           case EOLIAN_TYPEDECL_ALIAS:
             return "type alias";
           case EOLIAN_TYPEDECL_STRUCT:
           case EOLIAN_TYPEDECL_STRUCT_OPAQUE:
             return "struct";
           case EOLIAN_TYPEDECL_ENUM:
             return "enum";
           default:
             break;
          }
        goto end;
      case EOLIAN_OBJECT_VARIABLE:
        return "variable";
      default:
        break;
     }
end:
   return "unknown";
}

static void
redef_error(Eo_Lexer *ls, Eolian_Object *obj, Eolian_Object *nobj)
{
   char buf[256 + 128], fbuf[256] = { '\0' };
   if (ls->filename != obj->file)
     snprintf(fbuf, sizeof(fbuf), "%s:%d:%d", obj->file, obj->line, obj->column);
   else
     snprintf(fbuf, sizeof(fbuf), "%d:%d", obj->line, obj->column);

   if (nobj->type != obj->type)
     snprintf(buf, sizeof(buf), "%s '%s' redefined as %s (originally at %s)",
              _eolian_decl_name_get(obj), obj->name,
              _eolian_decl_name_get(nobj), fbuf);
   else
     snprintf(buf, sizeof(buf), "%s '%s' redefined (originally at %s)",
              _eolian_decl_name_get(obj), obj->name, fbuf);

   eo_lexer_syntax_error(ls, buf);
}

static Eina_Strbuf *
parse_name(Eo_Lexer *ls, Eina_Strbuf *buf)
{
   check(ls, TOK_VALUE);
   if (eo_lexer_is_type_keyword(ls->t.kw))
     eo_lexer_syntax_error(ls, "invalid name");
   eina_strbuf_reset(buf);
   for (;;)
     {
        eina_strbuf_append(buf, ls->t.value.s);
        eo_lexer_get(ls);
        if (ls->t.token != '.') break;
        eo_lexer_get(ls);
        eina_strbuf_append(buf, ".");
        check(ls, TOK_VALUE);
        if (eo_lexer_is_type_keyword(ls->t.kw))
          eo_lexer_syntax_error(ls, "invalid name");
     }
   return buf;
}

static Eolian_Binary_Operator
get_binop_id(int tok)
{
   switch (tok)
     {
      case '+': return EOLIAN_BINOP_ADD;
      case '-': return EOLIAN_BINOP_SUB;
      case '*': return EOLIAN_BINOP_MUL;
      case '/': return EOLIAN_BINOP_DIV;
      case '%': return EOLIAN_BINOP_MOD;

      case TOK_EQ: return EOLIAN_BINOP_EQ;
      case TOK_NQ: return EOLIAN_BINOP_NQ;
      case '>'   : return EOLIAN_BINOP_GT;
      case '<'   : return EOLIAN_BINOP_LT;
      case TOK_GE: return EOLIAN_BINOP_GE;
      case TOK_LE: return EOLIAN_BINOP_LE;

      case TOK_AND: return EOLIAN_BINOP_AND;
      case TOK_OR : return EOLIAN_BINOP_OR;

      case '&': return EOLIAN_BINOP_BAND;
      case '|': return EOLIAN_BINOP_BOR;
      case '^': return EOLIAN_BINOP_BXOR;

      case TOK_LSH: return EOLIAN_BINOP_LSH;
      case TOK_RSH: return EOLIAN_BINOP_RSH;

      default: return EOLIAN_BINOP_INVALID;
     }
}

static Eolian_Unary_Operator
get_unop_id(int tok)
{
   switch (tok)
     {
      case '-': return EOLIAN_UNOP_UNM;
      case '+': return EOLIAN_UNOP_UNP;
      case '!': return EOLIAN_UNOP_NOT;
      case '~': return EOLIAN_UNOP_BNOT;

      default: return EOLIAN_UNOP_INVALID;
     }
}

static const int binprec[] = {
   -1, /* invalid */

   8, /* + */
   8, /* - */
   9, /* * */
   9, /* / */
   9, /* % */

   3, /* == */
   3, /* != */
   3, /* >  */
   3, /* <  */
   3, /* >= */
   3, /* <= */

   2, /* && */
   1, /* || */

   6, /* &  */
   4, /* |  */
   5, /* ^  */
   7, /* << */
   7  /* >> */
};

#define UNARY_PRECEDENCE 10

static Eolian_Expression *parse_expr_bin(Eo_Lexer *ls, int min_prec);
static Eolian_Expression *parse_expr(Eo_Lexer *ls);

static Eolian_Expression *
parse_expr_simple(Eo_Lexer *ls)
{
   Eolian_Expression *expr;
   Eolian_Unary_Operator unop = get_unop_id(ls->t.token);
   if (unop != EOLIAN_UNOP_INVALID)
     {
        int line = ls->line_number, col = ls->column;
        eo_lexer_get(ls);
        Eolian_Expression *exp = parse_expr_bin(ls, UNARY_PRECEDENCE);
        expr = eo_lexer_expr_new(ls);
        FILL_BASE(expr->base, ls, line, col, EXPRESSION);
        expr->unop = unop;
        expr->type = EOLIAN_EXPR_UNARY;
        expr->expr = exp;
        eo_lexer_expr_release_ref(ls, exp);
        return expr;
     }
   switch (ls->t.token)
     {
      case TOK_NUMBER:
        {
           int line = ls->line_number, col = ls->column;
           expr = eo_lexer_expr_new(ls);
           FILL_BASE(expr->base, ls, line, col, EXPRESSION);
           expr->type = ls->t.kw + 1; /* map Numbers from lexer to expr type */
           memcpy(&expr->value, &ls->t.value, sizeof(expr->value));
           eo_lexer_get(ls);
           break;
        }
      case TOK_STRING:
        {
           int line = ls->line_number, col = ls->column;
           expr = eo_lexer_expr_new(ls);
           FILL_BASE(expr->base, ls, line, col, EXPRESSION);
           expr->type = EOLIAN_EXPR_STRING;
           expr->value.s = eina_stringshare_ref(ls->t.value.s);
           eo_lexer_get(ls);
           break;
        }
      case TOK_CHAR:
        {
           int line = ls->line_number, col = ls->column;
           expr = eo_lexer_expr_new(ls);
           FILL_BASE(expr->base, ls, line, col, EXPRESSION);
           expr->type = EOLIAN_EXPR_CHAR;
           expr->value.c = ls->t.value.c;
           eo_lexer_get(ls);
           break;
        }
      case TOK_VALUE:
        {
           int line = ls->line_number, col = ls->column;
           switch (ls->t.kw)
             {
              case KW_true:
              case KW_false:
                {
                   expr = eo_lexer_expr_new(ls);
                   expr->type = EOLIAN_EXPR_BOOL;
                   expr->value.b = (ls->t.kw == KW_true);
                   eo_lexer_get(ls);
                   break;
                }
              case KW_null:
                {
                   expr = eo_lexer_expr_new(ls);
                   expr->type = EOLIAN_EXPR_NULL;
                   eo_lexer_get(ls);
                   break;
                }
              default:
                {
                   Eina_Strbuf *buf = eina_strbuf_new();
                   eo_lexer_dtor_push(ls, EINA_FREE_CB(eina_strbuf_free), buf);
                   expr = eo_lexer_expr_new(ls);
                   expr->type = EOLIAN_EXPR_NAME;
                   parse_name(ls, buf);
                   expr->value.s = eina_stringshare_add(eina_strbuf_string_get
                       (buf));
                   eo_lexer_dtor_pop(ls);
                   break;
                }
             }
           FILL_BASE(expr->base, ls, line, col, EXPRESSION);
           break;
        }
      case '(':
        {
           int line = ls->line_number, col = ls->column;
           eo_lexer_get(ls);
           expr = parse_expr(ls);
           check_match(ls, ')', '(', line, col);
           break;
        }
      default:
        expr = NULL; /* shut up compiler */
        eo_lexer_syntax_error(ls, "unexpected symbol");
        break;
     }

   return expr;
}

static Eolian_Expression *
parse_expr_bin(Eo_Lexer *ls, int min_prec)
{
   int line = ls->line_number, col = ls->column;
   Eolian_Expression *lhs = parse_expr_simple(ls);
   for (;;)
     {
        Eolian_Expression *rhs, *bin;
        Eolian_Binary_Operator op = get_binop_id(ls->t.token);
        int prec = binprec[op];
        if ((op == EOLIAN_BINOP_INVALID) || (prec < 0) || (prec < min_prec))
          break;
        eo_lexer_get(ls);
        rhs = parse_expr_bin(ls, prec + 1);
        bin = eo_lexer_expr_new(ls);
        FILL_BASE(bin->base, ls, line, col, EXPRESSION);
        bin->binop = op;
        bin->type = EOLIAN_EXPR_BINARY;
        bin->lhs = lhs;
        eo_lexer_expr_release_ref(ls, lhs);
        bin->rhs = rhs;
        eo_lexer_expr_release_ref(ls, rhs);
        lhs = bin;
     }
   return lhs;
}

static Eolian_Expression *
parse_expr(Eo_Lexer *ls)
{
   return parse_expr_bin(ls, 1);
}

static Eolian_Type *parse_type_void(Eo_Lexer *ls);

static Eolian_Type *
parse_type(Eo_Lexer *ls)
{
   Eolian_Type *ret;
   eo_lexer_context_push(ls);
   ret = parse_type_void(ls);
   if (ret->type == EOLIAN_TYPE_VOID)
     {
        eo_lexer_context_restore(ls);
        eo_lexer_syntax_error(ls, "non-void type expected");
     }
   eo_lexer_context_pop(ls);
   return ret;
}

static void
_struct_field_free(Eolian_Struct_Type_Field *def)
{
   eina_stringshare_del(def->base.file);
   eina_stringshare_del(def->base.name);
   database_type_del(def->type);
   database_doc_del(def->doc);
   free(def);
}

static Eolian_Typedecl *
parse_struct(Eo_Lexer *ls, const char *name, Eina_Bool is_extern,
             int line, int column, const char *freefunc)
{
   int bline = ls->line_number, bcolumn = ls->column;
   Eolian_Typedecl *def = eo_lexer_typedecl_new(ls);
   def->is_extern = is_extern;
   def->base.name = name;
   def->type = EOLIAN_TYPEDECL_STRUCT;
   def->fields = eina_hash_string_small_new(EINA_FREE_CB(_struct_field_free));
   if (freefunc)
     {
        def->freefunc = eina_stringshare_ref(freefunc);
        eo_lexer_dtor_pop(ls);
     }
   check_next(ls, '{');
   FILL_DOC(ls, def, doc);
   while (ls->t.token != '}')
     {
        const char *fname;
        Eolian_Struct_Type_Field *fdef;
        Eolian_Type *tp;
        int fline = ls->line_number, fcol = ls->column;
        check(ls, TOK_VALUE);
        if (eina_hash_find(def->fields, ls->t.value.s))
          eo_lexer_syntax_error(ls, "double field definition");
        fdef = calloc(1, sizeof(Eolian_Struct_Type_Field));
        fname = eina_stringshare_ref(ls->t.value.s);
        eina_hash_add(def->fields, fname, fdef);
        def->field_list = eina_list_append(def->field_list, fdef);
        eolian_object_ref(&fdef->base);
        eo_lexer_get(ls);
        check_next(ls, ':');
        tp = parse_type(ls);
        FILL_BASE(fdef->base, ls, fline, fcol, STRUCT_FIELD);
        fdef->type = eo_lexer_type_release(ls, tp);
        fdef->base.name = eina_stringshare_ref(fname);
        if ((fdef->type->owned = (ls->t.kw == KW_at_owned)))
          eo_lexer_get(ls);
        check_next(ls, ';');
        FILL_DOC(ls, fdef, doc);
     }
   check_match(ls, '}', '{', bline, bcolumn);
   FILL_BASE(def->base, ls, line, column, TYPEDECL);
   database_struct_add(ls->unit, eo_lexer_typedecl_release(ls, def));
   return def;
}

static void
_enum_field_free(Eolian_Enum_Type_Field *def)
{
   eina_stringshare_del(def->base.file);
   eina_stringshare_del(def->base.name);
   database_expr_del(def->value);
   database_doc_del(def->doc);
   free(def);
}

static Eolian_Typedecl *
parse_enum(Eo_Lexer *ls, const char *name, Eina_Bool is_extern,
           int line, int column)
{
   int bline = ls->line_number, bcolumn = ls->column;
   Eolian_Typedecl *def = eo_lexer_typedecl_new(ls);
   def->is_extern = is_extern;
   def->base.name = name;
   def->type = EOLIAN_TYPEDECL_ENUM;
   def->fields = eina_hash_string_small_new(EINA_FREE_CB(_enum_field_free));
   check_next(ls, '{');
   FILL_DOC(ls, def, doc);
   if (ls->t.token == TOK_VALUE && ls->t.kw == KW_legacy)
     {
         if (eo_lexer_lookahead(ls) == ':')
           {
              /* consume keyword */
              eo_lexer_get(ls);
              /* consume colon */
              eo_lexer_get(ls);
              check(ls, TOK_VALUE);
              def->legacy = eina_stringshare_ref(ls->t.value.s);
              eo_lexer_get(ls);
              check_next(ls, ';');
           }
     }
   Eolian_Enum_Type_Field *prev_fl = NULL;
   int fl_nadd = 0;
   for (;;)
     {
        const char *fname;
        Eolian_Enum_Type_Field *fdef;
        int fline = ls->line_number, fcol = ls->column;
        check(ls, TOK_VALUE);
        if (eina_hash_find(def->fields, ls->t.value.s))
          eo_lexer_syntax_error(ls, "double field definition");
        fdef = calloc(1, sizeof(Eolian_Enum_Type_Field));
        fname = eina_stringshare_ref(ls->t.value.s);
        eina_hash_add(def->fields, fname, fdef);
        def->field_list = eina_list_append(def->field_list, fdef);
        eolian_object_ref(&fdef->base);
        eo_lexer_get(ls);
        FILL_BASE(fdef->base, ls, fline, fcol, ENUM_FIELD);
        fdef->base_enum = def;
        fdef->base.name = eina_stringshare_ref(fname);
        if (ls->t.token != '=')
          {
             if (!prev_fl)
               {
                  Eolian_Expression *eexp = eo_lexer_expr_new(ls);
                  FILL_BASE(eexp->base, ls, -1, -1, EXPRESSION);
                  eexp->type = EOLIAN_EXPR_INT;
                  eexp->value.i = 0;
                  fdef->value = eexp;
                  fdef->is_public_value = EINA_TRUE;
                  eo_lexer_expr_release_ref(ls, eexp);
                  prev_fl = fdef;
                  fl_nadd = 0;
               }
             else
               {
                  Eolian_Expression *rhs = eo_lexer_expr_new(ls),
                                    *bin = eo_lexer_expr_new(ls);
                  FILL_BASE(rhs->base, ls, -1, -1, EXPRESSION);
                  FILL_BASE(bin->base, ls, -1, -1, EXPRESSION);

                  rhs->type = EOLIAN_EXPR_INT;
                  rhs->value.i = ++fl_nadd;

                  bin->type = EOLIAN_EXPR_BINARY;
                  bin->binop = EOLIAN_BINOP_ADD;
                  bin->lhs = prev_fl->value;
                  bin->rhs = rhs;
                  bin->weak_lhs = EINA_TRUE;
                  eo_lexer_expr_release_ref(ls, rhs);

                  fdef->value = bin;
                  eo_lexer_expr_release_ref(ls, bin);
               }
          }
        else
          {
             ls->expr_mode = EINA_TRUE;
             eo_lexer_get(ls);
             fdef->value = parse_expr(ls);
             fdef->is_public_value = EINA_TRUE;
             ls->expr_mode = EINA_FALSE;
             prev_fl = fdef;
             fl_nadd = 0;
             eo_lexer_expr_release_ref(ls, fdef->value);
          }
        Eina_Bool want_next = (ls->t.token == ',');
        if (want_next)
          eo_lexer_get(ls);
        FILL_DOC(ls, fdef, doc);
        if (!want_next || ls->t.token == '}')
          break;
     }
   check_match(ls, '}', '{', bline, bcolumn);
   FILL_BASE(def->base, ls, line, column, TYPEDECL);
   database_enum_add(ls->unit, eo_lexer_typedecl_release(ls, def));
   return def;
}

static void
parse_struct_attrs(Eo_Lexer *ls, Eina_Bool is_enum, Eina_Bool *is_extern,
                   const char **freefunc)
{
   Eina_Bool has_extern = EINA_FALSE, has_free = EINA_FALSE;
   *freefunc = NULL;
   *is_extern = EINA_FALSE;
   for (;;) switch (ls->t.kw)
     {
      case KW_at_extern:
        CASE_LOCK(ls, extern, "@extern qualifier")
        eo_lexer_get(ls);
        *is_extern = EINA_TRUE;
        break;
      case KW_at_free:
        {
           CASE_LOCK(ls, free, "@free qualifier")
           if (is_enum)
             eo_lexer_syntax_error(ls, "enums cannot have @free");
           eo_lexer_get(ls);
           int pline = ls->line_number, pcol = ls->column;
           check_next(ls, '(');
           check(ls, TOK_VALUE);
           *freefunc = eina_stringshare_add(ls->t.value.s);
           eo_lexer_dtor_push(ls, EINA_FREE_CB(eina_stringshare_del), (void *)*freefunc);
           eo_lexer_get(ls);
           check_match(ls, ')', '(', pline, pcol);
           break;
        }
      default:
        return;
     }
}

static Eolian_Type *
parse_type_void(Eo_Lexer *ls)
{
   Eolian_Type *def;
   Eina_Strbuf *buf;
   int line = ls->line_number, col = ls->column;
   switch (ls->t.kw)
     {
      case KW_const:
        {
           int pline, pcol;
           eo_lexer_get(ls);
           pline = ls->line_number;
           pcol = ls->column;
           check_next(ls, '(');
           def = parse_type_void(ls);
           FILL_BASE(def->base, ls, line, col, TYPE);
           def->is_const = EINA_TRUE;
           check_match(ls, ')', '(', pline, pcol);
           return def;
        }
      case KW_ptr:
        {
           int pline, pcol;
           eo_lexer_get(ls);
           pline = ls->line_number;
           pcol = ls->column;
           check_next(ls, '(');
           def = parse_type_void(ls);
           FILL_BASE(def->base, ls, line, col, TYPE);
           def->is_ptr = EINA_TRUE;
           check_match(ls, ')', '(', pline, pcol);
           return def;
        }
      case KW_legacy:
        {
           int pline, pcol;
           eo_lexer_get(ls);
           pline = ls->line_number;
           pcol = ls->column;
           check_next(ls, '(');
           def = parse_type_void(ls);
           FILL_BASE(def->base, ls, line, col, TYPE);
           def->legacy = EINA_TRUE;
           check_match(ls, ')', '(', pline, pcol);
           return def;
        }
      case KW_free:
        {
           int pline, pcolumn;
           eo_lexer_get(ls);
           pline = ls->line_number;
           pcolumn = ls->column;
           check_next(ls, '(');
           def = parse_type_void(ls);
           check_next(ls, ',');
           check(ls, TOK_VALUE);
           def->freefunc = eina_stringshare_ref(ls->t.value.s);
           eo_lexer_get(ls);
           FILL_BASE(def->base, ls, line, col, TYPE);
           check_match(ls, ')', '(', pline, pcolumn);
           return def;
        }
      default:
        break;
     }
   def = eo_lexer_type_new(ls);
   FILL_BASE(def->base, ls, line, col, TYPE);
   if (ls->t.kw == KW_void)
     {
        def->type = EOLIAN_TYPE_VOID;
        def->btype = EOLIAN_TYPE_BUILTIN_VOID;
        eo_lexer_get(ls);
     }
   else if (ls->t.kw == KW___undefined_type)
     {
        def->type = EOLIAN_TYPE_UNDEFINED;
        eo_lexer_get(ls);
     }
   else
     {
        int tpid = ls->t.kw;
        def->type = EOLIAN_TYPE_REGULAR;
        check(ls, TOK_VALUE);
        if (eo_lexer_is_type_keyword(ls->t.kw))
          {
             def->btype = ls->t.kw - KW_byte + 1;
             def->base.name = eina_stringshare_ref(ls->t.value.s);
             eo_lexer_get(ls);
             if (tpid >= KW_accessor && tpid <= KW_inlist)
               {
                  int bline = ls->line_number, bcol = ls->column;
                  check_next(ls, '<');
                  if (tpid == KW_future)
                    def->base_type = eo_lexer_type_release(ls, parse_type_void(ls));
                  else
                    def->base_type = eo_lexer_type_release(ls, parse_type(ls));
                  if ((def->base_type->owned = (ls->t.kw == KW_at_owned)))
                    eo_lexer_get(ls);
                  if (tpid == KW_hash)
                    {
                       check_next(ls, ',');
                       def->base_type->next_type =
                         eo_lexer_type_release(ls, parse_type(ls));
                       if ((def->base_type->next_type->owned = (ls->t.kw == KW_at_owned)))
                         eo_lexer_get(ls);
                    }
                  check_match(ls, '>', '<', bline, bcol);
               }
          }
        else
          {
             const char *bnm, *nm;
             char *fnm;
             buf = eina_strbuf_new();
             eo_lexer_dtor_push(ls, EINA_FREE_CB(eina_strbuf_free), buf);
             eo_lexer_context_push(ls);
             parse_name(ls, buf);
             nm = eina_strbuf_string_get(buf);
             bnm = eina_stringshare_ref(ls->filename);
             fnm = database_class_to_filename(nm);
             if (!compare_class_file(bnm, fnm))
               {
                  eina_stringshare_del(bnm);
                  if (eina_hash_find(ls->state->filenames_eo, fnm))
                    {
                       database_defer(ls->state, fnm, EINA_TRUE);
                       def->type = EOLIAN_TYPE_CLASS;
                    }
                  free(fnm);
               }
             else
               {
                  eina_stringshare_del(bnm);
                  free(fnm);
                  def->type = EOLIAN_TYPE_CLASS;
               }
             def->base.name = eina_stringshare_add(nm);
             eo_lexer_context_pop(ls);
             eo_lexer_dtor_pop(ls);
          }
     }
   return def;
}

static Eolian_Typedecl *
parse_typedef(Eo_Lexer *ls)
{
   Eolian_Typedecl *def = eo_lexer_typedecl_new(ls);
   Eina_Bool has_extern;
   const char *freefunc;
   Eina_Strbuf *buf;
   eo_lexer_get(ls);
   parse_struct_attrs(ls, EINA_FALSE, &has_extern, &freefunc);
   if (freefunc)
     {
        def->freefunc = eina_stringshare_ref(freefunc);
        eo_lexer_dtor_pop(ls);
     }
   def->type = EOLIAN_TYPEDECL_ALIAS;
   def->is_extern = has_extern;
   buf = eina_strbuf_new();
   eo_lexer_dtor_push(ls, EINA_FREE_CB(eina_strbuf_free), buf);
   eo_lexer_context_push(ls);
   FILL_BASE(def->base, ls, ls->line_number, ls->column, TYPEDECL);
   parse_name(ls, buf);
   def->base.name = eina_stringshare_add(eina_strbuf_string_get(buf));
   Eolian_Object *decl = _eolian_decl_get(ls, def->base.name);
   if (decl)
     {
        eo_lexer_context_restore(ls);
        redef_error(ls, decl, &def->base);
     }
   eo_lexer_context_pop(ls);
   check_next(ls, ':');
   def->base_type = eo_lexer_type_release(ls, parse_type(ls));
   check_next(ls, ';');
   FILL_DOC(ls, def, doc);
   eo_lexer_dtor_pop(ls);
   return def;
}

static Eolian_Variable *
parse_variable(Eo_Lexer *ls, Eina_Bool global)
{
   Eolian_Variable *def = eo_lexer_variable_new(ls);
   Eina_Strbuf *buf;
   eo_lexer_get(ls);
   if (ls->t.kw == KW_at_extern)
     {
        def->is_extern = EINA_TRUE;
        eo_lexer_get(ls);
     }
   def->type = global ? EOLIAN_VAR_GLOBAL : EOLIAN_VAR_CONSTANT;
   buf = eina_strbuf_new();
   eo_lexer_dtor_push(ls, EINA_FREE_CB(eina_strbuf_free), buf);
   eo_lexer_context_push(ls);
   FILL_BASE(def->base, ls, ls->line_number, ls->column, VARIABLE);
   parse_name(ls, buf);
   def->base.name = eina_stringshare_add(eina_strbuf_string_get(buf));
   Eolian_Object *decl = _eolian_decl_get(ls, def->base.name);
   if (decl)
     {
        eo_lexer_context_restore(ls);
        redef_error(ls, decl, &def->base);
     }
   eo_lexer_context_pop(ls);
   check_next(ls, ':');
   def->base_type = eo_lexer_type_release(ls, parse_type(ls));
   /* constants are required to have a value */
   if (!global)
     check(ls, '=');
   /* globals can optionally have a value */
   if (ls->t.token == '=')
     {
        ls->expr_mode = EINA_TRUE;
        eo_lexer_get(ls);
        def->value = parse_expr(ls);
        ls->expr_mode = EINA_FALSE;
        eo_lexer_expr_release_ref(ls, def->value);
     }
   check_next(ls, ';');
   FILL_DOC(ls, def, doc);
   eo_lexer_dtor_pop(ls);
   return def;
}

typedef struct _Eo_Ret_Def
{
   Eolian_Type *type;
   Eolian_Documentation *doc;
   Eolian_Expression *default_ret_val;
   Eina_Bool warn_unused: 1;
   Eina_Bool owned: 1;
} Eo_Ret_Def;

static void
parse_return(Eo_Lexer *ls, Eo_Ret_Def *ret, Eina_Bool allow_void,
             Eina_Bool allow_def, Eina_Bool is_funcptr)
{
   eo_lexer_get(ls);
   check_next(ls, ':');
   if (allow_void)
     ret->type = parse_type_void(ls);
   else
     ret->type = parse_type(ls);
   ret->doc = NULL;
   ret->default_ret_val = NULL;
   ret->warn_unused = EINA_FALSE;
   ret->owned = EINA_FALSE;
   if (allow_def && (ls->t.token == '('))
     {
        int line = ls->line_number, col = ls->column;
        ls->expr_mode = EINA_TRUE;
        eo_lexer_get(ls);
        ret->default_ret_val = parse_expr(ls);
        ls->expr_mode = EINA_FALSE;
        check_match(ls, ')', '(', line, col);
     }
   Eina_Bool has_warn_unused = EINA_FALSE, has_owned = EINA_FALSE;
   if (!is_funcptr) for (;;) switch (ls->t.kw)
     {
      case KW_at_warn_unused:
        CASE_LOCK(ls, warn_unused, "warn_unused qualifier");
        ret->warn_unused = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_owned:
        CASE_LOCK(ls, owned, "owned qualifier");
        ret->owned = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      default:
        goto end;
     }
end:
   check_next(ls, ';');
   FILL_DOC(ls, ret, doc);
}

static void
parse_param(Eo_Lexer *ls, Eina_List **params, Eina_Bool allow_inout,
            Eina_Bool is_vals)
{
   Eina_Bool has_nonull   = EINA_FALSE, has_optional = EINA_FALSE,
             has_nullable = EINA_FALSE, has_owned    = EINA_FALSE;
   Eina_Bool cref = (ls->t.kw == KW_at_cref);
   Eolian_Function_Parameter *par = calloc(1, sizeof(Eolian_Function_Parameter));
   par->param_dir = EOLIAN_IN_PARAM;
   FILL_BASE(par->base, ls, ls->line_number, ls->column, FUNCTION_PARAMETER);
   *params = eina_list_append(*params, par);
   eolian_object_ref(&par->base);
   if (cref || (allow_inout && (ls->t.kw == KW_at_in)))
     {
        par->param_dir = EOLIAN_IN_PARAM;
        eo_lexer_get(ls);
     }
   else if (allow_inout && ls->t.kw == KW_at_out)
     {
        par->param_dir = EOLIAN_OUT_PARAM;
        eo_lexer_get(ls);
     }
   else if (allow_inout && ls->t.kw == KW_at_inout)
     {
        par->param_dir = EOLIAN_INOUT_PARAM;
        eo_lexer_get(ls);
     }
   else par->param_dir = EOLIAN_IN_PARAM;
   check(ls, TOK_VALUE);
   par->base.name = eina_stringshare_ref(ls->t.value.s);
   eo_lexer_get(ls);
   check_next(ls, ':');
   if (par->param_dir == EOLIAN_OUT_PARAM || par->param_dir == EOLIAN_INOUT_PARAM)
     par->type = eo_lexer_type_release(ls, parse_type_void(ls));
   else
     par->type = eo_lexer_type_release(ls, parse_type(ls));
   if ((is_vals || (par->param_dir == EOLIAN_OUT_PARAM)) && (ls->t.token == '('))
     {
        int line = ls->line_number, col = ls->column;
        ls->expr_mode = EINA_TRUE;
        eo_lexer_get(ls);
        par->value = parse_expr(ls);
        ls->expr_mode = EINA_FALSE;
        eo_lexer_expr_release_ref(ls, par->value);
        check_match(ls, ')', '(', line, col);
     }
   if (cref)
     {
        par->type->is_const = EINA_TRUE;
        par->type->is_ptr = EINA_TRUE;
     }
   for (;;) switch (ls->t.kw)
     {
      case KW_at_nonull:
        if (has_nullable)
          eo_lexer_syntax_error(ls, "both nullable and nonull specified");
        CASE_LOCK(ls, nonull, "nonull qualifier")
        par->nonull = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_optional:
        CASE_LOCK(ls, optional, "optional qualifier");
        par->optional = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_nullable:
        if (has_nonull)
          eo_lexer_syntax_error(ls, "both nullable and nonull specified");
        CASE_LOCK(ls, nullable, "nullable qualifier");
        par->nullable = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_owned:
        CASE_LOCK(ls, owned, "owned qualifier");
        par->type->owned = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      default:
        goto end;
     }
end:
   check_next(ls, ';');
   FILL_DOC(ls, par, doc);
}

static void
parse_legacy(Eo_Lexer *ls, const char **out)
{
   eo_lexer_get(ls);
   check_next(ls, ':');
   check(ls, TOK_VALUE);
   *out = eina_stringshare_ref(ls->t.value.s);
   eo_lexer_get(ls);
   check_next(ls, ';');
}

static void
parse_params(Eo_Lexer *ls, Eina_List **params, Eina_Bool allow_inout,
             Eina_Bool is_vals)
{
   int line, col;
   eo_lexer_get(ls);
   line = ls->line_number, col = ls->column;
   check_next(ls, '{');
   while (ls->t.token != '}')
     parse_param(ls, params, allow_inout, is_vals);
   check_match(ls, '}', '{', line, col);
}

static void
parse_accessor(Eo_Lexer *ls, Eolian_Function *prop)
{
   int line, col;
   Eina_Bool has_return = EINA_FALSE, has_legacy = EINA_FALSE,
             has_eo     = EINA_FALSE, has_keys   = EINA_FALSE,
             has_values = EINA_FALSE, has_protected = EINA_FALSE,
             has_virtp  = EINA_FALSE;
   Eina_Bool is_get = (ls->t.kw == KW_get);
   if (is_get)
     {
        if (prop->base.file)
          eina_stringshare_del(prop->base.file);
        FILL_BASE(prop->base, ls, ls->line_number, ls->column, FUNCTION);
        if (prop->type == EOLIAN_PROP_SET)
          prop->type = EOLIAN_PROPERTY;
        else
          prop->type = EOLIAN_PROP_GET;
     }
   else
     {
        FILL_BASE(prop->set_base, ls, ls->line_number, ls->column, FUNCTION);
        if (prop->type == EOLIAN_PROP_GET)
          prop->type = EOLIAN_PROPERTY;
        else
          prop->type = EOLIAN_PROP_SET;
     }
   eo_lexer_get(ls);
   for (;;) switch (ls->t.kw)
     {
      case KW_at_pure_virtual:
        CASE_LOCK(ls, virtp, "pure_virtual qualifier");
        if (is_get) prop->impl->get_pure_virtual = EINA_TRUE;
        else prop->impl->set_pure_virtual = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_protected:
        CASE_LOCK(ls, protected, "protected qualifier");
        if (is_get) prop->get_scope = EOLIAN_SCOPE_PROTECTED;
        else prop->set_scope = EOLIAN_SCOPE_PROTECTED;
        eo_lexer_get(ls);
        break;
      default:
        goto parse_accessor;
     }
parse_accessor:
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
   if ((ls->t.token == TOK_DOC) && !prop->impl->common_doc)
     {
        if (getenv("EOLIAN_PROPERTY_DOC_WARN"))
          {
             Eolian_Object tmp;
             memset(&tmp, 0, sizeof(Eolian_Object));
             tmp.file = prop->base.file;
             tmp.line = line;
             tmp.column = col;
             tmp.unit = ls->unit;
             eolian_state_log_obj(ls->state, &tmp,
                                  "%s doc without property doc for '%s.%s'",
                                  is_get ? "getter" : "setter",
                                  ls->klass->base.name, prop->base.name);
          }
     }
   if (is_get)
     {
        FILL_DOC(ls, prop->impl, get_doc);
     }
   else
     {
        FILL_DOC(ls, prop->impl, set_doc);
     }
   for (;;) switch (ls->t.kw)
     {
      case KW_return:
        CASE_LOCK(ls, return, "return")
        Eo_Ret_Def ret;
        parse_return(ls, &ret, is_get, EINA_TRUE, EINA_FALSE);
        if (ret.default_ret_val)
          eo_lexer_expr_release_ref(ls, ret.default_ret_val);
        if (is_get)
          {
             prop->get_ret_type = eo_lexer_type_release(ls, ret.type);
             prop->get_return_doc = ret.doc;
             prop->get_ret_val = ret.default_ret_val;
             prop->get_return_warn_unused = ret.warn_unused;
             prop->get_ret_type->owned = ret.owned;
          }
        else
          {
             prop->set_ret_type = eo_lexer_type_release(ls, ret.type);
             prop->set_return_doc = ret.doc;
             prop->set_ret_val = ret.default_ret_val;
             prop->set_return_warn_unused = ret.warn_unused;
             prop->set_ret_type->owned = ret.owned;
          }
        break;
      case KW_legacy:
        CASE_LOCK(ls, legacy, "legacy name")
        if (is_get)
          parse_legacy(ls, &prop->get_legacy);
        else
          parse_legacy(ls, &prop->set_legacy);
        break;
      case KW_eo:
        CASE_LOCK(ls, eo, "eo name")
        eo_lexer_get(ls);
        check_next(ls, ':');
        check_kw_next(ls, KW_null);
        check_next(ls, ';');
        if (is_get)
          prop->get_only_legacy = EINA_TRUE;
        else
          prop->set_only_legacy = EINA_TRUE;
        break;
      case KW_keys:
        {
           Eina_List **stor;
           CASE_LOCK(ls, keys, "keys definition")
           stor = is_get ? &prop->prop_keys_get : &prop->prop_keys_set;
           parse_params(ls, stor, EINA_FALSE, EINA_FALSE);
           break;
        }
      case KW_values:
        {
           Eina_List **stor;
           CASE_LOCK(ls, values, "values definition")
           stor = is_get ? &prop->prop_values_get : &prop->prop_values_set;
           parse_params(ls, stor, EINA_FALSE, EINA_TRUE);
           break;
        }
      default:
        goto end;
     }
end:
   check_match(ls, '}', '{', line, col);
}

static void
_func_pure_virtual_set(Eo_Lexer *ls, Eolian_Function *foo_id, Eina_Bool virt)
{
   if (ls->klass->type != EOLIAN_CLASS_INTERFACE && !virt)
     return;

   if (foo_id->type == EOLIAN_PROP_GET || foo_id->type == EOLIAN_METHOD)
     foo_id->impl->get_pure_virtual = EINA_TRUE;
   else if (foo_id->type == EOLIAN_PROP_SET)
     foo_id->impl->set_pure_virtual = EINA_TRUE;
   else if (foo_id->type == EOLIAN_PROPERTY)
     foo_id->impl->get_pure_virtual = foo_id->impl->set_pure_virtual = EINA_TRUE;
}

static void
parse_property(Eo_Lexer *ls)
{
   int line, col;
   Eolian_Function *prop = NULL;
   Eolian_Implement *impl = NULL;
   Eina_Bool has_get       = EINA_FALSE, has_set    = EINA_FALSE,
             has_keys      = EINA_FALSE, has_values = EINA_FALSE,
             has_protected = EINA_FALSE, has_class  = EINA_FALSE,
             has_beta      = EINA_FALSE, has_virtp  = EINA_FALSE;
   prop = calloc(1, sizeof(Eolian_Function));
   prop->klass = ls->klass;
   prop->type = EOLIAN_UNRESOLVED;
   prop->get_scope = prop->set_scope = EOLIAN_SCOPE_PUBLIC;
   FILL_BASE(prop->base, ls, ls->line_number, ls->column, FUNCTION);
   impl = calloc(1, sizeof(Eolian_Implement));
   impl->klass = impl->implklass = ls->klass;
   impl->foo_id = prop;
   FILL_BASE(impl->base, ls, ls->line_number, ls->column, IMPLEMENT);
   prop->impl = impl;
   ls->klass->properties = eina_list_append(ls->klass->properties, prop);
   ls->klass->implements = eina_list_append(ls->klass->implements, impl);
   eolian_object_ref(&prop->base);
   eolian_object_ref(&impl->base);
   check(ls, TOK_VALUE);
   if (ls->t.kw == KW_get || ls->t.kw == KW_set)
     {
        eo_lexer_syntax_error(ls, "reserved keyword as property name");
        return;
     }
   prop->base.name = eina_stringshare_ref(ls->t.value.s);
   impl->base.name = eina_stringshare_printf("%s.%s", ls->klass->base.name, prop->base.name);
   eo_lexer_get(ls);
   for (;;) switch (ls->t.kw)
     {
      case KW_at_protected:
        CASE_LOCK(ls, protected, "protected qualifier")
        prop->get_scope = prop->set_scope = EOLIAN_SCOPE_PROTECTED;
        eo_lexer_get(ls);
        break;
      case KW_at_class:
        CASE_LOCK(ls, class, "class qualifier");
        prop->is_class = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_beta:
        CASE_LOCK(ls, beta, "beta qualifier");
        prop->is_beta = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_pure_virtual:
        CASE_LOCK(ls, virtp, "pure_virtual qualifier");
        eo_lexer_get(ls);
        break;
      default:
        goto body;
     }
body:
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
   FILL_DOC(ls, prop->impl, common_doc);
   for (;;) switch (ls->t.kw)
     {
      case KW_get:
        CASE_LOCK(ls, get, "get definition")
        impl->is_prop_get = EINA_TRUE;
        parse_accessor(ls, prop);
        break;
      case KW_set:
        CASE_LOCK(ls, set, "set definition")
        impl->is_prop_set = EINA_TRUE;
        parse_accessor(ls, prop);
        break;
      case KW_keys:
        CASE_LOCK(ls, keys, "keys definition")
        parse_params(ls, &prop->prop_keys, EINA_FALSE, EINA_FALSE);
        break;
      case KW_values:
        CASE_LOCK(ls, values, "values definition")
        parse_params(ls, &prop->prop_values, EINA_FALSE, EINA_TRUE);
        break;
      default:
        goto end;
     }
end:
   check_match(ls, '}', '{', line, col);
   if (!has_get && !has_set)
     {
        prop->type = EOLIAN_PROPERTY;
        impl->is_prop_get = impl->is_prop_set = EINA_TRUE;
     }
   _func_pure_virtual_set(ls, prop, has_virtp);
}

static Eolian_Typedecl*
parse_function_pointer(Eo_Lexer *ls)
{
   int bline, bcol;
   int line = ls->line_number, col = ls->column;

   Eolian_Typedecl *def = eo_lexer_typedecl_new(ls);
   Eina_Strbuf *buf = eina_strbuf_new();
   eo_lexer_dtor_push(ls, EINA_FREE_CB(eina_strbuf_free), buf);
   Eolian_Function *meth = NULL;

   Eina_Bool has_params = EINA_FALSE,
             has_return = EINA_FALSE;

   eo_lexer_get(ls);

   def->type = EOLIAN_TYPEDECL_FUNCTION_POINTER;
   def->is_extern = (ls->t.kw == KW_at_extern);
   if (def->is_extern)
     eo_lexer_get(ls);

   parse_name(ls, buf);
   def->base.name = eina_stringshare_add(eina_strbuf_string_get(buf));
   eo_lexer_dtor_pop(ls);

   meth = calloc(1, sizeof(Eolian_Function));
   meth->klass = NULL;
   meth->type = EOLIAN_FUNCTION_POINTER;
   meth->get_scope = meth->set_scope = EOLIAN_SCOPE_PUBLIC;
   meth->base.name = eina_stringshare_add(eolian_object_short_name_get(&def->base));

   def->function_pointer = meth;
   eolian_object_ref(&meth->base);

   meth->is_beta = (ls->t.kw == KW_at_beta);
   if (meth->is_beta)
     eo_lexer_get(ls);

   bline = ls->line_number;
   bcol = ls->column;
   check_next(ls, '{');
   FILL_DOC(ls, def, doc);
   for (;;) switch (ls->t.kw)
     {
      case KW_return:
        CASE_LOCK(ls, return, "return");
        Eo_Ret_Def ret;
        parse_return(ls, &ret, EINA_FALSE, EINA_FALSE, EINA_TRUE);
        meth->get_ret_type = eo_lexer_type_release(ls, ret.type);
        meth->get_return_doc = ret.doc;
        meth->get_ret_val = NULL;
        meth->get_return_warn_unused = EINA_FALSE;
        break;
      case KW_params:
        CASE_LOCK(ls, params, "params definition");
        parse_params(ls, &meth->params, EINA_TRUE, EINA_FALSE);
        break;
      default:
        goto end;
     }
end:
   check_match(ls, '}', '{', bline, bcol);
   check_next(ls, ';');
   FILL_BASE(def->base, ls, line, col, TYPEDECL);
   FILL_BASE(meth->base, ls, line, col, FUNCTION);
   return def;
}

static void
parse_method(Eo_Lexer *ls)
{
   int line, col;
   Eolian_Function *meth = NULL;
   Eolian_Implement *impl = NULL;
   Eina_Bool has_const       = EINA_FALSE, has_params = EINA_FALSE,
             has_return      = EINA_FALSE, has_legacy = EINA_FALSE,
             has_protected   = EINA_FALSE, has_class  = EINA_FALSE,
             has_eo          = EINA_FALSE, has_beta   = EINA_FALSE,
             has_virtp       = EINA_FALSE;
   meth = calloc(1, sizeof(Eolian_Function));
   meth->klass = ls->klass;
   meth->type = EOLIAN_METHOD;
   meth->get_scope = meth->set_scope = EOLIAN_SCOPE_PUBLIC;
   FILL_BASE(meth->base, ls, ls->line_number, ls->column, FUNCTION);
   impl = calloc(1, sizeof(Eolian_Implement));
   impl->klass = impl->implklass = ls->klass;
   impl->foo_id = meth;
   FILL_BASE(impl->base, ls, ls->line_number, ls->column, IMPLEMENT);
   meth->impl = impl;
   ls->klass->methods = eina_list_append(ls->klass->methods, meth);
   ls->klass->implements = eina_list_append(ls->klass->implements, impl);
   eolian_object_ref(&meth->base);
   eolian_object_ref(&impl->base);
   check(ls, TOK_VALUE);
   if (ls->t.kw == KW_get || ls->t.kw == KW_set)
     {
        eo_lexer_syntax_error(ls, "reserved keyword as method name");
        return;
     }
   meth->base.name = eina_stringshare_ref(ls->t.value.s);
   impl->base.name = eina_stringshare_printf("%s.%s", ls->klass->base.name, meth->base.name);
   eo_lexer_get(ls);
   for (;;) switch (ls->t.kw)
     {
      case KW_at_protected:
        CASE_LOCK(ls, protected, "protected qualifier")
        meth->get_scope = meth->set_scope = EOLIAN_SCOPE_PROTECTED;
        eo_lexer_get(ls);
        break;
      case KW_at_const:
        CASE_LOCK(ls, const, "const qualifier")
        meth->obj_is_const = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_class:
        CASE_LOCK(ls, class, "class qualifier");
        meth->is_class = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_beta:
        CASE_LOCK(ls, beta, "beta qualifier");
        meth->is_beta = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_pure_virtual:
        CASE_LOCK(ls, virtp, "pure_virtual qualifier");
        eo_lexer_get(ls);
        break;
      default:
        goto body;
     }
body:
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
   FILL_DOC(ls, meth->impl, common_doc);
   for (;;) switch (ls->t.kw)
     {
      case KW_return:
        CASE_LOCK(ls, return, "return")
        Eo_Ret_Def ret;
        parse_return(ls, &ret, EINA_FALSE, EINA_TRUE, EINA_FALSE);
        if (ret.default_ret_val)
          eo_lexer_expr_release_ref(ls, ret.default_ret_val);
        meth->get_ret_type = eo_lexer_type_release(ls, ret.type);
        meth->get_return_doc = ret.doc;
        meth->get_ret_val = ret.default_ret_val;
        meth->get_return_warn_unused = ret.warn_unused;
        meth->get_ret_type->owned = ret.owned;
        break;
      case KW_legacy:
        CASE_LOCK(ls, legacy, "legacy name")
        parse_legacy(ls, &meth->get_legacy);
        break;
      case KW_eo:
        CASE_LOCK(ls, eo, "eo name")
        eo_lexer_get(ls);
        check_next(ls, ':');
        check_kw_next(ls, KW_null);
        check_next(ls, ';');
        meth->get_only_legacy = EINA_TRUE;
        break;
      case KW_params:
        CASE_LOCK(ls, params, "params definition")
        parse_params(ls, &meth->params, EINA_TRUE, EINA_FALSE);
        break;
      default:
        goto end;
     }
end:
   check_match(ls, '}', '{', line, col);
   _func_pure_virtual_set(ls, meth, has_virtp);
}

static void
parse_part(Eo_Lexer *ls)
{
   Eolian_Part *part = calloc(1, sizeof(Eolian_Part));
   FILL_BASE(part->base, ls, ls->line_number, ls->column, PART);
   ls->klass->parts = eina_list_append(ls->klass->parts, part);
   eolian_object_ref(&part->base);
   check(ls, TOK_VALUE);
   part->base.name = eina_stringshare_ref(ls->t.value.s);
   eo_lexer_get(ls);
   check_next(ls, ':');
   Eina_Strbuf *buf = eina_strbuf_new();
   eo_lexer_dtor_push(ls, EINA_FREE_CB(eina_strbuf_free), buf);
   eo_lexer_context_push(ls);
   parse_name(ls, buf);
   const char *nm = eina_strbuf_string_get(buf);
   char *fnm = database_class_to_filename(nm);
   if (!eina_hash_find(ls->state->filenames_eo, fnm))
     {
        free(fnm);
        char ebuf[PATH_MAX];
        eo_lexer_context_restore(ls);
        snprintf(ebuf, sizeof(ebuf), "unknown class '%s'", nm);
        eo_lexer_syntax_error(ls, ebuf);
        return;
     }
   database_defer(ls->state, fnm, EINA_TRUE);
   free(fnm);
   part->klass_name = eina_stringshare_add(nm);
   eo_lexer_dtor_pop(ls);
   check_next(ls, ';');
   FILL_DOC(ls, part, doc);
}

static void
parse_implement(Eo_Lexer *ls, Eina_Bool iface)
{
   Eina_Strbuf *buf = NULL;
   Eolian_Implement *impl = NULL;
   int iline = ls->line_number, icol = ls->column;
   if (iface)
     check_kw(ls, KW_class);
   if (ls->t.kw == KW_class)
     {
        eo_lexer_get(ls);
        check_next(ls, '.');
        if (ls->t.kw == KW_destructor)
          {
             ls->klass->class_dtor_enable = EINA_TRUE;
             eo_lexer_get(ls);
          }
        else
          {
             check_kw_next(ls, KW_constructor);
             ls->klass->class_ctor_enable = EINA_TRUE;
          }
        check_next(ls, ';');
        return;
     }
   Eina_Bool glob_auto = EINA_FALSE, glob_empty = EINA_FALSE;
   switch (ls->t.kw)
     {
        case KW_at_auto:
          glob_auto = EINA_TRUE;
          eo_lexer_get(ls);
          break;
        case KW_at_empty:
          glob_empty = EINA_TRUE;
          eo_lexer_get(ls);
          break;
        default:
          break;
     }
   if (ls->t.token == '.')
     {
        eo_lexer_get(ls);
        if (ls->t.token != TOK_VALUE)
          eo_lexer_syntax_error(ls, "name expected");
        Eina_Stringshare *iname = eina_stringshare_printf("%s.%s",
                                                          ls->klass->base.name,
                                                          ls->t.value.s);
        Eina_List *l;
        Eolian_Implement *fimp;
        EINA_LIST_FOREACH(ls->klass->implements, l, fimp)
          if (iname == fimp->base.name)
            {
               impl = fimp;
               break;
            }
        eina_stringshare_del(iname);
        if (!impl)
          {
             eo_lexer_syntax_error(ls, "implement of non-existent function");
             return;
          }
        eo_lexer_get(ls);
        goto propbeg;
     }
   else
     {
        impl = calloc(1, sizeof(Eolian_Implement));
        FILL_BASE(impl->base, ls, iline, icol, IMPLEMENT);
        ls->klass->implements = eina_list_append(ls->klass->implements, impl);
        eolian_object_ref(&impl->base);
     }
   if (ls->t.token != TOK_VALUE)
     eo_lexer_syntax_error(ls, "class name expected");
   buf = eina_strbuf_new();
   eo_lexer_dtor_push(ls, EINA_FREE_CB(eina_strbuf_free), buf);
   eina_strbuf_append(buf, ls->t.value.s);
   eo_lexer_get(ls);
   check_next(ls, '.');
   if (ls->t.token != TOK_VALUE)
     eo_lexer_syntax_error(ls, "name or constructor/destructor expected");
   for (;;)
     {
        if ((ls->t.kw == KW_constructor) || (ls->t.kw == KW_destructor))
          {
             eina_strbuf_append_char(buf, '.');
             eina_strbuf_append(buf, eo_lexer_keyword_str_get(ls->t.kw));
             eo_lexer_get(ls);
             check(ls, ';');
             goto propbeg;
          }
        eina_strbuf_append_char(buf, '.');
        check(ls, TOK_VALUE);
        eina_strbuf_append(buf, ls->t.value.s);
        eo_lexer_get(ls);
        if (ls->t.token != '.') break;
        eo_lexer_get(ls);
     }
propbeg:
   if (ls->t.token == '{')
     {
        Eina_Bool has_get = EINA_FALSE, has_set = EINA_FALSE;
        eo_lexer_get(ls);
        FILL_DOC(ls, impl, common_doc);
        for (;;) switch (ls->t.kw)
          {
           case KW_get:
             CASE_LOCK(ls, get, "get specifier");
             eo_lexer_get(ls);
             impl->is_prop_get = EINA_TRUE;
             impl->get_auto = glob_auto;
             impl->get_empty = glob_empty;
             if (ls->t.kw == KW_at_auto)
               {
                  impl->get_auto = EINA_TRUE;
                  eo_lexer_get(ls);
               }
             else if (ls->t.kw == KW_at_empty)
               {
                  impl->get_empty = EINA_TRUE;
                  eo_lexer_get(ls);
               }
             check_next(ls, ';');
             FILL_DOC(ls, impl, get_doc);
             break;
           case KW_set:
             CASE_LOCK(ls, set, "set specifier");
             eo_lexer_get(ls);
             impl->is_prop_set = EINA_TRUE;
             impl->set_auto = glob_auto;
             impl->set_empty = glob_empty;
             if (ls->t.kw == KW_at_auto)
               {
                  impl->set_auto = EINA_TRUE;
                  eo_lexer_get(ls);
               }
             else if (ls->t.kw == KW_at_empty)
               {
                  impl->set_empty = EINA_TRUE;
                  eo_lexer_get(ls);
               }
             check_next(ls, ';');
             FILL_DOC(ls, impl, set_doc);
             break;
           default:
             goto propend;
          }
propend:
        if (!has_get && !has_set)
          eo_lexer_syntax_error(ls, "property implements need at least get or set specified");
        check_next(ls, '}');
     }
   else
     {
        if (glob_auto)
          impl->get_auto = impl->set_auto = EINA_TRUE;
        if (glob_empty)
          impl->get_empty = impl->set_empty = EINA_TRUE;
        check_next(ls, ';');
        FILL_DOC(ls, impl, common_doc);
     }
   if (buf)
     {
        impl->base.name = eina_stringshare_add(eina_strbuf_string_get(buf));
        eo_lexer_dtor_pop(ls);
     }
}

static void
parse_constructor(Eo_Lexer *ls)
{
   Eina_Strbuf *buf = NULL;
   Eolian_Constructor *ctor = NULL;
   ctor = calloc(1, sizeof(Eolian_Constructor));
   FILL_BASE(ctor->base, ls, ls->line_number, ls->column, CONSTRUCTOR);
   ls->klass->constructors = eina_list_append(ls->klass->constructors, ctor);
   eolian_object_ref(&ctor->base);
   if (ls->t.token == '.')
     {
        check_next(ls, '.');
        if (ls->t.token != TOK_VALUE)
          eo_lexer_syntax_error(ls, "name expected");
        ctor->base.name = eina_stringshare_printf("%s.%s",
                                                  ls->klass->base.name,
                                                  ls->t.value.s);
        eo_lexer_get(ls);
        while (ls->t.kw == KW_at_optional || ls->t.kw == KW_at_ctor_param)
          {
             if (ls->t.kw == KW_at_optional)
               {
                  ctor->is_optional = EINA_TRUE;
               }
             if (ls->t.kw == KW_at_ctor_param)
               {
                  ctor->is_ctor_param = EINA_TRUE;
               }
             eo_lexer_get(ls);
          }
        check_next(ls, ';');
        return;
     }
   check(ls, TOK_VALUE);
   buf = eina_strbuf_new();
   eo_lexer_dtor_push(ls, EINA_FREE_CB(eina_strbuf_free), buf);
   eina_strbuf_append(buf, ls->t.value.s);
   eo_lexer_get(ls);
   check_next(ls, '.');
   check(ls, TOK_VALUE);
   for (;;)
     {
        eina_strbuf_append_char(buf, '.');
        check(ls, TOK_VALUE);
        eina_strbuf_append(buf, ls->t.value.s);
        eo_lexer_get(ls);
        if (ls->t.token != '.') break;
        eo_lexer_get(ls);
     }
   while (ls->t.kw == KW_at_optional || ls->t.kw == KW_at_ctor_param)
     {
        if (ls->t.kw == KW_at_optional)
          {
             ctor->is_optional = EINA_TRUE;
          }
        if (ls->t.kw == KW_at_ctor_param)
          {
             ctor->is_ctor_param = EINA_TRUE;
          }
        eo_lexer_get(ls);
     }
   check_next(ls, ';');
   ctor->base.name = eina_stringshare_add(eina_strbuf_string_get(buf));
   eo_lexer_dtor_pop(ls);
}

static void
parse_event(Eo_Lexer *ls)
{
   Eolian_Event *ev = calloc(1, sizeof(Eolian_Event));
   FILL_BASE(ev->base, ls, ls->line_number, ls->column, EVENT);
   ev->scope = EOLIAN_SCOPE_PUBLIC;
   Eina_Strbuf *buf = eina_strbuf_new();
   eo_lexer_dtor_push(ls, EINA_FREE_CB(eina_strbuf_free), buf);
   ls->klass->events = eina_list_append(ls->klass->events, ev);
   eolian_object_ref(&ev->base);
   check(ls, TOK_VALUE);
   eina_strbuf_append(buf, ls->t.value.s);
   eo_lexer_get(ls);
   while (ls->t.token == ',')
     {
        eo_lexer_get(ls);
        check(ls, TOK_VALUE);
        eina_strbuf_append_char(buf, ',');
        eina_strbuf_append(buf, ls->t.value.s);
        eo_lexer_get(ls);
     }
   ev->base.name = eina_stringshare_add(eina_strbuf_string_get(buf));
   eo_lexer_dtor_pop(ls);
   Eina_Bool has_scope = EINA_FALSE, has_beta = EINA_FALSE,
             has_hot   = EINA_FALSE, has_restart = EINA_FALSE,
             has_owned = EINA_FALSE;
   for (;;) switch (ls->t.kw)
     {
      case KW_at_private:
      case KW_at_protected:
        CASE_LOCK(ls, scope, "scope qualifier")
        ev->scope = (ls->t.kw == KW_at_private)
                     ? EOLIAN_SCOPE_PRIVATE
                     : EOLIAN_SCOPE_PROTECTED;
        eo_lexer_get(ls);
        break;
      case KW_at_beta:
        CASE_LOCK(ls, beta, "beta qualifier")
        ev->is_beta = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_hot:
        CASE_LOCK(ls, hot, "hot qualifier");
        ev->is_hot = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_restart:
        CASE_LOCK(ls, restart, "restart qualifier");
        ev->is_restart = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_owned:
        CASE_LOCK(ls, owned, "owned qualifier");
        eo_lexer_get(ls);
        break;
      default:
        goto end;
     }
end:
   check_next(ls, ':');
   ev->type = eo_lexer_type_release(ls, parse_type_void(ls));
   ev->type->owned = has_owned;
   check(ls, ';');
   eo_lexer_get(ls);
   FILL_DOC(ls, ev, doc);
   ev->klass = ls->klass;
}

static void
parse_methods(Eo_Lexer *ls)
{
   int line, col;
   eo_lexer_get(ls);
   line = ls->line_number, col = ls->column;
   check_next(ls, '{');
   while (ls->t.token != '}')
     {
        if (ls->t.kw == KW_at_property)
          {
             eo_lexer_get(ls);
             parse_property(ls);
             continue;
          }
        parse_method(ls);
     }
   check_match(ls, '}', '{', line, col);
}

static void
parse_parts(Eo_Lexer *ls)
{
   int line, col;
   eo_lexer_get(ls);
   line = ls->line_number, col = ls->column;
   check_next(ls, '{');
   while (ls->t.token != '}')
     parse_part(ls);
   check_match(ls, '}', '{', line, col);
}

static void
parse_implements(Eo_Lexer *ls, Eina_Bool iface)
{
   int line, col;
   eo_lexer_get(ls);
   line = ls->line_number, col = ls->column;
   check_next(ls, '{');
   while (ls->t.token != '}')
     parse_implement(ls, iface);
   check_match(ls, '}', '{', line, col);
}

static void
parse_constructors(Eo_Lexer *ls)
{
   int line, col;
   eo_lexer_get(ls);
   line = ls->line_number, col = ls->column;
   check_next(ls, '{');
   while (ls->t.token != '}')
     parse_constructor(ls);
   check_match(ls, '}', '{', line, col);
}

static void
parse_events(Eo_Lexer *ls)
{
   int line, col;
   eo_lexer_get(ls);
   line = ls->line_number;
   col = ls->column;
   check(ls, '{');
   eo_lexer_get(ls);
   while (ls->t.token != '}')
     parse_event(ls);
   check_match(ls, '}', '{', line, col);
}

static void
_validate_pfx(Eo_Lexer *ls)
{
   char ebuf[PATH_MAX];
   check(ls, TOK_VALUE);
   const char *str = ls->t.value.s;
   if ((*str != '_') && ((*str < 'a') || (*str > 'z')))
     goto error;
   for (++str; *str; ++str)
     {
        if (*str == '_')
          continue;
        if ((*str >= 'a') && (*str <= 'z'))
          continue;
        if ((*str >= '0') && (*str <= '9'))
          continue;
        goto error;
     }
   return;
error:
   snprintf(ebuf, sizeof(ebuf), "invalid prefix '%s'", ls->t.value.s);
   eo_lexer_syntax_error(ls, ebuf);
}

static void
parse_class_body(Eo_Lexer *ls, Eolian_Class_Type type)
{
   Eina_Bool has_legacy_prefix = EINA_FALSE,
             has_eo_prefix     = EINA_FALSE,
             has_event_prefix  = EINA_FALSE,
             has_data          = EINA_FALSE,
             has_methods       = EINA_FALSE,
             has_parts         = EINA_FALSE,
             has_implements    = EINA_FALSE,
             has_constructors  = EINA_FALSE,
             has_events        = EINA_FALSE;
   FILL_DOC(ls, ls->klass, doc);
   if (type == EOLIAN_CLASS_INTERFACE)
     {
        ls->klass->data_type = eina_stringshare_add("null");
     }
   for (;;) switch (ls->t.kw)
     {
      case KW_legacy_prefix:
        CASE_LOCK(ls, legacy_prefix, "legacy prefix definition")
        eo_lexer_get(ls);
        check_next(ls, ':');
        _validate_pfx(ls);
        ls->klass->legacy_prefix = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
        check_next(ls, ';');
        break;
      case KW_eo_prefix:
        CASE_LOCK(ls, eo_prefix, "eo prefix definition")
        eo_lexer_get(ls);
        check_next(ls, ':');
        _validate_pfx(ls);
        ls->klass->eo_prefix = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
        check_next(ls, ';');
        break;
      case KW_event_prefix:
        CASE_LOCK(ls, event_prefix, "event prefix definition")
        eo_lexer_get(ls);
        check_next(ls, ':');
        _validate_pfx(ls);
        ls->klass->ev_prefix = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
        check_next(ls, ';');
        break;
      case KW_data:
        if (type == EOLIAN_CLASS_INTERFACE) return;
        CASE_LOCK(ls, data, "data definition")
        eo_lexer_get(ls);
        check_next(ls, ':');
        check(ls, TOK_VALUE);
        ls->klass->data_type = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
        check_next(ls, ';');
        break;
      case KW_methods:
        CASE_LOCK(ls, methods, "methods definition")
        parse_methods(ls);
        break;
      case KW_parts:
        CASE_LOCK(ls, parts, "parts definition")
        parse_parts(ls);
        break;
      case KW_implements:
        CASE_LOCK(ls, implements, "implements definition")
        parse_implements(ls, type == EOLIAN_CLASS_INTERFACE);
        break;
      case KW_constructors:
        if (type == EOLIAN_CLASS_INTERFACE || type == EOLIAN_CLASS_MIXIN)
          return;
        CASE_LOCK(ls, constructors, "constructors definition")
        parse_constructors(ls);
        break;
      case KW_events:
        CASE_LOCK(ls, events, "events definition")
        parse_events(ls);
        break;
      default:
        return;
     }
}

static void
_inherit_dep(Eo_Lexer *ls, Eina_Strbuf *buf, Eina_Bool parent)
{
   char ebuf[PATH_MAX];
   const char *iname;
   char *fnm;
   eina_strbuf_reset(buf);
   eo_lexer_context_push(ls);
   parse_name(ls, buf);
   iname = eina_strbuf_string_get(buf);
   fnm = database_class_to_filename(iname);
   if (compare_class_file(fnm, ls->filename))
     {
        free(fnm);
        eo_lexer_context_restore(ls);
        snprintf(ebuf, sizeof(ebuf), "class '%s' cannot inherit from itself",
                 iname);
        eo_lexer_syntax_error(ls, ebuf);
        return; /* unreachable (longjmp above), make static analysis shut up */
     }
   if (!eina_hash_find(ls->state->filenames_eo, fnm))
     {
        free(fnm);
        eo_lexer_context_restore(ls);
        snprintf(ebuf, sizeof(ebuf), "unknown inherit '%s'", iname);
        eo_lexer_syntax_error(ls, ebuf);
        return;
     }

   Eina_Stringshare *inames = eina_stringshare_add(iname), *oiname = NULL;
   /* never allow duplicate inherits */
   if (!parent)
     {
        Eina_List *l;
        if (inames == ls->klass->parent_name)
          goto inherit_dup;
        EINA_LIST_FOREACH(ls->klass->extends, l, oiname)
          {
             if (inames == oiname)
               goto inherit_dup;
          }
     }
   database_defer(ls->state, fnm, EINA_TRUE);
   if (parent)
     ls->klass->parent_name = inames;
   else
     ls->klass->extends = eina_list_append(ls->klass->extends, inames);
   free(fnm);
   eo_lexer_context_pop(ls);
   return;

inherit_dup:
   free(fnm);
   eina_stringshare_del(inames);
   eo_lexer_context_restore(ls);
   snprintf(ebuf, sizeof(ebuf), "duplicate inherit '%s'", iname);
   eo_lexer_syntax_error(ls, ebuf);
}

static void
parse_class(Eo_Lexer *ls, Eolian_Class_Type type)
{
   const char *bnm;
   char *fnm;
   Eina_Bool same;
   int line, col;
   Eina_Strbuf *buf = eina_strbuf_new();
   eo_lexer_dtor_push(ls, EINA_FREE_CB(eina_strbuf_free), buf);
   ls->klass = (Eolian_Class *)eo_lexer_node_new(ls, sizeof(Eolian_Class));
   FILL_BASE(ls->klass->base, ls, ls->line_number, ls->column, CLASS);
   eo_lexer_get(ls);
   ls->klass->type = type;
   eo_lexer_context_push(ls);
   parse_name(ls, buf);
   bnm = eina_stringshare_ref(ls->filename);
   fnm = database_class_to_filename(eina_strbuf_string_get(buf));
   same = compare_class_file(bnm, fnm);
   eina_stringshare_del(bnm);
   free(fnm);
   if (!same)
     {
        eo_lexer_context_restore(ls);
        eo_lexer_syntax_error(ls, "class and file names differ");
     }
   ls->klass->base.name = eina_stringshare_add(eina_strbuf_string_get(buf));
   Eolian_Object *decl = _eolian_decl_get(ls, ls->klass->base.name);
   if (decl)
     {
        eo_lexer_context_restore(ls);
        redef_error(ls, decl, &ls->klass->base);
     }
   eo_lexer_context_pop(ls);
   eo_lexer_dtor_pop(ls);

   Eina_Bool is_reg = (type == EOLIAN_CLASS_REGULAR) || (type == EOLIAN_CLASS_ABSTRACT);
   if (ls->t.token != '{')
     {
        line = ls->line_number;
        col = ls->column;
        Eina_Strbuf *ibuf = eina_strbuf_new();
        eo_lexer_dtor_push(ls, EINA_FREE_CB(eina_strbuf_free), ibuf);
        /* new inherits syntax, keep alongside old for now */
        if (ls->t.kw == KW_extends || (is_reg && (ls->t.kw == KW_implements)))
          {
             Eina_Bool ext = (ls->t.kw == KW_extends);
             eo_lexer_get(ls);
             if (is_reg && ext)
               {
                  /* regular class can have a parent, but just one */
                  _inherit_dep(ls, ibuf, EINA_TRUE);
                  /* if not followed by implements, we're done */
                  if (ls->t.kw != KW_implements)
                    {
                       eo_lexer_dtor_pop(ls);
                       goto inherit_done;
                    }
                  eo_lexer_get(ls);
               }
             do
               _inherit_dep(ls, ibuf, EINA_FALSE);
             while (test_next(ls, ','));
          }
        else
          {
             check_next(ls, '(');
             if (ls->t.token != ')')
               {
                   _inherit_dep(ls, ibuf, is_reg);
                   while (test_next(ls, ','))
                     _inherit_dep(ls, ibuf, EINA_FALSE);
               }
             check_match(ls, ')', '(', line, col);
          }
        eo_lexer_dtor_pop(ls);
     }
inherit_done:
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
   parse_class_body(ls, type);
   check_match(ls, '}', '{', line, col);
}

static Eina_Bool
parse_unit(Eo_Lexer *ls, Eina_Bool eot)
{
   switch (ls->t.kw)
     {
      case KW_abstract:
        if (eot) goto def;
        parse_class(ls, EOLIAN_CLASS_ABSTRACT);
        goto found_class;
      case KW_class:
        if (eot) goto def;
        parse_class(ls, EOLIAN_CLASS_REGULAR);
        goto found_class;
      case KW_mixin:
        if (eot) goto def;
        parse_class(ls, EOLIAN_CLASS_MIXIN);
        goto found_class;
      case KW_interface:
        if (eot) goto def;
        parse_class(ls, EOLIAN_CLASS_INTERFACE);
        goto found_class;
      case KW_import:
      case KW_parse:
        {
           Eina_Bool isdep = (ls->t.kw == KW_import);
           Eina_Strbuf *buf = eina_strbuf_new();
           eo_lexer_dtor_push(ls, EINA_FREE_CB(eina_strbuf_free), buf);
           char errbuf[PATH_MAX];
           eo_lexer_get(ls);
           check(ls, TOK_VALUE);
           eina_strbuf_append(buf, ls->t.value.s);
           eina_strbuf_append(buf, ".eot");
           if (!eina_hash_find(ls->state->filenames_eot, eina_strbuf_string_get(buf)))
             {
                size_t buflen = eina_strbuf_length_get(buf);
                eina_strbuf_remove(buf, buflen - 1, buflen);
                if (!eina_hash_find(ls->state->filenames_eo, eina_strbuf_string_get(buf)))
                  {
                     eo_lexer_dtor_pop(ls);
                     snprintf(errbuf, sizeof(errbuf),
                              "unknown import '%s'", ls->t.value.s);
                     eo_lexer_syntax_error(ls, errbuf);
                  }
             }
           database_defer(ls->state, eina_strbuf_string_get(buf), isdep);
           eo_lexer_dtor_pop(ls);
           eo_lexer_get(ls);
           check_next(ls, ';');
           break;
        }
      case KW_type:
        {
           database_type_add(ls->unit,
             eo_lexer_typedecl_release(ls, parse_typedef(ls)));
           break;
        }
      case KW_function:
        {
           database_type_add(ls->unit,
             eo_lexer_typedecl_release(ls, parse_function_pointer(ls)));
           break;
        }
      case KW_const:
      case KW_var:
        {
           database_var_add(ls->unit, eo_lexer_variable_release(ls,
             parse_variable(ls, ls->t.kw == KW_var)));
           break;
        }
      case KW_struct:
      case KW_enum:
        {
           Eina_Bool is_enum = (ls->t.kw == KW_enum);
           const char *name;
           int line, col;
           Eina_Bool has_extern;
           const char *freefunc;
           Eina_Strbuf *buf;
           eo_lexer_get(ls);
           parse_struct_attrs(ls, is_enum, &has_extern, &freefunc);
           buf = eina_strbuf_new();
           eo_lexer_dtor_push(ls, EINA_FREE_CB(eina_strbuf_free), buf);
           eo_lexer_context_push(ls);
           line = ls->line_number;
           col = ls->column;
           parse_name(ls, buf);
           name = eina_stringshare_add(eina_strbuf_string_get(buf));
           Eolian_Object *decl = _eolian_decl_get(ls, name);
           if (decl)
             {
                eina_stringshare_del(name);
                eo_lexer_context_restore(ls);
                Eolian_Typedecl tdecl;
                tdecl.base.type = EOLIAN_OBJECT_TYPEDECL;
                tdecl.type = is_enum ? EOLIAN_TYPEDECL_ENUM : EOLIAN_TYPEDECL_STRUCT;
                redef_error(ls, decl, &tdecl.base);
             }
           eo_lexer_context_pop(ls);
           eo_lexer_dtor_pop(ls);
           if (!is_enum && ls->t.token == ';')
             {
                Eolian_Typedecl *def = eo_lexer_typedecl_new(ls);
                def->is_extern = has_extern;
                def->type = EOLIAN_TYPEDECL_STRUCT_OPAQUE;
                if (freefunc)
                  {
                     def->freefunc = eina_stringshare_ref(freefunc);
                     eo_lexer_dtor_pop(ls);
                  }
                def->base.name = name;
                eo_lexer_get(ls);
                FILL_DOC(ls, def, doc);
                FILL_BASE(def->base, ls, line, col, TYPEDECL);
                database_struct_add(ls->unit, eo_lexer_typedecl_release(ls, def));
                break;
             }
           if (is_enum)
             parse_enum(ls, name, has_extern, line, col);
           else
             parse_struct(ls, name, has_extern, line, col, freefunc);
           break;
        }
      def:
      default:
        eo_lexer_syntax_error(ls, "invalid token");
        break;
     }
   return EINA_FALSE;
found_class:
   database_object_add(ls->unit, &ls->klass->base);
   return EINA_TRUE;
}

static void
parse_chunk(Eo_Lexer *ls, Eina_Bool eot)
{
   while (ls->t.token >= 0)
     /* set eot to EINA_TRUE so that we only allow parsing of one class */
     if (parse_unit(ls, eot))
       eot = EINA_TRUE;
}

Eolian_Unit *
eo_parser_database_fill(Eolian_Unit *parent, const char *filename, Eina_Bool eot)
{
   int status = 0;
   const char *fsl = strrchr(filename, '/');
   const char *bsl = strrchr(filename, '\\');
   const char *fname = NULL;
   if (fsl || bsl)
     fname = eina_stringshare_add((fsl > bsl) ? (fsl + 1) : (bsl + 1));
   else
     fname = eina_stringshare_add(filename);

   Eolian_Unit *ret = eina_hash_find(parent->state->main.units, fname);
   if (!ret)
     ret = eina_hash_find(parent->state->staging.units, fname);

   if (ret)
     {
        if ((parent != ret) && !eina_hash_find(parent->children, fname))
          eina_hash_add(parent->children, fname, ret);
        eina_stringshare_del(fname);
        return ret;
     }

   Eo_Lexer *ls = eo_lexer_new(parent->state, filename);
   if (!ls)
     {
        eolian_state_log(parent->state, "unable to create lexer for file '%s'",
                         filename);
        goto error;
     }

   /* read first token */
   eo_lexer_get(ls);

   if ((status = setjmp(ls->err_jmp)))
     goto error;

   parse_chunk(ls, eot);
   if (eot) goto done;

   Eolian_Class *cl;
   if (!(cl = ls->klass))
     {
        eolian_state_log(ls->state, "no class for file '%s'", filename);
        goto error;
     }
   ls->klass = NULL;
   EOLIAN_OBJECT_ADD(ls->unit, cl->base.name, cl, classes);
   eina_hash_set(ls->state->staging.classes_f, cl->base.file, cl);
   eo_lexer_node_release(ls, &cl->base);

done:
   ret = ls->unit;
   eina_hash_add(parent->children, fname, ret);
   eina_stringshare_del(fname);

   eo_lexer_free(ls);
   return ret;

error:
   eina_stringshare_del(fname);
   eo_lexer_free(ls);
   switch (status)
     {
      case EO_LEXER_ERROR_OOM:
        eolian_state_panic(parent->state, "out of memory");
        break;
      default:
        break;
     }
   return NULL;
}

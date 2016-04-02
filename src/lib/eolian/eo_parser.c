#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eo_parser.h"

#define CASE_LOCK(ls, var, msg) \
   if (has_##var) \
     eo_lexer_syntax_error(ls, "double " msg); \
   has_##var = EINA_TRUE;

#define FILL_BASE(exp, ls, l, c) \
   (exp).file = eina_stringshare_ref(ls->filename); \
   (exp).line = l; \
   (exp).column = c;

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
   char  buf[256];
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
             char  buf[256];
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

static Eina_Strbuf *
push_strbuf(Eo_Lexer *ls)
{
   Eina_Strbuf *buf = eina_strbuf_new();
   ls->tmp.str_bufs = eina_list_prepend(ls->tmp.str_bufs, buf);
   return buf;
}

static void
pop_strbuf(Eo_Lexer *ls)
{
   Eina_Strbuf *buf = eina_list_data_get(ls->tmp.str_bufs);
   eina_strbuf_free(buf);
   ls->tmp.str_bufs = eina_list_remove_list(ls->tmp.str_bufs, ls->tmp.str_bufs);
}

static Eolian_Type *
push_type(Eo_Lexer *ls)
{
   Eolian_Type *def = calloc(1, sizeof(Eolian_Type));
   ls->tmp.type_defs = eina_list_prepend(ls->tmp.type_defs, def);
   return def;
}

static Eolian_Typedecl *
push_typedecl(Eo_Lexer *ls)
{
   Eolian_Typedecl *def = calloc(1, sizeof(Eolian_Typedecl));
   ls->tmp.type_decls = eina_list_prepend(ls->tmp.type_decls, def);
   return def;
}

static void
pop_type(Eo_Lexer *ls)
{
   ls->tmp.type_defs = eina_list_remove_list(ls->tmp.type_defs, ls->tmp.type_defs);
}

static void
pop_typedecl(Eo_Lexer *ls)
{
   ls->tmp.type_decls = eina_list_remove_list(ls->tmp.type_decls, ls->tmp.type_decls);
}

static Eina_Stringshare *
push_str(Eo_Lexer *ls, const char *val)
{
   Eina_Stringshare *shr = eina_stringshare_add(val);
   ls->tmp.strs = eina_list_prepend(ls->tmp.strs, shr);
   return shr;
}

static void
pop_str(Eo_Lexer *ls)
{
   ls->tmp.strs = eina_list_remove_list(ls->tmp.strs, ls->tmp.strs);
}

static Eina_Bool
compare_class_file(const char *fn1, const char *fn2)
{
   return !strcmp(fn1, fn2);
}

static const char *declnames[] = {
    "class", "type alias", "struct", "enum", "variable"
};

static void
redef_error(Eo_Lexer *ls, Eolian_Declaration *decl, Eolian_Declaration_Type newt)
{
   Eolian_Object *obj = (Eolian_Object *)decl->data;
   char buf[256], fbuf[256] = { '\0' };
   if (ls->filename != obj->file)
     snprintf(fbuf, sizeof(fbuf), "%s:%d:%d", obj->file, obj->line, obj->column);
   else
     snprintf(fbuf, sizeof(fbuf), "%d:%d", obj->line, obj->column);

   if (newt != decl->type)
     snprintf(buf, sizeof(buf), "%s '%s' redefined as %s (originally at %s)",
              declnames[decl->type], decl->name, declnames[newt], fbuf);
   else
     snprintf(buf, sizeof(buf), "%s '%s' redefined (originally at %s)",
              declnames[decl->type], decl->name, fbuf);

   eo_lexer_syntax_error(ls, buf);
}

static Eina_Strbuf *
parse_name(Eo_Lexer *ls, Eina_Strbuf *buf)
{
   check(ls, TOK_VALUE);
   if (eo_lexer_get_c_type(ls->t.kw))
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
        if (eo_lexer_get_c_type(ls->t.kw))
          eo_lexer_syntax_error(ls, "invalid name");
     }
   return buf;
}

static void
_fill_name(const char *input, Eina_Stringshare **full_name,
           Eina_Stringshare **name, Eina_List **namespaces)
{
   char *fname = strdup(input);
   char *sname = fname, *dot = fname;
   *full_name = input;
   do
     {
        dot = strchr(dot, '.');
        if (dot)
          {
             *dot = '\0';
             *namespaces = eina_list_append(*namespaces,
                                            eina_stringshare_add(sname));
             ++dot;
             sname = dot;
          }
     }
   while (dot);
   *name = eina_stringshare_add(sname);
   free(fname);
}

static Eolian_Expression *
push_expr(Eo_Lexer *ls)
{
   Eolian_Expression *def = calloc(1, sizeof(Eolian_Expression));
   ls->tmp.expr_defs = eina_list_prepend(ls->tmp.expr_defs, def);
   return def;
}

static void
pop_expr(Eo_Lexer *ls)
{
   ls->tmp.expr_defs = eina_list_remove_list(ls->tmp.expr_defs, ls->tmp.expr_defs);
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

      default: return -1;
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

      default: return -1;
     }
}

static const int binprec[] = {
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

static int
get_binop_prec(Eolian_Binary_Operator id)
{
   if (id < 0) return -1;
   return binprec[id];
}

static Eolian_Expression *parse_expr_bin(Eo_Lexer *ls, int min_prec);
static Eolian_Expression *parse_expr(Eo_Lexer *ls);

static Eolian_Expression *
parse_expr_simple(Eo_Lexer *ls)
{
   Eolian_Expression *expr;
   Eolian_Unary_Operator unop = get_unop_id(ls->t.token);
   if (unop >= 0)
     {
        int line = ls->line_number, col = ls->column;
        eo_lexer_get(ls);
        Eolian_Expression *exp = parse_expr_bin(ls, UNARY_PRECEDENCE);
        pop_expr(ls);
        expr = push_expr(ls);
        FILL_BASE(expr->base, ls, line, col);
        expr->unop = unop;
        expr->type = EOLIAN_EXPR_UNARY;
        expr->expr = exp;
        return expr;
     }
   switch (ls->t.token)
     {
      case TOK_NUMBER:
        {
           int line = ls->line_number, col = ls->column;
           expr = push_expr(ls);
           FILL_BASE(expr->base, ls, line, col);
           expr->type = ls->t.kw + 1; /* map Numbers from lexer to expr type */
           memcpy(&expr->value, &ls->t.value, sizeof(expr->value));
           eo_lexer_get(ls);
           break;
        }
      case TOK_STRING:
        {
           int line = ls->line_number, col = ls->column;
           expr = push_expr(ls);
           FILL_BASE(expr->base, ls, line, col);
           expr->type = EOLIAN_EXPR_STRING;
           expr->value.s = eina_stringshare_ref(ls->t.value.s);
           eo_lexer_get(ls);
           break;
        }
      case TOK_CHAR:
        {
           int line = ls->line_number, col = ls->column;
           expr = push_expr(ls);
           FILL_BASE(expr->base, ls, line, col);
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
                   expr = push_expr(ls);
                   expr->type = EOLIAN_EXPR_BOOL;
                   expr->value.b = (ls->t.kw == KW_true);
                   eo_lexer_get(ls);
                   break;
                }
              case KW_null:
                {
                   expr = push_expr(ls);
                   expr->type = EOLIAN_EXPR_NULL;
                   eo_lexer_get(ls);
                   break;
                }
              default:
                {
                   Eina_Strbuf *buf = push_strbuf(ls);
                   expr = push_expr(ls);
                   expr->type = EOLIAN_EXPR_NAME;
                   parse_name(ls, buf);
                   expr->value.s = eina_stringshare_add(eina_strbuf_string_get
                       (buf));
                   pop_strbuf(ls);
                   break;
                }
             }
           FILL_BASE(expr->base, ls, line, col);
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
        int prec = get_binop_prec(op);
        if ((op < 0) || (prec < 0) || (prec < min_prec))
          break;
        eo_lexer_get(ls);
        rhs = parse_expr_bin(ls, prec + 1);
        pop_expr(ls);
        pop_expr(ls);
        bin = push_expr(ls);
        FILL_BASE(bin->base, ls, line, col);
        bin->binop = op;
        bin->type = EOLIAN_EXPR_BINARY;
        bin->lhs = lhs;
        bin->rhs = rhs;
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
   if (def->base.file) eina_stringshare_del(def->base.file);
   if (def->name) eina_stringshare_del(def->name);
   database_type_del(def->type);
   database_doc_del(def->doc);
   free(def);
}

static Eolian_Typedecl *
parse_struct(Eo_Lexer *ls, const char *name, Eina_Bool is_extern,
             int line, int column, const char *freefunc)
{
   int bline = ls->line_number, bcolumn = ls->column;
   Eolian_Typedecl *def = push_typedecl(ls);
   def->is_extern = is_extern;
   if (name) _fill_name(name, &def->full_name, &def->name, &def->namespaces);
   def->type = EOLIAN_TYPEDECL_STRUCT;
   def->fields = eina_hash_string_small_new(EINA_FREE_CB(_struct_field_free));
   def->freefunc = freefunc;
   pop_str(ls);
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
        eo_lexer_get(ls);
        check_next(ls, ':');
        tp = parse_type(ls);
        FILL_BASE(fdef->base, ls, fline, fcol);
        fdef->type = tp;
        fdef->name = eina_stringshare_ref(fname);
        pop_type(ls);
        check_next(ls, ';');
        FILL_DOC(ls, fdef, doc);
     }
   check_match(ls, '}', '{', bline, bcolumn);
   FILL_BASE(def->base, ls, line, column);
   if (name) database_struct_add(def);
   return def;
}

static void
_enum_field_free(Eolian_Enum_Type_Field *def)
{
   if (def->base.file) eina_stringshare_del(def->base.file);
   if (def->name) eina_stringshare_del(def->name);
   database_expr_del(def->value);
   database_doc_del(def->doc);
   free(def);
}

static Eolian_Typedecl *
parse_enum(Eo_Lexer *ls, const char *name, Eina_Bool is_extern,
           int line, int column)
{
   int bline = ls->line_number, bcolumn = ls->column;
   Eolian_Typedecl *def = push_typedecl(ls);
   def->is_extern = is_extern;
   _fill_name(name, &def->full_name, &def->name, &def->namespaces);
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
        eo_lexer_get(ls);
        FILL_BASE(fdef->base, ls, fline, fcol);
        fdef->base_enum = def;
        fdef->name = eina_stringshare_ref(fname);
        if (ls->t.token != '=')
          {
             if (!prev_fl)
               {
                  Eolian_Expression *eexp = push_expr(ls);
                  FILL_BASE(eexp->base, ls, -1, -1);
                  eexp->type = EOLIAN_EXPR_INT;
                  eexp->value.i = 0;
                  fdef->value = eexp;
                  fdef->is_public_value = EINA_TRUE;
                  pop_expr(ls);
                  prev_fl = fdef;
                  fl_nadd = 0;
               }
             else
               {
                  Eolian_Expression *rhs = push_expr(ls),
                                    *bin = push_expr(ls);
                  FILL_BASE(rhs->base, ls, -1, -1);
                  FILL_BASE(bin->base, ls, -1, -1);

                  rhs->type = EOLIAN_EXPR_INT;
                  rhs->value.i = ++fl_nadd;

                  bin->type = EOLIAN_EXPR_BINARY;
                  bin->binop = EOLIAN_BINOP_ADD;
                  bin->lhs = prev_fl->value;
                  bin->rhs = rhs;
                  bin->weak_lhs = EINA_TRUE;

                  fdef->value = bin;
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
             pop_expr(ls);
          }
        Eina_Bool want_next = (ls->t.token == ',');
        if (want_next)
          eo_lexer_get(ls);
        FILL_DOC(ls, fdef, doc);
        if (!want_next || ls->t.token == '}')
          break;
     }
   check_match(ls, '}', '{', bline, bcolumn);
   FILL_BASE(def->base, ls, line, column);
   if (name) database_enum_add(def);
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
           *freefunc = push_str(ls, ls->t.value.s);
           eo_lexer_get(ls);
           check_match(ls, ')', '(', pline, pcol);
           break;
        }
      default:
        return;
     }
}

static void
_parse_dep(Eo_Lexer *ls, const char *fname, const char *name)
{
   if (eina_hash_find(_parsingeos, fname))
     {
        char buf[PATH_MAX];
        eo_lexer_context_restore(ls);
        snprintf(buf, sizeof(buf), "cyclic dependency '%s'", name);
        eo_lexer_syntax_error(ls, buf);
     }
   if (!eo_parser_database_fill(fname, EINA_FALSE))
     {
        char buf[PATH_MAX];
        eo_lexer_context_restore(ls);
        snprintf(buf, sizeof(buf), "error parsing dependency '%s'", name);
        eo_lexer_syntax_error(ls, buf);
     }
}

static Eolian_Type *
parse_type_void_base(Eo_Lexer *ls, Eina_Bool noptr)
{
   Eolian_Type *def;
   const char *ctype;
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
           def = parse_type_void_base(ls, EINA_TRUE);
           FILL_BASE(def->base, ls, line, col);
           def->is_const = EINA_TRUE;
           check_match(ls, ')', '(', pline, pcol);
           goto parse_ptr;
        }
      case KW_own:
        {
           int pline, pcolumn;
           eo_lexer_get(ls);
           pline = ls->line_number;
           pcolumn = ls->column;
           check_next(ls, '(');
           eo_lexer_context_push(ls);
           def = parse_type_void_base(ls, EINA_TRUE);
           if (def->type != EOLIAN_TYPE_POINTER)
             {
                eo_lexer_context_restore(ls);
                eo_lexer_syntax_error(ls, "pointer type expected");
             }
           eo_lexer_context_pop(ls);
           FILL_BASE(def->base, ls, line, col);
           def->is_own = EINA_TRUE;
           check_match(ls, ')', '(', pline, pcolumn);
           goto parse_ptr;
        }
      case KW_free:
        {
           int pline, pcolumn;
           eo_lexer_get(ls);
           pline = ls->line_number;
           pcolumn = ls->column;
           check_next(ls, '(');
           eo_lexer_context_push(ls);
           def = parse_type_void_base(ls, EINA_TRUE);
           if (def->type != EOLIAN_TYPE_POINTER)
             {
                eo_lexer_context_restore(ls);
                eo_lexer_syntax_error(ls, "pointer type expected");
             }
           eo_lexer_context_pop(ls);
           check_next(ls, ',');
           check(ls, TOK_VALUE);
           def->freefunc = eina_stringshare_ref(ls->t.value.s);
           eo_lexer_get(ls);
           FILL_BASE(def->base, ls, line, col);
           check_match(ls, ')', '(', pline, pcolumn);
           goto parse_ptr;
        }
      default:
        break;
     }
   def = push_type(ls);
   FILL_BASE(def->base, ls, line, col);
   if (ls->t.kw == KW_void)
     {
        def->type = EOLIAN_TYPE_VOID;
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
        ctype = eo_lexer_get_c_type(ls->t.kw);
        if (ctype)
          {
             _fill_name(eina_stringshare_ref(ls->t.value.s), &def->full_name,
                        &def->name, &def->namespaces);
             eo_lexer_get(ls);
             if (tpid >= KW_accessor && tpid <= KW_promise)
               {
                  int bline = ls->line_number, bcol = ls->column;
                  def->type = EOLIAN_TYPE_COMPLEX;
                  check_next(ls, '<');
                  def->subtypes = eina_list_append(def->subtypes,
                                                   parse_type(ls));
                  pop_type(ls);
                  if (tpid == KW_hash)
                    {
                       check_next(ls, ',');
                       def->subtypes = eina_list_append(def->subtypes,
                                                        parse_type(ls));
                       pop_type(ls);
                    }
                  check_match(ls, '>', '<', bline, bcol);
               }
          }
        else
          {
             const char *bnm, *nm;
             char *fnm;
             buf = push_strbuf(ls);
             eo_lexer_context_push(ls);
             parse_name(ls, buf);
             nm = eina_strbuf_string_get(buf);
             bnm = eina_stringshare_ref(ls->filename);
             fnm = database_class_to_filename(nm);
             if (!compare_class_file(bnm, fnm))
               {
                  const char *fname = eina_hash_find(_filenames, fnm);
                  eina_stringshare_del(bnm);
                  free(fnm);
                  if (fname)
                    {
                       _parse_dep(ls, fname, nm);
                       def->type = EOLIAN_TYPE_CLASS;
                    }
               }
             else
               {
                  eina_stringshare_del(bnm);
                  free(fnm);
                  def->type = EOLIAN_TYPE_CLASS;
               }
             _fill_name(eina_stringshare_add(nm), &def->full_name, &def->name,
                        &def->namespaces);
             eo_lexer_context_pop(ls);
             pop_strbuf(ls);
          }
     }
parse_ptr:
   /* check: complex/class type must always be behind a pointer */
   if (!noptr && ((def->type == EOLIAN_TYPE_CLASS) || (def->type == EOLIAN_TYPE_COMPLEX)))
     check(ls, '*');
   while (ls->t.token == '*')
     {
        Eolian_Type *pdef;
        pop_type(ls);
        pdef = push_type(ls);
        FILL_BASE(pdef->base, ls, ls->line_number, ls->column);
        pdef->base_type = def;
        pdef->type = EOLIAN_TYPE_POINTER;
        def = pdef;
        eo_lexer_get(ls);
     }
   return def;
}

static Eolian_Type *
parse_type_void(Eo_Lexer *ls)
{
   return parse_type_void_base(ls, EINA_FALSE);
}

static Eolian_Typedecl *
parse_typedef(Eo_Lexer *ls)
{
   Eolian_Declaration *decl;
   Eolian_Typedecl *def = push_typedecl(ls);
   Eina_Bool has_extern;
   const char *freefunc;
   Eina_Strbuf *buf;
   eo_lexer_get(ls);
   parse_struct_attrs(ls, EINA_FALSE, &has_extern, &freefunc);
   def->freefunc = freefunc;
   pop_str(ls);
   def->type = EOLIAN_TYPEDECL_ALIAS;
   def->is_extern = has_extern;
   buf = push_strbuf(ls);
   eo_lexer_context_push(ls);
   FILL_BASE(def->base, ls, ls->line_number, ls->column);
   parse_name(ls, buf);
   _fill_name(eina_stringshare_add(eina_strbuf_string_get(buf)),
              &def->full_name, &def->name, &def->namespaces);
   decl = (Eolian_Declaration *)eina_hash_find(_decls, def->full_name);
   if (decl)
     {
        eo_lexer_context_restore(ls);
        redef_error(ls, decl, EOLIAN_DECL_ALIAS);
     }
   eo_lexer_context_pop(ls);
   check_next(ls, ':');
   def->base_type = parse_type(ls);
   pop_type(ls);
   check_next(ls, ';');
   FILL_DOC(ls, def, doc);
   return def;
}

static Eolian_Variable *
parse_variable(Eo_Lexer *ls, Eina_Bool global)
{
   Eolian_Declaration *decl;
   Eolian_Variable *def = calloc(1, sizeof(Eolian_Variable));
   Eina_Bool has_extern = EINA_FALSE;
   Eina_Strbuf *buf;
   ls->tmp.var = def;
   eo_lexer_get(ls);
   if (ls->t.kw == KW_at_extern)
     {
        if (!global)
          eo_lexer_syntax_error(ls, "extern constant");
        has_extern = EINA_TRUE;
        eo_lexer_get(ls);
     }
   def->type = global ? EOLIAN_VAR_GLOBAL : EOLIAN_VAR_CONSTANT;
   def->is_extern = has_extern;
   buf = push_strbuf(ls);
   eo_lexer_context_push(ls);
   FILL_BASE(def->base, ls, ls->line_number, ls->column);
   parse_name(ls, buf);
   _fill_name(eina_stringshare_add(eina_strbuf_string_get(buf)),
              &def->full_name, &def->name, &def->namespaces);
   decl = (Eolian_Declaration *)eina_hash_find(_decls, def->full_name);
   if (decl)
     {
        eo_lexer_context_restore(ls);
        redef_error(ls, decl, EOLIAN_DECL_VAR);
     }
   eo_lexer_context_pop(ls);
   check_next(ls, ':');
   def->base_type = parse_type(ls);
   pop_type(ls);
   if ((ls->t.token == '=') && !has_extern)
     {
        ls->expr_mode = EINA_TRUE;
        eo_lexer_get(ls);
        def->value = parse_expr(ls);
        ls->expr_mode = EINA_FALSE;
        pop_expr(ls);
     }
   check_next(ls, ';');
   FILL_DOC(ls, def, doc);
   return def;
}

typedef struct _Eo_Ret_Def
{
   Eolian_Type *type;
   Eolian_Documentation *doc;
   Eolian_Expression *default_ret_val;
   Eina_Bool warn_unused:1;
} Eo_Ret_Def;

static void
parse_return(Eo_Lexer *ls, Eo_Ret_Def *ret, Eina_Bool allow_void)
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
   if (ls->t.token == '(')
     {
        int line = ls->line_number, col = ls->column;
        ls->expr_mode = EINA_TRUE;
        eo_lexer_get(ls);
        ret->default_ret_val = parse_expr(ls);
        ls->expr_mode = EINA_FALSE;
        check_match(ls, ')', '(', line, col);
     }
   if (ls->t.kw == KW_at_warn_unused)
     {
        ret->warn_unused = EINA_TRUE;
        eo_lexer_get(ls);
     }
   check_next(ls, ';');
   FILL_DOC(ls, ret, doc);
}

static void
parse_param(Eo_Lexer *ls, Eina_List **params, Eina_Bool allow_inout,
            Eina_Bool is_vals)
{
   Eina_Bool has_nonull   = EINA_FALSE, has_optional = EINA_FALSE,
             has_nullable = EINA_FALSE;
   Eolian_Function_Parameter *par = calloc(1, sizeof(Eolian_Function_Parameter));
   FILL_BASE(par->base, ls, ls->line_number, ls->column);
   *params = eina_list_append(*params, par);
   if (allow_inout)
     {
        if (ls->t.kw == KW_at_in)
          {
             par->param_dir = EOLIAN_IN_PARAM;
             eo_lexer_get(ls);
          }
        else if (ls->t.kw == KW_at_out)
          {
             par->param_dir = EOLIAN_OUT_PARAM;
             eo_lexer_get(ls);
          }
        else if (ls->t.kw == KW_at_inout)
          {
             par->param_dir = EOLIAN_INOUT_PARAM;
             eo_lexer_get(ls);
          }
        else
           par->param_dir = EOLIAN_IN_PARAM;
     }
   check(ls, TOK_VALUE);
   par->name = eina_stringshare_ref(ls->t.value.s);
   eo_lexer_get(ls);
   check_next(ls, ':');
   if (par->param_dir == EOLIAN_OUT_PARAM || par->param_dir == EOLIAN_INOUT_PARAM)
     par->type = parse_type_void(ls);
   else
     par->type = parse_type(ls);
   pop_type(ls);
   if ((is_vals || (par->param_dir == EOLIAN_OUT_PARAM)) && (ls->t.token == '('))
     {
        int line = ls->line_number, col = ls->column;
        ls->expr_mode = EINA_TRUE;
        eo_lexer_get(ls);
        par->value = parse_expr(ls);
        ls->expr_mode = EINA_FALSE;
        pop_expr(ls);
        check_match(ls, ')', '(', line, col);
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
             has_values = EINA_FALSE;
   Eina_Bool is_get = (ls->t.kw == KW_get);
   if (is_get)
     {
        if (prop->base.file)
          eina_stringshare_del(prop->base.file);
        FILL_BASE(prop->base, ls, ls->line_number, ls->column);
        if (prop->type == EOLIAN_PROP_SET)
          prop->type = EOLIAN_PROPERTY;
        else
          prop->type = EOLIAN_PROP_GET;
     }
   else
     {
        FILL_BASE(prop->set_base, ls, ls->line_number, ls->column);
        if (prop->type == EOLIAN_PROP_GET)
          prop->type = EOLIAN_PROPERTY;
        else
          prop->type = EOLIAN_PROP_SET;
     }
   eo_lexer_get(ls);
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
   if (is_get)
     {
        FILL_DOC(ls, prop, get_doc);
     }
   else
     {
        FILL_DOC(ls, prop, set_doc);
     }
   for (;;) switch (ls->t.kw)
     {
      case KW_return:
        CASE_LOCK(ls, return, "return")
        Eo_Ret_Def ret;
        parse_return(ls, &ret, is_get);
        pop_type(ls);
        if (ret.default_ret_val) pop_expr(ls);
        if (is_get)
          {
             prop->get_ret_type = ret.type;
             prop->get_return_doc = ret.doc;
             prop->get_ret_val = ret.default_ret_val;
             prop->get_return_warn_unused = ret.warn_unused;
          }
        else
          {
             prop->set_ret_type = ret.type;
             prop->set_return_doc = ret.doc;
             prop->set_ret_val = ret.default_ret_val;
             prop->set_return_warn_unused = ret.warn_unused;
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
_interface_virtual_set(Eo_Lexer *ls, Eolian_Function *foo_id)
{
   if (ls->tmp.kls->type != EOLIAN_CLASS_INTERFACE)
     return;

   if (foo_id->type == EOLIAN_PROP_GET || foo_id->type == EOLIAN_METHOD)
     foo_id->get_virtual_pure = EINA_TRUE;
   else if (foo_id->type == EOLIAN_PROP_SET)
     foo_id->set_virtual_pure = EINA_TRUE;
   else if (foo_id->type == EOLIAN_PROPERTY)
     foo_id->get_virtual_pure = foo_id->set_virtual_pure = EINA_TRUE;
}

static void
parse_property(Eo_Lexer *ls)
{
   int line, col;
   Eolian_Function *prop = NULL;
   Eina_Bool has_get       = EINA_FALSE, has_set    = EINA_FALSE,
             has_keys      = EINA_FALSE, has_values = EINA_FALSE,
             has_protected = EINA_FALSE, has_class  = EINA_FALSE,
             has_c_only    = EINA_FALSE, has_beta   = EINA_FALSE;
   prop = calloc(1, sizeof(Eolian_Function));
   prop->klass = ls->tmp.kls;
   prop->type = EOLIAN_UNRESOLVED;
   FILL_BASE(prop->base, ls, ls->line_number, ls->column);
   ls->tmp.kls->properties = eina_list_append(ls->tmp.kls->properties, prop);
   check(ls, TOK_VALUE);
   prop->name = eina_stringshare_ref(ls->t.value.s);
   eo_lexer_get(ls);
   for (;;) switch (ls->t.kw)
     {
      case KW_at_protected:
        CASE_LOCK(ls, protected, "protected qualifier")
        prop->scope = EOLIAN_SCOPE_PROTECTED;
        eo_lexer_get(ls);
        break;
      case KW_at_class:
        CASE_LOCK(ls, class, "class qualifier");
        prop->is_class = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_c_only:
        CASE_LOCK(ls, c_only, "c_only qualifier");
        prop->is_c_only = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_beta:
        CASE_LOCK(ls, beta, "beta qualifier");
        prop->is_beta = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      default:
        goto body;
     }
body:
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
   FILL_DOC(ls, prop, common_doc);
   for (;;) switch (ls->t.kw)
     {
      case KW_get:
        CASE_LOCK(ls, get, "get definition")
        parse_accessor(ls, prop);
        break;
      case KW_set:
        CASE_LOCK(ls, set, "set definition")
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
     prop->type = EOLIAN_PROPERTY;
   _interface_virtual_set(ls, prop);
}

static void
parse_method(Eo_Lexer *ls)
{
   int line, col;
   Eolian_Function *meth = NULL;
   Eina_Bool has_const       = EINA_FALSE, has_params = EINA_FALSE,
             has_return      = EINA_FALSE, has_legacy = EINA_FALSE,
             has_protected   = EINA_FALSE, has_class  = EINA_FALSE,
             has_eo          = EINA_FALSE, has_c_only = EINA_FALSE,
             has_beta        = EINA_FALSE;
   meth = calloc(1, sizeof(Eolian_Function));
   meth->klass = ls->tmp.kls;
   meth->type = EOLIAN_METHOD;
   FILL_BASE(meth->base, ls, ls->line_number, ls->column);
   ls->tmp.kls->methods = eina_list_append(ls->tmp.kls->methods, meth);
   check(ls, TOK_VALUE);
   meth->name = eina_stringshare_ref(ls->t.value.s);
   eo_lexer_get(ls);
   for (;;) switch (ls->t.kw)
     {
      case KW_at_protected:
        CASE_LOCK(ls, protected, "protected qualifier")
        meth->scope = EOLIAN_SCOPE_PROTECTED;
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
      case KW_at_c_only:
        CASE_LOCK(ls, c_only, "c_only qualifier");
        meth->is_c_only = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      case KW_at_beta:
        CASE_LOCK(ls, beta, "beta qualifier");
        meth->is_beta = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      default:
        goto body;
     }
body:
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
   FILL_DOC(ls, meth, common_doc);
   for (;;) switch (ls->t.kw)
     {
      case KW_return:
        CASE_LOCK(ls, return, "return")
        Eo_Ret_Def ret;
        parse_return(ls, &ret, EINA_FALSE);
        pop_type(ls);
        if (ret.default_ret_val) pop_expr(ls);
        meth->get_ret_type = ret.type;
        meth->get_return_doc = ret.doc;
        meth->get_ret_val = ret.default_ret_val;
        meth->get_return_warn_unused = ret.warn_unused;
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
   _interface_virtual_set(ls, meth);
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
             ls->tmp.kls->class_dtor_enable = EINA_TRUE;
             eo_lexer_get(ls);
          }
        else
          {
             check_kw_next(ls, KW_constructor);
             ls->tmp.kls->class_ctor_enable = EINA_TRUE;
          }
        check_next(ls, ';');
        return;
     }
   impl = calloc(1, sizeof(Eolian_Implement));
   FILL_BASE(impl->base, ls, iline, icol);
   ls->tmp.kls->implements = eina_list_append(ls->tmp.kls->implements, impl);
   switch (ls->t.kw)
     {
        case KW_at_virtual:
          impl->is_virtual = EINA_TRUE;
          eo_lexer_get(ls);
          break;
        case KW_at_auto:
          impl->is_auto = EINA_TRUE;
          eo_lexer_get(ls);
          break;
        case KW_at_empty:
          impl->is_empty = EINA_TRUE;
          eo_lexer_get(ls);
          break;
        default:
          break;
     }
   if (ls->t.token == '.')
     {
        if (!impl->is_virtual && !impl->is_auto && !impl->is_empty)
          goto fullclass;
        check_next(ls, '.');
        if ((ls->t.token != TOK_VALUE) || (ls->t.kw == KW_get || ls->t.kw == KW_set))
          eo_lexer_syntax_error(ls, "name expected");
        if (impl->is_virtual)
          impl->full_name = eina_stringshare_ref(ls->t.value.s);
        else
          impl->full_name = eina_stringshare_printf(".%s", ls->t.value.s);
        eo_lexer_get(ls);
        if (ls->t.token == '.')
          {
             eo_lexer_get(ls);
             if (ls->t.kw == KW_set)
               {
                  impl->is_prop_set = EINA_TRUE;
                  eo_lexer_get(ls);
               }
             else
               {
                  check_kw_next(ls, KW_get);
                  impl->is_prop_get = EINA_TRUE;
               }
          }
        check_next(ls, ';');
        return;
     }
fullclass:
   if ((ls->t.token != TOK_VALUE) || (ls->t.kw == KW_get || ls->t.kw == KW_set))
     eo_lexer_syntax_error(ls, "class name expected");
   buf = push_strbuf(ls);
   eina_strbuf_append(buf, ls->t.value.s);
   eo_lexer_get(ls);
   check_next(ls, '.');
   if ((ls->t.token != TOK_VALUE) || (ls->t.kw == KW_get || ls->t.kw == KW_set))
     eo_lexer_syntax_error(ls, "name or constructor/destructor expected");
   for (;;)
     {
        switch (ls->t.kw)
          {
           case KW_constructor:
           case KW_destructor:
             eina_strbuf_append_char(buf, '.');
             eina_strbuf_append(buf, eo_lexer_keyword_str_get(ls->t.kw));
             eo_lexer_get(ls);
             goto end;
           case KW_get:
             impl->is_prop_get = EINA_TRUE;
             eo_lexer_get(ls);
             goto end;
           case KW_set:
             impl->is_prop_set = EINA_TRUE;
             eo_lexer_get(ls);
             goto end;
           default:
             break;
          }
        eina_strbuf_append_char(buf, '.');
        check(ls, TOK_VALUE);
        eina_strbuf_append(buf, ls->t.value.s);
        eo_lexer_get(ls);
        if (ls->t.token != '.') break;
        eo_lexer_get(ls);
     }
end:
   check_next(ls, ';');
   impl->full_name = eina_stringshare_add(eina_strbuf_string_get(buf));
   pop_strbuf(ls);
}

static void
parse_constructor(Eo_Lexer *ls)
{
   Eina_Strbuf *buf = NULL;
   Eolian_Constructor *ctor = NULL;
   ctor = calloc(1, sizeof(Eolian_Constructor));
   FILL_BASE(ctor->base, ls, ls->line_number, ls->column);
   ls->tmp.kls->constructors = eina_list_append(ls->tmp.kls->constructors, ctor);
   if (ls->t.token == '.')
     {
        check_next(ls, '.');
        if (ls->t.token != TOK_VALUE)
          eo_lexer_syntax_error(ls, "name expected");
        ctor->full_name = eina_stringshare_printf("%s.%s",
                                                  ls->tmp.kls->full_name,
                                                  ls->t.value.s);
        eo_lexer_get(ls);
        if (ls->t.kw == KW_at_optional)
          {
             eo_lexer_get(ls);
             ctor->is_optional = EINA_TRUE;
          }
        check_next(ls, ';');
        return;
     }
   check(ls, TOK_VALUE);
   buf = push_strbuf(ls);
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
   if (ls->t.kw == KW_at_optional)
     {
        eo_lexer_get(ls);
        ctor->is_optional = EINA_TRUE;
     }
   check_next(ls, ';');
   ctor->full_name = eina_stringshare_add(eina_strbuf_string_get(buf));
   pop_strbuf(ls);
}

static void
parse_event(Eo_Lexer *ls)
{
   Eolian_Event *ev = calloc(1, sizeof(Eolian_Event));
   FILL_BASE(ev->base, ls, ls->line_number, ls->column);
   Eina_Strbuf *buf = push_strbuf(ls);
   ls->tmp.kls->events = eina_list_append(ls->tmp.kls->events, ev);
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
   ev->name = eina_stringshare_add(eina_strbuf_string_get(buf));
   pop_strbuf(ls);
   Eina_Bool has_scope = EINA_FALSE, has_beta = EINA_FALSE,
             has_hot   = EINA_FALSE;
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
      default:
        goto end;
     }
end:
   if (ls->t.token == ':')
     {
        eo_lexer_get(ls);
        ev->type = parse_type(ls);
        pop_type(ls);
     }
   check(ls, ';');
   eo_lexer_get(ls);
   FILL_DOC(ls, ev, doc);
   ev->klass = ls->tmp.kls;
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
             has_data          = EINA_FALSE,
             has_methods       = EINA_FALSE,
             has_implements    = EINA_FALSE,
             has_constructors  = EINA_FALSE,
             has_events        = EINA_FALSE;
   FILL_DOC(ls, ls->tmp.kls, doc);
   if (type == EOLIAN_CLASS_INTERFACE)
     {
        ls->tmp.kls->data_type = eina_stringshare_add("null");
     }
   for (;;) switch (ls->t.kw)
     {
      case KW_legacy_prefix:
        CASE_LOCK(ls, legacy_prefix, "legacy prefix definition")
        eo_lexer_get(ls);
        check_next(ls, ':');
        _validate_pfx(ls);
        ls->tmp.kls->legacy_prefix = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
        check_next(ls, ';');
        break;
      case KW_eo_prefix:
        CASE_LOCK(ls, eo_prefix, "eo prefix definition")
        eo_lexer_get(ls);
        check_next(ls, ':');
        _validate_pfx(ls);
        ls->tmp.kls->eo_prefix = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
        check_next(ls, ';');
        break;
      case KW_data:
        if (type == EOLIAN_CLASS_INTERFACE) return;
        CASE_LOCK(ls, data, "data definition")
        eo_lexer_get(ls);
        check_next(ls, ':');
        check(ls, TOK_VALUE);
        ls->tmp.kls->data_type = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
        check_next(ls, ';');
        break;
      case KW_methods:
        CASE_LOCK(ls, methods, "methods definition")
        parse_methods(ls);
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
_inherit_dep(Eo_Lexer *ls, Eina_Strbuf *buf)
{
   const char *fname, *iname;
   char *fnm;
   eina_strbuf_reset(buf);
   eo_lexer_context_push(ls);
   parse_name(ls, buf);
   iname = eina_strbuf_string_get(buf);
   fnm = database_class_to_filename(iname);
   if (compare_class_file(fnm, ls->filename))
     {
        char ebuf[PATH_MAX];
        free(fnm);
        eo_lexer_context_restore(ls);
        snprintf(ebuf, sizeof(ebuf), "class '%s' cannot inherit from itself",
                 iname);
        eo_lexer_syntax_error(ls, ebuf);
        return; /* unreachable (longjmp above), make static analysis shut up */
     }
   fname = eina_hash_find(_filenames, fnm);
   free(fnm);
   if (!fname)
     {
        char ebuf[PATH_MAX];
        eo_lexer_context_restore(ls);
        snprintf(ebuf, sizeof(ebuf), "unknown inherit '%s'", iname);
        eo_lexer_syntax_error(ls, ebuf);
     }
   _parse_dep(ls, fname, iname);
   ls->tmp.kls->inherits = eina_list_append(ls->tmp.kls->inherits,
                                            eina_stringshare_add(iname));
   eo_lexer_context_pop(ls);
}

static void
parse_class(Eo_Lexer *ls, Eolian_Class_Type type)
{
   Eolian_Declaration *decl;
   const char *bnm;
   char *fnm;
   Eina_Bool same;
   int line, col;
   Eina_Strbuf *buf = push_strbuf(ls);
   ls->tmp.kls = calloc(1, sizeof(Eolian_Class));
   FILL_BASE(ls->tmp.kls->base, ls, ls->line_number, ls->column);
   eo_lexer_get(ls);
   ls->tmp.kls->type = type;
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
   _fill_name(eina_stringshare_add(eina_strbuf_string_get(buf)),
              &ls->tmp.kls->full_name, &ls->tmp.kls->name,
              &ls->tmp.kls->namespaces);
   decl = (Eolian_Declaration *)eina_hash_find(_decls, ls->tmp.kls->full_name);
   if (decl)
     {
        eo_lexer_context_restore(ls);
        redef_error(ls, decl, EOLIAN_DECL_CLASS);
     }
   eo_lexer_context_pop(ls);
   pop_strbuf(ls);
   if (ls->t.token != '{')
     {
        line = ls->line_number;
        col = ls->column;
        check_next(ls, '(');
        if (ls->t.token != ')')
          {
              Eina_Strbuf *ibuf = push_strbuf(ls);
              _inherit_dep(ls, ibuf);
              while (test_next(ls, ','))
                _inherit_dep(ls, ibuf);
              pop_strbuf(ls);
          }
        check_match(ls, ')', '(', line, col);
     }
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
        {
           Eina_Bool is_eo = EINA_FALSE;
           Eina_Strbuf *buf = push_strbuf(ls);
           const char *found = NULL;
           char errbuf[PATH_MAX];
           eo_lexer_get(ls);
           check(ls, TOK_VALUE);
           eina_strbuf_append(buf, ls->t.value.s);
           eina_strbuf_append(buf, ".eot");
           if (!(found = eina_hash_find(_tfilenames, eina_strbuf_string_get(buf))))
             {
                size_t buflen = eina_strbuf_length_get(buf);
                eina_strbuf_remove(buf, buflen - 1, buflen);
                if (!(found = eina_hash_find(_filenames, eina_strbuf_string_get(buf))))
                  {
                     pop_strbuf(ls);
                     snprintf(errbuf, sizeof(errbuf),
                              "unknown import '%s'", ls->t.value.s);
                     eo_lexer_syntax_error(ls, errbuf);
                  }
                else is_eo = EINA_TRUE;
             }
           if (eina_hash_find(_parsingeos, found))
             {
                pop_strbuf(ls);
                snprintf(errbuf, sizeof(errbuf),
                         "cyclic import '%s'", ls->t.value.s);
                eo_lexer_syntax_error(ls, errbuf);
             }
           pop_strbuf(ls);
           if (!eo_parser_database_fill(found, !is_eo))
             {
                pop_strbuf(ls);
                snprintf(errbuf, sizeof(errbuf),
                         "error while parsing import '%s'", ls->t.value.s);
                eo_lexer_syntax_error(ls, errbuf);
             }
           pop_strbuf(ls);
           eo_lexer_get(ls);
           check_next(ls, ';');
           break;
        }
      case KW_type:
        {
           database_type_add(parse_typedef(ls));
           pop_typedecl(ls);
           break;
        }
      case KW_const:
      case KW_var:
        {
           database_var_add(parse_variable(ls, ls->t.kw == KW_var));
           ls->tmp.var = NULL;
           break;
        }
      case KW_struct:
      case KW_enum:
        {
           Eina_Bool is_enum = (ls->t.kw == KW_enum);
           const char *name;
           int line, col;
           Eolian_Declaration *decl;
           Eina_Bool has_extern;
           const char *freefunc;
           Eina_Strbuf *buf;
           eo_lexer_get(ls);
           parse_struct_attrs(ls, is_enum, &has_extern, &freefunc);
           buf = push_strbuf(ls);
           eo_lexer_context_push(ls);
           line = ls->line_number;
           col = ls->column;
           parse_name(ls, buf);
           name = eina_stringshare_add(eina_strbuf_string_get(buf));
           decl = (Eolian_Declaration *)eina_hash_find(_decls, name);
           if (decl)
             {
                eina_stringshare_del(name);
                eo_lexer_context_restore(ls);
                redef_error(ls, decl, is_enum ? EOLIAN_DECL_ENUM : EOLIAN_DECL_STRUCT);
             }
           eo_lexer_context_pop(ls);
           pop_strbuf(ls);
           if (!is_enum && ls->t.token == ';')
             {
                Eolian_Typedecl *def = push_typedecl(ls);
                def->is_extern = has_extern;
                def->type = EOLIAN_TYPEDECL_STRUCT_OPAQUE;
                def->freefunc = freefunc;
                pop_str(ls);
                _fill_name(name, &def->full_name, &def->name, &def->namespaces);
                eo_lexer_get(ls);
                FILL_DOC(ls, def, doc);
                FILL_BASE(def->base, ls, line, col);
                database_struct_add(def);
                pop_typedecl(ls);
                break;
             }
           if (is_enum)
             parse_enum(ls, name, has_extern, line, col);
           else
             parse_struct(ls, name, has_extern, line, col, freefunc);
           pop_typedecl(ls);
           break;
        }
      def:
      default:
        eo_lexer_syntax_error(ls, "invalid token");
        break;
     }
   return EINA_FALSE;
found_class:
   database_decl_add(ls->tmp.kls->full_name, EOLIAN_DECL_CLASS,
                     ls->tmp.kls->base.file, ls->tmp.kls);
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

Eina_Bool
eo_parser_walk(Eo_Lexer *ls, Eina_Bool eot)
{
   if (!setjmp(ls->err_jmp))
     {
        parse_chunk(ls, eot);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}


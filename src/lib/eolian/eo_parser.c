#include "eo_parser.h"

#define CASE_LOCK(ls, var, msg) \
   if (has_##var) \
     eo_lexer_syntax_error(ls, "double " msg); \
   has_##var = EINA_TRUE;

#define PARSE_SECTION \
   int line, col; \
   eo_lexer_get(ls); \
   line = ls->line_number; \
   col = ls->column; \
   check_next(ls, '{'); \
   while (ls->t.token != '}')

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

static void
pop_type(Eo_Lexer *ls)
{
   ls->tmp.type_defs = eina_list_remove_list(ls->tmp.type_defs, ls->tmp.type_defs);
}

static Eolian_Variable *
push_var(Eo_Lexer *ls)
{
   Eolian_Variable *def = calloc(1, sizeof(Eolian_Variable));
   ls->tmp.var_defs = eina_list_prepend(ls->tmp.var_defs, def);
   return def;
}

static void
pop_var(Eo_Lexer *ls)
{
   ls->tmp.var_defs = eina_list_remove_list(ls->tmp.var_defs, ls->tmp.var_defs);
}

static void
append_node(Eo_Lexer *ls, int type, void *def)
{
   Eo_Node *nd = calloc(1, sizeof(Eo_Node));
   nd->type  = type;
   nd->def   = def;
   ls->nodes = eina_list_append(ls->nodes, nd);
}

static Eina_Bool
compare_class_file(const char *fn_ext, const char *fn_noext)
{
   int fnlen = strlen(fn_ext);
   int cnlen = strlen(fn_noext);
   if (cnlen != (fnlen - 3))
     return EINA_FALSE;
   return !strncmp(fn_noext, fn_ext, cnlen);
}

static void
redef_error(Eo_Lexer *ls, Eolian_Type_Type type, Eolian_Type *old)
{
   char  buf[256];
   char fbuf[256] = { '\0' };
   const char *file = eina_stringshare_ref(ls->filename);
   if (file != old->base.file)
     snprintf(fbuf, sizeof(fbuf), " in file '%s'", old->base.file);
   eina_stringshare_del(file);
   snprintf(buf, sizeof(buf),
            "%s '%s' redefined (originally at line %d, column %d%s)",
            (type == EOLIAN_TYPE_ENUM) ? "enum" : ((type == EOLIAN_TYPE_STRUCT)
                ? "struct" : "type alias"),
            old->full_name, old->base.line, old->base.column, fbuf);
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

static Eina_List *
parse_name_list(Eo_Lexer *ls)
{
   Eina_Strbuf *buf = push_strbuf(ls);
   ls->tmp.str_items = NULL;
   parse_name(ls, buf);
   ls->tmp.str_items = eina_list_append(ls->tmp.str_items,
       eina_stringshare_add(eina_strbuf_string_get(buf)));
   while (test_next(ls, ','))
     {
        parse_name(ls, buf);
        ls->tmp.str_items = eina_list_append(ls->tmp.str_items,
           eina_stringshare_add(eina_strbuf_string_get(buf)));
     }
   pop_strbuf(ls);
   return ls->tmp.str_items;
}

#define NAMESPACE_PARSE(def, dname) \
   char *full_name = strdup(dname); \
   char *name = full_name, *dot = full_name; \
   def->full_name = dname; \
   do \
     { \
        dot = strchr(dot, '.'); \
        if (dot) \
          { \
             *dot = '\0'; \
             def->namespaces = eina_list_append(def->namespaces, \
                                               eina_stringshare_add(name)); \
             ++dot; \
             name = dot; \
          } \
     } \
   while (dot); \
   def->name = eina_stringshare_add(name); \
   free(full_name);

static void
_fill_type_name(Eolian_Type *tp, const char *type_name)
{
   NAMESPACE_PARSE(tp, type_name)
}

static void
_fill_variable_name(Eolian_Variable *var, const char *var_name)
{
   NAMESPACE_PARSE(var, var_name)
}

#undef NAMESPACE_PARSE

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
        Eolian_Expression *exp = parse_expr_bin(ls, UNARY_PRECEDENCE);
        pop_expr(ls);
        expr = push_expr(ls);
        expr->base.file = eina_stringshare_ref(ls->filename);
        expr->base.line = line;
        expr->base.column = col;
        expr->binop = unop;
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
           expr->base.file = eina_stringshare_ref(ls->filename);
           expr->base.line = line;
           expr->base.column = col;
           expr->type = ls->t.kw + 1; /* map Numbers from lexer to expr type */
           expr->value = ls->t.value;
           eo_lexer_get(ls);
           break;
        }
      case TOK_STRING:
        {
           int line = ls->line_number, col = ls->column;
           expr = push_expr(ls);
           expr->base.file = eina_stringshare_ref(ls->filename);
           expr->base.line = line;
           expr->base.column = col;
           expr->type = EOLIAN_EXPR_STRING;
           expr->value.s = eina_stringshare_ref(ls->t.value.s);
           eo_lexer_get(ls);
           break;
        }
      case TOK_CHAR:
        {
           int line = ls->line_number, col = ls->column;
           expr = push_expr(ls);
           expr->base.file = eina_stringshare_ref(ls->filename);
           expr->base.line = line;
           expr->base.column = col;
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
              case KW_enum:
              default:
                {
                   Eina_Strbuf *buf = push_strbuf(ls);
                   Eolian_Expression_Type tp = EOLIAN_EXPR_NAME;
                   if (ls->t.kw == KW_enum)
                     {
                        eo_lexer_get(ls);
                        tp = EOLIAN_EXPR_ENUM;
                     }
                   expr = push_expr(ls);
                   expr->type = tp;
                   parse_name(ls, buf);
                   expr->value.s = eina_stringshare_add(eina_strbuf_string_get
                       (buf));
                   pop_strbuf(ls);
                   break;
                }
             }
           expr->base.file = eina_stringshare_ref(ls->filename);
           expr->base.line = line;
           expr->base.column = col;
           break;
        }
      case '(':
        {
           int line = ls->line_number, col = ls->column;
           eo_lexer_get(ls);
           expr = parse_expr(ls);
           check_match(ls, ')', '(', line, col);
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
        bin->base.file = eina_stringshare_ref(ls->filename);
        bin->base.line = line;
        bin->base.column = col;
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
static Eolian_Type *parse_type_named_void(Eo_Lexer *ls, Eina_Bool allow_named);

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

static Eolian_Type *
parse_type_named(Eo_Lexer *ls, Eina_Bool allow_named)
{
   Eolian_Type *ret;
   eo_lexer_context_push(ls);
   ret = parse_type_named_void(ls, allow_named);
   if (ret->type == EOLIAN_TYPE_VOID)
     {
        eo_lexer_context_restore(ls);
        eo_lexer_syntax_error(ls, "non-void type expected");
     }
   eo_lexer_context_pop(ls);
   return ret;
}

static Eolian_Type *
parse_function_type(Eo_Lexer *ls)
{
   int line, col;
   Eolian_Type *def = push_type(ls);
   def->base.file = eina_stringshare_ref(ls->filename);
   def->base.line = ls->line_number;
   def->base.column = ls->column;
   eo_lexer_get(ls);
   if (ls->t.kw == KW_void)
     eo_lexer_get(ls);
   else
     {
        def->ret_type = parse_type_void(ls);
        pop_type(ls);
     }
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '(');
   if (ls->t.token != ')')
     {
        def->arguments = eina_list_append(def->arguments, parse_type(ls));
        pop_type(ls);
        while (test_next(ls, ','))
          {
             def->arguments = eina_list_append(def->arguments, parse_type(ls));
             pop_type(ls);
          }
     }
   check_match(ls, ')', '(', line, col);
   return def;
}

static void
_struct_field_free(Eolian_Struct_Field *def)
{
   if (def->base.file) eina_stringshare_del(def->base.file);
   database_type_del(def->type);
   if (def->comment) eina_stringshare_del(def->comment);
   free(def);
}

static Eolian_Type *
parse_struct(Eo_Lexer *ls, const char *name, Eina_Bool is_extern,
             int line, int column, const char *freefunc)
{
   int bline = ls->line_number, bcolumn = ls->column;
   Eolian_Type *def = push_type(ls);
   def->is_extern = is_extern;
   if (name) _fill_type_name(def, name);
   def->type = EOLIAN_TYPE_STRUCT;
   def->fields = eina_hash_string_small_new(EINA_FREE_CB(_struct_field_free));
   def->freefunc = freefunc;
   check_next(ls, '{');
   if (ls->t.token == TOK_COMMENT)
     {
        def->comment = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
     }
   while (ls->t.token != '}')
     {
        const char *fname;
        Eolian_Struct_Field *fdef;
        Eolian_Type *tp;
        int fline = ls->line_number, fcol = ls->column;
        check(ls, TOK_VALUE);
        if (eina_hash_find(def->fields, ls->t.value.s))
          eo_lexer_syntax_error(ls, "double field definition");
        fdef = calloc(1, sizeof(Eolian_Struct_Field));
        fname = eina_stringshare_ref(ls->t.value.s);
        eina_hash_add(def->fields, fname, fdef);
        def->field_names = eina_list_append(def->field_names, fname);
        eo_lexer_get(ls);
        check_next(ls, ':');
        tp = parse_type(ls);
        fdef->base.file = eina_stringshare_ref(ls->filename);
        fdef->base.line = fline;
        fdef->base.column = fcol;
        fdef->type = tp;
        pop_type(ls);
        check_next(ls, ';');
        if (ls->t.token == TOK_COMMENT)
          {
             fdef->comment = eina_stringshare_ref(ls->t.value.s);
             eo_lexer_get(ls);
          }
     }
   check_match(ls, '}', '{', bline, bcolumn);
   def->base.file = eina_stringshare_ref(ls->filename);
   def->base.line = line;
   def->base.column = column;
   if (name) database_struct_add(def);
   return def;
}

static void
_enum_field_free(Eolian_Enum_Field *def)
{
   if (def->base.file) eina_stringshare_del(def->base.file);
   database_expr_del(def->value);
   if (def->comment) eina_stringshare_del(def->comment);
   free(def);
}

static Eolian_Type *
parse_enum(Eo_Lexer *ls, const char *name, Eina_Bool is_extern,
           int line, int column)
{
   int bline = ls->line_number, bcolumn = ls->column;
   Eolian_Type *def = push_type(ls);
   def->is_extern = is_extern;
   _fill_type_name(def, name);
   def->type = EOLIAN_TYPE_ENUM;
   def->fields = eina_hash_string_small_new(EINA_FREE_CB(_enum_field_free));
   check_next(ls, '{');
   if (ls->t.token == TOK_COMMENT)
     {
        def->comment = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
     }
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
   Eolian_Expression *prev_exp = NULL;
   for (;;)
     {
        const char *fname;
        Eolian_Enum_Field *fdef;
        int fline = ls->line_number, fcol = ls->column;
        check(ls, TOK_VALUE);
        if (eina_hash_find(def->fields, ls->t.value.s))
          eo_lexer_syntax_error(ls, "double field definition");
        fdef = calloc(1, sizeof(Eolian_Enum_Field));
        fname = eina_stringshare_ref(ls->t.value.s);
        eina_hash_add(def->fields, fname, fdef);
        def->field_names = eina_list_append(def->field_names, fname);
        eo_lexer_get(ls);
        fdef->base.file = eina_stringshare_ref(ls->filename);
        fdef->base.line = fline;
        fdef->base.column = fcol;
        if (ls->t.token != '=')
          {
             if (!prev_exp)
               {
                  prev_exp = push_expr(ls);
                  prev_exp->base.file = eina_stringshare_ref(ls->filename);
                  prev_exp->base.line = -1;
                  prev_exp->base.column = -1;
                  prev_exp->type = EOLIAN_EXPR_INT;
                  prev_exp->value.i = 0;
                  fdef->value = prev_exp;
                  pop_expr(ls);
               }
          }
        else
          {
             ls->expr_mode = EINA_TRUE;
             eo_lexer_get(ls);
             fdef->value = parse_expr(ls);
             ls->expr_mode = EINA_FALSE;
             if (!prev_exp)
               prev_exp = fdef->value;
             pop_expr(ls);
          }
        Eina_Bool want_next = (ls->t.token == ',');
        if (want_next)
          eo_lexer_get(ls);
        if (ls->t.token == TOK_COMMENT)
          {
             fdef->comment = eina_stringshare_ref(ls->t.value.s);
             eo_lexer_get(ls);
          }
        if (!want_next)
          break;
     }
   check_match(ls, '}', '{', bline, bcolumn);
   def->base.file = eina_stringshare_ref(ls->filename);
   def->base.line = line;
   def->base.column = column;
   if (name) database_enum_add(def);
   return def;
}

static void
parse_struct_attrs(Eo_Lexer *ls, Eina_Bool is_enum, Eina_Bool allow_named,
                        Eina_Bool *is_extern, const char **freefunc)
{
   /* TODO: handle freefunc deref on error */
   Eina_Bool has_extern = EINA_FALSE, has_free = EINA_FALSE;
   *freefunc = NULL;
   *is_extern = EINA_FALSE;
   for (;;) switch (ls->t.kw)
     {
      case KW_at_extern:
        CASE_LOCK(ls, extern, "@extern qualifier")
        if (!allow_named)
          {
             if (is_enum)
               eo_lexer_syntax_error(ls,
                   "only enum declarations can be extern");
             else
               eo_lexer_syntax_error(ls,
                   "only named structs can be extern");
          }
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
           *freefunc = eina_stringshare_ref(ls->t.value.s);
           eo_lexer_get(ls);
           check_match(ls, ')', '(', pline, pcol);
           break;
        }
      default:
        return;
     }
}

static Eolian_Type *
parse_type_named_void(Eo_Lexer *ls, Eina_Bool allow_named)
{
   Eolian_Type *def;
   const char *ctype;
   const char *sname = NULL;
   Eina_Strbuf *buf;
   int line = ls->line_number, col = ls->column;
   switch (ls->t.kw)
     {
      case KW_const:
        {
           int line, col;
           eo_lexer_get(ls);
           line = ls->line_number;
           col = ls->column;
           check_next(ls, '(');
           def = parse_type_void(ls);
           def->base.file = eina_stringshare_ref(ls->filename);
           def->base.line = line;
           def->base.column = col;
           def->is_const = EINA_TRUE;
           check_match(ls, ')', '(', line, col);
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
           def = parse_type_void(ls);
           if (def->type != EOLIAN_TYPE_POINTER)
             {
                eo_lexer_context_restore(ls);
                eo_lexer_syntax_error(ls, "pointer type expected");
             }
           eo_lexer_context_pop(ls);
           def->base.file = eina_stringshare_ref(ls->filename);
           def->base.line = line;
           def->base.column = col;
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
           def = parse_type_void(ls);
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
           def->base.file = eina_stringshare_ref(ls->filename);
           def->base.line = line;
           def->base.column = col;
           check_match(ls, ')', '(', pline, pcolumn);
           goto parse_ptr;
        }
      case KW_struct:
      case KW_enum:
        {
           const char *freefunc;
           Eina_Bool has_extern;
           Eina_Bool is_enum = (ls->t.kw == KW_enum);
           eo_lexer_get(ls);
           parse_struct_attrs(ls, is_enum, allow_named, &has_extern, &freefunc);
           if (freefunc && !allow_named)
             check(ls, '{');
           if (!is_enum && (ls->t.token == '{'))
             {
                if (has_extern)
                  eo_lexer_syntax_error(ls, "extern anonymous struct");
                return parse_struct(ls, NULL, EINA_FALSE, line, col, freefunc);
             }
           buf = push_strbuf(ls);
           eo_lexer_context_push(ls);
           line = ls->line_number;
           col = ls->column;
           parse_name(ls, buf);
           sname = eina_stringshare_add(eina_strbuf_string_get(buf));
           pop_strbuf(ls);
           /* if we're extern and allow structs, gotta enforce it */
           if (allow_named && (has_extern || freefunc))
             check(ls, '{');
           if (allow_named && ls->t.token == '{')
             {
                Eolian_Type *tp = (Eolian_Type*)eina_hash_find(_structs,
                                                               sname);
                if (tp)
                  {
                     eina_stringshare_del(sname);
                     eo_lexer_context_restore(ls);
                     redef_error(ls, is_enum ? EOLIAN_TYPE_ENUM
                                             : EOLIAN_TYPE_STRUCT, tp);
                  }
                eo_lexer_context_pop(ls);
                if (is_enum)
                  return parse_enum(ls, sname, has_extern, line, col);
                return parse_struct(ls, sname, has_extern, line, col, freefunc);
             }
           eo_lexer_context_pop(ls);
           def = push_type(ls);
           def->type = is_enum ? EOLIAN_TYPE_REGULAR_ENUM
                               : EOLIAN_TYPE_REGULAR_STRUCT;
           _fill_type_name(def, sname);
           goto parse_ptr;
        }
      case KW_func:
        return parse_function_type(ls);
      default:
        break;
     }
   def = push_type(ls);
   def->base.file = eina_stringshare_ref(ls->filename);
   def->base.line = line;
   def->base.column = col;
   if (ls->t.kw == KW_void)
     {
        def->type = EOLIAN_TYPE_VOID;
        eo_lexer_get(ls);
     }
   else
     {
        def->type = EOLIAN_TYPE_REGULAR;
        check(ls, TOK_VALUE);
        ctype = eo_lexer_get_c_type(ls->t.kw);
        if (ctype)
          {
             _fill_type_name(def, eina_stringshare_add(ls->t.value.s));
             eo_lexer_get(ls);
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
                       if (!eolian_class_get_by_name(nm)
                        && !eolian_eo_file_parse(fname))
                         {
                            eo_lexer_context_restore(ls);
                            eo_lexer_syntax_error(ls,
                                "failed to parse dependency");
                         }
                       def->type = EOLIAN_TYPE_CLASS;
                    }
               }
             else
               {
                  eina_stringshare_del(bnm);
                  free(fnm);
                  def->type = EOLIAN_TYPE_CLASS;
               }
             eo_lexer_context_pop(ls);
             _fill_type_name(def, eina_stringshare_add(nm));
             pop_strbuf(ls);
          }
     }
parse_ptr:
   while (ls->t.token == '*')
     {
        Eolian_Type *pdef;
        pop_type(ls);
        pdef = push_type(ls);
        pdef->base.file = eina_stringshare_ref(ls->filename);
        pdef->base.line = ls->line_number;
        pdef->base.column = ls->column;
        pdef->base_type = def;
        pdef->type = EOLIAN_TYPE_POINTER;
        def = pdef;
        eo_lexer_get(ls);
     }
   if (ls->t.token == '<')
     {
        int line = ls->line_number, col = ls->column;
        eo_lexer_get(ls);
        def->subtypes = eina_list_append(def->subtypes, parse_type(ls));
        pop_type(ls);
        while (test_next(ls, ','))
          {
             def->subtypes = eina_list_append(def->subtypes, parse_type(ls));
             pop_type(ls);
          }
        check_match(ls, '>', '<', line, col);
     }
   return def;
}

static Eolian_Type *
parse_type_void(Eo_Lexer *ls)
{
   return parse_type_named_void(ls, EINA_FALSE);
}

static Eolian_Type *
parse_typedef(Eo_Lexer *ls)
{
   Eolian_Type *def = push_type(ls);
   Eina_Bool has_extern;
   const char *freefunc;
   Eina_Strbuf *buf;
   eo_lexer_get(ls);
   parse_struct_attrs(ls, EINA_FALSE, EINA_TRUE, &has_extern, &freefunc);
   def->freefunc = freefunc;
   def->type = EOLIAN_TYPE_ALIAS;
   def->is_extern = has_extern;
   buf = push_strbuf(ls);
   eo_lexer_context_push(ls);
   def->base.file = eina_stringshare_ref(ls->filename);
   def->base.line = ls->line_number;
   def->base.column = ls->column;
   parse_name(ls, buf);
   _fill_type_name(def, eina_stringshare_add(eina_strbuf_string_get(buf)));
   Eolian_Type *tp = (Eolian_Type*)eina_hash_find(_aliases, def->full_name);
   if (tp)
     {
        eo_lexer_context_restore(ls);
        redef_error(ls, EOLIAN_TYPE_ALIAS, tp);
     }
   eo_lexer_context_pop(ls);
   check_next(ls, ':');
   def->base_type = parse_type_named(ls, EINA_TRUE);
   pop_type(ls);
   check_next(ls, ';');
   if (ls->t.token == TOK_COMMENT)
     {
        def->comment = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
     }
   return def;
}

static Eolian_Variable *
parse_variable(Eo_Lexer *ls, Eina_Bool global)
{
   Eolian_Variable *def = push_var(ls);
   Eina_Bool has_extern = EINA_FALSE;
   Eina_Strbuf *buf;
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
   def->base.file = eina_stringshare_ref(ls->filename);
   def->base.line = ls->line_number;
   def->base.column = ls->column;
   parse_name(ls, buf);
   _fill_variable_name(def, eina_stringshare_add(eina_strbuf_string_get(buf)));
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
   if (ls->t.token == TOK_COMMENT)
     {
        def->comment = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
     }
   return def;
}

static void
parse_return(Eo_Lexer *ls, Eina_Bool allow_void)
{
   Eo_Ret_Def *ret = calloc(1, sizeof(Eo_Ret_Def));
   ls->tmp.ret_def = ret;
   eo_lexer_get(ls);
   check_next(ls, ':');
   if (allow_void)
     ret->type = parse_type_void(ls);
   else
     ret->type = parse_type(ls);
   pop_type(ls);
   if (ls->t.token == '(')
     {
        int line = ls->line_number, col = ls->column;
        ls->expr_mode = EINA_TRUE;
        eo_lexer_get(ls);
        ret->default_ret_val = parse_expr(ls);
        ls->expr_mode = EINA_FALSE;
        pop_expr(ls);
        check_match(ls, ')', '(', line, col);
     }
   if (ls->t.kw == KW_at_warn_unused)
     {
        ret->warn_unused = EINA_TRUE;
        eo_lexer_get(ls);
     }
   check_next(ls, ';');
   if (ls->t.token == TOK_COMMENT)
     {
        ret->comment = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
     }
}

static void
parse_param(Eo_Lexer *ls, Eina_Bool allow_inout)
{
   Eo_Param_Def *par = calloc(1, sizeof(Eo_Param_Def));
   par->base.file = eina_stringshare_ref(ls->filename);
   par->base.line = ls->line_number;
   par->base.column = ls->column;
   ls->tmp.param = par;
   if (allow_inout)
     {
        if (ls->t.kw == KW_at_in)
          {
             par->way = EOLIAN_IN_PARAM;
             eo_lexer_get(ls);
          }
        else if (ls->t.kw == KW_at_out)
          {
             par->way = EOLIAN_OUT_PARAM;
             eo_lexer_get(ls);
          }
        else if (ls->t.kw == KW_at_inout)
          {
             par->way = EOLIAN_INOUT_PARAM;
             eo_lexer_get(ls);
          }
        else
           par->way = EOLIAN_IN_PARAM;
     }
   if (par->way == EOLIAN_OUT_PARAM || par->way == EOLIAN_INOUT_PARAM)
     par->type = parse_type_void(ls);
   else
     par->type = parse_type(ls);
   pop_type(ls);
   check(ls, TOK_VALUE);
   par->name = eina_stringshare_ref(ls->t.value.s);
   eo_lexer_get(ls);
   if (ls->t.kw == KW_at_nonull)
     {
        par->nonull = EINA_TRUE;
        eo_lexer_get(ls);
     }
   check_next(ls, ';');
   if (ls->t.token == TOK_COMMENT)
     {
        par->comment = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
     }
}

static void
parse_legacy(Eo_Lexer *ls)
{
   eo_lexer_get(ls);
   check_next(ls, ':');
   check(ls, TOK_VALUE);
   ls->tmp.legacy_def = eina_stringshare_ref(ls->t.value.s);
   eo_lexer_get(ls);
   check_next(ls, ';');
}

static void
parse_attrs(Eo_Lexer *ls)
{
   Eo_Accessor_Param *acc = NULL;
   Eina_Bool has_const = EINA_FALSE;
   acc = calloc(1, sizeof(Eo_Accessor_Param));
   ls->tmp.accessor_param = acc;
   acc->name = eina_stringshare_ref(ls->t.value.s);
   eo_lexer_get(ls);
   check_next(ls, ':');
   check(ls, TOK_VALUE);
   for (;;) switch (ls->t.kw)
     {
      case KW_const:
        CASE_LOCK(ls, const, "const qualifier")
        acc->is_const = EINA_TRUE;
        eo_lexer_get(ls);
        break;
      default:
        if (ls->t.token != ';')
          eo_lexer_syntax_error(ls, "attribute expected");
        goto end;
     }
end:
   check_next(ls, ';');
}

static void
parse_accessor(Eo_Lexer *ls)
{
   int line, col;
   Eo_Accessor_Def *acc = NULL;
   Eina_Bool has_return = EINA_FALSE, has_legacy = EINA_FALSE,
             has_eo     = EINA_FALSE;
   acc = calloc(1, sizeof(Eo_Accessor_Def));
   acc->base.file = eina_stringshare_ref(ls->filename);
   acc->base.line = ls->line_number;
   acc->base.column = ls->column;
   ls->tmp.accessor = acc;
   acc->type = (ls->t.kw == KW_get) ? GETTER : SETTER;
   eo_lexer_get(ls);
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
   if (ls->t.token == TOK_COMMENT)
     {
        acc->comment = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
     }
   for (;;) switch (ls->t.kw)
     {
      case KW_return:
        CASE_LOCK(ls, return, "return")
        parse_return(ls, acc->type == GETTER);
        acc->ret = ls->tmp.ret_def;
        ls->tmp.ret_def = NULL;
        break;
      case KW_legacy:
        CASE_LOCK(ls, legacy, "legacy name")
        parse_legacy(ls);
        acc->legacy = ls->tmp.legacy_def;
        ls->tmp.legacy_def = NULL;
        break;
      case KW_eo:
        CASE_LOCK(ls, eo, "eo name")
        eo_lexer_get(ls);
        check_next(ls, ':');
        check_kw_next(ls, KW_null);
        check_next(ls, ';');
        acc->only_legacy = EINA_TRUE;
        break;
      default:
        if (ls->t.token != '}')
          {
             check(ls, TOK_VALUE);
             parse_attrs(ls);
             acc->params = eina_list_append(acc->params,
                                            ls->tmp.accessor_param);
             ls->tmp.accessor_param = NULL;
             /* this code path is disabled for the time being,
              * it's not used in regular eolian yet either...
             eo_lexer_lookahead(ls);
             if (ls->lookahead.token == ':')
               parse_attrs(ls);
             else
               parse_param(ls, EINA_TRUE);*/
          }
        else
          goto end;
     }
end:
   check_match(ls, '}', '{', line, col);
}

static void
parse_params(Eo_Lexer *ls, Eina_Bool allow_inout)
{
   PARSE_SECTION
     {
        parse_param(ls, allow_inout);
        ls->tmp.params = eina_list_append(ls->tmp.params, ls->tmp.param);
        ls->tmp.param = NULL;
     }
   check_match(ls, '}', '{', line, col);
}

static void
parse_property(Eo_Lexer *ls)
{
   int line, col;
   Eo_Property_Def *prop = NULL;
   Eina_Bool has_get       = EINA_FALSE, has_set    = EINA_FALSE,
             has_keys      = EINA_FALSE, has_values = EINA_FALSE,
             has_protected = EINA_FALSE, has_class  = EINA_FALSE,
             has_constructor = EINA_FALSE;
   prop = calloc(1, sizeof(Eo_Property_Def));
   prop->base.file = eina_stringshare_ref(ls->filename);
   prop->base.line = ls->line_number;
   prop->base.column = ls->column;
   ls->tmp.prop = prop;
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
      case KW_at_constructor:
        CASE_LOCK(ls, constructor, "constructor qualifier");
        eo_lexer_get(ls);
        break;

      default:
        goto body;
     }
body:
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
   if (ls->t.token == TOK_COMMENT)
     {
        /* just consume the comment for now */
        eo_lexer_get(ls);
     }
   for (;;) switch (ls->t.kw)
     {
      case KW_get:
        CASE_LOCK(ls, get, "get definition")
        parse_accessor(ls);
        prop->accessors = eina_list_append(prop->accessors,
                                           ls->tmp.accessor);
        ls->tmp.accessor = NULL;
        break;
      case KW_set:
        CASE_LOCK(ls, set, "set definition")
        parse_accessor(ls);
        prop->accessors = eina_list_append(prop->accessors,
                                           ls->tmp.accessor);
        ls->tmp.accessor = NULL;
        break;
      case KW_keys:
        CASE_LOCK(ls, keys, "keys definition")
        parse_params(ls, EINA_FALSE);
        prop->keys = ls->tmp.params;
        ls->tmp.params = NULL;
        break;
      case KW_values:
        CASE_LOCK(ls, values, "values definition")
        parse_params(ls, EINA_FALSE);
        prop->values = ls->tmp.params;
        ls->tmp.params = NULL;
        break;
      default:
        goto end;
     }
end:
   check_match(ls, '}', '{', line, col);
}

static void
parse_method(Eo_Lexer *ls, Eina_Bool ctor)
{
   int line, col;
   Eo_Method_Def *meth = NULL;
   Eina_Bool has_const       = EINA_FALSE, has_params = EINA_FALSE,
             has_return      = EINA_FALSE, has_legacy = EINA_FALSE,
             has_protected   = EINA_FALSE, has_class  = EINA_FALSE,
             has_constructor = EINA_FALSE, has_eo     = EINA_FALSE;
   meth = calloc(1, sizeof(Eo_Method_Def));
   meth->base.file = eina_stringshare_ref(ls->filename);
   meth->base.line = ls->line_number;
   meth->base.column = ls->column;
   ls->tmp.meth = meth;
   if (ctor)
     {
        if (ls->t.token != TOK_VALUE)
          eo_lexer_syntax_error(ls, "expected method name");
        meth->name = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
        for (;;) switch (ls->t.kw)
          {
           case KW_at_protected:
             CASE_LOCK(ls, protected, "protected qualifier")
             meth->scope = EOLIAN_SCOPE_PROTECTED;
             eo_lexer_get(ls);
             break;
           default:
             goto body;
          }
     }
   else
     {
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
             meth->obj_const = EINA_TRUE;
             eo_lexer_get(ls);
             break;
           case KW_at_class:
             CASE_LOCK(ls, class, "class qualifier");
             meth->is_class = EINA_TRUE;
             eo_lexer_get(ls);
             break;
           case KW_at_constructor:
             CASE_LOCK(ls, constructor, "constructor qualifier");
             eo_lexer_get(ls);
             break;
           default:
             goto body;
          }
     }
body:
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
   if (ls->t.token == TOK_COMMENT)
     {
        meth->comment = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
     }
   for (;;) switch (ls->t.kw)
     {
      case KW_return:
        CASE_LOCK(ls, return, "return")
        parse_return(ls, EINA_FALSE);
        meth->ret = ls->tmp.ret_def;
        ls->tmp.ret_def = NULL;
        break;
      case KW_legacy:
        CASE_LOCK(ls, legacy, "legacy name")
        parse_legacy(ls);
        meth->legacy = ls->tmp.legacy_def;
        ls->tmp.legacy_def = NULL;
        break;
      case KW_eo:
        CASE_LOCK(ls, eo, "eo name")
        eo_lexer_get(ls);
        check_next(ls, ':');
        check_kw_next(ls, KW_null);
        check_next(ls, ';');
        meth->only_legacy = EINA_TRUE;
        break;
      case KW_params:
        CASE_LOCK(ls, params, "params definition")
        parse_params(ls, EINA_TRUE);
        meth->params = ls->tmp.params;
        ls->tmp.params = NULL;
        break;
      default:
        goto end;
     }
end:
   check_match(ls, '}', '{', line, col);
}

static void
parse_implement(Eo_Lexer *ls, Eina_Bool iface)
{
   Eina_Strbuf *buf = NULL;
   Eolian_Implement *impl = NULL;
   buf = push_strbuf(ls);
   impl = calloc(1, sizeof(Eolian_Implement));
   impl->base.file = eina_stringshare_ref(ls->filename);
   impl->base.line = ls->line_number;
   impl->base.column = ls->column;
   ls->tmp.impl = impl;
   if (iface)
     check_kw(ls, KW_class);
   if (ls->t.kw == KW_class)
     {
        eina_strbuf_append(buf, "class.");
        eo_lexer_get(ls);
        check_next(ls, '.');
        if (ls->t.kw == KW_destructor)
          {
             eina_strbuf_append(buf, "destructor");
             eo_lexer_get(ls);
          }
        else
          {
             check_kw_next(ls, KW_constructor);
             eina_strbuf_append(buf, "constructor");
          }
        check_next(ls, ';');
        impl->full_name = eina_stringshare_add(eina_strbuf_string_get(buf));
        pop_strbuf(ls);
        return;
     }
   else if (ls->t.kw == KW_virtual)
     {
        eina_strbuf_append(buf, "virtual.");
        eo_lexer_get(ls);
        check_next(ls, '.');
        if ((ls->t.token != TOK_VALUE) || (ls->t.kw == KW_get || ls->t.kw == KW_set))
          eo_lexer_syntax_error(ls, "name expected");
        eina_strbuf_append(buf, ls->t.value.s);
        eo_lexer_get(ls);
        if (ls->t.token == '.')
          {
             eo_lexer_get(ls);
             if (ls->t.kw == KW_set)
               {
                  eina_strbuf_append(buf, ".set");
                  eo_lexer_get(ls);
               }
             else
               {
                  check_kw_next(ls, KW_get);
                  eina_strbuf_append(buf, ".get");
               }
          }
        check_next(ls, ';');
        impl->full_name = eina_stringshare_add(eina_strbuf_string_get(buf));
        pop_strbuf(ls);
        return;
     }
   if ((ls->t.token != TOK_VALUE) || (ls->t.kw == KW_get || ls->t.kw == KW_set))
     eo_lexer_syntax_error(ls, "class name expected");
   eina_strbuf_append(buf, ls->t.value.s);
   eo_lexer_get(ls);
   check_next(ls, '.');
   eina_strbuf_append(buf, ".");
   if ((ls->t.token != TOK_VALUE) || (ls->t.kw == KW_get || ls->t.kw == KW_set))
     eo_lexer_syntax_error(ls, "name or constructor/destructor expected");
   for (;;)
     {
        switch (ls->t.kw)
          {
           case KW_constructor:
           case KW_destructor:
           case KW_get:
           case KW_set:
             eina_strbuf_append(buf, eo_lexer_keyword_str_get(ls->t.kw));
             eo_lexer_get(ls);
             goto end;
           default:
             break;
          }
        check(ls, TOK_VALUE);
        eina_strbuf_append(buf, ls->t.value.s);
        eo_lexer_get(ls);
        if (ls->t.token != '.') break;
        eina_strbuf_append(buf, ".");
        eo_lexer_get(ls);
     }
end:
   check_next(ls, ';');
   impl->full_name = eina_stringshare_add(eina_strbuf_string_get(buf));
   pop_strbuf(ls);
}

static void
parse_event(Eo_Lexer *ls)
{
   Eolian_Event *ev = calloc(1, sizeof(Eolian_Event));
   ev->base.file = eina_stringshare_ref(ls->filename);
   ev->base.line = ls->line_number;
   ev->base.column = ls->column;
   Eina_Strbuf *buf = push_strbuf(ls);
   ls->tmp.event = ev;
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
   if (ls->t.kw == KW_at_private)
     {
        ev->scope = EOLIAN_SCOPE_PRIVATE;
        eo_lexer_get(ls);
     }
   else if (ls->t.kw == KW_at_protected)
     {
        ev->scope = EOLIAN_SCOPE_PROTECTED;
        eo_lexer_get(ls);
     }
   if (ls->t.token == ':')
     {
        eo_lexer_get(ls);
        ev->type = parse_type(ls);
        pop_type(ls);
     }
   check(ls, ';');
   eo_lexer_get(ls);
   if (ls->t.token == TOK_COMMENT)
     {
        ev->comment = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
     }
}

static void
parse_constructors(Eo_Lexer *ls)
{
   PARSE_SECTION
     {
        parse_method(ls, EINA_TRUE);
        ls->tmp.kls->constructors = eina_list_append(ls->tmp.kls->constructors,
                                                     ls->tmp.meth);
        ls->tmp.meth = NULL;
     }
   check_match(ls, '}', '{', line, col);
}

static void
parse_methods(Eo_Lexer *ls)
{
   PARSE_SECTION
     {
        parse_method(ls, EINA_FALSE);
        ls->tmp.kls->methods = eina_list_append(ls->tmp.kls->methods,
                                                ls->tmp.meth);
        ls->tmp.meth = NULL;
     }
   check_match(ls, '}', '{', line, col);
}

static void
parse_properties(Eo_Lexer *ls)
{
   PARSE_SECTION
     {
        parse_property(ls);
        ls->tmp.kls->properties = eina_list_append(ls->tmp.kls->properties,
                                                   ls->tmp.prop);
        ls->tmp.prop = NULL;
     }
   check_match(ls, '}', '{', line, col);
}

static void
parse_implements(Eo_Lexer *ls, Eina_Bool iface)
{
   PARSE_SECTION
     {
        parse_implement(ls, iface);
        ls->tmp.kls->implements = eina_list_append(ls->tmp.kls->implements,
                                                   ls->tmp.impl);
        ls->tmp.impl = NULL;
     }
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
     {
        parse_event(ls);
        ls->tmp.kls->events = eina_list_append(ls->tmp.kls->events,
                                               ls->tmp.event);
        ls->tmp.event = NULL;
     }
   check_match(ls, '}', '{', line, col);
}

static void
parse_class_body(Eo_Lexer *ls, Eina_Bool allow_ctors, Eolian_Class_Type type)
{
   Eina_Bool has_legacy_prefix = EINA_FALSE,
             has_eo_prefix     = EINA_FALSE,
             has_data          = EINA_FALSE,
             has_constructors  = EINA_FALSE,
             has_properties    = EINA_FALSE,
             has_methods       = EINA_FALSE,
             has_implements    = EINA_FALSE,
             has_events        = EINA_FALSE;
   if (ls->t.token == TOK_COMMENT)
     {
        ls->tmp.kls->comment = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
     }
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
        check(ls, TOK_VALUE);
        ls->tmp.kls->legacy_prefix = eina_stringshare_ref(ls->t.value.s);
        eo_lexer_get(ls);
        check_next(ls, ';');
        break;
      case KW_eo_prefix:
        CASE_LOCK(ls, eo_prefix, "eo prefix definition")
        eo_lexer_get(ls);
        check_next(ls, ':');
        check(ls, TOK_VALUE);
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
      case KW_constructors:
        if (!allow_ctors)
          return;
        CASE_LOCK(ls, constructors, "constructors definition")
        parse_constructors(ls);
        break;
      case KW_properties:
        CASE_LOCK(ls, properties, "properties definition")
        parse_properties(ls);
        break;
      case KW_methods:
        CASE_LOCK(ls, methods, "methods definition")
        parse_methods(ls);
        break;
      case KW_implements:
        CASE_LOCK(ls, implements, "implements definition")
        parse_implements(ls, type == EOLIAN_CLASS_INTERFACE);
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
parse_class(Eo_Lexer *ls, Eina_Bool allow_ctors, Eolian_Class_Type type)
{
   const char *bnm;
   char *fnm;
   Eina_Bool same;
   int line, col;
   Eina_Strbuf *buf = push_strbuf(ls);
   ls->tmp.kls = calloc(1, sizeof(Eo_Class_Def));
   ls->tmp.kls->base.file = eina_stringshare_ref(ls->filename);
   ls->tmp.kls->base.line = ls->line_number;
   ls->tmp.kls->base.column = ls->column;
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
   eo_lexer_context_pop(ls);
   ls->tmp.kls->name = eina_stringshare_add(eina_strbuf_string_get(buf));
   pop_strbuf(ls);
   if (ls->t.token != '{')
     {
        line = ls->line_number;
        col = ls->column;
        check_next(ls, '(');
        if (ls->t.token != ')')
          {
             ls->tmp.kls->inherits = parse_name_list(ls);
             ls->tmp.str_items = NULL;
          }
        check_match(ls, ')', '(', line, col);
     }
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
   parse_class_body(ls, allow_ctors, type);
   check_match(ls, '}', '{', line, col);
}

static Eina_Bool
parse_unit(Eo_Lexer *ls, Eina_Bool eot)
{
   switch (ls->t.kw)
     {
      case KW_abstract:
        if (eot) goto def;
        parse_class(ls, EINA_TRUE, EOLIAN_CLASS_ABSTRACT);
        goto found_class;
      case KW_class:
        if (eot) goto def;
        parse_class(ls, EINA_TRUE, EOLIAN_CLASS_REGULAR);
        goto found_class;
      case KW_mixin:
        if (eot) goto def;
        parse_class(ls, EINA_FALSE, EOLIAN_CLASS_MIXIN);
        goto found_class;
      case KW_interface:
        if (eot) goto def;
        parse_class(ls, EINA_FALSE, EOLIAN_CLASS_INTERFACE);
        goto found_class;
      case KW_type:
        {
           database_type_add(parse_typedef(ls));
           pop_type(ls);
           break;
        }
      case KW_const:
      case KW_var:
        {
           database_var_add(parse_variable(ls, ls->t.kw == KW_var));
           pop_var(ls);
           break;
        }
      case KW_struct:
      case KW_enum:
        {
           Eina_Bool is_enum = (ls->t.kw == KW_enum);
           const char *name;
           int line, col;
           Eolian_Type *tp;
           Eina_Bool has_extern;
           const char *freefunc;
           Eina_Strbuf *buf;
           eo_lexer_get(ls);
           parse_struct_attrs(ls, is_enum, EINA_TRUE, &has_extern, &freefunc);
           buf = push_strbuf(ls);
           eo_lexer_context_push(ls);
           line = ls->line_number;
           col = ls->column;
           parse_name(ls, buf);
           name = eina_stringshare_add(eina_strbuf_string_get(buf));
           tp = (Eolian_Type*)eina_hash_find(is_enum ? _enums
                                                     : _structs, name);
           if (tp)
             {
                eina_stringshare_del(name);
                eo_lexer_context_restore(ls);
                redef_error(ls, is_enum ? EOLIAN_TYPE_ENUM
                                        : EOLIAN_TYPE_STRUCT, tp);
             }
           eo_lexer_context_pop(ls);
           pop_strbuf(ls);
           if (ls->t.token == ';')
             {
                Eolian_Type *def = push_type(ls);
                def->is_extern = has_extern;
                def->type = EOLIAN_TYPE_STRUCT_OPAQUE;
                def->freefunc = freefunc;
                _fill_type_name(def, name);
                eo_lexer_get(ls);
                if (ls->t.token == TOK_COMMENT)
                  {
                     def->comment = eina_stringshare_ref(ls->t.value.s);
                     eo_lexer_get(ls);
                  }
                def->base.file = eina_stringshare_ref(ls->filename);
                def->base.line = line;
                def->base.column = col;
                database_struct_add(def);
                pop_type(ls);
                break;
             }
           if (is_enum)
             parse_enum(ls, name, has_extern, line, col);
           else
             parse_struct(ls, name, has_extern, line, col, freefunc);
           pop_type(ls);
           break;
        }
      def:
      default:
        eo_lexer_syntax_error(ls, "invalid token");
        break;
     }
   return EINA_FALSE;
found_class:
   append_node(ls, NODE_CLASS, ls->tmp.kls);
   ls->tmp.kls = NULL;
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


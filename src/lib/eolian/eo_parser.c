#include "eo_parser.h"
#include "eolian_database.h"

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
                      "'%s' expected (to close '%s' at line %d at column %d)",
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

static void
append_node(Eo_Lexer *ls, int type, void *def)
{
   Eo_Node *nd = calloc(1, sizeof(Eo_Node));
   nd->type  = type;
   nd->def   = def;
   ls->nodes = eina_list_append(ls->nodes, nd);
}

static Eina_Strbuf *
parse_name(Eo_Lexer *ls, Eina_Strbuf *buf)
{
   check(ls, TOK_VALUE);
   eina_strbuf_reset(buf);
   for (;;)
     {
        eina_strbuf_append(buf, ls->t.value);
        eo_lexer_get(ls);
        if (ls->t.token != '.') break;
        eo_lexer_get(ls);
        eina_strbuf_append(buf, ".");
        check(ls, TOK_VALUE);
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

static Eo_Type_Def *parse_type_void(Eo_Lexer *ls);
static Eo_Type_Def *parse_type_struct(Eo_Lexer *ls, Eina_Bool allow_struct,
                                      Eina_Bool allow_anon);

static Eo_Type_Def *
parse_type(Eo_Lexer *ls)
{
   int line = ls->line_number, column = ls->column;
   Eo_Type_Def *ret = parse_type_void(ls);
   if (ret->type == EOLIAN_TYPE_VOID)
     {
        ls->line_number = line;
        ls->column = column;
        eo_lexer_syntax_error(ls, "non-void type expected");
     }
   return ret;
}

static Eo_Type_Def *
parse_type_struct_nonvoid(Eo_Lexer *ls, Eina_Bool allow_struct,
                          Eina_Bool allow_anon)
{
   int line = ls->line_number, column = ls->column;
   Eo_Type_Def *ret = parse_type_struct(ls, allow_struct, allow_anon);
   if (ret->type == EOLIAN_TYPE_VOID)
     {
        ls->line_number = line;
        ls->column = column;
        eo_lexer_syntax_error(ls, "non-void type expected");
     }
   return ret;
}

static Eo_Type_Def *
parse_function_type(Eo_Lexer *ls)
{
   int line, col;
   Eo_Type_Def *def = calloc(1, sizeof(Eo_Type_Def));
   ls->tmp.type_def = def;
   eo_lexer_get(ls);
   if (ls->t.kw == KW_void)
     eo_lexer_get(ls);
   else
     def->ret_type = parse_type_void(ls);
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '(');
   if (ls->t.token != ')')
     {
        def->arguments = eina_list_append(def->arguments, parse_type(ls));
        while (test_next(ls, ','))
          def->arguments = eina_list_append(def->arguments, parse_type(ls));
     }
   check_match(ls, ')', '(', line, col);
   return def;
}

static Eo_Type_Def *
parse_struct(Eo_Lexer *ls, const char *name)
{
   int line = ls->line_number, column = ls->column;
   Eo_Type_Def *def = calloc(1, sizeof(Eo_Type_Def));
   ls->tmp.type_def = def;
   def->name = name;
   def->type = EOLIAN_TYPE_STRUCT;
   def->fields = eina_hash_string_small_new(EINA_FREE_CB(eo_definitions_type_free));
   check_next(ls, '{');
   while (ls->t.token != '}')
     {
        const char *fname;
        check(ls, TOK_VALUE);
        if (eina_hash_find(def->fields, ls->t.value))
          eo_lexer_syntax_error(ls, "double field definition");
        fname = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
        check_next(ls, ':');
        eina_hash_add(def->fields, fname, parse_type_struct_nonvoid(ls,
                      EINA_TRUE, EINA_FALSE));
        eina_stringshare_del(fname);
        check_next(ls, ';');
        if (ls->t.token == TOK_COMMENT)
          {
             eo_lexer_get(ls);
          }
     }
   check_match(ls, '}', '{', line, column);
   if (name) append_node(ls, NODE_STRUCT, def);
   return def;
}

static Eo_Type_Def *
parse_type_struct(Eo_Lexer *ls, Eina_Bool allow_struct, Eina_Bool allow_anon)
{
   Eina_Bool    has_struct = EINA_FALSE;
   Eo_Type_Def *def;
   const char  *ctype;
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
           def->is_const = EINA_TRUE;
           check_match(ls, ')', '(', line, col);
           goto parse_ptr;
        }
      case KW_own:
        {
           int sline = ls->line_number, scolumn = ls->column, line, column;
           eo_lexer_get(ls);
           line = ls->line_number;
           column = ls->column;
           check_next(ls, '(');
           def = parse_type_void(ls);
           if (def->type != EOLIAN_TYPE_POINTER)
             {
                ls->line_number = sline;
                ls->column = scolumn;
                eo_lexer_syntax_error(ls, "pointer type expected");
             }
           def->is_own = EINA_TRUE;
           check_match(ls, ')', '(', line, column);
           goto parse_ptr;
        }
      case KW_struct:
        eo_lexer_get(ls);
        if (allow_struct)
          {
             if (allow_anon && ls->t.token == '{')
               return parse_struct(ls, NULL);
             if (eo_lexer_lookahead(ls) == '{')
               {
                  const char *name;
                  check(ls, TOK_VALUE);
                  if (eo_lexer_get_c_type(ls->t.kw))
                    eo_lexer_syntax_error(ls, "invalid struct name");
                  name = eina_stringshare_add(ls->t.value);
                  eo_lexer_get(ls);
                  return parse_struct(ls, name);
               }
          }
        has_struct = EINA_TRUE;
        break;
      case KW_func:
        return parse_function_type(ls);
      default:
        break;
     }
   def = calloc(1, sizeof(Eo_Type_Def));
   ls->tmp.type_def = def;
   if (ls->t.kw == KW_void && !has_struct)
     def->type = EOLIAN_TYPE_VOID;
   else
     {
        def->type = EOLIAN_TYPE_REGULAR;
        def->is_const = EINA_FALSE;
        check(ls, TOK_VALUE);
        ctype = eo_lexer_get_c_type(ls->t.kw);
        if (ctype && has_struct)
          eo_lexer_syntax_error(ls, "invalid struct name");
        if (has_struct)
          {
             Eina_Strbuf *buf = eina_strbuf_new();
             eina_strbuf_append(buf, "struct ");
             eina_strbuf_append(buf, ls->t.value);
             def->name = eina_stringshare_add(eina_strbuf_string_get(buf));
             eina_strbuf_free(buf);
          }
        else
          def->name = eina_stringshare_add(ctype ? ctype : ls->t.value);
     }
   eo_lexer_get(ls);
parse_ptr:
   while (ls->t.token == '*')
     {
        Eo_Type_Def *pdef = calloc(1, sizeof(Eo_Type_Def));
        ls->tmp.type_def = pdef;
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
        while (test_next(ls, ','))
           def->subtypes = eina_list_append(def->subtypes, parse_type(ls));
        check_match(ls, '>', '<', line, col);
     }
   return def;
}

static Eo_Type_Def *
parse_type_void(Eo_Lexer *ls)
{
   return parse_type_struct(ls, EINA_FALSE, EINA_FALSE);
}

static void
parse_typedef(Eo_Lexer *ls)
{
   ls->tmp.typedef_def = calloc(1, sizeof(Eo_Typedef_Def));
   eo_lexer_get(ls);
   check(ls, TOK_VALUE);
   ls->tmp.typedef_def->alias = eina_stringshare_add(ls->t.value);
   eo_lexer_get(ls);
   test_next(ls, ':');
   ls->tmp.typedef_def->type = parse_type_struct_nonvoid(ls, EINA_TRUE,
                                                         EINA_TRUE);
   ls->tmp.type_def = NULL;
   check_next(ls, ';');
}

static void
parse_return(Eo_Lexer *ls, Eina_Bool allow_void)
{
   Eo_Ret_Def *ret = calloc(1, sizeof(Eo_Ret_Def));
   ls->tmp.ret_def = ret;
   eo_lexer_get(ls);
   if (allow_void)
     ret->type = parse_type_void(ls);
   else
     ret->type = parse_type(ls);
   ls->tmp.type_def = NULL;
   if (ls->t.token == '(')
     {
        int line = ls->line_number, col = ls->column;
        eo_lexer_get_balanced(ls, '(', ')');
        ret->dflt_ret_val = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
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
        ret->comment = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
     }
}

static void
parse_param(Eo_Lexer *ls, Eina_Bool allow_inout)
{
   Eo_Param_Def *par = calloc(1, sizeof(Eo_Param_Def));
   ls->tmp.param = par;
   if (allow_inout)
     {
        if (ls->t.kw == KW_at_in)
          {
             par->way = PARAM_IN;
             eo_lexer_get(ls);
          }
        else if (ls->t.kw == KW_at_out)
          {
             par->way = PARAM_OUT;
             eo_lexer_get(ls);
          }
        else if (ls->t.kw == KW_at_inout)
          {
             par->way = PARAM_INOUT;
             eo_lexer_get(ls);
          }
        else
           par->way = PARAM_IN;
     }
   if (par->way == PARAM_OUT)
     par->type = parse_type_void(ls);
   else
     par->type = parse_type(ls);
   ls->tmp.type_def = NULL;
   check(ls, TOK_VALUE);
   par->name = eina_stringshare_add(ls->t.value);
   eo_lexer_get(ls);
   if (ls->t.kw == KW_at_nonull)
     {
        par->nonull = EINA_TRUE;
        eo_lexer_get(ls);
     }
   check_next(ls, ';');
   if (ls->t.token == TOK_COMMENT)
     {
        par->comment = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
     }
}

static void
parse_legacy(Eo_Lexer *ls)
{
   eo_lexer_get(ls);
   check(ls, TOK_VALUE);
   ls->tmp.legacy_def = eina_stringshare_add(ls->t.value);
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
   acc->name = eina_stringshare_add(ls->t.value);
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
   Eina_Bool has_return = EINA_FALSE, has_legacy = EINA_FALSE;
   acc = calloc(1, sizeof(Eo_Accessor_Def));
   ls->tmp.accessor = acc;
   acc->type = (ls->t.kw == KW_get) ? GETTER : SETTER;
   eo_lexer_get(ls);
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
   if (ls->t.token == TOK_COMMENT)
     {
        acc->comment = eina_stringshare_add(ls->t.value);
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
   Eina_Bool has_get  = EINA_FALSE, has_set    = EINA_FALSE,
             has_keys = EINA_FALSE, has_values = EINA_FALSE;
   prop = calloc(1, sizeof(Eo_Property_Def));
   ls->tmp.prop = prop;
   if (ls->t.kw == KW_protected)
     {
        prop->scope = EOLIAN_SCOPE_PROTECTED;
        eo_lexer_get(ls);
     }
   prop->name = eina_stringshare_add(ls->t.value);
   eo_lexer_get(ls);
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
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
             has_return      = EINA_FALSE, has_legacy = EINA_FALSE;
   meth = calloc(1, sizeof(Eo_Method_Def));
   ls->tmp.meth = meth;
   if (ctor)
     {
        if (ls->t.token != TOK_VALUE)
          eo_lexer_syntax_error(ls, "expected method name");
        meth->type = METH_CONSTRUCTOR;
        meth->name = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
     }
   else
     {
        meth->type = METH_REGULAR;
        if (ls->t.kw == KW_protected)
          {
             meth->scope = EOLIAN_SCOPE_PROTECTED;
             eo_lexer_get(ls);
          }
        check(ls, TOK_VALUE);
        meth->name = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
     }
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
   if (ls->t.token == TOK_COMMENT)
     {
        meth->comment = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
     }
   for (;;) switch (ls->t.kw)
     {
      case KW_const:
        CASE_LOCK(ls, const, "const qualifier")
        meth->obj_const = EINA_TRUE;
        eo_lexer_get(ls);
        check_next(ls, ';');
        break;
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
   Eo_Implement_Def *impl = NULL;
   buf = push_strbuf(ls);
   impl = calloc(1, sizeof(Eo_Implement_Def));
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
        impl->meth_name = eina_stringshare_add(eina_strbuf_string_get(buf));
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
        eina_strbuf_append(buf, ls->t.value);
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
        impl->meth_name = eina_stringshare_add(eina_strbuf_string_get(buf));
        pop_strbuf(ls);
        return;
     }
   if ((ls->t.token != TOK_VALUE) || (ls->t.kw == KW_get || ls->t.kw == KW_set))
     eo_lexer_syntax_error(ls, "class name expected");
   eina_strbuf_append(buf, ls->t.value);
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
        eina_strbuf_append(buf, ls->t.value);
        eo_lexer_get(ls);
        if (ls->t.token != '.') break;
        eina_strbuf_append(buf, ".");
        eo_lexer_get(ls);
     }
end:
   check_next(ls, ';');
   impl->meth_name = eina_stringshare_add(eina_strbuf_string_get(buf));
   pop_strbuf(ls);
}

static void
parse_event(Eo_Lexer *ls)
{
   Eo_Event_Def *ev = calloc(1, sizeof(Eo_Event_Def));
   ls->tmp.event = ev;
   /* code path not in use yet
   if (ls->t.kw == KW_private)
     {
        eo_lexer_get_ident(ls, "_,");
     }*/
   check(ls, TOK_VALUE);
   ev->name = eina_stringshare_add(ls->t.value);
   eo_lexer_get(ls);
   if (ls->t.token == '(')
     {
        int line = ls->line_number, col = ls->column;
        eo_lexer_get_balanced(ls, '(', ')');
        ev->type = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
        check_match(ls, ')', '(', line, col);
     }
   check(ls, ';');
   eo_lexer_get_ident(ls, "_,");
   if (ls->t.token == TOK_COMMENT)
     {
        ev->comment = eina_stringshare_add(ls->t.value);
        eo_lexer_get_ident(ls, "_,");
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
   eo_lexer_get_ident(ls, "_,");
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
        ls->tmp.kls->comment = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
     }
   for (;;) switch (ls->t.kw)
     {
      case KW_legacy_prefix:
        CASE_LOCK(ls, legacy_prefix, "legacy prefix definition")
        eo_lexer_get(ls);
        check_next(ls, ':');
        check(ls, TOK_VALUE);
        ls->tmp.kls->legacy_prefix = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
        check_next(ls, ';');
        break;
      case KW_eo_prefix:
        CASE_LOCK(ls, eo_prefix, "eo prefix definition")
        eo_lexer_get(ls);
        check_next(ls, ':');
        check(ls, TOK_VALUE);
        ls->tmp.kls->eo_prefix = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
        check_next(ls, ';');
        break;
      case KW_data:
        CASE_LOCK(ls, data, "data definition")
        eo_lexer_get(ls);
        check(ls, ':');
        eo_lexer_get_until(ls, ';');
        ls->tmp.kls->data_type = eina_stringshare_add(ls->t.value);
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
   int line, col;
   Eina_Strbuf *buf = push_strbuf(ls);
   ls->tmp.kls = calloc(1, sizeof(Eo_Class_Def));
   eo_lexer_get(ls);
   ls->tmp.kls->type = type;
   parse_name(ls, buf);
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

static void
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
           parse_typedef(ls);
           append_node(ls, NODE_TYPEDEF, ls->tmp.typedef_def);
           ls->tmp.typedef_def = NULL;
           break;
        }
      case KW_struct:
        {
           const char *name;
           eo_lexer_get(ls);
           check(ls, TOK_VALUE);
           if (eo_lexer_get_c_type(ls->t.kw))
             eo_lexer_syntax_error(ls, "invalid struct name");
           name = eina_stringshare_add(ls->t.value);
           eo_lexer_get(ls);
           parse_struct(ls, name);
           ls->tmp.type_def = NULL;
        }
      def:
      default:
        eo_lexer_syntax_error(ls, "invalid token");
        break;
     }
   return;
found_class:
   append_node(ls, NODE_CLASS, ls->tmp.kls);
   ls->tmp.kls = NULL;
}

static void
parse_chunk(Eo_Lexer *ls, Eina_Bool eot)
{
   while (ls->t.token != TOK_EOF)
     parse_unit(ls, eot);
}

static char *_accessor_type_str[ACCESSOR_TYPE_LAST] = { "setter", "getter"   };
static char *    _param_way_str[    PARAM_WAY_LAST] = { "IN", "OUT", "INOUT" };

typedef struct
{
   Eina_List *subtypes;
   Eina_Stringshare *name;
   Eina_Bool is_own :1;
} _Parameter_Type;

static void
_print_type(FILE *f, Eo_Type_Def *tp)
{
   Eina_List *l;
   Eo_Type_Def *stp;
   if (tp->is_own)
     fputs("own(", f);
   if (tp->is_const)
     fputs("const(", f);
   if (tp->type == EOLIAN_TYPE_REGULAR)
     fputs(tp->name, f);
   else if (tp->type == EOLIAN_TYPE_POINTER)
     {
        _print_type(f, tp->base_type);
        fputc('*', f);
     }
   else if (tp->type == EOLIAN_TYPE_FUNCTION)
     {
        Eina_Bool first = EINA_TRUE;
        fputs("fn", f);
        if (tp->ret_type)
          {
             fputs(" -> ", f);
             _print_type(f, tp->ret_type);
          }
        fputs(" (", f);
        EINA_LIST_FOREACH(tp->arguments, l, stp)
          {
             if (!first) fputs(", ", f);
             first = EINA_FALSE;
             _print_type(f, stp);
          }
        fputc(')', f);
     }
   if (tp->is_own)
     fputc(')', f);
   if (tp->is_const)
     fputc(')', f);
}

static void
_dump_class(Eo_Class_Def *kls)
{
   const char *s;
   Eina_List *l, *m;

   Eo_Property_Def *prop;
   Eo_Method_Def *meth;
   Eo_Param_Def *param;
   Eo_Accessor_Def *accessor;
   Eo_Event_Def *sgn;
   Eo_Implement_Def *impl;

   printf("Class: %s (%s)\n",
          kls->name, (kls->comment ? kls->comment : "-"));
   printf("  inherits from :");
   EINA_LIST_FOREACH(kls->inherits, l, s)
      printf(" %s", s);
   printf("\n");
   printf("  implements:");
   EINA_LIST_FOREACH(kls->implements, l, impl)
      printf(" %s", impl->meth_name);
   printf("\n");
   printf("  events:\n");
   EINA_LIST_FOREACH(kls->events, l, sgn)
      printf("    %s <%s> (%s)\n", sgn->name, sgn->type, sgn->comment);
    EINA_LIST_FOREACH(kls->constructors, l, meth)
     {
        printf("  constructors: %s\n", meth->name);
        if (meth->ret)
          {
             printf("    return: ");
             _print_type(stdout, meth->ret->type);
             printf(" (%s)\n", meth->ret->comment);
          }
        printf("    legacy : %s\n", meth->legacy);
        EINA_LIST_FOREACH(meth->params, m, param)
          {
             printf("    param: %s %s : ", _param_way_str[param->way], param->name);
             _print_type(stdout, param->type);
             printf(" (%s)\n", param->comment);
          }
     }

   EINA_LIST_FOREACH(kls->properties, l, prop)
     {
        printf("  property: %s\n", prop->name);
        EINA_LIST_FOREACH(prop->keys, m, param)
          {
             printf("    key: %s : ", param->name);
             _print_type(stdout, param->type);
             printf(" (%s)\n", param->comment);
          }
        EINA_LIST_FOREACH(prop->values, m, param)
          {
             printf("    value: %s : ", param->name);
             _print_type(stdout, param->type);
             printf(" (%s)\n", param->comment);
          }
        EINA_LIST_FOREACH(prop->accessors, m, accessor)
          {
             printf("    accessor: ");
             if (accessor->ret)
                _print_type(stdout, accessor->ret->type);
             printf(" : %s (%s)\n", _accessor_type_str[accessor->type], accessor->comment);
             printf("      legacy : %s\n", accessor->legacy);
          }
     }

   EINA_LIST_FOREACH(kls->methods, l, meth)
     {
        printf("  method: %s\n", meth->name);
        if (meth->ret)
          {
             printf("    return: ");
             _print_type(stdout, meth->ret->type);
             printf(" (%s)\n", meth->ret->comment);
          }
        printf("    legacy : %s\n", meth->legacy);
        printf("    obj_const : %s\n", meth->obj_const?"true":"false");
        EINA_LIST_FOREACH(meth->params, m, param)
          {
             printf("    param: %s %s : ", _param_way_str[param->way], param->name);
             _print_type(stdout, param->type);
             printf(" (%s)\n", param->comment);
          }
     }
}

static void
_dump_type(Eo_Typedef_Def *type)
{
   printf("Typedef: %s ", type->alias);
   _print_type(stdout, type->type);
   printf("\n");
}

static void
_dump_struct(Eo_Type_Def *type)
{
   _print_type(stdout, type);
}

void
eo_parser_dump(Eo_Lexer *ls)
{
   Eina_List *k;
   Eo_Node *nd;

   EINA_LIST_FOREACH(ls->nodes, k, nd)
     {
        switch (nd->type)
          {
           case NODE_CLASS:
             _dump_class(nd->def_class);
             break;
           case NODE_TYPEDEF:
             _dump_type(nd->def_typedef);
             break;
           case NODE_STRUCT:
             _dump_struct(nd->def_struct);
           default:
             break;
          }
     }
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

static Eina_Bool
_db_fill_class(Eo_Class_Def *kls, const char *filename)
{
   const char *s;
   Eina_List *l, *m;

   Eo_Property_Def *prop;
   Eo_Method_Def *meth;
   Eo_Param_Def *param;
   Eo_Accessor_Def *accessor;
   Eo_Event_Def *event;
   Eo_Implement_Def *impl;

   Eolian_Class class = database_class_add(kls->name, kls->type);
   Eina_Bool is_iface = (kls->type == EOLIAN_CLASS_INTERFACE);
   database_class_file_set(class, filename);

   if (kls->comment) database_class_description_set(class, kls->comment);

   EINA_LIST_FOREACH(kls->inherits, l, s)
      database_class_inherit_add(class, s);

   if (kls->legacy_prefix)
     {
        database_class_legacy_prefix_set(class, kls->legacy_prefix);
     }
   if (kls->eo_prefix)
     {
        database_class_eo_prefix_set(class, kls->eo_prefix);
     }
   if (kls->data_type)
     {
        database_class_data_type_set(class, kls->data_type);
     }
   EINA_LIST_FOREACH(kls->constructors, l, meth)
     {
        Eolian_Function foo_id = database_function_new(meth->name, EOLIAN_CTOR);
        database_class_function_add(class, foo_id);
        if (meth->ret) database_function_return_comment_set(foo_id, EOLIAN_METHOD, meth->ret->comment);
        database_function_data_set(foo_id, EOLIAN_LEGACY, meth->legacy);
        EINA_LIST_FOREACH(meth->params, m, param)
          {
             database_method_parameter_add(foo_id, (Eolian_Parameter_Dir)param->way, (Eolian_Type)param->type, param->name, param->comment);
             param->type = NULL;
          }
     }

   EINA_LIST_FOREACH(kls->properties, l, prop)
     {
        Eolian_Function foo_id = database_function_new(prop->name, EOLIAN_UNRESOLVED);
        database_function_scope_set(foo_id, prop->scope);
        EINA_LIST_FOREACH(prop->keys, m, param)
          {
             Eolian_Function_Parameter p = database_property_key_add(
                   foo_id, (Eolian_Type)param->type, param->name, param->comment);
             database_parameter_nonull_set(p, param->nonull);
             param->type = NULL;
          }
        EINA_LIST_FOREACH(prop->values, m, param)
          {
             Eolian_Function_Parameter p = database_property_value_add(
                   foo_id, (Eolian_Type)param->type, param->name, param->comment);
             database_parameter_nonull_set(p, param->nonull);
             param->type = NULL;
          }
        EINA_LIST_FOREACH(prop->accessors, m, accessor)
          {
             database_function_type_set(foo_id, (accessor->type == SETTER?EOLIAN_PROP_SET:EOLIAN_PROP_GET));
             Eolian_Function_Type ftype =
                accessor->type == SETTER?EOLIAN_PROP_SET:EOLIAN_PROP_GET;
             if (accessor->ret && accessor->ret->type)
               {
                  database_function_return_type_set(foo_id, ftype, (Eolian_Type)accessor->ret->type);
                  database_function_return_comment_set(foo_id,
                        ftype, accessor->ret->comment);
                  database_function_return_flag_set_as_warn_unused(foo_id,
                        ftype, accessor->ret->warn_unused);
                  database_function_return_dflt_val_set(foo_id,
                        ftype, accessor->ret->dflt_ret_val);
                  accessor->ret->type = NULL;
               }
             if (accessor->legacy)
               {
                  database_function_data_set(foo_id,
                        (accessor->type == SETTER?EOLIAN_LEGACY_SET:EOLIAN_LEGACY_GET),
                        accessor->legacy);
               }
             database_function_description_set(foo_id,
                   (accessor->type == SETTER?EOLIAN_COMMENT_SET:EOLIAN_COMMENT_GET),
                   accessor->comment);
             Eo_Accessor_Param *acc_param;
             Eina_List *m2;
             EINA_LIST_FOREACH(accessor->params, m2, acc_param)
               {
                  Eolian_Function_Parameter desc = eolian_function_parameter_get(foo_id, acc_param->name);
                  if (!desc)
                    {
                       printf("Error - %s not known as parameter of property %s\n", acc_param->name, prop->name);
                    }
                  else
                     if (acc_param->is_const)
                       {
                          database_parameter_const_attribute_set(desc, accessor->type == GETTER, EINA_TRUE);
                       }
               }
             if (is_iface)
                database_function_set_as_virtual_pure(foo_id, ftype);
          }
        if (!prop->accessors)
          {
             database_function_type_set(foo_id, EOLIAN_PROPERTY);
             if (is_iface)
               database_function_set_as_virtual_pure(foo_id, EOLIAN_UNRESOLVED);
          }
        database_class_function_add(class, foo_id);
     }

   EINA_LIST_FOREACH(kls->methods, l, meth)
     {
        Eolian_Function foo_id = database_function_new(meth->name, EOLIAN_METHOD);
        database_function_scope_set(foo_id, meth->scope);
        database_class_function_add(class, foo_id);
        if (meth->ret)
          {
             database_function_return_type_set(foo_id, EOLIAN_METHOD, (Eolian_Type)meth->ret->type);
             database_function_return_comment_set(foo_id, EOLIAN_METHOD, meth->ret->comment);
             database_function_return_flag_set_as_warn_unused(foo_id,
                   EOLIAN_METHOD, meth->ret->warn_unused);
             database_function_return_dflt_val_set(foo_id,
                   EOLIAN_METHOD, meth->ret->dflt_ret_val);
             meth->ret->type = NULL;
          }
        database_function_description_set(foo_id, EOLIAN_COMMENT, meth->comment);
        database_function_data_set(foo_id, EOLIAN_LEGACY, meth->legacy);
        database_function_object_set_as_const(foo_id, meth->obj_const);
        EINA_LIST_FOREACH(meth->params, m, param)
          {
             Eolian_Function_Parameter p = database_method_parameter_add(foo_id,
                   (Eolian_Parameter_Dir)param->way, (Eolian_Type)param->type, param->name, param->comment);
             database_parameter_nonull_set(p, param->nonull);
             param->type = NULL;
          }
        if (is_iface)
          database_function_set_as_virtual_pure(foo_id, EOLIAN_METHOD);
     }

   EINA_LIST_FOREACH(kls->implements, l, impl)
     {
        const char *impl_name = impl->meth_name;
        if (!strcmp(impl_name, "class.constructor"))
          {
             database_class_ctor_enable_set(class, EINA_TRUE);
             continue;
          }
        if (!strcmp(impl_name, "class.destructor"))
          {
             database_class_dtor_enable_set(class, EINA_TRUE);
             continue;
          }
        if (!strncmp(impl_name, "virtual.", 8))
          {
             char *virtual_name = strdup(impl_name);
             char *func = strchr(virtual_name, '.');
             if (func) *func = '\0';
             func += 1;
             Eolian_Function_Type ftype = EOLIAN_UNRESOLVED;
             char *type_as_str = strchr(func, '.');
             if (type_as_str)
               {
                  *type_as_str = '\0';
                  if (!strcmp(type_as_str+1, "set")) ftype = EOLIAN_PROP_SET;
                  else if (!strcmp(type_as_str+1, "get")) ftype = EOLIAN_PROP_GET;
               }
             /* Search the function into the existing functions of the current class */
             Eolian_Function foo_id = eolian_class_function_find_by_name(
                   class, func, ftype);
             free(virtual_name);
             if (!foo_id)
               {
                  ERR("Error - %s not known in class %s", impl_name + 8, eolian_class_name_get(class));
                  return EINA_FALSE;
               }
             database_function_set_as_virtual_pure(foo_id, ftype);
             continue;
          }
        Eolian_Implement impl_desc = database_implement_new(impl_name);
        database_class_implement_add(class, impl_desc);
     }

   EINA_LIST_FOREACH(kls->events, l, event)
     {
        Eolian_Event ev = database_event_new(event->name, event->type, event->comment);
        database_class_event_add(class, ev);
     }

   return EINA_TRUE;
}

static Eina_Bool
_db_fill_type(Eo_Typedef_Def *type_def)
{
   Eina_Bool ret = database_type_add(type_def->alias, (Eolian_Type)type_def->type);
   type_def->type = NULL;
   return ret;
}

static Eina_Bool
_db_fill_struct(Eo_Type_Def *struct_def)
{
   return database_struct_add((Eolian_Type)struct_def);
}

Eina_Bool
eo_parser_database_fill(const char *filename, Eina_Bool eot)
{
   Eina_List *k;
   Eo_Node *nd;
   Eina_Bool has_class = EINA_FALSE;

   Eo_Lexer *ls = eo_lexer_new(filename);
   if (!ls)
     {
        ERR("unable to create lexer");
        return EINA_FALSE;
     }

   /* read first token */
   eo_lexer_get(ls);

   if (!eo_parser_walk(ls, eot))
     {
        eo_lexer_free(ls);
        return EINA_FALSE;
     }

   if (eot) goto nodeloop;

   EINA_LIST_FOREACH(ls->nodes, k, nd) if (nd->type == NODE_CLASS)
     {
        has_class = EINA_TRUE;
        break;
     }

   if (!has_class)
     {
        ERR("No classes for file %s", filename);
        eo_lexer_free(ls);
        return EINA_FALSE;
     }

nodeloop:
   EINA_LIST_FOREACH(ls->nodes, k, nd)
     {
        switch (nd->type)
          {
           case NODE_CLASS:
             if (!_db_fill_class(nd->def_class, filename))
               goto error;
             break;
           case NODE_TYPEDEF:
             if (!_db_fill_type(nd->def_typedef))
               goto error;
             break;
           case NODE_STRUCT:
             {
                Eo_Type_Def *def = nd->def_struct;
                nd->def_struct = NULL;
                if (!_db_fill_struct(def))
                  goto error;
                break;
             }
           default:
             break;
          }
     }

   eo_lexer_free(ls);
   return EINA_TRUE;

error:
   eo_lexer_free(ls);
   return EINA_FALSE;
}

#include "eo_parser.h"
#include "eolian_database.h"

#define FUNC_PUBLIC 0
#define FUNC_PROTECTED 1

#define eo_lexer_get_event_name(ls) eo_lexer_get_ident(ls, "@_.+-/\\='\"?!%,")

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
check_match(Eo_Lexer *ls, int what, int who, int where)
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
                      "'%s' expected (to close '%s' at line %d)",
                      tbuf, vbuf, where);
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

static Eina_Strbuf *
parse_name(Eo_Lexer *ls, Eina_Strbuf *buf)
{
   check(ls, TOK_VALUE);
   eina_strbuf_reset(buf);
   for (;;)
     {
        eina_strbuf_append(buf, ls->t.value);
        eo_lexer_get(ls);
        if (ls->t.token != TOK_DBCOLON) break;
        eo_lexer_get(ls);
        eina_strbuf_append(buf, "::");
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

static void
parse_type(Eo_Lexer *ls, Eina_Strbuf *buf)
{
   Eina_Bool has_struct = EINA_FALSE, need_space = EINA_FALSE;

   if (ls->t.kw == KW_at_own)
     {
        eina_strbuf_append(buf, "@own");
        eo_lexer_get(ls);
        need_space = EINA_TRUE;
     }

   if (ls->t.kw == KW_const)
     {
        if (need_space) eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append(buf, "const");
        eo_lexer_get(ls);
        need_space = EINA_TRUE;
     }

   if (ls->t.kw == KW_unsigned)
     {
        if (need_space) eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append(buf, "unsigned");
        eo_lexer_get(ls);
        need_space = EINA_TRUE;
     }
   else if (ls->t.kw == KW_signed)
     {
        if (need_space) eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append(buf, "signed");
        eo_lexer_get(ls);
        need_space = EINA_TRUE;
     }
   else if (ls->t.kw == KW_struct)
     {
        if (need_space) eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append(buf, "struct");
        eo_lexer_get(ls);
        need_space = EINA_TRUE;
     }

   if (!has_struct && ls->t.kw == KW_const)
     {
        if (need_space) eina_strbuf_append_char(buf, ' ');
        eina_strbuf_append(buf, "const");
        eo_lexer_get(ls);
        need_space = EINA_TRUE;
     }

   check(ls, TOK_VALUE);
   if (need_space) eina_strbuf_append_char(buf, ' ');
   eina_strbuf_append(buf, ls->t.value);
   eo_lexer_get(ls);
   need_space = EINA_TRUE;

   if (ls->t.kw == KW_const)
     {
        eina_strbuf_append(buf, " const");
        eo_lexer_get(ls);
     }

   if (ls->t.token == '*')
     {
        eina_strbuf_append_char(buf, ' ');
        while (ls->t.token == '*')
          {
             eina_strbuf_append_char(buf, '*');
             eo_lexer_get(ls);
             if (ls->t.kw == KW_const)
               {
                  eina_strbuf_append(buf, " const");
                  eo_lexer_get(ls);
               }
          }
     }

   if (ls->t.token == '<')
     {
        int line = ls->line_number;
        eina_strbuf_append(buf, " <");
        eo_lexer_get(ls);
        parse_type(ls, buf);
        check_match(ls, '>', '<', line);
        eina_strbuf_append_char(buf, '>');
     }
}

static void
parse_typedef(Eo_Lexer *ls)
{
   Eina_Strbuf *buf = push_strbuf(ls);
   ls->tmp.type_def = calloc(1, sizeof(Eo_Type_Def));
   eo_lexer_get(ls);
   check(ls, TOK_VALUE);
   ls->tmp.type_def->alias = eina_stringshare_add(ls->t.value);
   eo_lexer_get(ls);
   test_next(ls, ':');
   parse_type(ls, buf);
   ls->tmp.type_def->type = eina_stringshare_add(eina_strbuf_string_get(buf));
   pop_strbuf(ls);
   check_next(ls, ';');
}

static void
parse_return(Eo_Lexer *ls)
{
   Eo_Ret_Def *ret = calloc(1, sizeof(Eo_Ret_Def));
   Eina_Strbuf *buf = push_strbuf(ls);
   ls->tmp.ret_def = ret;
   eo_lexer_get(ls);
   parse_type(ls, buf);
   ret->type = eina_stringshare_add(eina_strbuf_string_get(buf));
   pop_strbuf(ls);
   if (ls->t.token == '(')
     {
        int line = ls->line_number;
        eo_lexer_get(ls);
        check(ls, TOK_VALUE);
        ret->dflt_ret_val = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
        check_match(ls, ')', '(', line);
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
   Eina_Strbuf *buf = NULL;
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
   buf = push_strbuf(ls);
   parse_type(ls, buf);
   par->type = eina_stringshare_add(eina_strbuf_string_get(buf));
   pop_strbuf(ls);
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
   Eina_Strbuf *buf = NULL;
   Eo_Accessor_Param *acc = NULL;
   Eina_Bool has_const  = EINA_FALSE, has_own = EINA_FALSE,
             has_nonull = EINA_FALSE, first   = EINA_TRUE;
   acc = calloc(1, sizeof(Eo_Accessor_Param));
   ls->tmp.accessor_param = acc;
   acc->name = eina_stringshare_add(ls->t.value);
   eo_lexer_get(ls);
   check_next(ls, ':');
   check(ls, TOK_VALUE);
   buf = push_strbuf(ls);
   for (;;)
     {
        switch (ls->t.kw)
          {
             case KW_const:
                if (has_const)
                   eo_lexer_syntax_error(ls, "double const qualifier");
                has_const = EINA_TRUE;
                if (!first) eina_strbuf_append_char(buf, ' ');
                eina_strbuf_append(buf, "const");
                first = EINA_FALSE;
                eo_lexer_get(ls);
                break;
             case KW_at_own:
                if (has_own)
                   eo_lexer_syntax_error(ls, "double @own qualifier");
                has_own = EINA_TRUE;
                if (!first) eina_strbuf_append_char(buf, ' ');
                eina_strbuf_append(buf, "@own");
                first = EINA_FALSE;
                eo_lexer_get(ls);
                break;
             case KW_at_nonull:
                if (has_nonull)
                   eo_lexer_syntax_error(ls, "double @nonull qualifier");
                has_nonull = EINA_TRUE;
                if (!first) eina_strbuf_append_char(buf, ' ');
                eina_strbuf_append(buf, "@nonull");
                first = EINA_FALSE;
                eo_lexer_get(ls);
                break;
             default:
                if (first) eo_lexer_syntax_error(ls, "attribute expected");
                goto end;
          }
     }
end:
   acc->attrs = eina_stringshare_add(eina_strbuf_string_get(buf));
   pop_strbuf(ls);
   check_next(ls, ';');
}

static void
parse_accessor(Eo_Lexer *ls)
{
   int line;
   Eo_Accessor_Def *acc = NULL;
   Eina_Bool has_return = EINA_FALSE, has_legacy = EINA_FALSE;
   acc = calloc(1, sizeof(Eo_Accessor_Def));
   ls->tmp.accessor = acc;
   acc->type = (ls->t.kw == KW_get) ? GETTER : SETTER;
   eo_lexer_get(ls);
   line = ls->line_number;
   check_next(ls, '{');
   if (ls->t.token == TOK_COMMENT)
     {
        acc->comment = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
     }
   for (;;)
     {
        switch (ls->t.kw)
          {
             case KW_return:
                if (has_return)
                   eo_lexer_syntax_error(ls, "double return");
                has_return = EINA_TRUE;
                parse_return(ls);
                acc->ret = ls->tmp.ret_def;
                ls->tmp.ret_def = NULL;
                break;
             case KW_legacy:
                if (has_legacy)
                   eo_lexer_syntax_error(ls, "double legacy name");
                has_legacy = EINA_TRUE;
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
     }
end:
   check_match(ls, '}', '{', line);
}

static void
parse_keys(Eo_Lexer *ls)
{
   int line;
   eo_lexer_get(ls);
   line = ls->line_number;
   check_next(ls, '{');
   while (ls->t.token != '}')
     {
        parse_param(ls, EINA_FALSE);
        ls->tmp.params = eina_list_append(ls->tmp.params, ls->tmp.param);
        ls->tmp.param = NULL;
     }
   check_match(ls, '}', '{', line);
}

static void
parse_values(Eo_Lexer *ls)
{
   int line;
   eo_lexer_get(ls);
   line = ls->line_number;
   check_next(ls, '{');
   while (ls->t.token != '}')
     {
        parse_param(ls, EINA_FALSE);
        ls->tmp.params = eina_list_append(ls->tmp.params, ls->tmp.param);
        ls->tmp.param = NULL;
     }
   check_match(ls, '}', '{', line);
}

static void
parse_property(Eo_Lexer *ls)
{
   int line;
   Eo_Property_Def *prop = NULL;
   Eina_Bool has_get  = EINA_FALSE, has_set    = EINA_FALSE,
             has_keys = EINA_FALSE, has_values = EINA_FALSE;
   prop = calloc(1, sizeof(Eo_Property_Def));
   ls->tmp.prop = prop;
   if (ls->t.kw == KW_protected)
     {
        prop->scope = FUNC_PROTECTED;
        eo_lexer_get(ls);
     }
   prop->name = eina_stringshare_add(ls->t.value);
   eo_lexer_get(ls);
   line = ls->line_number;
   check_next(ls, '{');
   for (;;)
     {
        switch (ls->t.kw)
          {
             case KW_get:
                if (has_get)
                   eo_lexer_syntax_error(ls, "double get definition");
                has_get = EINA_TRUE;
                parse_accessor(ls);
                prop->accessors = eina_list_append(prop->accessors,
                                                   ls->tmp.accessor);
                ls->tmp.accessor = NULL;
                break;
             case KW_set:
                if (has_set)
                   eo_lexer_syntax_error(ls, "double set definition");
                has_set = EINA_TRUE;
                parse_accessor(ls);
                prop->accessors = eina_list_append(prop->accessors,
                                                   ls->tmp.accessor);
                ls->tmp.accessor = NULL;
                break;
             case KW_keys:
                if (has_keys)
                   eo_lexer_syntax_error(ls, "double keys definition");
                has_keys = EINA_TRUE;
                parse_keys(ls);
                prop->keys = ls->tmp.params;
                ls->tmp.params = NULL;
                break;
             case KW_values:
                if (has_values)
                   eo_lexer_syntax_error(ls, "double values definition");
                has_values = EINA_TRUE;
                parse_values(ls);
                prop->values = ls->tmp.params;
                ls->tmp.params = NULL;
                break;
             default:
                goto end;
          }
     }
end:
   check_match(ls, '}', '{', line);
}

static void
parse_params(Eo_Lexer *ls)
{
   int line;
   eo_lexer_get(ls);
   line = ls->line_number;
   check_next(ls, '{');
   while (ls->t.token != '}')
     {
        parse_param(ls, EINA_TRUE);
        ls->tmp.params = eina_list_append(ls->tmp.params, ls->tmp.param);
        ls->tmp.param = NULL;
     }
   check_match(ls, '}', '{', line);
}

static void
parse_method(Eo_Lexer *ls, Eina_Bool ctor)
{
   int line;
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
             meth->scope = FUNC_PROTECTED;
             eo_lexer_get(ls);
          }
        check(ls, TOK_VALUE);
        meth->name = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
     }
   line = ls->line_number;
   check_next(ls, '{');
   if (ls->t.token == TOK_COMMENT)
     {
        meth->comment = eina_stringshare_add(ls->t.value);
        eo_lexer_get(ls);
     }
   for (;;)
     {
        switch (ls->t.kw)
          {
             case KW_const:
                if (has_const)
                   eo_lexer_syntax_error(ls, "double const qualifier");
                has_const = EINA_TRUE;
                meth->obj_const = EINA_TRUE;
                eo_lexer_get(ls);
                check_next(ls, ';');
                break;
             case KW_return:
                if (has_return)
                   eo_lexer_syntax_error(ls, "double return");
                has_return = EINA_TRUE;
                parse_return(ls);
                meth->ret = ls->tmp.ret_def;
                ls->tmp.ret_def = NULL;
                break;
             case KW_legacy:
                if (has_legacy)
                   eo_lexer_syntax_error(ls, "double legacy name");
                has_legacy = EINA_TRUE;
                parse_legacy(ls);
                meth->legacy = ls->tmp.legacy_def;
                ls->tmp.legacy_def = NULL;
                break;
             case KW_params:
                if (has_params)
                   eo_lexer_syntax_error(ls, "double params definition");
                has_params = EINA_TRUE;
                parse_params(ls);
                meth->params = ls->tmp.params;
                ls->tmp.params = NULL;
                break;
             default:
                goto end;
          }
     }
end:
   check_match(ls, '}', '{', line);
}

static void
parse_implement(Eo_Lexer *ls)
{
   Eina_Strbuf *buf = NULL;
   Eo_Implement_Def *impl = NULL;
   buf = push_strbuf(ls);
   impl = calloc(1, sizeof(Eo_Implement_Def));
   ls->tmp.impl = impl;
   if (ls->t.kw == KW_class)
     {
        eina_strbuf_append(buf, "class::");
        eo_lexer_get(ls);
        check_next(ls, TOK_DBCOLON);
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
        eina_strbuf_append(buf, "virtual::");
        eo_lexer_get(ls);
        check_next(ls, TOK_DBCOLON);
        if ((ls->t.token != TOK_VALUE) || (ls->t.kw == KW_get || ls->t.kw == KW_set))
           eo_lexer_syntax_error(ls, "name expected");
        eina_strbuf_append(buf, ls->t.value);
        eo_lexer_get(ls);
        if (ls->t.token == TOK_DBCOLON)
          {
             eo_lexer_get(ls);
             if (ls->t.kw == KW_set)
               {
                  eina_strbuf_append(buf, "::set");
                  eo_lexer_get(ls);
               }
             else
               {
                  check_kw_next(ls, KW_get);
                  eina_strbuf_append(buf, "::get");
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
   check_next(ls, TOK_DBCOLON);
   eina_strbuf_append(buf, "::");
   if ((ls->t.token != TOK_VALUE) || (ls->t.kw == KW_get || ls->t.kw == KW_set))
      eo_lexer_syntax_error(ls, "name or constructor/destructor expected");
   for (;;)
     {
        switch (ls->t.kw)
          {
             case KW_constructor:
                eina_strbuf_append(buf, "constructor");
                eo_lexer_get(ls);
                goto end;
             case KW_destructor:
                eina_strbuf_append(buf, "destructor");
                eo_lexer_get(ls);
                goto end;
             case KW_get:
                eina_strbuf_append(buf, "get");
                eo_lexer_get(ls);
                goto end;
             case KW_set:
                eina_strbuf_append(buf, "set");
                eo_lexer_get(ls);
                goto end;
             default:
                break;
          }
        check(ls, TOK_VALUE);
        eina_strbuf_append(buf, ls->t.value);
        eo_lexer_get(ls);
        if (ls->t.token != TOK_DBCOLON) break;
        eina_strbuf_append(buf, "::");
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
        eo_lexer_get_event_name(ls);
     }*/
   check(ls, TOK_VALUE);
   ev->name = eina_stringshare_add(ls->t.value);
   eo_lexer_get(ls);
   if (ls->t.token == '(')
     {
        Eina_Strbuf *buf;
        int line = ls->line_number;
        eo_lexer_get(ls);
        buf = push_strbuf(ls);
        parse_type(ls, buf);
        ev->type = eina_stringshare_add(eina_strbuf_string_get(buf));
        pop_strbuf(ls);
        check_match(ls, ')', '(', line);
     }
   check(ls, ';');
   eo_lexer_get_event_name(ls);
   if (ls->t.token == TOK_COMMENT)
     {
        ev->comment = eina_stringshare_add(ls->t.value);
        eo_lexer_get_event_name(ls);
     }
}

static void
parse_constructors(Eo_Lexer *ls)
{
   int line;
   eo_lexer_get(ls);
   line = ls->line_number;
   check_next(ls, '{');
   while (ls->t.token != '}')
     {
        parse_method(ls, EINA_TRUE);
        ls->tmp.kls->constructors = eina_list_append(ls->tmp.kls->constructors,
                                                     ls->tmp.meth);
        ls->tmp.meth = NULL;
     }
   check_match(ls, '}', '{', line);
}

static void
parse_methods(Eo_Lexer *ls)
{
   int line;
   eo_lexer_get(ls);
   line = ls->line_number;
   check_next(ls, '{');
   while (ls->t.token != '}')
     {
        parse_method(ls, EINA_FALSE);
        ls->tmp.kls->methods = eina_list_append(ls->tmp.kls->methods,
                                                ls->tmp.meth);
        ls->tmp.meth = NULL;
     }
   check_match(ls, '}', '{', line);
}

static void
parse_properties(Eo_Lexer *ls)
{
   int line;
   eo_lexer_get(ls);
   line = ls->line_number;
   check_next(ls, '{');
   while (ls->t.token != '}')
     {
        parse_property(ls);
        ls->tmp.kls->properties = eina_list_append(ls->tmp.kls->properties,
                                                   ls->tmp.prop);
        ls->tmp.prop = NULL;
     }
   check_match(ls, '}', '{', line);
}

static void
parse_implements(Eo_Lexer *ls)
{
   int line;
   eo_lexer_get(ls);
   line = ls->line_number;
   check_next(ls, '{');
   while (ls->t.token != '}')
     {
        parse_implement(ls);
        ls->tmp.kls->implements = eina_list_append(ls->tmp.kls->implements,
                                                   ls->tmp.impl);
        ls->tmp.impl = NULL;
     }
   check_match(ls, '}', '{', line);
}

static void
parse_events(Eo_Lexer *ls)
{
   int line;
   eo_lexer_get(ls);
   line = ls->line_number;
   check(ls, '{');
   eo_lexer_get_event_name(ls);
   while (ls->t.token != '}')
     {
        parse_event(ls);
        ls->tmp.kls->events = eina_list_append(ls->tmp.kls->events,
                                               ls->tmp.event);
        ls->tmp.event = NULL;
     }
   check_match(ls, '}', '{', line);
}

static void
parse_class_body(Eo_Lexer *ls, Eina_Bool allow_ctors)
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
   for (;;)
     {
        switch (ls->t.kw)
          {
             case KW_legacy_prefix:
                if (has_legacy_prefix)
                   eo_lexer_syntax_error(ls, "double legacy prefix definition");
                has_legacy_prefix = EINA_TRUE;
                eo_lexer_get(ls);
                check_next(ls, ':');
                check(ls, TOK_VALUE);
                ls->tmp.kls->legacy_prefix = eina_stringshare_add(ls->t.value);
                eo_lexer_get(ls);
                check_next(ls, ';');
                break;
             case KW_eo_prefix:
                if (has_eo_prefix)
                   eo_lexer_syntax_error(ls, "double eo prefix definition");
                has_eo_prefix = EINA_TRUE;
                eo_lexer_get(ls);
                check_next(ls, ':');
                check(ls, TOK_VALUE);
                ls->tmp.kls->eo_prefix = eina_stringshare_add(ls->t.value);
                eo_lexer_get(ls);
                check_next(ls, ';');
                break;
             case KW_data:
                if (has_data)
                   eo_lexer_syntax_error(ls, "double data definition");
                has_data = EINA_TRUE;
                eo_lexer_get(ls);
                check_next(ls, ':');
                check(ls, TOK_VALUE);
                ls->tmp.kls->data_type = eina_stringshare_add(ls->t.value);
                eo_lexer_get(ls);
                check_next(ls, ';');
                break;
             case KW_constructors:
                if (!allow_ctors)
                   return;
                if (has_constructors)
                   eo_lexer_syntax_error(ls, "double constructors definition");
                has_constructors = EINA_TRUE;
                parse_constructors(ls);
                break;
             case KW_properties:
                if (has_properties)
                   eo_lexer_syntax_error(ls, "double properties definition");
                has_properties = EINA_TRUE;
                parse_properties(ls);
                break;
             case KW_methods:
                if (has_methods)
                   eo_lexer_syntax_error(ls, "double methods definition");
                has_methods = EINA_TRUE;
                parse_methods(ls);
                break;
             case KW_implements:
                if (has_implements)
                   eo_lexer_syntax_error(ls, "double implements definition");
                has_implements = EINA_TRUE;
                parse_implements(ls);
                break;
             case KW_events:
                if (has_events)
                   eo_lexer_syntax_error(ls, "double events definition");
                has_events = EINA_TRUE;
                parse_events(ls);
                break;
             default:
                return;
          }
     }
}

static void
parse_class(Eo_Lexer *ls, Eina_Bool allow_ctors, Eolian_Class_Type type)
{
   int line;
   Eina_Strbuf *buf = push_strbuf(ls);
   ls->tmp.kls = calloc(1, sizeof(Eo_Class_Def));
   eo_lexer_get(ls);
   ls->tmp.kls->type = type;
   parse_name(ls, buf);
   ls->tmp.kls->name = eina_stringshare_add(eina_strbuf_string_get(buf));
   pop_strbuf(ls);
   line = ls->line_number;
   check_next(ls, '(');
   if (ls->t.token != ')')
     {
        ls->tmp.kls->inherits = parse_name_list(ls);
        ls->tmp.str_items = NULL;
     }
   check_match(ls, ')', '(', line);
   line = ls->line_number;
   check_next(ls, '{');
   parse_class_body(ls, allow_ctors);
   check_match(ls, '}', '{', line);
}

static void
parse_unit(Eo_Lexer *ls)
{
   switch (ls->t.kw)
     {
        case KW_abstract:
          {
             parse_class(ls, EINA_TRUE, EOLIAN_CLASS_ABSTRACT);
             ls->classes = eina_list_append(ls->classes, ls->tmp.kls);
             ls->tmp.kls = NULL;
             break;
          }
        case KW_class:
          {
             parse_class(ls, EINA_TRUE, EOLIAN_CLASS_REGULAR);
             ls->classes = eina_list_append(ls->classes, ls->tmp.kls);
             ls->tmp.kls = NULL;
             break;
          }
        case KW_mixin:
          {
             parse_class(ls, EINA_FALSE, EOLIAN_CLASS_MIXIN);
             ls->classes = eina_list_append(ls->classes, ls->tmp.kls);
             ls->tmp.kls = NULL;
             break;
          }
        case KW_interface:
          {
             parse_class(ls, EINA_FALSE, EOLIAN_CLASS_INTERFACE);
             ls->classes = eina_list_append(ls->classes, ls->tmp.kls);
             ls->tmp.kls = NULL;
             break;
          }
        case KW_type:
          {
             parse_typedef(ls);
             ls->typedefs = eina_list_append(ls->typedefs, ls->tmp.type_def);
             ls->tmp.type_def = NULL;
             break;
          }
        default:
          {
             eo_lexer_syntax_error(ls, "invalid token");
             break;
          }
     }
}

static void
parse_chunk(Eo_Lexer *ls)
{
   while (ls->t.token != TOK_EOF)
     {
        parse_unit(ls);
     }
}

static char *_accessor_type_str[ACCESSOR_TYPE_LAST] = { "setter", "getter"   };
static char *    _param_way_str[    PARAM_WAY_LAST] = { "IN", "OUT", "INOUT" };

void
eo_parser_dump(Eo_Lexer *ls)
{
   const char *s;
   Eina_List *k, *l, *m;

   Eo_Type_Def *type;
   Eo_Class_Def *kls;
   Eo_Property_Def *prop;
   Eo_Method_Def *meth;
   Eo_Param_Def *param;
   Eo_Accessor_Def *accessor;
   Eo_Event_Def *sgn;
   Eo_Implement_Def *impl;

   EINA_LIST_FOREACH(ls->classes, k, kls)
     {
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
                printf("    return: %s (%s)\n", meth->ret->type, meth->ret->comment);
             printf("    legacy : %s\n", meth->legacy);
             EINA_LIST_FOREACH(meth->params, m, param)
               {
                  printf("    param: %s %s : %s (%s)\n",
                         _param_way_str[param->way], param->name,
                         param->type, param->comment);
               }
          }

        EINA_LIST_FOREACH(kls->properties, l, prop)
          {
             printf("  property: %s\n", prop->name);
             EINA_LIST_FOREACH(prop->keys, m, param)
               {
                  printf("    key: %s : %s (%s)\n",
                         param->name, param->type, param->comment);
               }
             EINA_LIST_FOREACH(prop->values, m, param)
               {
                  printf("    value: %s : %s (%s)\n",
                         param->name, param->type, param->comment);
               }
             EINA_LIST_FOREACH(prop->accessors, m, accessor)
               {
                  printf("    accessor: %s : %s (%s)\n",
                         (accessor->ret?accessor->ret->type:""),
                         _accessor_type_str[accessor->type],
                         accessor->comment);
                  printf("      legacy : %s\n", accessor->legacy);
               }
          }

        EINA_LIST_FOREACH(kls->methods, l, meth)
          {
             printf("  method: %s\n", meth->name);
             if (meth->ret)
                printf("    return: %s (%s)\n", meth->ret->type, meth->ret->comment);
             printf("    legacy : %s\n", meth->legacy);
             printf("    obj_const : %s\n", meth->obj_const?"true":"false");
             EINA_LIST_FOREACH(meth->params, m, param)
               {
                  printf("    param: %s %s : %s (%s)\n",
                         _param_way_str[param->way], param->name,
                         param->type, param->comment);
               }
          }
     }

   EINA_LIST_FOREACH(ls->typedefs, k, type)
     {
        printf("Typedef: %s (%s)\n", type->alias, type->type);
     }
}

static Eina_Inlist *
_types_extract(const char *buf, int len)
{
   const char *save_buf = buf;
   Eolian_Type types = NULL;
   long depth = 0;
   char *tmp_type = malloc(2 * len + 1);

   while (len > 0)
     {
        char *d = tmp_type;
        Eina_Bool end_type = EINA_FALSE;
        Eina_Bool is_own = EINA_FALSE;
        char c;
        Eina_Bool in_spaces = EINA_TRUE, in_stars = EINA_FALSE;
        while (len > 0 && !end_type)
          {
             switch (c = *buf++)
               {
                /* @own */
                case '@':
                     {
                        if (!strncmp(buf, "own ", 4))
                          {
                             is_own = EINA_TRUE;
                             buf += 4; len -= 4;
                          }
                        break;
                     }
                /* if '*', we have to add a space. We set in_spaces to true in
                 * case spaces are between stars, to be sure we remove them.
                 */
                case '*':
                     {
                        if (!in_stars && !in_spaces)
                          {
                             *d++ = ' ';
                             in_stars = EINA_TRUE;
                             in_spaces = EINA_TRUE;
                          }
                        *d++ = c;
                        break;
                     }
                /* Only the first space is inserted. */
                case ' ':
                     {
                        if (!in_spaces) *d++ = c;
                        in_spaces = EINA_TRUE;
                        break;
                     }
                case '<':
                     {
                        if (depth < 0)
                          {
                             ERR("%s: Cannot reopen < after >", save_buf);
                             goto error;
                          }
                        depth++;
                        end_type = EINA_TRUE;
                        break;
                     }
                case '>':
                     {
                        if (depth == 0)
                          {
                             ERR("%s: Too much >", save_buf);
                             goto error;
                          }
                        if (depth > 0 && d == tmp_type)
                          {
                             ERR("%s: empty type inside <>", save_buf);
                             goto error;
                          }
                        if (depth > 0) depth *= -1;
                        depth++;
                        end_type = EINA_TRUE;
                        break;
                     }
                default:
                     {
                        *d++ = c;
                        in_spaces = EINA_FALSE;
                        in_stars = EINA_FALSE;
                     }
               }
             len--;
          }
        if (d != tmp_type)
          {
             *d = '\0';
             types = database_type_append(types, tmp_type, is_own);
          }
     }
   if (depth)
     {
        ERR("%s: < and > are not well used.", save_buf);
        goto error;
     }
   goto success;
error:
   database_type_del(types);
   types = NULL;
success:
   free(tmp_type);
   return types;
}

Eina_Bool
eo_parser_walk(Eo_Lexer *ls)
{
   if (!eo_lexer_setjmp(ls))
     {
        parse_chunk(ls);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

Eina_Bool
eo_parser_database_fill(const char *filename)
{
   Eina_Bool ret = EINA_FALSE;
   const char *s;
   Eina_List *k, *l, *m;

   Eo_Class_Def *kls;
   Eo_Type_Def *type_def;
   Eo_Property_Def *prop;
   Eo_Method_Def *meth;
   Eo_Param_Def *param;
   Eo_Accessor_Def *accessor;
   Eo_Event_Def *event;
   Eo_Implement_Def *impl;

   Eo_Lexer *ls = eo_lexer_new(filename);
   if (!ls)
     {
        ERR("unable to create lexer");
        goto end;
     }

   /* read first token */
   eo_lexer_get(ls);

   if (!eo_parser_walk(ls)) goto end;

   if (!ls->classes)
     {
        ERR("No classes for file %s", filename);
        goto end;
     }

   EINA_LIST_FOREACH(ls->classes, k, kls)
     {
        Eolian_Class class = database_class_add(kls->name, kls->type);
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
                  Eolian_Type type = _types_extract(param->type, strlen(param->type));
                  database_method_parameter_add(foo_id, (Eolian_Parameter_Dir)param->way, type, param->name, param->comment);
               }
          }

        EINA_LIST_FOREACH(kls->properties, l, prop)
          {
             Eolian_Function foo_id = database_function_new(prop->name, EOLIAN_UNRESOLVED);
             database_function_scope_set(foo_id, prop->scope);
             EINA_LIST_FOREACH(prop->keys, m, param)
               {
                  Eolian_Type type = _types_extract(param->type, strlen(param->type));
                  Eolian_Function_Parameter p = database_property_key_add(
                        foo_id, type, param->name, param->comment);
                  database_parameter_nonull_set(p, param->nonull);
               }
             EINA_LIST_FOREACH(prop->values, m, param)
               {
                  Eolian_Type type = _types_extract(param->type, strlen(param->type));
                  Eolian_Function_Parameter p = database_property_value_add(
                        foo_id, type, param->name, param->comment);
                  database_parameter_nonull_set(p, param->nonull);
               }
             EINA_LIST_FOREACH(prop->accessors, m, accessor)
               {
                  database_function_type_set(foo_id, (accessor->type == SETTER?EOLIAN_PROP_SET:EOLIAN_PROP_GET));
                  if (accessor->ret && accessor->ret->type)
                    {
                       Eolian_Function_Type ftype =
                          accessor->type == SETTER?EOLIAN_PROP_SET:EOLIAN_PROP_GET;
                       Eolian_Type types = _types_extract(accessor->ret->type, strlen(accessor->ret->type));
                       database_function_return_type_set(foo_id, ftype, types);
                       database_function_return_comment_set(foo_id,
                             ftype, accessor->ret->comment);
                       database_function_return_flag_set_as_warn_unused(foo_id,
                             ftype, accessor->ret->warn_unused);
                       database_function_return_dflt_val_set(foo_id,
                             ftype, accessor->ret->dflt_ret_val);
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
                          if (strstr(acc_param->attrs, "const"))
                            {
                               database_parameter_const_attribute_set(desc, accessor->type == GETTER, EINA_TRUE);
                            }
                    }
               }
             if (!prop->accessors) database_function_type_set(foo_id, EOLIAN_PROPERTY);
             database_class_function_add(class, foo_id);
          }

        EINA_LIST_FOREACH(kls->methods, l, meth)
          {
             Eolian_Function foo_id = database_function_new(meth->name, EOLIAN_METHOD);
             database_function_scope_set(foo_id, meth->scope);
             database_class_function_add(class, foo_id);
             if (meth->ret)
               {
                  Eolian_Type types = _types_extract(meth->ret->type, strlen(meth->ret->type));
                  database_function_return_type_set(foo_id, EOLIAN_METHOD, types);
                  database_function_return_comment_set(foo_id, EOLIAN_METHOD, meth->ret->comment);
                  database_function_return_flag_set_as_warn_unused(foo_id,
                        EOLIAN_METHOD, meth->ret->warn_unused);
                  database_function_return_dflt_val_set(foo_id,
                        EOLIAN_METHOD, meth->ret->dflt_ret_val);
               }
             database_function_description_set(foo_id, EOLIAN_COMMENT, meth->comment);
             database_function_data_set(foo_id, EOLIAN_LEGACY, meth->legacy);
             database_function_object_set_as_const(foo_id, meth->obj_const);
             EINA_LIST_FOREACH(meth->params, m, param)
               {
                  Eolian_Type type = _types_extract(param->type, strlen(param->type));
                  Eolian_Function_Parameter p = database_method_parameter_add(foo_id,
                        (Eolian_Parameter_Dir)param->way, type, param->name, param->comment);
                  database_parameter_nonull_set(p, param->nonull);
               }
          }

        EINA_LIST_FOREACH(kls->implements, l, impl)
          {
             const char *impl_name = impl->meth_name;
             if (!strcmp(impl_name, "class::constructor"))
               {
                  database_class_ctor_enable_set(class, EINA_TRUE);
                  continue;
               }
             if (!strcmp(impl_name, "class::destructor"))
               {
                  database_class_dtor_enable_set(class, EINA_TRUE);
                  continue;
               }
             if (!strncmp(impl_name, "virtual::", 9))
               {
                  char *virtual_name = strdup(impl_name);
                  char *func = strstr(virtual_name, "::");
                  if (func) *func = '\0';
                  func += 2;
                  Eolian_Function_Type ftype = EOLIAN_UNRESOLVED;
                  char *type_as_str = strstr(func, "::");
                  if (type_as_str)
                    {
                       *type_as_str = '\0';
                       if (!strcmp(type_as_str+2, "set")) ftype = EOLIAN_PROP_SET;
                       else if (!strcmp(type_as_str+2, "get")) ftype = EOLIAN_PROP_GET;
                    }
                  /* Search the function into the existing functions of the current class */
                  Eolian_Function foo_id = eolian_class_function_find_by_name(
                        class, func, ftype);
                  free(virtual_name);
                  if (!foo_id)
                    {
                       ERR("Error - %s not known in class %s", impl_name + 9, eolian_class_name_get(class));
                       goto end;
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

     }

   EINA_LIST_FOREACH(ls->typedefs, k, type_def)
     {
        database_type_add(type_def->alias, _types_extract(type_def->type, strlen(type_def->type)));
     }

   ret = EINA_TRUE;
end:
   if (ls) eo_lexer_free(ls);
   return ret;
}

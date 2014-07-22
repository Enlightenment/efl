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

static void
append_node(Eo_Lexer *ls, int type, void *def)
{
   Eo_Node *nd = calloc(1, sizeof(Eo_Node));
   nd->type  = type;
   nd->def   = def;
   ls->nodes = eina_list_append(ls->nodes, nd);
}

static const char *
get_filename(Eo_Lexer *ls)
{
   char *s = strdup(ls->source);
   Eina_Array *arr = eina_file_split(s);
   const char *file = eina_stringshare_add(eina_array_data_get(arr,
                                           eina_array_count_get(arr) - 1));
   free(s);
   eina_array_free(arr);
   return file;
}

static void
redef_error(Eo_Lexer *ls, Eolian_Type_Type type, Eolian_Type *old)
{
   char  buf[256];
   char fbuf[256] = { '\0' };
   const char *file = get_filename(ls);
   if (file != old->file)
     snprintf(fbuf, sizeof(fbuf), " in file '%s'", old->file);
   eina_stringshare_del(file);
   snprintf(buf, sizeof(buf),
            "%s '%s' redefined (originally at line %d, column %d%s)",
            (type == EOLIAN_TYPE_STRUCT) ? "struct" : "type alias",
            old->full_name, old->line, old->column, fbuf);
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
        eina_strbuf_append(buf, ls->t.value);
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

static void
_fill_type_name(Eolian_Type *tp, const char *type_name)
{
   char *full_name = strdup(type_name);
   char *name = full_name, *dot = full_name;
   tp->full_name = type_name;
   do
     {
        dot = strchr(dot, '.');
        if (dot)
          {
             *dot = '\0';
             tp->namespaces = eina_list_append(tp->namespaces,
                                               eina_stringshare_add(name));
             ++dot;
             name = dot;
          }
     }
   while (dot);
   tp->name = eina_stringshare_add(name);
   free(full_name);
}

static Eolian_Type *parse_type_void(Eo_Lexer *ls);
static Eolian_Type *parse_type_struct(Eo_Lexer *ls, Eina_Bool allow_struct,
                                      Eina_Bool allow_anon);

static Eolian_Type *
parse_type(Eo_Lexer *ls)
{
   int line = ls->line_number, column = ls->column;
   Eolian_Type *ret = parse_type_void(ls);
   if (ret->type == EOLIAN_TYPE_VOID)
     {
        ls->line_number = line;
        ls->column = column;
        eo_lexer_syntax_error(ls, "non-void type expected");
     }
   return ret;
}

static Eolian_Type *
parse_type_struct_nonvoid(Eo_Lexer *ls, Eina_Bool allow_struct,
                          Eina_Bool allow_anon)
{
   int line = ls->line_number, column = ls->column;
   Eolian_Type *ret = parse_type_struct(ls, allow_struct, allow_anon);
   if (ret->type == EOLIAN_TYPE_VOID)
     {
        ls->line_number = line;
        ls->column = column;
        eo_lexer_syntax_error(ls, "non-void type expected");
     }
   return ret;
}

static Eolian_Type *
parse_function_type(Eo_Lexer *ls)
{
   int line, col;
   Eolian_Type *def = push_type(ls);
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
   database_type_del(def->type);
   if (def->comment) eina_stringshare_del(def->comment);
   free(def);
}

static Eolian_Type *
parse_struct(Eo_Lexer *ls, const char *name, Eina_Bool is_extern,
             int line, int column)
{
   int bline = ls->line_number, bcolumn = ls->column;
   Eolian_Type *def = push_type(ls);
   def->is_extern = is_extern;
   def->file = get_filename(ls);
   if (name) _fill_type_name(def, name);
   def->type = EOLIAN_TYPE_STRUCT;
   def->fields = eina_hash_string_small_new(EINA_FREE_CB(_struct_field_free));
   check_next(ls, '{');
   if (ls->t.token == TOK_COMMENT)
     {
        def->comment = eina_stringshare_ref(ls->t.value);
        eo_lexer_get(ls);
     }
   while (ls->t.token != '}')
     {
        const char *fname;
        Eolian_Struct_Field *fdef;
        Eolian_Type *tp;
        check(ls, TOK_VALUE);
        if (eina_hash_find(def->fields, ls->t.value))
          eo_lexer_syntax_error(ls, "double field definition");
        fname = eina_stringshare_ref(ls->t.value);
        eo_lexer_get(ls);
        check_next(ls, ':');
        tp = parse_type_struct_nonvoid(ls, EINA_TRUE, EINA_FALSE);
        fdef = calloc(1, sizeof(Eolian_Struct_Field));
        fdef->type = tp;
        eina_hash_add(def->fields, fname, fdef);
        pop_type(ls);
        eina_stringshare_del(fname);
        check_next(ls, ';');
        if (ls->t.token == TOK_COMMENT)
          {
             fdef->comment = eina_stringshare_ref(ls->t.value);
             eo_lexer_get(ls);
          }
     }
   check_match(ls, '}', '{', bline, bcolumn);
   def->line = line;
   def->column = column;
   if (name) database_struct_add(def);
   return def;
}

static Eolian_Type *
parse_type_struct(Eo_Lexer *ls, Eina_Bool allow_struct, Eina_Bool allow_anon)
{
   Eolian_Type *def;
   const char *ctype;
   const char *sname = NULL;
   Eina_Strbuf *buf;
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
             Eina_Bool is_extern = EINA_FALSE;
             int line, col;
             if (ls->t.kw == KW_at_extern)
               {
                  is_extern = EINA_TRUE;
                  eo_lexer_get(ls);
               }
             if (allow_anon && ls->t.token == '{')
               {
                  if (is_extern)
                    eo_lexer_syntax_error(ls, "extern anonymous struct");
                  return parse_struct(ls, NULL, EINA_FALSE, 0, 0);
               }
             /* todo: see typedef */
             buf = push_strbuf(ls);
             line = ls->line_number;
             col = ls->column;
             parse_name(ls, buf);
             sname = eina_stringshare_add(eina_strbuf_string_get(buf));
             pop_strbuf(ls);
             if (ls->t.token == '{')
               {
                  Eolian_Type *tp = (Eolian_Type*)eina_hash_find(_structs,
                                                                 sname);
                  if (tp)
                    {
                       ls->line_number = line;
                       ls->column = col;
                       eina_stringshare_del(sname);
                       redef_error(ls, EOLIAN_TYPE_STRUCT, tp);
                    }
                  return parse_struct(ls, sname, is_extern, line, col);
               }
          }
        else
          {
             buf = push_strbuf(ls);
             parse_name(ls, buf);
             sname = eina_stringshare_add(eina_strbuf_string_get(buf));
             pop_strbuf(ls);
          }
        def = push_type(ls);
        def->type = EOLIAN_TYPE_REGULAR_STRUCT;
        _fill_type_name(def, sname);
        goto parse_ptr;
      case KW_func:
        return parse_function_type(ls);
      default:
        break;
     }
   def = push_type(ls);
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
             _fill_type_name(def, eina_stringshare_add(ctype));
             eo_lexer_get(ls);
          }
        else
          {
             buf = push_strbuf(ls);
             parse_name(ls, buf);
             _fill_type_name(def, eina_stringshare_add(eina_strbuf_string_get
                            (buf)));
             pop_strbuf(ls);
          }
     }
parse_ptr:
   while (ls->t.token == '*')
     {
        Eolian_Type *pdef;
        pop_type(ls);
        pdef = push_type(ls);
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
   return parse_type_struct(ls, EINA_FALSE, EINA_FALSE);
}

static Eolian_Type *
parse_typedef(Eo_Lexer *ls)
{
   Eolian_Type *def = push_type(ls);
   Eina_Bool is_extern = EINA_FALSE;
   Eina_Strbuf *buf;
   eo_lexer_get(ls);
   if (ls->t.kw == KW_at_extern)
     {
        is_extern = EINA_TRUE;
        eo_lexer_get(ls);
     }
   def->type = EOLIAN_TYPE_ALIAS;
   def->is_extern = is_extern;
   buf = push_strbuf(ls);
   def->line = ls->line_number;
   def->column = ls->column;
   parse_name(ls, buf);
   _fill_type_name(def, eina_stringshare_add(eina_strbuf_string_get(buf)));
   /* todo: store info about the previous definition and mention it here */
   Eolian_Type *tp = (Eolian_Type*)eina_hash_find(_aliases, def->full_name);
   if (tp)
     {
        ls->line_number = def->line;
        ls->column = def->column;
        redef_error(ls, EOLIAN_TYPE_ALIAS, tp);
     }
   def->file = get_filename(ls);
   (void)!!test_next(ls, ':');
   def->base_type = parse_type_struct_nonvoid(ls, EINA_TRUE, EINA_TRUE);
   pop_type(ls);
   check_next(ls, ';');
   return def;
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
   pop_type(ls);
   if (ls->t.token == '(')
     {
        int line = ls->line_number, col = ls->column;
        eo_lexer_get_balanced(ls, '(', ')');
        ret->default_ret_val = eina_stringshare_ref(ls->t.value);
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
        ret->comment = eina_stringshare_ref(ls->t.value);
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
   if (par->way == EOLIAN_OUT_PARAM)
     par->type = parse_type_void(ls);
   else
     par->type = parse_type(ls);
   pop_type(ls);
   check(ls, TOK_VALUE);
   par->name = eina_stringshare_ref(ls->t.value);
   eo_lexer_get(ls);
   if (ls->t.kw == KW_at_nonull)
     {
        par->nonull = EINA_TRUE;
        eo_lexer_get(ls);
     }
   check_next(ls, ';');
   if (ls->t.token == TOK_COMMENT)
     {
        par->comment = eina_stringshare_ref(ls->t.value);
        eo_lexer_get(ls);
     }
}

static void
parse_legacy(Eo_Lexer *ls)
{
   eo_lexer_get(ls);
   check(ls, TOK_VALUE);
   ls->tmp.legacy_def = eina_stringshare_ref(ls->t.value);
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
   acc->name = eina_stringshare_ref(ls->t.value);
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
        acc->comment = eina_stringshare_ref(ls->t.value);
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
   prop->name = eina_stringshare_ref(ls->t.value);
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
        meth->name = eina_stringshare_ref(ls->t.value);
        eo_lexer_get(ls);
     }
   else
     {
        if (ls->t.kw == KW_protected)
          {
             meth->scope = EOLIAN_SCOPE_PROTECTED;
             eo_lexer_get(ls);
          }
        check(ls, TOK_VALUE);
        meth->name = eina_stringshare_ref(ls->t.value);
        eo_lexer_get(ls);
     }
   line = ls->line_number;
   col = ls->column;
   check_next(ls, '{');
   if (ls->t.token == TOK_COMMENT)
     {
        meth->comment = eina_stringshare_ref(ls->t.value);
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
   Eolian_Implement *impl = NULL;
   buf = push_strbuf(ls);
   impl = calloc(1, sizeof(Eolian_Implement));
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
        impl->full_name = eina_stringshare_add(eina_strbuf_string_get(buf));
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
   impl->full_name = eina_stringshare_add(eina_strbuf_string_get(buf));
   pop_strbuf(ls);
}

static void
parse_event(Eo_Lexer *ls)
{
   Eolian_Event *ev = calloc(1, sizeof(Eolian_Event));
   Eina_Strbuf *buf = push_strbuf(ls);
   ls->tmp.event = ev;
   /* code path not in use yet
   if (ls->t.kw == KW_private)
     {
        eo_lexer_get(ls);
     }*/
   check(ls, TOK_VALUE);
   eina_strbuf_append(buf, ls->t.value);
   eo_lexer_get(ls);
   while (ls->t.token == ',')
     {
        eo_lexer_get(ls);
        check(ls, TOK_VALUE);
        eina_strbuf_append_char(buf, ',');
        eina_strbuf_append(buf, ls->t.value);
        eo_lexer_get(ls);
     }
   ev->name = eina_stringshare_add(eina_strbuf_string_get(buf));
   pop_strbuf(ls);
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
        ev->comment = eina_stringshare_ref(ls->t.value);
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
        ls->tmp.kls->comment = eina_stringshare_ref(ls->t.value);
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
        ls->tmp.kls->legacy_prefix = eina_stringshare_ref(ls->t.value);
        eo_lexer_get(ls);
        check_next(ls, ';');
        break;
      case KW_eo_prefix:
        CASE_LOCK(ls, eo_prefix, "eo prefix definition")
        eo_lexer_get(ls);
        check_next(ls, ':');
        check(ls, TOK_VALUE);
        ls->tmp.kls->eo_prefix = eina_stringshare_ref(ls->t.value);
        eo_lexer_get(ls);
        check_next(ls, ';');
        break;
      case KW_data:
        if (type == EOLIAN_CLASS_INTERFACE) return;
        CASE_LOCK(ls, data, "data definition")
        eo_lexer_get(ls);
        check_next(ls, ':');
        check(ls, TOK_VALUE);
        ls->tmp.kls->data_type = eina_stringshare_ref(ls->t.value);
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
           database_type_add(parse_typedef(ls));
           pop_type(ls);
           break;
        }
      case KW_struct:
        {
           int line, col;
           const char *name;
           Eolian_Type *tp;
           Eina_Bool is_extern = EINA_FALSE;
           Eina_Strbuf *buf;
           eo_lexer_get(ls);
           if (ls->t.kw == KW_at_extern)
             {
                is_extern = EINA_TRUE;
                eo_lexer_get(ls);
             }
           buf = push_strbuf(ls);
           line = ls->line_number;
           col = ls->column;
           parse_name(ls, buf);
           name = eina_stringshare_add(eina_strbuf_string_get(buf));
           /* todo: see typedef */
           tp = (Eolian_Type*)eina_hash_find(_structs, name);
           if (tp)
             {
                ls->line_number = line;
                ls->column = col;
                eina_stringshare_del(name);
                redef_error(ls, EOLIAN_TYPE_STRUCT, tp);
             }
           pop_strbuf(ls);
           parse_struct(ls, name, is_extern, line, col);
           pop_type(ls);
           break;
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
   while (ls->t.token >= 0)
     parse_unit(ls, eot);
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


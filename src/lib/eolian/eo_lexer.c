#include <stdio.h>
#include <ctype.h>

#include <setjmp.h>
#include <assert.h>
#include <libgen.h>

#include "eo_lexer.h"

int _eo_lexer_log_dom = -1;

static int lastbytes = 0;

static void
next_char(Eo_Lexer *ls)
{
   int nb;
   Eina_Bool end = EINA_FALSE;

   if (ls->stream == ls->stream_end)
     {
        end = EINA_TRUE;
        ls->current = '\0';
     }
   else
     ls->current = *(ls->stream++);

   nb = lastbytes;
   if (!nb && end) nb = 1;
   if (!nb) eina_unicode_utf8_next_get(ls->stream - 1, &nb);

   if (nb == 1)
     {
        nb = 0;
        ++ls->icolumn;
        ls->column = ls->icolumn;
     }
   else --nb;

   lastbytes = nb;
}

#define KW(x) #x
#define KWAT(x) "@" #x

static const char * const tokens[] =
{
   "==", "!=", ">=", "<=", "&&", "||", "<<", ">>",

   "<comment>", "<string>", "<char>", "<number>", "<value>",

   KEYWORDS
};

static const char * const ctypes[] =
{
   "signed char", "unsigned char", "char", "short", "unsigned short", "int",
   "unsigned int", "long", "unsigned long", "long long", "unsigned long long",

   "int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t",
   "int64_t", "uint64_t", "int128_t", "uint128_t",

   "size_t", "ssize_t", "intptr_t", "uintptr_t", "ptrdiff_t",

   "time_t",

   "float", "double",

   "Eina_Bool",

   "void",

   "Eina_Accessor", "Eina_Array", "Eina_Iterator", "Eina_Hash", "Eina_List"
};

#undef KW
#undef KWAT

#define is_newline(c) ((c) == '\n' || (c) == '\r')

static Eina_Hash *keyword_map = NULL;

static void
throw(Eo_Lexer *ls, const char *fmt, ...)
{
   const char *ln = ls->stream_line, *end = ls->stream_end;
   Eina_Strbuf *buf = eina_strbuf_new();
   int i;
   va_list ap;
   va_start(ap, fmt);
   eina_strbuf_append_vprintf(buf, fmt, ap);
   va_end(ap);
   eina_strbuf_append_char(buf, ' ');
   while (ln != end && !is_newline(*ln))
     eina_strbuf_append_char(buf,*(ln++));
   eina_strbuf_append_char(buf, '\n');
   for (i = 0; i < ls->column; ++i)
     eina_strbuf_append_char(buf, ' ');
   eina_strbuf_append(buf, "^\n");
   eina_log_print(_eo_lexer_log_dom, EINA_LOG_LEVEL_ERR, ls->source, "",
                  ls->line_number, "%s", eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   longjmp(ls->err_jmp, EINA_TRUE);
}

static void
init_hash(void)
{
   unsigned int i, u;
   if (keyword_map) return;
   keyword_map = eina_hash_string_superfast_new(NULL);
   for (i = u = 13; i < (sizeof(tokens) / sizeof(const char*)); ++i)
     {
         eina_hash_add(keyword_map, tokens[i], (void*)(size_t)(i - u + 1));
     }
}

static void
destroy_hash(void)
{
   if (keyword_map)
     {
        eina_hash_free(keyword_map);
        keyword_map = NULL;
     }
}

static void
txt_token(Eo_Lexer *ls, int token, char *buf)
{
   if (token == TOK_VALUE)
     {
        const char *str = eina_strbuf_string_get(ls->buff);
        memcpy(buf, str, strlen(str) + 1);
     }
   else
     return eo_lexer_token_to_str(token, buf);
}

void eo_lexer_lex_error   (Eo_Lexer *ls, const char *msg, int token);
void eo_lexer_syntax_error(Eo_Lexer *ls, const char *msg);

static void next_line(Eo_Lexer *ls)
{
   int old = ls->current;
   assert(is_newline(ls->current));
   ls->stream_line = ls->stream;
   next_char(ls);
   if (is_newline(ls->current) && ls->current != old)
     {
       next_char(ls);
       ls->stream_line = ls->stream;
     }
   if (++ls->line_number >= INT_MAX)
     eo_lexer_syntax_error(ls, "chunk has too many lines");
   ls->icolumn = ls->column = 0;
}

/* go to next line and strip leading whitespace */
static void next_line_ws(Eo_Lexer *ls)
{
   next_line(ls);
   while (isspace(ls->current) && !is_newline(ls->current))
     next_char(ls);
}

static void
read_long_comment(Eo_Lexer *ls, Eo_Token *tok)
{
   eina_strbuf_reset(ls->buff);

   if (is_newline(ls->current))
     next_line_ws(ls);

   for (;;)
     {
        if (!ls->current)
          eo_lexer_lex_error(ls, "unfinished long comment", -1);
        if (ls->current == '*')
          {
             next_char(ls);
             if (ls->current == '/')
               {
                  next_char(ls);
                  break;
               }
             eina_strbuf_append_char(ls->buff, '*');
          }
        else if (is_newline(ls->current))
          {
             eina_strbuf_append_char(ls->buff, '\n');
             next_line_ws(ls);
          }
        else
          {
             eina_strbuf_append_char(ls->buff, ls->current);
             next_char(ls);
          }
     }
   eina_strbuf_trim(ls->buff);
   if (tok) tok->value.s = eina_stringshare_add(eina_strbuf_string_get(ls->buff));
}

static void
esc_error(Eo_Lexer *ls, int *c, int n, const char *msg)
{
   int i;
   eina_strbuf_reset(ls->buff);
   eina_strbuf_append_char(ls->buff, '\\');
   for (i = 0; i < n && c[i]; ++i)
     eina_strbuf_append_char(ls->buff, c[i]);
   eo_lexer_lex_error(ls, msg, TOK_STRING);
}

static int
hex_val(int c)
{
   if (c >= 'a') return c - 'a' + 10;
   if (c >= 'A') return c - 'A' + 10;
   return c - '0';
}

static int
read_hex_esc(Eo_Lexer *ls)
{
   int c[3] = { 'x' };
   int i, r = 0;
   for (i = 1; i < 3; ++i)
     {
        next_char(ls);
        c[i] = ls->current;
        if (!isxdigit(c[i]))
          esc_error(ls, c, i + 1, "hexadecimal digit expected");
        r = (r << 4) + hex_val(c[i]);
     }
   return r;
}

static int
read_dec_esc(Eo_Lexer *ls)
{
   int c[3];
   int i, r = 0;
   for (i = 0; i < 3 && isdigit(ls->current); ++i)
     {
        c[i] = ls->current;
        r = r * 10 + (c[i] - '0');
        next_char(ls);
     }
   if (r > UCHAR_MAX)
     esc_error(ls, c, i, "decimal escape too large");
   return r;
}

static void
read_escape(Eo_Lexer *ls)
{
   switch (ls->current)
     {
      case 'a': eina_strbuf_append_char(ls->buff, '\a'); next_char(ls); break;
      case 'b': eina_strbuf_append_char(ls->buff, '\b'); next_char(ls); break;
      case 'f': eina_strbuf_append_char(ls->buff, '\f'); next_char(ls); break;
      case 'n': eina_strbuf_append_char(ls->buff, '\n'); next_char(ls); break;
      case 'r': eina_strbuf_append_char(ls->buff, '\r'); next_char(ls); break;
      case 't': eina_strbuf_append_char(ls->buff, '\t'); next_char(ls); break;
      case 'v': eina_strbuf_append_char(ls->buff, '\v'); next_char(ls); break;
      case 'x':
        eina_strbuf_append_char(ls->buff, read_hex_esc(ls));
        next_char(ls);
        break;
      case '\n': case '\r':
        next_line(ls);
        eina_strbuf_append_char(ls->buff, '\n');
        break;
      case '\\': case '"': case '\'':
        eina_strbuf_append_char(ls->buff, ls->current);
        break;
      case '\0':
        break;
      default:
        if (!isdigit(ls->current))
          esc_error(ls, &ls->current, 1, "invalid escape sequence");
        eina_strbuf_append_char(ls->buff, read_dec_esc(ls));
        break;
     }
}

static void
read_string(Eo_Lexer *ls, Eo_Token *tok)
{
   eina_strbuf_reset(ls->buff);
   eina_strbuf_append_char(ls->buff, '"');
   next_char(ls);
   while (ls->current != '"') switch (ls->current)
     {
      case '\0':
        eo_lexer_lex_error(ls, "unfinished string", -1);
        break;
      case '\n': case '\r':
        eo_lexer_lex_error(ls, "unfinished string", TOK_STRING);
        break;
      case '\\':
        {
           next_char(ls);
           read_escape(ls);
           break;
        }
      default:
        eina_strbuf_append_char(ls->buff, ls->current);
        next_char(ls);
     }
   eina_strbuf_append_char(ls->buff, ls->current);
   next_char(ls);
   tok->value.s = eina_stringshare_add_length(eina_strbuf_string_get(ls->buff) + 1,
                                (unsigned int)eina_strbuf_length_get(ls->buff) - 2);
}

static int
get_type(Eo_Lexer *ls, Eina_Bool is_float)
{
   if (is_float)
     {
        if (ls->current == 'f' || ls->current == 'F')
          {
             next_char(ls);
             return NUM_FLOAT;
          }
        return NUM_DOUBLE;
     }
   if (ls->current == 'u' || ls->current == 'U')
     {
        next_char(ls);
        if (ls->current == 'l' || ls->current == 'L')
          {
             next_char(ls);
             if (ls->current == 'l' || ls->current == 'L')
               {
                  next_char(ls);
                  return NUM_ULLONG;
               }
             return NUM_ULONG;
          }
        return NUM_UINT;
     }
   if (ls->current == 'l' || ls->current == 'L')
     {
        next_char(ls);
        if (ls->current == 'l' || ls->current == 'L')
          {
             next_char(ls);
             return NUM_LLONG;
          }
        return NUM_LONG;
     }
   return NUM_INT;
}

static void
write_val(Eo_Lexer *ls, Eo_Token *tok, Eina_Bool is_float)
{
   const char *str = eina_strbuf_string_get(ls->buff);
   int type = get_type(ls, is_float);
   char *end = NULL;
   if (is_float)
     {
        if (type == NUM_FLOAT)
          tok->value.f = strtof(str, &end);
        else if (type == NUM_DOUBLE)
          tok->value.d = strtod(str, &end);
     }
   else
     {
        /* signed is always in the same memory location */
        if (type == NUM_INT || type == NUM_UINT)
          tok->value.u = strtoul(str, &end, 0);
        else if (type == NUM_LONG || type == NUM_ULONG)
          tok->value.ul = strtoul(str, &end, 0);
        else if (type == NUM_LLONG || type == NUM_ULLONG)
          tok->value.ull = strtoull(str, &end, 0);
     }
   if (end && end[0])
     eo_lexer_lex_error(ls, "malformed number", TOK_NUMBER);
   tok->kw = type;
}

static void
write_exp(Eo_Lexer *ls)
{
   eina_strbuf_append_char(ls->buff, ls->current);
   next_char(ls);
   if (ls->current == '+' || ls->current == '-')
     {
        eina_strbuf_append_char(ls->buff, ls->current);
        next_char(ls);
        while (isdigit(ls->current))
          {
             eina_strbuf_append_char(ls->buff, ls->current);
             next_char(ls);
          }
     }
}

static void
read_hex_number(Eo_Lexer *ls, Eo_Token *tok)
{
   Eina_Bool is_float = EINA_FALSE;
   while (isxdigit(ls->current) || ls->current == '.')
     {
        eina_strbuf_append_char(ls->buff, ls->current);
        if (ls->current == '.') is_float = EINA_TRUE;
        next_char(ls);
     }
   if (is_float && (ls->current != 'p' && ls->current != 'P'))
     {
        eo_lexer_lex_error(ls, "hex float literals require an exponent",
                           TOK_NUMBER);
     }
   if (ls->current == 'p' || ls->current == 'P')
     {
        is_float = EINA_TRUE;
         write_exp(ls);
     }
   write_val(ls, tok, is_float);
}

static void
read_number(Eo_Lexer *ls, Eo_Token *tok)
{
   Eina_Bool is_float = eina_strbuf_string_get(ls->buff)[0] == '.';
   if (ls->current == '0' && !is_float)
     {
        eina_strbuf_append_char(ls->buff, ls->current);
        next_char(ls);
        if (ls->current == 'x' || ls->current == 'X')
          {
             eina_strbuf_append_char(ls->buff, ls->current);
             next_char(ls);
             read_hex_number(ls, tok);
             return;
          }
     }
   while (isdigit(ls->current) || ls->current == '.')
     {
        eina_strbuf_append_char(ls->buff, ls->current);
        if (ls->current == '.') is_float = EINA_TRUE;
        next_char(ls);
     }
   if (ls->current == 'e' || ls->current == 'E')
     {
        is_float = EINA_TRUE;
         write_exp(ls);
     }
   write_val(ls, tok, is_float);
}

static int
lex(Eo_Lexer *ls, Eo_Token *tok)
{
   eina_strbuf_reset(ls->buff);
   tok->value.s = NULL;
   for (;;) switch (ls->current)
     {
      case '\n':
      case '\r':
        next_line(ls);
        continue;
      case '/':
        {
           Eina_Bool doc = EINA_FALSE;
           next_char(ls);
           if (ls->current == '*')
             {
                next_char(ls);
                if ((doc = (ls->current == '@')))
                  next_char(ls);
                read_long_comment(ls, doc ? tok : NULL);
                if (doc)
                  return TOK_COMMENT;
                else
                  continue;
             }
           else if (ls->current != '/') return '/';
           next_char(ls);
           if ((doc = (ls->current == '@')))
             next_char(ls);
           eina_strbuf_reset(ls->buff);
           while (ls->current && !is_newline(ls->current))
             {
                eina_strbuf_append_char(ls->buff, ls->current);
                next_char(ls);
             }
           eina_strbuf_trim(ls->buff);
           if (doc)
             {
                tok->value.s = eina_stringshare_add(eina_strbuf_string_get(ls->buff));
                return TOK_COMMENT;
             }
           continue;
        }
      case '\0':
        return -1;
      case '=':
        next_char(ls);
        if (!ls->expr_mode || (ls->current != '=')) return '=';
        next_char(ls);
        return TOK_EQ;
      case '!':
        next_char(ls);
        if (!ls->expr_mode || (ls->current != '=')) return '!';
        next_char(ls);
        return TOK_NQ;
      case '>':
        next_char(ls);
        if (!ls->expr_mode) return '>';
        if (ls->current == '=')
          {
             next_char(ls);
             return TOK_GE;
          }
        else if (ls->current == '>')
          {
             next_char(ls);
             return TOK_RSH;
          }
        return '>';
      case '<':
        next_char(ls);
        if (!ls->expr_mode) return '<';
        if (ls->current == '=')
          {
             next_char(ls);
             return TOK_LE;
          }
        else if (ls->current == '<')
          {
             next_char(ls);
             return TOK_LSH;
          }
        return '<';
      case '&':
        next_char(ls);
        if (!ls->expr_mode || (ls->current != '&')) return '&';
        next_char(ls);
        return TOK_AND;
      case '|':
        next_char(ls);
        if (!ls->expr_mode || (ls->current != '|')) return '|';
        next_char(ls);
        return TOK_OR;
      case '"':
        if (!ls->expr_mode)
          {
             next_char(ls);
             return '"';
          }
        read_string(ls, tok);
        return TOK_STRING;
      case '\'':
        next_char(ls);
        if (!ls->expr_mode) return '\'';
        if (ls->current == '\\')
          {
             next_char(ls);
             eina_strbuf_reset(ls->buff);
             read_escape(ls);
             tok->value.c = (char)*eina_strbuf_string_get(ls->buff);
          }
        else
          {
             tok->value.c = ls->current;
             next_char(ls);
          }
        if (ls->current != '\'')
          eo_lexer_lex_error(ls, "unfinished character", TOK_CHAR);
        next_char(ls);
        return TOK_CHAR;
      case '.':
        next_char(ls);
        if (!ls->expr_mode) return '.';
        if (!isdigit(ls->current)) return '.';
        eina_strbuf_reset(ls->buff);
        eina_strbuf_append_char(ls->buff, '.');
        read_number(ls, tok);
        return TOK_NUMBER;
      default:
        {
           if (isspace(ls->current))
             {
                assert(!is_newline(ls->current));
                next_char(ls);
                continue;
             }
           else if (ls->expr_mode && isdigit(ls->current))
             {
                eina_strbuf_reset(ls->buff);
                read_number(ls, tok);
                return TOK_NUMBER;
             }
           if (ls->current && (isalnum(ls->current)
               || ls->current == '@' || ls->current == '_'))
             {
                int col = ls->column;
                Eina_Bool at_kw = (ls->current == '@');
                const char *str;
                eina_strbuf_reset(ls->buff);
                do
                  {
                     eina_strbuf_append_char(ls->buff, ls->current);
                     next_char(ls);
                  }
                while (ls->current && (isalnum(ls->current)
                       || ls->current == '_'));
                str     = eina_strbuf_string_get(ls->buff);
                tok->kw = (int)(uintptr_t)eina_hash_find(keyword_map,
                                                        str);
                ls->column = col + 1;
                if (at_kw && tok->kw == 0)
                  eo_lexer_syntax_error(ls, "invalid keyword");
                tok->value.s = eina_stringshare_add(str);
                return TOK_VALUE;
             }
           else
             {
                int c = ls->current;
                next_char(ls);
                return c;
             }
        }
     }
}

static const char *
get_filename(Eo_Lexer *ls)
{
   char *dup = strdup(ls->source);
   char *s = basename(dup);
   const char *file = eina_stringshare_add(s);
   free(dup);
   return file;
}

static void
eo_lexer_set_input(Eo_Lexer *ls, const char *source)
{
   Eina_File *f = eina_file_open(source, EINA_FALSE);
   if (!f)
     {
        ERR("%s", strerror(errno));
        longjmp(ls->err_jmp, EINA_TRUE);
     }
   ls->lookahead.token = -1;
   ls->buff            = eina_strbuf_new();
   ls->handle          = f;
   ls->stream          = eina_file_map_all(f, EINA_FILE_RANDOM);
   ls->stream_end      = ls->stream + eina_file_size_get(f);
   ls->stream_line     = ls->stream;
   ls->source          = eina_stringshare_add(source);
   ls->filename        = get_filename(ls);
   ls->line_number     = 1;
   ls->icolumn         = ls->column = 0;
   next_char(ls);
   if (ls->current != 0xEF)
     return;
   next_char(ls);
   if (ls->current != 0xBB)
     return;
   next_char(ls);
   if (ls->current != 0xBF)
     return;
   next_char(ls);
}

static void
_temps_free(Eo_Lexer_Temps *tmp)
{
   Eina_Strbuf *buf;
   Eolian_Type *tp;
   Eolian_Class *cl;
   const char *s;

   if (tmp->kls)
     database_class_del(tmp->kls);

   if (tmp->var)
     database_var_del(tmp->var);

   EINA_LIST_FREE(tmp->str_bufs, buf)
     eina_strbuf_free(buf);

   EINA_LIST_FREE(tmp->type_defs, tp)
     database_type_del(tp);

   EINA_LIST_FREE(tmp->strs, s)
     if (s) eina_stringshare_del(s);

   EINA_LIST_FREE(tmp->classes, cl)
     database_class_del(cl);
}

void
eo_lexer_free(Eo_Lexer *ls)
{
   if (!ls) return;
   if (ls->source  ) eina_stringshare_del(ls->source);
   if (ls->filename) eina_stringshare_del(ls->filename);
   if (ls->buff    ) eina_strbuf_free    (ls->buff);
   if (ls->handle  ) eina_file_close     (ls->handle);

   eo_lexer_context_clear(ls);
   _temps_free(&ls->tmp);
   free(ls);
}

Eo_Lexer *
eo_lexer_new(const char *source)
{
   Eo_Lexer   *ls = calloc(1, sizeof(Eo_Lexer));
   if (!setjmp(ls->err_jmp))
     {
        eo_lexer_set_input(ls, source);
        return ls;
     }
   eo_lexer_free(ls);
   return NULL;
}

int
eo_lexer_get(Eo_Lexer *ls)
{
   if (ls->t.token >= START_CUSTOM && ls->t.token != TOK_NUMBER
                                   && ls->t.token != TOK_CHAR)
     {
        eina_stringshare_del(ls->t.value.s);
        ls->t.value.s = NULL;
     }
   if (ls->lookahead.token >= 0)
     {
        ls->t               = ls->lookahead;
        ls->lookahead.token = -1;
        return ls->t.token;
     }
   ls->t.kw = 0;
   return (ls->t.token = lex(ls, &ls->t));
}

int
eo_lexer_lookahead(Eo_Lexer *ls)
{
   assert (ls->lookahead.token < 0);
   ls->lookahead.kw = 0;
   eo_lexer_context_push(ls);
   ls->lookahead.token = lex(ls, &ls->lookahead);
   eo_lexer_context_restore(ls);
   eo_lexer_context_pop(ls);
   return ls->lookahead.token;
}

void
eo_lexer_lex_error(Eo_Lexer *ls, const char *msg, int token)
{
   if (token)
     {
        char buf[256];
        txt_token(ls, token, buf);
        throw(ls, "%s at column %d near '%s'\n", msg, ls->column, buf);
     }
   else
     throw(ls, "%s at column %d\n", msg, ls->column);
}

void
eo_lexer_syntax_error(Eo_Lexer *ls, const char *msg)
{
   eo_lexer_lex_error(ls, msg, ls->t.token);
}

void
eo_lexer_token_to_str(int token, char *buf)
{
   if (token < 0)
     {
        memcpy(buf, "<eof>", 6);
     }
   if (token < START_CUSTOM)
     {
        assert((unsigned char)token == token);
        if (iscntrl(token))
          sprintf(buf, "char(%d)", token);
        else
          sprintf(buf, "%c", token);
     }
   else
     {
        const char *v = tokens[token - START_CUSTOM];
        memcpy(buf, v, strlen(v) + 1);
     }
}

const char *
eo_lexer_keyword_str_get(int kw)
{
   return tokens[kw + 12];
}

Eina_Bool
eo_lexer_is_type_keyword(int kw)
{
   return (kw >= KW_byte && kw <= KW_list);
}

int
eo_lexer_keyword_str_to_id(const char *kw)
{
   return (int)(uintptr_t)eina_hash_find(keyword_map, kw);
}

const char *
eo_lexer_get_c_type(int kw)
{
   if (!eo_lexer_is_type_keyword(kw)) return NULL;
   return ctypes[kw - KW_byte];
}

static int _init_counter = 0;

int
eo_lexer_init()
{
   if (!_init_counter)
     {
        eina_init();
        init_hash();
        eina_log_color_disable_set(EINA_FALSE);
        _eo_lexer_log_dom = eina_log_domain_register("eo_lexer", EINA_COLOR_CYAN);
     }
   return _init_counter++;
}

int
eo_lexer_shutdown()
{
   if (_init_counter <= 0) return 0;
   _init_counter--;
   if (!_init_counter)
     {
        eina_log_domain_unregister(_eo_lexer_log_dom);
        _eo_lexer_log_dom = -1;
        destroy_hash();
        eina_shutdown();
     }
   return _init_counter;
}

void
eo_lexer_context_push(Eo_Lexer *ls)
{
   Lexer_Ctx *ctx = malloc(sizeof(Lexer_Ctx));
   ctx->line = ls->line_number;
   ctx->column = ls->column;
   ctx->linestr = ls->stream_line;
   ls->saved_ctxs = eina_list_prepend(ls->saved_ctxs, ctx);
}

void
eo_lexer_context_pop(Eo_Lexer *ls)
{
   Lexer_Ctx *ctx = (Lexer_Ctx*)eina_list_data_get(ls->saved_ctxs);
   free(ctx);
   ls->saved_ctxs = eina_list_remove_list(ls->saved_ctxs, ls->saved_ctxs);
}

void
eo_lexer_context_restore(Eo_Lexer *ls)
{
   if (!eina_list_count(ls->saved_ctxs)) return;
   Lexer_Ctx *ctx = (Lexer_Ctx*)eina_list_data_get(ls->saved_ctxs);
   ls->line_number = ctx->line;
   ls->column      = ctx->column;
   ls->stream_line = ctx->linestr;
}

void
eo_lexer_context_clear(Eo_Lexer *ls)
{
   Lexer_Ctx *ctx;
   EINA_LIST_FREE(ls->saved_ctxs, ctx) free(ctx);
}

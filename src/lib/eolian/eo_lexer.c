#include <stdio.h>
#include <ctype.h>

#include <setjmp.h>
#include <assert.h>

#include "eo_lexer.h"

int _eo_lexer_log_dom = -1;

static void next_char(Eo_Lexer *ls)
{
   if (ls->stream == ls->stream_end)
     ls->current = '\0';
   else
     ls->current = *(ls->stream++);
}

#define KW(x) #x
#define KWAT(x) "@" #x

static const char * const tokens[] =
{
   "<comment>", "<eof>", "<value>",

   KEYWORDS
};

#undef KW
#undef KWAT

static Eina_Hash *keyword_map = NULL;

static void
throw(Eo_Lexer *ls, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    longjmp(ls->err_jmp, EINA_TRUE);
}

static void
init_hash(void)
{
   unsigned int i;
   if (keyword_map) return;
   keyword_map = eina_hash_string_superfast_new(NULL);
   for (i = 3; i < (sizeof(tokens) / sizeof(const char*)); ++i)
     {
         eina_hash_add(keyword_map, tokens[i], (void*)(size_t)(i - 2));
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

#define is_newline(c) ((c) == '\n' || (c) == '\r')

static void next_line(Eo_Lexer *ls)
{
   int old = ls->current;
   assert(is_newline(ls->current));
   next_char(ls);
   if (is_newline(ls->current) && ls->current != old)
      next_char(ls);
   if (++ls->line_number >= INT_MAX)
      eo_lexer_syntax_error(ls, "chunk has too many lines");
}

/* go to next line and strip leading whitespace */
static void next_line_ws(Eo_Lexer *ls)
{
   next_line(ls);
   while (isspace(ls->current) && !is_newline(ls->current))
     next_char(ls);
}

static void
read_long_comment(Eo_Lexer *ls, const char **value)
{
   eina_strbuf_reset(ls->buff);

   if (is_newline(ls->current))
      next_line_ws(ls);

   for (;;)
     {
        if (!ls->current)
             eo_lexer_lex_error(ls, "unfinished long comment", TOK_EOF);
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
   if (value) *value = eina_strbuf_string_get(ls->buff);
}

static int
lex(Eo_Lexer *ls, const char **value, int *kwid, const char *chars)
{
   eina_strbuf_reset(ls->buff);
   for (;;)
     {
        switch (ls->current)
          {
             case '\n':
             case '\r':
                next_line(ls);
                continue;
             case '/':
               {
                  next_char(ls);
                  if (ls->current == '*')
                    {
                       Eina_Bool doc;
                       next_char(ls);
                       if ((doc = (ls->current == '@')))
                          next_char(ls);
                       read_long_comment(ls, doc ? value : NULL);
                       if (doc)
                          return TOK_COMMENT;
                       else
                          continue;
                    }
                  else if (ls->current != '/') return '/';
                  while (ls->current && !is_newline(ls->current))
                     next_char(ls);
                  continue;
               }
             case '\0':
               return TOK_EOF;
             default:
               {
                  if (isspace(ls->current))
                    {
                       assert(!is_newline(ls->current));
                       next_char(ls);
                       continue;
                    }
                  if (ls->current && (isalnum(ls->current)
                      || ls->current == '@'
                      || strchr(chars, ls->current)))
                    {
                       Eina_Bool at_kw = (ls->current == '@');
                       const char *str;
                       eina_strbuf_reset(ls->buff);
                       do
                         {
                            eina_strbuf_append_char(ls->buff, ls->current);
                            next_char(ls);
                         }
                       while (ls->current && (isalnum(ls->current)
                                     || strchr(chars, ls->current)));
                       str    = eina_strbuf_string_get(ls->buff);
                       *kwid  = (int)(uintptr_t)eina_hash_find(keyword_map,
                                                               str);
                       if (at_kw && *kwid == 0)
                         eo_lexer_syntax_error(ls, "invalid keyword");
                       *value = str;
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
}

static int
lex_balanced(Eo_Lexer *ls, const char **value, int *kwid, char beg, char end)
{
   int depth = 0;
   const char *str;
   eina_strbuf_reset(ls->buff);
   while (ls->current)
     {
        if (ls->current == beg)
          ++depth;
        else if (ls->current == end)
          --depth;

        if (depth == -1)
          break;

        eina_strbuf_append_char(ls->buff, ls->current);
        next_char(ls);
     }
   eina_strbuf_trim(ls->buff);
   str    = eina_strbuf_string_get(ls->buff);
   *kwid  = (int)(uintptr_t)eina_hash_find(keyword_map, str);
   *value = str;
   return TOK_VALUE;
}

static int
lex_until(Eo_Lexer *ls, const char **value, int *kwid, char end)
{
   const char *str;
   eina_strbuf_reset(ls->buff);
   while (ls->current)
     {
        if (ls->current == end)
          break;
        eina_strbuf_append_char(ls->buff, ls->current);
        next_char(ls);
     }
   eina_strbuf_trim(ls->buff);
   str    = eina_strbuf_string_get(ls->buff);
   *kwid  = (int)(uintptr_t)eina_hash_find(keyword_map, str);
   *value = str;
   return TOK_VALUE;
}

static void
eo_lexer_set_input(Eo_Lexer *ls, const char *source)
{
   Eina_File *f = eina_file_open(source, EINA_FALSE);
   if (!f)
      throw(ls, "%s\n", strerror(errno));
   ls->lookahead.token = TOK_EOF;
   ls->buff            = eina_strbuf_new();
   ls->handle          = f;
   ls->stream          = eina_file_map_all(f, EINA_FILE_RANDOM);
   ls->stream_end      = ls->stream + eina_file_size_get(f);
   ls->source          = eina_stringshare_add(source);
   ls->line_number     = 1;
   next_char(ls);
}

void
eo_lexer_free(Eo_Lexer *ls)
{
   Eo_Class_Def *kls;
   Eo_Type_Def *type;

   if (!ls) return;
   if (ls->source) eina_stringshare_del(ls->source);
   if (ls->buff  ) eina_strbuf_free    (ls->buff);
   if (ls->handle) eina_file_close     (ls->handle);

   EINA_LIST_FREE(ls->classes, kls)
      eo_definitions_class_def_free(kls);

   EINA_LIST_FREE(ls->typedefs, type)
      eo_definitions_type_def_free(type);

   eo_definitions_temps_free(&ls->tmp);

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
eo_lexer_get_balanced(Eo_Lexer *ls, char beg, char end)
{
   assert(ls->lookahead.token == TOK_EOF);
   return (ls->t.token == lex_balanced(ls, &ls->t.value, &ls->t.kw, beg, end));
}

int
eo_lexer_get_until(Eo_Lexer *ls, char end)
{
   assert(ls->lookahead.token == TOK_EOF);
   return (ls->t.token == lex_until(ls, &ls->t.value, &ls->t.kw, end));
}

int
eo_lexer_get(Eo_Lexer *ls)
{
   return eo_lexer_get_ident(ls, "_");
}

int
eo_lexer_lookahead(Eo_Lexer *ls)
{
   return eo_lexer_lookahead_ident(ls, "_");
}

int
eo_lexer_lookahead_ident(Eo_Lexer *ls, const char *chars)
{
   assert (ls->lookahead.token == TOK_EOF);
   ls->lookahead.kw = 0;
   return (ls->lookahead.token = lex(ls, &ls->lookahead.value,
          &ls->lookahead.kw, chars));
}

int
eo_lexer_get_ident(Eo_Lexer *ls, const char *chars)
{
   if (ls->lookahead.token != TOK_EOF)
     {
        ls->t               = ls->lookahead;
        ls->lookahead.token = TOK_EOF;
        return ls->t.token;
     }
   ls->t.kw = 0;
   return (ls->t.token = lex(ls, &ls->t.value, &ls->t.kw, chars));
}

void
eo_lexer_lex_error(Eo_Lexer *ls, const char *msg, int token)
{
   if (token)
     {
        char buf[256];
        txt_token(ls, token, buf);
        throw(ls, "%s: %d: %s near '%s'\n", ls->source, ls->line_number, msg,
              buf);
     }
   else
      throw(ls, "%s: %d: %s\n", ls->source, ls->line_number, msg);
}

void
eo_lexer_syntax_error(Eo_Lexer *ls, const char *msg)
{
   eo_lexer_lex_error(ls, msg, ls->t.token);
}

void
eo_lexer_token_to_str(int token, char *buf)
{
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
   return tokens[kw + 2];
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
        _eo_lexer_log_dom = eina_log_domain_register("eo_toknz", EINA_COLOR_CYAN);
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

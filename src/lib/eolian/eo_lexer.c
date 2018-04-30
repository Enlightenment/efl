#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <ctype.h>
#include <locale.h>

#include <setjmp.h>
#include <assert.h>

#include "eo_lexer.h"
#include "eolian_priv.h"

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
   "<doc>", "<string>", "<char>", "<number>", "<value>"
};

static const char * const keywords[] = { KEYWORDS };

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

   "Eina_Accessor *", "Eina_Array *", "Eina_Iterator *", "Eina_Hash *",
   "Eina_List *", "Eina_Inarray *", "Eina_Inlist *",
   "Eina_Value", "Eina_Value *",
   "char *", "const char *", "Eina_Stringshare *", "Eina_Strbuf *",

   "void *",

   "Eina_Free_Cb",
   "function",
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
   eina_strbuf_append(buf, "\n ");
   while (ln != end && !is_newline(*ln))
     eina_strbuf_append_char(buf,*(ln++));
   eina_strbuf_append_char(buf, '\n');
   for (i = 0; i < ls->column; ++i)
     eina_strbuf_append_char(buf, ' ');
   eina_strbuf_append(buf, "^\n");
   Eolian_Object tmp;
   memset(&tmp, 0, sizeof(Eolian_Object));
   tmp.unit = ls->unit;
   tmp.file = ls->source;
   tmp.line = ls->line_number;
   tmp.column = ls->column;
   eolian_state_log_obj(ls->state, &tmp, "%s", eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   longjmp(ls->err_jmp, EO_LEXER_ERROR_NORMAL);
}

void
eo_lexer_init(void)
{
   unsigned int i;
   if (keyword_map) return;
   keyword_map = eina_hash_string_superfast_new(NULL);
   for (i = 0; i < (sizeof(keywords) / sizeof(keywords[0])); ++i)
     eina_hash_add(keyword_map, keywords[i], (void *)(size_t)(i + 1));
}

void
eo_lexer_shutdown(void)
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
     memcpy(buf, ls->t.value.s, strlen(ls->t.value.s) + 1);
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
   if (++ls->iline_number >= INT_MAX)
     eo_lexer_syntax_error(ls, "chunk has too many lines");
   ls->line_number = ls->iline_number;
   ls->icolumn = ls->column = 0;
}

static void skip_ws(Eo_Lexer *ls)
{
   while (isspace(ls->current) && !is_newline(ls->current))
     next_char(ls);
}

/* go to next line and strip leading whitespace */
static void next_line_ws(Eo_Lexer *ls)
{
   next_line(ls);
   skip_ws(ls);
}

static Eina_Bool
should_skip_star(Eo_Lexer *ls, int ccol, Eina_Bool *term)
{
   Eina_Bool had_star = EINA_FALSE;
   if (ls->column == ccol && ls->current == '*')
     {
        had_star = EINA_TRUE;
        next_char(ls);
        if (ls->current == '/')
          {
             next_char(ls);
             *term = EINA_TRUE;
             return EINA_FALSE;
          }
        skip_ws(ls);
     }
   return had_star;
}

static void
read_long_comment(Eo_Lexer *ls, int ccol)
{
   Eina_Bool had_star = EINA_FALSE, had_nl = EINA_FALSE;
   eina_strbuf_reset(ls->buff);

   if (is_newline(ls->current))
     {
        Eina_Bool term = EINA_FALSE;
        had_nl = EINA_TRUE;
        next_line_ws(ls);
        had_star = should_skip_star(ls, ccol, &term);
        if (term) goto cend;
     }

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
             if (!had_nl)
               {
                  Eina_Bool term = EINA_FALSE;
                  had_nl = EINA_TRUE;
                  had_star = should_skip_star(ls, ccol, &term);
                  if (term) break;
               }
             else if (had_star && ls->column == ccol && ls->current == '*')
               {
                  next_char(ls);
                  if (ls->current == '/')
                    {
                       next_char(ls);
                       break;
                    }
                  skip_ws(ls);
                }
          }
        else
          {
             eina_strbuf_append_char(ls->buff, ls->current);
             next_char(ls);
          }
     }
cend:
   eina_strbuf_trim(ls->buff);
}

enum Doc_Tokens {
    DOC_MANGLED = -2, DOC_UNFINISHED = -1, DOC_TEXT = 0, DOC_SINCE = 1
};

static void
doc_ref_class(Eo_Lexer *ls, const char *cname)
{
   size_t clen = strlen(cname);
   char *buf = alloca(clen + 4);
   memcpy(buf, cname, clen);
   buf[clen] = '\0';
   for (char *p = buf; *p; ++p)
     {
        if (*p == '.')
          *p = '_';
        else
          *p = tolower(*p);
     }
   memcpy(buf + clen, ".eo", sizeof(".eo"));
   if (!eina_hash_find(ls->state->filenames_eo, buf))
     return;
   /* ref'd classes do not become dependencies */
   database_defer(ls->state, buf, EINA_FALSE);
}

static void
doc_ref(Eo_Lexer *ls, Eolian_Documentation *doc)
{
   const char *st = ls->stream, *ste = ls->stream_end;
   size_t rlen = 0;
   while ((st != ste) && ((*st == '.') || (*st == '_') || isalnum(*st)))
     {
        ++st;
        ++rlen;
     }
   if ((rlen > 1) && (*(st - 1) == '.'))
     --rlen;
   if (!rlen)
     return;
   if (*ls->stream == '.')
     return;

   char *buf = alloca(rlen + 1);
   memcpy(buf, ls->stream, rlen);
   buf[rlen] = '\0';

   /* actual full class name */
   doc_ref_class(ls, buf);

   /* it's definitely a reference, add debug info
    * 20 bits for line and 12 bits for column, good enough
    */
   doc->ref_dbg = eina_list_append(doc->ref_dbg,
     (void *)(size_t)((ls->line_number & 0xFFFFF) | (((ls->column + 1) & 0xFFF) << 20)));

   /* method name at the end */
   char *end = strrchr(buf, '.');
   if (!end)
     return;
   *end = '\0';
   doc_ref_class(ls, buf);

   /* .get or .set at the end, handle possible property */
   if (strcmp(end + 1, "get") && strcmp(end + 1, "set"))
     return;
   end = strrchr(buf, '.');
   if (!end)
     return;
   *end = '\0';
   doc_ref_class(ls, buf);
}

static int
doc_lex(Eo_Lexer *ls, Eolian_Documentation *doc, Eina_Bool *term, Eina_Bool *since)
{
   int tokret = -1;
   eina_strbuf_reset(ls->buff);
   *since = EINA_FALSE;
   for (;;) switch (ls->current)
     {
      /* error case */
      case '\0':
        return DOC_UNFINISHED;
      /* newline case: if two or more newlines are present, new paragraph
       * if only one newline is present, append space to the text buffer
       * when starting new paragraph, reset doc continutation
       */
      case '\n':
      case '\r':
        next_line(ls);
        skip_ws(ls);
        if (!is_newline(ls->current))
          {
             eina_strbuf_append_char(ls->buff, ' ');
             continue;
          }
        while (is_newline(ls->current))
          next_line_ws(ls);
        tokret = DOC_TEXT;
        goto exit_with_token;
      /* escape case: for any \X, output \X
       * except for \\]], then output just ]]
       */
      case '\\':
        next_char(ls);
        if (ls->current == ']')
          {
             next_char(ls);
             if (ls->current == ']')
               {
                  next_char(ls);
                  eina_strbuf_append(ls->buff, "]]");
               }
             else
               eina_strbuf_append(ls->buff, "\\]");
          }
        else
          eina_strbuf_append_char(ls->buff, '\\');
        continue;
      /* terminating case */
      case ']':
        next_char(ls);
        if (ls->current == ']')
          {
             /* terminate doc */
             tokret = DOC_TEXT;
             goto terminated;
          }
        eina_strbuf_append_char(ls->buff, ']');
        continue;
      /* references and @since */
      case '@':
        if ((size_t)(ls->stream_end - ls->stream) >= (sizeof("since")) &&
            !memcmp(ls->stream, "since ", sizeof("since")))
          {
             next_char(ls);
             *since = EINA_TRUE;
             for (size_t i = 0; i < sizeof("since"); ++i)
               next_char(ls);
             skip_ws(ls);
             tokret = DOC_TEXT;
             goto exit_with_token;
          }
        doc_ref(ls, doc);
        eina_strbuf_append_char(ls->buff, '@');
        next_char(ls);
        /* in-class references */
        if (ls->klass && ls->current == '.')
          {
             next_char(ls);
             if (isalpha(ls->current) || ls->current == '_')
               eina_strbuf_append(ls->buff, ls->klass->base.name);
             eina_strbuf_append_char(ls->buff, '.');
          }
        continue;
      /* default case - append character */
      default:
        eina_strbuf_append_char(ls->buff, ls->current);
        next_char(ls);
        continue;
     }
terminated:
   next_char(ls);
   *term = EINA_TRUE;
exit_with_token:
   eina_strbuf_trim(ls->buff);
   return tokret;
}

static int
read_since(Eo_Lexer *ls)
{
   eina_strbuf_reset(ls->buff);
   while (ls->current && (ls->current == '.' ||
                          ls->current == '_' ||
                          isalnum(ls->current)))
     {
        eina_strbuf_append_char(ls->buff, ls->current);
        next_char(ls);
     }
   if (!eina_strbuf_length_get(ls->buff))
     return DOC_UNFINISHED;
   skip_ws(ls);
   while (is_newline(ls->current))
     next_line_ws(ls);
   if (ls->current != ']')
     return DOC_MANGLED;
   next_char(ls);
   if (ls->current != ']')
     return DOC_MANGLED;
   next_char(ls);
   return DOC_SINCE;
}

void doc_error(Eo_Lexer *ls, const char *msg, Eolian_Documentation *doc, Eina_Strbuf *buf)
{
   eina_stringshare_del(doc->summary);
   eina_stringshare_del(doc->description);
   eina_list_free(doc->ref_dbg);
   free(doc);
   eina_strbuf_free(buf);
   eo_lexer_lex_error(ls, msg, -1);
}

static void
read_doc(Eo_Lexer *ls, Eo_Token *tok, int line, int column)
{
   Eolian_Documentation *doc = calloc(1, sizeof(Eolian_Documentation));
   if (!doc)
     longjmp(ls->err_jmp, EO_LEXER_ERROR_OOM);

   doc->base.unit = ls->unit;
   doc->base.file = ls->filename;
   doc->base.line = line;
   doc->base.column = column;
   doc->base.type = EOLIAN_OBJECT_DOCUMENTATION;

   Eina_Strbuf *rbuf = eina_strbuf_new();

   Eina_Bool term = EINA_FALSE, since = EINA_FALSE;
   while (!term)
     {
        int read;
        if (since)
          {
             read = read_since(ls);
             term = EINA_TRUE;
          }
        else
          read = doc_lex(ls, doc, &term, &since);
        switch (read)
          {
           case DOC_MANGLED:
             doc_error(ls, "mangled documentation", doc, rbuf);
             return;
           case DOC_UNFINISHED:
             doc_error(ls, "unfinished documentation", doc, rbuf);
             return;
           case DOC_TEXT:
             if (!eina_strbuf_length_get(ls->buff))
               continue;
             if (!doc->summary)
               doc->summary = eina_stringshare_add(eina_strbuf_string_get(ls->buff));
             else
               {
                  if (eina_strbuf_length_get(rbuf))
                    eina_strbuf_append(rbuf, "\n\n");
                  eina_strbuf_append(rbuf, eina_strbuf_string_get(ls->buff));
               }
             break;
           case DOC_SINCE:
             doc->since = eina_stringshare_add(eina_strbuf_string_get(ls->buff));
             break;
          }
     }

   if (eina_strbuf_length_get(rbuf))
     doc->description = eina_stringshare_add(eina_strbuf_string_get(rbuf));
   if (!doc->summary)
     doc->summary = eina_stringshare_add("No description supplied.");
   if (!doc->since && ls->klass && ls->klass->doc)
     doc->since = eina_stringshare_ref(ls->klass->doc->since);
   eina_strbuf_free(rbuf);
   tok->value.doc = doc;
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
replace_decpoint(Eo_Lexer *ls, char prevdecp)
{
   if (ls->decpoint == prevdecp) return;
   char *bufs = eina_strbuf_string_steal(ls->buff);
   char *p = bufs;
   while ((p = strchr(p, prevdecp))) *p = ls->decpoint;
   eina_strbuf_append(ls->buff, bufs);
   free(bufs);
}

static void
write_val_with_decpoint(Eo_Lexer *ls, Eo_Token *tok, int type)
{
   struct lconv *lc = localeconv();
   char prev = ls->decpoint;
   ls->decpoint = lc ? lc->decimal_point[0] : '.';
   if (ls->decpoint == prev)
     {
        eo_lexer_lex_error(ls, "malformed number", TOK_NUMBER);
        return;
     }
   replace_decpoint(ls, prev);
   char *end = NULL;
   if (type == NUM_FLOAT)
     tok->value.f = strtof(eina_strbuf_string_get(ls->buff), &end);
   else if (type == NUM_DOUBLE)
     tok->value.d = strtod(eina_strbuf_string_get(ls->buff), &end);
   if (end && end[0])
     eo_lexer_lex_error(ls, "malformed number", TOK_NUMBER);
   tok->kw = type;
}

static void
write_val(Eo_Lexer *ls, Eo_Token *tok, Eina_Bool is_float)
{
   int type = get_type(ls, is_float);
   char *end = NULL;
   if (is_float)
     {
        replace_decpoint(ls, '.');
        if (type == NUM_FLOAT)
          tok->value.f = strtof(eina_strbuf_string_get(ls->buff), &end);
        else if (type == NUM_DOUBLE)
          tok->value.d = strtod(eina_strbuf_string_get(ls->buff), &end);
     }
   else
     {
        const char *str = eina_strbuf_string_get(ls->buff);
        /* signed is always in the same memory location */
        if (type == NUM_INT || type == NUM_UINT)
          tok->value.u = strtoul(str, &end, 0);
        else if (type == NUM_LONG || type == NUM_ULONG)
          tok->value.ul = strtoul(str, &end, 0);
        else if (type == NUM_LLONG || type == NUM_ULLONG)
          tok->value.ull = strtoull(str, &end, 0);
     }
   if (end && end[0])
     {
        if (is_float)
          {
             write_val_with_decpoint(ls, tok, type);
             return;
          }
        eo_lexer_lex_error(ls, "malformed number", TOK_NUMBER);
     }
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
           next_char(ls);
           if (ls->current == '*')
             {
                int ccol = ls->column;
                next_char(ls);
                if (ls->current == '@')
                  {
                     eo_lexer_lex_error(ls, "old style documentation comment", -1);
                     return -1; /* unreachable */
                  }
                read_long_comment(ls, ccol);
                continue;
             }
           else if (ls->current != '/') return '/';
           next_char(ls);
           while (ls->current && !is_newline(ls->current))
             next_char(ls);
           continue;
        }
      case '[':
        {
           int dline = ls->line_number, dcol = ls->column;
           const char *sline = ls->stream_line;
           next_char(ls);
           if (ls->current != '[') return '[';
           next_char(ls);
           read_doc(ls, tok, dline, dcol);
           ls->column = dcol + 1;
           /* doc is the only potentially multiline token */
           ls->line_number = dline;
           ls->stream_line = sline;
           return TOK_DOC;
        }
      case '\0':
        return -1;
      case '=':
        next_char(ls);
        if (!ls->expr_mode || (ls->current != '=')) return '=';
        next_char(ls);
        --ls->column;
        return TOK_EQ;
      case '!':
        next_char(ls);
        if (!ls->expr_mode || (ls->current != '=')) return '!';
        next_char(ls);
        --ls->column;
        return TOK_NQ;
      case '>':
        next_char(ls);
        if (!ls->expr_mode) return '>';
        if (ls->current == '=')
          {
             next_char(ls);
             --ls->column;
             return TOK_GE;
          }
        else if (ls->current == '>')
          {
             next_char(ls);
             --ls->column;
             return TOK_RSH;
          }
        return '>';
      case '<':
        next_char(ls);
        if (!ls->expr_mode) return '<';
        if (ls->current == '=')
          {
             next_char(ls);
             --ls->column;
             return TOK_LE;
          }
        else if (ls->current == '<')
          {
             next_char(ls);
             --ls->column;
             return TOK_LSH;
          }
        return '<';
      case '&':
        next_char(ls);
        if (!ls->expr_mode || (ls->current != '&')) return '&';
        next_char(ls);
        --ls->column;
        return TOK_AND;
      case '|':
        next_char(ls);
        if (!ls->expr_mode || (ls->current != '|')) return '|';
        next_char(ls);
        --ls->column;
        return TOK_OR;
      case '"':
        {
           int dcol = ls->column;
           if (!ls->expr_mode)
             {
                next_char(ls);
                return '"';
             }
           /* strings are not multiline for now at least */
           read_string(ls, tok);
           ls->column = dcol + 1;
           return TOK_STRING;
        }
      case '\'':
        {
           int dcol = ls->column;
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
           ls->column = dcol + 1;
           return TOK_CHAR;
        }
      case '.':
        {
           int dcol = ls->column;
           next_char(ls);
           if (!isdigit(ls->current)) return '.';
           eina_strbuf_reset(ls->buff);
           eina_strbuf_append_char(ls->buff, '.');
           read_number(ls, tok);
           ls->column = dcol + 1;
           return TOK_NUMBER;
        }
      default:
        {
           if (isspace(ls->current))
             {
                assert(!is_newline(ls->current));
                next_char(ls);
                continue;
             }
           else if (isdigit(ls->current))
             {
                int col = ls->column;
                eina_strbuf_reset(ls->buff);
                read_number(ls, tok);
                ls->column = col + 1;
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
                tok->value.s = eina_stringshare_add(str);
                if (at_kw && tok->kw == 0)
                  eo_lexer_syntax_error(ls, "invalid keyword");
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
   const char *fslash = strrchr(ls->source, '/');
   const char *bslash = strrchr(ls->source, '\\');
   if (fslash || bslash)
     return eina_stringshare_add((fslash > bslash) ? (fslash + 1) : (bslash + 1));
   return eina_stringshare_ref(ls->source);
}

static void
_node_free(Eolian_Object *obj)
{
#if 0
   /* for when we have a proper node allocator and collect on shutdown */
   if (obj->refcount > 1)
     {
        eolian_state_log(obj->state, "node %p (type %d, name %s at %s:%d:%d)"
                         " dangling ref (count: %d)", obj, obj->type, obj->name,
                         obj->file, obj->line, obj->column, obj->refcount);
     }
#endif
   switch (obj->type)
     {
      case EOLIAN_OBJECT_CLASS:
        database_class_del((Eolian_Class *)obj);
        break;
      case EOLIAN_OBJECT_TYPEDECL:
        database_typedecl_del((Eolian_Typedecl *)obj);
        break;
      case EOLIAN_OBJECT_TYPE:
        database_type_del((Eolian_Type *)obj);
        break;
      case EOLIAN_OBJECT_VARIABLE:
        database_var_del((Eolian_Variable *)obj);
        break;
      case EOLIAN_OBJECT_EXPRESSION:
        database_expr_del((Eolian_Expression *)obj);
        break;
      default:
        /* normally unreachable, just for debug */
        assert(0);
        break;
     }
}

static void
eo_lexer_set_input(Eo_Lexer *ls, Eolian_State *state, const char *source)
{
   Eina_File *f = eina_file_open(source, EINA_FALSE);
   if (!f)
     {
        eolian_state_log(state, "%s", strerror(errno));
        longjmp(ls->err_jmp, EO_LEXER_ERROR_NORMAL);
     }
   ls->lookahead.token = -1;
   ls->state           = state;
   ls->buff            = eina_strbuf_new();
   ls->handle          = f;
   ls->stream          = eina_file_map_all(f, EINA_FILE_RANDOM);
   ls->stream_end      = ls->stream + eina_file_size_get(f);
   ls->stream_line     = ls->stream;
   ls->source          = eina_stringshare_add(source);
   ls->filename        = get_filename(ls);
   ls->iline_number    = ls->line_number = 1;
   ls->icolumn         = ls->column = -1;
   ls->decpoint        = '.';
   ls->nodes           = eina_hash_pointer_new(EINA_FREE_CB(_node_free));
   next_char(ls);

   Eolian_Unit *ncunit = calloc(1, sizeof(Eolian_Unit));
   if (!ncunit)
     {
        eo_lexer_free(ls);
        eolian_state_panic(state, "out of memory");
     }
   ls->unit = ncunit;
   database_unit_init(state, ncunit, ls->filename);
   eina_hash_add(state->staging.units, ls->filename, ncunit);

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

Eolian_Object *
eo_lexer_node_new(Eo_Lexer *ls, size_t objsize)
{
   Eolian_Object *obj = calloc(1, objsize);
   if (!obj)
     longjmp(ls->err_jmp, EO_LEXER_ERROR_OOM);
   eina_hash_add(ls->nodes, &obj, obj);
   eolian_object_ref(obj);
   return obj;
}

Eolian_Object *
eo_lexer_node_release(Eo_Lexer *ls, Eolian_Object *obj)
{
   /* just for debug */
   assert(eina_hash_find(ls->nodes, &obj) && (obj->refcount >= 1));
   eolian_object_unref(obj);
   eina_hash_set(ls->nodes, &obj, NULL);
   return obj;
}

static void
_free_tok(Eo_Token *tok)
{
   if (tok->token < START_CUSTOM || tok->token == TOK_NUMBER ||
                                    tok->token == TOK_CHAR)
     return;
   if (tok->token == TOK_DOC)
     {
        /* free doc */
        if (!tok->value.doc) return;
        eina_stringshare_del(tok->value.doc->summary);
        eina_stringshare_del(tok->value.doc->description);
        free(tok->value.doc);
        tok->value.doc = NULL;
        return;
     }
   eina_stringshare_del(tok->value.s);
   tok->value.s = NULL;
}

void
eo_lexer_dtor_push(Eo_Lexer *ls, Eina_Free_Cb free_cb, void *data)
{
   Eo_Lexer_Dtor *dt = malloc(sizeof(Eo_Lexer_Dtor));
   if (!dt)
     {
        free_cb(data);
        longjmp(ls->err_jmp, EO_LEXER_ERROR_OOM);
     }
   dt->free_cb = free_cb;
   dt->data = data;
   ls->dtors = eina_list_prepend(ls->dtors, dt);
}

void
eo_lexer_dtor_pop(Eo_Lexer *ls)
{
   Eo_Lexer_Dtor *dt = eina_list_data_get(ls->dtors);
   ls->dtors = eina_list_remove_list(ls->dtors, ls->dtors);
   dt->free_cb(dt->data);
   free(dt);
}

void
eo_lexer_free(Eo_Lexer *ls)
{
   if (!ls) return;
   if (ls->source  ) eina_stringshare_del(ls->source);
   if (ls->filename) eina_stringshare_del(ls->filename);
   if (ls->buff    ) eina_strbuf_free    (ls->buff);
   if (ls->handle  ) eina_file_close     (ls->handle);

   _free_tok(&ls->t);
   eo_lexer_context_clear(ls);

   Eo_Lexer_Dtor *dtor;
   EINA_LIST_FREE(ls->dtors, dtor)
     dtor->free_cb(dtor->data);

   eina_hash_free(ls->nodes);

   free(ls);
}

Eo_Lexer *
eo_lexer_new(Eolian_State *state, const char *source)
{
   volatile Eo_Lexer *ls = calloc(1, sizeof(Eo_Lexer));
   if (!ls)
     eolian_state_panic(state, "out of memory");

   if (!setjmp(((Eo_Lexer *)(ls))->err_jmp))
     {
        eo_lexer_set_input((Eo_Lexer *) ls, state, source);
        return (Eo_Lexer *) ls;
     }
   eo_lexer_free((Eo_Lexer *) ls);
   return NULL;
}

int
eo_lexer_get(Eo_Lexer *ls)
{
   _free_tok(&ls->t);
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
        throw(ls, "%s near '%s'", msg, buf);
     }
   else
     throw(ls, "%s", msg);
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
   else if (token < START_CUSTOM)
     {
        assert((unsigned char)token == token);
        if (iscntrl(token))
          sprintf(buf, "char(%d)", token);
        else
          sprintf(buf, "%c", token);
     }
   else
     {
        const char *v;
        size_t idx = token - START_CUSTOM;
        size_t tsz = sizeof(tokens) / sizeof(tokens[0]);
        if (idx >= tsz)
          v = keywords[idx - tsz];
        else
          v = tokens[idx];
        memcpy(buf, v, strlen(v) + 1);
     }
}

const char *
eo_lexer_keyword_str_get(int kw)
{
   return keywords[kw - 1];
}

Eina_Bool
eo_lexer_is_type_keyword(int kw)
{
   return (kw >= KW_byte && kw < KW_true);
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

static Eina_Bool
_eo_is_tokstr(int t) {
    return (t == TOK_STRING) || (t == TOK_VALUE);
}

void
eo_lexer_context_push(Eo_Lexer *ls)
{
   Lexer_Ctx *ctx = malloc(sizeof(Lexer_Ctx));
   if (!ctx)
     longjmp(ls->err_jmp, EO_LEXER_ERROR_OOM);
   ctx->line = ls->line_number;
   ctx->column = ls->column;
   ctx->linestr = ls->stream_line;
   ctx->token = ls->t;
   if (_eo_is_tokstr(ctx->token.token))
     eina_stringshare_ref(ctx->token.value.s);
   ls->saved_ctxs = eina_list_prepend(ls->saved_ctxs, ctx);
}

void
eo_lexer_context_pop(Eo_Lexer *ls)
{
   Lexer_Ctx *ctx = (Lexer_Ctx*)eina_list_data_get(ls->saved_ctxs);
   if (_eo_is_tokstr(ctx->token.token))
     eina_stringshare_del(ctx->token.value.s);
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
   if (_eo_is_tokstr(ls->t.token))
     eina_stringshare_del(ls->t.value.s);
   ls->t = ctx->token;
   if (_eo_is_tokstr(ls->t.token))
     eina_stringshare_ref(ls->t.value.s);
}

void
eo_lexer_context_clear(Eo_Lexer *ls)
{
   Lexer_Ctx *ctx;
   EINA_LIST_FREE(ls->saved_ctxs, ctx) free(ctx);
}

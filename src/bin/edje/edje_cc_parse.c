#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "edje_cc.h"
#include <Ecore.h>
#include <Ecore_File.h>

#ifdef _WIN32
# define EPP_EXT ".exe"
#else
# define EPP_EXT
#endif

#define SKIP_NAMESPACE_VALIDATION_SUPPORTED " -DSKIP_NAMESPACE_VALIDATION=1 "

#define EDJE_1_18_SUPPORTED " -DEFL_VERSION_1_18=1 "
#define EDJE_1_19_SUPPORTED " -DEFL_VERSION_1_19=1 "
#define EDJE_1_20_SUPPORTED " -DEFL_VERSION_1_20=1 "
#define EDJE_1_21_SUPPORTED " -DEFL_VERSION_1_21=1 "

#define EDJE_CC_EFL_VERSION_SUPPORTED \
  EDJE_1_18_SUPPORTED                 \
  EDJE_1_19_SUPPORTED                 \
  EDJE_1_20_SUPPORTED                 \
  EDJE_1_21_SUPPORTED

static void        new_object(void);
static void        new_statement(void);
static char       *perform_math(char *input);
static int         isdelim(char c);
static char       *next_token(char *p, char *end, char **new_p, int *delim);
static const char *stack_id(void);
static void        parse(char *data, off_t size);

/* simple expression parsing protos */
static int         my_atoi(const char *s);
static char       *_alphai(char *s, int *val);
static char       *_betai(char *s, int *val);
static char       *_gammai(char *s, int *val);
static char       *_deltai(char *s, int *val);
static char       *_get_numi(char *s, int *val);
static int         _is_numi(char c);
static int         _is_op1i(char c);
static int         _is_op2i(char c);
static int         _calci(char op, int a, int b);

static double      my_atof(const char *s);
static char       *_alphaf(char *s, double *val);
static char       *_betaf(char *s, double *val);
static char       *_gammaf(char *s, double *val);
static char       *_deltaf(char *s, double *val);
static char       *_get_numf(char *s, double *val);
static int         _is_numf(char c);
static int         _is_op1f(char c);
static int         _is_op2f(char c);
static double      _calcf(char op, double a, double b);
static int         strstrip(const char *in, char *out, size_t size);

int line = 0;
Eina_List *stack = NULL;
Eina_Array params;
int had_quote = 0;
int params_quote = 0;

static char file_buf[4096];
static int did_wildcard = 0;
static int verbatim = 0;
static int verbatim_line1 = 0;
static int verbatim_line2 = 0;
static char *verbatim_str = NULL;
static Eina_Strbuf *stack_buf = NULL;

static void
err_show_stack(void)
{
   const char *s;

   s = stack_id();
   if (s)
     ERR("PARSE STACK:\n%s", s);
   else
     ERR("NO PARSE STACK");
}

static void
err_show_params(void)
{
   Eina_Array_Iterator iterator;
   unsigned int i;
   char *p;

   ERR("PARAMS:");
   EINA_ARRAY_ITER_NEXT(&params, i, p, iterator)
   {
      ERR("  %s", p);
   }
}

static void
err_show(void)
{
   err_show_stack();
   err_show_params();
}

static char *
_parse_param_get(int n)
{
   if (n < (int)eina_array_count(&params))
     return eina_array_data_get(&params, n);
   return NULL;
}

static Eina_Hash *_new_object_hash = NULL;
static Eina_Hash *_new_object_short_hash = NULL;
static Eina_Hash *_new_statement_hash = NULL;
static Eina_Hash *_new_statement_short_hash = NULL;
static Eina_Hash *_new_statement_short_single_hash = NULL;
static Eina_Hash *_new_nested_hash = NULL;
static Eina_Hash *_new_nested_short_hash = NULL;
static void
fill_object_statement_hashes(void)
{
   int i, n;

   if (_new_object_hash) return;

   _new_object_hash = eina_hash_string_superfast_new(NULL);
   _new_object_short_hash = eina_hash_string_superfast_new(NULL);
   _new_statement_hash = eina_hash_string_superfast_new(NULL);
   _new_statement_short_hash = eina_hash_string_superfast_new(NULL);
   _new_statement_short_single_hash = eina_hash_string_superfast_new(NULL);
   _new_nested_hash = eina_hash_string_superfast_new(NULL);
   _new_nested_short_hash = eina_hash_string_superfast_new(NULL);

   n = object_handler_num();
   for (i = 0; i < n; i++)
     {
        eina_hash_direct_add(_new_object_hash, object_handlers[i].type,
                             &(object_handlers[i]));
     }
   n = object_handler_short_num();
   for (i = 0; i < n; i++)
     {
        eina_hash_direct_add(_new_object_short_hash, object_handlers_short[i].type,
                             &(object_handlers_short[i]));
     }
   n = statement_handler_num();
   for (i = 0; i < n; i++)
     {
        eina_hash_direct_add(_new_statement_hash, statement_handlers[i].type,
                             &(statement_handlers[i]));
     }
   n = statement_handler_short_num();
   for (i = 0; i < n; i++)
     {
        eina_hash_direct_add(_new_statement_short_hash, statement_handlers_short[i].type,
                             &(statement_handlers_short[i]));
     }
   n = statement_handler_short_single_num();
   for (i = 0; i < n; i++)
     {
        eina_hash_direct_add(_new_statement_short_single_hash, statement_handlers_short_single[i].type,
                             &(statement_handlers_short_single[i]));
     }
   n = nested_handler_num();
   for (i = 0; i < n; i++)
     {
        eina_hash_direct_add(_new_nested_hash, nested_handlers[i].type,
                             &(nested_handlers[i]));
     }
   n = nested_handler_short_num();
   for (i = 0; i < n; i++)
     {
        eina_hash_direct_add(_new_nested_short_hash, nested_handlers_short[i].type,
                             &(nested_handlers_short[i]));
     }
}

static char *
stack_dup_wildcard(void)
{
   char buf[PATH_MAX] = { 0, };
   char *end;

   strncpy(buf, stack_id(), sizeof(buf) - 1);

   end = strrchr(buf, '.');
   if (end) end++;
   else end = buf;

   strcpy(end, "*");

   return eina_strdup(buf);
}

static void
new_object(void)
{
   const char *id;
   New_Object_Handler *oh = NULL;
   New_Statement_Handler *sh;

   fill_object_statement_hashes();
   id = stack_id();
   if (!had_quote)
     {
        oh = eina_hash_find(_new_object_hash, id);
        if (!oh)
          oh = eina_hash_find(_new_object_short_hash, id);
     }
   if (oh)
     {
        if (oh->func) oh->func();
     }
   else
     {
        did_wildcard = edje_cc_handlers_wildcard();
        if (!did_wildcard)
          {
             sh = eina_hash_find(_new_statement_hash, id);
             if (!sh)
               sh = eina_hash_find(_new_statement_short_hash, id);
             if (!sh)
               sh = eina_hash_find(_new_statement_short_single_hash, id);
             if (!sh)
               {
                  char *tmp = stack_dup_wildcard();
                  sh = eina_hash_find(_new_statement_hash, tmp);
                  free(tmp);
               }
             if ((!sh) && (!did_wildcard) && (!had_quote))
               {
                  ERR("%s:%i unhandled keyword %s",
                      file_in, line - 1,
                      (char *)eina_list_data_get(eina_list_last(stack)));
                  err_show();
                  exit(-1);
               }
             did_wildcard = !sh;
          }
     }
}

static void
new_statement(void)
{
   const char *id;
   New_Statement_Handler *sh = NULL;
   fill_object_statement_hashes();
   id = stack_id();
   sh = eina_hash_find(_new_statement_hash, id);
   if (!sh)
     sh = eina_hash_find(_new_statement_short_hash, id);
   if (sh)
     {
        if (sh->func) sh->func();
     }
   else
     {
        char *tmp = stack_dup_wildcard();
        sh = eina_hash_find(_new_statement_hash, tmp);
        free(tmp);

        if (sh)
          {
             if (sh->func) sh->func();
          }
        else
          {
             ERR("%s:%i unhandled keyword %s",
                 file_in, line - 1,
                 (char *)eina_list_data_get(eina_list_last(stack)));
             err_show();
             exit(-1);
          }
     }
}

static Eina_Bool
new_statement_single(void)
{
   const char *id;
   New_Statement_Handler *sh = NULL;
   fill_object_statement_hashes();
   id = stack_id();
   sh = eina_hash_find(_new_statement_short_single_hash, id);
   if (sh)
     {
        if (sh->func) sh->func();
     }
   return !!sh;
}

static char *
perform_math(char *input)
{
   char buf[256];
   double res;

   /* FIXME
    * Always apply floating-point arithmetic.
    * Does this cause problems for integer parameters? (yes it will)
    *
    * What we should do is, loop over the string and figure out whether
    * there are floating point operands, too and then switch to
    * floating point math.
    */
   res = my_atof(input);
   snprintf(buf, sizeof (buf), "%lf", res);
   return strdup(buf);
}

static int
isdelim(char c)
{
   const char *delims = "{},;:[]";
   char *d;

   d = (char *)delims;
   while (*d)
     {
        if (c == *d) return 1;
        d++;
     }
   return 0;
}

static char *
next_token(char *p, char *end, char **new_p, int *delim)
{
   char *tok_start = NULL, *tok_end = NULL, *tok = NULL, *sa_start = NULL;
   int in_tok = 0;
   int in_quote = 0;
   int in_parens = 0;
   int in_comment_ss = 0;
   int in_comment_cpp = 0;
   int in_comment_sa = 0;
   int is_escaped = 0;

   had_quote = 0;

   *delim = 0;
   if (p >= end) return NULL;
   while (p < end)
     {
        if (*p == '\n')
          {
             in_comment_ss = 0;
             in_comment_cpp = 0;
             line++;
          }
        if ((!in_comment_ss) && (!in_comment_sa))
          {
             if ((!in_quote) && (*p == '/') && (p < (end - 1)) && (*(p + 1) == '/'))
               in_comment_ss = 1;
             if ((!in_quote) && (*p == '#'))
               in_comment_cpp = 1;
             if ((!in_quote) && (*p == '/') && (p < (end - 1)) && (*(p + 1) == '*'))
               {
                  in_comment_sa = 1;
                  sa_start = p;
               }
          }
        if ((in_comment_cpp) && (*p == '#'))
          {
             char *pp, fl[4096];
             char *tmpstr = NULL;
             int l, nm;

             /* handle cpp comments */
             /* their line format is
              * #line <line no. of next line> <filename from next line on> [??]
              */

             pp = p;
             while ((pp < end) && (*pp != '\n'))
               {
                  pp++;
               }
             l = pp - p;
             tmpstr = alloca(l + 1);
             memcpy(tmpstr, p, l);
             tmpstr[l] = 0;
             if (l >= (int)sizeof(fl))
               {
                  ERR("Line too long: %i chars: %s", l, tmpstr);
                  err_show();
                  exit(-1);
               }
             l = sscanf(tmpstr, "%*s %i \"%[^\"]\"", &nm, fl);
             if (l == 2)
               {
                  strcpy(file_buf, fl);
                  line = nm;
                  file_in = file_buf;
               }
          }
        else if ((!in_comment_ss) && (!in_comment_sa) && (!in_comment_cpp))
          {
             if (!in_tok)
               {
                  if (!in_quote)
                    {
                       if (!isspace(*p))
                         {
                            if (*p == '"')
                              {
                                 in_quote = 1;
                                 had_quote = 1;
                              }
                            else if (*p == '(')
                              in_parens++;

                            in_tok = 1;
                            tok_start = p;
                            if (isdelim(*p)) *delim = 1;
                         }
                    }
               }
             else
               {
                  if (in_quote)
                    {
                       if ((*p) == '\\')
                         is_escaped = !is_escaped;
                       else if (((*p) == '"') && (!is_escaped))
                         {
                            in_quote = 0;
                            had_quote = 1;
                         }
                       else if (is_escaped)
                         is_escaped = 0;
                    }
                  else if (in_parens != 0 && (!is_escaped))
                    {
                       if (*p == '(')
                         in_parens++;
                       else if (*p == ')')
                         in_parens--;
                       else if (isdelim(*p))
                         {
                            ERR("check pair of parens %s:%i.", file_in, line - 1);
                            err_show();
                            exit(-1);
                         }
                    }
                  else
                    {
                       if (*p == '"')
                         {
                            in_quote = 1;
                            had_quote = 1;
                         }
                       else if (*p == '(')
                         in_parens++;
                       else if (*p == ')')
                         in_parens--;

                       /* check for end-of-token */
                       if (
                         (isspace(*p)) ||
                         ((*delim) && (!isdelim(*p))) ||
                         (isdelim(*p))
                         ) /*the line below this is never  used because it skips to
                            * the 'done' label which is after the return call for
                            * in_tok being 0. is this intentional?
                            */
                         {
                            in_tok = 0;

                            tok_end = p - 1;
                            if (*p == '\n') line--;
                            goto done;
                         }
                    }
               }
          }
        if (in_comment_sa)
          {
             if ((*p == '/') && (*(p - 1) == '*') && ((p - sa_start) > 2))
               in_comment_sa = 0;
          }
        p++;
     }
   if (!in_tok) return NULL;
   tok_end = p - 1;

done:
   *new_p = p;

   tok = mem_alloc(tok_end - tok_start + 2);
   if (!tok) return NULL;

   strncpy(tok, tok_start, tok_end - tok_start + 1);
   tok[tok_end - tok_start + 1] = 0;

   if (had_quote)
     {
        is_escaped = 0;
        p = tok;

        /* Note: if you change special chars list here make the same changes in
         * _edje_generate_source_of_style function
         */
        while (*p)
          {
             if ((*p == '\"') && (!is_escaped))
               {
                  memmove(p, p + 1, strlen(p));
               }
             else if ((*p == '\\') && (*(p + 1) == 'n'))
               {
                  memmove(p, p + 1, strlen(p));
                  *p = '\n';
               }
             else if ((*p == '\\') && (*(p + 1) == 't'))
               {
                  memmove(p, p + 1, strlen(p));
                  *p = '\t';
               }
             else if (*p == '\\')
               {
                  memmove(p, p + 1, strlen(p));
                  if (*p == '\\') p++;
                  else is_escaped = 1;
               }
             else
               {
                  if (is_escaped) is_escaped = 0;
                  p++;
               }
          }
     }
   else if (*tok == '(')
     {
        char *tmp;
        tmp = tok;
        tok = perform_math(tok);
        free(tmp);
     }

   return tok;
}

static void
stack_push(char *token)
{
   New_Nested_Handler *nested;
   Eina_Bool do_append = EINA_TRUE;

   if (eina_list_count(stack) > 1)
     {
        if (!strcmp(token, eina_list_data_get(eina_list_last(stack))))
          {
             char *tmp;
             int token_length;

             token_length = strlen(token);
             tmp = alloca(eina_strbuf_length_get(stack_buf));
             memcpy(tmp,
                    eina_strbuf_string_get(stack_buf),
                    eina_strbuf_length_get(stack_buf) - token_length - 1);
             tmp[eina_strbuf_length_get(stack_buf) - token_length - 1] = '\0';

             nested = eina_hash_find(_new_nested_hash, tmp);
             if (!nested)
               nested = eina_hash_find(_new_nested_short_hash, tmp);
             if (nested)
               {
                  if (!strcmp(token, nested->token) &&
                      stack && !strcmp(eina_list_data_get(eina_list_last(stack)), nested->token))
                    {
                       /* Do not append the nested token in buffer */
                       do_append = EINA_FALSE;
                       if (nested->func_push) nested->func_push();
                    }
               }
          }
     }
   if (do_append)
     {
        if (stack) eina_strbuf_append(stack_buf, ".");
        eina_strbuf_append(stack_buf, token);
     }
   stack = eina_list_append(stack, token);
}

static void
stack_pop(void)
{
   char *tmp;
   int tmp_length;
   Eina_Bool do_remove = EINA_TRUE;

   if (!stack)
     {
        ERR("parse error %s:%i. } marker without matching { marker",
            file_in, line - 1);
        err_show();
        exit(-1);
     }
   tmp = eina_list_data_get(eina_list_last(stack));
   tmp_length = strlen(tmp);

   stack = eina_list_remove_list(stack, eina_list_last(stack));
   if (eina_list_count(stack) > 0)
     {
        const char *prev;
        New_Nested_Handler *nested;
        char *hierarchy;
        char *lookup;

        hierarchy = alloca(eina_strbuf_length_get(stack_buf) + 1);
        memcpy(hierarchy,
               eina_strbuf_string_get(stack_buf),
               eina_strbuf_length_get(stack_buf) + 1);

        /* This is nasty, but it's the way to get parts.part when they are collapsed together. still not perfect */
        lookup = strrchr(hierarchy + eina_strbuf_length_get(stack_buf) - tmp_length, '.');
        while (lookup)
          {
             hierarchy[lookup - hierarchy] = '\0';
             nested = eina_hash_find(_new_nested_hash, hierarchy);
             if (!nested)
               nested = eina_hash_find(_new_nested_short_hash, hierarchy);
             if (nested && nested->func_pop) nested->func_pop();
             lookup = strrchr(hierarchy + eina_strbuf_length_get(stack_buf) - tmp_length, '.');
          }

        hierarchy[eina_strbuf_length_get(stack_buf) - 1 - tmp_length] = '\0';

        nested = eina_hash_find(_new_nested_hash, hierarchy);
        if (!nested)
          nested = eina_hash_find(_new_nested_short_hash, hierarchy);
        if (nested)
          {
             if (nested->func_pop) nested->func_pop();

             prev = eina_list_data_get(eina_list_last(stack));
             if (!strcmp(tmp, prev))
               {
                  if (!strcmp(nested->token, tmp))
                    do_remove = EINA_FALSE;
               }
          }
        else
          edje_cc_handlers_pop_notify(tmp);

        if (do_remove)
          eina_strbuf_remove(stack_buf,
                             eina_strbuf_length_get(stack_buf) - tmp_length - 1,
                             eina_strbuf_length_get(stack_buf));  /* remove: '.tmp' */
     }
   else
     {
        eina_strbuf_remove(stack_buf,
                           eina_strbuf_length_get(stack_buf) - tmp_length,
                           eina_strbuf_length_get(stack_buf)); /* remove: 'tmp' */
     }
   free(tmp);
}

void
stack_push_quick(const char *str)
{
   char *s;

   s = mem_strdup(str);
   stack = eina_list_append(stack, s);
   eina_strbuf_append_char(stack_buf, '.');
   eina_strbuf_append(stack_buf, s);
}

char *
stack_pop_quick(Eina_Bool check_last, Eina_Bool do_free)
{
   char *tmp, *str;

   str = tmp = eina_list_last_data_get(stack);
   if (check_last)
     {
        char *end;

        end = strrchr(tmp, '.');
        if (end)
          tmp = end + 1;
     }
   eina_strbuf_remove(stack_buf,
                      eina_strbuf_length_get(stack_buf) - strlen(tmp) - 1,
                      eina_strbuf_length_get(stack_buf));      /* remove: '.tmp' */
   stack = eina_list_remove_list(stack, eina_list_last(stack));
   if (do_free)
     {
        free(str);
        str = NULL;
     }
   return str;
}

/* replace the top of stack with given token */
void
stack_replace_quick(const char *token)
{
   char *str;

   str = stack_pop_quick(EINA_FALSE, EINA_FALSE);
   if ((str) && strchr(str, '.'))
     {
        char *end, *tmp = str;
        Eina_Strbuf *buf;

        end = strchr(tmp, '.');
        if (end)
          tmp = end + 1;

        buf = eina_strbuf_new();
        eina_strbuf_append(buf, str);
        eina_strbuf_remove(buf,
                           eina_strbuf_length_get(buf) - strlen(tmp),
                           eina_strbuf_length_get(buf));
        eina_strbuf_append(buf, token);

        stack_push_quick(eina_strbuf_string_get(buf));

        eina_strbuf_free(buf);
        free(str);
     }
   else
     {
        stack_push_quick(token);
     }
}

static const char *
stack_id(void)
{
   return eina_strbuf_string_get(stack_buf);
}

static void
parse(char *data, off_t size)
{
   char *p, *end, *token;
   int delim = 0;
   int do_params = 0;
   int do_indexes = 0;  // 0: none, 1: ready, 2: done

   DBG("Parsing input file");

   /* Allocate arrays used to impl nested parts */
   edje_cc_handlers_hierarchy_alloc();
   p = data;
   end = data + size;
   line = 1;
   while ((token = next_token(p, end, &p, &delim)))
     {
        /* if we are in param mode, the only delimiter
         * we'll accept is the semicolon
         */
        if (do_params && delim && *token != ';')
          {
             ERR("parse error %s:%i. %c marker before ; marker",
                 file_in, line - 1, *token);
             err_show();
             exit(-1);
          }
        else if (delim)
          {
             if ((do_indexes == 2) && (*token != ']'))
               {
                  ERR("parse error %s:%i. %c marker before ] marker",
                      file_in, line - 1, *token);
                  err_show();
                  exit(-1);
               }
             else if (*token == ',' || *token == ':')
               do_params = 1;
             else if (*token == '}')
               {
                  if (do_params)
                    {
                       ERR("parse error %s:%i. } marker before ; marker",
                           file_in, line - 1);
                       err_show();
                       exit(-1);
                    }
                  else
                    stack_pop();
               }
             else if (*token == ';')
               {
                  if (did_wildcard)
                    {
                       free(token);
                       did_wildcard = 0;
                       continue;
                    }
                  if (do_params)
                    {
                       void *param;

                       do_params = 0;
                       new_statement();
                       /* clear out params */
                       while ((param = eina_array_pop(&params)))
                         free(param);
                       params_quote = 0;
                       /* remove top from stack */
                       stack_pop();
                    }
                  else
                    {
                       if (new_statement_single())
                         stack_pop();
                    }
               }
             else if (*token == '{')
               {
                  if (do_params)
                    {
                       ERR("parse error %s:%i. { marker before ; marker",
                           file_in, line - 1);
                       err_show();
                       exit(-1);
                    }
               }
             else if (*token == '[')
               {
                  do_indexes = 1;
               }
             else if (*token == ']')
               {
                  if (do_indexes == 2)
                    do_indexes = 0;
                  else
                    {
                       if (do_indexes == 0)
                         ERR("parse error %s:%i. ] marker before [ marker",
                             file_in, line - 1);
                       else
                         ERR("parse error %s:%i. [?] empty bracket",
                             file_in, line - 1);

                       err_show();
                       exit(-1);
                    }
               }
             free(token);
          }
        else
          {
             if (do_params)
               {
                  if (had_quote)
                    params_quote |= (1 << eina_array_count(&params));
                  eina_array_push(&params, token);
               }
             else if (do_indexes)
               {
                  if (had_quote)
                    params_quote |= (1 << eina_array_count(&params));
                  do_indexes++;
                  eina_array_push(&params, token);
               }
             else
               {
                  stack_push(token);
                  new_object();
                  if ((verbatim == 1) && (p < (end - 2)))
                    {
                       int escaped = 0;
                       int inquotes = 0;
                       int insquotes = 0;
                       int squigglie = 1;
                       int l1 = 0, l2 = 0;
                       char *verbatim_1;
                       char *verbatim_2;

                       l1 = line;
                       while ((p[0] != '{') && (p < end))
                         {
                            if (*p == '\n') line++;
                            p++;
                         }
                       p++;
                       verbatim_1 = p;
                       verbatim_2 = NULL;
                       for (; p < end; p++)
                         {
                            if (*p == '\n') line++;
                            if (escaped) escaped = 0;
                            if (!escaped)
                              {
                                 if (p[0] == '\\') escaped = 1;
                                 else if (p[0] == '\"')
                                   {
                                      if (!insquotes)
                                        {
                                           if (inquotes) inquotes = 0;
                                           else inquotes = 1;
                                        }
                                   }
                                 else if (p[0] == '\'')
                                   {
                                      if (!inquotes)
                                        {
                                           if (insquotes) insquotes = 0;
                                           else insquotes = 1;
                                        }
                                   }
                                 else if ((!inquotes) && (!insquotes))
                                   {
                                      if (p[0] == '{') squigglie++;
                                      else if (p[0] == '}')
                                        squigglie--;
                                      if (squigglie == 0)
                                        {
                                           verbatim_2 = p - 1;
                                           l2 = line;
                                           break;
                                        }
                                   }
                              }
                         }
                       if (verbatim_2 > verbatim_1)
                         {
                            int l;
                            char *v;

                            l = verbatim_2 - verbatim_1 + 1;
                            v = malloc(l + 1);
                            strncpy(v, verbatim_1, l);
                            v[l] = 0;
                            set_verbatim(v, l1, l2);
                         }
                       else
                         {
                            ERR("Parse error %s:%i. { marker does not have matching } marker",
                                file_in, line - 1);
                            err_show();
                            exit(-1);
                         }
                       new_object();
                       verbatim = 0;
                    }
               }
          }
     }

   edje_cc_handlers_hierarchy_free();
   DBG("Parsing done");
}

static char *clean_file = NULL;
static void
clean_tmp_file(void)
{
   if (clean_file)
     {
        unlink(clean_file);
        free(clean_file);
     }
}

int
is_verbatim(void)
{
   return verbatim;
}

void
track_verbatim(int on)
{
   verbatim = on;
}

void
set_verbatim(char *s, int l1, int l2)
{
   verbatim_line1 = l1;
   verbatim_line2 = l2;
   verbatim_str = s;
}

char *
get_verbatim(void)
{
   return verbatim_str;
}

int
get_verbatim_line1(void)
{
   return verbatim_line1;
}

int
get_verbatim_line2(void)
{
   return verbatim_line2;
}

void
compile(void)
{
   char buf[4096 + 4096 + 4096], buf2[4096];
   Eina_Tmpstr *tmpn;
   int fd;
   off_t size;
   char *data;
   Eina_List *l;
   Edje_Style *stl;

   fd = eina_file_mkstemp("edje_cc.edc-tmp-XXXXXX", &tmpn);
   if (fd < 0)
     {
        CRI("Unable to open temp file \"%s\" for pre-processor.", tmpn);
        exit(-1);
     }

   if (fd >= 0)
     {
        int ret;
        char *def;

        clean_file = strdup(tmpn);
        eina_tmpstr_del(tmpn);
        close(fd);
        atexit(clean_tmp_file);
        if (!defines)
          def = mem_strdup("");
        else
          {
             int len;
             char *define;

             len = 0;
             EINA_LIST_FOREACH(defines, l, define)
               len += strlen(define) + 1;
             def = mem_alloc(len + 1);
             def[0] = 0;
             EINA_LIST_FOREACH(defines, l, define)
               {
                  strcat(def, define);
                  strcat(def, " ");
               }
          }

        /*
         * Run the input through the C pre-processor.
         */

        buf2[0] = '\0';
#ifdef NEED_RUN_IN_TREE
        if (getenv("EFL_RUN_IN_TREE"))
          {
             snprintf(buf2, sizeof(buf2),
                      "%s/src/bin/edje/epp/epp" EPP_EXT,
                      PACKAGE_BUILD_DIR);
             if (!ecore_file_exists(buf2))
               buf2[0] = '\0';
          }
#endif

        if (buf2[0] == '\0')
          snprintf(buf2, sizeof(buf2),
                   "%s/edje/utils/" MODULE_ARCH "/epp" EPP_EXT,
                   eina_prefix_lib_get(pfx));
        if (ecore_file_exists(buf2))
          {
             char *inc;

             inc = ecore_file_dir_get(file_in);
             if (depfile)
               snprintf(buf, sizeof(buf), "\"%s\" "SKIP_NAMESPACE_VALIDATION_SUPPORTED" -MMD \"%s\" -MT \"%s\" \"%s\""
                                          " -I\"%s\" %s -o \"%s\""
                                          " -DEFL_VERSION_MAJOR=%d -DEFL_VERSION_MINOR=%d"
                        EDJE_CC_EFL_VERSION_SUPPORTED,
                        buf2, depfile, file_out, file_in,
                        inc ? inc : "./", def, clean_file,
                        EINA_VERSION_MAJOR, EINA_VERSION_MINOR);
             else if (annotate)
               snprintf(buf, sizeof(buf), "\"%s\" "SKIP_NAMESPACE_VALIDATION_SUPPORTED" -annotate -a \"%s\" \"%s\""
                                          " -I\"%s\" %s -o \"%s\""
                                          " -DEFL_VERSION_MAJOR=%d -DEFL_VERSION_MINOR=%d"
                        EDJE_CC_EFL_VERSION_SUPPORTED,
                        buf2, watchfile ? watchfile : "/dev/null", file_in,
                        inc ? inc : "./", def, clean_file,
                        EINA_VERSION_MAJOR, EINA_VERSION_MINOR);
             else
               snprintf(buf, sizeof(buf), "\"%s\" "SKIP_NAMESPACE_VALIDATION_SUPPORTED" -a \"%s\" \"%s\" -I\"%s\" %s"
                                          " -o \"%s\""
                                          " -DEFL_VERSION_MAJOR=%d -DEFL_VERSION_MINOR=%d"
                        EDJE_CC_EFL_VERSION_SUPPORTED,
                        buf2, watchfile ? watchfile : "/dev/null", file_in,
                        inc ? inc : "./", def, clean_file,
                        EINA_VERSION_MAJOR, EINA_VERSION_MINOR);
#ifdef _WIN32
             /* On Windows, if command begins with double quotation marks,
              * then the first and the last double quotation marks may be
              * either deleted or not. (See "help cmd" on Windows.)
              *
              * Therefore, to preserve the string between the first and the last
              * double quotation marks, "cmd /S /C" and additional outer double
              * quotation marks are added.
              */
             char win_buf[4096];
             snprintf(win_buf, sizeof(win_buf), "cmd /S /C \"%s\"", buf);
             ret = system(win_buf);
#else
             ret = system(buf);
#endif
             if (inc)
               free(inc);
          }
        else
          {
             ERR("Cannot run epp: %s", buf2);
             exit(-1);
          }
        if (ret == EXIT_SUCCESS)
          file_in = (char *)clean_file;
        else
          {
             ERR("Exit code of epp not clean: %i", ret);
             exit(-1);
          }
        free(def);
     }
   fd = open(file_in, O_RDONLY | O_BINARY, S_IRUSR | S_IWUSR);
   if (fd < 0)
     {
        ERR("Cannot open file \"%s\" for input. %s",
            file_in, strerror(errno));
        exit(-1);
     }
   DBG("Opening \"%s\" for input", file_in);

   /* lseek can return -1 on error. trap that return and exit so that
    * we do not pass malloc a -1
    *
    * NB: Fixes Coverity CID 1040029 */
   size = lseek(fd, 0, SEEK_END);
   if (size < 0)
     {
        ERR("Cannot read file \"%s\". %s", file_in, strerror(errno));
        exit(-1);
     }

   lseek(fd, 0, SEEK_SET);
   data = malloc(size);
   if (data && (read(fd, data, size) == size))
     {
        stack_buf = eina_strbuf_new();
        eina_array_step_set(&params, sizeof (Eina_Array), 8);
        parse(data, size);
        eina_array_flush(&params);
        eina_strbuf_free(stack_buf);
        stack_buf = NULL;
        color_tree_root_free();
     }
   else
     {
        ERR("Cannot read file \"%s\". %s", file_in, strerror(errno));
        exit(-1);
     }
   free(data);
   close(fd);

   EINA_LIST_FOREACH(edje_file->styles, l, stl)
     {
        if (!stl->name)
          {
             ERR("style must have a name.");
             exit(-1);
          }
     }
}

int
is_param(int n)
{
   char *str;

   str = _parse_param_get(n);
   if (str) return 1;
   return 0;
}

int
is_num(int n)
{
   char *str;
   char *end;
   long int ret;

   str = _parse_param_get(n);
   if (!str)
     {
        ERR("%s:%i no parameter supplied as argument %i",
            file_in, line - 1, n + 1);
        err_show();
        exit(-1);
     }
   if (str[0] == 0) return 0;
   end = str;
   ret = strtol(str, &end, 0);
   if ((ret == LONG_MIN) || (ret == LONG_MAX))
     {
        n = 0; // do nothing. shut gcc warnings up
     }
   if ((end != str) && (end[0] == 0)) return 1;
   return 0;
}

char *
parse_str(int n)
{
   char *str;
   char *s;

   str = _parse_param_get(n);
   if (!str)
     {
        ERR("%s:%i no parameter supplied as argument %i",
            file_in, line - 1, n + 1);
        err_show();
        exit(-1);
     }
   s = mem_strdup(str);
   return s;
}

static int
_parse_enum(char *str, va_list va)
{
   va_list va2;
   va_copy(va2, va); /* iterator for the error message */

   for (;; )
     {
        char *s;
        int v;

        s = va_arg(va, char *);

        /* End of the list, nothing matched. */
        if (!s)
          {
             ERR("%s:%i token %s not one of:", file_in, line - 1, str);
             s = va_arg(va2, char *);
             while (s)
               {
                  va_arg(va2, int);
                  fprintf(stderr, " %s", s);
                  s = va_arg(va2, char *);
                  if (!s) break;
               }
             fprintf(stderr, "\n");
             va_end(va2);
             va_end(va);
             err_show();
             exit(-1);
          }

        v = va_arg(va, int);
        if (!strcmp(s, str) || !strcmp(s, "*"))
          {
             va_end(va2);
             va_end(va);
             return v;
          }
     }
   va_end(va2);
   va_end(va);
   return 0;
}

int
parse_enum(int n, ...)
{
   char *str;
   int result;
   va_list va;

   if (n >= 0)
     {
        str = _parse_param_get(n);
        if (!str)
          {
             ERR("%s:%i no parameter supplied as argument %i",
                 file_in, line - 1, n + 1);
             err_show();
             exit(-1);
          }
     }
   else
     {
        char *end;

        str = eina_list_last_data_get(stack);
        end = strrchr(str, '.');
        if (end)
          str = end + 1;
     }

   va_start(va, n);
   result = _parse_enum(str, va);
   va_end(va);

   return result;
}

int
parse_flags(int n, ...)
{
   int result = 0;
   va_list va;

   va_start(va, n);
   while (n < (int)eina_array_count(&params))
     {
        result |= _parse_enum(eina_array_data_get(&params, n), va);
        n++;
     }
   va_end(va);

   return result;
}

int
parse_int(int n)
{
   char *str;
   int i;

   str = _parse_param_get(n);
   if (!str)
     {
        ERR("%s:%i no parameter supplied as argument %i",
            file_in, line - 1, n + 1);
        err_show();
        exit(-1);
     }
   i = my_atoi(str);
   return i;
}

int
parse_int_range(int n, int f, int t)
{
   char *str;
   int i;

   str = _parse_param_get(n);
   if (!str)
     {
        ERR("%s:%i no parameter supplied as argument %i",
            file_in, line - 1, n + 1);
        err_show();
        exit(-1);
     }
   i = my_atoi(str);
   if ((i < f) || (i > t))
     {
        ERR("%s:%i integer %i out of range of %i to %i inclusive",
            file_in, line - 1, i, f, t);
        err_show();
        exit(-1);
     }
   return i;
}

int
parse_bool(int n)
{
   char *str, buf[4096];
   int i;

   str = _parse_param_get(n);
   if (!str)
     {
        ERR("%s:%i no parameter supplied as argument %i",
            file_in, line - 1, n + 1);
        err_show();
        exit(-1);
     }

   if (!strstrip(str, buf, sizeof (buf)))
     {
        ERR("%s:%i expression is too long",
            file_in, line - 1);
        return 0;
     }

   if (!strcasecmp(buf, "false") || !strcasecmp(buf, "off"))
     return 0;
   if (!strcasecmp(buf, "true") || !strcasecmp(buf, "on"))
     return 1;

   i = my_atoi(str);
   if ((i < 0) || (i > 1))
     {
        ERR("%s:%i integer %i out of range of 0 to 1 inclusive",
            file_in, line - 1, i);
        err_show();
        exit(-1);
     }
   return i;
}

double
parse_float(int n)
{
   char *str;
   double i;

   str = _parse_param_get(n);
   if (!str)
     {
        ERR("%s:%i no parameter supplied as argument %i",
            file_in, line - 1, n + 1);
        err_show();
        exit(-1);
     }
   i = my_atof(str);
   return i;
}

double
parse_float_range(int n, double f, double t)
{
   char *str;
   double i;

   str = _parse_param_get(n);
   if (!str)
     {
        ERR("%s:%i no parameter supplied as argument %i",
            file_in, line - 1, n + 1);
        err_show();
        exit(-1);
     }
   i = my_atof(str);
   if ((i < f) || (i > t))
     {
        ERR("%s:%i float %3.3f out of range of %3.3f to %3.3f inclusive",
            file_in, line - 1, i, f, t);
        err_show();
        exit(-1);
     }
   return i;
}

int
get_arg_count(void)
{
   return eina_array_count(&params);
}

void
check_arg_count(int required_args)
{
   int num_args = eina_array_count(&params);

   if (num_args != required_args)
     {
        ERR("%s:%i got %i arguments, but expected %i",
            file_in, line - 1, num_args, required_args);
        err_show();
        exit(-1);
     }
}

void
check_min_arg_count(int min_required_args)
{
   int num_args = eina_array_count(&params);

   if (num_args < min_required_args)
     {
        ERR("%s:%i got %i arguments, but expected at least %i",
            file_in, line - 1, num_args, min_required_args);
        err_show();
        exit(-1);
     }
}

int
check_range_arg_count(int min_required_args, int max_required_args)
{
   int num_args = eina_array_count(&params);

   if (num_args < min_required_args)
     {
        ERR("%s:%i got %i arguments, but expected at least %i",
            file_in, line - 1, num_args, min_required_args);
        err_show();
        exit(-1);
     }
   else if (num_args > max_required_args)
     {
        ERR("%s:%i got %i arguments, but expected at most %i",
            file_in, line - 1, num_args, max_required_args);
        err_show();
        exit(-1);
     }

   return num_args;
}

/* simple expression parsing stuff */

/*
 * alpha ::= beta + beta || beta
 * beta  ::= gamma + gamma || gamma
 * gamma ::= num || delta
 * delta ::= '(' alpha ')'
 *
 */

/* int set of function */

static int
my_atoi(const char *s)
{
   int res = 0;
   char buf[4096];

   if (!s) return 0;
   if (!strstrip(s, buf, sizeof(buf)))
     {
        ERR("%s:%i expression is too long",
            file_in, line - 1);
        return 0;
     }
   _alphai(buf, &res);
   return res;
}

static char *
_deltai(char *s, int *val)
{
   if (!val) return NULL;
   if ('(' != s[0])
     {
        ERR("%s:%i unexpected character at %s",
            file_in, line - 1, s);
        return s;
     }
   else
     {
        s++;
        s = _alphai(s, val);
        s++;
        return s;
     }
   return s;
}

static char *
_funci(char *s, int *val)
{
   if (!strncmp(s, "floor(", 6))
     {
        s += 5;
        s = _deltai(s, val);
     }
   else if (!strncmp(s, "ceil(", 5))
     {
        s += 4;
        s = _deltai(s, val);
     }
   else
     {
        ERR("%s:%i unexpected character at %s",
            file_in, line - 1, s);
     }
   return s;
}

static char *
_gammai(char *s, int *val)
{
   if (!val) return NULL;
   if (_is_numi(s[0]))
     {
        s = _get_numi(s, val);
        return s;
     }
   else if ('(' == s[0])
     {
        s = _deltai(s, val);
        return s;
     }
   else
     {
        s = _funci(s, val);
        //        ERR("%s:%i unexpected character at %s",
        //                progname, file_in, line - 1, s);
     }
   return s;
}

static char *
_betai(char *s, int *val)
{
   int a1, a2;
   char op;

   if (!val) return NULL;
   s = _gammai(s, &a1);
   while (_is_op1i(s[0]))
     {
        op = s[0];
        s++;
        s = _gammai(s, &a2);
        a1 = _calci(op, a1, a2);
     }
   (*val) = a1;
   return s;
}

static char *
_alphai(char *s, int *val)
{
   int a1 = 0, a2 = 0;
   char op;

   if (!val) return NULL;
   s = _betai(s, &a1);
   while (_is_op2i(s[0]))
     {
        op = s[0];
        s++;
        s = _betai(s, &a2);
        a1 = _calci(op, a1, a2);
     }
   (*val) = a1;
   return s;
}

char *
_get_numi(char *s, int *val)
{
   char buf[4096];
   int pos = 0;

   if (!val) return s;
   while ((('0' <= s[pos]) && ('9' >= s[pos])) ||
          ((0 == pos) && ('-' == s[pos])))
     {
        buf[pos] = s[pos];
        pos++;
     }
   buf[pos] = '\0';
   (*val) = atoi(buf);
   return s + pos;
}

int
_is_numi(char c)
{
   if (((c >= '0') && (c <= '9')) || ('-' == c) || ('+' == c))
     return 1;
   else
     return 0;
}

int
_is_op1i(char c)
{
   switch (c)
     {
      case '*':;

      case '%':;

      case '/': return 1;

      default: break;
     }
   return 0;
}

int
_is_op2i(char c)
{
   switch (c)
     {
      case '+':;

      case '-': return 1;

      default: break;
     }
   return 0;
}

int
_calci(char op, int a, int b)
{
   switch (op)
     {
      case '+':
        a += b;
        return a;

      case '-':
        a -= b;
        return a;

      case '/':
        if (0 != b) a /= b;
        else
          ERR("%s:%i divide by zero", file_in, line - 1);
        return a;

      case '*':
        a *= b;
        return a;

      case '%':
        if (0 != b) a = a % b;
        else
          ERR("%s:%i modula by zero", file_in, line - 1);
        return a;

      default:
        ERR("%s:%i unexpected character '%c'", file_in, line - 1, op);
     }
   return a;
}

/* float set of functoins */

double
my_atof(const char *s)
{
   double res = 0;
   char buf[4096];

   if (!s) return 0;

   if (!strstrip(s, buf, sizeof (buf)))
     {
        ERR("%s:%i expression is too long", file_in, line - 1);
        return 0;
     }
   _alphaf(buf, &res);
   return res;
}

static char *
_deltaf(char *s, double *val)
{
   if (!val) return NULL;
   if ('(' != s[0])
     {
        ERR("%s:%i unexpected character at %s", file_in, line - 1, s);
        return s;
     }
   else
     {
        s++;
        s = _alphaf(s, val);
        s++;
     }
   return s;
}

static char *
_funcf(char *s, double *val)
{
   if (!strncmp(s, "floor(", 6))
     {
        s += 5;
        s = _deltaf(s, val);
        *val = floor(*val);
     }
   else if (!strncmp(s, "ceil(", 5))
     {
        s += 4;
        s = _deltaf(s, val);
        *val = ceil(*val);
     }
   else
     {
        ERR("%s:%i unexpected character at %s", file_in, line - 1, s);
     }
   return s;
}

static char *
_gammaf(char *s, double *val)
{
   if (!val) return NULL;

   if (_is_numf(s[0]))
     {
        s = _get_numf(s, val);
        return s;
     }
   else if ('(' == s[0])
     {
        s = _deltaf(s, val);
        return s;
     }
   else
     {
        s = _funcf(s, val);
        //        ERR("%s:%i unexpected character at %s",
        //                progname, file_in, line - 1, s);
     }
   return s;
}

static char *
_betaf(char *s, double *val)
{
   double a1 = 0, a2 = 0;
   char op;

   if (!val) return NULL;
   s = _gammaf(s, &a1);
   while (_is_op1f(s[0]))
     {
        op = s[0];
        s++;
        s = _gammaf(s, &a2);
        a1 = _calcf(op, a1, a2);
     }
   (*val) = a1;
   return s;
}

static char *
_alphaf(char *s, double *val)
{
   double a1 = 0, a2 = 0;
   char op;

   if (!val) return NULL;
   s = _betaf(s, &a1);
   while (_is_op2f(s[0]))
     {
        op = s[0];
        s++;
        s = _betaf(s, &a2);
        a1 = _calcf(op, a1, a2);
     }
   (*val) = a1;
   return s;
}

static char *
_get_numf(char *s, double *val)
{
   char buf[4096];
   int pos = 0;

   if (!val) return s;

   while ((('0' <= s[pos]) && ('9' >= s[pos])) ||
          ('.' == s[pos]) ||
          ((0 == pos) && ('-' == s[pos])))
     {
        buf[pos] = s[pos];
        pos++;
     }
   buf[pos] = '\0';
   (*val) = atof(buf);
   return s + pos;
}

static int
_is_numf(char c)
{
   if (((c >= '0') && (c <= '9'))
       || ('-' == c)
       || ('.' == c)
       || ('+' == c))
     return 1;
   return 0;
}

static int
_is_op1f(char c)
{
   switch (c)
     {
      case '*':;

      case '%':;

      case '/': return 1;

      default: break;
     }
   return 0;
}

static int
_is_op2f(char c)
{
   switch (c)
     {
      case '+':;

      case '-': return 1;

      default: break;
     }
   return 0;
}

static double
_calcf(char op, double a, double b)
{
   switch (op)
     {
      case '+':
        a += b;
        return a;

      case '-':
        a -= b;
        return a;

      case '/':
        if (b != 0) a /= b;
        else
          ERR("%s:%i divide by zero", file_in, line - 1);
        return a;

      case '*':
        a *= b;
        return a;

      case '%':
        if (0 != b) a = (double)((int)a % (int)b);
        else
          ERR("%s:%i modula by zero", file_in, line - 1);
        return a;

      default:
        ERR("%s:%i unexpected character '%c'", file_in, line - 1, op);
     }
   return a;
}

static int
strstrip(const char *in, char *out, size_t size)
{
   if ((size - 1) < strlen(in))
     {
        ERR("%s:%i expression is too long", file_in, line - 1);
        return 0;
     }
   /* remove spaces and tabs */
   while (*in)
     {
        if ((0x20 != *in) && (0x09 != *in))
          {
             *out = *in;
             out++;
          }
        in++;
     }
   *out = '\0';
   return 1;
}

int
get_param_index(char *str)
{
   int index;
   char *p;

   for (index = 0; index < get_arg_count(); index++)
     {
        p = _parse_param_get(index);
        if (!p) continue;

        if (!strcmp(str, p))
          return index;
     }

   return -1;
}

int
param_had_quote(int n)
{
   return params_quote & (1 << n);
}


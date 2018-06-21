#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "edje_cc.h"

#define MESSAGE_OVERRIDE

typedef struct _Code_Symbol
{
   const char *name;
   const char *tag;
   Eina_List  *args;
   char       *body;
   Eina_Bool   is_public : 1;
} Code_Symbol;

typedef enum
{
   TOKEN_TYPE_INVALID = -1,
   TOKEN_TYPE_EOF,
   TOKEN_TYPE_COLON = (1 << 0),
   TOKEN_TYPE_SEMICOLON = (1 << 1),
   TOKEN_TYPE_COMMA = (1 << 2),
   TOKEN_TYPE_PARENS = (1 << 3),
   TOKEN_TYPE_BRACES = (1 << 4),
   TOKEN_TYPE_EQUAL_MARK = (1 << 5),
   TOKEN_TYPE_PUBLIC = (1 << 6),
   TOKEN_TYPE_IDENTIFIER = (1 << 7)
} Token_Type;

typedef struct _Token
{
   char      *str;
   Token_Type type;
} Token;

static void   code_parse_internal(Code *code);
static Token *next_token(char **begin, char *end);

static void
code_parse(Code *code)
{
   Edje_Part_Collection_Parser *pcp;
   Code *base;
   Eina_List *l;
   int id;

   if (code->is_lua || code->parsed) return;

   id = eina_list_data_idx(codes, code);
   pcp = eina_list_nth(edje_collections, id);

   EINA_LIST_FOREACH(pcp->base_codes, l, base)
     code_parse(base);

   if (code->shared)
     code_parse_internal(code);

   code->parsed = EINA_TRUE;
}

static void
code_parse_internal(Code *code)
{
   Code_Symbol *sym = NULL, *func = NULL;
   Token *token, *tmp = NULL;
   char *begin = code->shared;
   char *end = begin + strlen(begin);
   char *body;
   Eina_Array *name_stack;
   Eina_Bool is_args = EINA_FALSE;
   Eina_Bool is_public = EINA_FALSE;
   int depth = 0;

   name_stack = eina_array_new(4);

   while ((token = next_token(&begin, end)))
     {
        if (token->type == TOKEN_TYPE_EOF)
          break;

        // Variables in script cannot be initialized by assignment.
        // Skip until value assignment expression ends.
        if (token->type == TOKEN_TYPE_EQUAL_MARK)
          {
             while ((tmp = next_token(&begin, end)))
               {
                  if ((tmp->type == TOKEN_TYPE_COMMA) ||
                      (tmp->type == TOKEN_TYPE_SEMICOLON))
                    {
                       if (token->str) free(token->str);
                       free(token);

                       token = tmp;
                       tmp = NULL;
                       break;
                    }

                  if (tmp->str) free(tmp->str);
                  free(tmp);
               }
          }

        switch (token->type)
          {
           case TOKEN_TYPE_COLON:
             if (!sym)
               sym = mem_alloc(SZ(Code_Symbol));
             sym->tag = eina_array_pop(name_stack);
             break;

           case TOKEN_TYPE_SEMICOLON:
             if (eina_array_count(name_stack))
               {
                  if (!sym)
                    sym = mem_alloc(SZ(Code_Symbol));
                  sym->name = eina_array_pop(name_stack);
                  sym->is_public = is_public;
                  code->vars = eina_list_append(code->vars, sym);
                  sym = NULL;
               }
             is_public = EINA_FALSE;
             break;

           case TOKEN_TYPE_COMMA:
             if (!sym)
               sym = mem_alloc(SZ(Code_Symbol));
             sym->name = eina_array_pop(name_stack);
             if (is_args)
               func->args = eina_list_append(func->args, sym);
             else
               {
                  sym->is_public = is_public;
                  code->vars = eina_list_append(code->vars, sym);
               }
             sym = NULL;
             break;

           case TOKEN_TYPE_PARENS:
             is_args = !is_args;
             if (is_args)
               {
                  if (!sym)
                    func = mem_alloc(SZ(Code_Symbol));
                  else
                    {
                       func = sym;
                       sym = NULL;
                    }
                  func->name = eina_array_pop(name_stack);
               }
             else
               {
                  if (eina_array_count(name_stack))
                    {
                       if (!sym)
                         sym = mem_alloc(SZ(Code_Symbol));
                       sym->name = eina_array_pop(name_stack);
                       func->args = eina_list_append(func->args, sym);
                    }
                  sym = func;
               }
             break;

           case TOKEN_TYPE_BRACES:
             depth = 1;
             body = begin;
             while ((tmp = next_token(&begin, end)))
               {
                  if (tmp->type == TOKEN_TYPE_BRACES)
                    {
                       switch (tmp->str[0])
                         {
                          case '{':
                            depth++;
                            break;

                          case '}':
                            depth--;
                            break;
                         }
                    }
                  if (!depth)
                    break;

                  if (tmp->str) free(tmp->str);
                  free(tmp);
               }
             if ((begin - 1) > body)
               {
                  sym->body = mem_alloc(sizeof(char) * (begin - body - 1));
                  strncpy(sym->body, body, (begin - body - 2));
               }
             sym->is_public = is_public;
             code->func = eina_list_append(code->func, sym);
             sym = NULL;
             is_public = EINA_FALSE;
             break;

           case TOKEN_TYPE_PUBLIC:
             is_public = EINA_TRUE;
             break;

           case TOKEN_TYPE_IDENTIFIER:
             eina_array_push(name_stack, token->str);
             token->str = NULL;
             break;

           default:
             break;
          }

        if (token->str)
          free(token->str);
        free(token);

        if (tmp)
          {
             if (tmp->str) free(tmp->str);
             free(tmp);
             tmp = NULL;
          }
     }

   if (token)
     {
        if (token->str)
          free(token->str);
        free(token);
     }

   eina_array_free(name_stack);
}

static Token *
next_token(char **begin, char *end)
{
   char buf[PATH_MAX] = { 0, };
   char *src;
   int index;
   Token *token;
   Eina_Bool parsed = EINA_FALSE;

   if (!begin || (*begin >= end))
     return NULL;

   token = mem_alloc(SZ(Token));
   token->type = TOKEN_TYPE_INVALID;

   src = *begin - 1;
   index = 0;

   while (++src < end)
     {
        char ch = *src;

        switch (ch)
          {
           case ' ':
           case '\t':
           case '\n':
             if (index > 0)
               parsed = EINA_TRUE;
             break;

           case ':':
           case ';':
           case ',':
           case '(':
           case ')':
           case '{':
           case '}':
           case '=':
             if (!index)
               {
                  buf[index++] = ch;
                  src++;
               }
             goto exit;

           default:
             if (parsed)
               goto exit;
             buf[index++] = ch;
             break;
          }
     }

exit:
   switch (buf[0])
     {
      case ':':
        token->type = TOKEN_TYPE_COLON;
        break;

      case ';':
        token->type = TOKEN_TYPE_SEMICOLON;
        break;

      case ',':
        token->type = TOKEN_TYPE_COMMA;
        break;

      case '(':
      case ')':
        token->type = TOKEN_TYPE_PARENS;
        break;

      case '{':
      case '}':
        token->type = TOKEN_TYPE_BRACES;
        break;

      case '=':
        token->type = TOKEN_TYPE_EQUAL_MARK;
        break;

      case '\0':
        token->type = TOKEN_TYPE_EOF;
        break;
     }

   if (token->type < 0)
     {
        if (!strcmp(buf, "public"))
          token->type = TOKEN_TYPE_PUBLIC;
        else
          token->type = TOKEN_TYPE_IDENTIFIER;
     }

   *begin = src;
   token->str = strdup(buf);
   return token;
}

static void
_push_symbol(Eina_List **total, Code_Symbol *sym, Edje_Part_Collection *pc)
{
   Eina_List *list, *l;
   Code_Symbol *sym2;

   list = *total;

   EINA_LIST_FOREACH(list, l, sym2)
     {
        if (!strcmp(sym2->name, sym->name))
          {
             WRN("Symbols in group \"%s\" have same name \"%s\". Latter defined "
                 "will shadow former one.", pc->part, sym->name);
             list = eina_list_remove(list, sym2);
             break;
          }
     }
   list = eina_list_append(list, sym);
   *total = list;
}

void
script_rewrite(Code *code)
{
   Edje_Part_Collection *pc;
   Edje_Part_Collection_Parser *pcp;
   Code *base;
   Eina_List *l, *ll;
   int id, count;
   Eina_Strbuf *buf;
   Eina_List *vars = NULL;
   Eina_List *func = NULL;
#ifdef MESSAGE_OVERRIDE
   Eina_List *message = NULL;
#endif
   Code_Symbol *sym, *arg;

   code_parse(code);

   id = eina_list_data_idx(codes, code);
   pc = eina_list_nth(edje_collections, id);
   pcp = (Edje_Part_Collection_Parser *)pc;

   EINA_LIST_FOREACH(pcp->base_codes, l, base)
     {
        EINA_LIST_FOREACH(base->vars, ll, sym)
          _push_symbol(&vars, sym, pc);

        EINA_LIST_FOREACH(base->func, ll, sym)
          {
#ifndef MESSAGE_OVERRIDE
             _push_symbol(&func, sym, pc);
#else
             if (strcmp(sym->name, "message"))
               _push_symbol(&func, sym, pc);
             else
               message = eina_list_append(message, sym);
#endif
          }
     }

   EINA_LIST_FOREACH(code->vars, l, sym)
     _push_symbol(&vars, sym, pc);
   EINA_LIST_FOREACH(code->func, l, sym)
     {
#ifndef MESSAGE_OVERRIDE
        _push_symbol(&func, sym, pc);
#else
        if (strcmp(sym->name, "message"))
          _push_symbol(&func, sym, pc);
        else
          message = eina_list_append(message, sym);
#endif
     }

   buf = eina_strbuf_new();

   if (vars)
     {
        count = 0;
        EINA_LIST_FOREACH(vars, l, sym)
          {
             if (!sym->is_public) continue;

             if (count++)
               eina_strbuf_append(buf, ", ");
             else
               eina_strbuf_append(buf, "public ");

             if (sym->tag)
               eina_strbuf_append_printf(buf, "%s:", sym->tag);
             eina_strbuf_append(buf, sym->name);
          }
        if (count)
          eina_strbuf_append(buf, ";\n");
     }

   if (func)
     {
        EINA_LIST_FOREACH(func, l, sym)
          {
             eina_strbuf_append(buf, "\n");
             if (sym->is_public)
               eina_strbuf_append(buf, "public ");

             if (sym->tag)
               eina_strbuf_append_printf(buf, "%s:", sym->tag);
             eina_strbuf_append_printf(buf, "%s(", sym->name);

             count = 0;
             EINA_LIST_FOREACH(sym->args, ll, arg)
               {
                  if (count++)
                    eina_strbuf_append(buf, ", ");

                  if (arg->tag)
                    eina_strbuf_append_printf(buf, "%s:", arg->tag);
                  eina_strbuf_append(buf, arg->name);
               }
             eina_strbuf_append(buf, ") {");
             if (sym->body)
               {
                  eina_strbuf_append(buf, sym->body);
                  eina_strbuf_rtrim(buf);
               }
             eina_strbuf_append(buf, "\n}\n");
          }
     }

#ifdef MESSAGE_OVERRIDE
   if (message)
     {
        eina_strbuf_append(buf, "\npublic message(Msg_Type:type, id, ...) {");
        EINA_LIST_FOREACH(message, l, sym)
          {
             eina_strbuf_append(buf, sym->body);
             eina_strbuf_rtrim(buf);
             eina_strbuf_append(buf, "\n");
          }
        eina_strbuf_append(buf, "}\n");
     }
#endif

   code->shared = eina_strbuf_string_steal(buf);
   code->original = strdup(code->shared);
   eina_strbuf_free(buf);

   eina_list_free(code->vars);
   eina_list_free(code->func);

   code->vars = vars;
   code->func = func;
}


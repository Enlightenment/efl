#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Eina.h>
#include "Elementary.h"

#include "elm_code_private.h"

typedef struct _Elm_Code_Syntax
{
   const char *symbols;
   const char *keywords[];
} Elm_Code_Syntax;

static Elm_Code_Syntax _elm_code_syntax_c =
{
   "{}()[]:;*&|!=<->,.",
   {"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else",  "enum", "extern", \
      "float", "for", "goto", "if", "int", "long", "register", "return", "short", "signed", "sizeof", "static", \
      "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while", NULL}
};

EAPI Elm_Code_Syntax *
elm_code_syntax_for_mime_get(const char *mime)
{
   if (!strcmp("text/x-chdr", mime) || !strcmp("text/x-csrc", mime))
     return &_elm_code_syntax_c;

   return NULL;
}

static void
_elm_code_syntax_parse_token(Elm_Code_Syntax *syntax, Elm_Code_Line *line, unsigned int pos, const char *token, unsigned int length)
{
   const char **keyword;
   unsigned int i;

  for (keyword = syntax->keywords; *keyword; keyword++)
     if (strlen(*keyword) == length && !strncmp(token, *keyword, length))
       {
          elm_code_line_token_add(line, pos, pos + length - 1, 1, ELM_CODE_TOKEN_TYPE_KEYWORD);
          return;
       }

   for (i = 0; i < length; i++)
     {
        if (!isdigit(token[i]))
          break;
        if (i == length - 1)
          elm_code_line_token_add(line, pos, pos + length - 1, 1, ELM_CODE_TOKEN_TYPE_NUMBER);
     }
}

EAPI void
elm_code_syntax_parse_line(Elm_Code_Syntax *syntax, Elm_Code_Line *line)
{
   unsigned int i, count, length;
   const char *content;
   const char *sym, *ptr;

   EINA_SAFETY_ON_NULL_RETURN(syntax);

  content = elm_code_line_text_get(line, &length);
   ptr = content;
   count = 0;
   for (i = 0; i < length; i++)
     {
        if (_elm_code_text_char_is_whitespace(content[i]))
          {
             if (count)
               _elm_code_syntax_parse_token(syntax, line, ptr-content, ptr, count);

             ptr += count+1;
             count = 0;
             continue;
          }

        if (content[i] == '#')
          {
             elm_code_line_token_add(line, i, length - 1, 1, ELM_CODE_TOKEN_TYPE_PREPROCESSOR);
             return;
          }
        else if (count == 1 && content[i-1] == '/' && content[i] == '/')
          {
             elm_code_line_token_add(line, i - 1, length - 1, 1, ELM_CODE_TOKEN_TYPE_COMMENT);
             return;
          }
        else if (content[i] == '"')
          {
             unsigned int start = i, end;

             for (i++; content[i] != '"' && i < length; i++) {}
             end = i;

             elm_code_line_token_add(line, start, end, 1, ELM_CODE_TOKEN_TYPE_STRING);
             continue;
          }
        else if (content[i] == '\'')
          {
             unsigned int start = i, end;

             for (i++; content[i] != '\'' && i < length; i++) {}
             end = i;

             elm_code_line_token_add(line, start, end, 1, ELM_CODE_TOKEN_TYPE_STRING);
             continue;
         }

        for (sym = syntax->symbols; *sym; sym++)
          if (content[i] == *sym)
            {
               if (count)
                 _elm_code_syntax_parse_token(syntax, line, ptr-content, ptr, count);

               elm_code_line_token_add(line, i, i, 1, ELM_CODE_TOKEN_TYPE_BRACE);

               ptr = content + i+1;
               count = -1;
               break;
             }

       count++;
   }

   if (count)
     _elm_code_syntax_parse_token(syntax, line, ptr-content, ptr, count);
}

EAPI void
elm_code_syntax_parse_file(Elm_Code_Syntax *syntax, Elm_Code_File *file EINA_UNUSED)
{
   EINA_SAFETY_ON_NULL_RETURN(syntax);
}


#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"

#include "elm_code_private.h"

EAPI const char *elm_code_line_content_get(Elm_Code_Line *line, int *length)
{
   if (!line)
     return NULL;

   *length = line->length;

   if (line->modified)
     return line->modified;
   return line->content;
}

EAPI void elm_code_line_status_set(Elm_Code_Line *line, Elm_Code_Status_Type status)
{
   if (!line)
     return;

   line->status = status;
}

EAPI void elm_code_line_token_add(Elm_Code_Line *line, int start, int end, Elm_Code_Token_Type type)
{
   Elm_Code_Token *tok;

   if (!line)
     return;

   tok = calloc(1, sizeof(Elm_Code_Token));

   tok->start = start;
   tok->end = end;
   tok->type = type;

   line->tokens = eina_list_append(line->tokens, tok);
}


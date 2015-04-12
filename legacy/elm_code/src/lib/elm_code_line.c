#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"

#include "elm_code_private.h"

EAPI void
elm_code_line_free(Elm_Code_Line *line)
{
   if (!line)
     return;

   if (line->status_text)
     free((char *)line->status_text);
   if (line->modified)
     free(line->modified);

   free(line);
}

EAPI void elm_code_line_status_set(Elm_Code_Line *line, Elm_Code_Status_Type status)
{
   if (!line)
     return;

   line->status = status;
}

EAPI void elm_code_line_status_text_set(Elm_Code_Line *line, const char *text)
{
   if (line->status_text)
     free(line->status_text);

   if (text)
     line->status_text = strdup(text);
   else
     line->status_text = NULL;
}

EAPI void elm_code_line_token_add(Elm_Code_Line *line, int start, int end, int lines,
                                  Elm_Code_Token_Type type)
{
   Elm_Code_Token *tok;
   unsigned int end_line;
   Elm_Code_Line *next_line;

   if (!line)
     return;

   tok = calloc(1, sizeof(Elm_Code_Token));

   end_line = line->number;
   if (lines > 1)
     end_line += lines - 1;

   tok->start = start;
   tok->end = end;
   tok->end_line = end_line;
   tok->type = type;

   line->tokens = eina_list_append(line->tokens, tok);

   if (end_line > line->number)
     {
        next_line = elm_code_file_line_get(line->file, line->number + 1);
        elm_code_line_token_add(next_line, 1, end, lines - 1, type);
     }
}

EAPI void elm_code_line_tokens_clear(Elm_Code_Line *line)
{
   Elm_Code_Token *token;

   EINA_LIST_FREE(line->tokens, token)
     free(token);
   line->tokens = NULL;
}

EAPI void elm_code_line_status_clear(Elm_Code_Line *line)
{
   line->status = ELM_CODE_STATUS_TYPE_DEFAULT;
   if (line->status_text)
     {
        free((char *)line->status_text);
        line->status_text = NULL;
     }
}


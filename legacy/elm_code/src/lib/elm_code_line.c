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

static void
_elm_code_line_tokens_split_at(Elm_Code_Line *oldline, Elm_Code_Line *newline,
                               Eina_List *tokens, int position)
{
   Eina_List *item, *next;
   Elm_Code_Token *token, *newtoken;

   EINA_LIST_FOREACH_SAFE(tokens, item, next, token)
     {
        if (!token->continues && token->end < position)
          {
             oldline->tokens = eina_list_append(oldline->tokens, token);
             continue;
          }
        if (token->start >= position)
          {
             token->start -= position;
             token->end -= position;
             newline->tokens = eina_list_append(newline->tokens, token);
             continue;
          }

        if (token->continues)
          elm_code_line_token_add(newline, 0, token->end, 1, token->type);
        else
          {
             elm_code_line_token_add(newline, 0, token->end - position, 1, token->type);
             token->end = position - 1;
          }

        newtoken = eina_list_data_get(newline->tokens);
        newtoken->continues = token->continues;
        token->continues = EINA_TRUE;
        oldline->tokens = eina_list_append(oldline->tokens, token);
     }

   elm_code_callback_fire(oldline->file->parent, &ELM_CODE_EVENT_LINE_LOAD_DONE, oldline);
   elm_code_callback_fire(newline->file->parent, &ELM_CODE_EVENT_LINE_LOAD_DONE, newline);
}

EAPI void elm_code_line_split_at(Elm_Code_Line *line, unsigned int position)
{
   Elm_Code_Line *newline;
   Elm_Code_Token *token EINA_UNUSED;
   Eina_List *tokens;
   char *content;
   unsigned int length;

   content = (char *) elm_code_line_text_get(line, &length);
   content = strndup(content, length);
   elm_code_file_line_insert(line->file, line->number + 1, "", 0, NULL);
   newline = elm_code_file_line_get(line->file, line->number + 1);

   tokens = line->tokens;
   line->tokens = NULL;
   elm_code_line_text_set(newline, content + position, length - position);
   elm_code_line_text_set(line, content, position);
   _elm_code_line_tokens_split_at(line, newline, tokens, position);

   EINA_LIST_FREE(tokens, token) {} // don't free tokens, we re-used them
   free(content);
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
   Elm_Code_Line *next_line;

   if (!line)
     return;

   tok = calloc(1, sizeof(Elm_Code_Token));

   tok->start = start;
   tok->end = end;
   tok->continues = lines > 1;
   tok->type = type;

   line->tokens = eina_list_append(line->tokens, tok);

   if (lines > 1)
     {
        next_line = elm_code_file_line_get(line->file, line->number + 1);
        elm_code_line_token_add(next_line, 0, end, lines - 1, type);
     }
}

EAPI void elm_code_line_tokens_clear(Elm_Code_Line *line)
{
   Elm_Code_Token *token;

   if (!line->tokens)
     return;

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

EAPI Eina_Bool
elm_code_line_contains_widget_cursor(Elm_Code_Line *line)
{
   Elm_Code *code = line->file->parent;
   Eina_List *item;
   Eo *widget;
   unsigned int col, number;

   if (!code)
     return EINA_FALSE;

   EINA_LIST_FOREACH(code->widgets, item, widget)
     {
        elm_code_widget_cursor_position_get(widget, &col, &number);

        if (number == line->number)
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"

#include "elm_code_private.h"

EAPI const char *
elm_code_line_text_get(Elm_Code_Line *line, unsigned int *length)
{
   if (!line)
     return NULL;

   if (length)
     *length = line->length;

   if (line->modified)
     return line->modified;
   return line->content;
}

static void
_elm_code_line_tokens_move_right(Elm_Code_Line *line, int position, int move)
{
   Eina_List *item;
   Elm_Code_Token *token;

   EINA_LIST_FOREACH(line->tokens, item, token)
     {
        if (token->end >= position)
          token->end += move;

        if (token->start > position)
          token->start += move;
     }
}

static void
_elm_code_line_tokens_move_left(Elm_Code_Line *line, int position, int move)
{
   Eina_List *item, *next;
   Elm_Code_Token *token;

   EINA_LIST_FOREACH_SAFE(line->tokens, item, next, token)
     {
        if (token->end >= position)
          token->end -= move;

        if (token->start > position)
          token->start -= move;

        if (token->end < token->start)
          line->tokens = eina_list_remove_list(line->tokens, item);
     }
}

EAPI void
elm_code_line_text_insert(Elm_Code_Line *line, unsigned int position, const char *string, int length)
{
   Elm_Code_File *file;
   char *inserted;

   if (!line)
     return;

   inserted = malloc(sizeof(char) * line->length + length + 1);
   if (position > 0)
     position--;
   if (position > line->length)
     position = line->length;

   _elm_code_line_tokens_move_right(line, position + 1, length);

   if (line->modified)
     {
        strncpy(inserted, line->modified, position);
        strncpy(inserted + position, string, length);
        strncpy(inserted + position + length, line->modified + position, line->length - position);

        free(line->modified);
     }
   else
     {
        strncpy(inserted, line->content, position);
        strncpy(inserted + position, string, length);
        strncpy(inserted + position + length, line->content + position, line->length - position);
     }

   line->modified = inserted;
   line->length += length;

// TODO update calculation
   line->unicode_length += length;

   file = line->file;
   elm_code_callback_fire(file->parent, &ELM_CODE_EVENT_LINE_LOAD_DONE, line);
}

EAPI void
elm_code_line_text_remove(Elm_Code_Line *line, unsigned int position, int length)
{
   Elm_Code_File *file;
   char *removed;

   if (!line)
     return;

   removed = malloc(sizeof(char) * line->length - length + 1);
   if (position > 0)
     position--;
   if (position > line->length)
     position = line->length;

   _elm_code_line_tokens_move_left(line, position + 1, length);

   if (line->modified)
     {
        strncpy(removed, line->modified, position);
        strncpy(removed + position, line->modified + position + length, line->length - position - length);

        free(line->modified);
     }
   else
     {
        strncpy(removed, line->content, position);
        strncpy(removed + position, line->content + position + length, line->length - position - length);
     }

   line->modified = removed;
   line->length -= length;

// TODO update calculation
   line->unicode_length -= length;

   file = line->file;
   elm_code_callback_fire(file->parent, &ELM_CODE_EVENT_LINE_LOAD_DONE, line);
}

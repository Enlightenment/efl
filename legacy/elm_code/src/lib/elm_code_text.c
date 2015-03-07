#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"

#include "elm_code_private.h"

EAPI const char *
elm_code_line_text_get(Elm_Code_Line *line, int *length)
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

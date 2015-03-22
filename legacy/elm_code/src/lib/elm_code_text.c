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

EAPI void
elm_code_line_text_set(Elm_Code_Line *line, const char *chars, unsigned int length)
{
   Elm_Code_File *file;
   char *newtext;

   if (!line)
     return;

   if (line->modified)
     free(line->modified);

   newtext = malloc(sizeof(char) * length + 1);
   strncpy(newtext, chars, length);
   line->modified = newtext;
   line->length = length;
   line->unicode_length = elm_code_text_unicode_strlen(line->modified, line->length);

   file = line->file;
   elm_code_callback_fire(file->parent, &ELM_CODE_EVENT_LINE_LOAD_DONE, line);
}

EAPI int
elm_code_line_text_strpos(Elm_Code_Line *line, const char *search, int offset)
{
   unsigned int length, searchlen, c;
   const char *content;
   char *ptr;

   searchlen = strlen(search);
   content = elm_code_line_text_get(line, &length);
   ptr = (char *) content;

   if (searchlen > length)
     return ELM_CODE_TEXT_NOT_FOUND;

   ptr += offset;
   for (c = offset; c < length - strlen(search); c++)
     {
        if (!strncmp(ptr, search, searchlen))
          return c;

        ptr++;
     }

   return ELM_CODE_TEXT_NOT_FOUND;
}

EAPI Eina_Bool
elm_code_line_text_contains(Elm_Code_Line *line, const char *search)
{
   return elm_code_line_text_strpos(line, search, 0) != ELM_CODE_TEXT_NOT_FOUND;
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
   line->unicode_length = elm_code_text_unicode_strlen(line->modified, line->length);

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
   line->unicode_length = elm_code_text_unicode_strlen(line->modified, line->length);

   file = line->file;
   elm_code_callback_fire(file->parent, &ELM_CODE_EVENT_LINE_LOAD_DONE, line);
}

/* generic text functions */

EAPI unsigned int
elm_code_text_unicode_strlen(const char *chars, unsigned int length)
{
   Eina_Unicode unicode;
   unsigned int count = 0;
   int index = 0;

   if (chars == NULL)
     return 0;

   while ((unsigned int) index < length)
     {
        unicode = eina_unicode_utf8_next_get(chars, &index);
        if (unicode == 0)
          break;

        count++;
     }

   return count;
}

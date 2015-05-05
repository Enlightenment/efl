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

   file = line->file;
   if (file->parent)
     {
        _elm_code_parse_line(file->parent, line);
        elm_code_callback_fire(file->parent, &ELM_CODE_EVENT_LINE_LOAD_DONE, line);
     }
}

EAPI int
elm_code_text_strnpos(const char *content, unsigned int length, const char *search, int offset)
{
   unsigned int searchlen, c;
   char *ptr;

   searchlen = strlen(search);
   ptr = (char *) content;

   if (searchlen > length)
     return ELM_CODE_TEXT_NOT_FOUND;

   ptr += offset;
   for (c = offset; c <= length - searchlen; c++)
     {
        if (!strncmp(ptr, search, searchlen))
          return c;

        ptr++;
     }

   return ELM_CODE_TEXT_NOT_FOUND;
}

EAPI int
elm_code_line_text_strpos(Elm_Code_Line *line, const char *search, int offset)
{
   unsigned int length;
   const char *content;

   content = elm_code_line_text_get(line, &length);
   return elm_code_text_strnpos(content, length, search, offset);
}

EAPI Eina_Bool
elm_code_line_text_contains(Elm_Code_Line *line, const char *search)
{
   return elm_code_line_text_strpos(line, search, 0) != ELM_CODE_TEXT_NOT_FOUND;
}

EAPI char *
elm_code_line_text_substr(Elm_Code_Line *line, unsigned int position, int length)
{
   const char *content;

   if (!line || length < 1)
     return strdup("");

   if (position + length > line->length)
     length = line->length - position;

   content = elm_code_line_text_get(line, NULL);
   return strndup(content + position, length);
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

   file = line->file;
   elm_code_callback_fire(file->parent, &ELM_CODE_EVENT_LINE_LOAD_DONE, line);
}

/* generic text functions */

EAPI unsigned int
elm_code_text_tabwidth_at_position(unsigned int position, unsigned int tabstop)
{
   return tabstop - (position % tabstop);
}

EAPI int
elm_code_text_newlinenpos(const char *text, unsigned int length, short *nllen)
{
   int lfpos, crpos;
   int check;

   if (nllen)
     *nllen = 1;
   lfpos = elm_code_text_strnpos(text, length, "\n", 0);
   check = length;
   if (lfpos != ELM_CODE_TEXT_NOT_FOUND)
     check = lfpos + 1;
   crpos = elm_code_text_strnpos(text, check, "\r", 0);

   if (lfpos == ELM_CODE_TEXT_NOT_FOUND && crpos == ELM_CODE_TEXT_NOT_FOUND)
     return ELM_CODE_TEXT_NOT_FOUND;

   if (crpos == ELM_CODE_TEXT_NOT_FOUND)
     return lfpos;
   if (lfpos == ELM_CODE_TEXT_NOT_FOUND)
     return crpos;

   if (nllen)
     *nllen = 2;
   if (lfpos < crpos)
     return lfpos;
   return crpos;
}

EAPI unsigned int
elm_code_line_text_column_width_to_position(Elm_Code_Line *line, unsigned int position, unsigned int tabstop)
{
   Eina_Unicode unicode;
   unsigned int count = 0;
   int index = 0;
   const char *chars;

   if (line->length == 0)
     return 0;

   if (line->modified)
     chars = line->modified;
   else
     chars = line->content;
   if (position > line->length)
     position = line->length;

   while ((unsigned int) index < position)
     {
        unicode = eina_unicode_utf8_next_get(chars, &index);
        if (unicode == 0)
          break;

        if (unicode == '\t')
          count += elm_code_text_tabwidth_at_position(count, tabstop);
        else
          count++;
     }

   return count;
}

EAPI unsigned int
elm_code_line_text_column_width(Elm_Code_Line *line, unsigned int tabstop)
{
   return elm_code_line_text_column_width_to_position(line, line->length, tabstop);
}

EAPI unsigned int
elm_code_line_text_position_for_column_get(Elm_Code_Line *line, unsigned int column, unsigned int tabstop)
{
   Eina_Unicode unicode;
   unsigned int count = 0;
   int index = 0;
   const char *chars;

   if (line->length == 0)
     return 0;

   if (line->modified)
     chars = line->modified;
   else
     chars = line->content;

   while ((unsigned int) count < column && index <= (int) line->length)
     {
        unicode = eina_unicode_utf8_next_get(chars, &index);

        if (unicode == 0)
          return line->length + 1;
        else if (unicode == '\t')
          count += elm_code_text_tabwidth_at_position(count, tabstop);
        else
          count++;
     }

   return (unsigned int) index;
}

#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include "Elm_Code.h"

#include "elm_code_private.h"

EAPI const char
*elm_code_line_text_get(Elm_Code_Line *line, int *length)
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
elm_code_line_text_insert(Elm_Code_Line *line, int position, const char *string, int length)
{
   Elm_Code_File *file;
   char *inserted;

   if (!line)
     return;

   inserted = malloc(sizeof(char) * line->length + length + 1);
   position--;
   if (position > line->length)
     position = line->length;
   if (position < 0)
     position = 0;

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
   line->length = line->length + length;

   file = line->file;
   elm_code_callback_fire(file->parent, &ELM_CODE_EVENT_LINE_LOAD_DONE, line);
}

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "Elementary.h"

#include "elm_code_private.h"

EAPI char *
elm_code_line_indent_get(Elm_Code_Line *line)
{
   Elm_Code_Line *prevline;
   const char *prevtext;
   unsigned int prevlength, count = 0;
   char *buf, *ptr;
   char next, last;

   if (line->number <= 1)
     return strdup("");

   prevline = elm_code_file_line_get(line->file, line->number - 1);
   prevtext = elm_code_line_text_get(prevline, &prevlength);

   ptr = (char *)prevtext;
   buf = malloc((prevlength + 3) * sizeof(char));
   while (count < prevlength)
     {
        if (!_elm_code_text_char_is_whitespace(*ptr))
          break;

        count++;
        ptr++;
     }

   strncpy(buf, prevtext, count);
   buf[count] = '\0';
   if (count < prevlength)
     {
        next = *ptr;
        last = prevtext[prevlength - 1];

        // comment handling
        // TODO this should all be based on comment SCOPE not text matching
        if (next == '/')
          {
             if (count == prevlength - 1)
               return buf;

             if (*(ptr+1) == '/')
               strcpy(buf + count, "//");
             else if (*(ptr+1) == '*')
               strcpy(buf + count, " * ");
          }
        else if (next == '*')
          {
             if (count < prevlength - 1 && *(ptr+1) == ' ')
               strcpy(buf + count, "* ");
             else if (count < prevlength - 1 && *(ptr+1) == '/')
               {
                  if (count >= 1)
                    buf[count-1] = '\0';
               }
             else
               strcpy(buf + count, "*");
          }
        // VERY simple handling of braces
        else if (last == '{')
          {
             strcpy(buf + count, "   ");
          }
        else if (last == '}')
          {
             if (count >= 2)
               buf[count-2] = '\0';
             else if (count >= 1)
               buf[count-1] = '\0';
          }
     }
   return buf;
}

EAPI const char *
elm_code_line_indent_matching_braces_get(Elm_Code_Line *line, unsigned int *length)
{
   Elm_Code_File *file;
   int stack, row;
   unsigned int len_tmp, count = 0;
   const char *content, *ptr;

   file = line->file;
   stack = 0;
   row = line->number - 1;
   *length = 0;

   while (row > 0)
     {
        line = elm_code_file_line_get(file, row);
        content = elm_code_line_text_get(line, &len_tmp);

        if (memchr(content, '{', len_tmp)) stack--;
        else if (memchr(content, '}', len_tmp)) stack++;

        if (stack < 0)
          {
             if (len_tmp == 0)
               return "";

             ptr = content;
             while (count < len_tmp)
               {
                  if (!_elm_code_text_char_is_whitespace(*ptr))
                    break;

                  count++;
                  ptr++;
               }

             *length = count;
             return content;
          }
        row--;
     }
   return "";
}

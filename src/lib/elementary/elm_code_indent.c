#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "regex.h"
#include "Elementary.h"

#include "elm_code_private.h"

static Eina_Bool
elm_code_line_indent_startswith_keyword(Elm_Code_Line *line)
{
   regex_t regex;
   char *text;
   int ret;
   unsigned int textlen;

   text = (char *)elm_code_line_text_get(line, &textlen);
   text = eina_strndup(text, textlen);

   ret = regcomp(&regex, "^\\s*("
                         "((if|else\\s*if|while|for|switch)\\s*\\(.*\\)\\s*\\{?)|"
                         "((else|do)\\s*\\{?)|"
                         "(case\\s+.+:)|"
                         "(default:)"
                         ")\\s*$", REG_EXTENDED | REG_NOSUB);
   if (ret == 0)
     ret = regexec(&regex, text, 0, NULL, 0);

   regfree(&regex);
   free(text);

   return ret == 0;
}

EAPI char *
elm_code_line_indent_get(Elm_Code_Line *line)
{
   Elm_Code_Line *prevline;
   const char *prevtext;
   unsigned int prevlength, count = 0;
   char *buf, *ptr;
   char next, last;
   const char *indent = "\t";
   Eina_Bool eflindent = ((Elm_Code *)line->file->parent)->config.indent_style_efl;

   if (line->number <= 1)
     return strdup("");

   prevline = elm_code_file_line_get(line->file, line->number - 1);
   prevtext = elm_code_line_text_get(prevline, &prevlength);

   ptr = (char *)prevtext;
   buf = malloc((prevlength + 5) * sizeof(char));
   while (count < prevlength)
     {
        if (!_elm_code_text_char_is_whitespace(*ptr))
          break;

        count++;
        ptr++;
     }

   strncpy(buf, prevtext, count);
   buf[count] = '\0';

   if (eflindent)
     {
        indent = "   ";
        if (elm_code_line_indent_startswith_keyword(prevline))
          {
             strcpy(buf + count, "  ");
             count += 2;
          }
     }

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
        // Simple handling of braces
        else if (last == '{' || (!eflindent && elm_code_line_indent_startswith_keyword(prevline)))
          {
             strcpy(buf + count, indent);
          }
        else if (last == '}')
          {
             unsigned int offset = strlen(indent) - 1;
             if (count >= offset)
               buf[count-offset] = '\0';
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

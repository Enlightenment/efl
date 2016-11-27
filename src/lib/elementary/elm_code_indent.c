#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "Elementary.h"

#include "elm_code_private.h"

EAPI char *
elm_code_line_indent_get(const char *prevtext, unsigned int prevlength)
{
   unsigned int count = 0;
   char *buf, *ptr = (char *)prevtext;
   char next;

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
     }
   return buf;
}


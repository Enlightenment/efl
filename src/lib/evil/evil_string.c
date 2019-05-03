#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "evil_macro.h"
#include "evil_string.h"


/*
 * string related functions
 *
 */

char *strcasestr(const char *haystack, const char *needle)
{
   size_t length_needle;
   size_t length_haystack;
   size_t i;

   if (!haystack || !needle)
     return NULL;

   length_needle = strlen(needle);
   length_haystack = strlen(haystack);
   if (length_haystack < length_needle) return NULL;
   length_haystack = length_haystack - length_needle + 1;

   for (i = 0; i < length_haystack; i++)
     {
        size_t j;

        for (j = 0; j < length_needle; j++)
          {
            unsigned char c1;
            unsigned char c2;

            c1 = haystack[i+j];
            c2 = needle[j];
            if (toupper(c1) != toupper(c2))
              goto next;
          }
        return (char *) haystack + i;
     next:
        ;
     }

   return NULL;
}

char *
strsep (char **stringp, const char *delim)
{
  char *begin, *end;

  begin = *stringp;
  if (begin == NULL)
    return NULL;

  /* A frequent case is when the delimiter string contains only one
     character.  Here we don't need to call the expensive `strpbrk'
     function and instead work using `strchr'.  */
  if (delim[0] == '\0' || delim[1] == '\0')
    {
      char ch = delim[0];

      if (ch == '\0')
        end = NULL;
      else
        {
          if (*begin == ch)
            end = begin;
          else if (*begin == '\0')
            end = NULL;
          else
            end = strchr (begin + 1, ch);
        }
    }
  else
    /* Find the end of the token.  */
    end = strpbrk (begin, delim);

  if (end)
    {
      /* Terminate the token and set *STRINGP past NUL character.  */
      *end++ = '\0';
      *stringp = end;
    }
  else
    /* No more delimiters; this is the last token.  */
    *stringp = NULL;

  return begin;
}

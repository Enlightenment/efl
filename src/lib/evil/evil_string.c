#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "Evil.h"
#include "evil_private.h"


/*
 * bit related functions
 *
 */

int ffs(int i)
{
   int size;
   int x;

   if (!i) return 1;

   /* remove the sign bit */
   x = i & -i;
   size = sizeof(int) << 3;
   for (i = size; i > 0; --i, x <<= 1)
     if (x & (1 << (size - 1))) return i;

   return x;
}

char *
strrstr (const char *str, const char *substr)
{
  char *it;
  char *ret = NULL;

  while ((it = strstr(str, substr)))
    ret = it;

  return ret;
}

#ifdef _MSC_VER

int strcasecmp(const char *s1, const char *s2)
{
   return lstrcmpi(s1, s2);
}

#endif /* _MSC_VER */

char *strcasestr(const char *haystack, const char *needle)
{
   size_t length_needle;
   size_t length_haystack;
   size_t i;

   if (!haystack || !needle)
     return NULL;

   length_needle = strlen(needle);
   length_haystack = strlen(haystack) - length_needle + 1;

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

/* Leave the OpenBSD version below so we can track upstream fixes */
/*      $OpenBSD: strlcpy.c,v 1.11 2006/05/05 15:27:38 millert Exp $        */

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <string.h>

#include "ecore_private.h"
#include "Ecore_Data.h"

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
ecore_strlcpy(char *dst, const char *src, size_t siz)
{
#ifdef HAVE_STRLCPY
   return strlcpy(dst, src, siz);
#else
   char *d = dst;
   const char *s = src;
   size_t n = siz;

   /* Copy as many bytes as will fit */
   if (n != 0)
     {
        while (--n != 0)
          {
             if ((*d++ = *s++) == '\0')
               break;
          }
     }

   /* Not enough room in dst, add NUL and traverse rest of src */
   if (n == 0)
     {
        if (siz != 0)
          *d = '\0';                /* NUL-terminate dst */
        while (*s++)
          ;
     }

   return(s - src - 1);        /* count does not include NUL */
#endif
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t
ecore_strlcat(char *dst, const char *src, size_t siz)
{
        char *d = dst;
        const char *s = src;
        size_t n = siz;
        size_t dlen;

        /* Find the end of dst and adjust bytes left but don't go past end */
        while (n-- != 0 && *d != '\0')
                d++;
        dlen = d - dst;
        n = siz - dlen;

        if (n == 0)
                return(dlen + strlen(s));
        while (*s != '\0') {
                if (n != 1) {
                        *d++ = *s;
                        n--;
                }
                s++;
        }
        *d = '\0';

        return(dlen + (s - src));        /* count does not include NUL */
}


int
ecore_str_has_prefix(const char *str, const char *prefix)
{
   int str_len;
   int prefix_len;

   CHECK_PARAM_POINTER_RETURN("str", str, 0);
   CHECK_PARAM_POINTER_RETURN("prefix", prefix, 0);

   str_len = strlen(str);
   prefix_len = strlen(prefix);
   if (prefix_len > str_len)
     return 0;

   return (strncmp(str, prefix, prefix_len) == 0);
}

int
ecore_str_has_suffix(const char *str, const char *suffix)
{
   int str_len;
   int suffix_len;

   CHECK_PARAM_POINTER_RETURN("str", str, 0);
   CHECK_PARAM_POINTER_RETURN("suffix", suffix, 0);

   str_len = strlen(str);
   suffix_len = strlen(suffix);
   if (suffix_len > str_len)
     return 0;

   return (strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0);
}

/**
 * Splits a string into a maximum of max_tokens pieces, using the given
 * delimiter. If max_tokens is reached, the final string in the returned
 * string array contains the remainder of string.
 *
 * @param string      A string to split.
 * @param delimiter   A string which specifies the places at which to split the 
 *                    string. The delimiter is not included in any of the 
 *                    resulting strings, unless max_tokens is reached.
 * @param max_tokens  The maximum number of strings to split string into. 
 *                    If this is less than 1, the string is split completely.
 * @return            A newly-allocated NULL-terminated array of strings.
 *                    Use ecore_str_vector_free() to free it.
 */
char**
ecore_str_split(const char *string, const char *delimiter, int max_tokens)
{
   char **str_array = NULL;
   char *s;
   size_t n = 0;
   int max = max_tokens;
   const char *remainder;
   size_t delimiter_len;   

   CHECK_PARAM_POINTER_RETURN("string", string, NULL);
   CHECK_PARAM_POINTER_RETURN("delimiter", delimiter, NULL);
  
  /* on the first run we just count the number of the strings we'll finally
   * have */ 
   remainder = string;
   s = strstr(remainder, delimiter);
   if (s)
   {
	delimiter_len = strlen(delimiter);   
	while (--max_tokens && s)
	{
	  remainder = s + delimiter_len;
	  s = strstr(remainder, delimiter);
	  n++;
	}
   }
   if (*string != '\0') n++;
   
   str_array = malloc(sizeof(char *)*(n + 1));
   str_array[n] = NULL;

   /* reset to the initial values */
   n = 0;
   max_tokens = max;
   remainder = string;
   s = strstr(remainder, delimiter);
   if (s)
   {
	while (--max_tokens && s)
	{
	  size_t len;     
	  char *new_string;

	  len = s - remainder;
	  new_string = malloc(sizeof(char)*(len + 1));
	  memcpy(new_string, remainder, len);
	  new_string[len] = 0;
	  str_array[n++] = new_string;

	  remainder = s + delimiter_len;
	  s = strstr(remainder, delimiter);
	}
   }
   if (*string != '\0') str_array[n] = strdup(remainder);

   return str_array;
}

/**
 * Free an array of strings and the array itself
 *
 * @param str_array An NULL-terminated array of strings to free.
 */
void
ecore_str_vector_free(char **str_array)
{
   CHECK_PARAM_POINTER("str_array", str_array);
   int i;

   for(i=0; str_array[i] != NULL; i++)
   {
	FREE(str_array[i]);
   }
   FREE(str_array);
}


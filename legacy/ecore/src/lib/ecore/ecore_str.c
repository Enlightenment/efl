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
#include "Ecore_Str.h"

static int ecore_str_has_suffix_helper(const char *str, const char *suffix, 
		int (*cmp)(const char *, const char *));
/**
 * @param dst the destination
 * @param src the source
 * @param siz the size of the destination
 * @return the length of the source string
 * @brief copy a c-string
 *
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

/**
 * @param dst the destination
 * @param src the source
 * @param siz the size of the destination
 * @return the length of the source string plus MIN(siz, strlen(initial dst)) 
 * @brief append a c-string
 *
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

/**
 * @param str the string to work with
 * @param prefix the prefix to check for
 * @return true if str has the given prefix
 * @brief checks if the string has the given prefix
 */
int
ecore_str_has_prefix(const char *str, const char *prefix)
{
   size_t str_len;
   size_t prefix_len;

   CHECK_PARAM_POINTER_RETURN("str", str, 0);
   CHECK_PARAM_POINTER_RETURN("prefix", prefix, 0);

   str_len = strlen(str);
   prefix_len = strlen(prefix);
   if (prefix_len > str_len)
     return 0;

   return (strncmp(str, prefix, prefix_len) == 0);
}

/**
 * @param str the string to work with
 * @param suffix the suffix to check for
 * @return true if str has the given suffix
 * @brief checks if the string has the given suffix
 */
int
ecore_str_has_suffix(const char *str, const char *suffix)
{
   CHECK_PARAM_POINTER_RETURN("str", str, 0);
   CHECK_PARAM_POINTER_RETURN("suffix", suffix, 0);
   
   return ecore_str_has_suffix_helper(str, suffix, strcmp);
}

/**
 * This function does the same like ecore_str_has_suffix(), but with a
 * case insensitive compare.
 *
 * @param str the string to work with
 * @param ext the  extension to check for
 * @return true if str has the given extension
 * @brief checks if the string has the given extension
 */
int
ecore_str_has_extension(const char *str, const char *ext)
{
   CHECK_PARAM_POINTER_RETURN("str", str, 0);
   CHECK_PARAM_POINTER_RETURN("ext", ext, 0);
   
   return ecore_str_has_suffix_helper(str, ext, strcasecmp);
}

/*
 * Internal helper function used by ecore_str_has_suffix() and 
 * ecore_str_has_extension()
 */
static int
ecore_str_has_suffix_helper(const char *str, const char *suffix, 
		int (*cmp)(const char *, const char *))
{
   size_t str_len;
   size_t suffix_len;

   str_len = strlen(str);
   suffix_len = strlen(suffix);
   if (suffix_len > str_len)
     return 0;

   return cmp(str + str_len - suffix_len, suffix) == 0;
}

/**
 * Splits a string into a maximum of max_tokens pieces, using the given
 * delimiter. If max_tokens is reached, the final string in the returned
 * string array contains the remainder of string.
 *
 * @param str         A string to split.
 * @param delim       A string which specifies the places at which to split the 
 *                    string. The delimiter is not included in any of the 
 *                    resulting strings, unless max_tokens is reached.
 * @param max_tokens  The maximum number of strings to split string into. 
 *                    If this is less than 1, the string is split completely.
 * @return            A newly-allocated NULL-terminated array of strings.
 *                    To free it: free the first element of the array
 *                    and the array itself.
 */
char **
ecore_str_split(const char *str, const char *delim, int max_tokens)
{
   char *s, *sep, **str_array;
   size_t len, dlen;
   int i;

   CHECK_PARAM_POINTER_RETURN("str", str, NULL);
   CHECK_PARAM_POINTER_RETURN("delim", delim, NULL);

   if (*delim == '\0')
      return NULL;

   max_tokens = ((max_tokens <= 0) ? (INT_MAX) : (max_tokens - 1));
   len = strlen(str);
   dlen = strlen(delim);
   s = strdup(str);
   str_array = malloc(sizeof(char *) * (len + 1));
   for (i = 0; (i < max_tokens) && (sep = strstr(s, delim)); i++) 
      {
          str_array[i] = s;
          s = sep + dlen;
          *sep = 0;
      }

   str_array[i++] = s;
   str_array = realloc(str_array, sizeof(char *) * (i + 1));
   str_array[i] = NULL;
 
   return str_array;
}


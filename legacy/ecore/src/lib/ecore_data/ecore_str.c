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

/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore_Str.h"

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
 *
 * @deprecated use eina_strlcpy() instead.
 */
size_t
ecore_strlcpy(char *dst, const char *src, size_t siz)
{
   EINA_LOG_ERR("use eina_strlcpy() instead!");
   return eina_strlcpy(dst, src, siz);
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
 *
 * @deprecated use eina_strlcat() instead.
 */
size_t
ecore_strlcat(char *dst, const char *src, size_t siz)
{
   EINA_LOG_ERR("use eina_strlcat() instead!");
   return eina_strlcat(dst, src, siz);
}

/**
 * @param str the string to work with
 * @param prefix the prefix to check for
 * @return true if str has the given prefix
 * @brief checks if the string has the given prefix
 *
 * @deprecated use eina_str_has_prefix() instead.
 */
int
ecore_str_has_prefix(const char *str, const char *prefix)
{
   EINA_LOG_ERR("use eina_str_has_prefix() instead!");
   return eina_str_has_prefix(str, prefix);
}

/**
 * @param str the string to work with
 * @param suffix the suffix to check for
 * @return true if str has the given suffix
 * @brief checks if the string has the given suffix
 *
 * @deprecated use eina_str_has_suffix() instead.
 */
int
ecore_str_has_suffix(const char *str, const char *suffix)
{
   EINA_LOG_ERR("use eina_str_has_suffix() instead!");
   return eina_str_has_suffix(str, suffix);
}

/**
 * This function does the same like ecore_str_has_suffix(), but with a
 * case insensitive compare.
 *
 * @param str the string to work with
 * @param ext the  extension to check for
 * @return true if str has the given extension
 * @brief checks if the string has the given extension
 *
 * @deprecated use eina_str_has_extension() instead.
 */
int
ecore_str_has_extension(const char *str, const char *ext)
{
   EINA_LOG_ERR("use eina_str_has_extension() instead!");
   return eina_str_has_extension(str, ext);
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
 *
 * @deprecated eina_str_split()
 */
char **
ecore_str_split(const char *str, const char *delim, int max_tokens)
{
   EINA_LOG_ERR("use eina_str_split() instead!");
   return eina_str_split(str, delim, max_tokens);
}

/**
 * Join two strings of known length and store the result in @a dst buffer.
 *
 * @param dst where to store the result.
 * @param size byte size of dst, will write at most (size - 1)
 *     characters and then the '\0' (null terminator).
 * @param sep separator character to use.
 * @param a first string to use, before @a sep.
 * @param a_len length of @a a, not including '\0' (strlen()-like)
 * @param b second string to use, after @a sep.
 * @param b_len length of @a b, not including '\0' (strlen()-like)
 *
 * @return the number of characters printed (not including the
 *     trailing '\0' used to end output to strings). Just like
 *     snprintf(), it will not write more than @a size bytes, thus a
 *     return value of @a size or more means that the output was
 *     truncated.
 *
 * @see ecore_str_join() and ecore_str_join_static()
 *
 * @deprecated use eina_str_join_len() instead.
 */
size_t
ecore_str_join_len(char *dst, size_t size, char sep, const char *a, size_t a_len, const char *b, size_t b_len)
{
   EINA_LOG_ERR("use eina_str_join_len() instead!");
   return eina_str_join_len(dst, size, sep, a, a_len, b, b_len);
}

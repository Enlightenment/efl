#ifndef _EINA_STR_H
#define _EINA_STR_H

#include <stddef.h>
#include <string.h>

#include "eina_types.h"

/**
 * @file eina_str.h
 * @brief Contains useful C string functions.
 */

/* strlcpy implementation for libc's lacking it */
EAPI size_t eina_strlcpy(char *dst, const char *src, size_t siz);
EAPI size_t eina_strlcat(char *dst, const char *src, size_t siz);

EAPI int eina_str_has_prefix(const char *str, const char *prefix);

EAPI int eina_str_has_suffix(const char *str, const char *suffix);
EAPI int eina_str_has_extension(const char *str, const char *ext);

EAPI char **eina_str_split(const char *string, const char *delimiter, 
                            int max_tokens);

EAPI size_t eina_str_join_len(char *dst, size_t size, char sep, const char *a, size_t a_len, const char *b, size_t b_len);


/**
 * Join two strings and store the result in @a dst buffer.
 *
 * Similar to eina_str_join_len(), but will compute the length of @a
 * and @a b using strlen().
 *
 * @param dst where to store the result.
 * @param size byte size of dst, will write at most (size - 1)
 *     characters and then the '\0' (null terminator).
 * @param sep separator character to use.
 * @param a first string to use, before @a sep.
 * @param b second string to use, after @a sep.
 *
 * @return the number of characters printed (not including the
 *     trailing '\0' used to end output to strings). Just like
 *     snprintf(), it will not write more than @a size bytes, thus a
 *     return value of @a size or more means that the output was
 *     truncated.
 *
 * @see eina_str_join_len() and eina_str_join_static()
 */
static inline size_t eina_str_join(char *dst, size_t size, char sep, const char *a, const char *b)
{
   return eina_str_join_len(dst, size, sep, a, strlen(a), b, strlen(b));
}

/**
 * Join two static strings and store the result in static @a dst buffer.
 *
 * Similar to eina_str_join_len(), but will assume string sizes are
 * know using sizeof(X).
 *
 * @param dst where to store the result.
 * @param sep separator character to use.
 * @param a first string to use, before @a sep.
 * @param b second string to use, after @a sep.
 *
 * @return the number of characters printed (not including the
 *     trailing '\0' used to end output to strings). Just like
 *     snprintf(), it will not write more than @a size bytes, thus a
 *     return value of @a size or more means that the output was
 *     truncated.
 *
 * @see eina_str_join() and eina_str_join_static()
 */
#define eina_str_join_static(dst, sep, a, b) eina_str_join_len(dst, sizeof(dst), sep, a, (sizeof(a) > 0) ? sizeof(a) - 1 : 0, b, (sizeof(b) > 0) ? sizeof(b) - 1 : 0)

#endif /* EINA_STR_H */

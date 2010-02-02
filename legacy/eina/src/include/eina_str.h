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

EAPI Eina_Bool eina_str_has_prefix(const char *str, const char *prefix);

EAPI Eina_Bool eina_str_has_suffix(const char *str, const char *suffix);
EAPI Eina_Bool eina_str_has_extension(const char *str, const char *ext);

EAPI char **eina_str_split(const char *string, const char *delimiter, 
                            int max_tokens);

EAPI size_t eina_str_join_len(char *dst, size_t size, char sep, const char *a, size_t a_len, const char *b, size_t b_len);

EAPI char *eina_str_convert(const char *enc_from, const char *enc_to, const char *text);


/**
 * @brief Join two strings of known length.
 *
 * @param dst The buffer to store the result.
 * @param size Size (in byte) of the buffer.
 * @param sep The separator character to use.
 * @param a First string to use, before @p sep.
 * @param b Second string to use, after @p sep.
 * @return The number of characters printed.
 *
 * This function is similar to eina_str_join_len(), but will compute
 * the length of @p a  and @p b using strlen().
 *
 * @see eina_str_join_len()
 * @see eina_str_join_static()
 */
static inline size_t eina_str_join(char *dst, size_t size, char sep, const char *a, const char *b)
{
   return eina_str_join_len(dst, size, sep, a, strlen(a), b, strlen(b));
}

/**
 * @brief Join two static strings and store the result in a static buffer.
 *
 * @param dst The buffer to store the result.
 * @param size Size (in byte) of the buffer.
 * @param sep The separator character to use.
 * @param a First string to use, before @p sep.
 * @param b Second string to use, after @p sep.
 * @return The number of characters printed.
 *
 * This function is similar to eina_str_join_len(), but will assume
 * string sizes are know using sizeof(X).
 *
 * @see eina_str_join()
 * @see eina_str_join_static()
 */
#define eina_str_join_static(dst, sep, a, b) eina_str_join_len(dst, sizeof(dst), sep, a, (sizeof(a) > 0) ? sizeof(a) - 1 : 0, b, (sizeof(b) > 0) ? sizeof(b) - 1 : 0)

#endif /* EINA_STR_H */

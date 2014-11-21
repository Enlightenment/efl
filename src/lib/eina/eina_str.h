#ifndef _EINA_STR_H
#define _EINA_STR_H

#include <stddef.h>
#include <string.h>

#include "eina_types.h"

/**
 * @defgroup Eina_String_Group String
 * @ingroup Eina_Tools_Group
 *
 * @brief The group discusses useful functions for C string manipulation.
 *
 * This group of functions allows you to manipulate strings more easily, they
 * provide functionality that is not available through string.h.
 *
 * Since these functions modify the strings they can't be used with
 * shared strings(eina_stringshare).
 *
 * @{
 */

/* strlcpy implementation for libc's lacking it */

/**
 * @brief Copies one c-string to another.
 *
 * @details This function copies up to @a siz - 1 characters from the
 *          NULL-terminated string @a src to @a dst, NULL-terminating the result
 *          (unless @a siz is equal to 0). The returned value is the length of
 *          @a src. If the returned value is greater than @a siz, truncation
 *          occurs.
 *
 * @since_tizen 2.3
 *
 * @remarks The main difference between eina_strlcpy and strncpy is that this
 *          ensures that @a dst is NULL-terminated even if no @c NULL byte is found in the first
 *          @a siz bytes of @a src.
 *
 * @param[in] dst The destination string
 * @param[in] src The source string
 * @param[in] siz The size of the destination string
 * @return The length of the source string
 *
 */
EAPI size_t          eina_strlcpy(char *dst, const char *src, size_t siz) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Appends a c-string.
 *
 * @details This function appends @a src to @a dst of size @a siz (unlike
 *          strncat, @a siz is the full size of @a dst, no space is left).  At
 *          most @a siz - 1 characters are copied.  Always NULL-terminates
 *          (unless @a siz <= strlen(dst)). This function returns strlen(src) +
 *          MIN(siz, strlen(initial dst)). If the returned value is greater than or
 *          equal to @a siz, truncation occurs.
 *
 * @since_tizen 2.3
 *
 * @param[in] dst The destination string
 * @param[in] src The source string
 * @param[in] siz The size of the destination string
 * @return The length of the source string plus MIN(siz, strlen(initial dst))
 *
 */
EAPI size_t          eina_strlcat(char *dst, const char *src, size_t siz) EINA_ARG_NONNULL(1, 2);


/**
 * @brief Check whether the given string has the given prefix.
 *
 * @details This function returns @c EINA_TRUE if @a str has the prefix
 *          @a prefix, otherwise it returns @c EINA_FALSE. If the length of @a prefix is
 *          greater than @a str, @c EINA_FALSE is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] str The string to work with
 * @param[in] prefix The prefix to check for
 * @return @c EINA_TRUE if the string has the given prefix, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool       eina_str_has_prefix(const char *str, const char *prefix) EINA_PURE EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Check whether the given string has the given suffix.
 *
 * @details This function returns @c EINA_TRUE if @a str has the suffix
 *          @a suffix, otherwise it returns @c EINA_FALSE. If the length of @a suffix is
 *          greater than @a str, @c EINA_FALSE is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] str The string to work with
 * @param[in] suffix The suffix to check for
 * @return @c EINA_TRUE if the string has the given suffix, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool       eina_str_has_suffix(const char *str, const char *suffix) EINA_PURE EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Check whether the given string has the given extension.
 *
 * @details This function does the same as eina_str_has_suffix(), except it's case
 *          insensitive.
 *
 * @since_tizen 2.3
 *
 * @param[in] str The string to work with
 * @param[in] ext The  extension to check for
 * @return @c EINA_TRUE if the string has the given extension, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool       eina_str_has_extension(const char *str, const char *ext) EINA_PURE EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Splits a string using a delimiter.
 *
 * @details This function splits @a string into a maximum of @a max_tokens pieces,
 *          using the given delimiter @a delimiter. @a delimiter is not included in any
 *          of the resulting strings, unless @a max_tokens is reached. If
 *          @a max_tokens is less than @c 1, the string is splitted as many times as possible. If
 *          @a max_tokens is reached, the last string in the returned string
 *          array contains the remainder of the string. The returned value is a
 *          newly allocated NULL-terminated array of strings or @c NULL if it fails to
 *          allocate the array. To free it, free the first element of the array and the
 *          array itself.
 *
 * @since_tizen 2.3
 *
 * @remarks If you need the number of elements in the returned array see
 *          eina_str_split_full().
 *
 * @param[in] string The string to split
 * @param[in] delimiter The string that specifies the places at which to split the string
 * @param[in] max_tokens The maximum number of strings to split the string into, or a number less
 *                   than @c 1 to split as many times as possible \n
 *                   This parameter IGNORES the added @c NULL terminator.
 * @return A newly-allocated NULL-terminated array of strings, otherwise @c NULL if it
 *         fails to allocate the array
 *
 */
EAPI char          **eina_str_split(const char *string, const char *delimiter, int max_tokens) EINA_ARG_NONNULL(1, 2) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Splits a string using a delimiter and returns the number of elements.
 *
 * @details This function splits @a string into a maximum of @a max_tokens pieces,
 *           using the given delimiter @a delimiter. @a delimiter is not included in any
 *           of the resulting strings, unless @a max_tokens is reached. If
 *           @a max_tokens is less than @c 1, the string is splitted as many times as possible. If
 *           @a max_tokens is reached, the last string in the returned string
 *           array contains the remainder of the string. The returned value is a
 *           newly allocated NULL-terminated array of strings or @c NULL if it fails to
 *           allocate the array. To free it, free the first element of the array and the
 *           array itself.
 *
 * @since_tizen 2.3
 *
 * @remarks The actual size of the returned array, when @a elements returns greater than zero,
 *          is always @a elements + 1. This is due to the @c NULL terminator element that
 *          is added to the array for safety. If it returns @c 6, the number of split strings returned
 *          is 6, but the size of the array (including the @c NULL element) is actually 7.
 *
 * @param string The string to split
 * @param[in] delimiter The string that specifies the places at which to split the string
 * @param[in] max_tokens The maximum number of strings to split the string into, or a number less
 *                   than @c 1 to split as many times as possible \n
 *                   This parameter IGNORES the added @c NULL terminator.
 * @param[out] elements The number of elements in the returned array \n
 *                 This array is guaranteed to be no greater than @a max_tokens, and
 *                 it does NOT count the @c NULL terminator element.
 * @return A newly-allocated NULL-terminated array of strings, otherwise @c NULL if it
 *         fails to allocate the array
 *
 * @see eina_str_split()
 */
EAPI char          **eina_str_split_full(const char *string, const char *delimiter, int max_tokens, unsigned int *elements) EINA_ARG_NONNULL(1, 2, 4) EINA_MALLOC EINA_WARN_UNUSED_RESULT;


/**
 * @brief Joins two strings of known length.
 *
 * @details This function joins the strings @a a and @a b (in that order) and
 *          separates them with @a sep. The result is stored in the buffer
 *          @a dst and at most @a size - 1 characters are written and the
 *          string is NULL-terminated. @a a_len is the length of @a a (not
 *          including '\\0') and @a b_len is the length of @a b (not including
 *          '\\0'). This function returns the number of characters printed (not
 *          including the trailing '\\0' used to end output to the strings). Just
 *          like snprintf(), it does not write more than @a size bytes, thus a
 *          returned value of @a size or more means that the output is
 *          truncated.
 *
 * @since_tizen 2.3
 *
 * @param[in] dst The buffer to store the result
 * @param[in] size The size (in byte) of the buffer
 * @param[in] sep The separator character to use
 * @param[in] a The first string to use, before @a sep
 * @param[in] a_len The length of @a a
 * @param[in] b The second string to use, after @a sep
 * @param[in] b_len The length of @a b
 * @return The number of characters printed
 *
 * @see eina_str_join()
 * @see eina_str_join_static()
 */
EAPI size_t          eina_str_join_len(char *dst, size_t size, char sep, const char *a, size_t a_len, const char *b, size_t b_len) EINA_ARG_NONNULL(1, 4, 6);


/**
 * @brief Uses Iconv to convert a text string from one encoding to another.
 *
 * @details This function converts @a text, encoded in @a enc_from. On success,
 *          the converted text is returned and is encoded in @a enc_to. On
 *          failure, @c NULL is returned. Iconv is used to convert @a text. If
 *          Iconv is not available, @c NULL is returned. When not used anymore,
 *          the returned value must be freed.
 *
 * @since_tizen 2.3
 *
 * @param[in] enc_from The encoding to convert from
 * @param[in] enc_to The encoding to convert to
 * @param[in] text The text to convert
 * @return The converted text
 *
 */
EAPI char           *eina_str_convert(const char *enc_from, const char *enc_to, const char *text) EINA_WARN_UNUSED_RESULT EINA_MALLOC EINA_ARG_NONNULL(1, 2, 3);


/**
 * @brief Escapes back slashes, spaces, and apostrophes in strings.
 *
 * @since_tizen 2.3
 *
 * @remarks Escaping is done by adding a back slash \ before any occurrence of back slashes \,
 *          spaces " ", or apostrophes "'". This function returns a newly allocated
 *          escaped string on success or @c NULL on failure. When not used anymore, the
 *          returned value must be freed.
 *
 * @param[in] str The string to escape
 * @return The escaped string
 *
 */
EAPI char           *eina_str_escape(const char *str) EINA_WARN_UNUSED_RESULT EINA_MALLOC EINA_ARG_NONNULL(1);


/**
 * @brief Lowercases all the characters in the range [A-Z] in the given string.
 *
 * @details This function modifies the original string, changing all characters
 *          in [A-Z] to lowercase. If @a str is @c NULL or is an empty string,
 *          this function does nothing.
 *
 * @since_tizen 2.3
 *
 * @param[out] str The string to lowercase
 *
 */
EAPI void            eina_str_tolower(char **str);

/**
 * @brief Uppercases all the characters in the range [a-z] in the given string.
 *
 * @details This function modifies the original string, changing all characters
 *          in [a-z] to uppercase. If @a str is @c NULL or is an empty string,
 *          this function does nothing.
 *
 * @since_tizen 2.3
 *
 * @param[out] str The string to uppercase
 *
 */
EAPI void            eina_str_toupper(char **str);

/**
 * @brief Join two strings of known length.
 *
 * @details This function is similar to eina_str_join_len(), but will compute
 *          the length of @p a  and @p b using strlen().
 *
 * @since_tizen 2.3
 *
 * @param[in] dst The buffer to store the result.
 * @param[in] size Size (in byte) of the buffer.
 * @param[in] sep The separator character to use.
 * @param[in] a First string to use, before @p sep.
 * @param[in] b Second string to use, after @p sep.
 * @return The number of characters printed.
 *
 * @see eina_str_join_len()
 * @see eina_str_join_static()
 */
static inline size_t eina_str_join(char *dst, size_t size, char sep, const char *a, const char *b) EINA_ARG_NONNULL(1, 4, 5);

/**
 * @def eina_str_join_static(dst, sep, a, b)
 * @brief Joins two static strings and stores the result in a static buffer.
 *
 * @details This function is similar to eina_str_join_len(), but assumes
 *          that string sizes are known using sizeof(X).
 *
 * @since_tizen 2.3
 *
 * @param dst The buffer to store the result
 * @param sep The separator character to use
 * @param a The first string to use, before @a sep
 * @param b The second string to use, after @a sep
 * @return The number of characters printed
 *
 * @see eina_str_join()
 * @see eina_str_join_static()
 */
#define eina_str_join_static(dst, sep, a, b) eina_str_join_len(dst, sizeof(dst), sep, a, (sizeof(a) > 0) ? sizeof(a) - 1 : 0, b, (sizeof(b) > 0) ? sizeof(b) - 1 : 0)

/**
 * @brief Count up to a given amount of bytes of the given string.
 *
 * @details This function returns the size of @p str, up to @p maxlen
 *          characters. It avoid needless iterations after that size. @p str
 *          must be a valid pointer and MUST not be @c NULL, otherwise this
 *          function will crash. This function returns the string size, or
 *          (size_t)-1 if the size is greater than @a maxlen.
 *
 * @since_tizen 2.3
 *
 * @param[in] str The string pointer.
 * @param[in] maxlen The maximum length to allow.
 * @return the string size or (size_t)-1 if greater than @a maxlen.
 */
static inline size_t eina_strlen_bounded(const char *str, size_t maxlen) EINA_PURE EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

#include "eina_inline_str.x"

/**
 * @}
 */

#endif /* EINA_STR_H */

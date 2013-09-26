#ifndef _EINA_STR_H
#define _EINA_STR_H

#include <stddef.h>
#include <string.h>

#include "eina_types.h"

/**
 * @page tutorial_eina_string Eina String example
 * @dontinclude eina_str_01.c
 *
 * Whenever using eina we need to include it:
 * @skipline #include
 * @line #include
 *
 * In our main function we declare(and initialize) some variables and initialize
 * eina:
 * @until eina_init
 *
 * It's frequently necessary to split a string into its constituent parts,
 * eina_str_split() make's it easy to do so:
 * @until printf
 *
 * Another common need is to make a string uppercase or lowercase, so let's
 * create a string and make it uppercase and then make it lowercase again:
 * @until printf
 * @until printf
 *
 * Next we use eina to check if our @p names string starts or ends with some
 * values:
 * @until Has
 *
 * When strings will be used in a terminal(or a number of other places) it
 * necessary to escape certain characters that appear in them:
 * @until printf
 *
 * Much as we previously split a string we will now join two strings:
 * @until printf
 *
 * With strlcpy() we can copy what portion of the @p prologue fits in @p str and
 * be sure that it's still NULL terminated:
 * @until printf
 *
 * Since we are done with @p prologue and @p str we should free them:
 * @until free(str
 *
 * Finally we see strlcat in action:
 * @until printf("
 *
 * And then shut eina down and exit:
 * @until }
 * @example eina_str_01.c
 */
/**
 * @addtogroup Eina_String_Group String
 *
 * @brief Provide useful functions for C string manipulation.
 *
 * This group of functions allow you to more easily manipulate strings, they
 * provide functionality not available through string.h.
 *
 * @warning Since these functions modify the strings they can't be used with
 * shared strings(eina_stringshare).
 *
 * See an example @ref tutorial_eina_string "here".
 */

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * For more information refer to the @ref tutorial_eina_string "string example".
 *
 * @{
 */

/**
 * @defgroup Eina_String_Group String
 *
 * @{
 */

/* strlcpy implementation for libc's lacking it */

/**
 * @brief Copy a c-string to another.
 *
 * @param dst The destination string.
 * @param src The source string.
 * @param siz The size of the destination string.
 * @return The length of the source string.
 *
 * This function copies up to @p siz - 1 characters from the
 * NULL-terminated string @p src to @p dst, NULL-terminating the result
 * (unless @p siz is equal to 0). The returned value is the length of
 * @p src. If the returned value is greater than @p siz, truncation
 * occurred.
 *
 * @note The main difference between eina_strlcpy and strncpy is that this
 * ensures @p dst is NULL-terminated even if no @c NULL byte is found in the first
 * @p siz bytes of src.
 */
EAPI size_t          eina_strlcpy(char *dst, const char *src, size_t siz) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Append a c-string.
 *
 * @param dst The destination string.
 * @param src The source string.
 * @param siz The size of the destination string.
 * @return The length of the source string plus MIN(siz, strlen(initial dst))
 *
 * This function appends @p src to @p dst of size @p siz (unlike
 * strncat, @p siz is the full size of @p dst, not space left).  At
 * most @p siz - 1 characters will be copied.  Always NULL-terminates
 * (unless @p siz <= strlen(dst)). This function returns strlen(src) +
 * MIN(siz, strlen(initial dst)). If the returned value is greater or
 * equal than @p siz, truncation occurred.
 */
EAPI size_t          eina_strlcat(char *dst, const char *src, size_t siz) EINA_ARG_NONNULL(1, 2);


/**
 * @brief Check if the given string has the given prefix.
 *
 * @param str The string to work with.
 * @param prefix The prefix to check for.
 * @return #EINA_TRUE if the string has the given prefix, #EINA_FALSE otherwise.
 *
 * This function returns #EINA_TRUE if @p str has the prefix
 * @p prefix, #EINA_FALSE otherwise. If the length of @p prefix is
 * greater than @p str, #EINA_FALSE is returned.
 */
EAPI Eina_Bool       eina_str_has_prefix(const char *str, const char *prefix) EINA_PURE EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Check if the given string has the given suffix.
 *
 * @param str The string to work with.
 * @param suffix The suffix to check for.
 * @return #EINA_TRUE if the string has the given suffix, #EINA_FALSE otherwise.
 *
 * This function returns #EINA_TRUE if @p str has the suffix
 * @p suffix, #EINA_FALSE otherwise. If the length of @p suffix is
 * greater than @p str, #EINA_FALSE is returned.
 */
EAPI Eina_Bool       eina_str_has_suffix(const char *str, const char *suffix) EINA_PURE EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Check if the given string has the given extension.
 *
 * @param str The string to work with.
 * @param ext The  extension to check for.
 * @return #EINA_TRUE if the string has the given extension, #EINA_FALSE otherwise.
 *
 * This function does the same as eina_str_has_suffix(), except it's case
 * insensitive.
 */
EAPI Eina_Bool       eina_str_has_extension(const char *str, const char *ext) EINA_PURE EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Split a string using a delimiter.
 *
 * @param string The string to split.
 * @param delimiter The string which specifies the places at which to split the string.
 * @param max_tokens The maximum number of strings to split string into, or a number less
 *                   than 1 to split as many times as possible. This parameter
 *                   IGNORES the added @c NULL terminator.
 * @return A newly-allocated NULL-terminated array of strings or @c NULL if it
 * fails to allocate the array.
 *
 * This function splits @p string into a maximum of @p max_tokens pieces,
 * using the given delimiter @p delimiter. @p delimiter is not included in any
 * of the resulting strings, unless @p max_tokens is reached. If
 * @p max_tokens is less than @c 1, the string is splitted as many times as possible. If
 * @p max_tokens is reached, the last string in the returned string
 * array contains the remainder of string. The returned value is a
 * newly allocated NULL-terminated array of strings or @c NULL if it fails to
 * allocate the array. To free it, free the first element of the array and the
 * array itself.
 *
 * @note If you need the number of elements in the returned array see
 * eina_str_split_full().
 */
EAPI char          **eina_str_split(const char *string, const char *delimiter, int max_tokens) EINA_ARG_NONNULL(1, 2) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Split a string using a delimiter and returns number of elements.
 *
 * @param string The string to split.
 * @param delimiter The string which specifies the places at which to split the string.
 * @param max_tokens The maximum number of strings to split string into, or a number less
 *                   than 1 to split as many times as possible. This parameter
 *                   IGNORES the added @c NULL terminator.
 * @param elements Where to return the number of elements in returned
 *        array. This array is guaranteed to be no greater than @p max_tokens, and
 *        it will NOT count the @c NULL terminator element.
 * @return A newly-allocated NULL-terminated array of strings or @c NULL if it
 * fails to allocate the array.
 *
 * This function splits @p string into a maximum of @p max_tokens pieces,
 * using the given delimiter @p delimiter. @p delimiter is not included in any
 * of the resulting strings, unless @p max_tokens is reached. If
 * @p max_tokens is less than @c 1, the string is splitted as many times as possible. If
 * @p max_tokens is reached, the last string in the returned string
 * array contains the remainder of string. The returned value is a
 * newly allocated NULL-terminated array of strings or @c NULL if it fails to
 * allocate the array. To free it, free the first element of the array and the
 * array itself.
 *
 * @note The actual size of the returned array, when @p elements returns greater than zero,
 *       will always be @p elements + 1. This is due to the @c NULL terminator element that
 *       is added to the array for safety. If it returns @c 6, the number of split strings returned
 *       will be 6, but the size of the array (including the @c NULL element) will actually be 7.
 *
 * @see eina_str_split()
 */
EAPI char          **eina_str_split_full(const char *string, const char *delimiter, int max_tokens, unsigned int *elements) EINA_ARG_NONNULL(1, 2, 4) EINA_MALLOC EINA_WARN_UNUSED_RESULT;


/**
 * @brief Join two strings of known length.
 *
 * @param dst The buffer to store the result.
 * @param size Size (in byte) of the buffer.
 * @param sep The separator character to use.
 * @param a First string to use, before @p sep.
 * @param a_len length of @p a.
 * @param b Second string to use, after @p sep.
 * @param b_len length of @p b.
 * @return The number of characters printed.
 *
 * This function joins the strings @p a and @p b (in that order) and
 * separate them with @p sep. The result is stored in the buffer
 * @p dst and at most @p size - 1 characters will be written and the
 * string is NULL-terminated. @p a_len is the length of @p a (not
 * including '\\0') and @p b_len is the length of @p b (not including
 * '\\0'). This function returns the number of characters printed (not
 * including the trailing '\\0' used to end output to strings). Just
 * like snprintf(), it will not write more than @p size bytes, thus a
 * returned value of @p size or more means that the output was
 * truncated.
 *
 * @see eina_str_join()
 * @see eina_str_join_static()
 */
EAPI size_t          eina_str_join_len(char *dst, size_t size, char sep, const char *a, size_t a_len, const char *b, size_t b_len) EINA_ARG_NONNULL(1, 4, 6);


/**
 * @brief Use Iconv to convert a text string from one encoding to another.
 *
 * @param enc_from Encoding to convert from.
 * @param enc_to Encoding to convert to.
 * @param text The text to convert.
 * @return The converted text.
 *
 * This function converts @p text, encoded in @p enc_from. On success,
 * the converted text is returned and is encoded in @p enc_to. On
 * failure, @c NULL is returned. Iconv is used to convert @p text. If
 * Iconv is not available, @c NULL is returned. When not used anymore,
 * the returned value must be freed.
 *
 * @warning This function is guaranteed to break when '\0' characters are in @p text.
 * DO NOT USE THIS FUNCTION IF YOUR TEXT CONTAINS NON-TERMINATING '\0' CHARACTERS.
 */
EAPI char           *eina_str_convert(const char *enc_from, const char *enc_to, const char *text) EINA_WARN_UNUSED_RESULT EINA_MALLOC EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Use Iconv to convert a text string from one encoding to another.
 *
 * @param enc_from Encoding to convert from.
 * @param enc_to Encoding to convert to.
 * @param text The text to convert.
 * @param len The size in bytes of the text to convert.
 * @param retlen The size in bytes of the converted text.
 * @return The converted text.
 *
 * This function converts @p text, encoded in @p enc_from. On success,
 * the converted text is returned and is encoded in @p enc_to. On
 * failure, @c NULL is returned. Iconv is used to convert @p text. If
 * Iconv is not available, @c NULL is returned. When not used anymore,
 * the returned value must be freed.
 *
 * @since 1.8
 */
EAPI char           *eina_str_convert_len(const char *enc_from, const char *enc_to, const char *text, size_t len, size_t *retlen) EINA_WARN_UNUSED_RESULT EINA_MALLOC EINA_ARG_NONNULL(1, 2, 3);


/**
 * @brief Escape slashes, spaces and apostrophes in strings.
 *
 * @param str The string to escape.
 * @return The escaped string.
 *
 * Escaping is done by adding a slash "\" before any occurrence of slashes "\",
 * spaces " " or apostrophes "'". This function returns a newly allocated
 * escaped string on success, @c NULL on failure. When not used anymore, the
 * returned value must be freed.
 */
EAPI char           *eina_str_escape(const char *str) EINA_WARN_UNUSED_RESULT EINA_MALLOC EINA_ARG_NONNULL(1);


/**
 * @brief Lowercase all the characters in range [A-Z] in the given string.
 *
 * @param str The string to lowercase.
 *
 * This function modifies the original string, changing all characters
 * in [A-Z] to lowercase. If @p str is @c NULL or is an empty string,
 * this function does nothing.
 */
EAPI void            eina_str_tolower(char **str);

/**
 * @brief Uppercase all the characters in range [a-z] in the given string.
 *
 * @param str The string to uppercase.
 *
 * This function modifies the original string, changing all characters
 * in [a-z] to uppercase. If @p str is @c NULL or is an empty string,
 * this function does nothing.
 */
EAPI void            eina_str_toupper(char **str);

static inline size_t eina_str_join(char *dst, size_t size, char sep, const char *a, const char *b) EINA_ARG_NONNULL(1, 4, 5);

/**
 * @def eina_str_join_static(dst, sep, a, b)
 * @brief Join two static strings and store the result in a static buffer.
 *
 * @param dst The buffer to store the result.
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

static inline size_t eina_strlen_bounded(const char *str, size_t maxlen) EINA_PURE EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

#include "eina_inline_str.x"

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_STR_H */

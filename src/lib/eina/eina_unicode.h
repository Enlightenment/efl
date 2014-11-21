#ifndef EINA_UNICODE_H
#define EINA_UNICODE_H

#include <stdlib.h>

#include "eina_config.h"
#include "eina_types.h"

/**
 * @defgroup Eina_Unicode_String Unicode String
 * @ingroup Eina_Data_Types_Group
 *
 * @brief This group discusses the functions that provide basic unicode string handling.
 *
 * Eina_Unicode is a type that holds unicode codepoints.
 *
 * @{
 */

/**
 * @typedef Eina_Unicode
 * @brief A type that holds Unicode codepoints.
 */
#if EINA_SIZEOF_WCHAR_T >= 4
# include <wchar.h>
typedef wchar_t Eina_Unicode;
#elif defined(EINA_HAVE_INTTYPES_H)
# include <inttypes.h>
typedef uint32_t Eina_Unicode;
#elif defined(EINA_HAVE_STDINT_H)
# include <stdint.h>
typedef uint32_t Eina_Unicode;
#else
/* Hope that int is big enough */
typedef unsigned int Eina_Unicode;
#endif


/**
 * @brief Same as the standard strlen just with Eina_Unicode instead of char.
 *
 * @since_tizen 2.3
 */
EAPI extern const Eina_Unicode *EINA_UNICODE_EMPTY_STRING;

/**
 * @brief Returns the length of a Eina_Unicode string.
 *
 * @details This function returns the number of characters in a string.
 *
 * @since_tizen 2.3
 *
 * @remarks If the terminating character is not found in the string, it can
 *          go infinite loop
 *
 * @param[in] ustr The string to search
 * @return The number of characters
 */
EAPI size_t        eina_unicode_strlen(const Eina_Unicode *ustr) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Returns the length of a Eina_Unicode string, up to a limit.
 *
 * @details This function returns the number of characters in a string, up to a maximum
 *          of @a n. If the terminating character is not found in the string, it returns
 *          @a n.
 *
 * @since_tizen 2.3
 *
 * @param[in] ustr The string to search
 * @param[in] n The maximum length to search
 * @return The number of characters or @a n
 */
EAPI size_t        eina_unicode_strnlen(const Eina_Unicode *ustr, int n) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;


/**
 * @brief Same as the standard strdup just with Eina_Unicode instead of char.
 *
 * @param[in] text The string to be duplicated
 * @return A new string
 *
 * @since_tizen 2.3
 */
EAPI Eina_Unicode *eina_unicode_strdup(const Eina_Unicode *text) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;


/**
 * @brief Same as strdup but cuts on the given size. Assumes n < len.
 *
 * @details This function duplicates @a text. The resulting string is cut on @a n.
 *          @a n is assumed to be lesser (<) than the length of @a
 *          text. When not needed anymore, the returned string must be freed.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @param[in] text The text to duplicate
 * @param[in] n The maximum size of the text to duplicate
 * @return The duplicated string
 *
 */
EAPI Eina_Unicode *eina_unicode_strndup(const Eina_Unicode *text, size_t n) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;


/**
 * @brief Same as the standard strcmp just with Eina_Unicode instead of char.
 *
 * @param[in] a A string to be compared
 * @param[in] b A string to be compared
 * @return an integer less than, equal to, or greater than zero if a is found,
 *         respectively, to be less than, to match, or be greater than b.
 *
 * @since_tizen 2.3
 */
EAPI int           eina_unicode_strcmp(const Eina_Unicode *a, const Eina_Unicode *b) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;


/**
 * @brief Same as the standard strcpy just with Eina_Unicode instead of char.
 *
 * @param[in] dest The destination string
 * @param[in] source The source string
 * @return The copied string pointer
 *
 * @since_tizen 2.3
 */
EAPI Eina_Unicode *eina_unicode_strcpy(Eina_Unicode *dest, const Eina_Unicode *source) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Same as the standard strstr just with Eina_Unicode instead of char.
 *
 * @param[in] haystack The string
 * @param[in] needle The substring to be found
 * @return haystack is returned
 *
 * @since_tizen 2.3
 */
EAPI Eina_Unicode *eina_unicode_strstr(const Eina_Unicode *haystack, const Eina_Unicode *needle) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;

/**
 * @brief Same as the standard strncpy just with Eina_Unicode instead of char.
 *
 * @param[in] dest The destination string
 * @param[in] source The source string
 * @param[in] n Bytes to be copied
 * @return The copied string pointer
 *
 * @since_tizen 2.3
 */
EAPI Eina_Unicode *eina_unicode_strncpy(Eina_Unicode *dest, const Eina_Unicode *source, size_t n) EINA_ARG_NONNULL(1, 2);


/**
 * @brief Escapes a unicode string.
 *
 * @since_tizen 2.3
 *
 * @param[in] str The string to escape
 * @return The escaped string
 *
 * @see eina_str_escape()
 */
EAPI Eina_Unicode *eina_unicode_escape(const Eina_Unicode *str) EINA_ARG_NONNULL(1) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/* UTF-8 handling */


/**
 * @brief Reads UTF8 bytes from @a buf, starting at @a iindex and returns
 *        the decoded code point at @a iindex offset, and advances @a iindex
 *        to the next code point after this. @a iindex is always advanced,
 *        unless the advancement is after @c NULL.
 *        On error, return a code point between DC80 and DCFF where the lower 8 bits
 *        are the byte's value.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string
 * @param[out] iindex The index to look at and return from
 * @return The found code point
 */
EAPI Eina_Unicode eina_unicode_utf8_get_next(const char *buf, int *iindex) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Reads UTF8 bytes from @a buf, starting at @a iindex and returns
 *        the decoded code point at @a iindex offset, and moves  @a iindex
 *        to the previous code point. @a iindex is always moved, as long
 *        as it's not past the start of the string.
 *        On error, return a code point between DC80 and DCFF where the lower 8 bits
 *        are the byte's value.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string
 * @param[out] iindex The index to look at and return from
 * @return The found code point, @c 0 if @a buf or @a iindex are @c NULL
 */
EAPI Eina_Unicode eina_unicode_utf8_get_prev(const char *buf, int *iindex) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Returns the number of unicode characters in the string. That is,
 *        the number of Eina_Unicodes it takes to store this string in
 *        an Eina_Unicode string.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @param[in] buf The string
 * @return The number of unicode characters (not bytes) in the string
 */
EAPI int eina_unicode_utf8_get_len(const char *buf) EINA_ARG_NONNULL(1);

/**
 * @brief Converts a UTF-8 string to a newly allocated Eina_Unicode string.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @param[in] utf The string in UTF-8
 * @param[out] _len The length of the returned Eina_Unicode string
 * @return The newly allocated Eina_Unicode string
 */
EAPI Eina_Unicode *eina_unicode_utf8_to_unicode(const char *utf, int *_len) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @brief Converts an Eina_Unicode string to a newly allocated UTF-8 string.
 *
 * @since 1.1.0
 *
 * @since_tizen 2.3
 *
 * @param[in] uni The Eina_Unicode string
 * @param[out] _len The length of the returned UTF-8 string in bytes
 * @return The newly allocated UTF-8 string
 */
EAPI char * eina_unicode_unicode_to_utf8(const Eina_Unicode *uni, int *_len) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * @}
 */

#endif

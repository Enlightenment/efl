#ifndef EINA_UNICODE_H
#define EINA_UNICODE_H

#include <stdlib.h>

#include "eina_config.h"
#include "eina_types.h"

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */
/**
 * @addtogroup Eina_Unicode_String Unicode String
 *
 * @brief These functions provide basic unicode string handling
 *
 * Eina_Unicode is a type that holds unicode codepoints.
 *
 * @{
 */

/**
 * @typedef Eina_Unicode
 * A type that holds Unicode codepoints.
 */
#if EINA_SIZEOF_WCHAR_T >= 4
# include <wchar.h>
typedef wchar_t Eina_Unicode;
#else
# include <inttypes.h>
typedef uint32_t Eina_Unicode;
#endif

/**
 * @brief Same as the standard strlen just with Eina_Unicode instead of char.
 */
EAPI extern const Eina_Unicode *EINA_UNICODE_EMPTY_STRING;

EAPI size_t        eina_unicode_strlen(const Eina_Unicode *ustr) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;

/**
 * @brief Returns the length of a Eina_Unicode string, up to a limit.
 *
 * This function returns the number of characters in string, up to a maximum
 * of n.  If the terminating character is not found in the string, it returns
 * n.
 *
 * @param ustr String to search
 * @param n Max length to search
 * @return Number of characters or n.
 */
EAPI size_t        eina_unicode_strnlen(const Eina_Unicode *ustr, int n) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT EINA_PURE;


/**
 * @brief Same as the standard strdup just with Eina_Unicode instead of char.
 */
EAPI Eina_Unicode *eina_unicode_strdup(const Eina_Unicode *text) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;


/**
 * @brief Same as strdup but cuts on the given size. Assumes n < len
 *
 * @param text The text to duplicate.
 * @param n The maximum size of the text to duplicate.
 * @return The duplicated string.
 *
 * This function duplicates @p text. The resuting string is cut on @p
 * n. @p n is assumed to be lesser  (<) than the length of @p
 * text. When not needed anymore, the returned string must be freed.
 *
 * @since 1.1.0
 */
EAPI Eina_Unicode *eina_unicode_strndup(const Eina_Unicode *text, size_t n) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;


/**
 * @brief Same as the standard strcmp just with Eina_Unicode instead of char.
 */
EAPI int           eina_unicode_strcmp(const Eina_Unicode *a, const Eina_Unicode *b) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;


/**
 * @brief Same as the standard strcpy just with Eina_Unicode instead of char.
 */
EAPI Eina_Unicode *eina_unicode_strcpy(Eina_Unicode *dest, const Eina_Unicode *source) EINA_ARG_NONNULL(1, 2);


/**
 * @brief Same as the standard strstr just with Eina_Unicode instead of char.
 */
EAPI Eina_Unicode *eina_unicode_strstr(const Eina_Unicode *haystack, const Eina_Unicode *needle) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1, 2) EINA_PURE;


/**
 * @brief Same as the standard strncpy just with Eina_Unicode instead of char.
 */
EAPI Eina_Unicode *eina_unicode_strncpy(Eina_Unicode *dest, const Eina_Unicode *source, size_t n) EINA_ARG_NONNULL(1, 2);


/**
 * @see eina_str_escape()
 *
 * @param str The string to escape.
 * @return The escaped string.
 */
EAPI Eina_Unicode *eina_unicode_escape(const Eina_Unicode *str) EINA_ARG_NONNULL(1) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/* UTF-8 Handling */


/**
 * Reads UTF8 bytes from @p buf, starting at @p iindex and returns
 * the decoded code point at @p iindex offset, and advances @p iindex
 * to the next code point after this. @p iindex is always advanced,
 * unless if the advancement is after the @c NULL.
 * On error: return a codepoint between DC80 to DCFF where the low 8 bits
 *   are the byte's value.
 *
 * @param buf the string
 * @param iindex the index to look at and return by.
 * @return the codepoint found, 0 if @p buf or @p iindex are NULL
 * @since 1.8.0
 */
static inline Eina_Unicode eina_unicode_utf8_next_get(const char *buf, int *iindex) EINA_ARG_NONNULL(1, 2);

/**
 * Reads UTF8 bytes from @p buf, starting at @p iindex and returns
 * the decoded code point at @p iindex offset, and advances @p iindex
 * to the next code point after this. @p iindex is always advanced,
 * unless if the advancement is after the @c NULL.
 * On error: return a codepoint between DC80 to DCFF where the low 8 bits
 *   are the byte's value.
 *
 * @param buf the string
 * @param iindex the index to look at and return by.
 * @return the codepoint found, 0 if @p buf or @p iindex are NULL
 * @since 1.1.0
 * @deprecated use eina_unicode_utf8_next_get
 */
EAPI Eina_Unicode eina_unicode_utf8_get_next(const char *buf, int *iindex) EINA_ARG_NONNULL(1, 2) EINA_DEPRECATED;

/**
 * Reads UTF8 bytes from @p buf, starting at @p iindex and returns
 * the decoded code point at @p iindex offset, and moves  àp iindex
 * to the previous code point. @p iindex is always moved, as long
 * as it's not past the start of the string.
 * On error: return a codepoint between DC80 to DCFF where the low 8 bits
 *   are the byte's value.
 *
 * @param buf the string
 * @param iindex the index to look at and return by.
 * @return the codepoint found.
 * @since 1.1.0
 */
EAPI Eina_Unicode eina_unicode_utf8_get_prev(const char *buf, int *iindex) EINA_ARG_NONNULL(1, 2);

/**
 * Returns the number of unicode characters in the string. That is,
 * the number of Eina_Unicodes it'll take to store this string in
 * an Eina_Unicode string.
 *
 * @param buf the string
 * @return the number of unicode characters (not bytes) in the string
 * @since 1.1.0
 */
EAPI int eina_unicode_utf8_get_len(const char *buf) EINA_ARG_NONNULL(1);

/**
 * Converts a utf-8 string to a newly allocated Eina_Unicode string.
 *
 * @param utf the string in utf-8
 * @param _len the length of the returned Eina_Unicode string.
 * @return the newly allocated Eina_Unicode string.
 * @since 1.1.0
 */
EAPI Eina_Unicode *eina_unicode_utf8_to_unicode(const char *utf, int *_len) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

/**
 * Converts an Eina_Unicode string to a newly allocated utf-8 string.
 *
 * @param uni the Eina_Unicode string
 * @param _len the length byte length of the return utf8 string.
 * @return the newly allocated utf-8 string.
 * @since 1.1.0
 */
EAPI char * eina_unicode_unicode_to_utf8(const Eina_Unicode *uni, int *_len) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_MALLOC;

#include "eina_inline_unicode.x"

/**
 * @}
 */
/**
 * @}
 */

#endif

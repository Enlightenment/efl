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
 * @brief Same as strdup but cuts on n. Assumes n < len
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
 */
EAPI Eina_Unicode *eina_unicode_escape(const Eina_Unicode *str) EINA_ARG_NONNULL(1) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/* UTF-8 Handling */


/**
 * Reads UTF8 bytes from @buf, starting at *@index and returns
 * the decoded code point at iindex offset, and advances iindex
 * to the next code point after this. iindex is always advanced,
 * unless if the advancement is after the NULL.
 * On error: return a codepoint between DC80 to DCFF where the low 8 bits
 *   are the byte's value.
 *
 * @param buf the string
 * @param iindex the index to look at and return by.
 * @return the codepoint found.
 * @since 1.1.0
 */
EAPI Eina_Unicode eina_unicode_utf8_get_next(const char *buf, int *iindex) EINA_ARG_NONNULL(1, 2);

/**
 * Reads UTF8 bytes from @buf, starting at *@iindex and returns
 * the decoded code point at iindex offset, and moves iindex
 * to the previous code point. iindex is always moved, as long
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

/**
 * @}
 */
/**
 * @}
 */

#endif

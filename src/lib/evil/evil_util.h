#ifndef __EVIL_UTIL_H__
#define __EVIL_UTIL_H__


/**
 * @brief Convert a string from char * to wchar_t *.
 *
 * @param text The string to convert.
 * @return The converted string.
 *
 * Convert a string from char * to wchar_t * and return it. If the
 * allocation or conversion fails, NULL is returned. On success, the
 * returned value must be freed when it is not used anymore.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EVIL_API wchar_t *evil_char_to_wchar(const char *text);

/**
 * @brief Convert a string from wchar_t * to char *.
 *
 * @param text The string to convert.
 * @return The converted string.
 *
 * Convert a string from wchar_t * to char * and return it. If the
 * allocation or conversion fails, NULL is returned. On success, the
 * returned value must be freed when it is not used anymore.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EVIL_API char *evil_wchar_to_char(const wchar_t *text);

/**
 * @brief Convert a string from UTF-16 to UTF-8.
 *
 * @param text The string to convert in UTF-16.
 * @return The converted string in UTF-8.
 *
 * Convert a string from UTF-16 to UTF-8 and return it. If the
 * allocation or conversion fails, NULL is returned. On success, the
 * returned value must be freed when it is not used anymore.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EVIL_API char *evil_utf16_to_utf8(const wchar_t *text);

/**
 * @brief Convert a string from UTF-8 to UTF-16.
 *
 * @param text The string to convert in UTF-8.
 * @return The converted string in UTF-16.
 *
 * Convert a string from UTF-8 to UTF-16 and return it. If the
 * allocation or conversion fails, NULL is returned. On success, the
 * returned value must be freed when it is not used anymore.
 *
 * Conformity: Non applicable.
 *
 * @since 1.24
 *
 * @ingroup Evil
 */
EVIL_API wchar_t *evil_utf8_to_utf16(const char *text);

EVIL_API const char *evil_format_message(long err);

EVIL_API const char *evil_last_error_get(void);

#endif /* __EVIL_UTIL_H__ */

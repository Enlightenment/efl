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
 * returned value must be freed.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI wchar_t *evil_char_to_wchar(const char *text);

/**
 * @brief Convert a string from wchar_t * to char *.
 *
 * @param text The string to convert.
 * @return The converted string.
 *
 * Convert a string from wchar_t * to char * and return it. If the
 * allocation or conversion fails, NULL is returned. On success, the
 * returned value must be freed.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI char *evil_wchar_to_char(const wchar_t *text);

EAPI char *evil_format_message(long err);

EAPI char *evil_last_error_get(void);

/**
 * @brief Return a dir to store temporary files.
 *
 * @return The directory to store temporary files.
 *
 * Return a directory to store temporary files. The function gets
 * the value of the following environment variables, and in that order:
 * - TMP
 * - TEMP
 * - USERPROFILE
 * - WINDIR
 * and returns its value if it exists. If none exists, the function
 * returns "C:\".
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI const char *evil_tmpdir_get(void);

/**
 * @brief Return a dir to store personal files.
 *
 * @return The directory to store personal files.
 *
 * Return a directory to store personal files. The function gets
 * the value of the following environment variables, and in that order:
 * - HOME
 * - USERPROFILE
 * - WINDIR
 * and returns its value if it exists. If none exists, the function
 * returns "C:\".
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI const char *evil_homedir_get(void);


#endif /* __EVIL_UTIL_H__ */

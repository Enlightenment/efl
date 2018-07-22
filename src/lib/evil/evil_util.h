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
EAPI wchar_t *evil_char_to_wchar(const char *text);

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
EAPI char *evil_wchar_to_char(const wchar_t *text);

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
EAPI char *evil_utf16_to_utf8(const wchar_t *text);

EAPI const char *evil_format_message(long err);

EAPI const char *evil_last_error_get(void);

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

 * @deprecated
 * This function is deprecated and returns @c NULL.
 * Use eina_environment_tmp_get() instead.
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

 * @deprecated
 * This function is deprecated and returns @c NULL.
 * Use eina_environment_home_get() instead.
 *
 * @ingroup Evil
 */
EAPI const char *evil_homedir_get(void);

/**
 * @brief check if the given path is absolute.
 *
 * @param path The path to check.
 * @return 1 if the given path is absolute, 0 otherwise.
 *
 * Check if the path @p path is absolute or not. An absolute path must
 * begin with a letter (upper or lower case), followed by by the char
 * ':', followed by the char '/' or '\'. If @p path is absolute this
 * function returns 1, otherwise it returns 0. If @p path is @c NULL,
 * it returns 0.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @since 1.7
 *
 * @ingroup Evil
 */
EAPI int evil_path_is_absolute(const char *path);

#endif /* __EVIL_UTIL_H__ */

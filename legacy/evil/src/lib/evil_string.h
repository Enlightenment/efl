#ifndef __EVIL_STRING_H__
#define __EVIL_STRING_H__


/**
 * @file evil_string.h
 * @brief The file that provides functions ported from Unix in string.h.
 * @defgroup Evil_String_Group String.h functions.
 *
 * This header provides functions ported from Unix in string.h.
 *
 * @{
 */


#ifdef _WIN32_WCE

/*
 * Environment variable related functions
 *
 */

/**
 * @brief Return the static string "[Windows CE] error\n".
 *
 * @param errnum Unused parameter.
 * @return The static string "[Windows CE] error\n".
 *
 * This function just returns the static string "[Windows CE]
 * error\n".
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows CE (not cegcc).
 */
EAPI char *strerror (int errnum);

#endif /* _WIN32_WCE */

/*
 * bit related functions
 *
 */

/**
 * @brief Return the position of the first (least significant) bit set in a word
 *
 * @param i Word to take the first bit.
 * @return The position of the first bit set, or 0 if no bits are set.
 *
 * This function returns the position of the first (least significant)
 * bit set in @p i. The least significant bit is position 1 and the
 * most significant position e.g. 32 or 64. The function returns 0 if
 * no bits are set in @p i, or the position of the first bit set
 * otherwise.
 *
 * Conformity: BSD
 *
 * Supported OS: Windows XP, Windows CE (not cegcc).
 */
EAPI int ffs(int i);


#ifdef _WIN32_WCE

/*
 * String manipulation related functions
 *
 */

/**
 * @brief Compare two strings.
 *
 * @param s1 The first string to compare.
 * @param s2 The second string to compare.
 * @return < 0 if s1 < s2, >0 if s1 > s2, 0 otherwise.
 *
 * This function is exactly the same as strcmp(). No possible way to
 * achieve the behavior of strcoll() on Windows CE.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows CE.
 */
EAPI int strcoll (const char *s1, const char *s2);

#endif /* _WIN32_WCE */

/**
 * @brief Get the last substring occurence.
 *
 * @param str The string to search from.
 * @param substr The substring to search.
 * @return The last occurrence of the substring if found, @c NULL otherwise.
 *
 * This function retrieves the last occurrence of @p substring in the
 * string @p str. If @p str or @p substr are @c NULL, of if @p substr
 * is not found in @p str, @c NULL is returned.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows XP, Windows CE.
 */
EAPI char *strrstr (const char *str, const char *substr);

#ifdef _MSC_VER

/**
 * @brief Compare two string, ignoring case.
 *
 * @param s1 The first string to compare.
 * @param s2 The first string to compare.
 * @return
 *
 * This function compares the two strings @p s1 and @p s2, ignoring
 * the case of the characters. It returns an integer less than, equal
 * to, or greater than zero if s1 is found, respectively, to be less
 * than, to match, or be greater than s2.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows XP (vc++ only)
 */
EAPI int strcasecmp(const char *s1, const char *s2);

#endif /* _MSC_VER */


/**
 * @}
 */


#endif /* __EVIL_STRING_H__ */

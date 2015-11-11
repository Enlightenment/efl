#ifndef __EVIL_STRING_H__
#define __EVIL_STRING_H__


/**
 * @file evil_string.h
 * @brief The file that provides functions ported from Unix in string.h.
 * @defgroup Evil_String_Group String.h functions.
 * @ingroup Evil
 *
 * This header provides functions ported from Unix in string.h.
 *
 *
 */


/*
 * bit related functions
 *
 */
/**
 * @brief Duplicate a string
 *
 * @param str String to be duplicated
 * @param n size of new duplicated string
 * @return The strndup() function returns a pointer to the duplicated string, or NULL if insufficient memory was available.
 *
 * This function returns a pointer to a new string which is a duplicate of the string str, 
 * but only copies at most n bytes. If str is longer than n, only n bytes are copied,
 * and a terminating null byte ('\0') is added.
 *
 * Conformity: BSD
 *
 * Supported OS: Windows XP.
 *
 * @since 1.13
 */
EAPI char *strndup(const char *str, size_t n);

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
 * Supported OS: Windows XP.
 */
EAPI int ffs(int i);


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
 * Supported OS: Windows XP.
 */
EAPI char *strrstr (const char *str, const char *substr);

/**
 * @brief Locate a substring into a string, ignoring case.
 *
 * @param haystack The string to search in.
 * @param needle The substring to find.
 * @return
 *
 * This function locates the string @p needle into the string @p haystack,
 * ignoring the case of the characters. It returns apointer to the
 * beginning of the substring, or NULL if the substring is not found.
 * If @p haystack or @p needle are @c NULL, this function returns @c NULL.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows XP.
 */
EAPI char *strcasestr(const char *haystack, const char *needle);

/**
 * @brief Implements the strsep function which is used to separate strings.
 *
 * @param stringp The pointer to the string to search in.
 * @param delim The delimiter that contains characters used to find the next token.
 * @return a pointer to the next token or NULL;
 *
 * The strsep() function locates, in the string referenced by *stringp, the
 * first occurrence of any character in the string delim (or the terminating
 * `\0' character) and replaces it with a `\0'.  The location of the next
 * character after the delimiter character (or NULL, if the end of the
 * string was reached) is stored in *stringp.  The original value of
 * stringp is returned.
 *
 * An ``empty'' field (i.e., a character in the string delim occurs as the
 * first character of *stringp) can be detected by comparing the location
 * referenced by the returned pointer to `\0'.

 * If *stringp is initially NULL, strsep() returns NULL.
 *
 * This function is from LibGW32C.
 * @since 1.8
 *
 */
EAPI char *strsep(char **stringp, const char *delim);

#endif /* __EVIL_STRING_H__ */

#ifndef __EVIL_LOCALE_H__
#define __EVIL_LOCALE_H__


/**
 * @file evil_locale.h
 * @brief The file that provides functions ported from Unix in locale.h.
 * @defgroup Evil_Locale_Group locale.h functions.
 * @ingroup Evil
 *
 * This header provides functions ported from Unix in locale.h.
 *
 * @{
 */


/**
 * @def LC_MESSAGES
 *
 * New locale value, based on the one in libintl.h
 */
#ifdef LC_MESSAGES
# undef LC_MESSAGES
#endif
#define LC_MESSAGES 1729

/**
 * @brief Return the string associated to the given locale and category.
 *
 * @param category The category affected by locale.
 * @param locale The locale specifier.
 * @return The string associated to the specified locale and category.
 *
 * This function returns the string associated to @p locale and
 * @p category. If @p category is LC_ALL, LC_COLLATE, LC_CTYPE,
 * LC_MONETARY, LC_NUMERIC or LC_TIME, it just returns the standard
 * setlocale() function. If @p category is #LC_MESSAGES, then if @p locale
 * is not @c NULL, errno is set to EINVAL and @c NULL is returned, otherwise
 * the string <language>_<country> is returned. This string is a static buffer
 * and must not be freed. It will also be rewritten each time @category is
 * #LC_MESSAGES and @p locale is @c NULL.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows XP.
 */
EAPI char *evil_setlocale(int category, const char *locale);

/**
 * @def setlocale(cat, loc)
 *
 * Wrapper around evil_setlocale().
 */
#ifdef setlocale /* libintl.h defines setlocale() but always returns "C" */
# undef setlocale
#endif
#define setlocale(cat, loc) evil_setlocale(cat, loc)


/**
 * @}
 */


#endif /*__EVIL_LOCALE_H__ */

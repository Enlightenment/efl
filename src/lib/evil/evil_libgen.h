#ifndef __EVIL_LIBGEN_H__
#define __EVIL_LIBGEN_H__


/**
 * @file evil_libgen.h
 * @brief The file that provides functions ported from Unix in libgen.h.
 * @defgroup Evil_Libgen_Group Libgen.h functions.
 *
 * This header provides functions ported from Unix in libgen.h.
 *
 * @{
 */

/**
 * @brief Parse the base name component of a path.
 *
 * @param path The path to parse.
 * @return The component following the final '/'.
 *
 * This function parses @p path and returns its component following
 * the final '\'. Trailing '\' are not taken into account. On Windows
 * XP, @p path must beginning by a drive letter followed by ':/' or
 * ':\', otherwise "C:\" is returned. All characters '/' are replaced by '\'. On
 * error (memory allocation failure), "C:\" is returned, otherwise the
 * component following the final '\' is returned as a statically
 * allocated memory. Hence the returns value must not be freed.
 *
 * Concatenating the string returned by dirname(), a "\", and the
 * string returned by basename() yields a complete pathname.
 *
 * @see evil_dirname()
 * @see dirname()
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows XP.
 */
EAPI char *evil_basename(char *path);

/**
 * @def basename(p)
 *
 * Wrapper around evil_basename().
 */
#define basename(p) evil_basename(p)

/**
 * @brief Parse the dir name component of a path.
 *
 * @param path The path to parse.
 * @return The component up to, but not including, the final '/'.
 *
 * This function parses @p path and returns its component up to, but
 * not including, the final '/'. Trailing '\' are not taken into
 * account. On Windows XP, @p path must beginning by a drive letter
 * followed by ':/' or ':\', otherwise "C:\" is returned. All
 * characters '/' are replaced by '\'. On error (memory allocation
 * failure), "C:\" is returned, otherwise,  the component up to, but
 * not including, the final '/' is returned as a statically allocated
 * memory. Hence the returns value must not be freed.
 *
 * Concatenating the string returned by dirname(), a "\", and the
 * string returned by basename() yields a complete pathname.
 *
 * @see evil_basename()
 * @see basename()
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows XP.
 */
EAPI char *evil_dirname(char *path);

/**
 * @def dirname(p)
 *
 * Wrapper around evil_dirname().
 */
#define dirname(p) evil_dirname(p)


/**
 * @}
 */


#endif /* __EVIL_LIBGEN_H__ */

#ifndef __EVIL_STDLIB_H__
#define __EVIL_STDLIB_H__


/**
 * @file evil_stdlib.h
 * @brief The file that provides functions ported from Unix in stdlib.h.
 * @defgroup Evil_Stdlib_Group Stdlib.h functions.
 * @ingroup Evil
 *
 * This header provides functions ported from Unix in stdlib.h.
 *
 * @{
 */


/*
 * Environment variable related functions
 *
 */

/**
 * @brief Create, modify, or remove environment variables.
 *
 * @param name The name of the environment variable.
 * @param value The value of the environment variable to set.
 * @param overwrite 0 to let the environment variable unchanged, 1 otherwise.
 * @return 0 on success, -1 otherwise.
 *
 * Add the new environment variable @p name or modify its value if it
 * exists, and set it to @p value. Environment variables define the
 * environment in which a process executes. If @p value is @c NULL, the
 * variable is removed (unset) and that call is equivalent to
 * unsetenv().If the environment variable named by @p name already
 * exists and the value of @p overwrite is 0, the function shall
 * return success and the environment shall remain unchanged.
 * If the function succeeds, it returns 0, otherwise it returns -1.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows XP.
 */
EAPI int setenv(const char *name,
                const char *value,
                int         overwrite);

/**
 * @brief Remove environment variables.
 *
 * @param name The name of the environment variable.
 * @return 0 on success, -1 otherwise.
 *
 * Remove the new environment variable @p name if it exists. That
 * function is equivalent to setenv() with its second parameter to
 * @c NULL and the third to 1. If the function succeeds, it returns 0,
 * otherwise it returns -1.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows XP.
 */
EAPI int unsetenv(const char *name);


/*
 * Files related functions
 *
 */

/**
 * @brief create an unique temporary directory
 *
 * @since 1.8.0
 */
EAPI char *mkdtemp(char *__template);

/**
 * @brief Create a unique temporary file name with a suffix.
 *
 * @param __template Template of the file to create.
 * @param suffixlen Length of the suffix following the 'XXXXXX' placeholder.
 * @return A file descriptor on success, -1 otherwise.
 *
 * @since 1.10.0
 */
EAPI int mkstemps(char *__template, int suffixlen);

/**
 * @brief Return an absolute or full path name for a specified relative path name.
 *
 * @param file_name The absolute path name.
 * @param resolved_name The relative path name.
 * @return @c NULL on failure, a pointer to the absolute path name otherwise.
 *
 * The function expands the relative path name @p file_name to its
 * fully qualified or absolute path and store it in the buffer pointed
 * by @p resolved_name. The buffer is at most @c PATH_MAX bytes long.
 * If @p resolved_name is @c NULL, malloc() is used to allocate a
 * buffer of sufficient length to hold the path name. In that case, it
 * is the responsibility of the caller to free this buffer with free().
 *
 * That function can be used to obtain the absolute path name for
 * relative paths (relPath) that include "./" or "../" in their names.
 *
 * On Windows XP, errno is set in the following cases:
 *
 * @li EACCESS: if @p file_name can not be accessed.
 * @li EINVAL: if @p file_name is @c NULL.
 * @li ENAMETOOLONG: if the path name is too long.
 * @li ENOENT: @p file_name does not exist
 * @li ENOMEM: if memory allocation fails.
 *
 * Conformity: None.
 *
 * Supported OS: Windows XP.
 */
EAPI char *realpath(const char *file_name, char *resolved_name);


/**
 * @}
 */


#endif /* __EVIL_STDLIB_H__ */


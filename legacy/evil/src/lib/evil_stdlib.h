#ifndef __EVIL_STDLIB_H__
#define __EVIL_STDLIB_H__


/*
 * Environment variable related functions
 *
 */

#if defined(__CEGCC__) || defined(__MINGW32CE__)

EAPI char *getenv(const char *name);

#endif /* __CEGCC__ || __MINGW32CE__ */


#if ! defined(__CEGCC__)

EAPI int putenv(const char *string);

#endif /* ! __CEGCC__ */

/**
 * @brief Create, modify, or remove environment variables.
 *
 * @param name The name of the environment variable.
 * @param value The value of the environment variable to set.
 * @return 0 on success, -1 otherwise.
 *
 * Add the new environment variable @p name or modify its value if it
 * exists, and set it to @p value. Environment variables define the
 * environment in which a process executes. If @p value is @c NULL, the
 * variable is removed (unset) and that call is equivalent to unsetenv().
 * If the function succeeds, it returns 0, otherwise it returns -1.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI int setenv(const char *name,
                const char *value,
                int         overwrite);


#if ! defined(__CEGCC__)

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
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI int unsetenv(const char *name);

#endif /* ! __CEGCC__ */


/*
 * Files related functions
 *
 */

/**
 * @brief Make temporay unique file name.
 *
 * @param template Template of the file to create.
 * @return A file descriptor on success, -1 otherwise.
 *
 * Take the given file name @p template and overwrite a portion of it
 * to create a file name. This file is guaranted not to exist at the
 * time invocation and is suitable for use by the function.
 *
 * The @p template parameter can be any file name with some number of
 * 'Xs' appended to it, for example @em baseXXXXXX, where @em base is
 * the part of the new file that you supply and eacg 'X' is a placeholder
 * for a character supplied by mkstemp(). The trailing 'Xs' are replaced
 * with a five-digit value; this value is a unique number. Each successful
 * call to mkstemp() modifes @p template.
 *
 * When mkstemp() succeeds, it creates and opens the template file for
 * reading and writing.
 *
 * On success, the function returns the file descriptor of the
 * temporary file. Otherwise, it returns -1 and errno is set to the
 * following values:
 * - EINVAL: @p template has an invalid format.
 * - EACCESS: Given path is a directory, or file is read-only, but an
 * open-for-writing operation was attempted.
 * - EEXISTS: File name already exists.
 * - EMFILE: No more file descriptors available.
 * - ENOENT: File or path not found.
 *
 * Conformity: Should follow BSD conformity.
 *
 * Supported OS: Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI int mkstemp(char *template);

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
 * is the responsability of the caller to free this buffer with free().
 *
 * That function can be used to obtain the absolute path name for
 * relative paths (relPath) that include "./" or "../" in their names.
 *
 * Conformity: None.
 *
 * Supported OS: Windows 95, Windows 98, Windows Me, Windows NT, Windows 2000,
 * Windows XP.
 *
 * @ingroup Evil
 */
EAPI char *realpath(const char *file_name, char *resolved_name);


#endif /* __EVIL_STDLIB_H__ */


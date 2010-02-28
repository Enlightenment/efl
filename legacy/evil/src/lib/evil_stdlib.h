#ifndef __EVIL_STDLIB_H__
#define __EVIL_STDLIB_H__


/**
 * @file evil_stdlib.h
 * @brief The file that provides functions ported from Unix in stdlib.h.
 * @defgroup Evil_Stdlib_Group Stdlib.h functions.
 *
 * This header provides functions ported from Unix in stdlib.h.
 *
 * @{
 */


/*
 * Environment variable related functions
 *
 */

#ifdef _WIN32_WCE

/**
 * @brief Retrieve the value of environment variables.
 *
 * @param name The name of the environment variable.
 * @return The value of the environment variable.
 *
 * This function searches the environment variable @p name if it
 * exists and return a pointer to the value of the environment
 * variable. If the specified environment variable cannot be found,
 * @c NULL is returned.
 *
 * The returned value may be overwritten by a subsequent call to
 * getenv(), setenv(), or unsetenv().
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows CE.
 *
 * @note On Windows CE, there is no environment variable. This is
 * faked by storing a value in a key in the base registry.
 */
EAPI char *getenv(const char *name);

#endif /* _WIN32_WCE */


#ifdef __MINGW32CE__

/**
 * @brief Set the value of environment variables.
 *
 * @param string A formatted string.
 * @return 0 in success, non-zero otherwise.
 *
 * This function uses @p string to set environment variable values.
 * @p string should point to a string of the form "name=value". This
 * function makes the value of the environment variable name equal to
 * value by altering an existing variable or creating a new one. In
 * either case, the string pointed to by @p string becomes part of the
 * environment, so altering @p string shall change the environment.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows CE.
 *
 * @note On Windows CE, there is no environment variable. This is
 * faked by storing a value in a key in the base registry.
 */
EAPI int putenv(const char *string);

#endif /* __MINGW32CE__ */

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
 * Supported OS: Windows XP, Windows CE.
 *
 * @note On Windows CE, there is no environment variable. This is
 * faked by storing a value in a key in the base registry.
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
 * Supported OS: Windows XP, Windows CE (not cegcc).
 *
 * @note On Windows CE, there is no environment variable. This is
 * faked by storing a value in a key in the base registry.
 */
EAPI int unsetenv(const char *name);


/*
 * Files related functions
 *
 */

/**
 * @brief Make temporay unique file name.
 *
 * @param __template Template of the file to create.
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
 * - EEXISTS: File name already exists.
 *
 * Conformity: Should follow BSD conformity.
 *
 * Supported OS: Windows XP, Windows CE.
 */
EAPI int mkstemp(char *__template);

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
 * Supported OS: Windows XP, Windows CE.
 */
EAPI char *realpath(const char *file_name, char *resolved_name);


/**
 * @}
 */


#endif /* __EVIL_STDLIB_H__ */


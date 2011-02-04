#ifndef __EVIL_STDIO_H__
#define __EVIL_STDIO_H__


/**
 * @file evil_stdio.h
 * @brief The file that provides functions ported from Unix in stdio.h.
 * @defgroup Evil_Stdio_Group Stdio.h functions
 *
 * This header provides functions ported from Unix in stdio.h.
 *
 * @{
 */


#ifdef _WIN32_WCE

/*
 * Error related functions
 *
 */

EAPI void evil_perror (const char *s);

# define perror(s) evil_perror(s)

/*
 * Stream related functions
 *
 */

EAPI FILE *evil_fopen(const char *path, const char *mode);

# define fopen(path, mode) evil_fopen(path, mode)

EAPI void evil_rewind(FILE *stream);

# define rewind(f) evil_rewind(f)

EAPI int evil_remove(const char *path);

# define remove(p) evil_remove(p)

#endif /* _WIN32_WCE */


#ifdef _WIN32_WCE

EAPI FILE *evil_fopen_native(const char *path, const char *mode);

EAPI size_t evil_fread_native(void* buffer, size_t size, size_t count, FILE* stream);

EAPI int evil_fclose_native(FILE *stream);

#endif /* _WIN32_WCE */

/**
 * @brief Print allocated string using a va_list.
 *
 * @param strp The returned pointer of the allocated string.
 * @param fmt The format string.
 * @param ap The variable argument list.
 * @return -1 on failure, the length of the printed string.
 *
 * This function allocates a buffer large enough to hold the output
 * including the terminating null byte, and return a pointer to it
 * into @p strp. The format @p fmt is the same than the one used with
 * printf(). When not needed anymore, the pointer returned in @p strp
 * must be freed. On error (memory allocation failure or other error),
 * this function returns -1 and the content of @p strp is undefined,
 * otherwise it returns the length of the string (not including the
 * terminating null byte).
 *
 * Conformity: GNU extension.
 *
 * Supported OS: Windows XP, CE.
 */
EAPI int vasprintf(char **strp, const char *fmt, va_list ap);

/**
 * @brief Print allocated string using a variable number of arguments.
 *
 * @param strp The returned pointer of the allocated string.
 * @param fmt The format string.
 * @return -1 on failure, the length of the printed string.
 *
 * This function allocates a buffer large enough to hold the output
 * including the terminating null byte, and return a pointer to it
 * into @p strp. The format @p fmt is the same than the one used with
 * printf(). When not needed anymore, the pointer returned in @p strp
 * must be freed. On error (memory allocation failure or other error),
 * this function returns -1 and the content of @p strp is undefined,
 * otherwise it returns the length of the string (not including the
 * terminating null byte).
 *
 * Conformity: GNU extension.
 *
 * Supported OS: Windows XP, CE.
 */
EAPI int asprintf(char **strp, const char *fmt, ...);


/**
 * @}
 */


#endif /* __EVIL_STDIO_H__ */

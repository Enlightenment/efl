#ifndef __EVIL_STDIO_H__
#define __EVIL_STDIO_H__


#if defined (_WIN32_WCE) && ! defined (__CEGCC__)


# include <stdio.h>

/*
 * Error related functions
 *
 */

EAPI void perror (const char *s);

/*
 * Stream related functions
 *
 */

EAPI FILE *evil_fopen(const char *path, const char *mode);

# define fopen(path, mode) evil_fopen(path, mode)

EAPI void rewind(FILE *stream);

#endif /* _WIN32_WCE && ! __CEGCC__ */


#endif /* __EVIL_STDIO_H__ */

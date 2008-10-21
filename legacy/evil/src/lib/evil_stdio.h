#ifndef __EVIL_STDIO_H__
#define __EVIL_STDIO_H__


#ifdef __MINGW32CE__


#include <stdio.h>

/*
 * Error related functions
 *
 */

EAPI void perror (const char *s);

/*
 * Stream related functions
 *
 */

EAPI void rewind(FILE *stream);

#endif /* __MINGW32CE__ */


#endif /* __EVIL_STDIO_H__ */

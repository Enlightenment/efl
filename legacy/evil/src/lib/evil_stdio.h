#ifndef __EVIL_STDIO_H__
#define __EVIL_STDIO_H__


#include <stdio.h>


#ifdef __MINGW32CE__

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

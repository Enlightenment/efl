#ifndef _EET_PRIVATE_H
#define _EET_PRIVATE_H

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#if HAVE_ALLOCA_H
#include <alloca.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#elif __MINGW32__
#include <winsock.h>
#endif

#include <zlib.h>
#include <string.h>
#include <fnmatch.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <locale.h>

#ifdef __GNUC__
# if __GNUC__ >= 4
// BROKEN in gcc 4 on amd64
//#  pragma GCC visibility push(hidden)
# endif
#endif

FILE *_eet_memfile_read_open(const void *data, size_t size);
void  _eet_memfile_read_close(FILE *f);
FILE *_eet_memfile_write_open(void **data, size_t *size);
void  _eet_memfile_write_close(FILE *f);
void  _eet_memfile_shutdown();

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#endif

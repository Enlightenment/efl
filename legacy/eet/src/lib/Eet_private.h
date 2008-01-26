#ifndef _EET_PRIVATE_H
#define _EET_PRIVATE_H

#include <config.h>

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif
#include <ctype.h>

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
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
void  _eet_memfile_shutdown(void);
int   _eet_hash_gen(const char *key, int hash_size);
int   _eet_string_to_double_convert(const char *src, long long *m, long *e);
void  _eet_double_to_string_convert(char *des, double d);

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#endif

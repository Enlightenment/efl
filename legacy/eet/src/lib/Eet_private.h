#ifndef _EET_PRIVATE_H
#define _EET_PRIVATE_H

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <zlib.h>
#include <string.h>
#include <fnmatch.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <locale.h>

FILE *_eet_memfile_read_open(void *data, size_t size);
void  _eet_memfile_read_close(FILE *f);
FILE *_eet_memfile_write_open(void **data, size_t *size);
void  _eet_memfile_write_close(FILE *f);
void  _eet_memfile_shutdown();

#ifndef PATH_MAX
#define PATH_MAX 4095
#endif

#endif

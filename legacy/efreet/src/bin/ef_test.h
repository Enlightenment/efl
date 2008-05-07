#ifndef EF_TEST
#define EF_TEST

#include "config.h"

#if HAVE___ATTRIBUTE__
#define __UNUSED__ __attribute__((unused))
#else
#define __UNUSED__
#endif

#define IF_FREE(x) do { if (x) free(x); x = NULL; } while (0);
#define NEW(x, c) calloc(c, sizeof(x))

#endif

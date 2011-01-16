#ifndef EF_TEST
#define EF_TEST

#include "config.h"

#include <eina_types.h>

#define IF_FREE(x) do { if (x) free(x); x = NULL; } while (0);
#define NEW(x, c) calloc(c, sizeof(x))

#endif

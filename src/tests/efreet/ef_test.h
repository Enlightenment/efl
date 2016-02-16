#ifndef EF_TEST
#define EF_TEST

#include <eina_types.h>

#define IF_FREE(x) do { if (x) free(x); x = NULL; } while (0);
#define NEW(x, c) calloc(c, sizeof(x))

const char *ef_test_path_get(const char *component);

#endif

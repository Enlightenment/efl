#ifndef _ECORE_DESKTOP_PRIVATE_H
# define _ECORE_DESKTOP_PRIVATE_H

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <Ecore_File.h>

#define E_FN_DEL(_fn, _h) if (_h) { _fn(_h); _h = NULL; }
#define E_REALLOC(p, s, n) p = (s *)realloc(p, sizeof(s) * n)
#define E_NEW(s, n) (s *)calloc(n, sizeof(s))
#define E_NEW_BIG(s, n) (s *)malloc(n * sizeof(s))
#define E_FREE(p) { if (p) {free(p); p = NULL;} }

# ifdef __cplusplus
extern              "C"
{
# endif

# ifdef __cplusplus
}
# endif

#endif

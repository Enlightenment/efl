#ifndef E_MEM_H
#define E_MEM_H 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MEMCPY(src, dst, type, num) memcpy(dst, src, sizeof(type) * (num))

#define NEW(dat, num) malloc(sizeof(dat) * (num))
#define NEW_PTR(num) malloc(sizeof(void *) * (num))
#define FREE(dat) {free(dat); dat = NULL;}
#define IF_FREE(dat) {if (dat) FREE(dat);}
#define REALLOC(dat, type, num) {if (dat) dat = realloc(dat, sizeof(type) * (num)); else dat = malloc(sizeof(type) * (num));}
#define REALLOC_PTR(dat, num) {if (dat) dat = realloc(dat, sizeof(void *) * (num)); else dat = malloc(sizeof(void *) * (num));}

#endif

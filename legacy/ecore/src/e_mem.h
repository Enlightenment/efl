#ifndef E_MEM_H
#define E_MEM_H 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MEMCPY(src, dst, type, num) memcpy(dst, src, sizeof(type) * (num))

#if 1
#define NEW(dat, num) malloc(sizeof(dat) * (num))
#define NEW_PTR(num) malloc(sizeof(void *) * (num))
#define FREE(dat) {if (dat) {free(dat); dat = NULL;} else {printf("eek - NULL free\n");sleep(30);}}
#define IF_FREE(dat) {if (dat) FREE(dat);}
#define REALLOC(dat, type, num) {if (dat) dat = realloc(dat, sizeof(type) * (num)); else dat = malloc(sizeof(type) * (num));}
#define REALLOC_PTR(dat, num) {if (dat) dat = realloc(dat, sizeof(void *) * (num)); else dat = malloc(sizeof(void *) * (num));}

#else
static void        *
_NEW(int size, int num)
{
   void               *dat;

   dat = malloc(size * num);
   printf("NEW(%i) = %p\n", size * num, dat);
   return dat;
}
static void        *
_NEW_PTR(int num)
{
   void               *dat;

   dat = malloc(sizeof(void *) * num);
   printf("NEW(%i) = %p\n", sizeof(void *) * num, dat);

   return dat;
}
static void
_IF_FREE(void *dat)
{
   printf("IF_FREE(%p)\n", dat);
   if (dat)
      free(dat);
}
static void
_FREE(void *dat)
{
   printf("FREE(%p)\n", dat);
   free(dat);
}
#define NEW(dat, num) _NEW(sizeof(dat), (num))
#define NEW_PTR(num) _NEW(sizeof(void *), (num))
#define IF_FREE(dat) {_IF_FREE(dat);}
#define FREE(dat) {_FREE(dat);}

#define REALLOC(dat, type, num) {dat = realloc(dat, sizeof(type) * (num)); printf("REALLOC(%i) = %p\n", sizeof(type) * (num), dat);}
#define REALLOC_PTR(dat, num) {dat = realloc(dat, sizeof(void *) * (num)); printf("REALLOC_PTR(%i) = %p\n", sizeof(void *) * (num), dat);}

#endif
#endif

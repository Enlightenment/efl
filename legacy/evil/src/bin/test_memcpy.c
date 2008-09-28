
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "suite.h"
#include "test_memcpy.h"


typedef void *(*memcpy_decl)(void *dest, const void *src, size_t n);

void *memcpy_glibc(void *dest, const void *src, size_t n);


extern unsigned char *buf1;
extern unsigned char *buf2;
extern size_t         page_size;


static void
test_memcpy_test_run(memcpy_decl fct, char *dst, const char *src, size_t len)
{
   double start;
   double end;
   double best;
   int    i;

   best = 1000000000.0;

   for (i = 0; i < 32; ++i)
     {
        double time;

        suite_time_start();
        fct(dst, src, len);
        suite_time_stop();
        time = suite_time_get();
        if (time < best) best = time;
     }

   printf ("  %e", best);
}

static void
test_memcpy_tests_run(size_t align1, size_t align2, size_t len)
{
   size_t i, j;
   char *s1, *s2;

   printf ("running test..\n");

/*    align1 &= 63; */
/*    if (align1 + len >= page_size) */
/*      return; */

/*    align2 &= 63; */
/*    if (align2 + len >= page_size) */
/*      return; */

   s1 = (char *) (buf1 + align1);
   s2 = (char *) (buf2 + align2);

   for (i = 0, j = 1; i < len; i++, j += 23)
     s1[i] = j;

   printf ("length: %6d, align %2d/%2d:", (int)len, align1, align2);

   test_memcpy_test_run(memcpy, s2, s1, len);
#ifdef EVIL_HAVE_WINCE
   test_memcpy_test_run(memcpy_glibc, s2, s1, len);
#endif /* EVIL_HAVE_WINCE */

   printf ("\n");
}

void
test_memcpy(void)
{
   size_t i;

/*   for (i = 0; i < 18; ++i) */
/*     { */
/*       test_memcpy_tests_run(0, 0, 1 << i); */
/*       test_memcpy_tests_run(i, 0, 1 << i); */
/*       test_memcpy_tests_run(0, i, 1 << i); */
/*       test_memcpy_tests_run(i, i, 1 << i); */
/*     } */

/*   for (i = 0; i < 32; ++i) */
/*     { */
/*       test_memcpy_tests_run(0, 0, i); */
/*       test_memcpy_tests_run(i, 0, i); */
/*       test_memcpy_tests_run(0, i, i); */
/*       test_memcpy_tests_run(i, i, i); */
/*     } */

/*   for (i = 3; i < 32; ++i) */
/*     { */
/*       if ((i & (i - 1)) == 0) */
/* 	continue; */
/*       test_memcpy_tests_run(0, 0, 16 * i); */
/*       test_memcpy_tests_run(i, 0, 16 * i); */
/*       test_memcpy_tests_run(0, i, 16 * i); */
/*       test_memcpy_tests_run(i, i, 16 * i); */
/*     } */

  test_memcpy_tests_run(0, 0, getpagesize ());
  test_memcpy_tests_run(0, 0, 2 * getpagesize ());
  test_memcpy_tests_run(0, 0, 4 * getpagesize ());
  test_memcpy_tests_run(0, 0, 8 * getpagesize ());
}

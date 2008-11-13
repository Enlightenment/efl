
#include <stdlib.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "evil_suite.h"
#include "evil_test_memcpy.h"


typedef void *(*memcpy_decl)(void *dest, const void *src, size_t n);

void *memcpy_glibc(void *dest, const void *src, size_t n);


static unsigned char *buf1 = NULL;
static unsigned char *buf2 = NULL;
static size_t         page_size = 0;


#ifdef __MINGW32CE__
static int
getpagesize()
{
   return 1024;
}
#endif /* __MINGW32CE__ */


static void
test_memcpy_test_run(suite *s, memcpy_decl fct, char *dst, const char *src, size_t len)
{
   double best;
   int    i;

   best = 1000000000.0;

   for (i = 0; i < 128; ++i)
     {
        double time;

        suite_time_start(s);
        fct(dst, src, len);
        suite_time_stop(s);
        time = suite_time_get(s);
        if (time < best) best = time;
     }

   printf ("  %e", best);
}

static void
test_memcpy_tests_run(suite *s, size_t align1, size_t align2, size_t len)
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

   test_memcpy_test_run(s, memcpy, s2, s1, len);
#ifdef _WIN32_WCE
   test_memcpy_test_run(s, memcpy_glibc, s2, s1, len);
#endif /* _WIN32_WCE */

   printf ("\n");
}

int
test_memcpy(suite *s)
{
   size_t i;

   page_size = 2 * 1024;

   buf1 = (unsigned char *)malloc(16 * getpagesize());
   if (!buf1) return 0;

   buf2 = (unsigned char *)malloc(16 * getpagesize());
   if (!buf2)
     {
        free(buf1);
        return 0;
     }

   memset (buf1, 0xa5, page_size);
   memset (buf2, 0x5a, page_size);

  for (i = 0; i < 5; ++i)
    {
      test_memcpy_tests_run(s, 0, 0, 1 << i);
      test_memcpy_tests_run(s, i, 0, 1 << i);
      test_memcpy_tests_run(s, 0, i, 1 << i);
      test_memcpy_tests_run(s, i, i, 1 << i);
    }

  for (i = 0; i < 32; ++i)
    {
      test_memcpy_tests_run(s, 0, 0, i);
      test_memcpy_tests_run(s, i, 0, i);
      test_memcpy_tests_run(s, 0, i, i);
      test_memcpy_tests_run(s, i, i, i);
    }

  for (i = 3; i < 32; ++i)
    {
      if ((i & (i - 1)) == 0)
	continue;
      test_memcpy_tests_run(s, 0, 0, 16 * i);
      test_memcpy_tests_run(s, i, 0, 16 * i);
      test_memcpy_tests_run(s, 0, i, 16 * i);
      test_memcpy_tests_run(s, i, i, 16 * i);
    }

  test_memcpy_tests_run(s, 0, 0, getpagesize ());
  test_memcpy_tests_run(s, 0, 0, 2 * getpagesize ());
  test_memcpy_tests_run(s, 0, 0, 4 * getpagesize ());
  test_memcpy_tests_run(s, 0, 0, 8 * getpagesize ());
  test_memcpy_tests_run(s, 0, 0, 16 * getpagesize ());

  free(buf2);
  free(buf1);

  return 1;
}

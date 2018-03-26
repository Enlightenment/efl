#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Ecore_Fb.h>

#include "ecore_suite.h"

#define MAX_ITER 10

EFL_START_TEST(ecore_test_ecore_fb_init)
{
// SKIP fb test as all it tests is init and shutdown and nothing else
// and these require basically a linux box with root or special
// permission access etc. etc. which prety much will NOt be the case
// on any build machine runing make check - so de-noise the tests
// so we can focus on the rest that should work
/*
   int ret, i, j;

   for (i = 1; i <= MAX_ITER; i++)
     {
        ret = ecore_fb_init("display");
        fprintf(stderr, "Created %d ecore fb instance.\n", i);
        fail_if(ret != i);
     }

   for (j = MAX_ITER - 1; j >= 0; j--)
     {
        ret = ecore_fb_shutdown();
        fprintf(stderr, "Deleted %d ecore fb instance.\n", MAX_ITER - j);
        fail_if(ret != j);
     }
 */
}
EFL_END_TEST

void ecore_test_ecore_fb(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_fb_init);
}

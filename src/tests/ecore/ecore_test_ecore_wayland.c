#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Ecore_Wayland.h>

#include "ecore_suite.h"

#define MAX_ITER 15

START_TEST(ecore_test_ecore_wl_init)
{
   int ret, i, j;

   for (i = 1; i <= MAX_ITER; i++)
     {
        ret = ecore_wl_init(NULL);
        fprintf(stderr, "Created %d ecore wayland instance.\n", i);
        fail_if(ret != i);
     }

   for (j = MAX_ITER - 1; j >= 0; j--)
     {
        ret = ecore_wl_shutdown();
        fprintf(stderr, "Deleted %d ecore wayland instance.\n", MAX_ITER - j);
        fail_if(ret != j);
     }
}
END_TEST

void ecore_test_ecore_wayland(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_wl_init);
}

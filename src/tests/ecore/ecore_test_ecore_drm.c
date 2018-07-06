#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Ecore_Drm.h>

#include "ecore_suite.h"

#define MAX_ITER 10

EFL_START_TEST(ecore_test_ecore_drm_shutdown_bef_init)
{
   int ret = 0;

   ret = ecore_drm_shutdown();
   fail_if(ret != 0);

   ret = ecore_drm_init();
   fail_if(ret != 1);

   ret = ecore_drm_shutdown();
   fail_if(ret != 0);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_drm_init)
{
   int ret, i, j;

   for (i = 1; i <= MAX_ITER; i++)
     {
        ret = ecore_drm_init();
        fail_if(ret != i);
     }

   for (j = MAX_ITER - 1; j >= 0; j--)
     {
        ret = ecore_drm_shutdown();
        fail_if(ret != j);
     }
}
EFL_END_TEST

void ecore_test_ecore_drm(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_drm_init);
   tcase_add_test(tc, ecore_test_ecore_drm_shutdown_bef_init);
}

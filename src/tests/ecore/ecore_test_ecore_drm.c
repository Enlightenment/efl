#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Ecore_Drm.h>

#include "ecore_suite.h"

#define MAX_ITER 10

START_TEST(ecore_test_ecore_drm_shutdown_bef_init)
{
   int ret = 0;

   ret = ecore_drm_shutdown();
   fprintf(stderr, "Calling ecore_drm_shutdown without calling ecore_drm_init.\n");
   fail_if(ret != 0);

   ret = ecore_drm_init();
   fprintf(stderr, "Calling ecore_drm_init.\n");
   fail_if(ret != 1);

   ret = ecore_drm_shutdown();
   fprintf(stderr, "Calling ecore_drm_shutdown after ecore_drm_init.\n");
   fail_if(ret != 0);
}
END_TEST

START_TEST(ecore_test_ecore_drm_init)
{
   int ret, i, j;

   for (i = 1; i <= MAX_ITER; i++)
     {
        ret = ecore_drm_init();
        fprintf(stderr, "Created %d ecore drm instance.\n", i);
        fail_if(ret != i);
     }

   for (j = MAX_ITER - 1; j >= 0; j--)
     {
        ret = ecore_drm_shutdown();
        fprintf(stderr, "Deleted %d ecore drm instance.\n", MAX_ITER - j);
        fail_if(ret != j);
     }
}
END_TEST

void ecore_test_ecore_drm(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_drm_init);
   tcase_add_test(tc, ecore_test_ecore_drm_shutdown_bef_init);
}

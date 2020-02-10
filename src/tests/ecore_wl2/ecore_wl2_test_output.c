#include "ecore_wl2_suite.h"

EFL_START_TEST(wl2_output_dpi_get)
{
   int ret;

   //FIXME: Need some discussion about how to validate this API in TC.
   ret = ecore_wl2_output_dpi_get(NULL);

   fail_if(ret != 75);
}
EFL_END_TEST

EFL_START_TEST(wl2_output_transform_get)
{
   int ret;

   //FIXME: Need some discussion about how to validate this API in TC.
   ret = ecore_wl2_output_transform_get(NULL);

   fail_if(ret != 0);
}
EFL_END_TEST

void
ecore_wl2_test_output(TCase *tc)
{
   if (getenv("WAYLAND_DISPLAY"))
     {
        tcase_add_test(tc, wl2_output_dpi_get);
        tcase_add_test(tc, wl2_output_transform_get);
     }
}

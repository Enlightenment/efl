#include "ecore_wl2_suite.h"

EFL_START_TEST(ecore_wl2_simple)
{
}
EFL_END_TEST

void
ecore_wl2_test_init(TCase *tc)
{
   tcase_add_test(tc, ecore_wl2_simple);
}

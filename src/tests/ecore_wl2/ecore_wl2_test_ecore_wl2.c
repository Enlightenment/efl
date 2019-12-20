#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Wl2.h>

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

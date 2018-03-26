#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Evas.h>

#include "evas_suite.h"

EFL_START_TEST(evas_simple)
{
}
EFL_END_TEST

void evas_test_init(TCase *tc)
{
   tcase_add_test(tc, evas_simple);
}

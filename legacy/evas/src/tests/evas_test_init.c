
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "evas_suite.h"
#include "Evas.h"

START_TEST(evas_simple)
{
}
END_TEST

void evas_test_init(TCase *tc)
{
   tcase_add_test(tc, evas_simple);
}


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "evas_suite.h"
#include "Evas.h"

START_TEST(evas_simple)
{
   fail_if(evas_init() != 1); /* one init by test suite */
   fail_if(evas_shutdown() != 0);
}
END_TEST

void evas_test_init(TCase *tc)
{
   tcase_add_test(tc, evas_simple);
}

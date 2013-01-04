#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>

#include <Eina.h>
#include <Edje.h>

#include "edje_suite.h"

START_TEST(edje_test_edje_init)
{
   int ret;

   ret = edje_init();
   fail_if(ret != 1);

   ret = edje_shutdown();
   fail_if(ret != 0);
}
END_TEST

void edje_test_edje(TCase *tc)
{
   tcase_add_test(tc, edje_test_edje_init);
}

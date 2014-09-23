#include "config.h" 

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efreet.h>

#include "efreet_suite.h"


START_TEST(efreet_test_efreet_init)
{
   int ret;

   ret = efreet_init();
   fail_if(ret != 1);

   ret = efreet_shutdown();
   fail_if(ret != 0);
}
END_TEST

void efreet_test_efreet(TCase *tc)
{
   tcase_add_test(tc, efreet_test_efreet_init);
}

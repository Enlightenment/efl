#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eeze.h>

#include "eeze_suite.h"

START_TEST(eeze_test_eeze_init)
{
   int ret;

   ret = eeze_init();
   fail_if(ret != 1);

   ret = eeze_shutdown();
   fail_if(ret != 0);
}
END_TEST

void eeze_test_init(TCase *tc)
{
   tcase_add_test(tc, eeze_test_eeze_init);
}

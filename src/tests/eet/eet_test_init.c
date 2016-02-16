#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eet.h>

#include "eet_suite.h"

START_TEST(_eet_test_init)
{
   int ret;

   ret = eet_init();
   fail_if(ret != 1);

   ret = eet_shutdown();
   fail_if(ret != 0);
}
END_TEST

void eet_test_init(TCase *tc)
{
   tcase_add_test(tc, _eet_test_init);
}

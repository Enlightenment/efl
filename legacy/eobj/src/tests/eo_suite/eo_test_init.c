#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "Eo.h"
#include "eo_suite.h"

START_TEST(eo_simple)
{
   fail_if(!eo_init()); /* one init by test suite */
   fail_if(eo_init() != 1);
   fail_if(eo_shutdown() != 1);
   fail_if(!eo_shutdown());
}
END_TEST

void eo_test_init(TCase *tc)
{
   tcase_add_test(tc, eo_simple);
}

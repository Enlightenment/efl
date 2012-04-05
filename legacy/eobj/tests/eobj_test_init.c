#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "eobj_suite.h"
#include "eobj.h"

START_TEST(eobj_simple)
{
   fail_if(!eobj_init()); /* one init by test suite */
   fail_if(!eobj_shutdown());
}
END_TEST

void eobj_test_init(TCase *tc)
{
   tcase_add_test(tc, eobj_simple);
}

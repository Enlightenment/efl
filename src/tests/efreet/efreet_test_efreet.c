#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efreet.h>

#include "efreet_suite.h"


EFL_START_TEST(efreet_test_efreet_init)
{
}
EFL_END_TEST

void efreet_test_efreet(TCase *tc)
{
   tcase_add_test(tc, efreet_test_efreet_init);
}

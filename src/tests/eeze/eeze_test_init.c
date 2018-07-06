#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eeze.h>

#include "eeze_suite.h"

EFL_START_TEST(eeze_test_eeze_init)
{
}
EFL_END_TEST

void eeze_test_init(TCase *tc)
{
   tcase_add_test(tc, eeze_test_eeze_init);
}

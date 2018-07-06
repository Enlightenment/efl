#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eet.h>

#include "eet_suite.h"

EFL_START_TEST(_eet_test_init)
{
}
EFL_END_TEST

void eet_test_init(TCase *tc)
{
   tcase_add_test(tc, _eet_test_init);
}

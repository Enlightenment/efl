#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Emile.h>

#include "emile_suite.h"

EFL_START_TEST(emile_test_init)
{
}
EFL_END_TEST

void emile_test_base(TCase *tc)
{
   tcase_add_test(tc, emile_test_init);
}

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Emile.h>

#include "emile_suite.h"

START_TEST(emile_test_init)
{
   fail_if(emile_init() <= 0);
   fail_if(emile_shutdown() != 0);
}
END_TEST

void emile_test_base(TCase *tc)
{
   tcase_add_test(tc, emile_test_init);
}

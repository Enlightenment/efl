#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_batterystatus_test.h"
#include "ecordova_suite.h"

#include <stdbool.h>

static void
_setup(void)
{
   int ret = ecordova_init();
   ck_assert_int_eq(ret, 1);
}

static void
_teardown(void)
{
   int ret = ecordova_shutdown();
   ck_assert_int_eq(ret, 0);
}

static Ecordova_Device *
_batterystatus_new(void)
{
   return eo_add(ECORDOVA_BATTERYSTATUS_CLASS,
                 NULL,
                 ecordova_batterystatus_constructor());
}

START_TEST(smoke)
{
   Ecordova_Device *batterystatus = _batterystatus_new();
   eo_unref(batterystatus);
}
END_TEST

void
ecordova_batterystatus_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
}

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_vibration_test.h"
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
_vibration_new(void)
{
   return eo_add(ECORDOVA_VIBRATION_CLASS,
                 NULL,
                 ecordova_vibration_constructor());
}

START_TEST(smoke)
{
   Ecordova_Device *vibration = _vibration_new();
   eo_unref(vibration);
}
END_TEST

void
ecordova_vibration_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   //tcase_add_test(tc, smoke); // disabled: not supported on common profile
}

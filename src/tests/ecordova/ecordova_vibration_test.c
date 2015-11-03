#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecordova.h>

#include "ecordova_suite.h"

#include <check.h>

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

#ifdef HAVE_TIZEN_CONFIGURATION_MANAGER
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
   tcase_add_test(tc, smoke); // not supported on common profile
}
#else
START_TEST(vibration_load_fail)
{
   Ecordova_Device *vibration = eo_add(ECORDOVA_VIBRATION_CLASS, NULL);
   ck_assert_ptr_eq(vibration, NULL);
}
END_TEST

void
ecordova_vibration_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, vibration_load_fail);
}
#endif

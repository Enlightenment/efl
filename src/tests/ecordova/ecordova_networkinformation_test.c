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
_networkinformation_new(void)
{
   return eo_add(ECORDOVA_NETWORKINFORMATION_CLASS,
                 NULL,
                 ecordova_networkinformation_constructor());
}

START_TEST(smoke)
{
   Ecordova_Device *networkinformation = _networkinformation_new();
   eo_unref(networkinformation);
}
END_TEST

void
ecordova_networkinformation_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke); // not supported on common profile
}
#else
START_TEST(networkinformation_load_fail)
{
   Ecordova_Device *networkinformation = eo_add(ECORDOVA_NETWORKINFORMATION_CLASS, NULL);
   ck_assert_ptr_eq(networkinformation, NULL);
}
END_TEST

void
ecordova_networkinformation_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, networkinformation_load_fail);
}
#endif

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

#if defined(HAVE_TIZEN_CONFIGURATION_MANAGER) && defined(HAVE_TIZEN_INFO)
static Ecordova_Device *
_device_new(void)
{
   return eo_add(ECORDOVA_DEVICE_CLASS, NULL, ecordova_device_constructor());
}

START_TEST(smoke)
{
   Ecordova_Device *device = _device_new();
   eo_unref(device);
}
END_TEST

START_TEST(model_get)
{
   Ecordova_Device *device = _device_new();

   const char *model = eo_do_ret(device, model, ecordova_device_model_get());
   fail_if(NULL == model);

   INF("%s", model);

   eo_unref(device);
}
END_TEST

START_TEST(platform_get)
{
   Ecordova_Device *device = _device_new();

   const char *platform = eo_do_ret(device,
                                    platform,
                                    ecordova_device_platform_get());
   fail_if(NULL == platform);

   INF("%s", platform);

   eo_unref(device);
}
END_TEST

START_TEST(uuid_get)
{
   Ecordova_Device *device = _device_new();

   const char *uuid = eo_do_ret(device, uuid, ecordova_device_uuid_get());
   fail_if(NULL == uuid);

   INF("%s", uuid);

   eo_unref(device);
}
END_TEST

START_TEST(version_get)
{
   Ecordova_Device *device = _device_new();

   const char *version = eo_do_ret(device,
                                   version,
                                   ecordova_device_version_get());
   fail_if(NULL == version);

   INF("%s", version);

   eo_unref(device);
}
END_TEST

void
ecordova_device_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, model_get);
   tcase_add_test(tc, platform_get);
   //tcase_add_test(tc, uuid_get); // disabled: returns NULL
   tcase_add_test(tc, version_get);
}
#else
START_TEST(device_load_fail)
{
   Ecordova_Device *device = eo_add(ECORDOVA_DEVICE_CLASS, NULL);
   ck_assert_ptr_eq(device, NULL);
}
END_TEST

void
ecordova_device_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, device_load_fail);
}
#endif

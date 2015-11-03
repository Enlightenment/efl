#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecordova.h>

#include "ecordova_suite.h"

#include <check.h>

#ifdef HAVE_TIZEN_CONFIGURATION_MANAGER
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
_devicemotion_new(void)
{
   return eo_add(ECORDOVA_DEVICEMOTION_CLASS,
                 NULL,
                 ecordova_devicemotion_constructor());
}

START_TEST(smoke)
{
   Ecordova_Device *devicemotion = _devicemotion_new();
   eo_unref(devicemotion);
}
END_TEST

static Eina_Bool
_current_acceleration_get_cb(void *data,
                             Eo *obj EINA_UNUSED,
                             const Eo_Event_Description *desc EINA_UNUSED,
                             void *event_info EINA_UNUSED)
{
   bool *called = data;
   *called = true;
   ecore_main_loop_quit();
   return EINA_FALSE;
}

static Eina_Bool
_error_cb(void *data,
          Eo *obj EINA_UNUSED,
          const Eo_Event_Description *desc EINA_UNUSED,
          void *event_info EINA_UNUSED)
{
   bool *error = data;
   *error = true;
   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(current_acceleration_get)
{
   Ecordova_Device *devicemotion = _devicemotion_new();

   bool success_event_called = false;
   bool error_event_called = false;
   eo_do(devicemotion, eo_event_callback_add(ECORDOVA_DEVICEMOTION_EVENT_CURRENT_SUCCESS, _current_acceleration_get_cb, &success_event_called));
   eo_do(devicemotion, eo_event_callback_add(ECORDOVA_DEVICEMOTION_EVENT_ERROR, _error_cb, &error_event_called));
   eo_do(devicemotion, ecordova_devicemotion_current_acceleration_get());

   if (!success_event_called && !error_event_called)
     ecore_main_loop_begin();

   fail_if(error_event_called);
   fail_unless(success_event_called);

   eo_unref(devicemotion);
}
END_TEST

void
ecordova_devicemotion_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   //tcase_add_test(tc, current_acceleration_get); // disabled: not supported
}
#else
START_TEST(devicemotion_fail_load)
{
   ecordova_init();

   Ecordova_Device *devicemotion
     = eo_add(ECORDOVA_DEVICEMOTION_CLASS, NULL);

   ck_assert_ptr_eq(devicemotion, NULL);

   ecordova_shutdown();
}
END_TEST

void
ecordova_devicemotion_test(TCase *tc)
{
   tcase_add_test(tc, devicemotion_fail_load);
}
#endif

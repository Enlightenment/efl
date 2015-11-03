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
_deviceorientation_new(void)
{
   return eo_add(ECORDOVA_DEVICEORIENTATION_CLASS,
                 NULL,
                 ecordova_deviceorientation_constructor());
}

START_TEST(smoke)
{
   Ecordova_Device *deviceorientation = _deviceorientation_new();
   eo_unref(deviceorientation);
}
END_TEST

static Eina_Bool
_current_heading_get_cb(void *data,
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

START_TEST(current_heading_get)
{
   Ecordova_Device *deviceorientation = _deviceorientation_new();

   bool success_event_called = false;
   bool error_event_called = false;
   eo_do(deviceorientation, eo_event_callback_add(ECORDOVA_DEVICEORIENTATION_EVENT_CURRENT_SUCCESS, _current_heading_get_cb, &success_event_called));
   eo_do(deviceorientation, eo_event_callback_add(ECORDOVA_DEVICEORIENTATION_EVENT_ERROR, _error_cb, &error_event_called));
   eo_do(deviceorientation, ecordova_deviceorientation_current_heading_get());

   if (!success_event_called && !error_event_called)
     ecore_main_loop_begin();

   fail_if(error_event_called);
   fail_unless(success_event_called);

   eo_unref(deviceorientation);
}
END_TEST

void
ecordova_deviceorientation_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   //tcase_add_test(tc, current_heading_get); // disabled: not supported
}
#else
START_TEST(deviceorientation_fail_load)
{
   Ecordova_Device *deviceorientation = eo_add(ECORDOVA_DEVICEORIENTATION_CLASS, NULL);

   ck_assert_ptr_eq(deviceorientation, NULL);
}
END_TEST

void
ecordova_deviceorientation_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, deviceorientation_fail_load);
}
#endif

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
_geolocation_new(void)
{
   return eo_add(ECORDOVA_GEOLOCATION_CLASS,
                 NULL,
                 ecordova_geolocation_constructor());
}

START_TEST(smoke)
{
   Ecordova_Device *geolocation = _geolocation_new();
   eo_unref(geolocation);
}
END_TEST

static Eina_Bool
_current_position_get_cb(void *data,
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

START_TEST(current_position_get)
{
   Ecordova_Device *geolocation = _geolocation_new();

   bool success_event_called = false;
   bool error_event_called = false;
   eo_do(geolocation, eo_event_callback_add(ECORDOVA_GEOLOCATION_EVENT_CURRENT_SUCCESS, _current_position_get_cb, &success_event_called));
   eo_do(geolocation, eo_event_callback_add(ECORDOVA_GEOLOCATION_EVENT_ERROR, _error_cb, &error_event_called));
   const Ecordova_Geolocation_Options options = {
      .enable_high_accuracy = false,
      .timeout = 4000
   };
   eo_do(geolocation, ecordova_geolocation_current_position_get(&options));

   if (!success_event_called && !error_event_called)
     ecore_main_loop_begin();

   fail_if(error_event_called);
   fail_unless(success_event_called);

   eo_unref(geolocation);
}
END_TEST

void
ecordova_geolocation_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   //tcase_add_test(tc, current_position_get); // disabled: not supported
}
#else
START_TEST(geolocation_fail_load)
{
   Ecordova_Device *geolocation = eo_add(ECORDOVA_GEOLOCATION_CLASS, NULL);
   ck_assert_ptr_eq(geolocation, NULL);
}
END_TEST

void
ecordova_geolocation_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, geolocation_fail_load);
}
#endif

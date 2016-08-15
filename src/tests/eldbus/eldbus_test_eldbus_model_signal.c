#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdbool.h>

#include <Eina.h>
#include <Ecore.h>
#include <Eldbus_Model.h>

#include "eldbus_fake_server.h"
#include "eldbus_test_eldbus_model.h"
#include "eldbus_suite.h"

#define ARGUMENT_A "arg0"

static Eo *fake_server_object = NULL;
static Eo *fake_server_proxy = NULL;
static Eldbus_Service_Interface *fake_server = NULL;
static Fake_Server_Data fake_server_data = {0};
static Eo *pong_signal = NULL;

static void
_setup(void)
{
   check_init();

   fake_server = fake_server_start(&fake_server_data);

   fake_server_object = efl_add(ELDBUS_MODEL_OBJECT_CLASS, NULL, eldbus_model_object_constructor(efl_self, ELDBUS_CONNECTION_TYPE_SESSION, NULL, EINA_FALSE, FAKE_SERVER_BUS, FAKE_SERVER_PATH));
   ck_assert_ptr_ne(NULL, fake_server_object);

   fake_server_proxy = eldbus_model_proxy_from_object_get(fake_server_object, FAKE_SERVER_INTERFACE);
   ck_assert_ptr_ne(NULL, fake_server_proxy);

   pong_signal = eldbus_model_signal_from_proxy_get(fake_server_proxy, FAKE_SERVER_PONG_SIGNAL_NAME);
   ck_assert_ptr_ne(NULL, pong_signal);
}

static void
_teardown(void)
{
   efl_unref(fake_server_object);

   fake_server_stop(fake_server);

   check_shutdown();
}

START_TEST(properties_get)
{
   const Eina_Array *properties = NULL;
   properties = efl_model_properties_get(pong_signal);
   ck_assert_ptr_ne(NULL, properties);

   const unsigned int expected_properties_count = 1; // 'response' only
   const unsigned int actual_properties_count = eina_array_count(properties);
   ck_assert_int_eq(expected_properties_count, actual_properties_count);
}
END_TEST

START_TEST(property_get)
{
   // Signal properties always have output direction
   Eina_Promise *promise;
   promise = efl_model_property_get(pong_signal, ARGUMENT_A);
   efl_model_promise_then(promise);

   // Nonexistent property must return ERROR
   promise = efl_model_property_get(pong_signal, "nonexistent");
   check_efl_model_promise_error(promise, &EFL_MODEL_ERROR_NOT_FOUND);
}
END_TEST

START_TEST(property_set)
{
   // Signals have output arguments only. All returns error
   Eina_Promise *promise;
   Eina_Value dummy = {0};
   efl_model_property_set(pong_signal, ARGUMENT_A, &dummy, &promise);
   check_efl_model_promise_error(promise, NULL);
}
END_TEST

static void
_test_signal_children_count(Eo *efl_model)
{
   check_efl_model_children_count_eq(efl_model, 0);
}

START_TEST(children_count)
{
   _test_signal_children_count(pong_signal);
}
END_TEST

START_TEST(children_slice_get)
{
   Eina_Promise *promise;
   promise = efl_model_children_slice_get(pong_signal, 1, 1);
   check_efl_model_promise_error(promise, &EFL_MODEL_ERROR_NOT_SUPPORTED);
}
END_TEST

START_TEST(child_add)
{
   Eo *child;
   child = efl_model_child_add(pong_signal);
   ck_assert_ptr_eq(NULL, child);
}
END_TEST

START_TEST(child_del)
{
   // efl_model_child_del always returns ERROR FIXME catch error
   Eo *child = NULL;
   efl_model_child_del(pong_signal, child);
}
END_TEST

START_TEST(signals)
{
   Eldbus_Model_Method *ping_method = eldbus_model_method_from_proxy_get(fake_server_proxy, FAKE_SERVER_PING_METHOD_NAME);
   ck_assert_ptr_ne(NULL, ping_method);

   check_efl_model_property_int_set(ping_method, ARGUMENT_A, 99);

   eldbus_model_method_call(ping_method);

   efl_model_wait_for_event(pong_signal, EFL_MODEL_EVENT_PROPERTIES_CHANGED);

   check_efl_model_property_int_eq(pong_signal, ARGUMENT_A, 100);
}
END_TEST

void eldbus_test_eldbus_model_signal(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, properties_get);
   tcase_add_test(tc, property_get);
   tcase_add_test(tc, property_set);
   tcase_add_test(tc, children_count);
   tcase_add_test(tc, children_slice_get);
   tcase_add_test(tc, child_add);
   tcase_add_test(tc, child_del);
   tcase_add_test(tc, signals);
}

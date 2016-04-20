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
#define ARGUMENT_B "arg1"
#define ARGUMENT_RESULT "arg2"

static Eo *fake_server_object = NULL;
static Eo *fake_server_proxy = NULL;
static Eldbus_Service_Interface *fake_server = NULL;
static Fake_Server_Data fake_server_data = {0};
static Eo *method = NULL;

static void
_setup(void)
{
   check_init();

   fake_server = fake_server_start(&fake_server_data);

   fake_server_object = eo_add(ELDBUS_MODEL_OBJECT_CLASS, NULL, eldbus_model_object_constructor(eo_self, ELDBUS_CONNECTION_TYPE_SESSION, NULL, EINA_FALSE, FAKE_SERVER_BUS, FAKE_SERVER_PATH));
   ck_assert_ptr_ne(NULL, fake_server_object);

   fake_server_proxy = eldbus_model_proxy_from_object_get(fake_server_object, FAKE_SERVER_INTERFACE);
   ck_assert_ptr_ne(NULL, fake_server_proxy);

   method = eldbus_model_method_from_proxy_get(fake_server_proxy, FAKE_SERVER_SUM_METHOD_NAME);
   ck_assert_ptr_ne(NULL, method);
}

static void
_teardown(void)
{
   eo_unref(fake_server_object);

   fake_server_stop(fake_server);

   check_shutdown();
}

START_TEST(properties_get)
{
   const Eina_Array *properties = NULL;
   properties = efl_model_properties_get(method);
   ck_assert_ptr_ne(NULL, properties);

   const unsigned int expected_properties_count = 3; // a, b and result arguments of 'sum' method
   const unsigned int actual_properties_count = eina_array_count(properties);
   ck_assert_int_eq(expected_properties_count, actual_properties_count);
}
END_TEST

START_TEST(property_get)
{
   // Input only property returns error
   Eina_Promise *promise;
   efl_model_property_get(method, ARGUMENT_A, &promise);
   check_efl_model_promise_error(promise, NULL);

   efl_model_property_get(method, ARGUMENT_RESULT, &promise);
   efl_model_promise_then(promise);

   // Nonexistent property returns error
   efl_model_property_get(method, "nonexistent", &promise);
   check_efl_model_promise_error(promise, NULL);
}
END_TEST

START_TEST(property_set)
{
   // Output argument returns error
   Eina_Promise *promise;
   Eina_Value dummy = {0};
   efl_model_property_set(method, ARGUMENT_RESULT, &dummy, &promise);
   check_efl_model_promise_error(promise, NULL);
}
END_TEST

static void
_test_method_children_count(Eo *efl_model)
{
   check_efl_model_children_count_eq(efl_model, 0);
}

START_TEST(children_count)
{
   _test_method_children_count(method);
}
END_TEST

START_TEST(children_slice_get)
{
   Eina_Promise *promise;
   efl_model_children_slice_get(method, 1, 1, &promise);
   check_efl_model_promise_error(promise, &EFL_MODEL_ERROR_NOT_SUPPORTED);
}
END_TEST

START_TEST(child_add)
{
   Eo *child;
   child = efl_model_child_add(method);
   ck_assert_ptr_eq(NULL, child);
}
END_TEST

START_TEST(child_del)
{
   // efl_model_child_del always returns ERROR
   Eo *child = NULL;
   efl_model_child_del(method, child);
   //ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);
}
END_TEST

START_TEST(call)
{
   check_efl_model_property_int_set(method, ARGUMENT_A, 12345678);
   check_efl_model_property_int_set(method, ARGUMENT_B, 87654321);

   eldbus_model_method_call(method);

   efl_model_wait_for_event(method, ELDBUS_MODEL_METHOD_EVENT_SUCCESSFUL_CALL);
   check_efl_model_property_int_eq(method, ARGUMENT_RESULT, 99999999);
}
END_TEST

void eldbus_test_eldbus_model_method(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, properties_get);
   tcase_add_test(tc, property_get);
   tcase_add_test(tc, property_set);
   tcase_add_test(tc, children_count);
   tcase_add_test(tc, children_slice_get);
   tcase_add_test(tc, child_add);
   tcase_add_test(tc, child_del);
   tcase_add_test(tc, call);
}

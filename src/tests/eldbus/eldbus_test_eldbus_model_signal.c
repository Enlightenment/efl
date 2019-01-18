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
   char buf[1024];
   snprintf(buf, sizeof(buf), FAKE_SERVER_BUS ".%s", basename(__FILE__));
   fake_server = fake_server_start(&fake_server_data, buf);

   fake_server_object = efl_add(ELDBUS_MODEL_OBJECT_CLASS, efl_main_loop_get(),
                                eldbus_model_connect(efl_added, ELDBUS_CONNECTION_TYPE_SESSION, NULL, EINA_FALSE),
                                eldbus_model_object_bus_set(efl_added, buf),
                                eldbus_model_object_path_set(efl_added, FAKE_SERVER_PATH));
   ck_assert_ptr_ne(NULL, fake_server_object);

   fake_server_proxy = eldbus_model_proxy_from_object_get(fake_server_object, FAKE_SERVER_INTERFACE);
   ck_assert_ptr_ne(NULL, fake_server_proxy);

   pong_signal = eldbus_model_signal_from_proxy_get(fake_server_proxy, FAKE_SERVER_PONG_SIGNAL_NAME);
   ck_assert_ptr_ne(NULL, pong_signal);
}

static void
_teardown(void)
{
   efl_del(fake_server_object);

   fake_server_stop(fake_server);

}

EFL_START_TEST(properties_get)
{
   Eina_Iterator *properties = NULL;
   properties = efl_model_properties_get(pong_signal);
   ck_assert_ptr_ne(NULL, properties);

   const unsigned int expected_properties_count = 1; // 'response' only
   unsigned int actual_properties_count = 0;
   const char *prop;

   EINA_ITERATOR_FOREACH(properties, prop)
     actual_properties_count += 1;
   eina_iterator_free(properties);
   ck_assert_int_eq(expected_properties_count, actual_properties_count);
}
EFL_END_TEST

EFL_START_TEST(property_get)
{
   Eina_Value *value;
   Eina_Error err;

   // Signal properties always have output direction
   value = efl_model_property_get(pong_signal, ARGUMENT_A);
   fail_if(value == NULL);

   // Nonexistent property must return ERROR
   value = efl_model_property_get(pong_signal, "nonexistent");
   fail_if(value == NULL);
   fail_if(eina_value_type_get(value) != EINA_VALUE_TYPE_ERROR);

   eina_value_error_get(value, &err);
   fail_if(err != EFL_MODEL_ERROR_NOT_FOUND);
}
EFL_END_TEST

EFL_START_TEST(property_set)
{
   // Signals have output arguments only. All returns error
   Eina_Value dummy = EINA_VALUE_EMPTY;
   Eina_Future *future;

   future = efl_model_property_set(pong_signal, ARGUMENT_A, &dummy);
   check_efl_model_future_error(future, NULL);
}
EFL_END_TEST

static void
_test_signal_children_count(Eo *efl_model)
{
   check_efl_model_children_count_eq(efl_model, 0);
}

EFL_START_TEST(children_count)
{
   _test_signal_children_count(pong_signal);
}
EFL_END_TEST

EFL_START_TEST(children_slice_get)
{
   Eina_Future *future;

   future = efl_model_children_slice_get(pong_signal, 1, 1);
   check_efl_model_future_error(future, &EFL_MODEL_ERROR_NOT_SUPPORTED);
}
EFL_END_TEST

EFL_START_TEST(child_add)
{
   Eo *child;
   child = efl_model_child_add(pong_signal);
   ck_assert_ptr_eq(NULL, child);
}
EFL_END_TEST

EFL_START_TEST(child_del)
{
   // efl_model_child_del always returns ERROR FIXME catch error
   Eo *child = NULL;
   efl_model_child_del(pong_signal, child);
}
EFL_END_TEST

EFL_START_TEST(signals)
{
   Eldbus_Model_Method *ping_method = eldbus_model_method_from_proxy_get(fake_server_proxy, FAKE_SERVER_PING_METHOD_NAME);
   ck_assert_ptr_ne(NULL, ping_method);

   check_efl_model_property_int_set(ping_method, ARGUMENT_A, 99);

   eldbus_model_method_call(ping_method);

   efl_model_wait_for_event(pong_signal, EFL_MODEL_EVENT_PROPERTIES_CHANGED);

   check_efl_model_property_int_eq(pong_signal, ARGUMENT_A, 100);
}
EFL_END_TEST

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

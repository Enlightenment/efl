#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eldbus_fake_server.h"
#include "eldbus_suite.h"
#include "eldbus_test_eldbus_model.h"

#include <Ecore.h>
#include <Eina.h>
#include <Eldbus_Model.h>

#include <stdbool.h>

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

   fake_server_object = eo_add(ELDBUS_MODEL_OBJECT_CLASS, NULL,
     eldbus_model_object_constructor(ELDBUS_CONNECTION_TYPE_SESSION,
                                     NULL,
                                     EINA_FALSE,
                                     FAKE_SERVER_BUS,
                                     FAKE_SERVER_PATH));
   ck_assert_ptr_ne(NULL, fake_server_object);

   efl_model_load_and_wait_for_load_status(fake_server_object, EFL_MODEL_LOAD_STATUS_LOADED);

   fake_server_proxy = eldbus_model_proxy_from_object_get(fake_server_object, FAKE_SERVER_INTERFACE);
   ck_assert_ptr_ne(NULL, fake_server_proxy);

   efl_model_load_and_wait_for_load_status(fake_server_proxy, EFL_MODEL_LOAD_STATUS_LOADED);

   pong_signal = eldbus_model_signal_from_proxy_get(fake_server_proxy, FAKE_SERVER_PONG_SIGNAL_NAME);
   ck_assert_ptr_ne(NULL, pong_signal);

   efl_model_load_and_wait_for_load_status(pong_signal, EFL_MODEL_LOAD_STATUS_LOADED);
}

static void
_teardown(void)
{
   eo_unref(fake_server_object);

   fake_server_stop(fake_server);

   check_shutdown();
}

START_TEST(load_status_get)
{
   check_efl_model_load_status_get(pong_signal, EFL_MODEL_LOAD_STATUS_LOADED);
}
END_TEST

START_TEST(properties_get)
{
   Eina_Array *properties = NULL;
   Efl_Model_Load_Status status;
   eo_do(pong_signal, status = efl_model_properties_get(&properties));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, properties);

   const unsigned int expected_properties_count = 1; // 'response' only
   const unsigned int actual_properties_count = eina_array_count(properties);
   ck_assert_int_eq(expected_properties_count, actual_properties_count);
}
END_TEST

START_TEST(property_get)
{
   // Signal properties always have output direction
   Eina_Value const* dummy;
   Efl_Model_Load_Status status;
   eo_do(pong_signal, status = efl_model_property_get(ARGUMENT_A, &dummy));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, dummy);

   // Nonexistent property must return EFL_MODEL_LOAD_STATUS_ERROR
   eo_do(pong_signal, status = efl_model_property_get("nonexistent", &dummy));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);
}
END_TEST

START_TEST(property_set)
{
   // Signals have output arguments only. All returns error
   Eina_Value dummy = {0};
   Efl_Model_Load_Status status;
   eo_do(pong_signal, status = efl_model_property_set(ARGUMENT_A, &dummy));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);
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
   Eina_Accessor *accessor;
   Efl_Model_Load_Status status;
   eo_do(pong_signal, status = efl_model_children_slice_get(1, 1, &accessor));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_eq(NULL, accessor);
}
END_TEST

static void
_check_unload(void)
{
   check_efl_model_load_status_get(pong_signal, EFL_MODEL_LOAD_STATUS_LOADED);
   eo_do(pong_signal, efl_model_unload());
   check_efl_model_load_status_get(pong_signal, EFL_MODEL_LOAD_STATUS_UNLOADED);

   check_efl_model_children_count_eq(pong_signal, 0);
}

START_TEST(unload)
{
   _check_unload();
}
END_TEST

START_TEST(properties_load)
{
   _check_unload();

   eo_do(pong_signal, efl_model_properties_load());
   efl_model_wait_for_load_status(pong_signal, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);

   check_efl_model_load_status_get(pong_signal, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);
}
END_TEST

START_TEST(children_load)
{
   _check_unload();

   eo_do(pong_signal, efl_model_children_load());
   efl_model_wait_for_load_status(pong_signal, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   check_efl_model_load_status_get(pong_signal, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   _test_signal_children_count(pong_signal);
}
END_TEST

START_TEST(child_add)
{
   Eo *child;
   eo_do(pong_signal, child = efl_model_child_add());
   ck_assert_ptr_eq(NULL, child);
}
END_TEST

START_TEST(child_del)
{
   // efl_model_child_del always returns ERROR
   Eo *child = NULL;
   Efl_Model_Load_Status status;
   eo_do(pong_signal, status = efl_model_child_del(child));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);
}
END_TEST

START_TEST(signals)
{
   Eldbus_Model_Method *ping_method = eldbus_model_method_from_proxy_get(fake_server_proxy, FAKE_SERVER_PING_METHOD_NAME);
   ck_assert_ptr_ne(NULL, ping_method);

   efl_model_load_and_wait_for_load_status(ping_method, EFL_MODEL_LOAD_STATUS_LOADED);

   check_efl_model_property_int_set(ping_method, ARGUMENT_A, 99);

   Efl_Model_Load_Status status;
   eo_do(ping_method, status = eldbus_model_method_call());
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);

   efl_model_wait_for_event(pong_signal, EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED);

   check_efl_model_property_int_eq(pong_signal, ARGUMENT_A, 100);
}
END_TEST

void eldbus_test_eldbus_model_signal(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, load_status_get);
   tcase_add_test(tc, properties_get);
   tcase_add_test(tc, property_get);
   tcase_add_test(tc, property_set);
   tcase_add_test(tc, children_count);
   tcase_add_test(tc, children_slice_get);
   tcase_add_test(tc, unload);
   tcase_add_test(tc, properties_load);
   tcase_add_test(tc, children_load);
   tcase_add_test(tc, child_add);
   tcase_add_test(tc, child_del);
   tcase_add_test(tc, signals);
}

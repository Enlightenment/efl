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

   method = eldbus_model_method_from_proxy_get(fake_server_proxy, FAKE_SERVER_SUM_METHOD_NAME);
   ck_assert_ptr_ne(NULL, method);

   efl_model_load_and_wait_for_load_status(method, EFL_MODEL_LOAD_STATUS_LOADED);
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
   check_efl_model_load_status_get(method, EFL_MODEL_LOAD_STATUS_LOADED);
}
END_TEST

START_TEST(properties_get)
{
   Eina_Array *properties = NULL;
   Efl_Model_Load_Status status;
   eo_do(method, status = efl_model_properties_get(&properties));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, properties);

   const unsigned int expected_properties_count = 3; // a, b and result arguments of 'sum' method
   const unsigned int actual_properties_count = eina_array_count(properties);
   ck_assert_int_eq(expected_properties_count, actual_properties_count);
}
END_TEST

START_TEST(property_get)
{
   // Input only property returns error
   Eina_Value const* dummy;
   Efl_Model_Load_Status status;
   eo_do(method, status = efl_model_property_get(ARGUMENT_A, &dummy));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);

   eo_do(method, status = efl_model_property_get(ARGUMENT_RESULT, &dummy));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);

   // Nonexistent property returns error
   eo_do(method, status = efl_model_property_get("nonexistent", &dummy));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);
}
END_TEST

START_TEST(property_set)
{
   // Output argument returns error
   Eina_Value dummy = {0};
   Efl_Model_Load_Status status;
   eo_do(method, status = efl_model_property_set(ARGUMENT_RESULT, &dummy));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);
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
   Eina_Accessor *accessor;
   Efl_Model_Load_Status status;
   eo_do(method, status = efl_model_children_slice_get(1, 1, &accessor));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_eq(NULL, accessor);
}
END_TEST

static void
_check_unload(void)
{
   check_efl_model_load_status_get(method, EFL_MODEL_LOAD_STATUS_LOADED);
   eo_do(method, efl_model_unload());
   check_efl_model_load_status_get(method, EFL_MODEL_LOAD_STATUS_UNLOADED);

   check_efl_model_children_count_eq(method, 0);
}

START_TEST(unload)
{
   _check_unload();
}
END_TEST

START_TEST(properties_load)
{
   _check_unload();

   eo_do(method, efl_model_properties_load());
   efl_model_wait_for_load_status(method, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);

   check_efl_model_load_status_get(method, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);
}
END_TEST

START_TEST(children_load)
{
   _check_unload();

   eo_do(method, efl_model_children_load());
   efl_model_wait_for_load_status(method, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   check_efl_model_load_status_get(method, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   _test_method_children_count(method);
}
END_TEST

START_TEST(child_add)
{
   Eo *child;
   eo_do(method, child = efl_model_child_add());
   ck_assert_ptr_eq(NULL, child);
}
END_TEST

START_TEST(child_del)
{
   // efl_model_child_del always returns ERROR
   Eo *child = NULL;
   Efl_Model_Load_Status status;
   eo_do(method, status = efl_model_child_del(child));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);
}
END_TEST

START_TEST(call)
{
   check_efl_model_property_int_set(method, ARGUMENT_A, 12345678);
   check_efl_model_property_int_set(method, ARGUMENT_B, 87654321);

   Efl_Model_Load_Status status;
   eo_do(method, status = eldbus_model_method_call());
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);

   efl_model_wait_for_event(method, ELDBUS_MODEL_METHOD_EVENT_SUCCESSFUL_CALL);

   check_efl_model_property_int_eq(method, ARGUMENT_RESULT, 99999999);
}
END_TEST

void eldbus_test_eldbus_model_method(TCase *tc)
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
   tcase_add_test(tc, call);
}

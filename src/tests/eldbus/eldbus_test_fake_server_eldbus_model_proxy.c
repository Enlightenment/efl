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

static Eo *fake_server_object = NULL;
static Eo *fake_server_proxy = NULL;
static Eldbus_Service_Interface *fake_server = NULL;
static Fake_Server_Data fake_server_data = {0};

#define FAKE_SERVER_READONLY_PROPERTY_VALUE 1111
#define FAKE_SERVER_WRITEONLY_PROPERTY_VALUE 2222
#define FAKE_SERVER_READWRITE_PROPERTY_VALUE 3333

static void
_setup(void)
{
   fake_server_data = (Fake_Server_Data){
     .readonly_property = FAKE_SERVER_READONLY_PROPERTY_VALUE,
     .writeonly_property = FAKE_SERVER_WRITEONLY_PROPERTY_VALUE,
     .readwrite_property = FAKE_SERVER_READWRITE_PROPERTY_VALUE
   };
   char buf[1024];
   snprintf(buf, sizeof(buf), FAKE_SERVER_BUS ".%s", basename(__FILE__));
   fake_server = fake_server_start(&fake_server_data, buf);

   fake_server_object = efl_add(ELDBUS_MODEL_OBJECT_CLASS, efl_main_loop_get(),
                                eldbus_model_connect(efl_added, ELDBUS_CONNECTION_TYPE_SESSION, NULL, EINA_FALSE),
                                eldbus_model_object_bus_set(efl_added, buf),
                                eldbus_model_object_path_set(efl_added, FAKE_SERVER_PATH));
   ck_assert_ptr_ne(NULL, fake_server_object);

   fake_server_proxy = eldbus_model_proxy_from_object_get(fake_server_object, FAKE_SERVER_INTERFACE);
}

static void
_teardown(void)
{
   efl_del(fake_server_object);

   fake_server_stop(fake_server);
}

EFL_START_TEST(properties_get)
{
   Eina_Iterator *properties = efl_model_properties_get(fake_server_proxy);
   ck_assert_ptr_ne(NULL, properties);

   // FAKE_SERVER_READONLY_PROPERTY, FAKE_SERVER_WRITEONLY_PROPERTY and FAKE_SERVER_READWRITE_PROPERTY properties
   const unsigned int expected_properties_count = 3;
   unsigned int actual_properties_count = 0;
   const char *prop;

   EINA_ITERATOR_FOREACH(properties, prop)
     actual_properties_count++;
   eina_iterator_free(properties);
   ck_assert_int_eq(expected_properties_count, actual_properties_count);

   _teardown();
}
EFL_END_TEST

static Eina_Future *f = NULL;

static Eina_Value
_leave(void *data EINA_UNUSED, const Eina_Value v,
       const Eina_Future *dead EINA_UNUSED)
{
   ecore_main_loop_quit();
   f = NULL;
   return v;
}

static void
_property_ready(void *data EINA_UNUSED, const Efl_Event *ev)
{
   if (f) return ;
   f = efl_loop_job(efl_provider_find(ev->object, EFL_LOOP_CLASS));
   eina_future_then(f, _leave, NULL, NULL);
}

EFL_START_TEST(property_get)
{
   Eina_Value *value;

   efl_event_callback_add(fake_server_proxy, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _property_ready, NULL);
   value = efl_model_property_get(fake_server_proxy, FAKE_SERVER_READONLY_PROPERTY);
   eina_value_free(value);
   value = efl_model_property_get(fake_server_proxy, FAKE_SERVER_READWRITE_PROPERTY);
   eina_value_free(value);

   ecore_main_loop_begin();

   check_efl_model_property_int_eq(fake_server_proxy, FAKE_SERVER_READONLY_PROPERTY, FAKE_SERVER_READONLY_PROPERTY_VALUE);
   check_efl_model_property_int_eq(fake_server_proxy, FAKE_SERVER_READWRITE_PROPERTY, FAKE_SERVER_READWRITE_PROPERTY_VALUE);

   // Write-only property returns error
   value = efl_model_property_get(fake_server_proxy, FAKE_SERVER_WRITEONLY_PROPERTY);
   eina_value_free(value);
   //ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);

   _teardown();
}
EFL_END_TEST

static void
_check_property_set(const char *property_name, int expected_property_value, int *actual_property_value)
{
   Eina_Value value;
   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, expected_property_value);
   efl_model_property_set(fake_server_proxy, property_name, &value);
   eina_value_flush(&value);

   efl_model_wait_for_event(fake_server_proxy, EFL_MODEL_EVENT_PROPERTIES_CHANGED);

   ck_assert_int_eq(expected_property_value, *actual_property_value);
}

EFL_START_TEST(property_set)
{
   Eina_Future *future;
   Eina_Value dummy = EINA_VALUE_EMPTY;

   _check_property_set(FAKE_SERVER_WRITEONLY_PROPERTY, 0x12345678, &fake_server_data.writeonly_property);
   _check_property_set(FAKE_SERVER_READWRITE_PROPERTY, 0x76543210, &fake_server_data.readwrite_property);

   // Read-only property returns error
   future = efl_model_property_set(fake_server_proxy, FAKE_SERVER_READONLY_PROPERTY, &dummy);
   check_efl_model_future_error(future, &EFL_MODEL_ERROR_READ_ONLY);

   _teardown();
}
EFL_END_TEST

static void
_test_fake_server_proxy_children_count(Eo *efl_model)
{
   // 'Sum' and 'Ping' methods and 'Pong' signal
   check_efl_model_children_count_eq(efl_model, 3);
}

EFL_START_TEST(children_count)
{
   _test_fake_server_proxy_children_count(fake_server_proxy);

   _teardown();
}
EFL_END_TEST

EFL_START_TEST(children_slice_get)
{
   Eldbus_Model_Arguments *method1 = efl_model_nth_child_get(fake_server_proxy, 0);
   Eldbus_Model_Arguments *method2 = efl_model_nth_child_get(fake_server_proxy, 1);
   Eldbus_Model_Arguments *signal1 = efl_model_nth_child_get(fake_server_proxy, 2);

   const char *actual_method1_name = eldbus_model_arguments_arg_name_get(method1);
   const char *actual_method2_name = eldbus_model_arguments_arg_name_get(method2);
   const char *actual_signal1_name = eldbus_model_arguments_arg_name_get(signal1);

   ck_assert_ptr_ne(NULL, actual_method1_name);
   ck_assert_ptr_ne(NULL, actual_method2_name);
   ck_assert_ptr_ne(NULL, actual_signal1_name);

   // Eldbus doesn't have order for method names. Methods order are determined by Eina_Hash
   if (strcmp(FAKE_SERVER_SUM_METHOD_NAME, actual_method1_name) == 0)
     ck_assert(strcmp(FAKE_SERVER_PING_METHOD_NAME, actual_method2_name) == 0);
   else
     ck_assert(strcmp(FAKE_SERVER_SUM_METHOD_NAME, actual_method2_name) == 0);

   ck_assert(strcmp(FAKE_SERVER_PONG_SIGNAL_NAME, actual_signal1_name) == 0);

   _teardown();
}
EFL_END_TEST

EFL_START_TEST(child_add)
{
   Eo *child = efl_model_child_add(fake_server_proxy);
   ck_assert_ptr_eq(NULL, child);

   _teardown();
}
EFL_END_TEST

EFL_START_TEST(child_del)
{
   // Tests that it is not possible to delete children
   Eo *child;
   unsigned int expected_children_count = 0;
   unsigned int actual_children_count = 0;

   expected_children_count = efl_model_children_count_get(fake_server_proxy);
   ck_assert_msg(expected_children_count, "There must be at least 1 child to test");

   // efl_model_child_del always returns ERROR
   child = efl_model_first_child_get(fake_server_proxy);
   efl_model_child_del(fake_server_proxy, child);

   actual_children_count = efl_model_children_count_get(fake_server_proxy);
   ck_assert_int_le(expected_children_count, actual_children_count);

   _teardown();
}
EFL_END_TEST

void eldbus_test_fake_server_eldbus_model_proxy(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, NULL);
   tcase_add_test(tc, properties_get);
   tcase_add_test(tc, property_get);
   tcase_add_test(tc, property_set);
   tcase_add_test(tc, children_count);
   tcase_add_test(tc, children_slice_get);
   tcase_add_test(tc, child_add);
   tcase_add_test(tc, child_del);
}

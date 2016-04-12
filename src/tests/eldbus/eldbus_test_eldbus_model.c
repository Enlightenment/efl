#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include <Eina.h>
#include <Ecore.h>
#include <Eldbus.h>
#include <Eldbus_Model.h>

#include "eldbus_test_eldbus_model.h"
#include "eldbus_suite.h"

static Eina_Bool
_eo_event_quit_cb(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
   ecore_main_loop_quit();
   return EINA_FALSE;
}

void
efl_model_wait_for_event(Eo *obj, const Eo_Event_Description* event)
{
   eo_event_callback_add(obj, event, _eo_event_quit_cb, NULL);
   ecore_main_loop_begin();
   eo_event_callback_del(obj, event, _eo_event_quit_cb, NULL);
}

static Eina_Bool
_event_load_status_quit_cb(void *data, const Eo_Event *event)
{
   printf("_event_load_status_quit_cb\n");
   Efl_Model_Load_Status expected_status = (Efl_Model_Load_Status)data;
   Efl_Model_Load *actual_load = (Efl_Model_Load*)event->info;

   if (expected_status == actual_load->status)
     {
        ecore_main_loop_quit();
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

void
efl_model_wait_for_load_status(Efl_Model_Base *efl_model, Efl_Model_Load_Status expected_status)
{
   Efl_Model_Load_Status actual_status;
   actual_status = efl_model_load_status_get(efl_model);
   if (expected_status == actual_status)
     return;

   eo_event_callback_add(efl_model, EFL_MODEL_BASE_EVENT_LOAD_STATUS, _event_load_status_quit_cb, (void*)expected_status);
   ecore_main_loop_begin();
   eo_event_callback_del(efl_model, EFL_MODEL_BASE_EVENT_LOAD_STATUS, _event_load_status_quit_cb, (void*)expected_status);
}

Efl_Model_Base *
efl_model_nth_child_get(Efl_Model_Base *efl_model, unsigned int n)
{
   Eina_Accessor *accessor;
   Efl_Model_Load_Status status;
   status = efl_model_children_slice_get(efl_model, n, 1, &accessor);
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, accessor);
   Eo *child = NULL;
   Eina_Bool ret = eina_accessor_data_get(accessor, 0, (void**)&child);
   eina_accessor_free(accessor);
   ck_assert(ret);
   ck_assert_ptr_ne(NULL, child);
   return child;
}

Efl_Model_Base *
efl_model_first_child_get(Efl_Model_Base *efl_model)
{
   return efl_model_nth_child_get(efl_model, 1);
}

void
efl_model_load_and_wait_for_load_status(Eo *obj, Efl_Model_Load_Status expected_status)
{
   efl_model_load(obj);
   efl_model_wait_for_load_status(obj, expected_status);
}

static int expected_ret = 0;

void
check_init(void)
{
   ecore_init();
   expected_ret = eldbus_init();
   ck_assert_int_ge(expected_ret, 1);
}

void
check_shutdown(void)
{
   int ret = eldbus_shutdown();
   ck_assert_int_eq(ret, expected_ret - 1);
   ecore_shutdown();
}

void
check_property(Eo *object, const char *property_name, const char *expected_value)
{
   Eina_Value const* property_value;
   Efl_Model_Load_Status status;
   status = efl_model_property_get(object, property_name, &property_value);
   ck_assert_msg(EFL_MODEL_LOAD_STATUS_ERROR != status, "Nonexistent property: %s", property_name);
   ck_assert_ptr_ne(NULL, property_value);
   char *actual_value = eina_value_to_string(property_value);
   if (!actual_value)
     ck_assert_ptr_eq(expected_value, actual_value);
   else
     {
        bool is_property_equal = strcmp(expected_value, actual_value) == 0;
        ck_assert_msg(is_property_equal, "'%s' != '%s'", expected_value, actual_value);
        free(actual_value);
     }
}

Eo *
create_connection(void)
{
   Eo *connection = eo_add_ref(ELDBUS_MODEL_CONNECTION_CLASS, NULL, eldbus_model_connection_constructor(eo_self, ELDBUS_CONNECTION_TYPE_SESSION, NULL, EINA_FALSE));
   ck_assert_ptr_ne(NULL, connection);
   return connection;
}

Eo *
create_and_load_connection(void)
{
   Eo *connection = create_connection();
   efl_model_load_and_wait_for_load_status(connection, EFL_MODEL_LOAD_STATUS_LOADED);
   return connection;
}

Eo *
create_object(void)
{
   Eo *object = eo_add_ref(ELDBUS_MODEL_OBJECT_CLASS, NULL, eldbus_model_object_constructor(eo_self, ELDBUS_CONNECTION_TYPE_SESSION, NULL, EINA_FALSE, ELDBUS_FDO_BUS, ELDBUS_FDO_PATH));
   ck_assert_ptr_ne(NULL, object);
   return object;
}

Eo *
create_and_load_object(void)
{
   Eo *object = create_object();
   efl_model_load_and_wait_for_load_status(object, EFL_MODEL_LOAD_STATUS_LOADED);
   return object;
}

void
check_efl_model_load_status_get(Efl_Model_Base *efl_model, Efl_Model_Load_Status expected_load_status)
{
   Efl_Model_Load_Status actual_load_status;
   actual_load_status = efl_model_load_status_get(efl_model);
   ck_assert_int_eq(expected_load_status, actual_load_status);
}

void
check_efl_model_children_count_eq(Efl_Model_Base *efl_model, unsigned int expected_children_count)
{
   unsigned int actual_children_count = 0;
   efl_model_children_count_get(efl_model, &actual_children_count);
   ck_assert_int_eq(expected_children_count, actual_children_count);
}

void
check_efl_model_children_count_ge(Efl_Model_Base *efl_model, unsigned int minimum_children_count)
{
   unsigned int actual_children_count = 0;
   Efl_Model_Load_Status status;
   status = efl_model_children_count_get(efl_model, &actual_children_count);
   // A minimum count only exists if model have EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN
   ck_assert((EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN & status) == EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   ck_assert_int_ge(actual_children_count, minimum_children_count);
}

void
check_efl_model_children_slice_get(Efl_Model_Base *efl_model)
{
   unsigned int count = 0;
   Efl_Model_Load_Status status;
   status = efl_model_children_count_get(efl_model, &count);
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_msg(count, "There must be at least 1 child to test");

   // Test slice all
   Eina_Accessor *accessor;
   status = efl_model_children_slice_get(efl_model, 0, 0, &accessor);
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, accessor);
   // Get first child
   Eo *first_child = NULL;
   Eina_Bool ret = eina_accessor_data_get(accessor, 0, (void**)&first_child);
   ck_assert(ret);
   ck_assert_ptr_ne(NULL, first_child);
   // get last child
   Eo *last_child = NULL;
   ret = eina_accessor_data_get(accessor, count - 1, (void**)&last_child);
   ck_assert(ret);
   ck_assert_ptr_ne(NULL, last_child);
   // Test nonexistent child
   Eo *nonexistent_child = NULL;
   ret = eina_accessor_data_get(accessor, count, (void**)&nonexistent_child);
   ck_assert(!ret);
   ck_assert_ptr_eq(NULL, nonexistent_child);
   eina_accessor_free(accessor);

   // Test slice first child
   Eo *child = NULL;
   status = efl_model_children_slice_get(efl_model, 1, 1, &accessor);
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, accessor);
   ret = eina_accessor_data_get(accessor, 0, (void**)&child);
   ck_assert(ret);
   ck_assert_ptr_ne(NULL, child);
   ret = eina_accessor_data_get(accessor, 1, (void**)&child);
   ck_assert(!ret);
   ck_assert_ptr_eq(first_child, child);
   eina_accessor_free(accessor);

   // Test slice last child
   status = efl_model_children_slice_get(efl_model, count, 1, &accessor);
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, accessor);
   ret = eina_accessor_data_get(accessor, 0, (void**)&child);
   ck_assert(ret);
   ck_assert_ptr_ne(NULL, child);
   ret = eina_accessor_data_get(accessor, 1, (void**)&child);
   ck_assert(!ret);
   ck_assert_ptr_eq(last_child, child);
   eina_accessor_free(accessor);

   // Test slice nonexistent element
   status = efl_model_children_slice_get(efl_model, count + 1, 1, &accessor);
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_eq(NULL, accessor);
}

START_TEST(smoke)
{
   check_init();

   Eo *connection = create_and_load_connection();
   eo_unref(connection);

   check_shutdown();
}
END_TEST

START_TEST(object)
{
   check_init();

   Eo *root = create_object();

   efl_model_load_and_wait_for_load_status(root, EFL_MODEL_LOAD_STATUS_LOADED);

   eo_unref(root);

   check_shutdown();
}
END_TEST

START_TEST(proxy)
{
   check_init();

   Eo *root = create_object();

   efl_model_load_and_wait_for_load_status(root, EFL_MODEL_LOAD_STATUS_LOADED);

   Eina_Accessor *accessor = NULL;
   efl_model_children_slice_get(root, 0, 0, &accessor);
   ck_assert_ptr_ne(NULL, accessor);

   unsigned int i;
   Eo *proxy;
   EINA_ACCESSOR_FOREACH(accessor, i, proxy)
     {
       efl_model_load_and_wait_for_load_status(proxy, EFL_MODEL_LOAD_STATUS_LOADED);
     }
   eina_accessor_free(accessor);

   eo_unref(root);

   check_shutdown();
}
END_TEST

void
eldbus_test_eldbus_model(TCase *tc)
{
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, object);
   tcase_add_test(tc, proxy);
}

Eldbus_Model_Proxy *
eldbus_model_proxy_from_object_get(Eldbus_Model_Object *object, const char *interface_name)
{
   Eina_Accessor *accessor;
   Efl_Model_Load_Status status;
   status = efl_model_children_slice_get(object, 0, 0, &accessor);
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, accessor);

   Eo *proxy = NULL;
   unsigned int i;
   EINA_ACCESSOR_FOREACH(accessor, i, proxy)
     {
        const char *name;
        name = eldbus_model_proxy_name_get(proxy);
        ck_assert_ptr_ne(NULL, name);
        if (strcmp(name, interface_name) == 0)
          goto end;
     }
   proxy = NULL;

end:
   eina_accessor_free(accessor);
   return proxy;
}

static Eldbus_Model_Arguments *
_eldbus_model_arguments_from_proxy_get(Eldbus_Model_Proxy *proxy, const char *method_name, const Eo_Class *klass)
{
   Eina_Accessor *accessor;
   Efl_Model_Load_Status status;
   status = efl_model_children_slice_get(proxy, 0, 0, &accessor);
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, accessor);

   Eo *child = NULL;
   unsigned int i;
   EINA_ACCESSOR_FOREACH(accessor, i, child)
     {
        if (!eo_isa(child, klass))
          continue;

        const char *name;
        name = eldbus_model_arguments_name_get(child);
        ck_assert_ptr_ne(NULL, name);
        if (strcmp(name, method_name) == 0)
          goto end;
     }
   child = NULL;

end:
   eina_accessor_free(accessor);
   return child;
}

Eldbus_Model_Method *
eldbus_model_method_from_proxy_get(Eldbus_Model_Proxy *proxy, const char *method_name)
{
   return _eldbus_model_arguments_from_proxy_get(proxy, method_name, ELDBUS_MODEL_METHOD_CLASS);
}

Eldbus_Model_Signal *
eldbus_model_signal_from_proxy_get(Eldbus_Model_Proxy *proxy, const char *signal_name)
{
   return _eldbus_model_arguments_from_proxy_get(proxy, signal_name, ELDBUS_MODEL_SIGNAL_CLASS);
}

void
check_efl_model_property_int_eq(Efl_Model_Base *efl_model, const char *property, int expected_value)
{
   Eina_Value const* property_value;
   Efl_Model_Load_Status status;
   status = efl_model_property_get(efl_model, property, &property_value);
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, property_value);

   const Eina_Value_Type *property_type = eina_value_type_get(property_value);
   ck_assert_ptr_eq(EINA_VALUE_TYPE_INT, property_type);

   int actual_value = 0;
   eina_value_get(property_value, &actual_value);
   ck_assert_int_eq(expected_value, actual_value);
}

void
check_efl_model_property_int_set(Efl_Model_Base *efl_model, const char *property, int value)
{
   Eina_Value eina_value;
   eina_value_setup(&eina_value, EINA_VALUE_TYPE_INT);
   eina_value_set(&eina_value, value);
   Efl_Model_Load_Status status;
   status = efl_model_property_set(efl_model, property, &eina_value);
   eina_value_flush(&eina_value);
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
}


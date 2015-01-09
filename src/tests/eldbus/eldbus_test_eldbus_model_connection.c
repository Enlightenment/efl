#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eldbus_suite.h"
#include "eldbus_test_eldbus_model.h"

#include <Ecore.h>
#include <Eina.h>
#include <Eldbus_Model.h>

#include <stdbool.h>

static Eo *connection = NULL;
static Eo *unloaded_connection = NULL;

#define UNIQUE_NAME_PROPERTY "unique_name"

static void
_setup(void)
{
   check_init();
   connection = create_and_load_connection();
   unloaded_connection = create_connection();
}

static void
_teardown(void)
{
   eo_unref(unloaded_connection);
   eo_unref(connection);
   check_shutdown();
}

START_TEST(load_status_get)
{
   check_efl_model_load_status_get(connection, EFL_MODEL_LOAD_STATUS_LOADED);
   check_efl_model_load_status_get(unloaded_connection, EFL_MODEL_LOAD_STATUS_UNLOADED);
}
END_TEST

START_TEST(properties_get)
{
   Eina_Array *properties = NULL;
   Efl_Model_Load_Status status;
   eo_do(connection, status = efl_model_properties_get(&properties));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, properties);

   const unsigned int expected_properties_count = 1;
   unsigned int actual_properties_count = eina_array_count(properties);
   ck_assert_int_eq(expected_properties_count, actual_properties_count);

   // Unloaded connection populates its properties
   eo_do(unloaded_connection, status = efl_model_properties_get(&properties));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_UNLOADED, status);
   ck_assert_ptr_ne(NULL, properties);

   actual_properties_count = eina_array_count(properties);
   ck_assert_int_eq(expected_properties_count, actual_properties_count);
}
END_TEST

START_TEST(property_get)
{
   Eina_Value const* property_value;
   Efl_Model_Load_Status status;
   eo_do(connection, status = efl_model_property_get(UNIQUE_NAME_PROPERTY, &property_value));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, property_value);

   // Nonexistent property must return EFL_MODEL_LOAD_STATUS_ERROR
   eo_do(connection, status = efl_model_property_get("nonexistent", &property_value));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);
}
END_TEST

START_TEST(property_set)
{
   // Nonexistent property must return EFL_MODEL_LOAD_STATUS_ERROR
   Eina_Value value;
   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, 1);
   Efl_Model_Load_Status status;
   eo_do(connection, status = efl_model_property_set("nonexistent", &value));
   eina_value_flush(&value);
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);

   // UNIQUE_NAME_PROPERTY is read-only
   eo_do(connection, status = efl_model_property_set(UNIQUE_NAME_PROPERTY, &value));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);

   // The model must be loaded to be able to set its properties
   const char *expected_value = "unloaded";
   eina_value_setup(&value, EINA_VALUE_TYPE_STRING);
   eina_value_set(&value, expected_value);
   eo_do(unloaded_connection, status = efl_model_property_set(UNIQUE_NAME_PROPERTY, &value));
   eina_value_flush(&value);
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);
}
END_TEST

static void
_test_children_count(Eo *efl_model)
{
   // At least this connection <unique_name> and 'org.freedesktop.DBus' must exist
   check_efl_model_children_count_ge(efl_model, 2);
}

START_TEST(children_count)
{
   _test_children_count(connection);
}
END_TEST

START_TEST(children_slice_get)
{
   check_efl_model_children_slice_get(connection);

   // Unloaded connection must return EFL_MODEL_LOAD_STATUS_UNLOADED
   Eina_Accessor *accessor;
   Efl_Model_Load_Status status;
   eo_do(unloaded_connection, status = efl_model_children_slice_get(0, 0, &accessor));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_UNLOADED, status);
   ck_assert_ptr_eq(NULL, accessor);
}
END_TEST

START_TEST(unload)
{
   check_efl_model_load_status_get(connection, EFL_MODEL_LOAD_STATUS_LOADED);
   eo_do(connection, efl_model_unload());
   check_efl_model_load_status_get(connection, EFL_MODEL_LOAD_STATUS_UNLOADED);

   check_efl_model_children_count_eq(connection, 0);
}
END_TEST

START_TEST(properties_load)
{
   eo_do(unloaded_connection, efl_model_properties_load());
   check_efl_model_load_status_get(unloaded_connection, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);
}
END_TEST

START_TEST(children_load)
{
   eo_do(unloaded_connection, efl_model_children_load());

   check_efl_model_load_status_get(unloaded_connection, EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN);

   efl_model_wait_for_load_status(unloaded_connection, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   check_efl_model_load_status_get(unloaded_connection, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   _test_children_count(unloaded_connection);
}
END_TEST

START_TEST(child_add)
{
   Eo *child;
   eo_do(connection, child = efl_model_child_add());
   ck_assert_ptr_eq(NULL, child);
}
END_TEST

START_TEST(child_del)
{
   unsigned int expected_children_count = 0;
   Efl_Model_Load_Status status;
   eo_do(connection, status = efl_model_children_count_get(&expected_children_count));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);

   Eo *child = efl_model_first_child_get(connection);
   eo_do(connection, status = efl_model_child_del(child));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);

   unsigned int actual_children_count = 0;
   eo_do(connection, status = efl_model_children_count_get(&actual_children_count));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);

   ck_assert_int_le(expected_children_count, actual_children_count);
}
END_TEST

void eldbus_test_eldbus_model_connection(TCase *tc)
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
}

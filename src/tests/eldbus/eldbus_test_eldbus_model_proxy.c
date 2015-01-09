#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eldbus_suite.h"
#include "eldbus_test_eldbus_model.h"

#include <Ecore.h>
#include <Eina.h>
#include <Eldbus_Model.h>

#include <stdbool.h>

static Eo *dbus_object1 = NULL;
static Eo *dbus_object2 = NULL;
static Eo *dbus_proxy = NULL;
static Eo *unloaded_dbus_proxy = NULL;

static void
_setup(void)
{
   check_init();
   dbus_object1 = create_and_load_object();
   dbus_object2 = create_and_load_object();

   dbus_proxy = eldbus_model_proxy_from_object_get(dbus_object1, ELDBUS_FDO_INTERFACE);
   ck_assert_ptr_ne(NULL, dbus_proxy);
   efl_model_load_and_wait_for_load_status(dbus_proxy, EFL_MODEL_LOAD_STATUS_LOADED);

   unloaded_dbus_proxy = eldbus_model_proxy_from_object_get(dbus_object2, ELDBUS_FDO_INTERFACE);
   ck_assert_ptr_ne(NULL, dbus_proxy);
}

static void
_teardown(void)
{
   eo_unref(dbus_object2);
   eo_unref(dbus_object1);
   check_shutdown();
}

START_TEST(load_status_get)
{
   check_efl_model_load_status_get(dbus_proxy, EFL_MODEL_LOAD_STATUS_LOADED);
   check_efl_model_load_status_get(unloaded_dbus_proxy, EFL_MODEL_LOAD_STATUS_UNLOADED);
}
END_TEST

START_TEST(properties_get)
{
   // ELDBUS_FDO_INTERFACE have no properties
   Eina_Array *properties = NULL;
   Efl_Model_Load_Status status;
   eo_do(dbus_proxy, status = efl_model_properties_get(&properties));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, properties);
   ck_assert_int_eq(0, eina_array_count(properties));

   // Must be loaded to get the properties
   eo_do(unloaded_dbus_proxy, status = efl_model_properties_get(&properties));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);
}
END_TEST

START_TEST(property_get)
{
   // Nonexistent property must return EFL_MODEL_LOAD_STATUS_ERROR
   const Eina_Value* property_value;
   Efl_Model_Load_Status status;
   eo_do(dbus_proxy, status = efl_model_property_get("nonexistent", &property_value));
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
   eo_do(dbus_proxy, status = efl_model_property_set("nonexistent", &value));
   eina_value_flush(&value);
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);
}
END_TEST

static void
_test_dbus_proxy_children_count(Eo *efl_model)
{
   // 'org.freedesktop.DBus::AddMatch' and 'org.freedesktop.DBus::ListNames' at least
   check_efl_model_children_count_ge(efl_model, 2);
}

START_TEST(children_count)
{
   _test_dbus_proxy_children_count(dbus_proxy);
}
END_TEST

START_TEST(children_slice_get)
{
   check_efl_model_children_slice_get(dbus_proxy);

   // Unloaded dbus_proxy must return EFL_MODEL_LOAD_STATUS_UNLOADED
   Eina_Accessor *accessor;
   Efl_Model_Load_Status status;
   eo_do(unloaded_dbus_proxy, status = efl_model_children_slice_get(0, 0, &accessor));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_UNLOADED, status);
   ck_assert_ptr_eq(NULL, accessor);
}
END_TEST

START_TEST(unload)
{
   check_efl_model_load_status_get(dbus_proxy, EFL_MODEL_LOAD_STATUS_LOADED);
   eo_do(dbus_proxy, efl_model_unload());
   check_efl_model_load_status_get(dbus_proxy, EFL_MODEL_LOAD_STATUS_UNLOADED);

   check_efl_model_children_count_eq(dbus_proxy, 0);
}
END_TEST

START_TEST(properties_load)
{
   eo_do(unloaded_dbus_proxy, efl_model_properties_load());
   check_efl_model_load_status_get(unloaded_dbus_proxy, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);
}
END_TEST

START_TEST(children_load)
{
   eo_do(unloaded_dbus_proxy, efl_model_children_load());

   efl_model_wait_for_load_status(unloaded_dbus_proxy, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   check_efl_model_load_status_get(unloaded_dbus_proxy, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   _test_dbus_proxy_children_count(unloaded_dbus_proxy);
}
END_TEST

START_TEST(child_add)
{
   Eo *child;
   eo_do(dbus_proxy, child = efl_model_child_add());
   ck_assert_ptr_eq(NULL, child);
}
END_TEST

START_TEST(child_del)
{
   unsigned int expected_children_count = 0;
   Efl_Model_Load_Status status;
   eo_do(dbus_proxy, status = efl_model_children_count_get(&expected_children_count));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);

   Eo *child = efl_model_first_child_get(dbus_proxy);
   eo_do(dbus_proxy, status = efl_model_child_del(child));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);

   unsigned int actual_children_count = 0;
   eo_do(dbus_proxy, status = efl_model_children_count_get(&actual_children_count));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);

   ck_assert_int_le(expected_children_count, actual_children_count);
}
END_TEST

void eldbus_test_eldbus_model_proxy(TCase *tc)
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

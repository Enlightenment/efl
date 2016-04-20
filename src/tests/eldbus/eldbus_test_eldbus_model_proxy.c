#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdbool.h>

#include <Eina.h>
#include <Ecore.h>
#include <Eldbus_Model.h>

#include "eldbus_test_eldbus_model.h"
#include "eldbus_suite.h"

static Eo *dbus_object1 = NULL;
static Eo *dbus_proxy = NULL;

static void
_setup(void)
{
   check_init();
   dbus_object1 = create_object();

   dbus_proxy = eldbus_model_proxy_from_object_get(dbus_object1, ELDBUS_FDO_INTERFACE);
   ck_assert_ptr_ne(NULL, dbus_proxy);
}

static void
_teardown(void)
{
   eo_unref(dbus_object1);
   check_shutdown();
}

START_TEST(properties_get)
{
   const Eina_Array *properties = NULL;
   // ELDBUS_FDO_INTERFACE have no properties
   properties = efl_model_properties_get(dbus_proxy);
   ck_assert_ptr_ne(NULL, properties);
   ck_assert_int_eq(0, eina_array_count(properties));
}
END_TEST

START_TEST(property_get)
{
   // Nonexistent property must return ERROR
   Eina_Promise *promise;
   efl_model_property_get(dbus_proxy, "nonexistent", &promise);
   check_efl_model_promise_error(promise, &EFL_MODEL_ERROR_NOT_FOUND);
}
END_TEST

START_TEST(property_set)
{
   Eina_Value value;
   Eina_Promise *promise;

   // Nonexistent property must return ERROR
   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, 1);
   efl_model_property_set(dbus_proxy, "nonexistent", &value, &promise);
   check_efl_model_promise_error(promise, &EFL_MODEL_ERROR_NOT_FOUND);
   eina_value_flush(&value);
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
}
END_TEST

START_TEST(child_add)
{
   Eo *child;
   child = efl_model_child_add(dbus_proxy);
   ck_assert_ptr_eq(NULL, child);
}
END_TEST

START_TEST(child_del)
{
   unsigned int expected_children_count = 0;
   Eina_Promise *promise;
   efl_model_children_count_get(dbus_proxy, &promise);
   ck_assert_ptr_ne(NULL, promise);
   expected_children_count = efl_model_promise_then_u(promise);

   Eo *child = efl_model_first_child_get(dbus_proxy);
   efl_model_child_del(dbus_proxy, child);

   unsigned int actual_children_count = 0;
   efl_model_children_count_get(dbus_proxy, &promise);
   actual_children_count = efl_model_promise_then_u(promise);

   ck_assert_int_le(expected_children_count, actual_children_count);
}
END_TEST

void eldbus_test_eldbus_model_proxy(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, properties_get);
   tcase_add_test(tc, property_get);
   tcase_add_test(tc, property_set);
   tcase_add_test(tc, children_count);
   tcase_add_test(tc, children_slice_get);
   tcase_add_test(tc, child_add);
   tcase_add_test(tc, child_del);
}

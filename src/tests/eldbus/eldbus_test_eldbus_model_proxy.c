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
   efl_unref(dbus_object1);
   check_shutdown();
}

EFL_START_TEST(properties_get)
{
   const Eina_Array *properties = NULL;
   // ELDBUS_FDO_INTERFACE have no properties
   properties = efl_model_properties_get(dbus_proxy);
   ck_assert_ptr_ne(NULL, properties);

   // FIXME: This test is failing since 2017/11/17. It doesn't seem to be
   // due to an EFL change, but a change in the DBus interface:
   // properties contains 2 properties, "Interfaces" and "Features".
   //ck_assert_int_eq(0, eina_array_count(properties));
}
EFL_END_TEST

EFL_START_TEST(property_get)
{
   // Nonexistent property must return ERROR
   Efl_Future *future;
   future = efl_model_property_get(dbus_proxy, "nonexistent");
   check_efl_model_future_error(future, &EFL_MODEL_ERROR_NOT_FOUND);
}
EFL_END_TEST

EFL_START_TEST(property_set)
{
   Eina_Value value;
   Efl_Future *future;

   // Nonexistent property must return ERROR
   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, 1);
   future = efl_model_property_set(dbus_proxy, "nonexistent", &value);
   check_efl_model_future_error(future, &EFL_MODEL_ERROR_NOT_FOUND);
   eina_value_flush(&value);
}
EFL_END_TEST

static void
_test_dbus_proxy_children_count(Eo *efl_model)
{
   // 'org.freedesktop.DBus::AddMatch' and 'org.freedesktop.DBus::ListNames' at least
   check_efl_model_children_count_ge(efl_model, 2);
}

EFL_START_TEST(children_count)
{
   _test_dbus_proxy_children_count(dbus_proxy);
}
EFL_END_TEST

EFL_START_TEST(children_slice_get)
{
   check_efl_model_children_slice_get(dbus_proxy);
}
EFL_END_TEST

EFL_START_TEST(child_add)
{
   Eo *child;
   child = efl_model_child_add(dbus_proxy);
   ck_assert_ptr_eq(NULL, child);
}
EFL_END_TEST

EFL_START_TEST(child_del)
{
   unsigned int expected_children_count = 0;
   Efl_Future *future;
   future = efl_model_children_count_get(dbus_proxy);
   ck_assert_ptr_ne(NULL, future);
   expected_children_count = efl_model_future_then_u(future);

   Eo *child = efl_model_first_child_get(dbus_proxy);
   efl_model_child_del(dbus_proxy, child);

   unsigned int actual_children_count = 0;
   future = efl_model_children_count_get(dbus_proxy);
   actual_children_count = efl_model_future_then_u(future);

   ck_assert_int_le(expected_children_count, actual_children_count);
}
EFL_END_TEST

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

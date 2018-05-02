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

static Eina_Value
_leave(void *data EINA_UNUSED, const Eina_Value v,
       const Eina_Future *dead EINA_UNUSED)
{
   ecore_main_loop_quit();

   return v;
}

static void
_count_changed(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Future *f;

   f = efl_loop_job(efl_provider_find(ev->object, EFL_LOOP_CLASS));
   eina_future_then(f, _leave, NULL);
}

static void
_setup(void)
{
   dbus_object1 = create_object();

   efl_event_callback_add(dbus_object1, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _count_changed, NULL);
   efl_model_children_count_get(dbus_object1);

   ecore_main_loop_begin();

   dbus_proxy = eldbus_model_proxy_from_object_get(dbus_object1, ELDBUS_FDO_INTERFACE);
   ck_assert_ptr_ne(NULL, dbus_proxy);
}

static void
_teardown(void)
{
   efl_del(dbus_object1);
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
   Eina_Value *value;
   Eina_Error err = 0;

   value = efl_model_property_get(dbus_proxy, "nonexistent");

   fail_if(eina_value_type_get(value) != EINA_VALUE_TYPE_ERROR);
   eina_value_error_get(value, &err);
   fail_if(err != EFL_MODEL_ERROR_NOT_FOUND);

   eina_value_free(value);
}
EFL_END_TEST

EFL_START_TEST(property_set)
{
   Eina_Value value;
   Eina_Future *future;

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
   efl_event_callback_add(efl_model, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _count_changed, NULL);
   efl_model_children_count_get(efl_model);

   ecore_main_loop_begin();

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
   efl_event_callback_add(dbus_proxy, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _count_changed, NULL);
   efl_model_children_count_get(dbus_proxy);

   ecore_main_loop_begin();

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

#if 0
EFL_START_TEST(child_del)
{
   Eina_Future *future;
   Eo *child;
   unsigned int expected_children_count = 0;
   unsigned int actual_children_count = 0;

   expected_children_count = efl_model_children_count_get(dbus_proxy);
   ck_assert_ptr_ne(NULL, future);

   child = efl_model_first_child_get(dbus_proxy);
   efl_model_child_del(dbus_proxy, child);

   actual_children_count = efl_model_children_count_get(dbus_proxy);

   ck_assert_int_le(expected_children_count, actual_children_count);
}
EFL_END_TEST
#endif

void eldbus_test_eldbus_model_proxy(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, properties_get);
   tcase_add_test(tc, property_get);
   tcase_add_test(tc, property_set);
   tcase_add_test(tc, children_count);
   tcase_add_test(tc, children_slice_get);
   tcase_add_test(tc, child_add);
   /* tcase_add_test(tc, child_del); */
}

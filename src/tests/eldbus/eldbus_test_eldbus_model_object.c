#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdbool.h>

#include <Eina.h>
#include <Ecore.h>
#include <Eldbus_Model.h>

#include "eldbus_test_eldbus_model.h"
#include "eldbus_suite.h"

static Eo *object = NULL;

#define UNIQUE_NAME_PROPERTY "unique_name"

static void
_setup(void)
{
   check_init();
   object = create_object();
}

static void
_teardown(void)
{
   efl_unref(object);
   check_shutdown();
}

START_TEST(properties_get)
{
   const Eina_Array *properties = NULL;
   properties = efl_model_properties_get(object);
   ck_assert_ptr_ne(NULL, properties);

   const unsigned int expected_properties_count = 1;
   unsigned int actual_properties_count = eina_array_count(properties);
   ck_assert_int_eq(expected_properties_count, actual_properties_count);
}
END_TEST

START_TEST(property_get)
{
   Efl_Future *future;
   future = efl_model_property_get(object, UNIQUE_NAME_PROPERTY);
   efl_model_future_then(future);

   // Nonexistent property must raise ERROR
   future = NULL;
   future = efl_model_property_get(object, "nonexistent");
   check_efl_model_future_error(future, &EFL_MODEL_ERROR_NOT_FOUND);
}
END_TEST

START_TEST(property_set)
{
   Eina_Value value;
   Efl_Future *future;

   // Nonexistent property must raise ERROR
   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, 1);
   future = efl_model_property_set(object, "nonexistent", &value);
   check_efl_model_future_error(future, &EFL_MODEL_ERROR_NOT_FOUND);

   // UNIQUE_NAME_PROPERTY is read-only
   future = efl_model_property_set(object, UNIQUE_NAME_PROPERTY, &value);
   check_efl_model_future_error(future, &EFL_MODEL_ERROR_READ_ONLY);

   eina_value_flush(&value);
}
END_TEST

static void
_test_children_count(Eo *efl_model)
{
   // 'org.freedesktop.DBus' and 'org.freedesktop.DBus.Introspectable'
   check_efl_model_children_count_ge(efl_model, 2);
}

START_TEST(children_count)
{
   _test_children_count(object);
}
END_TEST

START_TEST(children_slice_get)
{
   check_efl_model_children_slice_get(object);
}
END_TEST

START_TEST(child_add)
{
   Eo *child;
   child = efl_model_child_add(object);
   ck_assert_ptr_eq(NULL, child);
}
END_TEST

START_TEST(child_del)
{
   unsigned int expected_children_count = 0;
   Efl_Future *future;
   future = efl_model_children_count_get(object);
   ck_assert_ptr_ne(NULL, future);
   expected_children_count = efl_model_future_then_u(future);
   ck_assert_msg(expected_children_count, "There must be at least 1 child to test");

   Eo *child = efl_model_first_child_get(object);
   efl_model_child_del(object, child);

   future = NULL;
   unsigned int actual_children_count = 0;
   future = efl_model_children_count_get(object);
   ck_assert_ptr_ne(NULL, future);
   actual_children_count = efl_model_future_then_u(future);
   ck_assert_int_le(expected_children_count, actual_children_count);
}
END_TEST

void eldbus_test_eldbus_model_object(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   /* tcase_add_test(tc, properties_get); */
   /* tcase_add_test(tc, property_get); */
   /* tcase_add_test(tc, property_set); */
   /* tcase_add_test(tc, children_count); */
   tcase_add_test(tc, children_slice_get);
   tcase_add_test(tc, child_add);
   tcase_add_test(tc, child_del);
}

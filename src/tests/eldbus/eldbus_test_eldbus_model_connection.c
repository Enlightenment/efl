#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdbool.h>

#include <Eina.h>
#include <Ecore.h>
#include <Eldbus_Model.h>

#include "eldbus_test_eldbus_model.h"
#include "eldbus_suite.h"

static Eo *connection = NULL;

#define UNIQUE_NAME_PROPERTY "unique_name"

static void
_setup(void)
{
   connection = create_connection();
}

static void
_teardown(void)
{
   efl_del(connection);
}

EFL_START_TEST(properties_get)
{
   const Eina_Array *properties = NULL;
   properties = efl_model_properties_get(connection);
   ck_assert_ptr_ne(NULL, properties);

   const unsigned int expected_properties_count = 1;
   unsigned int actual_properties_count = eina_array_count(properties);
   ck_assert_int_eq(expected_properties_count, actual_properties_count);
}
EFL_END_TEST

EFL_START_TEST(property_get)
{
   Eina_Value *v;

   v = efl_model_property_get(connection, UNIQUE_NAME_PROPERTY);
   fail_if(v == NULL);

   // Nonexistent property must raise ERROR
   v = efl_model_property_get(connection, "nonexistent");
   fail_if(v == NULL);
   fail_if(eina_value_type_get(v) != EINA_VALUE_TYPE_ERROR);
   /* fail_if(v != NULL || eina_value_type_get(v) != EINA_VALUE_TYPE_ERROR); */
}
EFL_END_TEST

static Eina_Value
_failed_property_set(void *data, const Eina_Value v,
                     const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Value *expected = data;

   fail_if(eina_value_type_get(expected) != eina_value_type_get(&v));

   if (eina_value_type_get(expected) == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error exerr = 0, goterr = 0;

        eina_value_error_get(expected, &exerr);
        eina_value_error_get(&v, &goterr);

        fail_if(exerr != goterr);
     }
   ecore_main_loop_quit();

   return v;
}

EFL_START_TEST(property_set)
{
   Eina_Value value;
   Eina_Future *future;

   // Nonexistent property must raise ERROR
   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, 1);
   future = efl_model_property_set(connection, "nonexistent", &value);
   eina_future_then(future, _failed_property_set, eina_value_error_new(EFL_MODEL_ERROR_NOT_FOUND));

   ecore_main_loop_begin();

   // UNIQUE_NAME_PROPERTY is read-only
   future = efl_model_property_set(connection, UNIQUE_NAME_PROPERTY, &value);
   eina_future_then(future, _failed_property_set, eina_value_error_new(EFL_MODEL_ERROR_READ_ONLY));

   ecore_main_loop_begin();

   eina_value_flush(&value);
}
EFL_END_TEST

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
_test_children_count(Eo *efl_model)
{
   // At least this connection <unique_name> and 'org.freedesktop.DBus' must exist
   efl_event_callback_add(efl_model, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _count_changed, NULL);
   if (!efl_model_children_count_get(efl_model))
     ecore_main_loop_begin();
   efl_event_callback_del(efl_model, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _count_changed, NULL);

   check_efl_model_children_count_ge(efl_model, 2);
}

EFL_START_TEST(children_count)
{
   _test_children_count(connection);
}
EFL_END_TEST

EFL_START_TEST(children_slice_get)
{
   efl_event_callback_add(connection, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _count_changed, NULL);
   if (!efl_model_children_count_get(connection))
     ecore_main_loop_begin();
   efl_event_callback_del(connection, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _count_changed, NULL);

   check_efl_model_children_slice_get(connection);
}
EFL_END_TEST

EFL_START_TEST(child_add)
{
   Eo *child;
   child = efl_model_child_add(connection);
   ck_assert_ptr_eq(NULL, child);
}
EFL_END_TEST

EFL_START_TEST(child_del)
{
   unsigned int expected_children_count = 0;
   unsigned int actual_children_count = 0;
   Eo *child;

   expected_children_count = efl_model_children_count_get(connection);

   child = efl_model_first_child_get(connection);
   efl_model_child_del(connection, child);

   actual_children_count = efl_model_children_count_get(connection);

   ck_assert_int_le(expected_children_count, actual_children_count);
}
EFL_END_TEST

void eldbus_test_eldbus_model_connection(TCase *tc)
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

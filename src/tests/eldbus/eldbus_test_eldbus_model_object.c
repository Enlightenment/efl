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
   object = create_object();
}

static void
_teardown(void)
{
   efl_del(object);
}

EFL_START_TEST(properties_get)
{
   Eina_Iterator *properties = NULL;
   properties = efl_model_properties_get(object);
   ck_assert_ptr_ne(NULL, properties);

   const unsigned int expected_properties_count = 1;
   unsigned int actual_properties_count = 0;
   const char *prop;

   EINA_ITERATOR_FOREACH(properties, prop)
     actual_properties_count++;
   eina_iterator_free(properties);
   ck_assert_int_eq(expected_properties_count, actual_properties_count);
}
EFL_END_TEST

EFL_START_TEST(property_get)
{
   Eina_Value *v;

   v = efl_model_property_get(object, UNIQUE_NAME_PROPERTY);
   fail_if(v == NULL);
   fail_if(eina_value_type_get(v) == EINA_VALUE_TYPE_ERROR);
   eina_value_free(v);

   // Nonexistent property must raise ERROR
   v = efl_model_property_get(object, "nonexistent");
   fail_if(v == NULL);
   fail_if(eina_value_type_get(v) != EINA_VALUE_TYPE_ERROR);
   eina_value_free(v);
}
EFL_END_TEST

static Eina_Value
_expect_error(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Error *expected = data;
   Eina_Error result = 0;

   fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ERROR);

   if (!expected) return v;

   eina_value_error_get(&v, &result);
   fail_if(result != *expected);

   ecore_main_loop_quit();

   return v;
}

EFL_START_TEST(property_set)
{
   Eina_Future *future;
   Eina_Value value = EINA_VALUE_EMPTY;

   // Nonexistent property must raise ERROR
   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, 1);

   future = efl_model_property_set(object, "nonexistent", &value);
   eina_future_then(future, _expect_error, &EFL_MODEL_ERROR_NOT_FOUND, NULL);

   ecore_main_loop_begin();

   // UNIQUE_NAME_PROPERTY is read-only
   future = efl_model_property_set(object, UNIQUE_NAME_PROPERTY, &value);
   eina_future_then(future, _expect_error, &EFL_MODEL_ERROR_READ_ONLY, NULL);

   ecore_main_loop_begin();

   eina_value_flush(&value);
}
EFL_END_TEST

static void
_test_children_count(Eo *efl_model)
{
   // 'org.freedesktop.DBus' and 'org.freedesktop.DBus.Introspectable'
   check_efl_model_children_count_ge(efl_model, 2);
}

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
   eina_future_then(f, _leave, NULL, NULL);
}

EFL_START_TEST(children_count)
{
   efl_event_callback_add(object, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _count_changed, NULL);
   efl_model_children_count_get(object);

   ecore_main_loop_begin();

   _test_children_count(object);
}
EFL_END_TEST

EFL_START_TEST(children_slice_get)
{
   efl_event_callback_add(object, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _count_changed, NULL);
   efl_model_children_count_get(object);

   ecore_main_loop_begin();

   check_efl_model_children_slice_get(object);
}
EFL_END_TEST

EFL_START_TEST(child_add)
{
   Eo *child;
   child = efl_model_child_add(object);
   ck_assert_ptr_eq(NULL, child);
}
EFL_END_TEST

#if 0
static Eina_Value
_one_child(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   Eo **child = data;

   fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY);
   fail_if(eina_value_array_count(&v) != 1);
   eina_value_array_get(&v, 0, &child);

   ecore_main_loop_quit();

   return v;
}

// FIXME: I don't know what the expected behavior for destroying a child of eldbus model object should be
EFL_START_TEST(child_del)
{
   Eina_Future *future;
   Eo *child = NULL;
   unsigned int expected_children_count = 0;
   unsigned int actual_children_count = 0;

   efl_event_callback_add(object, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _count_changed, NULL);
   efl_model_children_count_get(object);

   ecore_main_loop_begin();

   expected_children_count = efl_model_children_count_get(object);

   fail_if(expected_children_count == 0);

   future = efl_model_children_slice_get(object, 0, 1);
   eina_future_then(future, _one_child, &child, NULL);

   ecore_main_loop_begin();

   fail_if(!child);
   efl_model_child_del(object, child);

   actual_children_count = efl_model_children_count_get(object);
   ck_assert_int_le(expected_children_count, actual_children_count);
}
EFL_END_TEST
#endif

void eldbus_test_eldbus_model_object(TCase *tc)
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

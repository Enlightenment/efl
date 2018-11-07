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

static Eina_Value
_future_then_quit_u_cb(void *data,
                       const Eina_Value v,
                       const Eina_Future *dead_future EINA_UNUSED)
{
   unsigned *lhs = data;
   unsigned r = 0;

   eina_value_uint_get(&v, &r);

   *lhs = r;
   ecore_main_loop_quit();

   return v;
}

static Eina_Value
_future_then_cp(void *data ,
                const Eina_Value v,
                const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Value *value = data;

   eina_value_copy(&v, value);
   ecore_main_loop_quit();

   return v;
}


static Eina_Value
_future_check_err(void *data,
                  const Eina_Value v,
                  const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Error *expected_error = data;
   Eina_Error got = 0;

   fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ERROR);

   eina_value_error_get(&v, &got);

   if (expected_error)
     ck_assert_int_eq(got, *expected_error);

   ecore_main_loop_quit();

   return v;
}

static Eina_Value
_future_then_quit_cb(void* data,
                     const Eina_Value v,
                     const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Value **value = data;

   *value = eina_value_dup(&v);

   ecore_main_loop_quit();

   return v;
}

static void
_efl_event_quit_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   ecore_main_loop_quit();
   efl_event_callback_stop(event->object);
}

Eina_Value *
efl_model_future_then(Eina_Future *future)
{
   Eina_Value *data = NULL;
   eina_future_then(future, &_future_then_quit_cb, &data, NULL);
   ecore_main_loop_begin();
   return data;
}

void
check_efl_model_future_error(Eina_Future *future, Eina_Error *err)
{
   eina_future_then(future, &_future_check_err, err, NULL);
   ecore_main_loop_begin();
}

int
efl_model_future_then_u(Eina_Future *future)
{
   unsigned i = -1;
   eina_future_then(future, &_future_then_quit_u_cb, &i, NULL);
   ecore_main_loop_begin();
   return i;
}

void
efl_model_wait_for_event(Eo *obj, const Efl_Event_Description* event)
{
   efl_event_callback_add(obj, event, _efl_event_quit_cb, NULL);
   ecore_main_loop_begin();
   efl_event_callback_del(obj, event, _efl_event_quit_cb, NULL);
}

Efl_Model *
efl_model_nth_child_get(Efl_Model *efl_model, unsigned int n)
{
   Eina_Future *future;
   Eina_Value *array;
   Eo *child = NULL;
   unsigned int len, i;

   future = efl_model_children_slice_get(efl_model, n, 1);

   array = efl_model_future_then(future);
   fail_if(eina_value_type_get(array) != EINA_VALUE_TYPE_ARRAY);

   EINA_VALUE_ARRAY_FOREACH(array, len, i, child)
     ;

   child = efl_ref(child);
   eina_value_free(array);

   fail_if(!child);

   return child;
}

Efl_Model *
efl_model_first_child_get(Efl_Model *efl_model)
{
   return efl_model_nth_child_get(efl_model, 0);
}

void
check_property(Eo *object, const char *property_name, const char *expected_value)
{
   Eina_Value *value;
   char *actual_value;

   value = efl_model_property_get(object, property_name);
   actual_value = eina_value_to_string(value);
   if (!actual_value)
     ck_assert_ptr_eq(expected_value, actual_value);
   else
     {
        bool is_property_equal = strcmp(expected_value, actual_value) == 0;
        ck_assert_msg(is_property_equal, "'%s' != '%s'", expected_value, actual_value);
        free(actual_value);
     }
   eina_value_free(value);
}

Eo *
create_connection(void)
{
   Eo *connection = efl_add(ELDBUS_MODEL_CONNECTION_CLASS, efl_main_loop_get(),
                            eldbus_model_connect(efl_added, ELDBUS_CONNECTION_TYPE_SESSION, NULL, EINA_FALSE));
   ck_assert_ptr_ne(NULL, connection);
   return connection;
}

Eo *
create_object(void)
{
   Eo *object = efl_add(ELDBUS_MODEL_OBJECT_CLASS, efl_main_loop_get(),
                        eldbus_model_connect(efl_added, ELDBUS_CONNECTION_TYPE_SESSION, NULL, EINA_FALSE),
                        eldbus_model_object_bus_set(efl_added, ELDBUS_FDO_BUS),
                        eldbus_model_object_path_set(efl_added, ELDBUS_FDO_PATH));
   ck_assert_ptr_ne(NULL, object);
   return object;
}

void
check_efl_model_children_count_eq(Efl_Model *efl_model, unsigned int expected_children_count)
{
   unsigned int actual_children_count;

   actual_children_count = efl_model_children_count_get(efl_model);
   ck_assert_int_eq(expected_children_count, actual_children_count);
}

void
check_efl_model_children_count_ge(Efl_Model *efl_model, unsigned int minimum_children_count)
{
   unsigned int actual_children_count;

   actual_children_count = efl_model_children_count_get(efl_model);
   ck_assert_int_ge(actual_children_count, minimum_children_count);
}

void
check_efl_model_children_slice_get(Efl_Model *efl_model)
{
   unsigned count;
   Eina_Future *future;
   Eina_Value *array;
   Eo *first_child = NULL;
   Eo *last_child = NULL;
   Eo *nonexistent_child = NULL;
   Eo *child = NULL;

   count = efl_model_children_count_get(efl_model);
   ck_assert_msg((int)count > 0, "There must be at least 1 child to test");

   // Test slice all
   future = efl_model_children_slice_get(efl_model, 0, count);
   array = efl_model_future_then(future);
   ck_assert_ptr_ne(NULL, array);
   fail_if(eina_value_type_get(array) != EINA_VALUE_TYPE_ARRAY);

   // Get first child
   eina_value_array_get(array, 0, &first_child);
   ck_assert_ptr_ne(NULL, first_child);

   // get last child
   eina_value_array_get(array, count - 1, &last_child);
   ck_assert_ptr_ne(NULL, last_child);

   // Test nonexistent child
   eina_value_array_get(array, count, &nonexistent_child);
   ck_assert_ptr_eq(NULL, nonexistent_child);
   eina_value_free(array);

   // Test slice first child
   future = efl_model_children_slice_get(efl_model, 0, 1);
   array = efl_model_future_then(future);
   ck_assert_ptr_ne(NULL, array);
   eina_value_array_get(array, 0, &child);
   ck_assert_ptr_ne(NULL, child);
   eina_value_array_get(array, 1, &child);
   ck_assert_ptr_eq(first_child, child);
   eina_value_free(array);

   // Test slice last child
   future = efl_model_children_slice_get(efl_model, count - 1, 1);
   array = efl_model_future_then(future);
   ck_assert_ptr_ne(NULL, array);
   eina_value_array_get(array, 0, &child);
   ck_assert_ptr_ne(NULL, child);
   eina_value_array_get(array, 1, &child);
   ck_assert_ptr_eq(last_child, child);

   // Test slice nonexistent element
   future = efl_model_children_slice_get(efl_model, count, 1);
   ck_assert_ptr_ne(NULL, future);
   array = efl_model_future_then(future);
   fail_if(eina_value_type_get(array) != EINA_VALUE_TYPE_ERROR);
   eina_value_free(array);
}

EFL_START_TEST(smoke)
{
   Eo *connection = create_connection();
   efl_del(connection);
}
EFL_END_TEST

EFL_START_TEST(object)
{
   Eo *root = create_object();

   efl_del(root);

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
   eina_future_then(f, _leave, NULL, NULL);
}

static Eina_Future *
_async_slice_get(Efl_Model *model)
{
   efl_event_callback_add(model, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _count_changed, NULL);
   if (!efl_model_children_count_get(model))
     ecore_main_loop_begin();

   efl_event_callback_del(model, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _count_changed, NULL);

   return efl_model_children_slice_get(model, 0, efl_model_children_count_get(model));
}

EFL_START_TEST(proxy)
{
   Eina_Future *future = NULL;
   Eina_Value *array = NULL;
   Eo *proxy = NULL;
   Eo *root;
   unsigned int i, len;

   root = create_object();

   future = _async_slice_get(root);
   ck_assert_ptr_ne(NULL, future);

   array = efl_model_future_then(future);
   ck_assert_ptr_ne(NULL, array);

   EINA_VALUE_ARRAY_FOREACH(array, len, i, proxy)
     {
        ck_assert_ptr_ne(NULL, proxy);
        //efl_model_load_and_wait_for_load_status(proxy, EFL_MODEL_LOAD_STATUS_LOADED);
     }

   eina_value_free(array);

   efl_del(root);

}
EFL_END_TEST

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
   Eina_Future *future = NULL;
   Eo *proxy = NULL;
   Eina_Value *array;
   unsigned int len, i;

   future = _async_slice_get(object);
   array = efl_model_future_then(future);
   ck_assert_ptr_ne(NULL, array);

   EINA_VALUE_ARRAY_FOREACH(array, len, i, proxy)
     {
        const char *name;

        name = eldbus_model_proxy_name_get(proxy);
        ck_assert_ptr_ne(NULL, name);
        if (strcmp(name, interface_name) == 0)
          goto end;
     }
   proxy = NULL;

end:
   eina_value_free(array);
   return proxy;
}

static Eldbus_Model_Arguments *
_eldbus_model_arguments_from_proxy_get(Eldbus_Model_Proxy *proxy, const char *method_name, const Efl_Class *klass)
{
   Eina_Future *future = NULL;
   Eo *child = NULL;
   Eina_Value *array;
   unsigned int len, i;

   future = _async_slice_get(proxy);
   ck_assert_ptr_ne(NULL, future);
   array = efl_model_future_then(future);
   ck_assert_ptr_ne(NULL, array);

   EINA_VALUE_ARRAY_FOREACH(array, len, i, child)
     {
        const char *name;

        if (!efl_isa(child, klass))
          continue;

        name = eldbus_model_arguments_arg_name_get(child);
        ck_assert_ptr_ne(NULL, name);
        if (strcmp(name, method_name) == 0)
          goto end;
     }
   child = NULL;

end:
   eina_value_free(array);
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
check_efl_model_property_int_eq(Efl_Model *efl_model, const char *property, int expected_value)
{
   const Eina_Value_Type *property_type;
   Eina_Value *property_value;
   int actual_value = 0;

   property_value = efl_model_property_get(efl_model, property);
   property_type = eina_value_type_get(property_value);
   ck_assert_ptr_eq(EINA_VALUE_TYPE_INT, property_type);

   eina_value_get(property_value, &actual_value);
   ck_assert_int_eq(expected_value, actual_value);
   eina_value_free(property_value);
}

void
check_efl_model_property_int_set(Efl_Model *efl_model, const char *property, int value)
{
   Eina_Value eina_value = EINA_VALUE_EMPTY, value_ret = EINA_VALUE_EMPTY;
   Eina_Future *future;
   const Eina_Value_Type *property_type;

   eina_value_setup(&eina_value, EINA_VALUE_TYPE_INT);
   eina_value_set(&eina_value, value);
   future = efl_model_property_set(efl_model, property, &eina_value);

   eina_future_then(future, &_future_then_cp, &value_ret, NULL);
   ecore_main_loop_begin();

   property_type = eina_value_type_get(&value_ret);
   ck_assert_ptr_eq(EINA_VALUE_TYPE_INT, property_type);

   int actual_value;
   eina_value_get(&value_ret, &actual_value);
   ck_assert_int_eq(value, actual_value);
   eina_value_flush(&eina_value);
   eina_value_flush(&value_ret);
}

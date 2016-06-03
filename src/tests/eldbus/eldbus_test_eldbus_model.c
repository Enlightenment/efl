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

static void
_promise_then_quit_cb(void *data , void *value)
{
   *(void**)data = value;
   ecore_main_loop_quit();
}

static void
_promise_then_quit_u_cb(void *data , void *value)
{
   unsigned *lhs = data;
   unsigned *rhs = value;
   *lhs = *rhs;
   ecore_main_loop_quit();
}

static void
_promise_then_cp(void *data , void *value)
{
   eina_value_copy(value, data);
   ecore_main_loop_quit();
}


static void
_promise_check_err(void *data EINA_UNUSED, void *value EINA_UNUSED)
{
   ck_assert_msg(0, "Promise Expected Error:\n");
   ecore_main_loop_quit();
}

static void
_error_then_ok(void* data, Eina_Error error)
{
   Eina_Error const* expected_error = (Eina_Error*)data;
   if (data != NULL)
   {
     ck_assert_int_eq(error, *expected_error);
   }

   ecore_main_loop_quit();
}

static void
_error_then_cb(void* data EINA_UNUSED, Eina_Error error)
{
   ck_assert_msg(0,"Promise ERROR: %s\n", eina_error_msg_get(error));
   ecore_main_loop_quit();
}

static Eina_Bool
_eo_event_quit_cb(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
   ecore_main_loop_quit();
   return EINA_FALSE;
}

void *
efl_model_promise_then(Eina_Promise *promise)
{
   void *data = NULL;
   eina_promise_then(promise, &_promise_then_quit_cb, &_error_then_cb, &data);
   ecore_main_loop_begin();
   return data;
}

void
check_efl_model_promise_error(Eina_Promise *promise, Eina_Error *err)
{
   eina_promise_then(promise, &_promise_check_err, &_error_then_ok, err);
   ecore_main_loop_begin();
}

int
efl_model_promise_then_u(Eina_Promise *promise)
{
   unsigned i = 0;
   eina_promise_then(promise, &_promise_then_quit_u_cb, &_error_then_cb, &i);
   ecore_main_loop_begin();
   return i;
}

void
efl_model_wait_for_event(Eo *obj, const Eo_Event_Description* event)
{
   eo_event_callback_add(obj, event, _eo_event_quit_cb, NULL);
   ecore_main_loop_begin();
   eo_event_callback_del(obj, event, _eo_event_quit_cb, NULL);
}

Efl_Model *
efl_model_nth_child_get(Efl_Model *efl_model, unsigned int n)
{
   Eina_Accessor *accessor;
   Eina_Promise *promise;
   efl_model_children_slice_get(efl_model, n, 1, &promise);
   eina_promise_ref(promise);
   ck_assert_ptr_ne(NULL, promise);

   accessor = efl_model_promise_then(promise);
   ck_assert_ptr_ne(NULL, accessor);
   Eo *child = NULL;

   Eina_Bool ret = eina_accessor_data_get(accessor, 0, (void**)&child);
   eina_promise_unref(promise);
   ck_assert(ret);
   ck_assert_ptr_ne(NULL, child);
   return child;
}

Efl_Model *
efl_model_first_child_get(Efl_Model *efl_model)
{
   return efl_model_nth_child_get(efl_model, 1);
}

void
check_init(void)
{
   ecore_init();
   int ret = eldbus_init();
   ck_assert_int_ge(ret, 1);
}

void
check_shutdown(void)
{
   ecore_shutdown();
   int ret = eldbus_shutdown();
   ck_assert_int_eq(ret, 0);
}

void
check_property(Eo *object, const char *property_name, const char *expected_value)
{
   Eina_Promise *promise;
   efl_model_property_get(object, property_name, &promise);
   ck_assert_ptr_ne(NULL, promise);
   Eina_Value  *value = efl_model_promise_then(promise);
   char *actual_value;
   eina_value_get(value, &actual_value);
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
create_object(void)
{
   Eo *object = eo_add_ref(ELDBUS_MODEL_OBJECT_CLASS, NULL, eldbus_model_object_constructor(eo_self, ELDBUS_CONNECTION_TYPE_SESSION, NULL, EINA_FALSE, ELDBUS_FDO_BUS, ELDBUS_FDO_PATH));
   ck_assert_ptr_ne(NULL, object);
   return object;
}

void
check_efl_model_children_count_eq(Efl_Model *efl_model, unsigned int expected_children_count)
{
   unsigned int actual_children_count;
   Eina_Promise *promise;
   efl_model_children_count_get(efl_model, &promise);
   actual_children_count = efl_model_promise_then_u(promise);
   ck_assert_int_eq(expected_children_count, actual_children_count);
}

void
check_efl_model_children_count_ge(Efl_Model *efl_model, unsigned int minimum_children_count)
{
   unsigned int actual_children_count;
   Eina_Promise *promise;
   efl_model_children_count_get(efl_model, &promise);
   actual_children_count = efl_model_promise_then_u(promise);
   ck_assert_int_ge(actual_children_count, minimum_children_count);
}

void
check_efl_model_children_slice_get(Efl_Model *efl_model)
{
   unsigned count;
   Eina_Promise *promise;
   Eina_Accessor *accessor;
   efl_model_children_count_get(efl_model, &promise);
   count = efl_model_promise_then_u(promise);
   ck_assert_msg(count, "There must be at least 1 child to test");

   // Test slice all
   efl_model_children_slice_get(efl_model, 0, 0, &promise);
   eina_promise_ref(promise);
   accessor = efl_model_promise_then(promise);
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
   eina_promise_unref(promise);

   // Test slice first child
   Eo *child = NULL;
   efl_model_children_slice_get(efl_model, 1, 1, &promise);
   eina_promise_ref(promise);
   accessor = efl_model_promise_then(promise);
   ck_assert_ptr_ne(NULL, accessor);
   ret = eina_accessor_data_get(accessor, 0, (void**)&child);
   ck_assert(ret);
   ck_assert_ptr_ne(NULL, child);
   ret = eina_accessor_data_get(accessor, 1, (void**)&child);
   ck_assert(!ret);
   ck_assert_ptr_eq(first_child, child);
   eina_promise_unref(promise);

   // Test slice last child
   efl_model_children_slice_get(efl_model, count, 1, &promise);
   eina_promise_ref(promise);
   accessor = efl_model_promise_then(promise);
   ck_assert_ptr_ne(NULL, accessor);
   ret = eina_accessor_data_get(accessor, 0, (void**)&child);
   ck_assert(ret);
   ck_assert_ptr_ne(NULL, child);
   ret = eina_accessor_data_get(accessor, 1, (void**)&child);
   ck_assert(!ret);
   ck_assert_ptr_eq(last_child, child);
   eina_promise_unref(promise);

   // Test slice nonexistent element
   efl_model_children_slice_get(efl_model, count + 1, 1, &promise);
   eina_promise_ref(promise);
   ck_assert_ptr_ne(NULL, promise);
   accessor = efl_model_promise_then(promise);
   ck_assert_ptr_eq(NULL, accessor);
   eina_promise_unref(promise);
}

START_TEST(smoke)
{
   check_init();

   Eo *connection = create_connection();
   eo_unref(connection);

   check_shutdown();
}
END_TEST

START_TEST(object)
{
   check_init();

   Eo *root = create_object();

   eo_unref(root);

   check_shutdown();
}
END_TEST

START_TEST(proxy)
{
   check_init();

   Eo *root = create_object();

   Eina_Accessor *accessor = NULL;
   Eina_Promise *promise = NULL;
   efl_model_children_slice_get(root, 0, 0, &promise);
   eina_promise_ref(promise);
   ck_assert_ptr_ne(NULL, promise);

   accessor = efl_model_promise_then(promise);
   ck_assert_ptr_ne(NULL, accessor);

   unsigned int i;
   Eo *proxy;
   EINA_ACCESSOR_FOREACH(accessor, i, proxy)
     {
        ck_assert_ptr_ne(NULL, proxy);
       //efl_model_load_and_wait_for_load_status(proxy, EFL_MODEL_LOAD_STATUS_LOADED);
     }
   eina_promise_unref(promise);

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
   Eina_Promise *promise = NULL;
   efl_model_children_slice_get(object, 0, 0, &promise);
   ck_assert_ptr_ne(NULL, promise);
   eina_promise_ref(promise);
   accessor = efl_model_promise_then(promise);
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
   eina_promise_unref(promise);
   return proxy;
}

static Eldbus_Model_Arguments *
_eldbus_model_arguments_from_proxy_get(Eldbus_Model_Proxy *proxy, const char *method_name, const Eo_Class *klass)
{
   Eina_Accessor *accessor;
   Eina_Promise *promise = NULL;
   efl_model_children_slice_get(proxy, 0, 0, &promise);
   ck_assert_ptr_ne(NULL, promise);
   eina_promise_ref(promise);
   accessor = efl_model_promise_then(promise);
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
   eina_promise_unref(promise);
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
   Eina_Value property_value;
   Eina_Promise *promise;
   efl_model_property_get(efl_model, property, &promise);
   ck_assert_ptr_ne(NULL, promise);

   eina_promise_then(promise, &_promise_then_cp, &_error_then_cb, &property_value);
   ecore_main_loop_begin();

   const Eina_Value_Type *property_type = eina_value_type_get(&property_value);
   ck_assert_ptr_eq(EINA_VALUE_TYPE_INT, property_type);

   int actual_value = 0;
   eina_value_get(&property_value, &actual_value);
   ck_assert_int_eq(expected_value, actual_value);
   eina_value_flush(&property_value);
}

void
check_efl_model_property_int_set(Efl_Model *efl_model, const char *property, int value)
{
   Eina_Value eina_value, value_ret;
   Eina_Promise *promise;

   eina_value_setup(&eina_value, EINA_VALUE_TYPE_INT);
   eina_value_set(&eina_value, value);
   efl_model_property_set(efl_model, property, &eina_value, &promise);

   eina_promise_then(promise, &_promise_then_cp, &_error_then_cb, &value_ret);
   ecore_main_loop_begin();

   const Eina_Value_Type *property_type = eina_value_type_get(&value_ret);
   ck_assert_ptr_eq(EINA_VALUE_TYPE_INT, property_type);

   int actual_value;
   eina_value_get(&value_ret, &actual_value);
   ck_assert_int_eq(value, actual_value);
   eina_value_flush(&eina_value);
}


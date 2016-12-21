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
_future_then_quit_cb(void *data , Efl_Event const *event)
{
   Efl_Future_Event_Success* info = event->info;
   *(void**)data = info->value;
   ecore_main_loop_quit();
}

static void
_future_then_quit_u_cb(void *data , Efl_Event const *event)
{
   unsigned *lhs = data;
   unsigned *rhs = ((Efl_Future_Event_Success*)event->info)->value;;
   *lhs = *rhs;
   ecore_main_loop_quit();
}

static void
_future_then_cp(void *data , Efl_Event const *event)
{
   Eina_Value* value = ((Efl_Future_Event_Success*)event->info)->value;
   eina_value_copy(value, data);
   ecore_main_loop_quit();
}


static void
_future_check_err(void *data EINA_UNUSED, Efl_Event const *value EINA_UNUSED)
{
   ck_assert_msg(0, "Future Expected Error:\n");

   ecore_main_loop_quit();
}

static void
_error_then_ok(void* data, Efl_Event const* event)
{
   Efl_Future_Event_Failure* info = event->info;
   Eina_Error const** expected_error = (Eina_Error const**)data;

   if (*expected_error)
     ck_assert_int_eq(info->error, **expected_error);
   ecore_main_loop_quit();
}

static void
_error_then_cb(void* data EINA_UNUSED, Efl_Event const* event)
{
   Efl_Future_Event_Failure* info = event->info;
   ck_assert_msg(0,"Future ERROR: %s\n", eina_error_msg_get(info->error));
   ecore_main_loop_quit();
}

static void
_efl_event_quit_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   ecore_main_loop_quit();
   efl_event_callback_stop(event->object);
}

void *
efl_model_future_then(Efl_Future *future)
{
   void *data = NULL;
   efl_future_then(future, &_future_then_quit_cb, &_error_then_cb, NULL, &data);
   ecore_main_loop_begin();
   return data;
}

void
check_efl_model_future_error(Efl_Future *future, Eina_Error *err)
{
   efl_future_then(future, &_future_check_err, &_error_then_ok, NULL, &err);
   ecore_main_loop_begin();
}

int
efl_model_future_then_u(Efl_Future *future)
{
   unsigned i = -1;
   efl_future_then(future, &_future_then_quit_u_cb, &_error_then_cb, NULL, &i);
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
   Eina_Accessor *accessor;
   Efl_Future *future;

   future = efl_model_children_slice_get(efl_model, n, 1);
   efl_ref(future);
   ck_assert_ptr_ne(NULL, future);

   accessor = efl_model_future_then(future);
   ck_assert_ptr_ne(NULL, accessor);
   Eo *child = NULL;

   Eina_Bool ret = eina_accessor_data_get(accessor, 0, (void**)&child);
   efl_unref(future);
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
   Efl_Future *future;
   future = efl_model_property_get(object, property_name);
   ck_assert_ptr_ne(NULL, future);
   Eina_Value  *value = efl_model_future_then(future);
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
   Eo *connection = efl_add_ref(ELDBUS_MODEL_CONNECTION_CLASS, ecore_main_loop_get(), eldbus_model_connection_constructor(efl_added, ELDBUS_CONNECTION_TYPE_SESSION, NULL, EINA_FALSE));
   ck_assert_ptr_ne(NULL, connection);
   return connection;
}

Eo *
create_object(void)
{
   Eo *object = efl_add_ref(ELDBUS_MODEL_OBJECT_CLASS, ecore_main_loop_get(), eldbus_model_object_constructor(efl_added, ELDBUS_CONNECTION_TYPE_SESSION, NULL, EINA_FALSE, ELDBUS_FDO_BUS, ELDBUS_FDO_PATH));
   ck_assert_ptr_ne(NULL, object);
   return object;
}

void
check_efl_model_children_count_eq(Efl_Model *efl_model, unsigned int expected_children_count)
{
   unsigned int actual_children_count;
   Efl_Future *future;
   future = efl_model_children_count_get(efl_model);
   actual_children_count = efl_model_future_then_u(future);
   ck_assert_int_eq(expected_children_count, actual_children_count);
}

void
check_efl_model_children_count_ge(Efl_Model *efl_model, unsigned int minimum_children_count)
{
   unsigned int actual_children_count;
   Efl_Future *future;
   future = efl_model_children_count_get(efl_model);
   actual_children_count = efl_model_future_then_u(future);
   ck_assert_int_ge(actual_children_count, minimum_children_count);
}

void
check_efl_model_children_slice_get(Efl_Model *efl_model)
{
   unsigned count;
   Efl_Future *future;
   Eina_Accessor *accessor;

   future = efl_model_children_count_get(efl_model);
   count = efl_model_future_then_u(future);
   ck_assert_msg((int)count > 0, "There must be at least 1 child to test");

   // Test slice all
   future = efl_model_children_slice_get(efl_model, 0, 0);
   efl_ref(future);
   accessor = efl_model_future_then(future);
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
   efl_unref(future);

   // Test slice first child
   Eo *child = NULL;
   future = efl_model_children_slice_get(efl_model, 1, 1);
   efl_ref(future);
   accessor = efl_model_future_then(future);
   ck_assert_ptr_ne(NULL, accessor);
   ret = eina_accessor_data_get(accessor, 0, (void**)&child);
   ck_assert(ret);
   ck_assert_ptr_ne(NULL, child);
   ret = eina_accessor_data_get(accessor, 1, (void**)&child);
   ck_assert(!ret);
   ck_assert_ptr_eq(first_child, child);
   efl_unref(future);

   // Test slice last child
   future = efl_model_children_slice_get(efl_model, count, 1);
   efl_ref(future);
   accessor = efl_model_future_then(future);
   ck_assert_ptr_ne(NULL, accessor);
   ret = eina_accessor_data_get(accessor, 0, (void**)&child);
   ck_assert(ret);
   ck_assert_ptr_ne(NULL, child);
   ret = eina_accessor_data_get(accessor, 1, (void**)&child);
   ck_assert(!ret);
   ck_assert_ptr_eq(last_child, child);
   efl_unref(future);

   // Test slice nonexistent element
   future = efl_model_children_slice_get(efl_model, count + 1, 1);
   efl_ref(future);
   ck_assert_ptr_ne(NULL, future);
   accessor = efl_model_future_then(future);
   ck_assert_ptr_eq(NULL, accessor);
   efl_unref(future);
}

START_TEST(smoke)
{
   check_init();

   Eo *connection = create_connection();
   efl_unref(connection);

   check_shutdown();
}
END_TEST

START_TEST(object)
{
   check_init();

   Eo *root = create_object();

   efl_unref(root);

   check_shutdown();
}
END_TEST

START_TEST(proxy)
{
   check_init();

   Eo *root = create_object();

   Eina_Accessor *accessor = NULL;
   Efl_Future *future = NULL;
   future = efl_model_children_slice_get(root, 0, 0);
   efl_ref(future);
   ck_assert_ptr_ne(NULL, future);

   accessor = efl_model_future_then(future);
   ck_assert_ptr_ne(NULL, accessor);

   unsigned int i;
   Eo *proxy;
   EINA_ACCESSOR_FOREACH(accessor, i, proxy)
     {
        ck_assert_ptr_ne(NULL, proxy);
       //efl_model_load_and_wait_for_load_status(proxy, EFL_MODEL_LOAD_STATUS_LOADED);
     }
   efl_unref(future);

   efl_unref(root);

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
   Efl_Future *future = NULL;
   future = efl_model_children_slice_get(object, 0, 0);
   ck_assert_ptr_ne(NULL, future);
   efl_ref(future);
   accessor = efl_model_future_then(future);
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
   efl_unref(future);
   return proxy;
}

static Eldbus_Model_Arguments *
_eldbus_model_arguments_from_proxy_get(Eldbus_Model_Proxy *proxy, const char *method_name, const Efl_Class *klass)
{
   Eina_Accessor *accessor;
   Efl_Future *future = NULL;
   future = efl_model_children_slice_get(proxy, 0, 0);
   ck_assert_ptr_ne(NULL, future);
   efl_ref(future);
   accessor = efl_model_future_then(future);
   ck_assert_ptr_ne(NULL, accessor);

   Eo *child = NULL;
   unsigned int i;
   EINA_ACCESSOR_FOREACH(accessor, i, child)
     {
        if (!efl_isa(child, klass))
          continue;

        const char *name;
        name = eldbus_model_arguments_name_get(child);
        ck_assert_ptr_ne(NULL, name);
        if (strcmp(name, method_name) == 0)
          goto end;
     }
   child = NULL;

end:
   efl_unref(future);
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
   Efl_Future *future;
   future = efl_model_property_get(efl_model, property);
   ck_assert_ptr_ne(NULL, future);

   efl_future_then(future, &_future_then_cp, &_error_then_cb, NULL, &property_value);
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
   Efl_Future *future;

   eina_value_setup(&eina_value, EINA_VALUE_TYPE_INT);
   eina_value_set(&eina_value, value);
   future = efl_model_property_set(efl_model, property, &eina_value);

   efl_future_then(future, &_future_then_cp, &_error_then_cb, NULL, &value_ret);
   ecore_main_loop_begin();

   const Eina_Value_Type *property_type = eina_value_type_get(&value_ret);
   ck_assert_ptr_eq(EINA_VALUE_TYPE_INT, property_type);

   int actual_value;
   eina_value_get(&value_ret, &actual_value);
   ck_assert_int_eq(value, actual_value);
   eina_value_flush(&eina_value);
}

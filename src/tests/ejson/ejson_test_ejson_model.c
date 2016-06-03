#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ejson_test_ejson_model.h"
#include "ejson_suite.h"

#include <Ecore.h>
#include <Eina.h>
#include <Ejson.h>

#include <stdbool.h>

static void
_setup(void)
{
   int ret = ejson_init();
   ck_assert_int_ge(ret, 1);
}

static void
_teardown(void)
{
   int ret = ejson_shutdown();
   ck_assert_int_eq(ret, 0);
}


void _name_set_ok(void *data, void *value)
{
    const char *given_name = (const char *)data;
    Eina_Value *stored_value = (Eina_Value *)value;
    const char *stored_name = NULL;
    eina_value_get(stored_value, &stored_name);

    ck_assert(given_name);
    ck_assert(stored_name);
    ck_assert_str_eq(given_name, stored_name);
    ecore_main_loop_quit();
}

void _prop_set_error(void *data, Eina_Error error EINA_UNUSED)
{
    ERR("Error!");
    const char *v = data;
    ck_abort_msg("Failed to set value %s on some property", v);
    ecore_main_loop_quit();
}

static void
_set_name_property(Eo *model, const char *v)
{
   Eina_Value name;
   eina_value_setup(&name, EINA_VALUE_TYPE_STRING);
   eina_value_set(&name, v);
   Eina_Promise *promise;
   efl_model_property_set(model, EJSON_MODEL_NAME_PROPERTY, &name, &promise);
   eina_promise_then(promise, _name_set_ok, _prop_set_error, (void*) v);
   ecore_main_loop_begin();
   eina_value_flush(&name);
}

static void
_int_value_set_ok(void *data, void *value)
{
   int *given_value = (int*) data;
   Eina_Value *stored_generic_value = (Eina_Value *)value;
   int stored_value = -1;
   eina_value_get(stored_generic_value, &stored_value);
   ck_assert_int_eq(*given_value, stored_value);
   ecore_main_loop_quit();
}

static void
_set_int_value_property(Eo *model, int *v)
{
   Eina_Value value;
   Eina_Promise *promise;

   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, *v);

   efl_model_property_set(model, EJSON_MODEL_VALUE_PROPERTY, &value, &promise);
   eina_promise_then(promise, _int_value_set_ok, _prop_set_error, (void *) v);
   ecore_main_loop_begin();
   eina_value_flush(&value);
}

static void
_check_value_type_cannot_have_children(Eo *model)
{
   Eo *child = efl_model_child_add(model);
   ck_assert_ptr_eq(NULL, child);
}

static void
_check_int_value_property_set(Eo *model, int64_t v)
{
   Eina_Value value;
   Eina_Value *ret_value = NULL;
   Eina_Promise *promise = NULL;
   int64_t x = 0;

   eina_value_setup(&value, EINA_VALUE_TYPE_INT64);
   eina_value_set(&value, v);

   efl_model_property_set(model, EJSON_MODEL_VALUE_PROPERTY, &value, &promise);
   ck_assert_ptr_ne(NULL, promise);

   ck_assert_int_eq(eina_promise_error_get(promise), 0);

   ret_value = eina_promise_value_get(promise);
   eina_value_get(ret_value, &x);

   ck_assert_int_eq(x, v);

   eina_promise_unref(promise);
   eina_value_flush(&value);
}

static void
_check_bool_value_property_set(Eo *model, bool v)
{
   Eina_Value value;
   Eina_Value *ret_value = NULL;
   Eina_Promise *promise;
   int x = 0;

   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, (int)v);

   efl_model_property_set(model, EJSON_MODEL_VALUE_PROPERTY, &value, &promise);

   ck_assert_int_eq(eina_promise_error_get(promise), 0);

   ret_value = eina_promise_value_get(promise);
   eina_value_get(ret_value, &x);

   ck_assert_int_eq(x, v);

   eina_promise_unref(promise);
   eina_value_flush(&value);
}

static void
_check_string_value_property_set(Eo *model, const char *v)
{
   Eina_Value value;
   Eina_Value *ret_value = NULL;
   Eina_Promise *promise;
   const char *x = NULL;

   eina_value_setup(&value, EINA_VALUE_TYPE_STRING);
   eina_value_set(&value, v);

   efl_model_property_set(model, EJSON_MODEL_VALUE_PROPERTY, &value, &promise);

   ck_assert_int_eq(eina_promise_error_get(promise), 0);

   ret_value = eina_promise_value_get(promise);
   eina_value_get(ret_value, &x);

   ck_assert_str_eq(x, v);

   eina_promise_unref(promise);
   eina_value_flush(&value);
}

static void
_check_efl_model_children_count_eq(Eo *model, unsigned int expected_children_count)
{
   Eina_Promise *promise = NULL;
   unsigned int *actual_children_count = NULL;
   efl_model_children_count_get(model, &promise);

   ck_assert_int_eq(eina_promise_error_get(promise), 0);

   actual_children_count = eina_promise_value_get(promise);

   ck_assert_int_eq(expected_children_count, *actual_children_count);

   eina_promise_unref(promise);
}

static void
_check_efl_model_property_int_eq(Eo *emodel, const char *property, int64_t expected_value)
{
   const Eina_Value *value;
   Eina_Promise *promise;

   efl_model_property_get(emodel, property, &promise);
   ck_assert_ptr_ne(NULL, promise);

   ck_assert_int_eq(eina_promise_error_get(promise), 0);

   value = eina_promise_value_get(promise);

   ck_assert_ptr_ne(NULL, value);

   const Eina_Value_Type *property_type = eina_value_type_get(value);
   ck_assert_ptr_eq(EINA_VALUE_TYPE_INT64, property_type);

   int64_t actual_value = 0;
   eina_value_get(value, &actual_value);
   ck_assert_int_eq(expected_value, actual_value);

   eina_promise_unref(promise);
}

static void
_check_efl_model_property_str_eq(Eo *emodel, const char *property, const char *expected_value)
{
   const Eina_Value *value;
   Eina_Promise *promise;

   efl_model_property_get(emodel, property, &promise);
   ck_assert_ptr_ne(NULL, promise);

   ck_assert_int_eq(eina_promise_error_get(promise), 0);

   value = eina_promise_value_get(promise);

   ck_assert_ptr_ne(NULL, value);

   const Eina_Value_Type *property_type = eina_value_type_get(value);
   ck_assert_ptr_eq(EINA_VALUE_TYPE_STRING, property_type);

   const char *actual_value = NULL;
   eina_value_get(value, &actual_value);
   ck_assert_str_eq(expected_value, actual_value);

   eina_promise_unref(promise);
}

static Eo *
_emodel_nth_child_get(Eo *emodel, unsigned int n)
{
   Eina_Promise *promise = NULL;
   Eina_Accessor *accessor;
   Eo *child = NULL;

   efl_model_children_slice_get(emodel, n, 1, &promise);

   ck_assert_ptr_ne(NULL, promise);
   ck_assert_int_eq(eina_promise_error_get(promise), 0);

   accessor = eina_promise_value_get(promise);

   ck_assert_ptr_ne(NULL, accessor);

   Eina_Bool ret = eina_accessor_data_get(accessor, 0, (void**)&child);
   ck_assert(ret);
   ck_assert_ptr_ne(NULL, child);

   eina_promise_unref(promise);

   return child;
}

static void
_check_ejson_type(Eo *model, Ejson_Model_Type expected_type)
{
   Ejson_Model_Type actual_type = ejson_model_type_get(model);
   ck_assert_int_eq(expected_type, actual_type);
}


static void
_check_efl_model_properties(Eo *model, const char *expected_properties[])
{
   const Eina_Array *properties = NULL;
   properties = efl_model_properties_get(model);
   ck_assert_ptr_ne(NULL, properties);

   unsigned int actual_properties_count = eina_array_count(properties);

   // This assumes a fixed order for the properties.
   unsigned int expected_properties_count = 0;
   const char *expected_property = NULL;
   while ((expected_property = *expected_properties++))
     {
        const char *actual_property = eina_array_data_get(properties,
                                                          expected_properties_count);
        ck_assert_str_eq(expected_property, actual_property);
        ++expected_properties_count;
        ck_assert_int_le(expected_properties_count, actual_properties_count);
     }

   ck_assert_int_eq(expected_properties_count, actual_properties_count);
}

START_TEST(smoke)
{
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL);
   ck_assert_ptr_ne(NULL, json);
   eo_unref(json);
}
END_TEST

START_TEST(object_constructor)
{
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL,
                         ejson_model_type_set(eo_self, EJSON_MODEL_TYPE_OBJECT));
   ck_assert_ptr_ne(NULL, json);
   ck_assert_int_eq(EJSON_MODEL_TYPE_OBJECT, ejson_model_type_get(json));

   eo_unref(json);
}
END_TEST

START_TEST(object_get_null_value)
{
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL);
   ck_assert_ptr_ne(NULL, json);

   Eina_Promise *promise = NULL;
   efl_model_property_get(json, EJSON_MODEL_VALUE_PROPERTY, &promise);

   // Check below based on returning a null string eina value if not value was
   // set.
   Eina_Value *value = eina_promise_value_get(promise);
   ck_assert_ptr_eq(EINA_VALUE_TYPE_OPTIONAL, eina_value_type_get(value));
   Eina_Bool empty = EINA_FALSE;
   ck_assert(eina_value_optional_empty_is(value, &empty));
   ck_assert(empty);

   eina_promise_unref(promise);
   eo_unref(json);
}
END_TEST

START_TEST(object_simple_get_set)
{
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL,
                         ejson_model_type_set(eo_self, EJSON_MODEL_TYPE_OBJECT));
   ck_assert_ptr_ne(NULL, json);

   // Add child to an existing object
   Eo *child = efl_model_child_add(json);
   ck_assert_ptr_ne(NULL, child);

   // If you don't specify the type, it must be null
   Ejson_Model_Type type = ejson_model_type_get(child);
   ck_assert_int_eq(EJSON_MODEL_TYPE_NULL, type);

   // Changing the model type
   ejson_model_type_set(child, EJSON_MODEL_TYPE_INT);
   type = ejson_model_type_get(child);
   ck_assert_int_eq(EJSON_MODEL_TYPE_INT, type);

   // Setting and getting a name
   const char *name_to_be_set = "a";
   _set_name_property(child, name_to_be_set);

   Eina_Promise *promise = NULL;
   efl_model_property_get(child, EJSON_MODEL_NAME_PROPERTY, &promise);

   Eina_Value *property = eina_promise_value_get(promise);
   ck_assert_ptr_eq(EINA_VALUE_TYPE_STRING, eina_value_type_get(property));

   const char *value = NULL;
   eina_value_get(property, &value);
   ck_assert_str_eq(name_to_be_set, value);

   eina_promise_unref(promise);

   // Setting and getting a value
   int value_to_be_set = 0xdeadbeef;
   _set_int_value_property(child, &value_to_be_set);

   efl_model_property_get(child, EJSON_MODEL_VALUE_PROPERTY, &promise);

   property = eina_promise_value_get(promise);
   ck_assert_ptr_eq(EINA_VALUE_TYPE_INT, eina_value_type_get(property));

   int retrieved_value = -1;
   eina_value_get(property, &retrieved_value);
   ck_assert_int_eq(value_to_be_set, retrieved_value);

   eina_promise_unref(promise);
   eo_unref(json);
}
END_TEST

START_TEST(object_set_invalid_property)
{
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL,
                        ejson_model_type_set(eo_self, EJSON_MODEL_TYPE_INT));
   Eina_Promise *promise;
   Eina_Value value;

   ck_assert_ptr_ne(NULL, json);

   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, 0xdeadbeef);

   // Invalid property
   efl_model_property_set(json, "foobar", &value, &promise);
   ck_assert_int_eq(eina_promise_error_get(promise), EFL_MODEL_ERROR_NOT_FOUND);

   eina_promise_unref(promise);
   promise = NULL;

   // Invalid 'name' property
   efl_model_property_set(json, EJSON_MODEL_NAME_PROPERTY, NULL, &promise);
   ck_assert_int_eq(eina_promise_error_get(promise), EFL_MODEL_ERROR_INCORRECT_VALUE);

   eina_promise_unref(promise);
   eina_value_flush(&value);
   eo_unref(json);

}
END_TEST

START_TEST(array_object_property_set)
{
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL,
                         ejson_model_type_set(eo_self, EJSON_MODEL_TYPE_ARRAY));
   ck_assert_ptr_ne(NULL, json);

   Eo *child = efl_model_child_add(json);
   ck_assert_ptr_ne(NULL, child);

   Ejson_Model_Type type = ejson_model_type_get(child);
   ck_assert_int_eq(EJSON_MODEL_TYPE_NULL, type);
   ejson_model_type_set(child, EJSON_MODEL_TYPE_INT);
   type = ejson_model_type_get(child);
   ck_assert_int_eq(EJSON_MODEL_TYPE_INT, type);

   _check_int_value_property_set(child, 1234);

   eo_unref(json);
}
END_TEST

START_TEST(int_property_set)
{
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL,
                         ejson_model_type_set(eo_self, EJSON_MODEL_TYPE_INT));
   ck_assert_ptr_ne(NULL, json);

   _check_int_value_property_set(json, 1234);

   _check_value_type_cannot_have_children(json);

   eo_unref(json);
}
END_TEST

START_TEST(boolean_property_set)
{
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL,
                         ejson_model_type_set(eo_self, EJSON_MODEL_TYPE_BOOLEAN));
   ck_assert_ptr_ne(NULL, json);

   _check_bool_value_property_set(json, true);
   _check_bool_value_property_set(json, false);

   _check_value_type_cannot_have_children(json);

   eo_unref(json);
}
END_TEST

START_TEST(string_property_set)
{
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL,
                         ejson_model_type_set(eo_self, EJSON_MODEL_TYPE_STRING));
   ck_assert_ptr_ne(NULL, json);

   _check_string_value_property_set(json, "Hello world!");

   _check_value_type_cannot_have_children(json);

   eo_unref(json);
}
END_TEST

START_TEST(json_object_property_set)
{
   const char *json_str = "{ \"a\": 1, \"b\": \"Hello\" }";
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL,
                         ejson_model_string_set(eo_self, json_str));
   ck_assert_ptr_ne(NULL, json);

   _check_efl_model_children_count_eq(json, 2);

   Eo *child1 = _emodel_nth_child_get(json, 1);
   Eo *child2 = _emodel_nth_child_get(json, 2);

   _check_ejson_type(child1, EJSON_MODEL_TYPE_INT);
   _check_ejson_type(child2, EJSON_MODEL_TYPE_STRING);


   _check_efl_model_property_int_eq(child1, EJSON_MODEL_VALUE_PROPERTY, 1);
   _check_efl_model_property_str_eq(child1, EJSON_MODEL_NAME_PROPERTY, "a");

   _check_efl_model_property_str_eq(child2, EJSON_MODEL_VALUE_PROPERTY, "Hello");
   _check_efl_model_property_str_eq(child2, EJSON_MODEL_NAME_PROPERTY, "b");

   eo_unref(child1);
   eo_unref(child2);

   eo_unref(json);
}
END_TEST

START_TEST(json_array_property_set)
{
   const char *json_str = "[ 1, \"Hello\" ]";
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL,
                         ejson_model_string_set(eo_self, json_str));
   ck_assert_ptr_ne(NULL, json);

   _check_efl_model_children_count_eq(json, 2);

   Eo *child1 = _emodel_nth_child_get(json, 1);
   Eo *child2 = _emodel_nth_child_get(json, 2);

   _check_ejson_type(child1, EJSON_MODEL_TYPE_INT);
   _check_ejson_type(child2, EJSON_MODEL_TYPE_STRING);

   _check_efl_model_property_int_eq(child1, EJSON_MODEL_VALUE_PROPERTY, 1);

   _check_efl_model_property_str_eq(child2, EJSON_MODEL_VALUE_PROPERTY, "Hello");

   // Get rid of the refs from nth_child_get
   eo_unref(child1);
   eo_unref(child2);

   eo_unref(json);
}
END_TEST

static const char *_all_json_types_object_string =
   "{ \"i\": 1, "
     "\"s\": \"Hello World!\", "
     "\"b\": true, "
     "\"d\": 1.234, "
     "\"n\": null, "
     "\"o\": { }, "
     "\"a\": [ ] "
   "}";

static Eo *
_create_model_with_all_json_types()
{
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL,
                       ejson_model_string_set(eo_self, _all_json_types_object_string));

   _check_efl_model_children_count_eq(json, 7);
   return json;
}

START_TEST(properties_get)
{
   Eo *json = _create_model_with_all_json_types();

   _check_efl_model_properties(json, (const char*[]){NULL});

   Eo *child_i = _emodel_nth_child_get(json, 1);
   Eo *child_s = _emodel_nth_child_get(json, 2);
   Eo *child_b = _emodel_nth_child_get(json, 3);
   Eo *child_d = _emodel_nth_child_get(json, 4);
   Eo *child_n = _emodel_nth_child_get(json, 5);
   Eo *child_o = _emodel_nth_child_get(json, 6);
   Eo *child_a = _emodel_nth_child_get(json, 7);

   _check_ejson_type(child_i, EJSON_MODEL_TYPE_INT);
   _check_ejson_type(child_s, EJSON_MODEL_TYPE_STRING);
   _check_ejson_type(child_b, EJSON_MODEL_TYPE_BOOLEAN);
   _check_ejson_type(child_d, EJSON_MODEL_TYPE_DOUBLE);
   _check_ejson_type(child_n, EJSON_MODEL_TYPE_NULL);
   _check_ejson_type(child_o, EJSON_MODEL_TYPE_OBJECT);
   _check_ejson_type(child_a, EJSON_MODEL_TYPE_ARRAY);

   _check_efl_model_properties(child_i, (const char*[]){"name", "value", NULL});
   _check_efl_model_properties(child_s, (const char*[]){"name", "value", NULL});
   _check_efl_model_properties(child_b, (const char*[]){"name", "value", NULL});
   _check_efl_model_properties(child_d, (const char*[]){"name", "value", NULL});
   _check_efl_model_properties(child_n, (const char*[]){"name", NULL});
   _check_efl_model_properties(child_o, (const char*[]){"name", NULL});
   _check_efl_model_properties(child_a, (const char*[]){"name", NULL});

   // Get rid of the refs we got from the slice
   eo_unref(child_i);
   eo_unref(child_s);
   eo_unref(child_b);
   eo_unref(child_d);
   eo_unref(child_n);
   eo_unref(child_o);
   eo_unref(child_a);

   eo_unref(json);
}
END_TEST

START_TEST(nested_object_test)
{
   /* const char *json_str = "{ \"a\": { } }"; */
   const char *json_str = "{ }";
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL,
                       ejson_model_string_set(eo_self, json_str));

   ck_assert_ptr_ne(NULL, json);

   eo_unref(json);
}
END_TEST

START_TEST(object_child_del)
{
   Eo *json = _create_model_with_all_json_types();

   Eo *child_i = _emodel_nth_child_get(json, 1);
   Eo *child_s = _emodel_nth_child_get(json, 2);
   Eo *child_b = _emodel_nth_child_get(json, 3);
   Eo *child_d = _emodel_nth_child_get(json, 4);
   Eo *child_n = _emodel_nth_child_get(json, 5);
   Eo *child_o = _emodel_nth_child_get(json, 6);
   Eo *child_a = _emodel_nth_child_get(json, 7);

   efl_model_child_del(json, child_s);
   _check_efl_model_children_count_eq(json, 6);

   efl_model_child_del(json, child_i);
   _check_efl_model_children_count_eq(json, 5);

   efl_model_child_del(json, child_a);
   _check_efl_model_children_count_eq(json, 4);

   efl_model_child_del(json, child_d);
   _check_efl_model_children_count_eq(json, 3);

   efl_model_child_del(json, child_b);
   _check_efl_model_children_count_eq(json, 2);

   efl_model_child_del(json, child_o);
   _check_efl_model_children_count_eq(json, 1);

   efl_model_child_del(json, child_n);
   _check_efl_model_children_count_eq(json, 0);

   eo_unref(json);
}
END_TEST

START_TEST(object_child_out_of_bounds)
{
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL);
   Eina_Promise *promise;

   ck_assert_ptr_ne(NULL, json);

   efl_model_children_slice_get(json, 10, 2, &promise);

   ck_assert_ptr_ne(NULL, promise);
   ck_assert_int_eq(eina_promise_error_get(promise), EFL_MODEL_ERROR_NOT_FOUND);

   eina_promise_unref(promise);

   eo_unref(json);
}
END_TEST

START_TEST(wrong_properties_get)
{
   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL);
   Eina_Promise *promise = NULL;

   ck_assert_ptr_ne(NULL, json);

   efl_model_property_get(json, NULL, &promise);

   ck_assert_ptr_ne(NULL, promise);
   ck_assert_int_eq(eina_promise_error_get(promise), EFL_MODEL_ERROR_INCORRECT_VALUE);

   eina_promise_unref(promise);
   promise = NULL;

   efl_model_property_get(json, "FOOBAR", &promise);

   ck_assert_ptr_ne(NULL, promise);
   ck_assert_int_eq(eina_promise_error_get(promise), EFL_MODEL_ERROR_NOT_FOUND);

   eina_promise_unref(promise);
   eo_unref(json);
}
END_TEST

START_TEST(array_child_del)
{
   const char *_all_json_types_array_string =
     "[ 1, \"Hello World!\", true, 1.234, null, { }, [ ] ]";

   Eo *json = eo_add(EJSON_MODEL_CLASS, NULL,
                         ejson_model_string_set(eo_self, _all_json_types_array_string));
   ck_assert_ptr_ne(NULL, json);

   _check_efl_model_children_count_eq(json, 7);

   Eo *child_i = _emodel_nth_child_get(json, 1);
   Eo *child_s = _emodel_nth_child_get(json, 2);
   Eo *child_b = _emodel_nth_child_get(json, 3);
   Eo *child_d = _emodel_nth_child_get(json, 4);
   Eo *child_n = _emodel_nth_child_get(json, 5);
   Eo *child_o = _emodel_nth_child_get(json, 6);
   Eo *child_a = _emodel_nth_child_get(json, 7);

   efl_model_child_del(json, child_s);
   _check_efl_model_children_count_eq(json, 6);

   efl_model_child_del(json, child_i);
   _check_efl_model_children_count_eq(json, 5);

   efl_model_child_del(json, child_a);
   _check_efl_model_children_count_eq(json, 4);

   efl_model_child_del(json, child_d);
   _check_efl_model_children_count_eq(json, 3);

   efl_model_child_del(json, child_b);
   _check_efl_model_children_count_eq(json, 2);

   efl_model_child_del(json, child_o);
   _check_efl_model_children_count_eq(json, 1);

   efl_model_child_del(json, child_n);
   _check_efl_model_children_count_eq(json, 0);

   eo_unref(json);
}
END_TEST

void
ejson_test_ejson_model(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, object_constructor);
   tcase_add_test(tc, object_get_null_value);
   tcase_add_test(tc, object_simple_get_set);
   tcase_add_test(tc, object_set_invalid_property);
   tcase_add_test(tc, array_object_property_set);
   tcase_add_test(tc, int_property_set);
   tcase_add_test(tc, boolean_property_set);
   tcase_add_test(tc, string_property_set);
   tcase_add_test(tc, json_object_property_set);
   tcase_add_test(tc, json_array_property_set);
   tcase_add_test(tc, properties_get);
   tcase_add_test(tc, nested_object_test);
   tcase_add_test(tc, object_child_del);
   tcase_add_test(tc, object_child_out_of_bounds);
   tcase_add_test(tc, wrong_properties_get);
   tcase_add_test(tc, array_child_del);
}

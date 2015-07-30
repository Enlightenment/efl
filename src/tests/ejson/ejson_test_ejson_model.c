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

static Eina_Bool
_eo_event_load_status_cb(void *data,
                         Eo *obj EINA_UNUSED,
                         const Eo_Event_Description *desc EINA_UNUSED,
                         void *event_info EINA_UNUSED)
{
   Efl_Model_Load_Status expected_status = *((Efl_Model_Load_Status*)data);
   Efl_Model_Load load = *((Efl_Model_Load*)event_info);
   if ((load.status & expected_status) != expected_status)
     return EINA_TRUE;

   ecore_main_loop_quit();
   return EINA_FALSE;
}

static void
_wait_until_load_status(Efl_Model_Base *emodel,
                        Efl_Model_Load_Status expected_status)
{
   Efl_Model_Load_Status actual_status;
   eo_do(emodel, actual_status = efl_model_load_status_get());
   if (expected_status == actual_status) return;

   eo_do(emodel, eo_event_callback_add(EFL_MODEL_BASE_EVENT_LOAD_STATUS, _eo_event_load_status_cb, &expected_status));
   ecore_main_loop_begin();
   eo_do(emodel, eo_event_callback_del(EFL_MODEL_BASE_EVENT_LOAD_STATUS, _eo_event_load_status_cb, &expected_status));
}

static void
_check_json_string(Efl_Model_Base *model, const char *expected_json_string)
{
   const Eina_Value *json_value;
   Efl_Model_Load_Status status;
   eo_do(model, status = efl_model_property_get(EJSON_MODEL_JSON_PROPERTY, &json_value));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES, status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);
   ck_assert_ptr_ne(NULL, json_value);

   const char *actual_json_string;
   fail_unless(eina_value_get(json_value, &actual_json_string));

   ck_assert_str_eq(expected_json_string, actual_json_string);
}

static void
_check_value_type_cannot_have_children(Efl_Model_Base *model)
{
   Efl_Model_Base *child = eo_do_ret(model, child, efl_model_child_add());
   ck_assert_ptr_eq(NULL, child);
}

static Efl_Model_Load_Status
_json_property_set(Efl_Model_Base *model, const char *v)
{
   Eina_Value json;
   eina_value_setup(&json, EINA_VALUE_TYPE_STRING);
   eina_value_set(&json, v);
   Efl_Model_Load_Status status;
   eo_do(model, status = efl_model_property_set(EJSON_MODEL_JSON_PROPERTY, &json));
   eina_value_flush(&json);
   return status;
}

static void
_check_name_property_set(Efl_Model_Base *model, const char *v)
{
   Eina_Value name;
   eina_value_setup(&name, EINA_VALUE_TYPE_STRING);
   eina_value_set(&name, v);
   Efl_Model_Load_Status status;
   eo_do(model, status = efl_model_property_set(EJSON_MODEL_NAME_PROPERTY, &name));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   eina_value_flush(&name);
}

static void
_check_int_value_property_set(Efl_Model_Base *model, int64_t v)
{
   Eina_Value value;
   eina_value_setup(&value, EINA_VALUE_TYPE_INT64);
   eina_value_set(&value, v);

   Efl_Model_Load_Status status;
   eo_do(model, status = efl_model_property_set(EJSON_MODEL_VALUE_PROPERTY, &value));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);

   char buf[64];
   snprintf(buf, sizeof(buf), "%lld", (long long)v);

   _check_json_string(model, buf);

   eina_value_flush(&value);
}

static void
_check_bool_value_property_set(Efl_Model_Base *model, bool v)
{
   Eina_Value value;
   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, (int)v);

   Efl_Model_Load_Status status;
   eo_do(model, status = efl_model_property_set(EJSON_MODEL_VALUE_PROPERTY, &value));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);

   _check_json_string(model, v ? "true" : "false");

   eina_value_flush(&value);
}

static void
_check_string_value_property_set(Efl_Model_Base *model, const char *v)
{
   Eina_Value value;
   eina_value_setup(&value, EINA_VALUE_TYPE_STRING);
   eina_value_set(&value, v);

   Efl_Model_Load_Status status;
   eo_do(model, status = efl_model_property_set(EJSON_MODEL_VALUE_PROPERTY, &value));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);

   size_t len = 1 + strlen(v) + 1 + 1;
   char buf[len];
   snprintf(buf, len, "\"%s\"", v);

   _check_json_string(model, buf);

   eina_value_flush(&value);
}

static void
_check_emodel_children_count_eq(Efl_Model_Base *model,
                                unsigned int expected_children_count)
{
   unsigned int actual_children_count = 0;
   eo_do(model, efl_model_children_count_get(&actual_children_count));
   ck_assert_int_eq(expected_children_count, actual_children_count);
}

static void
_check_emodel_property_int_eq(Efl_Model_Base *emodel,
                              const char *property,
                              int64_t expected_value)
{
   const Eina_Value *value;
   Efl_Model_Load_Status status;
   eo_do(emodel, status = efl_model_property_get(property, &value));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, value);

   const Eina_Value_Type *property_type = eina_value_type_get(value);
   ck_assert_ptr_eq(EINA_VALUE_TYPE_INT64, property_type);

   int64_t actual_value = 0;
   eina_value_get(value, &actual_value);
   ck_assert_int_eq(expected_value, actual_value);
}

static void
_check_emodel_property_str_eq(Efl_Model_Base *emodel,
                              const char *property,
                              const char *expected_value)
{
   const Eina_Value *value;
   Efl_Model_Load_Status status;
   eo_do(emodel, status = efl_model_property_get(property, &value));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, value);

   const Eina_Value_Type *property_type = eina_value_type_get(value);
   fail_if(EINA_VALUE_TYPE_STRING != property_type && EINA_VALUE_TYPE_STRINGSHARE != property_type);

   const char *actual_value = NULL;
   eina_value_get(value, &actual_value);
   ck_assert_str_eq(expected_value, actual_value);
}

static Efl_Model_Base *
_emodel_nth_child_get(Efl_Model_Base *emodel, unsigned int n)
{
   Eina_Accessor *accessor;
   Efl_Model_Load_Status status;
   eo_do(emodel, status = efl_model_children_slice_get(n, 1, &accessor));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, accessor);
   Efl_Model_Base *child = NULL;
   Eina_Bool ret = eina_accessor_data_get(accessor, 0, (void**)&child);
   eina_accessor_free(accessor);
   ck_assert(ret);
   ck_assert_ptr_ne(NULL, child);
   return child;
}

static void
_check_ejson_type(Efl_Model_Base *model, Ejson_Model_Type expected_type)
{
   Ejson_Model_Type actual_type;
   eo_do(model, actual_type = ejson_model_type_get());
   ck_assert_int_eq(expected_type, actual_type);
}

static void
_check_emodel_load(Efl_Model_Base *model)
{
   eo_do(model, efl_model_load());
   _wait_until_load_status(model, EFL_MODEL_LOAD_STATUS_LOADED);
}

static void
_check_emodel_properties(Efl_Model_Base *model,
                         const char *expected_properties[])
{
   Eina_Array *properties = NULL;
   Efl_Model_Load_Status status;
   eo_do(model, status = efl_model_properties_get(&properties));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, properties);

   unsigned int actual_properties_count = eina_array_count(properties);

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

static void
_check_properties_count_eq(Efl_Model_Base *model,
                           unsigned int expected_properties_count)
{
   Eina_Array *properties = NULL;
   Efl_Model_Load_Status status;
   eo_do(model, status = efl_model_properties_get(&properties));
   ck_assert_int_ne(EFL_MODEL_LOAD_STATUS_ERROR, status);
   if (!expected_properties_count && !properties)
     return;

   ck_assert_ptr_ne(NULL, properties);

   unsigned int actual_properties_count = eina_array_count(properties);
   ck_assert_int_eq(expected_properties_count, actual_properties_count);
}

START_TEST(smoke)
{
   Efl_Model_Base *json = eo_add(EJSON_MODEL_CLASS,
                                 NULL,
                                 ejson_model_constructor(EJSON_MODEL_TYPE_NULL));
   ck_assert_ptr_ne(NULL, json);
   eo_unref(json);
}
END_TEST

START_TEST(load_object)
{
   Efl_Model_Base *json = eo_add(EJSON_MODEL_CLASS,
                                 NULL,
                                 ejson_model_constructor(EJSON_MODEL_TYPE_OBJECT));
   ck_assert_ptr_ne(NULL, json);

   _check_emodel_load(json);

   eo_unref(json);
}
END_TEST

START_TEST(object_property_set)
{
   Efl_Model_Base *json = eo_add(EJSON_MODEL_CLASS,
                          NULL,
                          ejson_model_constructor(EJSON_MODEL_TYPE_OBJECT));
   ck_assert_ptr_ne(NULL, json);

   _check_emodel_load(json);

   _check_json_string(json, "{ }");

   Efl_Model_Base *child = eo_do_ret(json, child, efl_model_child_add());
   ck_assert_ptr_ne(NULL, child);

   _check_json_string(json, "{ }");

   Ejson_Model_Type type = eo_do_ret(child, type, ejson_model_type_get());
   ck_assert_int_eq(EJSON_MODEL_TYPE_NULL, type);
   eo_do(child, ejson_model_type_set(EJSON_MODEL_TYPE_INT));
   eo_do(child, type = ejson_model_type_get());
   ck_assert_int_eq(EJSON_MODEL_TYPE_INT, type);

   _check_emodel_load(child);

   _check_json_string(child, "null");
   _check_json_string(json, "{ }");

   // set member name
   _check_name_property_set(child, "a");
   _check_json_string(child, "null");
   _check_json_string(json, "{ \"a\": null }");

   // set member value
   _check_int_value_property_set(child, 1234);
   _check_json_string(json, "{ \"a\": 1234 }");

   // change member name
   _check_name_property_set(child, "b");
   _check_json_string(json, "{ \"b\": 1234 }");

   eo_unref(json);
}
END_TEST

START_TEST(array_object_property_set)
{
   Efl_Model_Base *json = eo_add(EJSON_MODEL_CLASS,
                                 NULL,
                                 ejson_model_constructor(EJSON_MODEL_TYPE_ARRAY));
   ck_assert_ptr_ne(NULL, json);

   _check_emodel_load(json);

   _check_json_string(json, "[ ]");

   Efl_Model_Base *child = eo_do_ret(json, child, efl_model_child_add());
   ck_assert_ptr_ne(NULL, child);

   _check_json_string(json, "[ ]");

   Ejson_Model_Type type = eo_do_ret(child, type, ejson_model_type_get());
   ck_assert_int_eq(EJSON_MODEL_TYPE_NULL, type);
   eo_do(child, ejson_model_type_set(EJSON_MODEL_TYPE_INT));
   eo_do(child, type = ejson_model_type_get());
   ck_assert_int_eq(EJSON_MODEL_TYPE_INT, type);

   _check_emodel_load(child);

   _check_json_string(child, "null");
   _check_json_string(json, "[ ]");

   _check_int_value_property_set(child, 1234);

   _check_json_string(json, "[ 1234 ]");

   eo_unref(json);
}
END_TEST

START_TEST(int_property_set)
{
   Efl_Model_Base *json = eo_add(EJSON_MODEL_CLASS,
                                 NULL,
                                 ejson_model_constructor(EJSON_MODEL_TYPE_INT));
   ck_assert_ptr_ne(NULL, json);

   _check_emodel_load(json);

   _check_json_string(json, "null");

   _check_int_value_property_set(json, 1234);

   _check_value_type_cannot_have_children(json);

   eo_unref(json);
}
END_TEST

START_TEST(boolean_property_set)
{
   Efl_Model_Base *json = eo_add(EJSON_MODEL_CLASS,
                                 NULL,
                                 ejson_model_constructor(EJSON_MODEL_TYPE_BOOLEAN));
   ck_assert_ptr_ne(NULL, json);

   _check_emodel_load(json);

   _check_json_string(json, "null");

   _check_bool_value_property_set(json, true);
   _check_bool_value_property_set(json, false);

   _check_value_type_cannot_have_children(json);

   eo_unref(json);
}
END_TEST

START_TEST(string_property_set)
{
   Efl_Model_Base *json = eo_add(EJSON_MODEL_CLASS,
                                 NULL,
                                 ejson_model_constructor(EJSON_MODEL_TYPE_STRING));
   ck_assert_ptr_ne(NULL, json);

   _check_emodel_load(json);

   _check_json_string(json, "null");

   _check_string_value_property_set(json, "Hello world!");

   _check_value_type_cannot_have_children(json);

   eo_unref(json);
}
END_TEST

START_TEST(json_object_property_set)
{
   Efl_Model_Base *json = eo_add(EJSON_MODEL_CLASS,
                                 NULL,
                                 ejson_model_constructor(EJSON_MODEL_TYPE_NULL));
   ck_assert_ptr_ne(NULL, json);

   _json_property_set(json, "{ \"a\": 1, \"b\": \"Hello\" }");

   _check_emodel_load(json);

   _check_json_string(json, "{ \"a\": 1, \"b\": \"Hello\" }");

   _check_emodel_children_count_eq(json, 2);

   Efl_Model_Base *child1 = _emodel_nth_child_get(json, 1);
   Efl_Model_Base *child2 = _emodel_nth_child_get(json, 2);

   _check_ejson_type(child1, EJSON_MODEL_TYPE_INT);
   _check_ejson_type(child2, EJSON_MODEL_TYPE_STRING);

   _check_emodel_load(child1);

   _check_emodel_property_int_eq(child1, EJSON_MODEL_VALUE_PROPERTY, 1);
   _check_emodel_property_str_eq(child1, EJSON_MODEL_NAME_PROPERTY, "a");

   _check_emodel_load(child2);

   _check_emodel_property_str_eq(child2, EJSON_MODEL_VALUE_PROPERTY, "Hello");
   _check_emodel_property_str_eq(child2, EJSON_MODEL_NAME_PROPERTY, "b");

   _json_property_set(child2, "{ \"c\": 2, \"d\": \"World\" }");

   _check_json_string(json, "{ \"a\": 1, \"b\": { \"c\": 2, \"d\": \"World\" } }");

   eo_unref(json);
}
END_TEST

START_TEST(json_array_property_set)
{
   Efl_Model_Base *json = eo_add(EJSON_MODEL_CLASS,
                                 NULL,
                                 ejson_model_constructor(EJSON_MODEL_TYPE_NULL));
   ck_assert_ptr_ne(NULL, json);

   _json_property_set(json, "[ 1, \"Hello\" ]");

   _check_emodel_load(json);

   _check_json_string(json, "[ 1, \"Hello\" ]");

   _check_emodel_children_count_eq(json, 2);

   Efl_Model_Base *child1 = _emodel_nth_child_get(json, 1);
   Efl_Model_Base *child2 = _emodel_nth_child_get(json, 2);

   _check_ejson_type(child1, EJSON_MODEL_TYPE_INT);
   _check_ejson_type(child2, EJSON_MODEL_TYPE_STRING);

   _check_emodel_load(child1);

   _check_emodel_property_int_eq(child1, EJSON_MODEL_VALUE_PROPERTY, 1);

   _check_emodel_load(child2);

   _check_emodel_property_str_eq(child2, EJSON_MODEL_VALUE_PROPERTY, "Hello");

   _json_property_set(child2, "[ 2, \"World\" ]");

   _check_json_string(json, "[ 1, [ 2, \"World\" ] ]");

   eo_unref(json);
}
END_TEST

START_TEST(load_status_get)
{
   Efl_Model_Base *json = eo_add(EJSON_MODEL_CLASS,
                                 NULL,
                                 ejson_model_constructor(EJSON_MODEL_TYPE_NULL));
   ck_assert_ptr_ne(NULL, json);

   Efl_Model_Load_Status status;
   eo_do(json, status = efl_model_load_status_get());
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_UNLOADED, status);

   _check_emodel_load(json);

   eo_do(json, status = efl_model_load_status_get());
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);

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

static Efl_Model_Base *
_create_model_with_all_json_types()
{
   Efl_Model_Base *json = eo_add(EJSON_MODEL_CLASS,
                                 NULL,
                                 ejson_model_constructor(EJSON_MODEL_TYPE_NULL));
   ck_assert_ptr_ne(NULL, json);

   _check_emodel_load(json);

   _json_property_set(json, _all_json_types_object_string);
   _check_json_string(json, _all_json_types_object_string);

   _check_emodel_children_count_eq(json, 7);
   return json;
}

START_TEST(properties_get)
{
   Efl_Model_Base *json = _create_model_with_all_json_types();

   _check_emodel_properties(json, (const char*[]){"json", NULL});

   Efl_Model_Base *child_i = _emodel_nth_child_get(json, 1);
   Efl_Model_Base *child_s = _emodel_nth_child_get(json, 2);
   Efl_Model_Base *child_b = _emodel_nth_child_get(json, 3);
   Efl_Model_Base *child_d = _emodel_nth_child_get(json, 4);
   Efl_Model_Base *child_n = _emodel_nth_child_get(json, 5);
   Efl_Model_Base *child_o = _emodel_nth_child_get(json, 6);
   Efl_Model_Base *child_a = _emodel_nth_child_get(json, 7);

   _check_emodel_load(child_i);
   _check_emodel_load(child_s);
   _check_emodel_load(child_b);
   _check_emodel_load(child_d);
   _check_emodel_load(child_n);
   _check_emodel_load(child_o);
   _check_emodel_load(child_a);

   _check_ejson_type(child_i, EJSON_MODEL_TYPE_INT);
   _check_ejson_type(child_s, EJSON_MODEL_TYPE_STRING);
   _check_ejson_type(child_b, EJSON_MODEL_TYPE_BOOLEAN);
   _check_ejson_type(child_d, EJSON_MODEL_TYPE_DOUBLE);
   _check_ejson_type(child_n, EJSON_MODEL_TYPE_NULL);
   _check_ejson_type(child_o, EJSON_MODEL_TYPE_OBJECT);
   _check_ejson_type(child_a, EJSON_MODEL_TYPE_ARRAY);

   _check_emodel_properties(child_i, (const char*[]){"json", "value", "name", NULL});
   _check_emodel_properties(child_s, (const char*[]){"json", "value", "name", NULL});
   _check_emodel_properties(child_b, (const char*[]){"json", "value", "name", NULL});
   _check_emodel_properties(child_d, (const char*[]){"json", "value", "name", NULL});
   _check_emodel_properties(child_n, (const char*[]){"json", "name", NULL});
   _check_emodel_properties(child_o, (const char*[]){"json", "name", NULL});
   _check_emodel_properties(child_a, (const char*[]){"json", "name", NULL});

   eo_unref(json);
}
END_TEST

START_TEST(unload)
{
   Efl_Model_Base *json = _create_model_with_all_json_types();

   eo_do(json, efl_model_unload());
   _wait_until_load_status(json, EFL_MODEL_LOAD_STATUS_UNLOADED);

   // reload
   _check_emodel_load(json);
   _check_json_string(json, "{ }");

   _json_property_set(json, _all_json_types_object_string);
   _check_json_string(json, _all_json_types_object_string);

   eo_unref(json);
}
END_TEST

START_TEST(properties_load)
{
   Efl_Model_Base *json = eo_add(EJSON_MODEL_CLASS,
                                 NULL,
                                 ejson_model_constructor(EJSON_MODEL_TYPE_NULL));
   ck_assert_ptr_ne(NULL, json);

   _json_property_set(json, _all_json_types_object_string);

   eo_do(json, efl_model_properties_load());
   _wait_until_load_status(json, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);

   _check_emodel_children_count_eq(json, 0);

   _check_json_string(json, _all_json_types_object_string);

   eo_unref(json);
}
END_TEST

START_TEST(children_load)
{
   Efl_Model_Base *json = eo_add(EJSON_MODEL_CLASS,
                                 NULL,
                                 ejson_model_constructor(EJSON_MODEL_TYPE_NULL));
   ck_assert_ptr_ne(NULL, json);

   _json_property_set(json, _all_json_types_object_string);

   eo_do(json, efl_model_children_load());
   _wait_until_load_status(json, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   _check_emodel_children_count_eq(json, 7);

   _check_properties_count_eq(json, 0);

   eo_unref(json);
}
END_TEST


START_TEST(object_child_del)
{
   Efl_Model_Base *json = _create_model_with_all_json_types();

   Efl_Model_Base *child_i = _emodel_nth_child_get(json, 1);
   Efl_Model_Base *child_s = _emodel_nth_child_get(json, 2);
   Efl_Model_Base *child_b = _emodel_nth_child_get(json, 3);
   Efl_Model_Base *child_d = _emodel_nth_child_get(json, 4);
   Efl_Model_Base *child_n = _emodel_nth_child_get(json, 5);
   Efl_Model_Base *child_o = _emodel_nth_child_get(json, 6);
   Efl_Model_Base *child_a = _emodel_nth_child_get(json, 7);

   eo_do(json, efl_model_child_del(child_s));
   _check_emodel_children_count_eq(json, 6);
   _check_json_string(json, "{ \"i\": 1, \"b\": true, \"d\": 1.234, \"n\": null, \"o\": { }, \"a\": [ ] }");

   eo_do(json, efl_model_child_del(child_i));
   _check_emodel_children_count_eq(json, 5);
   _check_json_string(json, "{ \"b\": true, \"d\": 1.234, \"n\": null, \"o\": { }, \"a\": [ ] }");

   eo_do(json, efl_model_child_del(child_a));
   _check_emodel_children_count_eq(json, 4);
   _check_json_string(json, "{ \"b\": true, \"d\": 1.234, \"n\": null, \"o\": { } }");

   eo_do(json, efl_model_child_del(child_d));
   _check_emodel_children_count_eq(json, 3);
   _check_json_string(json, "{ \"b\": true, \"n\": null, \"o\": { } }");

   eo_do(json, efl_model_child_del(child_b));
   _check_emodel_children_count_eq(json, 2);
   _check_json_string(json, "{ \"n\": null, \"o\": { } }");

   eo_do(json, efl_model_child_del(child_o));
   _check_emodel_children_count_eq(json, 1);
   _check_json_string(json, "{ \"n\": null }");

   eo_do(json, efl_model_child_del(child_n));
   _check_emodel_children_count_eq(json, 0);
   _check_json_string(json, "{ }");

   eo_unref(json);
}
END_TEST

START_TEST(array_child_del)
{
   const char *_all_json_types_array_string =
     "[ 1, \"Hello World!\", true, 1.234, null, { }, [ ] ]";

   Efl_Model_Base *json = eo_add(EJSON_MODEL_CLASS, NULL,
                         ejson_model_constructor(EJSON_MODEL_TYPE_NULL));
   ck_assert_ptr_ne(NULL, json);

   _check_emodel_load(json);

   _json_property_set(json, _all_json_types_array_string);
   _check_json_string(json, _all_json_types_array_string);

   _check_emodel_children_count_eq(json, 7);

   Efl_Model_Base *child_i = _emodel_nth_child_get(json, 1);
   Efl_Model_Base *child_s = _emodel_nth_child_get(json, 2);
   Efl_Model_Base *child_b = _emodel_nth_child_get(json, 3);
   Efl_Model_Base *child_d = _emodel_nth_child_get(json, 4);
   Efl_Model_Base *child_n = _emodel_nth_child_get(json, 5);
   Efl_Model_Base *child_o = _emodel_nth_child_get(json, 6);
   Efl_Model_Base *child_a = _emodel_nth_child_get(json, 7);

   eo_do(json, efl_model_child_del(child_s));
   _check_emodel_children_count_eq(json, 6);
   _check_json_string(json, "[ 1, true, 1.234, null, { }, [ ] ]");

   eo_do(json, efl_model_child_del(child_i));
   _check_emodel_children_count_eq(json, 5);
   _check_json_string(json, "[ true, 1.234, null, { }, [ ] ]");

   eo_do(json, efl_model_child_del(child_a));
   _check_emodel_children_count_eq(json, 4);
   _check_json_string(json, "[ true, 1.234, null, { } ]");

   eo_do(json, efl_model_child_del(child_d));
   _check_emodel_children_count_eq(json, 3);
   _check_json_string(json, "[ true, null, { } ]");

   eo_do(json, efl_model_child_del(child_b));
   _check_emodel_children_count_eq(json, 2);
   _check_json_string(json, "[ null, { } ]");

   eo_do(json, efl_model_child_del(child_o));
   _check_emodel_children_count_eq(json, 1);
   _check_json_string(json, "[ null ]");

   eo_do(json, efl_model_child_del(child_n));
   _check_emodel_children_count_eq(json, 0);
   _check_json_string(json, "[ ]");

   eo_unref(json);
}
END_TEST

void
ejson_test_ejson_model(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, load_object);
   tcase_add_test(tc, object_property_set);
   tcase_add_test(tc, array_object_property_set);
   tcase_add_test(tc, int_property_set);
   tcase_add_test(tc, boolean_property_set);
   tcase_add_test(tc, string_property_set);
   tcase_add_test(tc, json_object_property_set);
   tcase_add_test(tc, json_array_property_set);
   tcase_add_test(tc, load_status_get);
   tcase_add_test(tc, properties_get);
   tcase_add_test(tc, unload);
   tcase_add_test(tc, properties_load);
   tcase_add_test(tc, children_load);
   tcase_add_test(tc, object_child_del);
   tcase_add_test(tc, array_child_del);
}

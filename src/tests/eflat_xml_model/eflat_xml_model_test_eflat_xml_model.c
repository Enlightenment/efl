#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eflat_xml_model_test_eflat_xml_model.h"
#include "eflat_xml_model_suite.h"

#include <Ecore.h>
#include <Eina.h>
#include <Eflat_Xml_Model.h>

#include <stdbool.h>

static void
_setup(void)
{
   int ret = eflat_xml_model_init();
   ck_assert_int_ge(ret, 1);
}

static void
_teardown(void)
{
   int ret = eflat_xml_model_shutdown();
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

   eo_do(emodel, eo_event_callback_add(EFL_MODEL_BASE_EVENT_LOAD_STATUS,
                                       _eo_event_load_status_cb,
                                       &expected_status));
   ecore_main_loop_begin();
   eo_do(emodel, eo_event_callback_del(EFL_MODEL_BASE_EVENT_LOAD_STATUS,
                                       _eo_event_load_status_cb,
                                       &expected_status));
}

static void
_check_efl_model_property_str_eq(Efl_Model_Base *emodel,
                                 const char *property,
                                 const char *expected_value)
{
   const Eina_Value *value;
   Efl_Model_Load_Status status;
   eo_do(emodel, status = efl_model_property_get(property, &value));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, value);

   const Eina_Value_Type *property_type = eina_value_type_get(value);
   fail_if(EINA_VALUE_TYPE_STRING != property_type &&
           EINA_VALUE_TYPE_STRINGSHARE != property_type);

   const char *actual_value = NULL;
   eina_value_get(value, &actual_value);
   ck_assert_str_eq(expected_value, actual_value);
}

static void
_check_efl_model_load(Efl_Model_Base *model)
{
   eo_do(model, efl_model_load());
   _wait_until_load_status(model, EFL_MODEL_LOAD_STATUS_LOADED);
}

static void
_check_efl_model_properties(Efl_Model_Base *model,
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

static Eflat_Xml_Model *
_create_flat_xml_model(void)
{
   Efl_Model_Base *xml_model = eo_add(EFLAT_XML_MODEL_CLASS,
                                      NULL,
                                      eflat_xml_model_constructor(NULL));
   ck_assert_ptr_ne(NULL, xml_model);
   return xml_model;
}

START_TEST(smoke)
{
   Efl_Model_Base *xml_model = _create_flat_xml_model();
   eo_unref(xml_model);
}
END_TEST

static void
_check_string_property_set(Efl_Model_Base *model,
                           const char *property,
                           const char *v)
{
   Eina_Value value;
   eina_value_setup(&value, EINA_VALUE_TYPE_STRING);
   eina_value_set(&value, v);
   Efl_Model_Load_Status status;
   eo_do(model, status = efl_model_property_set(property, &value));
   eina_value_flush(&value);
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
}

static void
_check_xml_string(Efl_Model_Base *model, const char *expected_xml_string)
{
   _check_efl_model_property_str_eq(model, "/", expected_xml_string);
}

static const char *SAMPLE_XML =
  "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
  "<soap:Envelope xmlns:soap=\"http://www.w3.org/2001/12/soap-envelope\" soap:encodingStyle=\"http://www.w3.org/2001/12/soap-encoding\">\n"
  "  <soap:Header>\n"
  "    <m:Trans xmlns:m=\"http://my.namespace/header\" soap:mustUnderstand=\"1\">\n"
  "      1234\n"
  "    </m:Trans>\n"
  "  </soap:Header>\n"
  "  <soap:Body>\n"
  "    <m:MethodName xmlns:m=\"http://my.namespece/body\">\n"
  "      <m:Item>\n"
  "        Value\n"
  "      </m:Item>\n"
  "    </m:MethodName>\n"
  "  </soap:Body>\n"
  "</soap:Envelope>\n";

static void
_check_sample_xml_properties_names(Efl_Model_Base *xml_model)
{
   _check_efl_model_properties(xml_model, (const char*[]){
     "?xml",
     "soap:Envelope@xmlns:soap",
     "soap:Envelope@soap:encodingStyle",
     "soap:Envelope/soap:Header/m:Trans@xmlns:m",
     "soap:Envelope/soap:Header/m:Trans@soap:mustUnderstand",
     "soap:Envelope/soap:Header/m:Trans",
     "soap:Envelope/soap:Body/m:MethodName@xmlns:m",
     "soap:Envelope/soap:Body/m:MethodName/m:Item",
     NULL});
}

static void
_check_sample_xml_properties_values(Efl_Model_Base *xml_model)
{
   _check_efl_model_property_str_eq(xml_model, "?xml", "version=\"1.0\" encoding=\"UTF-8\"");
   _check_efl_model_property_str_eq(xml_model, "soap:Envelope@xmlns:soap", "http://www.w3.org/2001/12/soap-envelope");
   _check_efl_model_property_str_eq(xml_model, "soap:Envelope@soap:encodingStyle", "http://www.w3.org/2001/12/soap-encoding");
   _check_efl_model_property_str_eq(xml_model, "soap:Envelope/soap:Header/m:Trans@xmlns:m", "http://my.namespace/header");
   _check_efl_model_property_str_eq(xml_model, "soap:Envelope/soap:Header/m:Trans@soap:mustUnderstand", "1");
   _check_efl_model_property_str_eq(xml_model, "soap:Envelope/soap:Header/m:Trans", "1234");
   _check_efl_model_property_str_eq(xml_model, "soap:Envelope/soap:Body/m:MethodName@xmlns:m", "http://my.namespece/body");
   _check_efl_model_property_str_eq(xml_model, "soap:Envelope/soap:Body/m:MethodName/m:Item", "Value");
}

static void
_check_property_set(Efl_Model_Base *xml_model)
{
   _check_string_property_set(xml_model, "?xml", "version=\"1.0\" encoding=\"UTF-8\"");
   _check_string_property_set(xml_model, "soap:Envelope@xmlns:soap", "http://www.w3.org/2001/12/soap-envelope");
   _check_string_property_set(xml_model, "soap:Envelope@soap:encodingStyle", "http://www.w3.org/2001/12/soap-encoding");
   _check_string_property_set(xml_model, "soap:Envelope/soap:Header/m:Trans@xmlns:m", "http://my.namespace/header");
   _check_string_property_set(xml_model, "soap:Envelope/soap:Header/m:Trans@soap:mustUnderstand", "1");
   _check_string_property_set(xml_model, "soap:Envelope/soap:Header/m:Trans", "1234");
   _check_string_property_set(xml_model, "soap:Envelope/soap:Body/m:MethodName@xmlns:m", "http://my.namespece/body");
   _check_string_property_set(xml_model, "soap:Envelope/soap:Body/m:MethodName/m:Item", "Value");

   _check_xml_string(xml_model, SAMPLE_XML);

   _check_sample_xml_properties_names(xml_model);

   _check_efl_model_property_str_eq(xml_model, "?xml", "version=\"1.0\" encoding=\"UTF-8\"");
}

START_TEST(property_set_basic)
{
   Efl_Model_Base *xml_model = _create_flat_xml_model();

   _check_efl_model_load(xml_model);

   _check_property_set(xml_model);

   eo_unref(xml_model);
}
END_TEST

static void
_check_property_not_exists(Efl_Model_Base *model, const char *property)
{
   const Eina_Value *value;
   Efl_Model_Load_Status status;
   eo_do(model, status = efl_model_property_get(property, &value));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_ERROR, status);

   Eina_Array *properties = NULL;
   eo_do(model, status = efl_model_properties_get(&properties));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, properties);

   unsigned int i;
   char *item;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(properties, i, item, it)
     ck_assert_str_ne(property, item);
}

START_TEST(property_set_existing)
{
   Efl_Model_Base *xml_model = _create_flat_xml_model();

   _check_efl_model_load(xml_model);

   _check_property_set(xml_model);

   // replaces an attribute
   _check_string_property_set(xml_model, "soap:Envelope/soap:Header/m:Trans@xmlns:m", "http://other.namespace/header");

   _check_efl_model_properties(xml_model, (const char*[]){
     "?xml",
     "soap:Envelope@xmlns:soap",
     "soap:Envelope@soap:encodingStyle",
     "soap:Envelope/soap:Header/m:Trans@soap:mustUnderstand",
     "soap:Envelope/soap:Header/m:Trans",
     "soap:Envelope/soap:Body/m:MethodName@xmlns:m",
     "soap:Envelope/soap:Body/m:MethodName/m:Item",
     "soap:Envelope/soap:Header/m:Trans@xmlns:m",
     NULL});

   // replaces a value
   _check_string_property_set(xml_model, "soap:Envelope/soap:Header/m:Trans", "4321");

   _check_efl_model_properties(xml_model, (const char*[]){
     "?xml",
     "soap:Envelope@xmlns:soap",
     "soap:Envelope@soap:encodingStyle",
     "soap:Envelope/soap:Header/m:Trans@soap:mustUnderstand",
     "soap:Envelope/soap:Body/m:MethodName@xmlns:m",
     "soap:Envelope/soap:Body/m:MethodName/m:Item",
     "soap:Envelope/soap:Header/m:Trans@xmlns:m",
     "soap:Envelope/soap:Header/m:Trans",
     NULL});

   // clears a value
   _check_string_property_set(xml_model, "soap:Envelope/soap:Body/m:MethodName/m:Item", "");

   _check_xml_string(xml_model,
     "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
     "<soap:Envelope xmlns:soap=\"http://www.w3.org/2001/12/soap-envelope\" soap:encodingStyle=\"http://www.w3.org/2001/12/soap-encoding\">\n"
     "  <soap:Header>\n"
     "    <m:Trans xmlns:m=\"http://other.namespace/header\" soap:mustUnderstand=\"1\">\n"
     "      4321\n"
     "    </m:Trans>\n"
     "  </soap:Header>\n"
     "  <soap:Body>\n"
     "    <m:MethodName xmlns:m=\"http://my.namespece/body\">\n"
     "      <m:Item/>\n"
     "    </m:MethodName>\n"
     "  </soap:Body>\n"
     "</soap:Envelope>\n"
   );

   _check_efl_model_properties(xml_model, (const char*[]){
     "?xml",
     "soap:Envelope@xmlns:soap",
     "soap:Envelope@soap:encodingStyle",
     "soap:Envelope/soap:Header/m:Trans@soap:mustUnderstand",
     "soap:Envelope/soap:Body/m:MethodName@xmlns:m",
     "soap:Envelope/soap:Header/m:Trans@xmlns:m",
     "soap:Envelope/soap:Header/m:Trans",
     "soap:Envelope/soap:Body/m:MethodName/m:Item",
     NULL});

   // clearing a value clears its children too
   _check_string_property_set(xml_model, "soap:Envelope/soap:Body", "");

   _check_property_not_exists(xml_model, "soap:Envelope/soap:Body/m:MethodName/m:Item");
   _check_property_not_exists(xml_model, "soap:Envelope/soap:Body/m:MethodName@xmlns:m");

   _check_xml_string(xml_model,
     "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
     "<soap:Envelope xmlns:soap=\"http://www.w3.org/2001/12/soap-envelope\" soap:encodingStyle=\"http://www.w3.org/2001/12/soap-encoding\">\n"
     "  <soap:Header>\n"
     "    <m:Trans xmlns:m=\"http://other.namespace/header\" soap:mustUnderstand=\"1\">\n"
     "      4321\n"
     "    </m:Trans>\n"
     "  </soap:Header>\n"
     "  <soap:Body/>\n"
     "</soap:Envelope>\n"
   );

   _check_efl_model_properties(xml_model, (const char*[]){
     "?xml",
     "soap:Envelope@xmlns:soap",
     "soap:Envelope@soap:encodingStyle",
     "soap:Envelope/soap:Header/m:Trans@soap:mustUnderstand",
     "soap:Envelope/soap:Header/m:Trans@xmlns:m",
     "soap:Envelope/soap:Header/m:Trans",
     "soap:Envelope/soap:Body",
     NULL});

   eo_unref(xml_model);
}
END_TEST

START_TEST(property_set_stream)
{
   Efl_Model_Base *xml_model = _create_flat_xml_model();

   _check_efl_model_load(xml_model);

   // root level
   _check_string_property_set(xml_model, "/", SAMPLE_XML);
   _check_sample_xml_properties_names(xml_model);
   _check_sample_xml_properties_values(xml_model);

   _check_string_property_set(xml_model, "soap:Envelope/soap:Body/",
     "    <m:OtherMethod xmlns:m=\"http://my.namespece/body\">\n"
     "      <m:OtherItem>\n"
     "        OtherValue\n"
     "      </m:OtherItem>\n"
     "    </m:OtherMethod>\n"
   );

   _check_efl_model_properties(xml_model, (const char*[]){
     "?xml",
     "soap:Envelope@xmlns:soap",
     "soap:Envelope@soap:encodingStyle",
     "soap:Envelope/soap:Header/m:Trans@xmlns:m",
     "soap:Envelope/soap:Header/m:Trans@soap:mustUnderstand",
     "soap:Envelope/soap:Header/m:Trans",
     "soap:Envelope/soap:Body/m:OtherMethod@xmlns:m",
     "soap:Envelope/soap:Body/m:OtherMethod/m:OtherItem",
     NULL});


   _check_xml_string(xml_model,
     "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
     "<soap:Envelope xmlns:soap=\"http://www.w3.org/2001/12/soap-envelope\" soap:encodingStyle=\"http://www.w3.org/2001/12/soap-encoding\">\n"
     "  <soap:Header>\n"
     "    <m:Trans xmlns:m=\"http://my.namespace/header\" soap:mustUnderstand=\"1\">\n"
     "      1234\n"
     "    </m:Trans>\n"
     "  </soap:Header>\n"
     "  <soap:Body>\n"
     "    <m:OtherMethod xmlns:m=\"http://my.namespece/body\">\n"
     "      <m:OtherItem>\n"
     "        OtherValue\n"
     "      </m:OtherItem>\n"
     "    </m:OtherMethod>\n"
     "  </soap:Body>\n"
     "</soap:Envelope>\n"
   );

   eo_unref(xml_model);
}
END_TEST

START_TEST(property_get_stream)
{
   Efl_Model_Base *xml_model = _create_flat_xml_model();

   _check_efl_model_load(xml_model);

   _check_property_set(xml_model);

   // sublevel
   _check_efl_model_property_str_eq(xml_model, "soap:Envelope/",
     "<soap:Header>\n"
     "  <m:Trans xmlns:m=\"http://my.namespace/header\" soap:mustUnderstand=\"1\">\n"
     "    1234\n"
     "  </m:Trans>\n"
     "</soap:Header>\n"
     "<soap:Body>\n"
     "  <m:MethodName xmlns:m=\"http://my.namespece/body\">\n"
     "    <m:Item>\n"
     "      Value\n"
     "    </m:Item>\n"
     "  </m:MethodName>\n"
     "</soap:Body>\n"
   );

   // sublevel
   _check_efl_model_property_str_eq(xml_model, "soap:Envelope/soap:Header/",
     "<m:Trans xmlns:m=\"http://my.namespace/header\" soap:mustUnderstand=\"1\">\n"
     "  1234\n"
     "</m:Trans>\n"
   );

   // sublevel data
   _check_efl_model_property_str_eq(xml_model, "soap:Envelope/soap:Header/m:Trans/",
     "1234\n"
   );

   eo_unref(xml_model);
}
END_TEST

START_TEST(load_status_get)
{
   Efl_Model_Base *xml_model = _create_flat_xml_model();

   Efl_Model_Load_Status status;
   eo_do(xml_model, status = efl_model_load_status_get());
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_UNLOADED, status);

   _check_efl_model_load(xml_model);

   eo_do(xml_model, status = efl_model_load_status_get());
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);

   eo_unref(xml_model);
}
END_TEST

START_TEST(unload)
{
   Efl_Model_Base *xml_model = _create_flat_xml_model();

   _check_efl_model_load(xml_model);
   _check_property_set(xml_model);

   eo_do(xml_model, efl_model_unload());

   // status must be unloaded now
   Efl_Model_Load_Status status;
   eo_do(xml_model, status = efl_model_load_status_get());
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_UNLOADED, status);

   // reload and manipulate content
   _check_efl_model_load(xml_model);
   _check_property_set(xml_model);

   eo_unref(xml_model);
}
END_TEST

START_TEST(xml_load)
{
   Efl_Model_Base *xml_model = eo_add(EFLAT_XML_MODEL_CLASS,
                                      NULL,
                                      eflat_xml_model_constructor(SAMPLE_XML));
   ck_assert_ptr_ne(NULL, xml_model);

   _check_efl_model_load(xml_model);
   _check_sample_xml_properties_names(xml_model);
   _check_sample_xml_properties_values(xml_model);

   eo_unref(xml_model);
}
END_TEST

void
eflat_xml_test_eflat_xml_model(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, property_set_basic);
   tcase_add_test(tc, property_set_existing);
   tcase_add_test(tc, property_set_stream);
   tcase_add_test(tc, property_get_stream);
   tcase_add_test(tc, load_status_get);
   tcase_add_test(tc, unload);
   tcase_add_test(tc, xml_load);
}

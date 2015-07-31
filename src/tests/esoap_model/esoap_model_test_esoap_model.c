#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "esoap_model_test_esoap_model.h"
#include "esoap_model_suite.h"

#include <Ecore.h>
#include <Ecore_Con.h>
#include <Eina.h>
#include <Esoap_Model.h>

#include <stdbool.h>

static Eina_Bool
server_data_callback(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
  Ecore_Con_Event_Client_Data* client_event = event;

  const char check_data[] =
    "POST /axis2/services/echo HTTP/1.1\r\n"
    "Host: 127.0.0.1:9090\r\n"
    "Accept: */*\r\n"
    "Content-Type:application/soap+xml\r\n"
    "SOAPAction:http://ws.apache.org/axis2/c/samples/echoString\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
    "150\r\n"
    "<esoap:Envelope xmlns:esoap=\"http://www.w3.org/2003/05/esoap-envelope\" esoap:encodingStyle=\"http://www.w3.org/2001/12/esoap-encoding\">\n"
    "  <esoap:Body>\n"
    "    <ns1:echoString xmlns:ns1=\"http://ws.apache.org/axis2/services/echo\">\n"
    "      <ns1:text>\n"
    "        Hello World!\n"
    "      </ns1:text>\n"
    "    </ns1:echoString>\n"
    "  </esoap:Body>\n"
    "</esoap:Envelope>\n"
    "\r\n"
    "0\r\n"
    "\r\n";

  // static check that check_data is at least 545 bytes
  char a[545-sizeof(check_data)]; (void)a;
  // static check that check_data is at most 545 bytes
  char b[sizeof(check_data) - 545]; (void)b;
  
  ck_assert(client_event->size == 544);
  ck_assert(!memcmp(check_data, client_event->data, sizeof(check_data)-1));

  const char data_to_send[] =
    "HTTP/1.1 200\r\n"
    "Host: 127.0.0.1:9090\r\n"
    "Content-Type:application/soap+xml\r\n"
    "Content-Length: 276\r\n"
    "\r\n"
    "<esoapenv:Envelope xmlns:esoapenv=\"http://www.w3.org/2003/05/esoap-envelope\">\n"
    "  <esoapenv:Body>\n"
    "    <ns1:echoString xmlns:ns1=\"http://ws.apache.org/axis2/c/samples\">\n"
    "      <text>\n"
    "        Hello World!\n"
    "      </text>\n"
    "    </ns1:echoString>\n"
    "  </esoapenv:Body>\n"
    "</esoapenv:Envelope>\n";

  ecore_con_client_send(client_event->client, data_to_send, sizeof(data_to_send)-1);
    
  return EINA_TRUE;
}

static Ecore_Con_Server* server;

static void
_setup(void)
{
   int ret = esoap_model_init();
   ck_assert_int_ge(ret, 1);
   ecore_con_init();

   ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA,
                           server_data_callback,
                           NULL);

   server = ecore_con_server_add
     (ECORE_CON_REMOTE_TCP, "0.0.0.0", 9090, NULL);

}

static void
_teardown(void)
{
   int ret = esoap_model_shutdown();
   ck_assert_int_eq(ret, 0);

   ecore_con_server_del(server);
   
   ecore_con_shutdown();
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
_wait_until_load_status(Efl_Model_Base *model, Efl_Model_Load_Status expected_status)
{
   Efl_Model_Load_Status actual_status;
   eo_do(model, actual_status = efl_model_load_status_get());
   if ((expected_status & actual_status) == expected_status) return;

   eo_do(model, eo_event_callback_add(EFL_MODEL_BASE_EVENT_LOAD_STATUS, _eo_event_load_status_cb, &expected_status));
   ecore_main_loop_begin();
   eo_do(model, eo_event_callback_del(EFL_MODEL_BASE_EVENT_LOAD_STATUS, _eo_event_load_status_cb, &expected_status));
}

static void
_check_model_children_count_eq(Efl_Model_Base *model, unsigned int expected_children_count)
{
   unsigned int actual_children_count = 0;
   eo_do(model, efl_model_children_count_get(&actual_children_count));
   ck_assert_int_eq(expected_children_count, actual_children_count);
}

static void
_check_model_property_str_eq(Efl_Model_Base *model, const char *property, const char *expected_value)
{
   const Eina_Value *value;
   Efl_Model_Load_Status status;
   eo_do(model, status = efl_model_property_get(property, &value));
   ck_assert_int_eq(EFL_MODEL_LOAD_STATUS_LOADED, status);
   ck_assert_ptr_ne(NULL, value);

   const Eina_Value_Type *property_type = eina_value_type_get(value);
   fail_if(EINA_VALUE_TYPE_STRING != property_type && EINA_VALUE_TYPE_STRINGSHARE != property_type);

   const char *actual_value = NULL;
   eina_value_get(value, &actual_value);
   ck_assert_str_eq(expected_value, actual_value);
}

static Efl_Model_Base *
_efl_model_nth_child_get(Efl_Model_Base *model, unsigned int n)
{
   Eina_Accessor *accessor;
   Efl_Model_Load_Status status;
   eo_do(model, status = efl_model_children_slice_get(n, 1, &accessor));
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
_check_model_load(Efl_Model_Base *model)
{
   eo_do(model, efl_model_load());
   _wait_until_load_status(model, EFL_MODEL_LOAD_STATUS_LOADED);
}

static void
_check_model_properties(Efl_Model_Base *model, const char *expected_properties[])
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

static Esoap_Model *
_create_esoap_model()
{
   Efl_Model_Base *esoap = eo_add(ESOAP_MODEL_CLASS, NULL,
     esoap_model_constructor("http://127.0.0.1:9090/axis2/services/echo",
                            "http://ws.apache.org/axis2/c/samples/echoString"));
   ck_assert_ptr_ne(NULL, esoap);
   return esoap;
}

START_TEST(smoke)
{
   Efl_Model_Base *esoap = _create_esoap_model();
   eo_unref(esoap);
}
END_TEST

static void
_check_string_property_set(Efl_Model_Base *model, const char *property, const char *v)
{
   Eina_Value value;
   eina_value_setup(&value, EINA_VALUE_TYPE_STRING);
   eina_value_set(&value, v);
   Efl_Model_Load_Status status;
   eo_do(model, status = efl_model_property_set(property, &value));
   eina_value_flush(&value);
   ck_assert(EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES & status);
}

START_TEST(load_children)
{
   Efl_Model_Base *esoap = _create_esoap_model();

   eo_do(esoap, efl_model_properties_load());
   _wait_until_load_status(esoap, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);

   _check_string_property_set(esoap, "esoap:Envelope@xmlns:esoap", "http://www.w3.org/2003/05/esoap-envelope");
   _check_string_property_set(esoap, "esoap:Envelope@esoap:encodingStyle", "http://www.w3.org/2001/12/esoap-encoding");
   _check_string_property_set(esoap, "esoap:Envelope/esoap:Body/ns1:echoString@xmlns:ns1", "http://ws.apache.org/axis2/services/echo");
   _check_string_property_set(esoap, "esoap:Envelope/esoap:Body/ns1:echoString/ns1:text", "Hello World!");

   // make the call loading the children
   eo_do(esoap, efl_model_children_load());
   _wait_until_load_status(esoap, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   _check_model_children_count_eq(esoap, 1);

   Efl_Model_Base *child = _efl_model_nth_child_get(esoap, 1);
   _check_model_load(child);

   _check_model_properties(child, (const char*[]){
     "esoapenv:Envelope@xmlns:esoapenv",
     "esoapenv:Envelope/esoapenv:Body/ns1:echoString@xmlns:ns1",
     "esoapenv:Envelope/esoapenv:Body/ns1:echoString/text",
     NULL});

   _check_model_property_str_eq(child, "esoapenv:Envelope@xmlns:esoapenv", "http://www.w3.org/2003/05/esoap-envelope");
   _check_model_property_str_eq(child, "esoapenv:Envelope/esoapenv:Body/ns1:echoString@xmlns:ns1", "http://ws.apache.org/axis2/c/samples");
   _check_model_property_str_eq(child, "esoapenv:Envelope/esoapenv:Body/ns1:echoString/text", "Hello World!");

   eo_unref(esoap);
}
END_TEST

void
esoap_model_test_esoap_model(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, load_children);
}

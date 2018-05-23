#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ecore.h>

#include "eldbus_suite.h"
#include "eldbus_fake_server.h"

#define FAKE_SERVER_DATA_KEY "data"

static void _fake_server_name_request_cb(void *, const Eldbus_Message *, Eldbus_Pending *);
static Eina_Bool _fakse_server_property_get(const Eldbus_Service_Interface *, const char *, Eldbus_Message_Iter *, const Eldbus_Message *, Eldbus_Message **);
static Eldbus_Message *_fake_server_property_set(const Eldbus_Service_Interface *, const char *, Eldbus_Message_Iter *, const Eldbus_Message *);
static Eldbus_Message *_fake_server_sum(const Eldbus_Service_Interface *, const Eldbus_Message *);
static Eldbus_Message *_fake_server_ping(const Eldbus_Service_Interface *, const Eldbus_Message *);
static Eina_Bool _fake_server_send_pong_signal(void *);

static Eldbus_Connection *conn;

static const Eldbus_Method methods[] = {
   {
     FAKE_SERVER_SUM_METHOD_NAME, ELDBUS_ARGS({"i", "a"}, {"i", "b"}), ELDBUS_ARGS({"i", "result"}),
     _fake_server_sum
   },
   {
     FAKE_SERVER_PING_METHOD_NAME, ELDBUS_ARGS({"i", "a"}), NULL,
     _fake_server_ping
   },
   { 0 }
};

enum
{
   FAKE_SERVER_PONG_SIGNAL = 0
};

static const Eldbus_Signal signals[] = {
   [FAKE_SERVER_PONG_SIGNAL] = {FAKE_SERVER_PONG_SIGNAL_NAME, ELDBUS_ARGS({ "i", NULL }), 0},
   { 0 }
};

static const Eldbus_Property properties[] = {
   { FAKE_SERVER_READONLY_PROPERTY, "i", _fakse_server_property_get, NULL, 0 },
   { FAKE_SERVER_WRITEONLY_PROPERTY, "i", NULL, _fake_server_property_set, 0 },
   { FAKE_SERVER_READWRITE_PROPERTY, "i", _fakse_server_property_get, _fake_server_property_set, 0 },
   { 0 }
};

static const Eldbus_Service_Interface_Desc test_interface_desc = {
   FAKE_SERVER_INTERFACE, methods, signals, properties, NULL, NULL
};


Eldbus_Service_Interface *
fake_server_start(Fake_Server_Data *data)
{
   ck_assert_ptr_ne(NULL, data);

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);

   Eldbus_Service_Interface *interface = eldbus_service_interface_register(conn, FAKE_SERVER_PATH, &test_interface_desc);
   eldbus_service_object_data_set(interface, FAKE_SERVER_DATA_KEY, data);

   eldbus_name_request(conn, FAKE_SERVER_BUS, ELDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                       _fake_server_name_request_cb, interface);

   ecore_main_loop_begin();

   return interface;
}

void
fake_server_stop(Eldbus_Service_Interface *interface)
{
   eldbus_service_object_unregister(interface);
   eldbus_connection_unref(conn);
}

static Eldbus_Message *
_fake_server_sum(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   int a, b;
   if (!eldbus_message_arguments_get(msg, "ii", &a, &b))
     return eldbus_message_error_new(msg, "Invalid arguments", "Error getting arguments..");

   int sum = a + b;

   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(reply, "i", sum);
   return reply;
}

static Eldbus_Message *
_fake_server_ping(const Eldbus_Service_Interface *iface , const Eldbus_Message *msg)
{
   int a;
   if (!eldbus_message_arguments_get(msg, "i", &a))
     return eldbus_message_error_new(msg, "Invalid arguments", "Error getting arguments..");

   Fake_Server_Data *pd = eldbus_service_object_data_get(iface, FAKE_SERVER_DATA_KEY);
   ck_assert_ptr_ne(NULL, pd);

   pd->pong_response = a + 1;
   Ecore_Timer *timer = ecore_timer_add(0.1, _fake_server_send_pong_signal, iface);
   ck_assert_ptr_ne(NULL, timer);

   return NULL;
}

static Eina_Bool
_fake_server_send_pong_signal(void *data)
{
   Eldbus_Service_Interface *iface = (Eldbus_Service_Interface*)data;

   Fake_Server_Data *pd = eldbus_service_object_data_get(iface, FAKE_SERVER_DATA_KEY);
   ck_assert_ptr_ne(NULL, pd);

   eldbus_service_signal_emit(iface, FAKE_SERVER_PONG_SIGNAL, pd->pong_response);
   return ECORE_CALLBACK_CANCEL;
}

Eina_Bool
_fakse_server_property_get(const Eldbus_Service_Interface *iface,
                           const char *propname,
                           Eldbus_Message_Iter *iter,
                           const Eldbus_Message *request_msg EINA_UNUSED,
                           Eldbus_Message **error EINA_UNUSED)
{
   Fake_Server_Data *data = eldbus_service_object_data_get(iface, FAKE_SERVER_DATA_KEY);
   ck_assert_ptr_ne(NULL, data);

   if (strcmp(propname, FAKE_SERVER_READONLY_PROPERTY) == 0)
     eldbus_message_iter_arguments_append(iter, "i", data->readonly_property);
   else
   if (strcmp(propname, FAKE_SERVER_READWRITE_PROPERTY) == 0)
     eldbus_message_iter_arguments_append(iter, "i", data->readwrite_property);
   else
     return EINA_FALSE;

   return EINA_TRUE;
}

Eldbus_Message *
_fake_server_property_set(const Eldbus_Service_Interface *iface,
                          const char *propname,
                          Eldbus_Message_Iter *iter,
                          const Eldbus_Message *msg)
{
   Fake_Server_Data *data = eldbus_service_object_data_get(iface, FAKE_SERVER_DATA_KEY);
   ck_assert_ptr_ne(NULL, data);

   if (strcmp(propname, FAKE_SERVER_WRITEONLY_PROPERTY) == 0)
     eldbus_message_iter_arguments_get(iter, "i", &data->writeonly_property);
   else
   if (strcmp(propname, FAKE_SERVER_READWRITE_PROPERTY) == 0)
     eldbus_message_iter_arguments_get(iter, "i", &data->readwrite_property);
   else
     return eldbus_message_error_new(msg, "Invalid property", "Invalid property.");

   return eldbus_message_method_return_new(msg);
}

static void
_fake_server_name_request_cb(void *data EINA_UNUSED,
                             const Eldbus_Message *msg,
                             Eldbus_Pending *pending EINA_UNUSED)
{
   const char *name, *text;
   if (eldbus_message_error_get(msg, &name, &text))
     ck_abort_msg("error on _fake_server_name_request_cb: %s %s", name, text);

   unsigned int reply;
   if (!eldbus_message_arguments_get(msg, "u", &reply))
     ck_abort_msg("error geting arguments on _fake_server_name_request_cb");

   if (ELDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER != reply)
     {
        const char *errcode = "Unknown reply";
        char errmsg[512];
        switch (reply)
          {
           case ELDBUS_NAME_REQUEST_REPLY_IN_QUEUE:
             errcode = "Service could not become the primary owner and has "
                       "been placed in the queue";
             break;
           case ELDBUS_NAME_REQUEST_REPLY_EXISTS:
             errcode = "Service is already in the queue";
             break;
           case ELDBUS_NAME_REQUEST_REPLY_ALREADY_OWNER:
             errcode = "Service is already the primary owner";
             break;
           default: break;
          }
        snprintf(errmsg, sizeof(errmsg), "Failed to start fake server: %s (%u)",
                 errcode, reply);
        ck_abort_msg(errmsg);
     }

   ecore_main_loop_quit();
}

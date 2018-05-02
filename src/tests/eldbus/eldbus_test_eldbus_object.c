#include <Eldbus.h>
#include <Ecore.h>
#include <string.h>
#include <stdio.h>
#include <Eina.h>

#include "eldbus_suite.h"

static Eina_Bool is_success = EINA_FALSE;
static Eina_Bool is_register_service = EINA_FALSE;
static Eina_Bool is_peer_ping = EINA_FALSE;

static Ecore_Timer *timeout = NULL;

static const char *empty_string = "";

static const char *bus = "org.freedesktop.DBus";
static const char *interface = "org.freedesktop.DBus";
static const char *path = "/org/freedesktop/DBus";
static const char *method_name = "GetId";

static int cb_data = 5;

/**
* @addtogroup eldbus
* @{
* @defgroup eldbus_object
*
* @precondition
* @step 1 Initialize eldbus with eldbus_init()
*/

static Eina_Bool
_ecore_loop_close(void *data EINA_UNUSED)
{
   ecore_main_loop_quit();

   return ECORE_CALLBACK_CANCEL;
}

static void
_object_message_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   if (timeout != NULL)
     {
        ecore_timer_del(timeout);
        timeout = NULL;
     }

   const char *errname, *errmsg;
   int *user_data = data;

   if ((user_data) && (*user_data == cb_data))
     {
        if (!eldbus_message_error_get(msg, &errname, &errmsg))
          {
             char *txt;
             if (eldbus_message_arguments_get(msg, "s", &txt))
               {
                  if ((txt) && strcmp(txt, empty_string))
                    is_success = EINA_TRUE;
               }
          }
     }

   ecore_main_loop_quit();
}

static void
_name_request(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   unsigned int reply;
   const char *errname, *errmsg;
   if (!eldbus_message_error_get(msg, &errname, &errmsg))
     {
        if (eldbus_message_arguments_get(msg, "u", &reply))
          {
             if (reply == ELDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER)
               {
                  is_register_service = EINA_TRUE;
               }
          }
     }
}

static const Eldbus_Method methods[] = {
      {
        "Ping", NULL,
        NULL, NULL, 0
      },
      {
        "GetMachineId", NULL,
        ELDBUS_ARGS({"s", "machine_id"}), NULL, 0
      },
      { }
};

static const Eldbus_Service_Interface_Desc iface_desc = {
   "org.freedesktop.DBus.Peer", methods, NULL
};

static void
_peer_ping_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   int *user_data = data;

   if (user_data && (*user_data == cb_data))
     {
        if (!eldbus_message_error_get(msg, NULL, NULL))
          {
             is_peer_ping = EINA_TRUE;
          }
     }
}

static void
_machine_id_get(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   int *user_data = data;

   if (user_data && (*user_data == cb_data))
     {
        if (!eldbus_message_error_get(msg, NULL, NULL))
          {
             char *txt;
             if (eldbus_message_arguments_get(msg, "s", &txt))
               {
                  if (txt && strcmp(txt, empty_string))
                    {
                       is_success = EINA_TRUE;
                    }
               }
          }
     }
}

/**
 * @addtogroup eldbus_object
 * @{
 * @defgroup eldbus_object_send_info_get
 * @li eldbus_object_bus_name_get()
 * @li eldbus_object_connection_get()
 * @li eldbus_object_method_call_new()
 * @li eldbus_object_path_get()
 * @li eldbus_object_ref()
 * @li eldbus_object_send()
 * @li eldbus_object_get()
 * @{
 * @objective Positive test case checks if functions get expected bus name,
 * get Eldbus_Connection expected associated with Eldbus_Object object,
 * get expected path associated with object,
 * create message object and send a message to DBus service,
 * increase object reference. Without segmentation fault.
 *
 * @n Input Data:
 * @li the conn object connection with bus
 * @li the obj Eldbus_Object object of the given bus and path.
 *
 * @procedure
 * @step 1 Call eldbus_connection_get function to get connection object.
 * @step 2 Check returned connection object on NULL.
 * @step 3 Call eldbus_object_get get an object of the given bus and path.
 * @step 4 Check returned object on NULL.
 * @step 5 Call eldbus_object_bus_name_get function to get valid string and check on expected string.
 * @step 6 Call eldbus_object_path_get function to get valid string and check on expected string.
 * @step 7 Call eldbus_object_ref function to increase object reference.
 * @step 8 Check two objects on the same address.
 * @step 9 Call eldbus_object_unref function to decrease object reference.
 * @step 10 Call eldbus_object_method_call_new function to call a dbus method on the object.
 * @step 11 Check returned message object on NULL.
 * @step 12 Call eldbus_object_send function to send a message.
 * @step 13 Check returned pending object on NULL.
 * @step 14 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 15 Start of main loop and wait for tested response in callback executing.
 * @step 16 Check static variable named is_success.
 * If is equal EINA_TRUE, that callback was executed and method send valid message response.
 * @step 17 Call eldbus_message_unref function to delete message object
 * @step 18 Call eldbus_object_unref function to delete connection dbus object
 * @step 19 Call eldbus_connection_unref function to delete connection object
 *
 * @passcondition Function should get valid expected object information about service.
 * Variables named is_success must equals EINA_TRUE. Without segmentation fault.
 * @}
 * @}
 */

EFL_START_TEST(utc_eldbus_object_send_info_get_p)
{
   is_success = EINA_FALSE;

   Eldbus_Connection *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   ck_assert_ptr_ne(NULL, conn);

   Eldbus_Object *obj = eldbus_object_get(conn, bus, path);
   ck_assert_ptr_ne(NULL, obj);

    const char *bus_from_object_ptr = eldbus_object_bus_name_get(obj);
    ck_assert_msg((strcmp(bus, bus_from_object_ptr) == 0), "'%s' != '%s'", bus, bus_from_object_ptr);

    const char *path_from_object_ptr = eldbus_object_path_get(obj);
    ck_assert_msg((strcmp(path, path_from_object_ptr) == 0), "'%s' != '%s'", path, path_from_object_ptr);

   Eldbus_Object *obj_ref = eldbus_object_ref(obj);
   ck_assert_ptr_eq(obj_ref, obj);

   eldbus_object_unref(obj_ref);

   Eldbus_Connection *connection_obj = eldbus_object_connection_get(obj);
   ck_assert_ptr_eq(connection_obj, conn);

   Eldbus_Message *msg = eldbus_object_method_call_new(obj, interface, method_name);
   ck_assert_ptr_ne(NULL, msg);

   Eldbus_Pending *pending = eldbus_object_send(obj, msg, _object_message_cb, &cb_data, -1);
   ck_assert_ptr_ne(NULL, pending);

   timeout = ecore_timer_add(1.5, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   ck_assert_msg(is_success, "Method %s is not call", method_name);

   eldbus_connection_unref(conn);
}
EFL_END_TEST

/**
 * @addtogroup eldbus_object
 * @{
 * @defgroup eldbus_introspect eldbus_object_introspect()
 * @{
 * @objective Positive test case checks if function call the method "Introspect"
 * on the eldbus object and get valid response in callback function. Without segmentation fault.
 *
 * @n Input Data:
 * @li the conn object connection with bus
 * @li the obj Eldbus_Object object of the given bus and path.
 *
 * @procedure
 * @step 1 Call eldbus_connection_get function to get connection object.
 * @step 2 Check returned connection object on NULL.
 * @step 3 Call eldbus_object_get to get an object of the given bus and path.
 * @step 4 Check returned object on NULL.
 * @step 5 Call eldbus_object_introspect functiont to call the method "Introspect" on the eldbus object.
 * @step 6 Check returned pending object on NULL.
 * @step 7 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 8 Start of main loop and wait for tested response in callback executing.
 * @step 9 Check static variable named is_success.
 * If is equal EINA_TRUE, that callback was executed and method send valid message response.
 * @step 10 Call eldbus_object_unref function to delete connection dbus object.
 * @step 11 Call eldbus_connection_unref function to delete connection object.
 *
 * @passcondition Variables named is_success must equals EINA_TRUE, and there is no segmentation fault.
 * @}
 * @}
 */

EFL_START_TEST(utc_eldbus_introspect_p)
{
   is_success = EINA_FALSE;

   Eldbus_Connection *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   ck_assert_ptr_ne(NULL, conn);

   Eldbus_Object *obj = eldbus_object_get(conn, bus, path);
   ck_assert_ptr_ne(NULL, obj);

   Eldbus_Pending *pending = eldbus_object_introspect(obj, _object_message_cb, &cb_data);
   ck_assert_ptr_ne(NULL, pending);

   timeout = ecore_timer_add(1.0, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   ck_assert_msg(is_success, "Method Introspect is not call");

   eldbus_object_unref(obj);
   eldbus_connection_unref(conn);
}
EFL_END_TEST

/**
 * @addtogroup eldbus_object
 * @{
 * @defgroup eldbus_object_peer
 * @li eldbus_object_peer_ping()
 * @li eldbus_object_peer_machine_id_get()
 * @li eldbus_object_get()
 * @{
 * @objective Positive test case checks if function call the method "Ping",
 * "GetMachineId" and get expected response without error. Without segmentation fault.
 *
 * @n Input Data:
 * @li the conn_server object connection to server
 * @li the conn_client object connection to client
 * @li the iface object register an interface
 *
 * @procedure
 * @step 1 Call eldbus_connection_get function to get server connection object.
 * @step 2 Check returned connection object on NULL.
 * @step 3 Call eldbus_service_interface_register function to register an interface.
 * in the given path and connection.
 * @step 4 Check returned interface object on NULL.
 * @step 5 Call eldbus_name_request function to send a "RequestName" method call in proxy.
 * @step 6 Check returned pending object on NULL.
 * @step 7 Call eldbus_connection_get function to get client connection object.
 * @step 8 Check returned connection object on NULL.
 * @step 9 Call eldbus_object_get function to get an client object
 * of the given bus and path.
 * @step 10 Check returned object on NULL.
 * @step 11 Call eldbus_object_peer_machine_id_get function to call the
 * method "GetMachineId" on the dbus object.
 * @step 12 Check returned pending object on NULL.
 * @step 13 Call eldbus_object_peer_ping function to call the
 * method "Ping" on the dbus object.
 * @step 14 Check returned pending object on NULL.
 * @step 15 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 16 Start of main loop and wait for tested response in callback executing.
 * @step 17 Check static variables named is_success, is_register_service, is_peer_ping.
 * If are equals 1, that callbacks were executed and methods send valid message response.
 * In other cases error.
 * @step 18 Call eldbus_object_unref function to delete connection dbus object.
 * @step 19 Call eldbus_connection_unref function to delete client connection object.
 * @step 20 Call eldbus_service_interface_unregister function to unregister a interface.
 * @step 21 Call eldbus_connection_unref function to delete server connection object
 *
 * @passcondition Variables named is_success_cb, is_register_service, is_peer_ping must equals 1, and there is no segmentation fault.
 * @}
 * @}
 */
EFL_START_TEST(utc_eldbus_object_peer_p)
{
   const char *dbus_path = "/org/freedesktop/Test";
   const char *bus_session = "org.freedesktop.Test";

   is_success = EINA_FALSE;
   is_register_service = EINA_FALSE;
   is_peer_ping = EINA_FALSE;

   Eldbus_Connection *conn_server = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   ck_assert_ptr_ne(NULL, conn_server);

   Eldbus_Service_Interface *iface = eldbus_service_interface_register(conn_server, dbus_path, &iface_desc);
   ck_assert_ptr_ne(NULL, iface);

   Eldbus_Pending *pending_name = eldbus_name_request(conn_server, bus_session, ELDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                                                      _name_request, iface);
   ck_assert_ptr_ne(NULL, pending_name);

   Eldbus_Connection *conn_client = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   ck_assert_ptr_ne(NULL, conn_client);

   Eldbus_Object *obj = eldbus_object_get(conn_client, bus_session, dbus_path);
   ck_assert_ptr_ne(NULL, obj);

   Eldbus_Pending *pending1 = eldbus_object_peer_machine_id_get(obj, _machine_id_get, &cb_data);
   ck_assert_ptr_ne(NULL, pending1);

   Eldbus_Pending *pending2 = eldbus_object_peer_ping(obj, _peer_ping_cb, &cb_data);
   ck_assert_ptr_ne(NULL, pending2);

   timeout = ecore_timer_add(2.5, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   ck_assert_msg(is_register_service, "Can not register interface");
   ck_assert_msg(is_peer_ping, "Method Ping is not call");
   ck_assert_msg(is_success, "Method GetMachineId is not call");

   eldbus_object_unref(obj);
   eldbus_connection_unref(conn_client);
   eldbus_service_interface_unregister(iface);
   eldbus_connection_unref(conn_server);
}
EFL_END_TEST

/**
 *@}
 */
void
eldbus_test_eldbus_object(TCase *tc)
{
   tcase_add_test(tc, utc_eldbus_object_send_info_get_p);
   tcase_add_test(tc, utc_eldbus_introspect_p);
   tcase_add_test(tc, utc_eldbus_object_peer_p);
}

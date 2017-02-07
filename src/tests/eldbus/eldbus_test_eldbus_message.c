#include <Eldbus.h>
#include <Ecore.h>
#include <string.h>
#include <Eina.h>

#include "eldbus_suite.h"

static Eldbus_Message *message_vparam = NULL;

static Eina_Bool is_success = EINA_FALSE;

static Ecore_Timer *timeout = NULL;

static const char *empty_string = "";

static const char *bus = "org.freedesktop.DBus";
static const char *interface = "org.freedesktop.DBus";
static const char *path = "/org/freedesktop/DBus";
static const char *method_name = "GetId";
static const char *signal_name = "NameOwnerChanged";

/**
* @addtogroup eldbus
* @{
* @defgroup eldbus_message
*
*
* @precondition
* @step 1 Initialize ecore with ecore_init()
* @step 2 Initialize eldbus with eldbus_init()
*/

static void
_setup(void)
{
   ecore_init();
   int ret = eldbus_init();
   ck_assert_int_ge(ret, 1);
}

static void
_teardown(void)
{
   ecore_shutdown();
   int ret = eldbus_shutdown();
   ck_assert_int_eq(ret, 0);
}

static Eina_Bool
_ecore_loop_close(void *data EINA_UNUSED)
{
   ecore_main_loop_quit();

   return ECORE_CALLBACK_CANCEL;
}

static void
_varg_add(char *str,...)
{
   va_list ap;
   va_start(ap, str);
   eldbus_message_arguments_vappend(message_vparam, str, ap);
   va_end(ap);
}

static void
_response_message_cb(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   if (timeout != NULL)
     {
        ecore_timer_del(timeout);
        timeout = NULL;
     }

   const char *errname, *errmsg;
   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ecore_main_loop_quit();
        return;
     }

   Eldbus_Message_Iter *iter_main = eldbus_message_iter_get(msg);
   if (!iter_main)
     {
        ecore_main_loop_quit();
        return;
     }

   const char *signature = eldbus_message_iter_signature_get(iter_main);
   if ((!signature) || (!strcmp(signature, empty_string)))
     {
        ecore_main_loop_quit();
        return;
     }

   Eldbus_Message_Iter *iterator;

   if (!eldbus_message_iter_arguments_get(iter_main, signature, &iterator))
     {
        ecore_main_loop_quit();
        return;
     }

   const char *bus_name;
   Eina_Bool is_has_data = EINA_FALSE;

   while(eldbus_message_iter_get_and_next(iterator, 's', &bus_name))
     {
        if ((!bus_name) || (!strcmp(bus_name, empty_string)))
          {
             ecore_main_loop_quit();
             return;
          }
         is_has_data = EINA_TRUE;
     }

   is_success = is_has_data;

   ecore_main_loop_quit();
}

static void
_message_method_cb(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   if (timeout != NULL)
     {
        ecore_timer_del(timeout);
        timeout = NULL;
     }

   const char *errname, *errmsg;
   if (!eldbus_message_error_get(msg, &errname, &errmsg))
     {
        char *txt;
        if (eldbus_message_arguments_get(msg, "s", &txt))
          {
             if ((txt) && (strcmp(txt, empty_string)))
               {
                  if (!strcmp(bus, eldbus_message_sender_get(msg)) &&
                      strcmp(eldbus_message_destination_get(msg), empty_string))
                    {
                       is_success = EINA_TRUE;
                    }
               }
          }
     }
    ecore_main_loop_quit();
}

static void
_activatable_list_response_cb(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   if (timeout != NULL)
     {
        ecore_timer_del(timeout);
        timeout = NULL;
     }

   const char *errname, *errmsg;
   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ecore_main_loop_quit();
        return;
     }

   Eldbus_Message_Iter *iter_main = eldbus_message_iter_get(msg);
   if (!iter_main)
     {
        ecore_main_loop_quit();
        return;
     }

   const char *signature = eldbus_message_iter_signature_get(iter_main);
   if ((!signature) || (!strcmp(signature, empty_string)))
     {
        ecore_main_loop_quit();
        return;
     }

   Eldbus_Message_Iter *iterator;

   if (!eldbus_message_iter_arguments_get(iter_main, signature, &iterator))
     {
        ecore_main_loop_quit();
        return;
     }

   const char *bus_name;
   Eina_Bool is_has_data = EINA_FALSE;
   do
     {
        eldbus_message_iter_arguments_get(iterator, "s", &bus_name);
        if ((!bus_name) || (!strcmp(bus_name, empty_string)))
          {
             ecore_main_loop_quit();
             return;
          }

        is_has_data = EINA_TRUE;
     }while (eldbus_message_iter_next(iterator));

   is_success = is_has_data;

   ecore_main_loop_quit();
}

static void
_signal_name_owner_changed(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   if (timeout != NULL)
     {
        ecore_timer_del(timeout);
        timeout = NULL;
     }

  const char *errname, *errmsg;
  if (!eldbus_message_error_get(msg, &errname, &errmsg))
    {
       is_success = EINA_TRUE;
    }

   ecore_main_loop_quit();
}

static void
_response_owner_has_cb(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   if (timeout != NULL)
     {
        ecore_timer_del(timeout);
        timeout = NULL;
     }

   const char *errname, *errmsg;
   if (!eldbus_message_error_get(msg, &errname, &errmsg))
     {
        Eldbus_Message_Iter *iter = eldbus_message_iter_get(msg);
        if (iter)
          {
             Eina_Bool bool_basic = EINA_FALSE;
             eldbus_message_iter_basic_get(iter, &bool_basic);
             Eina_Value *eina_value = eldbus_message_to_eina_value(msg);
             if (eina_value)
               {
                  Eina_Bool eina_bool_value = EINA_FALSE;
                  eina_value_struct_get(eina_value, "arg0", &eina_bool_value);
                  if (bool_basic && eina_bool_value)
                    {
                       is_success = EINA_TRUE;
                    }

                  eina_value_free(eina_value);
               }
          }
     }

   ecore_main_loop_quit();
}

static void
_message_without_body_cb(void *data EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
}

static void
_message_response_cb(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   if (timeout != NULL)
     {
        ecore_timer_del(timeout);
        timeout = NULL;
     }

   const char *errname, *errmsg;
   if (!eldbus_message_error_get(msg, &errname, &errmsg))
     {
        char *txt;
        if (eldbus_message_arguments_get(msg, "s", &txt))
          {
             if ((txt) && (strcmp(txt, empty_string)))
               {
                  is_success = EINA_TRUE;
               }
          }
     }
    ecore_main_loop_quit();
}

static void
_activable_list_get(Eldbus_Message_Cb message_cb)
{
   is_success = EINA_FALSE;

   Eldbus_Connection *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   ck_assert_ptr_ne(NULL, conn);

   Eldbus_Pending *pending = eldbus_names_activatable_list(conn, message_cb, NULL);
   ck_assert_ptr_ne(NULL, pending);

   timeout = ecore_timer_add(1.5, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   ck_assert_msg(is_success, "Method ListActivatableNames is not call");

   eldbus_connection_unref(conn);
}

/**
 * @addtogroup eldbus_message
 * @{
 * @defgroup eldbus_message_iterator_activatable_list
 * @li eldbus_message_iter_get()
 * @li eldbus_message_iter_signature_get()
 * @li eldbus_message_iter_arguments_get()
 * @li eldbus_message_iter_get_and_next()
 * @li eldbus_message_error_get()
 * @{
 * @objective Positive test case checks if function get response from DBus service.
 * Get valid iterator from message response, current signature of a message iterator,
 * get data from iterator and move the next field to end, and without segmentation fault.
 *
 * @n Input Data:
 * @li the conn object connection with bus
 *
 * @procedure
 * @step 1 Call eldbus_connection_get function to get connection object
 * @step 2 Check returned connection object on NULL.
 * @step 3 Call eldbus_names_activatable_list send a "ListActivatableNames" method call in proxy.
 * @step 4 Check returned object on NULL.
 * @step 5 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 6 Start of main loop and wait for tested response in callback executing.
 * @step 7 Check static variables named is_success.
 * If are equals 1, that callback was executed and method "ListActivatableNames" send valid message response.
 * Message iterator returned valid iteration data. In other cases error.
 * @step 8 Call eldbus_connection_unref function to delete connection object
 *
 * @passcondition Message response should returned valid iteration data, and worked correctly.
 * @}
 * @}
 */

START_TEST(utc_eldbus_message_iterator_activatable_list_p)
{
   _activable_list_get(_response_message_cb);
}
END_TEST

/**
 * @addtogroup eldbus_message
 * @{
 * @defgroup eldbus_message_info_data_get
 * @li eldbus_message_method_call_new()
 * @li eldbus_message_interface_get()
 * @li eldbus_message_member_get()
 * @li eldbus_message_path_get()
 * @li eldbus_message_sender_get()
 * @li eldbus_message_destination_get()
 * @li eldbus_message_arguments_get()
 * @li eldbus_message_error_get()
 * @{
 * @objective Positive test case checks if function create a new message to invoke a method.
 * Get from message object expected valid information data about DBus service. Without segmentation fault.
 *
 * @n Input Data:
 * @li the conn object connection with bus
 *
 * @procedure
 * @step 1 Call eldbus_connection_get function to get connection object.
 * @step 2 Check returned connection object on NULL.
 * @step 3 Call eldbus_message_method_call_new for create a new message to invoke a method on a remote object.
 * @step 4 Check returned object on NULL.
 * @step 5 Call eldbus_message_interface_get function to get valid string and check on expected string.
 * @step 6 Call eldbus_message_member_get function to get valid string and check on expected string.
 * @step 7 Call eldbus_message_path_get function to get valid string and check on expected string.
 * @step 8 Call eldbus_connection_send function to send a message a dbus object with timeout.
 * @step 9 Check returned pending on NULL.
 * @step 10 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 11 Start of main loop and wait for tested response in callback executing.
 * @step 12 Check static variables named is_success.
 * If are equals 1, that callback was executed and method send valid message response
 * from expected sender and not empty destination
 * @step 13 Call eldbus_message_unref function to delete message object
 * @step 14 Call eldbus_connection_unref function to delete connection object
 *
 * @passcondition Function should get valid expected message information about service.
 * Variables named is_success must equals 1. Without segmentation fault.
 * @}
 * @}
 */

START_TEST(utc_eldbus_message_info_data_get_p)
{
   const int timeout_send_ms = 1000;

   is_success = EINA_FALSE;

   Eldbus_Connection *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   ck_assert_ptr_ne(NULL, conn);

   Eldbus_Message *msg = eldbus_message_method_call_new(bus, path, interface, method_name);
   ck_assert_ptr_ne(NULL, msg);

   const char *interface_msg = eldbus_message_interface_get(msg);
   ck_assert_msg(strcmp(interface, interface_msg) == 0, "%s != %s", interface, interface_msg);

   const char *method_msg = eldbus_message_member_get(msg);
   ck_assert_msg(strcmp(method_name, method_msg) == 0, "%s != %s", method_name, method_msg);

   const char *path_msg = eldbus_message_path_get(msg);
   ck_assert_msg(strcmp(path, path_msg) == 0, "%s != %s", path, path_msg);

   Eldbus_Pending *pending = eldbus_connection_send(conn, msg, _message_method_cb, NULL, timeout_send_ms);
   ck_assert_ptr_ne(NULL, pending);

   timeout = ecore_timer_add(1.5, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   ck_assert_msg(is_success, "Method %s is not call", method_name);

   eldbus_message_unref(msg);
   eldbus_connection_unref(conn);
}
END_TEST

/**
 * @addtogroup eldbus_message
 * @{
 * @defgroup eldbus_message_signal_new eldbus_message_signal_new()
 * @{
 * @objective Positive test case checks if function create a new signal message without error.
 *
 * @n Input Data:
 * @li the conn object connection with bus
 *
 * @procedure
 * @step 1 Get eldbus connection object and check on NULL
 * @step 2 Call eldbus_signal_handler_add with callback function
 * to be called when this signal is received.
 * @step 3 Check returned signal handler object on NULL.
 * @step 4 Call eldbus_message_signal_new to create a new signal message.
 * @step 5 Check returned message object on NULL.
 * @step 6 Call eldbus_connection_send to send a signal a dbus object.
 * @step 7 Check returned pending object on NULL.
 * @step 8 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 9 Start of main loop and wait for tested response in callback executing.
 * @step 10 Check static variables named is_success.
 * If are equals 1, that callback was executed and signal response arrives.
 * @step 11 Call eldbus_signal_handler_unref function to delete signal handler object
 * @step 12 Call eldbus_message_unref function to delete message object
 * @step 13 Call eldbus_connection_unref function to delete connection object
 *
 * @passcondition Variables named is_success_cb must equals 1, and there is no segmentation fault.
 * @}
 * @}
 */

START_TEST(utc_eldbus_message_signal_new_p)
{
   const int timeout_send_ms = 1000;

   is_success = EINA_FALSE;

   Eldbus_Connection *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   ck_assert_ptr_ne(NULL, conn);

   Eldbus_Signal_Handler *signal_handler = eldbus_signal_handler_add(conn, NULL, path, interface,
                                                                     signal_name, _signal_name_owner_changed, NULL);

   ck_assert_ptr_ne(NULL, signal_handler);

   Eldbus_Message *msg = eldbus_message_signal_new(path, interface, signal_name);
   ck_assert_ptr_ne(NULL, msg);

   Eldbus_Pending *pending = eldbus_connection_send(conn, msg, _message_without_body_cb, NULL, timeout_send_ms);
   ck_assert_ptr_ne(NULL, pending);

   timeout = ecore_timer_add(1.5, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   ck_assert_msg(is_success, "Signal NameOwnerChanged is not emit");

   eldbus_signal_handler_unref(signal_handler);
   eldbus_message_unref(msg);
   eldbus_connection_unref(conn);
}
END_TEST

/**
 * @addtogroup eldbus_message
 * @{
 * @defgroup eldbus_message_ref_unref
 * @li eldbus_message_ref()
 * @li eldbus_message_unref()
 * @{
 * @objective Positive test case checks if function increase, decrease message reference,
 * and without segmentation fault.
 *
 * @n Input Data:
 * @li the conn object connection with bus
 * @li the msg message object
 *
 * @procedure
 * @step 1 Call eldbus_connection_get function to get connection object
 * @step 2 Check returned connection object on NULL.
 * @step 3 Call eldbus_message_method_call_new to create a new message to invoke a method on a remote object.
 * @step 4 Check returned object on NULL.
 * @step 5 Call eldbus_message_ref to increase message reference (message counter == 2).
 * @step 6 Check two message objects on the same adress.
 * @step 7 Call eldbus_message_unref to decrease message reference (message counter == 1).
 * @step 8 Call eldbus_message_path_get and check return string on expected value.
 * @step 9 Call eldbus_message_unref to decrease message reference (message counter == 0).
 * @step 10 Call eldbus_message_path_get and check return string on NULL.
 * @step 11 Call eldbus_connection_unref function to delete connection object
 *
 * @passcondition Function should increase, decrease message reference count,
 * return expected result and without segmentation fault.
 * @}
 * @}
 */

START_TEST(utc_eldbus_message_ref_unref_p)
{
   Eldbus_Connection *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   ck_assert_ptr_ne(NULL, conn);

   Eldbus_Message *msg = eldbus_message_method_call_new(bus, path, interface, method_name);
   ck_assert_ptr_ne(NULL, msg);

   Eldbus_Message *msg_ref = eldbus_message_ref(msg);
   ck_assert_ptr_eq(msg_ref, msg);

   eldbus_message_unref(msg_ref);

   const char *path_msg = eldbus_message_path_get(msg);
   ck_assert_msg(strcmp(path, path_msg) == 0, "%s != %s", path, path_msg);

   eldbus_message_unref(msg);
   ck_assert_ptr_eq(NULL, eldbus_message_path_get(msg));

   eldbus_connection_unref(conn);
}
END_TEST

/**
 * @addtogroup eldbus_message
 * @{
 * @defgroup eldbus_message_basic_eina_value
 * @li eldbus_message_to_eina_value()
 * @li eldbus_message_iter_basic_get()
 * @li eldbus_message_error_get()
 * @{
 * @objective Positive test case checks if function convert message object to eina value,
 * get a basic type from message iterator, and without segmentation fault.
 *
 * @n Input Data:
 * @li the conn object connection with bus
 *
 * @procedure
 * @step 1 Call eldbus_connection_get function to get connection object
 * @step 2 Check returned connection object on NULL.
 * @step 3 Call eldbus_name_owner_get with callback function
 * to be called when method sended response.
 * @step 4 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 5 Start of main loop and wait for tested response in callback executing.
 * @step 6 Check static variables named is_success.
 * If are equals 1, that callback was executed and tested function return expeccted values.
 * @step 7 Call eldbus_connection_unref function to delete connection object
 *
 * @passcondition Variables named is_success_cb must equals 1, and there is no segmentation fault.
 * @}
 * @}
 */

START_TEST(utc_eldbus_message_basic_eina_value_p)
{
   is_success = EINA_FALSE;

   Eldbus_Connection *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   ck_assert_ptr_ne(NULL, conn);

   Eldbus_Pending *pending = eldbus_name_owner_has(conn, bus, _response_owner_has_cb, NULL);
   ck_assert_ptr_ne(NULL, pending);

   timeout = ecore_timer_add(1.5, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   ck_assert_msg(is_success, "Problem with eldbus eina value");

   eldbus_connection_unref(conn);
}
END_TEST

/**
 * @addtogroup eldbus_message
 * @{
 * @defgroup eldbus_message_iter_next eldbus_message_iter_next()
 * @{
 * @objective Positive test case checks if function get response from DBus service.
 * Get valid iterator from message response, get data from iterator
 * and move the next field to end, and without segmentation fault.
 *
 * @n Input Data:
 * @li the conn object connection with bus
 *
 * @procedure
 * @step 1 Call eldbus_connection_get function to get connection object
 * @step 2 Check returned connection object on NULL.
 * @step 3 Call eldbus_names_activatable_list send a "ListActivatableNames" method call in proxy.
 * @step 4 Check returned object on NULL.
 * @step 5 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 6 Start of main loop and wait for tested response in callback executing.
 * @step 7 Check static variables named is_success.
 * If are equals 1, that callback was executed and method "ListActivatableNames" send valid message response.
 * Message iterator returned valid iteration data. In other cases error.
 * @step 8 Call eldbus_connection_unref function to delete connection object
 *
 * @passcondition Message response should returned valid iteration data, and worked correctly.
 * @}
 * @}
 */

START_TEST(utc_eldbus_message_iter_next_p)
{
   _activable_list_get(_activatable_list_response_cb);
}
END_TEST

/**
 * @addtogroup eldbus_message
 * @{
 * @defgroup eldbus_message_arguments_vappend eldbus_message_arguments_vappend()
 * @{
 * @objective Positive test case checks if function append arguments into an message object
 * call the method  and get valid response in callback function, and without segmentation fault.
 *
 * @n Input Data:
 * @li the conn object connection with bus
 * @li the ap va_list arguments to message object
 *
 * @procedure
 * @step 1 Call eldbus_connection_get function to get connection object
 * @step 2 Check returned connection object on NULL.
 * @step 3 Call eldbus_message_method_call_new to create a new message to invoke a method on a remote object.
 * @step 4 Check returned object on NULL.
 * @step 5 Call eldbus_message_method_call_new to append arguments into an message object.
 * @step 6 Call eldbus_connection_send function to send a message a dbus object.
 * @step 7 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 8 Start of main loop and wait for tested response in callback executing.
 * @step 9 Check static variables named is_success.
 * If are equals 1, that callback was executed and method send valid message response.
 * In other cases error.
 * @step 10 Call eldbus_message_unref function to delete message object
 * @step 11 Call eldbus_connection_unref function to delete connection object
 *
 * @passcondition Variables named is_success must equals EINA_TRUE. Without segmentation fault.
 * @}
 * @}
 */
START_TEST(utc_eldbus_message_arguments_vappend_p)
{
   is_success = EINA_FALSE;
   Eldbus_Connection *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   ck_assert_ptr_ne(NULL, conn);

   message_vparam = eldbus_message_method_call_new(bus, path, interface, "GetNameOwner");
   ck_assert_ptr_ne(NULL, message_vparam);

   _varg_add("s", bus);

   Eldbus_Pending *pending = eldbus_connection_send(conn, message_vparam, _message_response_cb, NULL, -1);
   ck_assert_ptr_ne(NULL, pending);

   timeout = ecore_timer_add(1.5, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   ck_assert_msg(is_success, "Method GetNameOwner is not call");

   eldbus_message_unref(message_vparam);
   eldbus_connection_unref(conn);
}
END_TEST

void eldbus_test_eldbus_message(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, utc_eldbus_message_iterator_activatable_list_p);
   tcase_add_test(tc, utc_eldbus_message_info_data_get_p);
   tcase_add_test(tc, utc_eldbus_message_signal_new_p);
   tcase_add_test(tc, utc_eldbus_message_ref_unref_p);
   tcase_add_test(tc, utc_eldbus_message_basic_eina_value_p);
   tcase_add_test(tc, utc_eldbus_message_iter_next_p);
   tcase_add_test(tc, utc_eldbus_message_arguments_vappend_p);
}

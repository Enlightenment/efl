#include <Eldbus.h>
#include <Ecore.h>
#include <string.h>
#include <Eina.h>

#include "eldbus_suite.h"

static Eldbus_Message *message_vparam = NULL;
static Eldbus_Message *message_vget = NULL;

static Eina_Bool is_success = EINA_FALSE;
static Eina_Bool is_register_service = EINA_FALSE;
static Eina_Bool is_iter_vget = EINA_FALSE;
static Eina_Bool is_receive = EINA_FALSE;

static const int value_from_client = 10;
static const int value_from_server = 11;

static Ecore_Timer *timeout = NULL;

static const char *empty_string = "";

static const char *bus = "org.freedesktop.DBus";
static const char *interface = "org.freedesktop.DBus";
static const char *path = "/org/freedesktop/DBus";
static const char *method_name = "GetId";
static const char *signal_name = "NameOwnerChanged";

const char *dbus_session_path = "/org/freedesktop/Test";
const char *interface_session = "org.freedesktop.Test";
const char *bus_session = "org.freedesktop.Test";

#define CONTAINER_COUNT 2

static const char *container_data[CONTAINER_COUNT] = {"first", "second"};

static unsigned int numbers_int[] = { 10, 9, 8, 7, 6, 5 };

static int cb_data = 5;

static Eldbus_Message_Iter *iter_value_swap = NULL;
/**
* @addtogroup eldbus
* @{
* @defgroup eldbus_message
*
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
_varg_iter_add(char *str,...)
{
   va_list ap;
   va_start(ap, str);
   eldbus_message_iter_arguments_vappend(iter_value_swap, str, ap);
   va_end(ap);
}

static Eldbus_Message *
_swap_value(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   int num;
   if (eldbus_message_arguments_get(msg, "i", &num))
     {
        if (num == value_from_client)
          {
             iter_value_swap = eldbus_message_iter_get(reply);
             _varg_iter_add("i", value_from_server);
          }

        return reply;
     }

   return reply;
}

static Eldbus_Message *
_receive_container_data(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   Eldbus_Message_Iter *array;
   const char *txt;

   Eina_Bool fail = EINA_FALSE;
   if (eldbus_message_arguments_get(msg, "as", &array))
     {
        int int_iter = 0;
        while (eldbus_message_iter_get_and_next(array, 's', &txt))
          {
             if (!txt || strcmp(txt, container_data[int_iter]))
               {
                  fail = EINA_TRUE;
                  break;
               }
             ++int_iter;
          }
        if (!fail)
          {
             int numbers[] = {10, 9};

             Eldbus_Message_Iter *iter = eldbus_message_iter_get(reply);
             Eldbus_Message_Iter *array = eldbus_message_iter_container_new(iter, 'a', "i");
             if (array)
               {
                  eldbus_message_iter_fixed_array_append(array, 'i', numbers, EINA_C_ARRAY_LENGTH(numbers));
                  eldbus_message_iter_container_close(iter, array);
               }
          }
     }

   is_success = fail ? EINA_FALSE : EINA_TRUE;

   return reply;
}

static void
_on_send_bool(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        if (!strcmp(errmsg, "Invalid type"))
          is_success = EINA_TRUE;
     }
}

static Eldbus_Message *
_send_bool(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidSignature",
                                  "Invalid type");
}

static void
_on_send_array_int(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   Eldbus_Message_Iter *array = NULL;

   if (!eldbus_message_error_get(msg, NULL, NULL))
     {
        if (eldbus_message_arguments_get(msg, "ai", &array))
          {
             unsigned int *array_int;
             int array_count;
             if (eldbus_message_iter_fixed_array_get(array, 'i', &array_int, &array_count))
               {
                  if (EINA_C_ARRAY_LENGTH(numbers_int) == array_count)
                    {
                       int i = 0;
                       Eina_Bool is_equal = EINA_TRUE;
                       for (i = 0; i < array_count; ++i)
                         {
                            if (numbers_int[i] != array_int[i])
                              {
                                 is_equal = EINA_FALSE;
                                 break;
                              }
                         }
                       is_success = is_equal;
                    }
               }
          }
     }
}

static Eldbus_Message *
_send_array_int(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *reply = eldbus_message_method_return_new(msg);
   Eldbus_Message_Iter *iter, *array;

   iter = eldbus_message_iter_get(reply);
   array = eldbus_message_iter_container_new(iter, 'a', "i");
   eldbus_message_iter_fixed_array_append(array, 'i', numbers_int, EINA_C_ARRAY_LENGTH(numbers_int));
   eldbus_message_iter_container_close(iter, array);

   return reply;
}

static const Eldbus_Method methods[] = {
      {
        "SwapValue", ELDBUS_ARGS({"i", "integer"}),
        ELDBUS_ARGS({"i", "swap_value"}), _swap_value, 0
      },
      {
        "ArrayContainer", ELDBUS_ARGS({"as", "container_of_strings"}),
        ELDBUS_ARGS({"ai", "array_of_int"}), _receive_container_data, 0
      },
      { "SendBool", ELDBUS_ARGS({"b", "bool"}), ELDBUS_ARGS({"b", "bool"}),
        _send_bool
      },
      {
        "SendArrayInt", NULL,
        ELDBUS_ARGS({"ai", "array_of_int"}), _send_array_int, 0
      },
      { }
};

static const Eldbus_Service_Interface_Desc iface_desc = {
   "org.freedesktop.Test", methods, NULL
};

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
_vget_response(char *str,...)
{
   va_list ap;

   va_start(ap, str);
   if (!eldbus_message_arguments_vget(message_vget, str, ap))
     return;

   va_end(ap);
}

static void
_message_vget_response_cb(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   if (timeout != NULL)
     {
        ecore_timer_del(timeout);
        timeout = NULL;
     }

   const char *errname, *errmsg;
   if (!eldbus_message_error_get(msg, &errname, &errmsg))
     {
        Eina_Bool *result_response = malloc(sizeof(Eina_Bool));
        if (result_response)
          {
             message_vget = (Eldbus_Message *)msg;
             *result_response = EINA_FALSE;
             _vget_response("b", result_response);

             if (*result_response == EINA_TRUE)
               is_success = EINA_TRUE;

             free(result_response);
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

static void
_response_swap_value(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   Eina_Value *eina_value;
   int swap_value;

   if (!eldbus_message_error_get(msg, NULL, NULL))
    {
       Eldbus_Message_Iter *iter = eldbus_message_iter_get(msg);
       eina_value = eldbus_message_iter_struct_like_to_eina_value(iter);
       eina_value_struct_get(eina_value, "arg0", &swap_value);
       if (swap_value == value_from_server)
         is_success = EINA_TRUE;

       eina_value_free(eina_value);
    }
}

static void
_vget_iter_response(Eldbus_Message_Iter *iter,...)
{
   va_list ap;

   va_start(ap, iter);
   if (!eldbus_message_iter_arguments_vget(iter, "i", ap))
     return;

   va_end(ap);
}

static void
_response_swap_iter_vget(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   int swap_value;

   if (!eldbus_message_error_get(msg, NULL, NULL))
    {
       if (!strcmp(eldbus_message_signature_get(msg), "i"))
         {
            Eldbus_Message_Iter *iter = eldbus_message_iter_get(msg);
            _vget_iter_response(iter, &swap_value);

            if (swap_value == value_from_server)
              is_iter_vget = EINA_TRUE;
         }
    }
}

static void
_fill_value_to_server(Eldbus_Message *msg)
{
   Eina_Value *eina_value;
   Eina_Value_Struct_Member main_members[] = {
      {"value", EINA_VALUE_TYPE_INT, 0}
   };
   Eina_Value_Struct_Desc desc_struct = {
         EINA_VALUE_STRUCT_DESC_VERSION,
         NULL,
         main_members,
         1,
         sizeof(int)
   };

   eina_value = eina_value_struct_new(&desc_struct);
   eina_value_struct_set(eina_value, "value", value_from_client);
   eldbus_message_from_eina_value("i", msg, eina_value);

   eina_value_free(eina_value);
}

static void
_container_receive_cb(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   if (timeout != NULL)
     {
        ecore_timer_del(timeout);
        timeout = NULL;
     }

   if (!eldbus_message_error_get(msg, NULL, NULL))
     {
        int check_numbers[] = {10, 9};
        Eldbus_Message_Iter *array = NULL;
        int num;
        Eina_Bool fail = EINA_FALSE;

        if (eldbus_message_arguments_get(msg, "ai", &array))
          {
             int iter = 0;
             while (eldbus_message_iter_get_and_next(array, 'i', &num))
               {
                  if (check_numbers[iter] != num)
                    {
                       fail = EINA_TRUE;
                       break;
                    }

                  ++iter;
               }
          }

        is_receive = fail ? EINA_FALSE : EINA_TRUE;
     }

   ecore_main_loop_quit();
}

static void
_hello_cb(void *data, const Eldbus_Message *msg EINA_UNUSED, Eldbus_Pending *pending EINA_UNUSED)
{
   int *user_data = data;
   if (user_data && *user_data == cb_data)
     is_success = EINA_TRUE;
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

EFL_START_TEST(utc_eldbus_message_iterator_activatable_list_p)
{
   _activable_list_get(_response_message_cb);
}
EFL_END_TEST

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

EFL_START_TEST(utc_eldbus_message_info_data_get_p)
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

   eldbus_connection_unref(conn);
}
EFL_END_TEST

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

EFL_START_TEST(utc_eldbus_message_signal_new_p)
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

   eldbus_signal_handler_del(signal_handler);
   eldbus_connection_unref(conn);
}
EFL_END_TEST

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

EFL_START_TEST(utc_eldbus_message_ref_unref_p)
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
EFL_END_TEST

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

EFL_START_TEST(utc_eldbus_message_basic_eina_value_p)
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
EFL_END_TEST

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

EFL_START_TEST(utc_eldbus_message_iter_next_p)
{
   _activable_list_get(_activatable_list_response_cb);
}
EFL_END_TEST

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
EFL_START_TEST(utc_eldbus_message_arguments_vappend_p)
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

   eldbus_connection_unref(conn);
}
EFL_END_TEST

/**
 * @addtogroup eldbus_message
 * @{
 * @defgroup eldbus_message_arguments_vget
 * @li eldbus_message_arguments_vget()
 * @li eldbus_message_arguments_append()
 * @li eldbus_message_error_get()
 * @{
 * @objective Positive test case checks if function call the method
 * and get valid response in callback function used va_list, and without segmentation fault.
 *
 * @n Input Data:
 * @li the conn object connection with bus
 *
 * @procedure
 * @step 1 Call eldbus_connection_get function to get connection object
 * @step 2 Check returned connection object on NULL.
 * @step 3 Call eldbus_message_method_call_new to create a new message to invoke a method on a remote object.
 * @step 4 Check returned object on NULL.
 * @step 5 Call eldbus_message_method_call_new to append arguments into an message object.
 * @step 6 Call eldbus_message_arguments_append to append arguments into an message object.
 * @step 7 Call eldbus_connection_send function to send a message a dbus object.
 * @step 8 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 9 Start of main loop and wait for tested response in callback executing.
 * @step 10 Check static variables named is_success.
 * If are equals 1, that callback was executed and get the response arguments
 * from an message object using a va_list without error. In other cases error.
 * @step 11 Call eldbus_message_unref function to delete message object.
 * @step 12 Call eldbus_connection_unref function to delete connection object.
 *
 * @passcondition Variables named is_success must equals EINA_TRUE. Without segmentation fault.
 * @}
 * @}
 */
EFL_START_TEST(utc_eldbus_message_arguments_vget_p)
{
   is_success = EINA_FALSE;
   Eldbus_Connection *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   ck_assert_ptr_ne(NULL, conn);

   Eldbus_Message *message = eldbus_message_method_call_new(bus, path, interface, "NameHasOwner");
   ck_assert_ptr_ne(NULL, message);

   Eina_Bool is_arguments_append = eldbus_message_arguments_append(message, "s", bus);
   ck_assert_msg(is_arguments_append, "Can't append arguments");

   Eldbus_Pending *pending = eldbus_connection_send(conn, message, _message_vget_response_cb, NULL, -1);
   ck_assert_ptr_ne(NULL, pending);

   timeout = ecore_timer_add(1.5, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   ck_assert_msg(is_success, "Can't get arguments");

   eldbus_connection_unref(conn);
}
EFL_END_TEST

/**
 * @addtogroup eldbus_message
 * @{
 * @defgroup eldbus_message_eina
 * @li eldbus_message_from_eina_value()
 * @li eldbus_message_iter_struct_like_to_eina_value()
 * @li eldbus_message_iter_arguments_vappend()
 * @li eldbus_message_iter_arguments_vget()
 * @li eldbus_message_signature_get()
 * @li eldbus_message_method_return_new()
 * @li eldbus_message_error_get()
 * @{
 * @objective Positive test case checks if function convert Eina_Value to message object
 * and backward using iterator to message object without error. Set data to message object iterator using va_list.
 * Get data from message iterator using va_list. Get the Eldbus message signature. Without segmentation fault.
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
 * @step 11 Call eldbus_proxy_get function to get a proxy of the following interface name.
 * @step 12 Check returned proxy object on NULL.
 * @step 13 Call eldbus_proxy_method_call_new function to constructs
 * a new message to invoke a method on a remote interface.
 * @step 14 Check returned message object on NULL.
 * @step 15 Convert Eina_Value to message object.
 * @step 16 Call eldbus_proxy_send function to send a message by method on remote interface.
 * @step 17 Check returned pending object on NULL.
 * @step 18 Call eldbus_proxy_call function to Call a method in proxy.
 * @step 19 Check returned pending object on NULL.
 * @step 20 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 21 Start of main loop and wait for tested response in callback executing.
 * @step 22 Check static variables named is_success, is_register_service, is_iter_vget.
 * If are equals 1, that get and set property works correctly,
 * get expected response with expected signature from client and server without error. In other cases error.
 * @step 23 Call eldbus_message_unref function to delete message object.
 * @step 24 Call eldbus_proxy_unref function to delete proxy object.
 * @step 25 Call eldbus_object_unref function to delete connection dbus object.
 * @step 26 Call eldbus_connection_unref function to delete client connection object.
 * @step 27 Call eldbus_service_interface_unregister function to unregister a interface.
 * @step 28 Call eldbus_connection_unref function to delete server connection object
 *
 * @passcondition Variables named is_success_cb, is_register_service, is_iter_vget must equals 1, and there is no segmentation fault.
 * @}
 * @}
 */
EFL_START_TEST(utc_eldbus_message_eina_p)
{
   is_success = EINA_FALSE;
   is_register_service = EINA_FALSE;
   is_iter_vget = EINA_FALSE;

   Eldbus_Connection *conn_server = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   ck_assert_ptr_ne(NULL, conn_server);

   Eldbus_Service_Interface *iface = eldbus_service_interface_register(conn_server, dbus_session_path, &iface_desc);
   ck_assert_ptr_ne(NULL, iface);

   Eldbus_Pending *pending_name = eldbus_name_request(conn_server, bus_session, ELDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                                                      _name_request, iface);
   ck_assert_ptr_ne(NULL, pending_name);

   Eldbus_Connection *conn_client = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   ck_assert_ptr_ne(NULL, conn_client);

   Eldbus_Object *obj = eldbus_object_get(conn_client, bus_session, dbus_session_path);
   ck_assert_ptr_ne(NULL, obj);

   Eldbus_Proxy *proxy = eldbus_proxy_get(obj, interface_session);
   ck_assert_ptr_ne(NULL, proxy);

   Eldbus_Message *msg = eldbus_proxy_method_call_new(proxy, "SwapValue");
   ck_assert_ptr_ne(NULL, msg);

   _fill_value_to_server(msg);
   Eldbus_Pending *pending = eldbus_proxy_send(proxy, msg, _response_swap_value, NULL, -1);
   ck_assert_ptr_ne(NULL, pending);

   pending = eldbus_proxy_call(proxy, "SwapValue", _response_swap_iter_vget, NULL, -1 , "i", value_from_client);
   ck_assert_ptr_ne(NULL, pending);

   timeout = ecore_timer_add(2.5, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   ck_assert_msg(is_success, "Can't get response");
   ck_assert_msg(is_register_service, "Can't registered service");
   ck_assert_msg(is_iter_vget, "Can't get argument");

   eldbus_connection_unref(conn_client);
   eldbus_service_interface_unregister(iface);
   eldbus_connection_unref(conn_server);
}
EFL_END_TEST

/**
 * @addtogroup eldbus_message
 * @{
 * @defgroup eldbus_message_container
 * @li eldbus_message_iter_container_new()
 * @li eldbus_message_iter_container_close()
 * @li eldbus_message_iter_fixed_array_append()
 * @li eldbus_message_method_return_new()
 * @{
 * @objective Positive test case checks if function create and append a typed iterator to message iterator.
 * Closes a container-typed value appended to the message object. Append a array of basic type with fixed size
 * to message reply iterator. Without segmentation fault.
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
 * @step 11 Call eldbus_proxy_get function to get a proxy of the following interface name.
 * @step 12 Check returned proxy object on NULL.
 * @step 13 Call eldbus_proxy_method_call_new function to constructs
 * a new message to invoke a method on a remote interface.
 * @step 14 Check returned message object on NULL.
 * @step 15 Call eldbus_message_iter_get function to get the main message iterator from the message object.
 * @step 16 Call eldbus_message_iter_container_new function to create and append a typed iterator to main iterator.
 * @step 17 Check returned iterator on NULL.
 * @step 18 In loop call eldbus_message_iter_basic_append function to append a basic type into an new iterator
 * @step 19 Call eldbus_message_iter_container_close function to closes a container-typed value appended to the message object.
 * @step 20 Call eldbus_proxy_send function to send a message by method on remote interface.
 * @step 21 Check returned pending object on NULL.
 * @step 22 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 23 Start of main loop and wait for tested response in callback executing.
 * @step 24 Check static variables named is_success, is_register_service, is_receive.
 * If are equals 1, that get and set property works correctly,
 * get expected response from client and server without error. In other cases error.
 * @step 25 Call eldbus_message_unref function to delete message object.
 * @step 26 Call eldbus_proxy_unref function to delete proxy object.
 * @step 27 Call eldbus_object_unref function to delete connection dbus object.
 * @step 28 Call eldbus_connection_unref function to delete client connection object.
 * @step 29 Call eldbus_service_interface_unregister function to unregister a interface.
 * @step 30 Call eldbus_connection_unref function to delete server connection object
 *
 * @passcondition Variables named is_success_cb, is_register_service, is_receive must equals 1, and there is no segmentation fault.
 * @}
 * @}
 */
EFL_START_TEST(utc_eldbus_message_container_p)
{
   int i;

   is_success = EINA_FALSE;
   is_register_service = EINA_FALSE;
   is_receive = EINA_FALSE;

   Eldbus_Connection *conn_server = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   ck_assert_ptr_ne(NULL, conn_server);

   Eldbus_Service_Interface *iface = eldbus_service_interface_register(conn_server, dbus_session_path, &iface_desc);
   ck_assert_ptr_ne(NULL, iface);

   Eldbus_Pending *pending_name = eldbus_name_request(conn_server, bus_session, ELDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                                                      _name_request, iface);
   ck_assert_ptr_ne(NULL, pending_name);

   Eldbus_Connection *conn_client = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   ck_assert_ptr_ne(NULL, conn_client);

   Eldbus_Object *obj = eldbus_object_get(conn_client, bus_session, dbus_session_path);
   ck_assert_ptr_ne(NULL, obj);

   Eldbus_Proxy *proxy = eldbus_proxy_get(obj, interface_session);
   ck_assert_ptr_ne(NULL, proxy);

   Eldbus_Message *msg = eldbus_proxy_method_call_new(proxy, "ArrayContainer");
   ck_assert_ptr_ne(NULL, msg);

   Eldbus_Message_Iter *iter = eldbus_message_iter_get(msg);

   Eldbus_Message_Iter *container_iter = eldbus_message_iter_container_new(iter, 'a',"s");
   ck_assert_ptr_ne(NULL, container_iter);

   for (i = 0; i < CONTAINER_COUNT; i++)
     eldbus_message_iter_basic_append(container_iter, 's', container_data[i]);

   Eina_Bool is_container_iter_closed = eldbus_message_iter_container_close(iter, container_iter);
   ck_assert_msg(is_container_iter_closed, "Can't closed container");

   Eldbus_Pending *pending = eldbus_proxy_send(proxy, msg, _container_receive_cb, NULL, -1);
   ck_assert_ptr_ne(NULL, pending);

   timeout = ecore_timer_add(2.5, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   ck_assert_msg(is_success, "Can't get response");
   ck_assert_msg(is_register_service, "Can't registered service");
   ck_assert_msg(is_receive, "Can't get container data");

   eldbus_connection_unref(conn_client);
   eldbus_service_interface_unregister(iface);
   eldbus_connection_unref(conn_server);
}
EFL_END_TEST

/**
 * @addtogroup eldbus_message
 * @{
 * @defgroup eldbus_message_error_new eldbus_message_error_new()
 * @{
 * @objective Positive test case checks if function create a new message that is an error reply to another message.
 * Without segmentation fault.
 *
 * @n Input Data:
 * @li the message object replying to
 * @li the error name "org.freedesktop.DBus.Error.InvalidSignature"
 * @li the error message "Invalid type" string
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
 * @step 11 Call eldbus_proxy_get function to get a proxy of the following interface name.
 * @step 12 Check returned proxy object on NULL.
 * @step 13 Call eldbus_proxy_call function to call a method "SendBool" in proxy.
 * @step 14 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 15 Start of main loop and wait for tested response in callback executing.
 * @step 16 Call eldbus_proxy_unref function to delete proxy object.
 * @step 17 Call eldbus_object_unref function to delete connection dbus object.
 * @step 18 Call eldbus_connection_unref function to delete client connection object.
 * @step 19 Call eldbus_service_interface_unregister function to unregister a interface.
 * @step 20 Call eldbus_connection_unref function to delete server connection object
 * @step 21 Check static variable named is_success.
 * If is equal 1, that error reply to another message works correctly. In other cases error.
 *
 * @passcondition Variable named is_success_cb must equal 1, and there is no segmentation fault.
 * @}
 * @}
 */
EFL_START_TEST(utc_eldbus_message_error_new_p)
{
   is_success = EINA_FALSE;

   Eldbus_Connection *conn_server = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   ck_assert_ptr_ne(NULL, conn_server);

   Eldbus_Service_Interface *iface = eldbus_service_interface_register(conn_server, dbus_session_path, &iface_desc);
   ck_assert_ptr_ne(NULL, iface);

   Eldbus_Pending *pending_name = eldbus_name_request(conn_server, bus_session, ELDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                                                      _name_request, iface);
   ck_assert_ptr_ne(NULL, pending_name);

   Eldbus_Connection *conn_client = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   ck_assert_ptr_ne(NULL, conn_client);

   Eldbus_Object *obj = eldbus_object_get(conn_client, bus_session, dbus_session_path);
   ck_assert_ptr_ne(NULL, obj);

   Eldbus_Proxy *proxy = eldbus_proxy_get(obj, interface_session);
   ck_assert_ptr_ne(NULL, proxy);

   eldbus_proxy_call(proxy, "SendBool", _on_send_bool, NULL, -1, "b", NULL);

   timeout = ecore_timer_add(2.5, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   eldbus_connection_unref(conn_client);
   eldbus_service_interface_unregister(iface);
   eldbus_connection_unref(conn_server);

   ck_assert_msg(is_success, "Can't get response");
}
EFL_END_TEST

/**
 * @addtogroup eldbus_message
 * @{
 * @defgroup eldbus_message_iter_del eldbus_message_iter_del()
 * @{
 * @objective Positive test case checks if function manually delete the iterator without segmentation fault.
 *
 * @n Input Data:
 * @li the iterator to be deleted
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
 * @step 11 Create message object to call method "SendByte" and append arguments
 * @step 12 Get iterator from message object and check on NULL
 * @step 13 Manually delete the iterator
 * @step 14 Call eldbus_object_unref function to delete connection dbus object.
 * @step 15 Call eldbus_connection_unref function to delete client connection object.
 * @step 16 Call eldbus_service_interface_unregister function to unregister a interface.
 * @step 17 Call eldbus_connection_unref function to delete server connection object
 *
 * @passcondition Function should manually delete the message iterator without segmentation fault.
 * @}
 * @}
 */
EFL_START_TEST(utc_eldbus_message_iter_del_p)
{
   Eldbus_Connection *conn_server = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   ck_assert_ptr_ne(NULL, conn_server);

   Eldbus_Service_Interface *iface = eldbus_service_interface_register(conn_server, dbus_session_path, &iface_desc);
   ck_assert_ptr_ne(NULL, iface);

   Eldbus_Pending *pending_name = eldbus_name_request(conn_server, bus_session, ELDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                                                      _name_request, iface);
   ck_assert_ptr_ne(NULL, pending_name);

   Eldbus_Connection *conn_client = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   ck_assert_ptr_ne(NULL, conn_client);

   Eldbus_Object *obj = eldbus_object_get(conn_client, bus_session, dbus_session_path);
   ck_assert_ptr_ne(NULL, obj);

   Eldbus_Message *msg = eldbus_object_method_call_new(obj, "org.freedesktop.Test", "SendByte");
   ck_assert_ptr_ne(NULL, msg);

   eldbus_message_arguments_append(msg, "y", 25);

   Eldbus_Message_Iter *iter = NULL;
   if ((iter = eldbus_message_iter_get(msg)))
     {
        eldbus_message_iter_del(iter);
     }

   eldbus_connection_unref(conn_client);
   eldbus_service_interface_unregister(iface);
   eldbus_connection_unref(conn_server);
}
EFL_END_TEST

/**
 * @addtogroup eldbus_message
 * @{
 * @defgroup eldbus_message_iter_fixed_array_get eldbus_message_iter_fixed_array_get()
 * @{
 * @objective Positive test case checks if function gets a block of fixed-length values from the message iterator. Without segmentation fault.
 *
 * @n Input Data:
 * @li the message iterator
 * @li 'i' the signature block of fixed-length values
 * @li the block of fixed-length values
 * @li the count elemenats
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
 * @step 11 Call eldbus_proxy_get function to get a proxy of the following interface name.
 * @step 12 Check returned proxy object on NULL.
 * @step 13 Call a method "SendArrayInt" in proxy
 * @step 14 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 15 Start of main loop and wait for tested response in callback executing.
 * @step 16 Call eldbus_proxy_unref function to delete proxy object.
 * @step 17 Call eldbus_object_unref function to delete connection dbus object.
 * @step 18 Call eldbus_connection_unref function to delete client connection object.
 * @step 19 Call eldbus_service_interface_unregister function to unregister a interface.
 * @step 20 Call eldbus_connection_unref function to delete server connection object
 * @step 21 Check static variable named is_success.
 * If is equal 1, that get expected block of fixed-length values without error. In other cases error.
 *
 * @passcondition Variable named is_success must equals 1, and there is no segmentation fault.
 * @}
 * @}
 */
EFL_START_TEST(utc_eldbus_message_iter_fixed_array_get_p)
{
   is_success = EINA_FALSE;

   Eldbus_Connection *conn_server = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   ck_assert_ptr_ne(NULL, conn_server);

   Eldbus_Service_Interface *iface = eldbus_service_interface_register(conn_server, dbus_session_path, &iface_desc);
   ck_assert_ptr_ne(NULL, iface);

   Eldbus_Pending *pending_name = eldbus_name_request(conn_server, bus_session, ELDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE,
                                                      _name_request, iface);
   ck_assert_ptr_ne(NULL, pending_name);

   Eldbus_Connection *conn_client = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   ck_assert_ptr_ne(NULL, conn_client);

   Eldbus_Object *obj = eldbus_object_get(conn_client, bus_session, dbus_session_path);
   ck_assert_ptr_ne(NULL, obj);

   Eldbus_Proxy *proxy = eldbus_proxy_get(obj, interface_session);
   ck_assert_ptr_ne(NULL, proxy);

   Eldbus_Pending *pending = eldbus_proxy_call(proxy, "SendArrayInt", _on_send_array_int, NULL, -1 , "");
   ck_assert_ptr_ne(NULL, pending);

   timeout = ecore_timer_add(2.5, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   eldbus_connection_unref(conn_client);
   eldbus_service_interface_unregister(iface);
   eldbus_connection_unref(conn_server);

   ck_assert_msg(is_success, "Can't get response");

}
EFL_END_TEST

/**
 * @addtogroup eldbus_message
 * @{
 * @defgroup eldbus_hello eldbus_hello()
 * @{
 * @objective Positive test case checks if function send a "Hello" method call in proxy and get response.
 * Without segmentation fault.
 *
 * @n Input Data:
 * @li the сonnection object
 * @li the сallback to call when receiving answer
 * @li the 5 value data passed to callback
 *
 * @procedure
 * @step 1 Call eldbus_connection_get function to get server connection object
 * @step 2 Check returned connection object on NULL
 * @step 3 Call eldbus_hello function to send a "Hello" method call in proxy
 * @step 4 Set timer for preschedule termination of main loop if tested callback was executed
 * @step 5 Start of main loop and wait for tested response in callback executing
 * @step 6 Call eldbus_connection_unref function to delete server connection object
 * @step 7 Check static variable named is_success.
 * If is equal EINA_TRUE, that callback was executed. In other cases error
 *
 * @passcondition Variable named is_success must equal EINA_TRUE, and there is no segmentation fault
 * @}
 * @}
 */
EFL_START_TEST(utc_eldbus_hello_p)
{
   is_success = EINA_FALSE;

   Eldbus_Connection *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   ck_assert_ptr_ne(NULL, conn);

   eldbus_hello(conn, _hello_cb, &cb_data);

   timeout = ecore_timer_add(1.5, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   eldbus_connection_unref(conn);

   ck_assert_msg(is_success, "Can't get response");
}
EFL_END_TEST

void eldbus_test_eldbus_message(TCase *tc)
{
   tcase_add_test(tc, utc_eldbus_message_iterator_activatable_list_p);
   tcase_add_test(tc, utc_eldbus_message_info_data_get_p);
   tcase_add_test(tc, utc_eldbus_message_signal_new_p);
   tcase_add_test(tc, utc_eldbus_message_ref_unref_p);
   tcase_add_test(tc, utc_eldbus_message_basic_eina_value_p);
   tcase_add_test(tc, utc_eldbus_message_iter_next_p);
   tcase_add_test(tc, utc_eldbus_message_arguments_vappend_p);
   tcase_add_test(tc, utc_eldbus_message_arguments_vget_p);
   tcase_add_test(tc, utc_eldbus_message_eina_p);
   tcase_add_test(tc, utc_eldbus_message_container_p);
   tcase_add_test(tc, utc_eldbus_message_error_new_p);
   tcase_add_test(tc, utc_eldbus_message_iter_del_p);
   tcase_add_test(tc, utc_eldbus_message_iter_fixed_array_get_p);
   tcase_add_test(tc, utc_eldbus_hello_p);
}

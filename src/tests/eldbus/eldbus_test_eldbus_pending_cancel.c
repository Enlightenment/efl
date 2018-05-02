#include <check.h>
#include <Eldbus.h>
#include <Ecore.h>
#include <Eina.h>
#include <string.h>

#include "eldbus_suite.h"

static Eina_Bool is_success_cb = EINA_FALSE;

static Ecore_Timer *timeout = NULL;

static Eldbus_Connection *conn = NULL;
static Eldbus_Object *obj = NULL;
static Eldbus_Message *message = NULL;

const char *bus = "org.freedesktop.DBus";
const char *path = "/org/freedesktop/DBus";
const char *interface = "org.freedesktop.DBus";
const char *member = "GetId";

/**
* @addtogroup eldbus
* @{
* @defgroup eldbus_pending_info_get_cancel
* @li eldbus_pending_cancel()
* @li eldbus_pending_destination_get()
* @li eldbus_pending_interface_get()
* @li eldbus_pending_method_get()
* @li eldbus_pending_path_get()
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
_response_message_cb(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   if (timeout != NULL)
     {
        ecore_timer_del(timeout);
        timeout = NULL;
     }

   if (!pending)
     {
        ecore_main_loop_quit();
        return;
     }

   const char *pending_path = eldbus_pending_path_get(pending);
   if ((!pending_path) || (strcmp(pending_path, path)))
     {
        ecore_main_loop_quit();
        return;
     }

   const char *pending_method = eldbus_pending_method_get(pending);
   if ((!pending_method) || (strcmp(pending_method, member)))
     {
        ecore_main_loop_quit();
        return;
     }

   const char *pending_interface = eldbus_pending_interface_get(pending);
   if ((!pending_interface) || (strcmp(pending_interface, interface)))
     {
        ecore_main_loop_quit();
        return;
     }

   const char *pending_destination = eldbus_pending_destination_get(pending);
   if ((!pending_destination) || (strcmp(pending_destination, bus)))
     {
        ecore_main_loop_quit();
        return;
     }

   const char *errname, *errmsg;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        is_success_cb = EINA_TRUE;
     }

   ecore_main_loop_quit();
}

static Eldbus_Pending *
_pending_connection_get()
{
   const int send_timeout_ms = 500;

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   if (!conn)
     {
        return NULL;
     }

   obj = eldbus_object_get(conn, bus, path);
   if (!obj)
     {
        eldbus_connection_unref(conn);
        return NULL;
     }

   message = eldbus_object_method_call_new(obj, interface, member);
   if (!message)
     {
        eldbus_connection_unref(conn);
        return NULL;
     }

   Eldbus_Pending *pending = eldbus_connection_send(conn, message, _response_message_cb, NULL, send_timeout_ms);
   if (!pending)
     {
        eldbus_connection_unref(conn);
        return NULL;
     }

   return pending;
}

/**
 * @addtogroup eldbus_pending_info_get_cancel
 * @{
 * @objective Positive test case checks if pending connection object canceled success.
 * If functions eldbus_pending_cancel() eldbus_pending_destination_get()
 * eldbus_pending_interface_get() eldbus_pending_method_get() eldbus_pending_path_get()
 * returned expected values
 *
 * @n Input Data:
 * @li the pending connection object
 * @li callback function to be called
 * @li the bus name of bus who listens
 * @li the path Dbus object path
 * @li the interface Dbus interface name
 * @li the member name of the method to be called.
 *
 * @procedure
 * @step 1 Get pendindig connection object and check on NULL
 * @step 2 Call eldbus_pending_cancel to cancel the pending message
 * @step 3 Set timer for preschedule termination of main loop if tested callback wasn't executed.
 * @step 4 Start of main loop and wait for tested response in callback executing.
 * @step 5 Check static variable named is_success_cb.
 * If are equals 1, that pending message was canceled and pending object
 * returned expeted information about connection services. In other cases error.
 * @step 6 Call eldbus_message_unref function to delete message object
 * @step 7 Call eldbus_object_unref function to delete connection dbus object
 * @step 8 Call eldbus_connection_unref function to delete connection object
 *
 * @passcondition Variables named is_success_cb must equals 1, and there is no segmentation fault.
 * @}
 */

EFL_START_TEST(utc_eldbus_pending_info_get_cancel_p)
{
   Eldbus_Pending *pending = _pending_connection_get();
   ck_assert_ptr_ne(NULL, pending);

   eldbus_pending_cancel(pending);

   timeout = ecore_timer_add(1.5, _ecore_loop_close, NULL);
   ck_assert_ptr_ne(NULL, timeout);

   ecore_main_loop_begin();

   ck_assert(is_success_cb == EINA_TRUE);

   eldbus_connection_unref(conn);
}
EFL_END_TEST

void
eldbus_test_eldbus_pending_cancel(TCase *tc)
{
   tcase_add_test(tc, utc_eldbus_pending_info_get_cancel_p);
}

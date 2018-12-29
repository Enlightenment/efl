/*
 * compile with:
 * gcc -o client client.c `pkg-config --cflags --libs eldbus ecore eina`
 *
 * This simple example connects to an IBus daemon running on the machine.
 * IBus daemon is a dbus daemon running apart from system or session buses.
 *
 * Requires ibus-daemon to be running, for instance:
 * $ ibus-daemon -v -r -s
 *
 * After starting ibus-daemon, check for its address on the file located on
 * ~/.config/ibus/bus/ directory. For instance:
 *
 * $ cat ~/.config/ibus/bus/809ee018614317e782464b8900000029-unix-0
 * # This file is created by ibus-daemon, please do not modify it
 * IBUS_ADDRESS=unix:abstract=/tmp/dbus-VOJaWARY,guid=e0082807a08a81ad22e0d3f551d48d5e
 * IBUS_DAEMON_PID=3697
 *
 * Export or set the IBUS_ADDRESS environment variable and run this program. To exit,
 * just hit ctrl+c
 */

#include <stdlib.h>
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <signal.h>

#include "Eldbus.h"
#include <Ecore.h>

#define IBUS_BUS "org.freedesktop.IBus"
#define PANEL_BUS "org.freedesktop.IBUS.Panel"
#define PANEL_INTERFACE "org.freedesktop.IBUS.Panel"

static int _conn_addr_log_dom = -1;
#define DBG(...) EINA_LOG_DOM_DBG(_conn_addr_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_conn_addr_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_conn_addr_log_dom, __VA_ARGS__)

static Eldbus_Connection *conn;
static Eldbus_Signal_Handler *acquired;
static Eldbus_Signal_Handler *lost;

static void
on_name_owner_changed(void *data EINA_UNUSED, const char *bus, const char *old_id, const char *new_id EINA_UNUSED)
{
   INF("Name owner changed: bus=%s | old=%s | new=%s\n", bus, old_id, new_id);
}

static void
on_name_acquired(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *error, *error_msg, *name;

   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        ERR("Name acquired message error: %s: %s\n", error, error_msg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "s", &name))
     {
        ERR("Error reading message arguments");
        return;
     }

   INF("Name acquired: %s", name);
}

static void
on_name_lost(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *error, *error_msg, *name;

   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        ERR("Name lost message error: %s: %s\n", error, error_msg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "s", &name))
     {
        ERR("Error reading message arguments");
        return;
     }

   INF("Name lost: %s", name);
}

static void
name_request_cb(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *error, *error_msg;
   unsigned int reply;

   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        ERR("Name request message error: %s: %s\n", error, error_msg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "u", &reply))
     {
        ERR("Error reading message arguments");
        return;
     }

   switch (reply)
     {
      case ELDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER:
         INF("Got primary ownership");
         break;
      case ELDBUS_NAME_REQUEST_REPLY_IN_QUEUE:
         INF("Got queued for ownership");
         break;
      case ELDBUS_NAME_REQUEST_REPLY_EXISTS:
         INF("Already in queue for ownership");
         break;
      case ELDBUS_NAME_REQUEST_REPLY_ALREADY_OWNER:
         INF("Already primary owner");
         break;
      default:
         ERR("Unexpected reply: %d", reply);
         break;
     }
}

static Eina_Bool
main_loop_quit_idler(void *data EINA_UNUSED)
{
   eldbus_signal_handler_del(acquired);
   eldbus_signal_handler_del(lost);
   eldbus_connection_unref(conn);

   INF("Finishing");
   ecore_main_loop_quit();
   return ECORE_CALLBACK_CANCEL;
}

static void
name_release_cb(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *error, *error_msg;
   unsigned int reply;

   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        ERR("Name release message error: %s: %s\n", error, error_msg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "u", &reply))
     {
        ERR("Error reading message arguments");
        return;
     }

   switch (reply)
     {
      case ELDBUS_NAME_RELEASE_REPLY_RELEASED:
         INF("Name released");
         break;
      case ELDBUS_NAME_RELEASE_REPLY_NON_EXISTENT:
         INF("Name non existent");
         break;
      case ELDBUS_NAME_RELEASE_REPLY_NOT_OWNER:
         INF("Not owner");
         break;
      default:
         ERR("Unexpected reply: %d", reply);
         break;
     }

   ecore_idler_add(main_loop_quit_idler, NULL);
}

static void
finish(int foo EINA_UNUSED)
{
   eldbus_name_release(conn, PANEL_BUS, name_release_cb, NULL);
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   const char *address;

   /* Init */
   if (!eina_init())
     {
        EINA_LOG_ERR("Failed to initialize Eina");
        return 1;
     }

   _conn_addr_log_dom = eina_log_domain_register("connect_address", EINA_COLOR_CYAN);

   if (_conn_addr_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create 'client' log domain");
        goto exit_eina;
     }

   if (!(address = getenv("IBUS_ADDRESS")))
     {
        ERR("IBUS_ADDRESS environment variable must be set");
        goto exit_eina;
     }

   if (!ecore_init())
     {
        EINA_LOG_ERR("Failed to initialize Ecore");
        goto exit_eina;
     }

   if (!eldbus_init())
     {
        EINA_LOG_ERR("Failed to initialize Eldbus");
        goto exit_ecore;
     }

   signal(SIGTERM, finish);
   signal(SIGINT, finish);

   /* Connect */
   printf("Connecting to IBus with address: %s\n", address);
   conn = eldbus_address_connection_get(address);

   if (!conn)
     {
        ERR("Failed to get dbus connection with address '%s'\n", address);
        goto end;
     }

   eldbus_name_owner_changed_callback_add(conn, IBUS_BUS, on_name_owner_changed,
                                         conn, EINA_TRUE);

   eldbus_name_owner_changed_callback_add(conn, PANEL_BUS, on_name_owner_changed,
                                         conn, EINA_TRUE);

   acquired = eldbus_signal_handler_add(conn,
                                        ELDBUS_FDO_BUS,
                                        ELDBUS_FDO_PATH,
                                        PANEL_INTERFACE,
                                        "NameAcquired",
                                        on_name_acquired,
                                        NULL);

   lost = eldbus_signal_handler_add(conn,
                                    ELDBUS_FDO_BUS,
                                    ELDBUS_FDO_PATH,
                                    PANEL_INTERFACE,
                                    "NameLost",
                                    on_name_lost,
                                    NULL);

   /* Request ownership of PANEL_BUS */
   eldbus_name_request(conn, PANEL_BUS,
                       ELDBUS_NAME_REQUEST_FLAG_REPLACE_EXISTING | ELDBUS_NAME_REQUEST_FLAG_ALLOW_REPLACEMENT,
                       name_request_cb, NULL);

   ecore_main_loop_begin();

end:
   eldbus_shutdown();

exit_ecore:
   ecore_shutdown();
   eina_log_domain_unregister(_conn_addr_log_dom);

exit_eina:
   eina_shutdown();

   return 0;
}

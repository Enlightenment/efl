//Compile with:
// gcc -o client client.c `pkg-config --cflags --libs eldbus ecore eina`

#include <stdlib.h>

#include "Eldbus.h"
#include <Ecore.h>

#define BUS "org.freedesktop.IBus"
#define PATH "/org/freedesktop/IBus"
#define INTERFACE "org.freedesktop.IBus"

static int _client_log_dom = -1;
#define ERR(...)      EINA_LOG_DOM_ERR(_client_log_dom, __VA_ARGS__)

static void
_on_registry_changed(void *context EINA_UNUSED, const Eldbus_Message *msg EINA_UNUSED)
{
   printf("RegistryChanged\n\n");
}

static void
_on_global_engine_changed(void *context EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *txt;
   if (eldbus_message_arguments_get(msg, "s", &txt))
     printf("GlobalEngineChanged %s\n", txt);
}

static void
on_name_owner_changed(void *data EINA_UNUSED, const char *bus, const char *old_id, const char *new_id EINA_UNUSED)
{
   printf("Bus=%s | old=%s | new=%s\n", bus, old_id, new_id);
}

static Eina_Bool
finish(void *data EINA_UNUSED)
{
   ecore_main_loop_quit();
   return ECORE_CALLBACK_CANCEL;
}

int
main(void)
{
   Eldbus_Connection *conn;
   Eldbus_Object *obj;
   Eldbus_Proxy *proxy;
   const char *address;

   eina_init();
   _client_log_dom = eina_log_domain_register("connect_address", EINA_COLOR_CYAN);
   if (_client_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create 'client' log domain");
        goto exit_eina;
     }

   if (!(address = getenv("IBUS_ADDRESS")))
     {
        ERR("IBUS_ADDRESS environment variable must be set");
        goto exit_eina;
     }

   ecore_init();
   eldbus_init();

   printf("Connecting to %s\n", address);
   conn = eldbus_address_connection_get(address);

   if (!conn)
     {
        ERR("Failed to get dbus connection to address '%s'", address);
        goto end;
     }

   printf("CONNECTED!!!\n");
   obj = eldbus_object_get(conn, BUS, PATH);
   proxy = eldbus_proxy_get(obj, INTERFACE);
   eldbus_proxy_signal_handler_add(proxy, "RegistryChanged", _on_registry_changed, NULL);
   eldbus_proxy_signal_handler_add(proxy, "GlobalEngineChanged", _on_global_engine_changed, NULL);

   eldbus_name_owner_changed_callback_add(conn, BUS, on_name_owner_changed,
                                         conn, EINA_TRUE);
   ecore_timer_add(30, finish, NULL);

   ecore_main_loop_begin();

   eldbus_connection_unref(conn);

end:
   eldbus_shutdown();
   ecore_shutdown();

   eina_log_domain_unregister(_client_log_dom);
exit_eina:
   eina_shutdown();

   return 0;
}

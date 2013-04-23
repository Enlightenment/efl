//Compile with:
// gcc -o simple-signal-emit simple-signal-emit.c `pkg-config --cflags --libs eldbus`

#include <Eldbus.h>

enum {
   TEST_SIGNAL_ALIVE,
   TEST_SIGNAL_PROP,
   TEST_SIGNAL_NAME,
};

static const Eldbus_Signal test_signals[] = {
   [TEST_SIGNAL_ALIVE] = { "Alive" },
   [TEST_SIGNAL_PROP] = { "Properties", ELDBUS_ARGS({ "a{ss}", "properties"}) },
   [TEST_SIGNAL_NAME] = { "Name", ELDBUS_ARGS({ "s", "name"}) },
   { }
};

/* signal with complex arguments (a dict) */
static void emit_properties(Eldbus_Service_Interface *iface)
{
   Eldbus_Message *alive2;
   Eldbus_Message_Iter *iter, *dict;
   struct keyval {
      const char *key;
      const char *val;
   } keyval[] = {
      { "key1", "val1" },
      { "key2", "val2" },
      { }
   };
   struct keyval *k;

   alive2 = eldbus_service_signal_new(iface, TEST_SIGNAL_PROP);
   iter = eldbus_message_iter_get(alive2);
   dict = eldbus_message_iter_container_new(iter, 'a', "{ss}");

   for (k = keyval; k && k->key; k++)
     {
        Eldbus_Message_Iter *entry = eldbus_message_iter_container_new(dict, 'e',
                                                                     NULL);
        eldbus_message_iter_arguments_append(entry, "ss", k->key, k->val);
        eldbus_message_iter_container_close(dict, entry);
     }

   eldbus_message_iter_container_close(iter, dict);
   eldbus_service_signal_send(iface, alive2);
}

/* signal with basic args */
static void emit_name(Eldbus_Service_Interface *iface)
{
   eldbus_service_signal_emit(iface, TEST_SIGNAL_NAME, "TEST");
}

/* simple signal example */
static void emit_alive(Eldbus_Service_Interface *iface)
{
   eldbus_service_signal_emit(iface, TEST_SIGNAL_ALIVE);
}

static const Eldbus_Service_Interface_Desc iface_desc = {
   "org.enlightenment.Test", NULL, test_signals
};

int main(void)
{
   Eldbus_Connection *conn;
   Eldbus_Service_Interface *iface;

   eldbus_init();

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   iface = eldbus_service_interface_register(conn, "/org/enlightenment",
                                            &iface_desc);

   emit_alive(iface);
   emit_name(iface);
   emit_properties(iface);

   eldbus_connection_unref(conn);

   eldbus_shutdown();

   return 0;
}

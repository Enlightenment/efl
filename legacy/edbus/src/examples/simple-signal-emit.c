#include <EDBus.h>

enum {
   TEST_SIGNAL_ALIVE,
   TEST_SIGNAL_PROP,
   TEST_SIGNAL_NAME,
};

static const EDBus_Signal test_signals[] = {
   [TEST_SIGNAL_ALIVE] = { "Alive" },
   [TEST_SIGNAL_PROP] = { "Properties", EDBUS_ARGS({ "a{ss}", "properties"}) },
   [TEST_SIGNAL_NAME] = { "Name", EDBUS_ARGS({ "s", "name"}) },
   { }
};

/* signal with complex arguments (a dict) */
static void emit_properties(EDBus_Service_Interface *iface)
{
   EDBus_Message *alive2;
   EDBus_Message_Iter *iter, *dict;
   struct keyval {
      const char *key;
      const char *val;
   } keyval[] = {
      { "key1", "val1" },
      { "key2", "val2" },
      { }
   };
   struct keyval *k;

   alive2 = edbus_service_signal_new(iface, TEST_SIGNAL_PROP);
   iter = edbus_message_iter_get(alive2);
   dict = edbus_message_iter_container_new(iter, 'a', "{ss}");

   for (k = keyval; k && k->key; k++)
     {
        EDBus_Message_Iter *entry = edbus_message_iter_container_new(dict, 'e',
                                                                     NULL);
        edbus_message_iter_arguments_set(entry, "ss", k->key, k->val);
        edbus_message_iter_container_close(dict, entry);
     }

   edbus_message_iter_container_close(iter, dict);
   edbus_service_signal_send(iface, alive2);
}

/* signal with basic args */
static void emit_name(EDBus_Service_Interface *iface)
{
   edbus_service_signal_emit(iface, TEST_SIGNAL_NAME, "TEST");
}

/* simple signal example */
static void emit_alive(EDBus_Service_Interface *iface)
{
   edbus_service_signal_emit(iface, TEST_SIGNAL_ALIVE);
}

static const EDBus_Service_Interface_Desc iface_desc = {
   "org.enlightenment.Test", NULL, test_signals
};

int main(void)
{
   EDBus_Connection *conn;
   EDBus_Service_Interface *iface;

   edbus_init();

   conn = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);
   iface = edbus_service_interface_register(conn, "/org/enlightenment",
                                            &iface_desc);

   emit_alive(iface);
   emit_name(iface);
   emit_properties(iface);

   edbus_connection_unref(conn);

   edbus_shutdown();

   return 0;
}

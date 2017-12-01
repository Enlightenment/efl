#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Elementary.h>
#include <stdio.h>

static void _cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   Elm_Atspi_Bus_Watcher *watcher = data;
   printf("Screen reader status: %d\n", elm_atspi_bus_watcher_screen_reader_enabled_get(watcher));
   const char *addr = elm_atspi_bus_watcher_a11y_bus_address_get(watcher);
   printf("Bus address: %s\n", addr);
   eina_stringshare_del(addr);
}

static void _cb2(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   Elm_Atspi_Bus_Watcher *watcher = data;
   printf("Is enabled status: %d\n", elm_atspi_bus_watcher_is_enabled_get(watcher));
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Elm_Atspi_Bus_Watcher *watcher = efl_add(ELM_ATSPI_BUS_WATCHER_CLASS, NULL);
   Eldbus_Connection *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);

   elm_bus_watcher_connection_set(watcher, conn);
   efl_event_callback_add(watcher, ELM_ATSPI_BUS_WATCHER_EVENT_SCREEN_READER_STATUS_CHANGED, _cb, watcher);
   efl_event_callback_add(watcher, ELM_ATSPI_BUS_WATCHER_EVENT_ENABLED_STATUS_CHANGED, _cb2, watcher);
   eldbus_connection_unref(conn);

   elm_run();
   efl_unref(watcher);

   return 0;
}
ELM_MAIN()

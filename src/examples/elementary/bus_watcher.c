#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Elementary.h>
#include <stdio.h>

static void _cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Elm_Bus_Watcher_Service_Change_Info *info = event->info;
   printf("Registered %s by %s\n", info->service_name, info->new_id);
}

static void _cb2(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   Elm_Bus_Watcher_Service_Change_Info *info = event->info;
   printf("Unregistered %s by %s\n", info->service_name, info->old_id);
}

static void parse_args(Elm_Bus_Watcher *watcher, int argc, char **argv)
{
  int i;
  if (argc < 2)
    {
       fprintf(stderr, "Invalid usage:\n%s <dbus-service-name>\n", argv[0]);
       exit(1);
    }
  for (i = 1; i < argc; ++i)
  {
    elm_bus_watcher_service_add(watcher, argv[i]);
    elm_bus_watcher_service_add(watcher, argv[i]);
  }
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Elm_Bus_Watcher *watcher = efl_add(ELM_BUS_WATCHER_CLASS, NULL);
   Eldbus_Connection *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);

   parse_args(watcher, argc, argv);

   elm_bus_watcher_connection_set(watcher, conn);
   efl_event_callback_add(watcher, ELM_BUS_WATCHER_EVENT_REGISTERED, _cb, watcher);
   efl_event_callback_add(watcher, ELM_BUS_WATCHER_EVENT_UNREGISTERED, _cb2, watcher);
   eldbus_connection_unref(conn);

   elm_run();
   efl_unref(watcher);

   return 0;
}
ELM_MAIN()

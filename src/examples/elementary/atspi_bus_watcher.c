#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Elementary.h>
#include <stdio.h>

#define TEST_WRITE 1


static void _cb1(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   printf("A11y stack enabled\n");
}

static void _cb2(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   printf("A11y stack disabled\n");
}

static void _cb3(void *data EINA_UNUSED, const Efl_Event *event)
{
   Atspi_Bus_Property *property = event->info;
   switch (*property)
     {
      case ATSPI_BUS_PROPERTY_SCREENREADERENABLED:
         printf("ScreenReaderEnabled property changed\n");
         break;
      case ATSPI_BUS_PROPERTY_ISENABLED:
         printf("IsEnabled property changed\n");
         break;
     }
}

static Eina_Value
_success(void *data, Eina_Value value)
{
   printf("Getting address successed.\n");
   Eina_Value_Type *type = eina_value_type_get(&value);

   if (type == EINA_VALUE_TYPE_STRUCT)
     {
        char *address = NULL;
        if (!eina_value_struct_get(&value, "arg0", &address))
            printf("failed to get arg0 value.\n");
         printf("address is %s\n", address);
     }
   else {
         printf("invalid answer type\n");
     }
   return value;
}

static Eina_Value
_failed(void *data, Eina_Value value)
{
   printf("Getting address failed.\n");
   return value;
}

static void _registered(void *data, const Efl_Event *event EINA_UNUSED)
{
   Elm_Atspi_Bus_Watcher *watcher = data;

   elm_atspi_bus_watcher_property_try_set(watcher, ATSPI_BUS_PROPERTY_SCREENREADERENABLED, EINA_TRUE);
   Eina_Future *address_feature = elm_atspi_bus_watcher_a11y_bus_address_get(watcher);
   eina_future_cance(address_feature);
   //eina_future_then_easy(address_feature, _success, NULL, NULL, NULL, NULL);
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Elm_Atspi_Bus_Watcher *watcher = efl_add(ELM_ATSPI_BUS_WATCHER_CLASS, NULL);
   Eldbus_Connection *conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);

   elm_bus_watcher_connection_set(watcher, conn);
   efl_event_callback_add(watcher, ELM_ATSPI_BUS_WATCHER_EVENT_ENABLED, _cb1, watcher);
   efl_event_callback_add(watcher, ELM_ATSPI_BUS_WATCHER_EVENT_DISABLED, _cb2, watcher);
   efl_event_callback_add(watcher, ELM_ATSPI_BUS_WATCHER_EVENT_PROPERTY_CHANGED, _cb3, watcher);
   efl_event_callback_add(watcher, ELM_BUS_WATCHER_EVENT_REGISTERED, _registered, watcher);

   eldbus_connection_unref(conn);

   elm_run();
   efl_unref(watcher);

   return 0;
}
ELM_MAIN()

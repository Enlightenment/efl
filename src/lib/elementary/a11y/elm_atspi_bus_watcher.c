#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define ELM_BUS_WATCHER_PROTECTED
#define ELM_ATSPI_BUS_WATCHER_PROTECTED
#define A11Y_DBUS_NAME "org.a11y.Bus"
#define A11Y_DBUS_PATH "/org/a11y/bus"
#define A11Y_DBUS_INTERFACE "org.a11y.Bus"
#define A11Y_DBUS_STATUS_INTERFACE "org.a11y.Status"
#define SCREEN_READER_ENABLED_PROPERTY_NAME "ScreenReaderEnabled"
#define IS_ENABLED_PROPERTY_NAME "IsEnabled"
#define GET_ADDRESS_METHOD_NAME "GetAddress"

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_atspi_bus_watcher.eo.h"

typedef struct _Elm_Atspi_Bus_Watcher_Data
{
    Eldbus_Object *bus_object;
    Eldbus_Proxy *status_proxy;
    Eldbus_Proxy *bus_proxy;
    Eina_Bool is_enabled;
    Eina_Bool screen_reader_enabled;
} Elm_Atspi_Bus_Watcher_Data;

EOLIAN static Eo*
_elm_atspi_bus_watcher_efl_object_constructor(Eo *obj, Elm_Atspi_Bus_Watcher_Data *pd EINA_UNUSED)
{
    obj = efl_constructor(efl_super(obj, ELM_ATSPI_BUS_WATCHER_CLASS));
    elm_bus_watcher_service_add(obj, A11Y_DBUS_NAME);
    return obj;
}

static Eina_Bool
_fetch_bool_property(Eldbus_Proxy *proxy, const char *name, Eina_Bool *value)
{
    Eina_Value *val = eldbus_proxy_property_local_get(proxy, name);
    if (!val) return EINA_FALSE;
    return eina_value_get(val, value);
}

static void
_fetch_properties(Elm_Atspi_Bus_Watcher *watcher)
{
    Elm_Atspi_Bus_Watcher_Data *pd = efl_data_scope_get(watcher, ELM_ATSPI_BUS_WATCHER_CLASS);
    Eina_Bool val;

    if (_fetch_bool_property(pd->status_proxy, SCREEN_READER_ENABLED_PROPERTY_NAME, &val))
    {
        elm_atspi_bus_watcher_screen_reader_enabled_set(watcher, val);
    }

    if (_fetch_bool_property(pd->status_proxy, IS_ENABLED_PROPERTY_NAME, &val))
    {
        elm_atspi_bus_watcher_is_enabled_set(watcher, val);
    }
}

static void
_properties_changed_cb(void *data, Eldbus_Proxy *proxy EINA_UNUSED, void *event EINA_UNUSED)
{
   Eldbus_Proxy_Event_Property_Changed *ev = event;
   const char *ifc = eldbus_proxy_interface_get(ev->proxy);
   Elm_Atspi_Bus_Watcher *watcher = data;

   if (!ifc || strcmp(ifc, A11Y_DBUS_STATUS_INTERFACE)) return;

   _fetch_properties(watcher);
}

static void
_properties_loaded_cb(void *data, Eldbus_Proxy *proxy EINA_UNUSED, void *event)
{
   Eldbus_Proxy_Event_Property_Loaded *ev = event;
   Elm_Atspi_Bus_Watcher *watcher = data;
   const char *ifc = eldbus_proxy_interface_get(ev->proxy);

   if (!ifc || strcmp(ifc, A11Y_DBUS_STATUS_INTERFACE)) return;

   _fetch_properties(watcher);
}

EOLIAN static void
_elm_atspi_bus_watcher_elm_bus_watcher_on_registered(Eo *obj, Elm_Atspi_Bus_Watcher_Data *pd, const char *service_name, const char *oldid, const char *newid EINA_UNUSED)
{
    if (!service_name || strcmp(service_name, A11Y_DBUS_NAME))
        return;

    if (oldid && *oldid != '\0') // do not handle owner changed events
        return;

    pd->bus_object = eldbus_object_get(elm_bus_watcher_connection_get(obj), A11Y_DBUS_NAME, A11Y_DBUS_PATH);
    pd->status_proxy = eldbus_proxy_get(pd->bus_object, A11Y_DBUS_STATUS_INTERFACE);
    pd->bus_proxy = eldbus_proxy_get(pd->bus_object, A11Y_DBUS_INTERFACE);

    eldbus_proxy_properties_monitor(pd->status_proxy, EINA_TRUE);
    eldbus_proxy_event_callback_add(pd->status_proxy, ELDBUS_PROXY_EVENT_PROPERTY_CHANGED,
                                   _properties_changed_cb, obj);
    eldbus_proxy_event_callback_add(pd->status_proxy, ELDBUS_PROXY_EVENT_PROPERTY_LOADED,
                                   _properties_loaded_cb, obj);
}

EOLIAN static void
_elm_atspi_bus_watcher_elm_bus_watcher_on_unregistered(Eo *obj, Elm_Atspi_Bus_Watcher_Data *pd, const char *service_name, const char *oldid EINA_UNUSED)
{
    if (!service_name || strcmp(service_name, A11Y_DBUS_NAME))
        return;

   if (pd->status_proxy) eldbus_proxy_unref(pd->status_proxy);
   if (pd->bus_proxy) eldbus_proxy_unref(pd->bus_proxy);
   if (pd->bus_object) eldbus_object_unref(pd->bus_object);

   pd->status_proxy = pd->bus_proxy = NULL;
   pd->bus_object = NULL;

   elm_atspi_bus_watcher_screen_reader_enabled_set(obj, EINA_FALSE);
   elm_atspi_bus_watcher_is_enabled_set(obj, EINA_FALSE);
}

EOLIAN static const char*
_elm_atspi_bus_watcher_a11y_bus_address_get(Eo *obj EINA_UNUSED, Elm_Atspi_Bus_Watcher_Data *pd)
{
    const char *sock_addr = NULL, *errname, *errmsg;
    Eldbus_Message *msg = NULL, *reply = NULL;

    if (!pd->bus_proxy) return NULL;

    msg = eldbus_proxy_method_call_new(pd->bus_proxy, GET_ADDRESS_METHOD_NAME);
    if (!msg) return NULL;

    reply = eldbus_proxy_send_and_block(pd->bus_proxy, msg, -1);
    if (!reply)
    {
        ERR("Unable to make '" GET_ADDRESS_METHOD_NAME "' method call");
        eldbus_message_unref(msg);
        return NULL;
    }

   if (eldbus_message_error_get(reply, &errname, &errmsg))
   {
       ERR("GetAddress call failed: %s, %s", errname, errmsg);
       eldbus_message_unref(reply);
       return NULL;
   }

   if (!eldbus_message_arguments_get(reply, "s", &sock_addr))
   {
       ERR("Invalid GetAddress reply signature");
       eldbus_message_unref(reply);
       return NULL;
   }

    eldbus_message_unref(reply);

    return sock_addr ? eina_stringshare_add(sock_addr) : NULL;
}

static void
_elm_atspi_bus_watcher_is_enabled_set(Eo *obj, Elm_Atspi_Bus_Watcher_Data *pd, Eina_Bool val)
{
   if (pd->is_enabled == val) return;
   pd->is_enabled = val;
   efl_event_callback_call(obj, ELM_ATSPI_BUS_WATCHER_EVENT_ENABLED_STATUS_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_elm_atspi_bus_watcher_is_enabled_get(Eo *obj EINA_UNUSED, Elm_Atspi_Bus_Watcher_Data *pd)
{
   return pd->is_enabled;
}

EOLIAN static Eina_Bool
_elm_atspi_bus_watcher_screen_reader_enabled_get(Eo *obj EINA_UNUSED, Elm_Atspi_Bus_Watcher_Data *pd)
{
   return pd->screen_reader_enabled;
}

EOLIAN static void
_elm_atspi_bus_watcher_screen_reader_enabled_set(Eo *obj, Elm_Atspi_Bus_Watcher_Data *pd, Eina_Bool val)
{
   if (pd->screen_reader_enabled == val) return;
   pd->screen_reader_enabled = val;
   efl_event_callback_call(obj, ELM_ATSPI_BUS_WATCHER_EVENT_SCREEN_READER_STATUS_CHANGED, NULL);
}

#include "elm_atspi_bus_watcher.eo.c"
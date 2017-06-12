#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_atspi_status_monitor.eo.h"
#include <assert.h>

typedef struct {
   Eldbus_Connection *conn;
   const char        *a11y_bus_address;
   Eldbus_Object     *bus_obj;
   Eldbus_Proxy      *proxy;
   Eldbus_Pending    *pending;
   Eina_Bool         status : 1;
} Elm_Atspi_Status_Monitor_Data;

#define A11Y_BUS_NAME "org.a11y.Bus"
#define A11Y_BUS_PATH "/org/a11y/bus"
#define A11Y_BUS_INTERFACE "org.a11y.Bus"
#define A11Y_BUS_STATUS_INTERFACE "org.a11y.Status"
#define STATUS_PROPERTY_NAME "ScreenReaderEnabled"

#define ELM_ATSPI_STATUS_MONITOR_DATA_GET_OR_RETURN(obj, sd) \
   Elm_Atspi_Status_Monitor_Data *sd = efl_data_scope_get(obj, ELM_ATSPI_STATUS_MONITOR_CLASS); \
   if (!sd) return;

static void
_a11y_bus_address_get(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg, *sock_addr = NULL;
   Elm_Atspi_Status_Monitor *monitor = data;
   ELM_ATSPI_STATUS_MONITOR_DATA_GET_OR_RETURN(monitor, pd);

   pd->pending = NULL;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("%s %s", errname, errmsg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "s", &sock_addr) || !sock_addr)
     {
        ERR("Could not get A11Y Bus socket address.");
        return;
     }
   eina_stringshare_replace(&pd->a11y_bus_address, sock_addr);
   efl_event_callback_call(monitor, ELM_OBJ_ATSPI_STATUS_MONITOR_EVENT_STATUS_CHANGED, NULL);
}

static void
_elm_atspi_status_monitor_address_fetch(Elm_Atspi_Status_Monitor *obj, Elm_Atspi_Status_Monitor_Data *pd)
{
   if (pd->pending) eldbus_pending_cancel(pd->pending);

   Eldbus_Message *msg = eldbus_message_method_call_new(A11Y_BUS_NAME,
                                                        A11Y_BUS_PATH,
                                                        A11Y_BUS_INTERFACE,
                                                        "GetAddress");
   pd->pending = eldbus_connection_send(pd->conn, msg,
                                                _a11y_bus_address_get, obj, 5.0);
   if (!pd->pending)
     {
        ERR("eldbus_object_send failed");
        eldbus_message_unref(msg);
        return;
     }
}

// Gets status property from proxy cached values.
// aborts if reading value not poperly cached
static Eina_Bool
_elm_atspi_status_monitor_status_local_get(Eldbus_Proxy *proxy)
{
   Eina_Bool ret;
   Eina_Value *val = eldbus_proxy_property_local_get(proxy, STATUS_PROPERTY_NAME);

   assert (val != NULL);
   assert (eina_value_type_get(val) == EINA_VALUE_TYPE_UCHAR);
   assert (eina_value_get(val, &ret) == EINA_TRUE);

   return ret;
}

static void
_elm_atspi_status_monitor_status_update(Elm_Atspi_Status_Monitor *monitor,
                                        Elm_Atspi_Status_Monitor_Data *pd)
{
   Eina_Bool new_status = _elm_atspi_status_monitor_status_local_get(pd->proxy);

   if (pd->status == new_status) return;

   pd->status = new_status;

   if (pd->status)
     _elm_atspi_status_monitor_address_fetch(monitor, pd);
   else
     {
        eina_stringshare_replace(&pd->a11y_bus_address, NULL);
        efl_event_callback_call(monitor, ELM_OBJ_ATSPI_STATUS_MONITOR_EVENT_STATUS_CHANGED, NULL);
     }
}

static void
_elm_atspi_status_monitor_property_loaded(void *data, Eldbus_Proxy *proxy EINA_UNUSED, void *event EINA_UNUSED)
{
   Elm_Atspi_Status_Monitor *monitor = data;
   ELM_ATSPI_STATUS_MONITOR_DATA_GET_OR_RETURN(monitor, pd);

   _elm_atspi_status_monitor_status_update(monitor, pd);
}

static void
_elm_atspi_status_monitor_property_changed(void *data, Eldbus_Proxy *proxy EINA_UNUSED, void *event)
{
   Eldbus_Proxy_Event_Property_Changed *ev = event;
   Elm_Atspi_Status_Monitor *monitor = data;
   ELM_ATSPI_STATUS_MONITOR_DATA_GET_OR_RETURN(monitor, pd);

   if (!ev->name || strncmp(ev->name, STATUS_PROPERTY_NAME,
                            strlen(STATUS_PROPERTY_NAME)))
     return;

   _elm_atspi_status_monitor_status_update(monitor, pd);
}

EOLIAN static Elm_Atspi_Status_Monitor*
_elm_atspi_status_monitor_efl_object_constructor(Elm_Atspi_Status_Monitor *obj,
                                                Elm_Atspi_Status_Monitor_Data *pd)
{
   efl_constructor(efl_super(obj, ELM_ATSPI_STATUS_MONITOR_CLASS));

   pd->conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   assert (pd->conn != NULL);

   pd->bus_obj = eldbus_object_get(pd->conn, A11Y_BUS_NAME, A11Y_BUS_PATH);
   assert (pd->bus_obj != NULL);

   pd->proxy = eldbus_proxy_get(pd->bus_obj, A11Y_BUS_STATUS_INTERFACE);
   assert (pd->proxy != NULL);

   eldbus_proxy_event_callback_add(pd->proxy, ELDBUS_PROXY_EVENT_PROPERTY_LOADED,
                                   _elm_atspi_status_monitor_property_loaded, obj);
   eldbus_proxy_event_callback_add(pd->proxy, ELDBUS_PROXY_EVENT_PROPERTY_CHANGED,
                                   _elm_atspi_status_monitor_property_changed, obj);

   eldbus_proxy_properties_monitor(pd->proxy, EINA_TRUE);
   return obj;
}

EOLIAN static void
_elm_atspi_status_monitor_efl_object_destructor(Elm_Atspi_Status_Monitor *obj,
                                               Elm_Atspi_Status_Monitor_Data *pd)
{
   if (pd->pending) eldbus_pending_cancel(pd->pending);
   eldbus_proxy_unref(pd->proxy);
   eldbus_object_unref(pd->bus_obj);
   eldbus_connection_unref(pd->conn);

   if (pd->a11y_bus_address) eina_stringshare_del(pd->a11y_bus_address);
   efl_destructor(efl_super(obj, ELM_ATSPI_STATUS_MONITOR_CLASS));
}

EOLIAN static Eina_Bool
_elm_atspi_status_monitor_status_get(Elm_Atspi_Status_Monitor *monitor EINA_UNUSED,
                                    Elm_Atspi_Status_Monitor_Data *pd)
{
   if (eldbus_proxy_properties_monitor(pd->proxy, EINA_TRUE))
     return _elm_atspi_status_monitor_status_local_get(pd->proxy);
   else
     return EINA_FALSE;
}

EOLIAN static const char*
_elm_atspi_status_monitor_address_get(Elm_Atspi_Status_Monitor *monitor EINA_UNUSED,
                                        Elm_Atspi_Status_Monitor_Data *pd)
{
   return pd->a11y_bus_address;
}

#include "elm_atspi_status_monitor.eo.c"

#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif

#define ELM_BUS_WATCHER_PROTECTED
#define ELM_ATSPI_BUS_WATCHER_PROTECTED

// dbus interface description used by at-spi2 accesibility
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

    // cached properties values,
    Eina_Bool cached_properties[2];
} Elm_Atspi_Bus_Watcher_Data;

static void _cached_property_set(Eo *obj, Elm_Atspi_Bus_Watcher_Data *pd, Atspi_Bus_Property property, Eina_Bool val);

static inline void _is_enabled_try_set(Eo *obj, Elm_Atspi_Bus_Watcher_Data *pd, Eina_Bool val);
static inline void _screen_reader_enabled_try_set(Eo *obj, Elm_Atspi_Bus_Watcher_Data *pd, Eina_Bool val);

static Eina_Future* _eldbus_object_future_send(Eina_Future_Scheduler *scheduler, Eldbus_Object *proxy, Eldbus_Message *msg, int timeout);
static inline void _eldbus_proxy_boolean_property_set(Eldbus_Proxy *proxy, const char *property_name, Eina_Bool v);

EOLIAN static Eo*
_elm_atspi_bus_watcher_efl_object_constructor(Eo *obj, Elm_Atspi_Bus_Watcher_Data *pd EINA_UNUSED)
{
    obj = efl_constructor(efl_super(obj, ELM_ATSPI_BUS_WATCHER_CLASS));
    elm_bus_watcher_service_add(obj, A11Y_DBUS_NAME);
    return obj;
}

EOLIAN static void
_elm_atspi_bus_watcher_efl_object_destructor(Eo *obj, Elm_Atspi_Bus_Watcher_Data *pd EINA_UNUSED)
{
   if (pd->status_proxy) eldbus_proxy_unref(pd->status_proxy);
   if (pd->bus_object) eldbus_object_unref(pd->bus_object);
   efl_destructor(efl_super(obj, ELM_ATSPI_BUS_WATCHER_CLASS));
}

// Set cached value of property. Fires property changed and enabled events when neccessary.
static void
_cached_property_set(Eo *obj, Elm_Atspi_Bus_Watcher_Data *pd, Atspi_Bus_Property property, Eina_Bool val)
{
   if (pd->cached_properties[property] == val)
     return;

   pd->cached_properties[property] = val;
   efl_event_callback_call(obj, ELM_ATSPI_BUS_WATCHER_EVENT_PROPERTY_CHANGED, &property);
}

static inline void
_screen_reader_enabled_try_set(Eo *obj EINA_UNUSED, Elm_Atspi_Bus_Watcher_Data *pd, Eina_Bool val)
{
   _eldbus_proxy_boolean_property_set(pd->status_proxy, SCREEN_READER_ENABLED_PROPERTY_NAME, val);
}

static inline void
_is_enabled_try_set(Eo *obj EINA_UNUSED, Elm_Atspi_Bus_Watcher_Data *pd, Eina_Bool val)
{
   _eldbus_proxy_boolean_property_set(pd->status_proxy, IS_ENABLED_PROPERTY_NAME, val);
}

static Eina_Bool
_proxy_boolean_property_fetch(Eldbus_Proxy *proxy, const char *name, Eina_Bool *out_value)
{
    Eina_Value *val = eldbus_proxy_property_local_get(proxy, name);
    if (!val) return EINA_FALSE;
    if (eina_value_type_get(val) != EINA_VALUE_TYPE_UCHAR) return EINA_FALSE;
    return eina_value_get(val, out_value);
}

static void
_elm_atspi_bus_watcher_update_all_properties(Elm_Atspi_Bus_Watcher *watcher, Eldbus_Proxy *proxy)
{
    Elm_Atspi_Bus_Watcher_Data *pd = efl_data_scope_get(watcher, ELM_ATSPI_BUS_WATCHER_CLASS);
    Eina_Bool val;

    if (_proxy_boolean_property_fetch(proxy, SCREEN_READER_ENABLED_PROPERTY_NAME, &val))
      _cached_property_set(watcher, pd, ATSPI_BUS_PROPERTY_SCREENREADERENABLED, val);

    if (_proxy_boolean_property_fetch(proxy, IS_ENABLED_PROPERTY_NAME, &val))
      _cached_property_set(watcher, pd, ATSPI_BUS_PROPERTY_ISENABLED, val);
}

static void
_status_proxy_properties_changed_cb(void *data, Eldbus_Proxy *proxy, void *event EINA_UNUSED)
{
   Elm_Atspi_Bus_Watcher *watcher = data;
   _elm_atspi_bus_watcher_update_all_properties(watcher, proxy);
}

EOLIAN static void
_elm_atspi_bus_watcher_elm_bus_watcher_on_registered(Eo *obj, Elm_Atspi_Bus_Watcher_Data *pd,
                                                     const char *service_name)
{
   if (!service_name || strcmp(service_name, A11Y_DBUS_NAME))
     return;

   pd->bus_object = eldbus_object_get(elm_bus_watcher_connection_get(obj),
                                      A11Y_DBUS_NAME, A11Y_DBUS_PATH);
   pd->status_proxy = eldbus_proxy_get(pd->bus_object, A11Y_DBUS_STATUS_INTERFACE);

   eldbus_proxy_event_callback_add(pd->status_proxy, ELDBUS_PROXY_EVENT_PROPERTY_CHANGED,
                                   _status_proxy_properties_changed_cb, obj);
   eldbus_proxy_event_callback_add(pd->status_proxy, ELDBUS_PROXY_EVENT_PROPERTY_LOADED,
                                   _status_proxy_properties_changed_cb, obj);
   eldbus_proxy_properties_monitor(pd->status_proxy, EINA_TRUE);
}

/* Analysis of cohesion
 *
 * functional
 * sequential
 * communicational
 * procedural
 * temporal
 * logical
 * coincidential
 */
EOLIAN static void
_elm_atspi_bus_watcher_elm_bus_watcher_on_unregistered(Eo *obj, Elm_Atspi_Bus_Watcher_Data *pd,
                                                       const char *service_name)
{
   if (!service_name || strcmp(service_name, A11Y_DBUS_NAME))
     return;

   eldbus_proxy_unref(pd->status_proxy);
   eldbus_object_unref(pd->bus_object);
   pd->bus_object = NULL;
   pd->status_proxy = NULL;

   // reset status variables since we lost connection with org.a11y.Bus
   _cached_property_set(obj, pd, ATSPI_BUS_PROPERTY_ISENABLED, EINA_FALSE);
   _cached_property_set(obj, pd, ATSPI_BUS_PROPERTY_SCREENREADERENABLED, EINA_FALSE);
}

EOLIAN static Eina_Bool
_elm_atspi_bus_watcher_property_get(Eo *obj EINA_UNUSED,
                                    Elm_Atspi_Bus_Watcher_Data *pd, Atspi_Bus_Property property)
{
   return pd->cached_properties[property];
}

EOLIAN static void
_elm_atspi_bus_watcher_property_try_set(Eo *obj EINA_UNUSED,
                                        Elm_Atspi_Bus_Watcher_Data *pd,
                                        Atspi_Bus_Property property, Eina_Bool val)
{
   switch (property)
     {
        case ATSPI_BUS_PROPERTY_SCREENREADERENABLED:
           _screen_reader_enabled_try_set(obj, pd, val);
           break;
        case ATSPI_BUS_PROPERTY_ISENABLED:
           _is_enabled_try_set(obj, pd, val);
           break;
        default:
           ERR("Invalid property value.");
           break;
     }
}

static void
_dummy_cancel_cb(void *data EINA_UNUSED, const Eina_Promise *dead_ptr EINA_UNUSED)
{
}

static inline void
_eldbus_proxy_boolean_property_set(Eldbus_Proxy *proxy, const char *property_name,
                                   Eina_Bool val)
{
   eldbus_proxy_property_set(proxy, property_name, "b", (void*)val, NULL, NULL);
}

static void
_eldbus_object_call_cb(void *data, const Eldbus_Message *msg,
                      Eldbus_Pending *pending EINA_UNUSED)
{
   Eina_Promise *promise = data;
   Eina_Value *value = eldbus_message_to_eina_value(msg);
   eina_promise_resolve(promise, *value);
}

static Eina_Future*
_eldbus_object_future_send(Eina_Future_Scheduler *scheduler, Eldbus_Object *obj,
                           Eldbus_Message *msg, int timeout)
{
   Eina_Promise *p = eina_promise_new(scheduler, _dummy_cancel_cb, NULL);
   if (!p) goto on_err;

   if (!eldbus_object_send(obj, msg, _eldbus_object_call_cb, p, timeout))
     goto on_err;

   return eina_future_new(p);

on_err:
   eldbus_message_unref(msg);
   return eina_future_rejected(scheduler, -1);
}

EOLIAN static Eina_Future*
_elm_atspi_bus_watcher_a11y_bus_address_get(Eo *obj EINA_UNUSED, Elm_Atspi_Bus_Watcher_Data *pd)
{
   Eina_Future_Scheduler *scheduler = efl_loop_future_scheduler_get(ecore_main_loop_get());
   Eldbus_Message *msg = eldbus_object_method_call_new(pd->bus_object,
                                                       A11Y_DBUS_INTERFACE,
                                                       GET_ADDRESS_METHOD_NAME);
   if (!msg) return eina_future_rejected(scheduler, -1); //TODO add recent error code

   return _eldbus_object_future_send(scheduler, pd->bus_object, msg, 100);
}

#include "elm_atspi_bus_watcher.eo.c"

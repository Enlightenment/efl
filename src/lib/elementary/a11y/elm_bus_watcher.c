#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define ELM_BUS_WATCHER_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

#include "elm_bus_watcher.eo.h"
#include "elm_bus_watcher_private.x"

static Eina_Bool _bus_watcher_service_unregister_callback(const void *container, void *data, void *gdata);
static Eina_Bool _bus_watcher_service_register_callback(const void *container, void *data, void *gdata);

typedef struct _Elm_Bus_Watcher_Data
{
   Eina_Array *service_names;
   Eldbus_Connection *connection;
} Elm_Bus_Watcher_Data;

EOLIAN static Eo*
_elm_bus_watcher_efl_object_constructor(Eo *obj, Elm_Bus_Watcher_Data *pd)
{
  obj = efl_constructor(efl_super(obj, ELM_BUS_WATCHER_CLASS));
  pd->service_names = eina_array_new(2); // make step small since in most cases watchers will have 1-2 names.
  return obj;
}

EOLIAN static void
_elm_bus_watcher_efl_object_destructor(Eo *obj, Elm_Bus_Watcher_Data *pd)
{
  elm_bus_watcher_connection_set(obj, NULL);
  _eina_array_strings_clear(pd->service_names);
  eina_array_free(pd->service_names);

  efl_destructor(efl_super(obj, ELM_BUS_WATCHER_CLASS));
}

static void 
_elm_atspi_bus_watcher_name_owner_changed_callback(void *data, const char *bus, const char *old_id, const char *new_id)
{
   Elm_Bus_Watcher *watcher = data;
   Elm_Bus_Watcher_Data *pd = efl_data_scope_get(watcher, ELM_BUS_WATCHER_CLASS);
   Elm_Bus_Watcher_Service_Change_Info info = { .service_name = bus, .old_id = old_id, .new_id = new_id };

   if (!_eina_array_string_contains(pd->service_names, bus)) return;

   /**
   Dbus deamon in case when service gets unregisted sends NameChanged signal with "newid"
   parameter set to empty string "".
   */
   if (!new_id || !strcmp(new_id, ""))
   {
     elm_bus_watcher_on_unregistered(watcher, bus, old_id);
     efl_event_callback_call(watcher, ELM_BUS_WATCHER_EVENT_UNREGISTERED, &info);
   }
   else if (new_id)
   {
     elm_bus_watcher_on_registered(watcher, bus, old_id, new_id);
     efl_event_callback_call(watcher, ELM_BUS_WATCHER_EVENT_REGISTERED, &info);
   }
}

static void
_elm_bus_watcher_dbus_setup(Eo *obj, Eldbus_Connection *conn, const char *service_name)
{
   eldbus_name_owner_changed_callback_add(conn, service_name, _elm_atspi_bus_watcher_name_owner_changed_callback, obj, EINA_TRUE);
}

static void
_elm_bus_watcher_dbus_teardown(Eo *obj, Eldbus_Connection *conn, const char *service_name)
{
   eldbus_name_owner_changed_callback_del(conn, service_name, _elm_atspi_bus_watcher_name_owner_changed_callback, obj);
}

EOLIAN static void
_elm_bus_watcher_service_add(Eo *obj, Elm_Bus_Watcher_Data *pd, const char *service)
{
   if (!service || _eina_array_string_contains(pd->service_names, service))
      return;

   _eina_array_string_push(pd->service_names, service);
   if (pd->connection) _elm_bus_watcher_dbus_setup(obj, pd->connection, service);
}

EOLIAN static void
_elm_bus_watcher_service_remove(Eo *obj EINA_UNUSED, Elm_Bus_Watcher_Data *pd, const char *service)
{
   if (!_eina_array_string_contains(pd->service_names, service))
      return;

   _eina_array_string_remove(pd->service_names, service);
   if (pd->connection) _elm_bus_watcher_dbus_teardown(obj, pd->connection, service);
}

static Eina_Bool
_bus_watcher_service_register_callback(const void *container EINA_UNUSED, void *data, void *gdata)
{
  Elm_Bus_Watcher *watcher = gdata;
  Elm_Bus_Watcher_Data *pd = efl_data_scope_get(watcher, ELM_BUS_WATCHER_CLASS);
  const char *service_name = data;

  _elm_bus_watcher_dbus_setup(watcher, pd->connection, service_name);
  return EINA_TRUE;
}

static Eina_Bool
_bus_watcher_service_unregister_callback(const void *container EINA_UNUSED, void *data, void *gdata)
{
  Elm_Bus_Watcher *watcher = gdata;
  Elm_Bus_Watcher_Data *pd = efl_data_scope_get(watcher, ELM_BUS_WATCHER_CLASS);
  const char *service_name = data;

  _elm_bus_watcher_dbus_teardown(watcher, pd->connection, service_name);
  return EINA_TRUE;
}

EOLIAN static void
_elm_bus_watcher_connection_set(Eo *obj, Elm_Bus_Watcher_Data *pd, Eldbus_Connection *conn)
{
   if (pd->connection)
     {
        eina_array_foreach(pd->service_names, _bus_watcher_service_unregister_callback, obj);
        eldbus_connection_unref(pd->connection);
        pd->connection = NULL;
     }
   if (conn)
     {
        pd->connection = eldbus_connection_ref(conn);
        eina_array_foreach(pd->service_names, _bus_watcher_service_register_callback, obj);
     }
}

EOLIAN static Eldbus_Connection*
_elm_bus_watcher_connection_get(Eo *obj EINA_UNUSED, Elm_Bus_Watcher_Data *pd)
{
   return pd->connection;
}

EOLIAN static Eina_Iterator*
_elm_bus_watcher_services_get(Eo *obj EINA_UNUSED, Elm_Bus_Watcher_Data *pd)
{
  return eina_array_iterator_new(pd->service_names);
}

EOLIAN static void
_elm_bus_watcher_on_registered(Eo *obj EINA_UNUSED, Elm_Bus_Watcher_Data *pd EINA_UNUSED, const char *service EINA_UNUSED, const char *oldid EINA_UNUSED, const char *newid EINA_UNUSED)
{
}

EOLIAN static void
_elm_bus_watcher_on_unregistered(Eo *obj EINA_UNUSED, Elm_Bus_Watcher_Data *pd EINA_UNUSED, const char *service EINA_UNUSED, const char *oldid EINA_UNUSED)
{
}

#include "elm_bus_watcher.eo.c"
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"
#include "efl_net-connman.h"

typedef struct
{
   /* Eldbus_Proxy/Eldbus_Object keeps a list of pending calls, but
    * they are reference counted singletons and will only cancel
    * pending calls when everything is gone.  However we operate on
    * our private data, that may be gone before other refs. So
    * keep the pending list.
    */
   Eina_List *pending;
   Eina_List *signal_handlers;
   Eina_List *technologies;
   Eina_List *access_points;
   Eldbus_Service_Interface *agent; /* net.connman.Agent */
   Eldbus_Pending *agent_pending;
   struct {
      Eldbus_Message *msg;
   } agent_request_input;
   Efl_Net_Control_State state;
   Eina_Bool radios_offline;
   Eina_Bool operating; /* connman exists */
   Eina_Bool agent_enabled;
   struct {
      unsigned char radios_offline; /* 0xff = not requested */
      Eldbus_Pending *radios_offline_pending;
   } request;
} Efl_Net_Control_Manager_Data;

#define MY_CLASS EFL_NET_CONTROL_MANAGER_CLASS

static void _efl_net_control_manager_agent_enabled_set(Eo *o, Efl_Net_Control_Manager_Data *pd, Eina_Bool agent_enabled);

static Eo *
_efl_net_control_technology_find(const Efl_Net_Control_Manager_Data *pd, const char *path)
{
   const Eina_List *n;
   Eo *child;

   EINA_LIST_FOREACH(pd->technologies, n, child)
     {
        Eina_Stringshare *cp = efl_net_connman_technology_path_get(child);
        if (strcmp(path, cp) == 0)
          return child;
     }

   WRN("technology path='%s' not found!", path);
   return NULL;
}

static Eo *
_efl_net_control_access_point_find(const Efl_Net_Control_Manager_Data *pd, const char *path)
{
   const Eina_List *n;
   Eo *child;

   EINA_LIST_FOREACH(pd->access_points, n, child)
     {
        Eina_Stringshare *cp = efl_net_connman_access_point_path_get(child);
        if (strcmp(path, cp) == 0)
          return child;
     }

   DBG("access_point path='%s' not found!", path);
   return NULL;
}

static Eldbus_Message *
_efl_net_control_agent_release(const Eldbus_Service_Interface *service, const Eldbus_Message *msg)
{
   Eo *o = eldbus_service_object_data_get(service, "efl_net");
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);

   DBG("Agent %p is released %s", o, eldbus_message_path_get(msg));
   EINA_SAFETY_ON_NULL_GOTO(o, end);
   EINA_SAFETY_ON_NULL_GOTO(pd, end);

   pd->agent_enabled = EINA_FALSE;
   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_AGENT_RELEASED, NULL);

 end:
   return eldbus_message_method_return_new(msg);
}

static Eldbus_Message *
_efl_net_control_agent_cancel(const Eldbus_Service_Interface *service, const Eldbus_Message *msg)
{
   Eo *o = eldbus_service_object_data_get(service, "efl_net");
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eldbus_Message *reply;

   DBG("Agent %p request canceled %s", o, eldbus_message_path_get(msg));
   EINA_SAFETY_ON_NULL_GOTO(o, end);
   EINA_SAFETY_ON_NULL_GOTO(pd, end);

   if (!pd->agent_request_input.msg) goto end;

   reply = eldbus_message_error_new(pd->agent_request_input.msg,
                                    "net.connman.Error.Canceled",
                                    "Cancel requested.");
   eldbus_connection_send(efl_net_connman_connection_get(), reply, NULL, NULL, -1.0);
   eldbus_message_unref(pd->agent_request_input.msg);
   pd->agent_request_input.msg = NULL;

 end:
   return eldbus_message_method_return_new(msg);
}

static const char *
_efl_net_control_agent_entry_type_get(Eldbus_Message_Iter *dict)
{
   Eldbus_Message_Iter *array, *entry;

   if (!eldbus_message_iter_arguments_get(dict, "a{sv}", &array))
     {
        ERR("Expected dict, got %s", eldbus_message_iter_signature_get(dict));
        return NULL;
     }

   while (eldbus_message_iter_get_and_next(array, 'e', &entry))
     {
        Eldbus_Message_Iter *var;
        const char *key, *c;
        if (!eldbus_message_iter_arguments_get(entry, "sv", &key, &var))
          {
             ERR("Expected dict entry, got %s", eldbus_message_iter_signature_get(entry));
             continue;
          }

        if (!eldbus_message_iter_arguments_get(var, "s", &c))
          {
             ERR("Expected string, got %s", eldbus_message_iter_signature_get(var));
             continue;
          }

        if (strcmp(key, "Type") == 0)
          return c;
     }
   return NULL;
}

static Efl_Net_Control_Agent_Request_Input_Information *
_efl_net_control_agent_informational_get(const char *name, Eldbus_Message_Iter *dict)
{
   Eldbus_Message_Iter *array, *entry;
   Eina_Bool is_informational = EINA_FALSE;
   const char *value = NULL;

   if (!eldbus_message_iter_arguments_get(dict, "a{sv}", &array))
     {
        ERR("Expected dict, got %s", eldbus_message_iter_signature_get(dict));
        return NULL;
     }

   while (eldbus_message_iter_get_and_next(array, 'e', &entry))
     {
        Eldbus_Message_Iter *var;
        const char *key, *c;
        if (!eldbus_message_iter_arguments_get(entry, "sv", &key, &var))
          {
             ERR("Expected dict entry, got %s", eldbus_message_iter_signature_get(entry));
             continue;
          }

        if (!eldbus_message_iter_arguments_get(var, "s", &c))
          {
             ERR("Expected string, got %s", eldbus_message_iter_signature_get(var));
             continue;
          }

        if (strcmp(key, "Requirement") == 0)
          {
             if (strcmp(c, "informational") == 0)
               is_informational = EINA_TRUE;
          }
        else if (strcmp(key, "Value") == 0)
          value = c;
        else if (strcmp(key, "Alternates") == 0)
          { }
        else if (strcmp(key, "Type") == 0)
          { }
        else WRN("Unknown key '%s' for requested input entry %s", key, name);
     }

   if (is_informational && value)
     {
        Efl_Net_Control_Agent_Request_Input_Information *info;
        info = calloc(1, sizeof(*info));
        EINA_SAFETY_ON_NULL_RETURN_VAL(info, NULL);
        info->name = name;
        info->value = value;
        DBG("Field %s=%s is informational", name, value);
        return info;
     }

   DBG("Field %s=%s is not informational", name, value);
   return NULL;
}

static Eldbus_Message *
_efl_net_control_agent_request_input(const Eldbus_Service_Interface *service, const Eldbus_Message *msg)
{
   Eo *o = eldbus_service_object_data_get(service, "efl_net");
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Efl_Net_Control_Agent_Request_Input event = { };
   Efl_Net_Control_Agent_Request_Input_Information *info;
   Eldbus_Message_Iter *array, *entry;
   const char *path;

   DBG("Agent %p requested input %s", o, eldbus_message_path_get(msg));
   EINA_SAFETY_ON_NULL_GOTO(o, err);
   EINA_SAFETY_ON_NULL_GOTO(pd, err);

   if (pd->agent_request_input.msg)
     {
        Eldbus_Message *reply = eldbus_message_error_new(pd->agent_request_input.msg,
                                                         "net.connman.Error.OperationAborted",
                                                         "Received new input request, aborting previous.");
        eldbus_connection_send(efl_net_connman_connection_get(), reply, NULL, NULL, -1.0);
        eldbus_message_unref(pd->agent_request_input.msg);
        pd->agent_request_input.msg = NULL;
     }

   if (!eldbus_message_arguments_get(msg, "oa{sv}", &path, &array))
     goto err;

   event.access_point = _efl_net_control_access_point_find(pd, path);

   while (eldbus_message_iter_get_and_next(array, 'e', &entry))
     {
        Eldbus_Message_Iter *var;
        const char *name;
        if (!eldbus_message_iter_arguments_get(entry, "sv", &name, &var))
          goto err;

        if (strcmp(name, "Name") == 0)
          event.fields |= EFL_NET_CONTROL_AGENT_REQUEST_INPUT_FIELD_NAME;
        else if (strcmp(name, "SSID") == 0)
          event.fields |= EFL_NET_CONTROL_AGENT_REQUEST_INPUT_FIELD_SSID;
        else if (strcmp(name, "WPS") == 0)
          event.fields |= EFL_NET_CONTROL_AGENT_REQUEST_INPUT_FIELD_WPS;
        else if ((strcmp(name, "Identity") == 0) ||
                 (strcmp(name, "Username") == 0))
          event.fields |= EFL_NET_CONTROL_AGENT_REQUEST_INPUT_FIELD_USERNAME;
        else if ((strcmp(name, "Passphrase") == 0) ||
                 (strcmp(name, "Password") == 0))
          {
             const char *value = _efl_net_control_agent_entry_type_get(var);
             if ((!event.passphrase_type) && (value))
               event.passphrase_type = value;
             event.fields |= EFL_NET_CONTROL_AGENT_REQUEST_INPUT_FIELD_PASSPHRASE;
          }
        else
          {
             info = _efl_net_control_agent_informational_get(name, var);
             if (info)
               event.informational = eina_list_append(event.informational, info);
             else
               WRN("Unknown field name '%s'", name);
          }
     }

   pd->agent_request_input.msg = eldbus_message_ref((Eldbus_Message *)msg);
   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_AGENT_REQUEST_INPUT, &event);

   EINA_LIST_FREE(event.informational, info) free(info);

   return NULL; /* reply later */

 err:
   return eldbus_message_method_return_new(msg);
}

static Eldbus_Message *
_efl_net_control_agent_report_error(const Eldbus_Service_Interface *service, const Eldbus_Message *msg)
{
   Eo *o = eldbus_service_object_data_get(service, "efl_net");
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const char *path, *err_msg;
   Efl_Net_Control_Agent_Error event = { };

   if (!eldbus_message_arguments_get(msg, "os", &path, &err_msg))
     {
        ERR("Invalid net.connman.ReportError signature=%s", eldbus_message_signature_get(msg));
        goto end;
     }

   DBG("Agent %p error %s: %s", o, path, err_msg);

   EINA_SAFETY_ON_NULL_GOTO(o, end);
   EINA_SAFETY_ON_NULL_GOTO(pd, end);

   event.access_point = _efl_net_control_access_point_find(pd, path);
   event.message = err_msg;

   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_AGENT_ERROR, &event);

 end:
   return eldbus_message_method_return_new(msg);
}

static Eldbus_Message *
_efl_net_control_agent_request_browser(const Eldbus_Service_Interface *service, const Eldbus_Message *msg)
{
   Eo *o = eldbus_service_object_data_get(service, "efl_net");
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const char *path, *url;
   Efl_Net_Control_Agent_Browser_Url event = { };

   if (!eldbus_message_arguments_get(msg, "os", &path, &url))
     {
        ERR("Invalid net.connman.RequestBrowser signature=%s", eldbus_message_signature_get(msg));
        goto end;
     }

   DBG("Agent %p browser %s: %s", o, path, url);

   EINA_SAFETY_ON_NULL_GOTO(o, end);
   EINA_SAFETY_ON_NULL_GOTO(pd, end);

   event.access_point = _efl_net_control_access_point_find(pd, path);
   event.url = url;

   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_AGENT_BROWSER_URL, &event);

 end:
   return eldbus_message_method_return_new(msg);
}

static Eldbus_Message *
_efl_net_control_agent_report_peer_error(const Eldbus_Service_Interface *service, const Eldbus_Message *msg)
{
   Eo *o = eldbus_service_object_data_get(service, "efl_net");
   const char *path, *err_msg;

   if (!eldbus_message_arguments_get(msg, "os", &path, &err_msg))
     {
        ERR("Invalid net.connman.ReportPeerError signature=%s", eldbus_message_signature_get(msg));
        goto end;
     }

   DBG("Agent %p peer error %s: %s", o, path, err_msg);

 end:
   return eldbus_message_method_return_new(msg);
}


static Eldbus_Message *
_efl_net_control_agent_request_peer_authorization(const Eldbus_Service_Interface *service, const Eldbus_Message *msg)
{
   Eo *o = eldbus_service_object_data_get(service, "efl_net");
   DBG("Agent %p peer authorization required", o);
   return eldbus_message_method_return_new(msg);
}

static const Eldbus_Service_Interface_Desc _efl_net_control_agent_desc = {
  .interface = "net.connman.Agent",
  .methods = (const Eldbus_Method []){
     {
       .member = "Release",
       .cb = _efl_net_control_agent_release,
     },
     {
       .member = "ReportError",
       .in = ELDBUS_ARGS({"o", "service"}, {"s", "message"}),
       .cb = _efl_net_control_agent_report_error,
     },
     {
       .member = "RequestBrowser",
       .in = ELDBUS_ARGS({"o", "service"}, {"s", "url"}),
       .cb = _efl_net_control_agent_request_browser,
     },
     {
       .member = "RequestInput",
       .in = ELDBUS_ARGS({"o", "service"}, {"a{sv}", "fields"}),
       .cb = _efl_net_control_agent_request_input,
     },
     {
       .member = "Cancel",
       .cb = _efl_net_control_agent_cancel,
     },
     {
       .member = "ReportPeerError",
       .in = ELDBUS_ARGS({"o", "peer"}, {"s", "message"}),
       .cb = _efl_net_control_agent_report_peer_error,
     },
     {
       .member = "RequestPeerAuthorization",
       .in = ELDBUS_ARGS({"o", "peer"}, {"a{sv}", "fields"}),
       .cb = _efl_net_control_agent_request_peer_authorization,
     },
     { }
   },
};

static void
_efl_net_control_technology_added_internal(Eo *o, Efl_Net_Control_Manager_Data *pd, Eldbus_Message_Iter *itr)
{
   const char *path;
   Eldbus_Message_Iter *array;
   Eo *child;

   if (!eldbus_message_iter_arguments_get(itr, "oa{sv}", &path, &array))
     {
        ERR("could not get technology path or properties, signature=%s", eldbus_message_iter_signature_get(itr));
        return;
     }

   child = efl_net_connman_technology_new(o, path, array);
   if (!child)
     {
        ERR("could not create technology for path=%s", path);
        return;
     }

   pd->technologies = eina_list_append(pd->technologies, child);
   DBG("Technology Added Path=%s", path);
   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_TECHNOLOGY_ADD, child);
}

static void
_efl_net_control_technology_added(void *data, const Eldbus_Message *msg)
{
   Eo *o = data;
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eldbus_Message_Iter *itr = eldbus_message_iter_get(msg);
   _efl_net_control_technology_added_internal(o, pd, itr);
}

static void
_efl_net_control_technology_removed(void *data, const Eldbus_Message *msg)
{
   Eo *o = data;
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eo *child;
   const char *path;

   if (!eldbus_message_arguments_get(msg, "o", &path))
     {
        ERR("Could not get removed technology's path");
        return;
     }

   child = _efl_net_control_technology_find(pd, path);
   if (!child)
     {
        WRN("Technology path '%s' doesn't exist", path);
        return;
     }

   pd->technologies = eina_list_remove(pd->technologies, child);
   DBG("Technology Removed Path=%s", path);
   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_TECHNOLOGY_DEL, child);
   efl_del(child);
}

static void
_efl_net_control_access_point_added(Eo *o, Efl_Net_Control_Manager_Data *pd, const char *path, Eldbus_Message_Iter *properties, unsigned int priority)
{
   Eo *child;

   child = efl_net_connman_access_point_new(o, path, properties, priority);
   if (!child)
     {
        ERR("could not create access point for path=%s", path);
        return;
     }

   pd->access_points = eina_list_append(pd->access_points, child);
   DBG("Access Point Added Path=%s", path);
   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_ACCESS_POINT_ADD, child);
}

static void
_efl_net_control_access_point_updated(Eo *o, Efl_Net_Control_Manager_Data *pd, const char *path, Eldbus_Message_Iter *properties, unsigned int priority)
{
   Eo *child = _efl_net_control_access_point_find(pd, path);
   if (!child)
     {
        _efl_net_control_access_point_added(o, pd, path, properties, priority);
        return;
     }

   efl_net_connman_access_point_update(child, properties, priority);
}

static void
_efl_net_control_access_point_removed(Eo *o, Efl_Net_Control_Manager_Data *pd, const char *path)
{
   Eo *child = _efl_net_control_access_point_find(pd, path);
   if (!child)
     {
        WRN("Access Point path '%s' doesn't exist", path);
        return;
     }

   pd->access_points = eina_list_remove(pd->access_points, child);
   DBG("Access Point Removed Path=%s", path);
   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_ACCESS_POINT_DEL, child);
   efl_del(child);
}

static int
_efl_net_control_access_point_sort_cb(const void *d1, const void *d2)
{
   const Eo *a = d1;
   const Eo *b = d2;
   unsigned int pa = efl_net_control_access_point_priority_get(a);
   unsigned int pb = efl_net_control_access_point_priority_get(b);
   if (pa < pb) return -1;
   else if (pa > pb) return 1;
   return 0;
}

static void
_efl_net_control_services_changed(void *data, const Eldbus_Message *msg)
{
   Eo *o = data;
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eldbus_Message_Iter *changed, *invalidated, *sub;
   const char *path;
   unsigned int priority;

   if (!eldbus_message_arguments_get(msg, "a(oa{sv})ao", &changed, &invalidated))
     {
        ERR("could not get services changed, signature=%s", eldbus_message_signature_get(msg));
        return;
     }

   efl_ref(o); /* will trigger events, which call user which may delete us */

   priority = 0;
   while (eldbus_message_iter_get_and_next(changed, 'r', &sub))
     {
        Eldbus_Message_Iter *properties;
        if (!eldbus_message_iter_arguments_get(sub, "oa{sv}", &path, &properties))
          {
             ERR("could not get access point path or properties, signature=%s", eldbus_message_iter_signature_get(sub));
             continue;
          }
        _efl_net_control_access_point_updated(o, pd, path, properties, priority++);
     }

   while (eldbus_message_iter_get_and_next(invalidated, 'o', &path))
     _efl_net_control_access_point_removed(o, pd, path);

   pd->access_points = eina_list_sort(pd->access_points, 0, _efl_net_control_access_point_sort_cb);

   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_ACCESS_POINTS_CHANGED, NULL);
   efl_unref(o);
}

static Efl_Net_Control_State
_efl_net_control_state_from_str(const char *str)
{
   const struct {
      const char *name;
      Efl_Net_Control_State state;
   } *itr, map[] = {
     {"offline", EFL_NET_CONTROL_STATE_OFFLINE},
     {"idle", EFL_NET_CONTROL_STATE_OFFLINE},
     {"ready", EFL_NET_CONTROL_STATE_LOCAL},
     {"online", EFL_NET_CONTROL_STATE_ONLINE},
     { }
   };
   for (itr = map; itr->name != NULL; itr++)
     if (strcmp(itr->name, str) == 0) return itr->state;

   ERR("Unknown State '%s', assume offline.", str);
   return EFL_NET_CONTROL_STATE_OFFLINE;
}

static void
_efl_net_control_property_state_changed(Eo *o, Efl_Net_Control_Manager_Data *pd, Eldbus_Message_Iter *value)
{
   const char *str;
   Efl_Net_Control_State state;

   if (!eldbus_message_iter_arguments_get(value, "s", &str))
     {
        ERR("Expected string, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   state = _efl_net_control_state_from_str(str);
   if (pd->state == state) return;
   pd->state = state;
   DBG("state=%d (%s)", state, str);
   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_STATE_CHANGED, NULL);
}

static void
_efl_net_control_property_radios_offline_changed(Eo *o, Efl_Net_Control_Manager_Data *pd, Eldbus_Message_Iter *value)
{
   Eina_Bool offline;

   if (!eldbus_message_iter_arguments_get(value, "b", &offline))
     {
        ERR("Expected boolean, got %s", eldbus_message_iter_signature_get(value));
        return;
     }

   if (pd->radios_offline == offline) return;
   pd->radios_offline = offline;
   DBG("radios offline=%hhu", offline);
   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_RADIOS_OFFLINE_CHANGED, NULL);
}

static void
_efl_net_control_property_changed_internal(Eo *o, Efl_Net_Control_Manager_Data *pd, Eldbus_Message_Iter *itr)
{
   Eldbus_Message_Iter *value;
   const char *name;

   if (!eldbus_message_iter_arguments_get(itr, "sv", &name, &value))
     {
        ERR("Unexpected signature: %s", eldbus_message_iter_signature_get(itr));
        return;
     }

   if (strcmp(name, "State") == 0)
     _efl_net_control_property_state_changed(o, pd, value);
   else if (strcmp(name, "OfflineMode") == 0)
     _efl_net_control_property_radios_offline_changed(o, pd, value);
   else if (strcmp(name, "SessionMode") == 0)
     { }
   else
     WRN("Unknown property name: %s", name);
}

static void
_efl_net_control_property_changed(void *data, const Eldbus_Message *msg)
{
   Eo *o = data;
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eldbus_Message_Iter *itr;

   itr = eldbus_message_iter_get(msg);
   _efl_net_control_property_changed_internal(o, pd, itr);
}

static void
_efl_net_control_properties_get_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eo *o = data;
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eldbus_Message_Iter *array, *entry;
   const char *err_name, *err_msg;

   pd->pending = eina_list_remove(pd->pending, pending);
   if (eldbus_message_error_get(msg, &err_name, &err_msg))
     {
        ERR("Could not get properties %p: %s=%s", o, err_name, err_msg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "a{sv}", &array))
     {
        ERR("Expected dict ('a{sv}'), got '%s'", eldbus_message_signature_get(msg));
        return;
     }

   efl_ref(o); /* properties will trigger events, which call user which may delete us */

   /* force changed to be emitted */
   pd->radios_offline = 0xff;
   pd->state = 0xff;
   while (eldbus_message_iter_get_and_next(array, 'e', &entry))
     _efl_net_control_property_changed_internal(o, pd, entry);

   efl_unref(o);
}

static void
_efl_net_control_technologies_get_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eo *o = data;
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eldbus_Message_Iter *array, *entry;
   const char *err_name, *err_msg;

   pd->pending = eina_list_remove(pd->pending, pending);
   if (eldbus_message_error_get(msg, &err_name, &err_msg))
     {
        ERR("Could not get technologies %p: %s=%s", o, err_name, err_msg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "a(oa{sv})", &array))
     {
        ERR("Expected 'a{oa{sv}}', got '%s'", eldbus_message_signature_get(msg));
        return;
     }

   efl_ref(o); /* will trigger events, which call user which may delete us */

   while (eldbus_message_iter_get_and_next(array, 'r', &entry))
     _efl_net_control_technology_added_internal(o, pd, entry);

   efl_unref(o);
}

static void
_efl_net_control_services_get_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eo *o = data;
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eldbus_Message_Iter *array, *sub;
   const char *err_name, *err_msg, *path;
   unsigned int priority = 0;

   pd->pending = eina_list_remove(pd->pending, pending);
   if (eldbus_message_error_get(msg, &err_name, &err_msg))
     {
        ERR("Could not get services %p: %s=%s", o, err_name, err_msg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "a(oa{sv})", &array))
     {
        ERR("Expected 'a{oa{sv}}', got '%s'", eldbus_message_signature_get(msg));
        return;
     }

   efl_ref(o); /* will trigger events, which call user which may delete us */

   while (eldbus_message_iter_get_and_next(array, 'r', &sub))
     {
        Eldbus_Message_Iter *properties;
        if (!eldbus_message_iter_arguments_get(sub, "oa{sv}", &path, &properties))
          {
             ERR("could not get access point path or properties, signature=%s", eldbus_message_iter_signature_get(sub));
             continue;
          }
        _efl_net_control_access_point_updated(o, pd, path, properties, priority++);
     }

   pd->access_points = eina_list_sort(pd->access_points, 0, _efl_net_control_access_point_sort_cb);

   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_ACCESS_POINTS_CHANGED, NULL);
   efl_unref(o);
}

static void
_efl_net_control_radios_offline_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eo *o = data;
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const char *err_name, *err_msg;
   Eina_Bool radios_offline = pd->request.radios_offline;

   if (pd->request.radios_offline_pending == pending)
     {
        pd->request.radios_offline_pending = NULL;
        pd->request.radios_offline = 0xff;
     }

   pd->pending = eina_list_remove(pd->pending, pending);
   if (eldbus_message_error_get(msg, &err_name, &err_msg))
     {
        ERR("Could not set property radios_offline=%hhu %p: %s=%s", radios_offline, o, err_name, err_msg);
        return;
     }

   DBG("Set radios_offline=%hhu", radios_offline);
}

static void
_efl_net_control_radios_offline_apply(Eo *o, Efl_Net_Control_Manager_Data *pd)
{
   Eldbus_Proxy *mgr = efl_net_connman_manager_get();
   Eldbus_Message *msg = eldbus_proxy_method_call_new(mgr, "SetProperty");
   Eldbus_Message_Iter *msg_itr, *var;
   Eldbus_Pending *p;

   if (pd->request.radios_offline_pending)
     {
        pd->pending = eina_list_remove(pd->pending, pd->request.radios_offline_pending);
        eldbus_pending_cancel(pd->request.radios_offline_pending);
        pd->request.radios_offline_pending = NULL;
     }

   EINA_SAFETY_ON_NULL_RETURN(msg);

   msg_itr = eldbus_message_iter_get(msg);
   EINA_SAFETY_ON_NULL_GOTO(msg_itr, error_send);

   eldbus_message_iter_basic_append(msg_itr, 's', "OfflineMode");
   var = eldbus_message_iter_container_new(msg_itr, 'v', "b");
   eldbus_message_iter_basic_append(var, 'b', pd->request.radios_offline);
   eldbus_message_iter_container_close(msg_itr, var);

   p = eldbus_proxy_send(mgr, msg, _efl_net_control_radios_offline_cb, o, DEFAULT_TIMEOUT);
   EINA_SAFETY_ON_NULL_GOTO(p, error_send);

   pd->pending = eina_list_append(pd->pending, p);
   pd->request.radios_offline_pending = p;
   DBG("setting radios offline=%hhu", pd->request.radios_offline);
   return;

 error_send:
   eldbus_message_unref(msg);
}

static void
_efl_net_control_clear(Eo *o, Efl_Net_Control_Manager_Data *pd)
{
   Eo *child;

   EINA_LIST_FREE(pd->access_points, child)
     {
        efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_ACCESS_POINT_DEL, child);
        efl_del(child);
     }
   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_ACCESS_POINTS_CHANGED, NULL);

   EINA_LIST_FREE(pd->technologies, child)
     {
        efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_TECHNOLOGY_DEL, child);
        efl_del(child);
     }

   pd->state = EFL_NET_CONTROL_STATE_OFFLINE;
   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_STATE_CHANGED, NULL);
   pd->radios_offline = EINA_TRUE;
   efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_RADIOS_OFFLINE_CHANGED, NULL);
}

static void
_efl_net_control_connman_name_owner_changed(void *data, const char *bus, const char *old_id, const char *new_id)
{
   Eo *o = data;
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eldbus_Proxy *mgr;

   DBG("Name Owner Changed %s: %s->%s", bus, old_id, new_id);
   if ((!new_id) || (new_id[0] == '\0'))
     {
        _efl_net_control_clear(o, pd);
        pd->operating = EINA_FALSE;
        if (pd->agent_enabled)
          {
             pd->agent_enabled = EINA_FALSE;
             efl_event_callback_call(o, EFL_NET_CONTROL_MANAGER_EVENT_AGENT_RELEASED, NULL);
          }
        return;
     }

   mgr = efl_net_connman_manager_get();
   EINA_SAFETY_ON_NULL_RETURN(mgr);

#define SH(sig, cb) \
   do { \
     Eldbus_Signal_Handler *sh = eldbus_proxy_signal_handler_add(mgr, sig, cb, o); \
     if (sh) pd->signal_handlers = eina_list_append(pd->signal_handlers, sh); \
     else ERR("could not add DBus signal handler %s", sig); \
   } while (0)

   SH("TechnologyAdded", _efl_net_control_technology_added);
   SH("TechnologyRemoved", _efl_net_control_technology_removed);
   SH("ServicesChanged", _efl_net_control_services_changed);
   SH("PropertyChanged", _efl_net_control_property_changed);
#undef SH

#define CALL(meth, cb) \
   do { \
     Eldbus_Pending *p = eldbus_proxy_call(mgr, meth, cb, o, DEFAULT_TIMEOUT, ""); \
     if (p) pd->pending = eina_list_append(pd->pending, p); \
     else ERR("could not call DBus method %s", meth); \
   } while (0)

   CALL("GetProperties", _efl_net_control_properties_get_cb);
   CALL("GetTechnologies", _efl_net_control_technologies_get_cb);
   CALL("GetServices", _efl_net_control_services_get_cb);
#undef CALL

   pd->operating = EINA_TRUE;

   if (pd->request.radios_offline != 0xff) _efl_net_control_radios_offline_apply(o, pd);

   if (pd->agent_pending)
     {
        eldbus_pending_cancel(pd->agent_pending);
        pd->agent_pending = NULL;
     }
   if (pd->agent_enabled)
     {
        pd->agent_enabled = EINA_FALSE;
        _efl_net_control_manager_agent_enabled_set(o, pd, EINA_TRUE);
     }
}

void
efl_net_connman_control_access_points_reload(Eo *o)
{
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eldbus_Proxy *mgr;
   Eldbus_Pending *p;

   EINA_SAFETY_ON_NULL_RETURN(pd);
   mgr = efl_net_connman_manager_get();
   EINA_SAFETY_ON_NULL_RETURN(mgr);

   p = eldbus_proxy_call(mgr, "GetServices", _efl_net_control_services_get_cb, o, DEFAULT_TIMEOUT, "");
   if (p) pd->pending = eina_list_append(pd->pending, p);
   else ERR("could not call DBus method %s", "GetServices");
}

EOLIAN static Eo *
_efl_net_control_manager_efl_object_constructor(Eo *o, Efl_Net_Control_Manager_Data *pd EINA_UNUSED)
{
   pd->radios_offline = EINA_TRUE;
   pd->request.radios_offline = 0xff;

   if (!efl_net_connman_init())
     {
        ERR("could not initialize connman infrastructure");
        return NULL;
     }

   return efl_constructor(efl_super(o, MY_CLASS));
}

EOLIAN static Eo *
_efl_net_control_manager_efl_object_finalize(Eo *o, Efl_Net_Control_Manager_Data *pd EINA_UNUSED)
{
   Eldbus_Connection *conn;
   char path[128];

   o = efl_finalize(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   conn = efl_net_connman_connection_get();
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);

   snprintf(path, sizeof(path), "/connman/agent_%p", o);
   pd->agent = eldbus_service_interface_register(conn, path, &_efl_net_control_agent_desc);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->agent, NULL);
   eldbus_service_object_data_set(pd->agent, "efl_net", o);

   eldbus_name_owner_changed_callback_add(conn, "net.connman", _efl_net_control_connman_name_owner_changed, o, EINA_TRUE);
   DBG("waiting for net.connman to show on the DBus system bus...");

   return o;
}

EOLIAN static void
_efl_net_control_manager_efl_object_destructor(Eo *o, Efl_Net_Control_Manager_Data *pd)
{
   Eldbus_Pending *p;
   Eldbus_Signal_Handler *sh;

   pd->operating = EINA_FALSE;

   pd->request.radios_offline_pending = NULL;
   EINA_LIST_FREE(pd->pending, p)
     eldbus_pending_cancel(p);

   EINA_LIST_FREE(pd->signal_handlers, sh)
     eldbus_signal_handler_del(sh);

   if (pd->agent_request_input.msg)
     {
        Eldbus_Message *reply = eldbus_message_error_new(pd->agent_request_input.msg,
                                                         "net.connman.Error.OperationAborted",
                                                         "Agent is gone.");
        eldbus_connection_send(efl_net_connman_connection_get(), reply, NULL, NULL, -1.0);
        eldbus_message_unref(pd->agent_request_input.msg);
        pd->agent_request_input.msg = NULL;
     }

   if (pd->agent_enabled)
     {
        Eldbus_Proxy *mgr = efl_net_connman_manager_get();

        /* UnregisterAgent is required since the manager proxy and bus
         * may be alive, thus connman won't get any NameOwnerChanged
         * or related to know the object is gone
         */
        eldbus_proxy_call(mgr, "UnregisterAgent", NULL, NULL, DEFAULT_TIMEOUT,
                          "o", eldbus_service_object_path_get(pd->agent));
     }

   if (pd->agent)
     {
        eldbus_service_object_data_del(pd->agent, "efl_net");
        eldbus_service_object_unregister(pd->agent);
        pd->agent = NULL;
     }

   _efl_net_control_clear(o, pd);

   efl_destructor(efl_super(o, MY_CLASS));
   efl_net_connman_shutdown();
}

EOLIAN static void
_efl_net_control_manager_radios_offline_set(Eo *o, Efl_Net_Control_Manager_Data *pd, Eina_Bool radios_offline)
{
   pd->request.radios_offline = radios_offline;
   if (pd->operating) _efl_net_control_radios_offline_apply(o, pd);
}

EOLIAN static Eina_Bool
_efl_net_control_manager_radios_offline_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Manager_Data *pd)
{
   return pd->radios_offline;
}

EOLIAN static Efl_Net_Control_State
_efl_net_control_manager_state_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Manager_Data *pd)
{
   return pd->state;
}

EOLIAN static Eina_Iterator *
_efl_net_control_manager_access_points_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Manager_Data *pd)
{
   return eina_list_iterator_new(pd->access_points);
}

EOLIAN static Eina_Iterator *
_efl_net_control_manager_technologies_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Manager_Data *pd)
{
   return eina_list_iterator_new(pd->technologies);
}

Efl_Net_Control_Technology *
efl_net_connman_control_find_technology_by_type(Efl_Net_Control_Manager *o, const char *tech_type)
{
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Efl_Net_Control_Technology_Type desired = efl_net_connman_technology_type_from_str(tech_type);
   const Eina_List *n;
   Eo *child;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, NULL);

   EINA_LIST_FOREACH(pd->technologies, n, child)
     {
        Efl_Net_Control_Technology_Type type = efl_net_control_technology_type_get(child);
        if (type == desired)
          return child;
     }

   WRN("Could not find technology type %#x '%s'", desired, tech_type);
   return NULL;
}

static void
_efl_net_control_agent_register_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eo *o = data;
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const char *err_name, *err_msg;

   pd->pending = eina_list_remove(pd->pending, pending);
   if (pd->agent_pending == pending)
     pd->agent_pending = NULL;
   if (eldbus_message_error_get(msg, &err_name, &err_msg))
     {
        ERR("Could not Register Agent %p: %s=%s", o, err_name, err_msg);
        return;
     }

   DBG("Agent registered %p: %s", o, eldbus_service_object_path_get(pd->agent));
   pd->agent_enabled = EINA_TRUE;
}

static void
_efl_net_control_agent_unregister_cb(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   Eo *o = data;
   Efl_Net_Control_Manager_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const char *err_name, *err_msg;

   pd->pending = eina_list_remove(pd->pending, pending);
   if (pd->agent_pending == pending)
     pd->agent_pending = NULL;
   if (eldbus_message_error_get(msg, &err_name, &err_msg))
     {
        ERR("Could not Unregister Agent %p: %s=%s", o, err_name, err_msg);
        return;
     }
   DBG("Agent unregistered %p: %s", o, eldbus_service_object_path_get(pd->agent));
   pd->agent_enabled = EINA_FALSE;
}

EOLIAN static void
_efl_net_control_manager_agent_enabled_set(Eo *o, Efl_Net_Control_Manager_Data *pd, Eina_Bool agent_enabled)
{
   Eldbus_Pending *p;
   Eldbus_Proxy *mgr;

   if (!pd->operating)
     {
        DBG("no connman yet, postpone agent_enabled=%hhu", agent_enabled);
        pd->agent_enabled = agent_enabled;
        return;
     }
   if (pd->agent_enabled == agent_enabled)
     {
        DBG("Already agent_enabled=%hhu", agent_enabled);
        return;
     }

   if (pd->agent_pending)
     {
        eldbus_pending_cancel(pd->agent_pending);
        pd->agent_pending = NULL;
     }

   mgr = efl_net_connman_manager_get();
   EINA_SAFETY_ON_NULL_RETURN(mgr);

   if (agent_enabled)
     {
        p = eldbus_proxy_call(mgr, "RegisterAgent",
                              _efl_net_control_agent_register_cb, o, DEFAULT_TIMEOUT,
                              "o", eldbus_service_object_path_get(pd->agent));
        DBG("%p RegisterAgent=%p", o, p);
     }
   else
     {
        p = eldbus_proxy_call(mgr, "UnregisterAgent",
                              _efl_net_control_agent_unregister_cb, o, DEFAULT_TIMEOUT,
                              "o", eldbus_service_object_path_get(pd->agent));
        DBG("%p UnregisterAgent=%p", o, p);
     }

   EINA_SAFETY_ON_NULL_RETURN(p);

   pd->pending = eina_list_append(pd->pending, p);
   pd->agent_pending = p;
}

EOLIAN static Eina_Bool
_efl_net_control_manager_agent_enabled_get(const Eo *o EINA_UNUSED, Efl_Net_Control_Manager_Data *pd)
{
   return pd->agent_enabled;
}

static void
_append_dict_entry(Eldbus_Message_Iter *array, const char *name, const char *signature, ...)
{
   Eldbus_Message_Iter *entry, *var;
   va_list ap;

   if (!eldbus_message_iter_arguments_append(array, "{sv}", &entry))
     {
        ERR("could not append dict entry");
        return;
     }

   eldbus_message_iter_basic_append(entry, 's', name);
   var = eldbus_message_iter_container_new(entry, 'v', signature);

   va_start(ap, signature);
   eldbus_message_iter_arguments_vappend(var, signature, ap);
   va_end(ap);
   eldbus_message_iter_container_close(entry, var);
   eldbus_message_iter_container_close(array, entry);
}


static void
_append_dict_entry_byte_array(Eldbus_Message_Iter *array, const char *name, const Eina_Slice *slice)
{
   Eldbus_Message_Iter *entry, *var, *sub;

   if (!eldbus_message_iter_arguments_append(array, "{sv}", &entry))
     {
        ERR("could not append dict entry");
        return;
     }

   eldbus_message_iter_basic_append(entry, 's', name);
   var = eldbus_message_iter_container_new(entry, 'v', "ay");
   sub = eldbus_message_iter_container_new(var, 'a', "y");

   eldbus_message_iter_fixed_array_append(sub, 'y', slice->mem, slice->len);

   eldbus_message_iter_container_close(var, sub);
   eldbus_message_iter_container_close(entry, var);
   eldbus_message_iter_container_close(array, entry);
}

EOLIAN static void
_efl_net_control_manager_agent_reply(Eo *o EINA_UNUSED, Efl_Net_Control_Manager_Data *pd, const char *name, const Eina_Slice *ssid, const char *username, const char *passphrase, const char *wps)
{
   Eldbus_Message *reply;
   Eldbus_Message_Iter *msg_itr, *array;

   if (!pd->agent_request_input.msg)
     {
        ERR("no agent input was required.");
        return;
     }

   reply = eldbus_message_method_return_new(pd->agent_request_input.msg);
   EINA_SAFETY_ON_NULL_RETURN(reply);
   eldbus_message_unref(pd->agent_request_input.msg);
   pd->agent_request_input.msg = NULL;

   msg_itr = eldbus_message_iter_get(reply);
   eldbus_message_iter_arguments_append(msg_itr, "a{sv}", &array);

   if (name) _append_dict_entry(array, "Name", "s", name);
   if (ssid) _append_dict_entry_byte_array(array, "SSID", ssid);
   if (username)
     {
        _append_dict_entry(array, "Username", "s", username);
        _append_dict_entry(array, "Identity", "s", username);
     }
   if (passphrase)
     {
        _append_dict_entry(array, "Password", "s", passphrase);
        _append_dict_entry(array, "Passphrase", "s", passphrase);
     }
   if (wps) _append_dict_entry(array, "WPS", "s", wps);

   eldbus_message_iter_container_close(msg_itr, array);
   eldbus_connection_send(efl_net_connman_connection_get(), reply, NULL, NULL, -1.0);
}

#include "efl_net_control_manager.eo.c"

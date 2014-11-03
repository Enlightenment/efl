#include "eldbus_geo_clue2_client.h"

static int _log_main = -1;
#undef ERR
#define ERR(...) EINA_LOG_DOM_ERR(_log_main, __VA_ARGS__);
int GEO_CLUE2_CLIENT_LOCATION_UPDATED_EVENT = 0;

static void
cb_geo_clue2_client_start(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   Geo_Clue2_Client_Start_Cb cb = data;
   const char *error, *error_msg;
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        Eldbus_Error_Info error_info = {error, error_msg};
        cb(proxy, user_data, pending, &error_info);
        return;
     }
   if (!eldbus_message_arguments_get(msg, ""))
     {
        Eldbus_Error_Info error_info = {"", ""};
        ERR("Error: Getting arguments from message.");
        cb(proxy, user_data, pending, &error_info);
        return;
     }
   cb(proxy, user_data, pending, NULL);
   return;
}

Eldbus_Pending *
geo_clue2_client_start_call(Eldbus_Proxy *proxy, Geo_Clue2_Client_Start_Cb cb, const void *data)
{
   Eldbus_Message *msg;
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   msg = eldbus_proxy_method_call_new(proxy, "Start");
   if (!eldbus_message_arguments_append(msg, ""))
     {
        ERR("Error: Filling message.");
        return NULL;
     }
   p = eldbus_proxy_send(proxy, msg, cb_geo_clue2_client_start, cb, -1);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_client_stop(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   Geo_Clue2_Client_Stop_Cb cb = data;
   const char *error, *error_msg;
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        Eldbus_Error_Info error_info = {error, error_msg};
        cb(proxy, user_data, pending, &error_info);
        return;
     }
   if (!eldbus_message_arguments_get(msg, ""))
     {
        Eldbus_Error_Info error_info = {"", ""};
        ERR("Error: Getting arguments from message.");
        cb(proxy, user_data, pending, &error_info);
        return;
     }
   cb(proxy, user_data, pending, NULL);
   return;
}

Eldbus_Pending *
geo_clue2_client_stop_call(Eldbus_Proxy *proxy, Geo_Clue2_Client_Stop_Cb cb, const void *data)
{
   Eldbus_Message *msg;
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   msg = eldbus_proxy_method_call_new(proxy, "Stop");
   if (!eldbus_message_arguments_append(msg, ""))
     {
        ERR("Error: Filling message.");
        return NULL;
     }
   p = eldbus_proxy_send(proxy, msg, cb_geo_clue2_client_stop, cb, -1);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
geo_clue2_client_location_updated_data_free(void *user_data EINA_UNUSED, void *func_data)
{
   Geo_Clue2_Client_LocationUpdated_Data *s_data = func_data;
   free(s_data->old);
   free(s_data->new);
   free(s_data);
}

static void
on_geo_clue2_client_location_updated(void *data, const Eldbus_Message *msg)
{
   Eldbus_Proxy *proxy = data;
   Geo_Clue2_Client_LocationUpdated_Data *s_data = calloc(1, sizeof(Geo_Clue2_Client_LocationUpdated_Data));
   s_data->proxy = proxy;
   if (!eldbus_message_arguments_get(msg, "oo", &s_data->old, &s_data->new))
     {
        ERR("Error: Getting arguments from message.");
        free(s_data);
        return;
     }
   s_data->old = strdup(s_data->old);
   s_data->new = strdup(s_data->new);
   ecore_event_add(GEO_CLUE2_CLIENT_LOCATION_UPDATED_EVENT, s_data, geo_clue2_client_location_updated_data_free, NULL);
}

static void
cb_geo_clue2_client_location(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   const char *error, *error_msg;
   Eldbus_Codegen_Property_String_Get_Cb cb = data;
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   Eldbus_Message_Iter *variant;
   const char *v;
   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        Eldbus_Error_Info error_info = {error, error_msg};
        cb(user_data, pending, "Location", proxy, &error_info, NULL);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "Location", proxy, &error_info, NULL);
        return;
     }
   if (!eldbus_message_iter_arguments_get(variant, "o", &v))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "Location", proxy, &error_info, NULL);
        return;
     }
   cb(user_data, pending, "Location", proxy, NULL, v);
}

Eldbus_Pending *
geo_clue2_client_location_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_String_Get_Cb cb, const void *data)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   p = eldbus_proxy_property_get(proxy, "Location", cb_geo_clue2_client_location, cb);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_client_distance_threshold(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   const char *error, *error_msg;
   Eldbus_Codegen_Property_Uint32_Get_Cb cb = data;
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   Eldbus_Message_Iter *variant;
   unsigned int v;
   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        Eldbus_Error_Info error_info = {error, error_msg};
        cb(user_data, pending, "DistanceThreshold", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "DistanceThreshold", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_iter_arguments_get(variant, "u", &v))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "DistanceThreshold", proxy, &error_info, 0);
        return;
     }
   cb(user_data, pending, "DistanceThreshold", proxy, NULL, v);
}

Eldbus_Pending *
geo_clue2_client_distance_threshold_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Uint32_Get_Cb cb, const void *data)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   p = eldbus_proxy_property_get(proxy, "DistanceThreshold", cb_geo_clue2_client_distance_threshold, cb);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_client_distance_threshold_set(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   const char *error, *error_msg;
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   Eldbus_Codegen_Property_Set_Cb cb = data;
   if (eldbus_message_error_get(msg, &error, &error_msg))     {
        Eldbus_Error_Info error_info = {error, error_msg};

        cb(user_data, "DistanceThreshold", proxy, pending, &error_info);
        return;
     }
   cb(user_data, "DistanceThreshold", proxy, pending, NULL);
}

Eldbus_Pending *
geo_clue2_client_distance_threshold_propset(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Set_Cb cb, const void *data, const void *value)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, NULL);
   p = eldbus_proxy_property_set(proxy, "DistanceThreshold", "u", value, cb_geo_clue2_client_distance_threshold_set, data);
   eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_client_desktop_id(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   const char *error, *error_msg;
   Eldbus_Codegen_Property_String_Get_Cb cb = data;
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   Eldbus_Message_Iter *variant;
   const char *v;
   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        Eldbus_Error_Info error_info = {error, error_msg};
        cb(user_data, pending, "DesktopId", proxy, &error_info, NULL);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "DesktopId", proxy, &error_info, NULL);
        return;
     }
   if (!eldbus_message_iter_arguments_get(variant, "s", &v))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "DesktopId", proxy, &error_info, NULL);
        return;
     }
   cb(user_data, pending, "DesktopId", proxy, NULL, v);
}

Eldbus_Pending *
geo_clue2_client_desktop_id_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_String_Get_Cb cb, const void *data)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   p = eldbus_proxy_property_get(proxy, "DesktopId", cb_geo_clue2_client_desktop_id, cb);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_client_desktop_id_set(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   const char *error, *error_msg;
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   Eldbus_Codegen_Property_Set_Cb cb = data;
   if (eldbus_message_error_get(msg, &error, &error_msg))     {
        Eldbus_Error_Info error_info = {error, error_msg};

        cb(user_data, "DesktopId", proxy, pending, &error_info);
        return;
     }
   cb(user_data, "DesktopId", proxy, pending, NULL);
}

Eldbus_Pending *
geo_clue2_client_desktop_id_propset(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Set_Cb cb, const void *data, const void *value)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, NULL);
   p = eldbus_proxy_property_set(proxy, "DesktopId", "s", value, cb_geo_clue2_client_desktop_id_set, data);
   eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_client_requested_accuracy_level(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   const char *error, *error_msg;
   Eldbus_Codegen_Property_Uint32_Get_Cb cb = data;
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   Eldbus_Message_Iter *variant;
   unsigned int v;
   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        Eldbus_Error_Info error_info = {error, error_msg};
        cb(user_data, pending, "RequestedAccuracyLevel", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "RequestedAccuracyLevel", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_iter_arguments_get(variant, "u", &v))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "RequestedAccuracyLevel", proxy, &error_info, 0);
        return;
     }
   cb(user_data, pending, "RequestedAccuracyLevel", proxy, NULL, v);
}

Eldbus_Pending *
geo_clue2_client_requested_accuracy_level_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Uint32_Get_Cb cb, const void *data)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   p = eldbus_proxy_property_get(proxy, "RequestedAccuracyLevel", cb_geo_clue2_client_requested_accuracy_level, cb);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_client_requested_accuracy_level_set(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   const char *error, *error_msg;
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   Eldbus_Codegen_Property_Set_Cb cb = data;
   if (eldbus_message_error_get(msg, &error, &error_msg))     {
        Eldbus_Error_Info error_info = {error, error_msg};

        cb(user_data, "RequestedAccuracyLevel", proxy, pending, &error_info);
        return;
     }
   cb(user_data, "RequestedAccuracyLevel", proxy, pending, NULL);
}

Eldbus_Pending *
geo_clue2_client_requested_accuracy_level_propset(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Set_Cb cb, const void *data, const void *value)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, NULL);
   p = eldbus_proxy_property_set(proxy, "RequestedAccuracyLevel", "u", value, cb_geo_clue2_client_requested_accuracy_level_set, data);
   eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_client_active(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   const char *error, *error_msg;
   Eldbus_Codegen_Property_Bool_Get_Cb cb = data;
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   Eldbus_Message_Iter *variant;
   Eina_Bool v;
   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        Eldbus_Error_Info error_info = {error, error_msg};
        cb(user_data, pending, "Active", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "Active", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_iter_arguments_get(variant, "b", &v))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "Active", proxy, &error_info, 0);
        return;
     }
   cb(user_data, pending, "Active", proxy, NULL, v);
}

Eldbus_Pending *
geo_clue2_client_active_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Bool_Get_Cb cb, const void *data)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   p = eldbus_proxy_property_get(proxy, "Active", cb_geo_clue2_client_active, cb);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

void
geo_clue2_client_log_domain_set(int id)
{
   _log_main = id;
}

void
geo_clue2_client_proxy_unref(Eldbus_Proxy *proxy)
{
   Eldbus_Object *obj = eldbus_proxy_object_get(proxy);
   eldbus_proxy_unref(proxy);
   eldbus_object_unref(obj);
}

Eldbus_Proxy *
geo_clue2_client_proxy_get(Eldbus_Connection *conn, const char *bus, const char *path)
{
   Eldbus_Object *obj;
   Eldbus_Proxy *proxy;
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(bus, NULL);
   if (!path) path = "/";
   obj = eldbus_object_get(conn, bus, path);
   proxy = eldbus_proxy_get(obj, "org.freedesktop.GeoClue2.Client");
   eldbus_proxy_signal_handler_add(proxy, "LocationUpdated", on_geo_clue2_client_location_updated, proxy);
   if (!GEO_CLUE2_CLIENT_LOCATION_UPDATED_EVENT)
     GEO_CLUE2_CLIENT_LOCATION_UPDATED_EVENT = ecore_event_type_new();
   return proxy;
}

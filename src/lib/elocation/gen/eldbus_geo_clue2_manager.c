#include "eldbus_geo_clue2_manager.h"

static int _log_main = -1;
#undef ERR
#define ERR(...) EINA_LOG_DOM_ERR(_log_main, __VA_ARGS__);

static void
cb_geo_clue2_manager_get_client(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   Geo_Clue2_Manager_Get_Client_Cb cb = data;
   const char *error, *error_msg;
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   const char *client = NULL;
   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        Eldbus_Error_Info error_info = {error, error_msg};
        cb(proxy, user_data, pending, &error_info, client);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "o", &client))
     {
        Eldbus_Error_Info error_info = {"", ""};
        ERR("Error: Getting arguments from message.");
        cb(proxy, user_data, pending, &error_info, client);
        return;
     }
   cb(proxy, user_data, pending, NULL, client);
   return;
}

Eldbus_Pending *
geo_clue2_manager_get_client_call(Eldbus_Proxy *proxy, Geo_Clue2_Manager_Get_Client_Cb cb, const void *data)
{
   Eldbus_Message *msg;
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   msg = eldbus_proxy_method_call_new(proxy, "GetClient");
   if (!eldbus_message_arguments_append(msg, ""))
     {
        ERR("Error: Filling message.");
        return NULL;
     }
   p = eldbus_proxy_send(proxy, msg, cb_geo_clue2_manager_get_client, cb, -1);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_manager_add_agent(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   Geo_Clue2_Manager_Add_Agent_Cb cb = data;
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
geo_clue2_manager_add_agent_call(Eldbus_Proxy *proxy, Geo_Clue2_Manager_Add_Agent_Cb cb, const void *data, const char *id)
{
   Eldbus_Message *msg;
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   msg = eldbus_proxy_method_call_new(proxy, "AddAgent");
   if (!eldbus_message_arguments_append(msg, "s", id))
     {
        ERR("Error: Filling message.");
        return NULL;
     }
   p = eldbus_proxy_send(proxy, msg, cb_geo_clue2_manager_add_agent, cb, -1);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_manager_in_use(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
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
        cb(user_data, pending, "InUse", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "InUse", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_iter_arguments_get(variant, "b", &v))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "InUse", proxy, &error_info, 0);
        return;
     }
   cb(user_data, pending, "InUse", proxy, NULL, v);
}

Eldbus_Pending *
geo_clue2_manager_in_use_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Bool_Get_Cb cb, const void *data)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   p = eldbus_proxy_property_get(proxy, "InUse", cb_geo_clue2_manager_in_use, cb);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_manager_available_accuracy_level(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
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
        cb(user_data, pending, "AvailableAccuracyLevel", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "AvailableAccuracyLevel", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_iter_arguments_get(variant, "u", &v))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "AvailableAccuracyLevel", proxy, &error_info, 0);
        return;
     }
   cb(user_data, pending, "AvailableAccuracyLevel", proxy, NULL, v);
}

Eldbus_Pending *
geo_clue2_manager_available_accuracy_level_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Uint32_Get_Cb cb, const void *data)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   p = eldbus_proxy_property_get(proxy, "AvailableAccuracyLevel", cb_geo_clue2_manager_available_accuracy_level, cb);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

void
geo_clue2_manager_log_domain_set(int id)
{
   _log_main = id;
}

void
geo_clue2_manager_proxy_unref(Eldbus_Proxy *proxy)
{
   Eldbus_Object *obj = eldbus_proxy_object_get(proxy);
   eldbus_proxy_unref(proxy);
   eldbus_object_unref(obj);
}

Eldbus_Proxy *
geo_clue2_manager_proxy_get(Eldbus_Connection *conn, const char *bus, const char *path)
{
   Eldbus_Object *obj;
   Eldbus_Proxy *proxy;
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(bus, NULL);
   if (!path) path = "/";
   obj = eldbus_object_get(conn, bus, path);
   proxy = eldbus_proxy_get(obj, "org.freedesktop.GeoClue2.Manager");
   return proxy;
}

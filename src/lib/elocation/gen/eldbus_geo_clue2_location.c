#include "eldbus_geo_clue2_location.h"

static int _log_main = -1;
#undef ERR
#define ERR(...) EINA_LOG_DOM_ERR(_log_main, __VA_ARGS__);

static void
cb_geo_clue2_location_latitude(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   const char *error, *error_msg;
   Eldbus_Codegen_Property_Double_Get_Cb cb = data;
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   Eldbus_Message_Iter *variant;
   double v;
   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        Eldbus_Error_Info error_info = {error, error_msg};
        cb(user_data, pending, "Latitude", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "Latitude", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_iter_arguments_get(variant, "d", &v))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "Latitude", proxy, &error_info, 0);
        return;
     }
   cb(user_data, pending, "Latitude", proxy, NULL, v);
}

Eldbus_Pending *
geo_clue2_location_latitude_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Double_Get_Cb cb, const void *data)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   p = eldbus_proxy_property_get(proxy, "Latitude", cb_geo_clue2_location_latitude, cb);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_location_longitude(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   const char *error, *error_msg;
   Eldbus_Codegen_Property_Double_Get_Cb cb = data;
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   Eldbus_Message_Iter *variant;
   double v;
   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        Eldbus_Error_Info error_info = {error, error_msg};
        cb(user_data, pending, "Longitude", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "Longitude", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_iter_arguments_get(variant, "d", &v))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "Longitude", proxy, &error_info, 0);
        return;
     }
   cb(user_data, pending, "Longitude", proxy, NULL, v);
}

Eldbus_Pending *
geo_clue2_location_longitude_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Double_Get_Cb cb, const void *data)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   p = eldbus_proxy_property_get(proxy, "Longitude", cb_geo_clue2_location_longitude, cb);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_location_accuracy(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   const char *error, *error_msg;
   Eldbus_Codegen_Property_Double_Get_Cb cb = data;
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   Eldbus_Message_Iter *variant;
   double v;
   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        Eldbus_Error_Info error_info = {error, error_msg};
        cb(user_data, pending, "Accuracy", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "Accuracy", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_iter_arguments_get(variant, "d", &v))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "Accuracy", proxy, &error_info, 0);
        return;
     }
   cb(user_data, pending, "Accuracy", proxy, NULL, v);
}

Eldbus_Pending *
geo_clue2_location_accuracy_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Double_Get_Cb cb, const void *data)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   p = eldbus_proxy_property_get(proxy, "Accuracy", cb_geo_clue2_location_accuracy, cb);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_location_altitude(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
{
   void *user_data = eldbus_pending_data_del(pending, "__user_data");
   const char *error, *error_msg;
   Eldbus_Codegen_Property_Double_Get_Cb cb = data;
   Eldbus_Proxy *proxy = eldbus_pending_data_del(pending, "__proxy");
   Eldbus_Message_Iter *variant;
   double v;
   if (eldbus_message_error_get(msg, &error, &error_msg))
     {
        Eldbus_Error_Info error_info = {error, error_msg};
        cb(user_data, pending, "Altitude", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "Altitude", proxy, &error_info, 0);
        return;
     }
   if (!eldbus_message_iter_arguments_get(variant, "d", &v))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "Altitude", proxy, &error_info, 0);
        return;
     }
   cb(user_data, pending, "Altitude", proxy, NULL, v);
}

Eldbus_Pending *
geo_clue2_location_altitude_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Double_Get_Cb cb, const void *data)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   p = eldbus_proxy_property_get(proxy, "Altitude", cb_geo_clue2_location_altitude, cb);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

static void
cb_geo_clue2_location_description(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending)
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
        cb(user_data, pending, "Description", proxy, &error_info, NULL);
        return;
     }
   if (!eldbus_message_arguments_get(msg, "v", &variant))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "Description", proxy, &error_info, NULL);
        return;
     }
   if (!eldbus_message_iter_arguments_get(variant, "s", &v))
     {
        Eldbus_Error_Info error_info = {"", ""};
        cb(user_data, pending, "Description", proxy, &error_info, NULL);
        return;
     }
   cb(user_data, pending, "Description", proxy, NULL, v);
}

Eldbus_Pending *
geo_clue2_location_description_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_String_Get_Cb cb, const void *data)
{
   Eldbus_Pending *p;
   EINA_SAFETY_ON_NULL_RETURN_VAL(proxy, NULL);
   p = eldbus_proxy_property_get(proxy, "Description", cb_geo_clue2_location_description, cb);
   if (data)
     eldbus_pending_data_set(p, "__user_data", data);
   eldbus_pending_data_set(p, "__proxy", proxy);
   return p;
}

void
geo_clue2_location_log_domain_set(int id)
{
   _log_main = id;
}

void
geo_clue2_location_proxy_unref(Eldbus_Proxy *proxy)
{
   Eldbus_Object *obj = eldbus_proxy_object_get(proxy);
   eldbus_proxy_unref(proxy);
   eldbus_object_unref(obj);
}

Eldbus_Proxy *
geo_clue2_location_proxy_get(Eldbus_Connection *conn, const char *bus, const char *path)
{
   Eldbus_Object *obj;
   Eldbus_Proxy *proxy;
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(bus, NULL);
   if (!path) path = "/";
   obj = eldbus_object_get(conn, bus, path);
   proxy = eldbus_proxy_get(obj, "org.freedesktop.GeoClue2.Location");
   return proxy;
}

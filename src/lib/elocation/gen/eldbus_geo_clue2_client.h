#ifndef ELDBUS_GEO_CLUE2_CLIENT_H
#define ELDBUS_GEO_CLUE2_CLIENT_H

#include <Eina.h>
#include <Ecore.h>
#include <Eldbus.h>
#include "eldbus_utils.h"

Eldbus_Proxy *geo_clue2_client_proxy_get(Eldbus_Connection *conn, const char *bus, const char *path);
void geo_clue2_client_proxy_unref(Eldbus_Proxy *proxy);
void geo_clue2_client_log_domain_set(int id);
typedef void (*Geo_Clue2_Client_Start_Cb)(Eldbus_Proxy *proxy, void *data, Eldbus_Pending *pending, Eldbus_Error_Info *error);
Eldbus_Pending *geo_clue2_client_start_call(Eldbus_Proxy *proxy, Geo_Clue2_Client_Start_Cb cb, const void *data);
typedef void (*Geo_Clue2_Client_Stop_Cb)(Eldbus_Proxy *proxy, void *data, Eldbus_Pending *pending, Eldbus_Error_Info *error);
Eldbus_Pending *geo_clue2_client_stop_call(Eldbus_Proxy *proxy, Geo_Clue2_Client_Stop_Cb cb, const void *data);
extern int GEO_CLUE2_CLIENT_LOCATION_UPDATED_EVENT;
typedef struct _Geo_Clue2_Client_LocationUpdated_Data
{
   Eldbus_Proxy *proxy;
   char *old;
   char *new;
} Geo_Clue2_Client_LocationUpdated_Data;
Eldbus_Pending *geo_clue2_client_location_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_String_Get_Cb cb, const void *data);
Eldbus_Pending *geo_clue2_client_distance_threshold_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Uint32_Get_Cb cb, const void *data);
Eldbus_Pending *geo_clue2_client_distance_threshold_propset(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Set_Cb cb, const void *data, const void *value);
Eldbus_Pending *geo_clue2_client_desktop_id_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_String_Get_Cb cb, const void *data);
Eldbus_Pending *geo_clue2_client_desktop_id_propset(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Set_Cb cb, const void *data, const void *value);
Eldbus_Pending *geo_clue2_client_requested_accuracy_level_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Uint32_Get_Cb cb, const void *data);
Eldbus_Pending *geo_clue2_client_requested_accuracy_level_propset(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Set_Cb cb, const void *data, const void *value);
Eldbus_Pending *geo_clue2_client_active_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Bool_Get_Cb cb, const void *data);

#endif

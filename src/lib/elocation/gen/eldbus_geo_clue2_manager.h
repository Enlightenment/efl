#ifndef ELDBUS_GEO_CLUE2_MANAGER_H
#define ELDBUS_GEO_CLUE2_MANAGER_H

#include <Eina.h>
#include <Ecore.h>
#include <Eldbus.h>
#include "eldbus_utils.h"

Eldbus_Proxy *geo_clue2_manager_proxy_get(Eldbus_Connection *conn, const char *bus, const char *path);
void geo_clue2_manager_proxy_unref(Eldbus_Proxy *proxy);
void geo_clue2_manager_log_domain_set(int id);
typedef void (*Geo_Clue2_Manager_Get_Client_Cb)(Eldbus_Proxy *proxy, void *data, Eldbus_Pending *pending, Eldbus_Error_Info *error, const char *client);
Eldbus_Pending *geo_clue2_manager_get_client_call(Eldbus_Proxy *proxy, Geo_Clue2_Manager_Get_Client_Cb cb, const void *data);
typedef void (*Geo_Clue2_Manager_Add_Agent_Cb)(Eldbus_Proxy *proxy, void *data, Eldbus_Pending *pending, Eldbus_Error_Info *error);
Eldbus_Pending *geo_clue2_manager_add_agent_call(Eldbus_Proxy *proxy, Geo_Clue2_Manager_Add_Agent_Cb cb, const void *data, const char *id);
Eldbus_Pending *geo_clue2_manager_in_use_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Bool_Get_Cb cb, const void *data);
Eldbus_Pending *geo_clue2_manager_available_accuracy_level_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Uint32_Get_Cb cb, const void *data);

#endif
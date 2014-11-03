#ifndef ELDBUS_GEO_CLUE2_LOCATION_H
#define ELDBUS_GEO_CLUE2_LOCATION_H

#include <Eina.h>
#include <Ecore.h>
#include <Eldbus.h>
#include "eldbus_utils.h"

Eldbus_Proxy *geo_clue2_location_proxy_get(Eldbus_Connection *conn, const char *bus, const char *path);
void geo_clue2_location_proxy_unref(Eldbus_Proxy *proxy);
void geo_clue2_location_log_domain_set(int id);
Eldbus_Pending *geo_clue2_location_latitude_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Double_Get_Cb cb, const void *data);
Eldbus_Pending *geo_clue2_location_longitude_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Double_Get_Cb cb, const void *data);
Eldbus_Pending *geo_clue2_location_accuracy_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Double_Get_Cb cb, const void *data);
Eldbus_Pending *geo_clue2_location_altitude_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_Double_Get_Cb cb, const void *data);
Eldbus_Pending *geo_clue2_location_description_propget(Eldbus_Proxy *proxy, Eldbus_Codegen_Property_String_Get_Cb cb, const void *data);

#endif
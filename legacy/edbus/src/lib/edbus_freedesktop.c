#include "edbus_private.h"
#include "edbus_private_types.h"
#include <dbus/dbus.h>

static EDBus_Proxy *
get_freedesktop_proxy(EDBus_Connection *conn)
{
   EDBus_Object *freedesktop_obj;

   freedesktop_obj = edbus_object_get(conn, EDBUS_FDO_BUS, EDBUS_FDO_PATH);
   return edbus_proxy_get(freedesktop_obj, EDBUS_FDO_INTERFACE);
}

EAPI EDBus_Pending *
edbus_name_request(EDBus_Connection *conn, const char *name, unsigned int flags, EDBus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return edbus_proxy_call(get_freedesktop_proxy(conn), "RequestName", cb,
                           cb_data, -1, "su", name, flags);
}

EAPI EDBus_Pending *
edbus_name_release(EDBus_Connection *conn, const char *name, EDBus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return edbus_proxy_call(get_freedesktop_proxy(conn), "ReleaseName", cb,
                           cb_data, -1, "s", name);
}

EAPI EDBus_Pending *
edbus_name_owner_get(EDBus_Connection *conn, const char *name, EDBus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return edbus_proxy_call(get_freedesktop_proxy(conn), "GetNameOwner", cb,
                           cb_data, -1, "s", name);
}

EAPI EDBus_Pending *
edbus_name_owner_has(EDBus_Connection *conn, const char *name, EDBus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return edbus_proxy_call(get_freedesktop_proxy(conn), "NameHasOwner", cb,
                           cb_data, -1, "s", name);
}

EAPI EDBus_Pending *
edbus_names_list(EDBus_Connection *conn, EDBus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);

   return edbus_proxy_call(get_freedesktop_proxy(conn), "ListNames", cb,
                           cb_data, -1, "");
}

EAPI EDBus_Pending *
edbus_names_activatable_list(EDBus_Connection *conn, EDBus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);

   return edbus_proxy_call(get_freedesktop_proxy(conn), "ListActivatableNames", cb,
                           cb_data, -1, "");
}

EAPI EDBus_Pending *
edbus_name_start(EDBus_Connection *conn, const char *name, unsigned int flags, EDBus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return edbus_proxy_call(get_freedesktop_proxy(conn), "StartServiceByName", cb,
                           cb_data, -1, "su", name, flags);
}

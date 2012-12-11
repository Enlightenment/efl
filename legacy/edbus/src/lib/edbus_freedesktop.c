#include "edbus_private.h"
#include "edbus_private_types.h"
#include <dbus/dbus.h>

EAPI EDBus_Pending *
edbus_name_request(EDBus_Connection *conn, const char *name, unsigned int flags, EDBus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return edbus_proxy_call(conn->daemon, "RequestName", cb,
                           cb_data, -1, "su", name, flags);
}

EAPI EDBus_Pending *
edbus_name_release(EDBus_Connection *conn, const char *name, EDBus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return edbus_proxy_call(conn->daemon, "ReleaseName", cb,
                           cb_data, -1, "s", name);
}

EAPI EDBus_Pending *
edbus_name_owner_get(EDBus_Connection *conn, const char *name, EDBus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return edbus_proxy_call(conn->daemon, "GetNameOwner", cb,
                           cb_data, -1, "s", name);
}

EAPI EDBus_Pending *
edbus_name_owner_has(EDBus_Connection *conn, const char *name, EDBus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return edbus_proxy_call(conn->daemon, "NameHasOwner", cb,
                           cb_data, -1, "s", name);
}

EAPI EDBus_Pending *
edbus_names_list(EDBus_Connection *conn, EDBus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);

   return edbus_proxy_call(conn->daemon, "ListNames", cb,
                           cb_data, -1, "");
}

EAPI EDBus_Pending *
edbus_names_activatable_list(EDBus_Connection *conn, EDBus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);

   return edbus_proxy_call(conn->daemon, "ListActivatableNames", cb,
                           cb_data, -1, "");
}

EAPI EDBus_Pending *
edbus_name_start(EDBus_Connection *conn, const char *name, unsigned int flags, EDBus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return edbus_proxy_call(conn->daemon, "StartServiceByName", cb,
                           cb_data, -1, "su", name, flags);
}

EAPI EDBus_Pending *
edbus_object_managed_objects_get(EDBus_Object *obj, EDBus_Message_Cb cb, const void *data)
{
   EDBus_Proxy *proxy;
   proxy = edbus_proxy_get(obj, EDBUS_FDO_INTERFACE_OBJECT_MANAGER);
   return edbus_proxy_call(proxy, "GetManagedObjects", cb, data, -1, "");
}

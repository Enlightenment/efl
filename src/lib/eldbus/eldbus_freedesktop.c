#include "eldbus_private.h"
#include "eldbus_private_types.h"
#include <dbus/dbus.h>

EAPI Eldbus_Pending *
eldbus_name_request(Eldbus_Connection *conn, const char *name, unsigned int flags, Eldbus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return eldbus_proxy_call(conn->fdo_proxy, "RequestName", cb,
                           cb_data, -1, "su", name, flags);
}

EAPI Eldbus_Pending *
eldbus_name_release(Eldbus_Connection *conn, const char *name, Eldbus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return eldbus_proxy_call(conn->fdo_proxy, "ReleaseName", cb,
                           cb_data, -1, "s", name);
}

EAPI Eldbus_Pending *
eldbus_name_owner_get(Eldbus_Connection *conn, const char *name, Eldbus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return eldbus_proxy_call(conn->fdo_proxy, "GetNameOwner", cb,
                           cb_data, -1, "s", name);
}

EAPI Eldbus_Pending *
eldbus_name_owner_has(Eldbus_Connection *conn, const char *name, Eldbus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return eldbus_proxy_call(conn->fdo_proxy, "NameHasOwner", cb,
                           cb_data, -1, "s", name);
}

EAPI Eldbus_Pending *
eldbus_names_list(Eldbus_Connection *conn, Eldbus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);

   return eldbus_proxy_call(conn->fdo_proxy, "ListNames", cb,
                           cb_data, -1, "");
}

EAPI Eldbus_Pending *
eldbus_names_activatable_list(Eldbus_Connection *conn, Eldbus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);

   return eldbus_proxy_call(conn->fdo_proxy, "ListActivatableNames", cb,
                           cb_data, -1, "");
}

EAPI Eldbus_Pending *
eldbus_name_start(Eldbus_Connection *conn, const char *name, unsigned int flags, Eldbus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   return eldbus_proxy_call(conn->fdo_proxy, "StartServiceByName", cb,
                           cb_data, -1, "su", name, flags);
}

EAPI Eldbus_Pending *
eldbus_object_managed_objects_get(Eldbus_Object *obj, Eldbus_Message_Cb cb, const void *data)
{
   Eldbus_Message *msg;
   Eldbus_Pending *p;
   msg = eldbus_object_method_call_new(obj, ELDBUS_FDO_INTERFACE_OBJECT_MANAGER,
                                      "GetManagedObjects");
   p = eldbus_object_send(obj, msg, cb, data, -1);
   return p;
}

EAPI Eldbus_Signal_Handler *
eldbus_object_manager_interfaces_added(Eldbus_Object *obj, Eldbus_Signal_Cb cb, const void *cb_data)
{
   return eldbus_object_signal_handler_add(obj, ELDBUS_FDO_INTERFACE_OBJECT_MANAGER,
                                           "InterfacesAdded", cb, cb_data);
}

EAPI Eldbus_Signal_Handler *
eldbus_object_manager_interfaces_removed(Eldbus_Object *obj, Eldbus_Signal_Cb cb, const void *cb_data)
{
   return eldbus_object_signal_handler_add(obj, ELDBUS_FDO_INTERFACE_OBJECT_MANAGER,
                                           "InterfacesRemoved", cb, cb_data);
}

EAPI Eldbus_Pending *
eldbus_hello(Eldbus_Connection *conn, Eldbus_Message_Cb cb, const void *cb_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);

   return eldbus_proxy_call(conn->fdo_proxy, "Hello", cb, cb_data, -1, "");
}

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "atspi/atspi-constants.h"

void elm_atspi_dbus_object_reference_append(Eldbus_Connection *conn, Eldbus_Message_Iter *iter, Elm_Interface_Atspi_Accessible *accessible)
{
   struct dbus_address addr;
   ELM_ATSPI_BRIDGE_DATA_GET_OR_RETURN(bridge, pd);
   Eldbus_Message_Iter *iter_struct = eldbus_message_iter_container_new(iter, 'r', NULL);
   EINA_SAFETY_ON_NULL_RETURN(iter);

   _bridge_address_from_object(bridge, obj, &addr);

   eldbus_message_iter_basic_append(iter_struct, 's', addr.bus);
   eldbus_message_iter_basic_append(iter_struct, 'o', addr.path);
   eldbus_message_iter_container_close(iter, iter_struct);
}

void elm_atspi_dbus_object_interfaces_append(Eldbus_Message_Iter *iter, const Eo *obj)
{
   Eldbus_Message_Iter *iter_array;
   iter_array = eldbus_message_iter_container_new(iter, 'a', "s");
   if (!iter_array) return;

   if (efl_isa(obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN))
     {
        eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_ACCESSIBLE);
        eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_COLLECTION);
     }
   if (efl_isa(obj, ELM_INTERFACE_ATSPI_ACTION_MIXIN))
     eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_ACTION);
   if (efl_isa(obj, ELM_ATSPI_APP_OBJECT_CLASS))
     eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_APPLICATION);
   if (efl_isa(obj, ELM_INTERFACE_ATSPI_COMPONENT_MIXIN))
     eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_COMPONENT);
   if (efl_isa(obj, ELM_INTERFACE_ATSPI_TEXT_EDITABLE_INTERFACE))
     eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_EDITABLE_TEXT);
   if (efl_isa(obj, ELM_INTERFACE_ATSPI_IMAGE_MIXIN))
     eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_IMAGE);
   if (efl_isa(obj, ELM_INTERFACE_ATSPI_SELECTION_INTERFACE))
     eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_SELECTION);
   if (efl_isa(obj, ELM_INTERFACE_ATSPI_TEXT_INTERFACE))
     eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_TEXT);
   if (efl_isa(obj, ELM_INTERFACE_ATSPI_VALUE_INTERFACE))
     eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_VALUE);

   eldbus_message_iter_container_close(iter, iter_array);
}
 

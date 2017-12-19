#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_PROTECTED

#include <Elementary.h>
#include "atspi/atspi-constants.h"
#include "elm_priv.h"
#include "elm_atspi_adaptor_common.h"

#define ELM_ACCESS_OBJECT_PATH_PREFIX  "/org/a11y/atspi/accessible/"
#define ELM_ACCESS_OBJECT_REFERENCE_TEMPLATE ELM_ACCESS_OBJECT_PATH_PREFIX "%llu"
#define ELM_ACCESS_OBJECT_PATH_ROOT "root"

Efl_Access*
efl_access_unmarshal(const Eldbus_Message *msg)
{
   return NULL;
}

void
efl_access_marshal(Efl_Access *obj, Eldbus_Message_Iter *iter)
{
}

static const char*
_path_from_object(const Eo *eo)
{
   static char path[64];

   if (!eo)
     return ATSPI_DBUS_PATH_NULL;

   if (eo == efl_access_root_get(EFL_ACCESS_MIXIN))
     snprintf(path, sizeof(path), "%s%s", ELM_ACCESS_OBJECT_PATH_PREFIX, ELM_ACCESS_OBJECT_PATH_ROOT);
   else
     snprintf(path, sizeof(path), ELM_ACCESS_OBJECT_REFERENCE_TEMPLATE, (unsigned long long)(uintptr_t)eo);

   return path;
}

void
eldbus_message_iter_efl_access_reference_append(Eldbus_Message_Iter *iter, Eldbus_Connection *conn, Efl_Access *obj)
{
   EINA_SAFETY_ON_NULL_RETURN(iter);
   EINA_SAFETY_ON_NULL_RETURN(conn);

   Eldbus_Message_Iter *iter_struct = eldbus_message_iter_container_new(iter, 'r', NULL);
   if (!iter_struct) return;

   eldbus_message_iter_basic_append(iter_struct, 's', eldbus_connection_unique_name_get(conn));
   eldbus_message_iter_basic_append(iter_struct, 'o', _path_from_object(obj));
   eldbus_message_iter_container_close(iter, iter_struct);
}

void
eldbus_message_iter_efl_access_reference_array_append(Eldbus_Message_Iter *iter, Eldbus_Connection *conn, Eina_List *obj_list)
{
   Eina_List *l;
   Efl_Access *obj;

   EINA_SAFETY_ON_NULL_RETURN(iter);
   EINA_SAFETY_ON_NULL_RETURN(conn);

   Eldbus_Message_Iter *iter_array = eldbus_message_iter_container_new(iter, 'a', "(so)");
   if (!iter_array) return;

   EINA_LIST_FOREACH(obj_list, l, obj)
      eldbus_message_iter_efl_access_reference_append(iter_array, conn, obj);

   eldbus_message_iter_container_close(iter, iter_array);
}

void
eldbus_message_iter_efl_access_interfaces_append(Eldbus_Message_Iter *iter, Efl_Access *obj)
{
  Eldbus_Message_Iter *iter_array = eldbus_message_iter_container_new(iter, 'a', "s");
  if (!iter_array) return;

  if (efl_isa(obj, EFL_ACCESS_MIXIN))
    {
       eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_ACCESSIBLE);
       eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_COLLECTION);
    }
  if (efl_isa(obj, EFL_ACCESS_ACTION_MIXIN))
    eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_ACTION);
  if (efl_isa(obj, ELM_ATSPI_APP_OBJECT_CLASS))
    eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_APPLICATION);
  if (efl_isa(obj, EFL_ACCESS_COMPONENT_MIXIN))
    eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_COMPONENT);
  if (efl_isa(obj, EFL_ACCESS_EDITABLE_TEXT_INTERFACE))
    eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_EDITABLE_TEXT);
  if (efl_isa(obj, EFL_ACCESS_IMAGE_MIXIN))
    eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_IMAGE);
  if (efl_isa(obj, EFL_ACCESS_SELECTION_INTERFACE))
    eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_SELECTION);
  if (efl_isa(obj, EFL_ACCESS_TEXT_INTERFACE))
    eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_TEXT);
  if (efl_isa(obj, EFL_ACCESS_VALUE_INTERFACE))
    eldbus_message_iter_basic_append(iter_array, 's', ATSPI_DBUS_INTERFACE_VALUE);

  eldbus_message_iter_container_close(iter, iter_array);
}

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#include "atspi/atspi-constants.h"

/*
 * Accessibility Bus info not defined in atspi-constants.h
 */
#define A11Y_DBUS_NAME "org.a11y.Bus"
#define A11Y_DBUS_PATH "/org/a11y/bus"
#define A11Y_DBUS_INTERFACE "org.a11y.Bus"

static int _init_count = 0;
static Eldbus_Connection *_a11y_bus = NULL;

static Eldbus_Message *
_role_get(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "u", ATSPI_ROLE_APPLICATION);
   return ret;
}

static Eldbus_Message *
_role_name_get(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   // Currently only root object is available.
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "s", "application");

   return ret;
}

static Eldbus_Message *
_role_localized_name_get(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   // Currently only root object is available.
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "s", gettext("application"));

   return ret;
}

static const Eldbus_Method accessible_methods[] = {
   { "GetRole", NULL, ELDBUS_ARGS({"u", "Role"}), _role_get, 0 },
   { "GetRoleName", NULL, ELDBUS_ARGS({"s", "Name"}), _role_name_get, 0 },
   { "GetLocalizedRoleName", NULL, ELDBUS_ARGS({"s", "LocalizedName"}), _role_localized_name_get, 0},
   { NULL, NULL, NULL, NULL, 0 }
};

static Eina_Bool
_accessible_property_get(const Eldbus_Service_Interface *interface EINA_UNUSED, const char *property,
                         Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED,
                         Eldbus_Message **error EINA_UNUSED)
{
   if (!strcmp(property, "Name"))
     {
        eldbus_message_iter_basic_append(iter, 's', elm_app_name_get());
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static const Eldbus_Property accessible_properties[] = {
   { "Name", "s", NULL, NULL, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Service_Interface_Desc accessible_iface_desc = {
   ATSPI_DBUS_INTERFACE_ACCESSIBLE, accessible_methods, NULL, accessible_properties, _accessible_property_get, NULL
};

static void
_on_app_register(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("%s %s", errname, errmsg);
        return;
     }
   DBG("Application successfuly registered at ATSPI2 bus.");
}

static Eina_Bool
_app_register(Eldbus_Connection *a11y_bus)
{
   Eldbus_Message *message = eldbus_message_method_call_new(ATSPI_DBUS_NAME_REGISTRY,
                                    ATSPI_DBUS_PATH_ROOT,
                                    ATSPI_DBUS_INTERFACE_SOCKET,
                                    "Embed");
   Eldbus_Message_Iter *iter = eldbus_message_iter_get(message);
   Eldbus_Message_Iter *iter_struct = eldbus_message_iter_container_new(iter, 'r', NULL);

   const char *bus = eldbus_connection_unique_name_get(a11y_bus);
   char *path = ATSPI_DBUS_PATH_ROOT;

   eldbus_message_iter_basic_append(iter_struct, 's', bus);
   eldbus_message_iter_basic_append(iter_struct, 'o', path);
   eldbus_message_iter_container_close(iter, iter_struct);

   eldbus_connection_send(a11y_bus, message, _on_app_register, NULL, -1);

   return EINA_TRUE;
}

static void
_interfaces_register(Eldbus_Connection *a11y_bus)
{
    eldbus_service_interface_register(a11y_bus, ATSPI_DBUS_PATH_ROOT, &accessible_iface_desc);
}

static void
_a11y_bus_initialize(const char *socket_addr)
{
   Eldbus_Connection *a11y_bus = eldbus_address_connection_get(socket_addr);

   _interfaces_register(a11y_bus);
   _app_register(a11y_bus);

   _a11y_bus = a11y_bus;
}

static void
_a11y_bus_address_get(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg, *sock_addr = NULL;
   Eldbus_Connection *session_bus = data;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        ERR("%s %s", errname, errmsg);
        goto end;
     }

   if (!eldbus_message_arguments_get(msg, "s", &sock_addr) || !sock_addr)
     {
        ERR("Could not get A11Y Bus socket address.");
        goto end;
     }

   _a11y_bus_initialize(sock_addr);

end:
   eldbus_connection_unref(session_bus);
}

void
_elm_atspi_bridge_init(void)
{
   Eldbus_Message *msg;
   Eldbus_Connection *session_bus;

   if (!_init_count && _elm_config->access_mode == ELM_ACCESS_MODE_ON)
     {
        session_bus = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
        msg = eldbus_message_method_call_new(A11Y_DBUS_NAME, A11Y_DBUS_PATH, A11Y_DBUS_INTERFACE, "GetAddress");
        eldbus_connection_send(session_bus, msg, _a11y_bus_address_get, session_bus, -1);
        _init_count = 1;
     }
}

void
_elm_atspi_bridge_shutdown(void)
{
   if (_init_count)
     {
       if (_a11y_bus)
           eldbus_connection_unref(_a11y_bus);
       _a11y_bus = NULL;
       _init_count = 0;
     }
}

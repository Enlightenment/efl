#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define ELM_ATSPI_ADAPTOR_PROTECTED
#define EFL_ACCESS_COMPONENT_PROTECTED

#include "atspi/atspi-constants.h"
#include <Elementary.h>
#include "elm_priv.h"
#include "elm_atspi_access_component_adaptor.eo.h"

typedef struct _Elm_Atspi_Access_Component_Adaptor_Data
{
} Elm_Atspi_Access_Component_Adaptor_Data;

static Eldbus_Message*
_handle_contains(Eo *obj, Efl_Access *access, const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   int x, y;
   Eina_Bool contains = EINA_FALSE;
   AtspiCoordType coord_type;
   Eldbus_Message *ret;

   if (!eldbus_message_arguments_get(msg, "iiu", &x, &y, &coord_type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   Eina_Bool type = coord_type == ATSPI_COORD_TYPE_SCREEN ? EINA_TRUE : EINA_FALSE;
   contains = efl_access_component_contains(access, type, x, y);

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   eldbus_message_arguments_append(ret, "b", contains);

   return ret;
}

static Eldbus_Message*
_handle_get_accessible_at_point(Eo *obj, Efl_Access *access, const Eldbus_Service_Interface *service EINA_UNUSED, const Eldbus_Message *msg)
{
   return NULL;
}

static Eldbus_Message*
_handle_get_extents(Eo *obj, Efl_Access *access, const Eldbus_Service_Interface *service EINA_UNUSED, const Eldbus_Message *msg)
{
   return NULL;
}

static Eldbus_Message*
_handle_get_size(Eo *obj, Efl_Access *access, const Eldbus_Service_Interface *service EINA_UNUSED, const Eldbus_Message *msg)
{
   return NULL;
}

static Eldbus_Message*
_handle_get_layer(Eo *obj, Efl_Access *access, const Eldbus_Service_Interface *service EINA_UNUSED, const Eldbus_Message *msg)
{
   return NULL;
}

static Eldbus_Message*
_handle_grab_focus(Eo *obj, Efl_Access *access, const Eldbus_Service_Interface *service EINA_UNUSED, const Eldbus_Message *msg)
{
   return NULL;
}

static Eldbus_Message*
_handle_get_alpha(Eo *obj, Efl_Access *access, const Eldbus_Service_Interface *service EINA_UNUSED, const Eldbus_Message *msg)
{
   return NULL;
}

static Eldbus_Message*
_handle_set_extents(Eo *obj, Efl_Access *access, const Eldbus_Service_Interface *service EINA_UNUSED, const Eldbus_Message *msg)
{
   return NULL;
}

static Eldbus_Message*
_handle_set_position(Eo *obj, Efl_Access *access, const Eldbus_Service_Interface *service EINA_UNUSED, const Eldbus_Message *msg)
{
   return NULL;
}

static Eldbus_Message*
_handle_set_size(Eo *obj, Efl_Access *access, const Eldbus_Service_Interface *service EINA_UNUSED, const Eldbus_Message *msg)
{
   return NULL;
}

static Eldbus_Message*
_component_msg_handle(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Elm_Atspi_Access_Component_Adaptor *obj = elm_atspi_adaptor_instance_get(ELM_ATSPI_ADAPTOR_CLASS, iface);
   Efl_Access *access = elm_atspi_adaptor_object_get(obj, eldbus_message_path_get(msg));

   if (!access || !efl_isa(access, EFL_ACCESS_COMPONENT_MIXIN))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid object reference.");

   const char *method = eldbus_message_member_get(msg);
   if (!method)
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid method name.");

   if (!strcmp(method, "Contains"))
     return _handle_contains(obj, access, iface, msg);
   else if (!strcmp(method, "GetAccessibleAtPoint"))
     return _handle_get_accessible_at_point(obj, access, iface, msg);
   else if (!strcmp(method, "GetExtents"))
     return _handle_get_extents(obj, access, iface, msg);
   else if (!strcmp(method, "GetSize"))
     return _handle_get_size(obj, access, iface, msg);
   else if (!strcmp(method, "GetLayer"))
     return _handle_get_layer(obj, access, iface, msg);
   else if (!strcmp(method, "GrabFocus"))
     return _handle_grab_focus(obj, access, iface, msg);
   else if (!strcmp(method, "GetAlpha"))
     return _handle_get_alpha(obj, access, iface, msg);
   else if (!strcmp(method, "SetExtents"))
     return _handle_set_extents(obj, access, iface, msg);
   else if (!strcmp(method, "SetPosition"))
     return _handle_set_position(obj, access, iface, msg);
   else if (!strcmp(method, "SetSize"))
     return _handle_set_size(obj, access, iface, msg);

   return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.NotSupported", "Method not supported.");
}

static const Eldbus_Method component_methods[] = {
   { "Contains", ELDBUS_ARGS({"i", "x"}, {"i", "y"}, {"u", "coord_type"}), ELDBUS_ARGS({"b", "contains"}), _component_msg_handle, 0 },
   { "GetAccessibleAtPoint", ELDBUS_ARGS({"i", "x"}, {"i", "y"}, {"u", "coord_type"}), ELDBUS_ARGS({"(so)", "accessible"}), _component_msg_handle, 0 },
   { "GetExtents", ELDBUS_ARGS({"u", "coord_type"}), ELDBUS_ARGS({"(iiii)", "extents"}), _component_msg_handle, 0 },
   { "GetPosition", ELDBUS_ARGS({"u", "coord_type"}), ELDBUS_ARGS({"i", "x"}, {"i","y"}), _component_msg_handle, 0 },
   { "GetSize", NULL, ELDBUS_ARGS({"i", "w"}, {"i", "h"}), _component_msg_handle, 0 },
   { "GetLayer", NULL, ELDBUS_ARGS({"u", "layer"}), _component_msg_handle, 0 },
   { "GetMDIZOrder", NULL, ELDBUS_ARGS({"n", "MDIZOrder"}), _component_msg_handle, 0 },
   { "GrabFocus", NULL, ELDBUS_ARGS({"b", "focus"}), _component_msg_handle, 0 },
   { "GetAlpha", NULL, ELDBUS_ARGS({"d", "alpha"}), _component_msg_handle, 0 },
   { "SetExtents", ELDBUS_ARGS({"i", "x"}, {"i", "y"}, {"i", "width"}, {"i", "height"}, {"u", "coord_type"}), ELDBUS_ARGS({"b", "result"}), _component_msg_handle, 0 },
   { "SetPosition", ELDBUS_ARGS({"i", "x"}, {"i", "y"}, {"u", "coord_type"}), ELDBUS_ARGS({"b", "result"}), _component_msg_handle, 0 },
   { "SetSize", ELDBUS_ARGS({"i", "width"}, {"i", "height"}), ELDBUS_ARGS({"b", "result"}), _component_msg_handle, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

EOLIAN static const Eldbus_Service_Interface_Desc*
_elm_atspi_access_component_interface_get(Eo *obj, Elm_Atspi_Access_Component_Adaptor_Data *pd)
{
   static const Eldbus_Service_Interface_Desc component_iface_desc = {
      ATSPI_DBUS_INTERFACE_COMPONENT, component_methods, NULL, NULL, NULL, NULL
   };
   return &component_iface_desc;
}

#include "elm_atspi_access_component_adaptor.eo.c"

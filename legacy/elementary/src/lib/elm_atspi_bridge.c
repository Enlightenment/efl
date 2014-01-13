#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <stdint.h>
#include <Elementary.h>
#include "elm_priv.h"

#include "atspi/atspi-constants.h"

/*
 * Accessibility Bus info not defined in atspi-constants.h
 */
#define A11Y_DBUS_NAME "org.a11y.Bus"
#define A11Y_DBUS_PATH "/org/a11y/bus"
#define A11Y_DBUS_INTERFACE "org.a11y.Bus"

#define CACHE_ITEM_SIGNATURE "((so)(so)(so)a(so)assusau)"
#define CACHE_INTERFACE_PATH "/org/a11y/atspi/cache"

#define ELM_ACCESS_OBJECT_PATH_ROOT "root"
#define ELM_ACCESS_OBJECT_PATH_PREFIX  "/org/a11y/atspi/accessible/"
#define ELM_ACCESS_OBJECT_REFERENCE_TEMPLATE ELM_ACCESS_OBJECT_PATH_PREFIX "%llu"

static int _init_count = 0;
static Eldbus_Connection *_a11y_bus = NULL;
static Elm_Atspi_Object *_root;
static Eina_Hash *_cache;
static Eldbus_Service_Interface *_cache_interface = NULL;
static Elm_Atspi_Object * _access_object_from_path(const char *path);
static char * _path_from_access_object(Elm_Atspi_Object *eo);
static void object_append_reference(Eldbus_Message_Iter *iter,  Elm_Atspi_Object *obj);
static void object_append_desktop_reference(Eldbus_Message_Iter *iter);
static const Eldbus_Service_Interface_Desc accessible_iface_desc;

enum
{
  ADD_ACCESSIBLE = 0,
  REMOVE_ACCESSIBLE
};

static Eldbus_Message *
_accessible_get_role(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   AtspiRole role;

   eo_do(obj, elm_atspi_obj_role_get(&role));
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "u", role);
   return ret;
}

static Eldbus_Message *
_accessible_get_role_name(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *role_name, *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);

   eo_do(obj, elm_atspi_obj_role_name_get(&role_name));
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "s", role_name);

   return ret;
}

static Eldbus_Message *
_accessible_get_localized_role_name(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *l_role_name, *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);

   eo_do(obj, elm_atspi_obj_localized_role_name_get(&l_role_name));
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "s", l_role_name);

   return ret;
}

static Eldbus_Message *
_accessible_get_children(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   Eina_List *children_list = NULL, *l;
   Eldbus_Message *ret;
   Eldbus_Message_Iter *iter, *iter_array;
   Elm_Atspi_Object *children;

   eo_do(obj, elm_atspi_obj_children_get(&children_list));

   ret = eldbus_message_method_return_new(msg);
   iter = eldbus_message_iter_get(ret);
   iter_array = eldbus_message_iter_container_new(iter, 'a', "(so)");

   EINA_LIST_FOREACH(children_list, l, children)
      object_append_reference(iter_array, children);

   eldbus_message_iter_container_close(iter, iter_array);
   eina_list_free(children_list);
   return ret;
}

static Eldbus_Message *
_accessible_get_application(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   Eldbus_Message_Iter *iter = eldbus_message_iter_get(ret);
   object_append_reference(iter, _root);

   return ret;
}

static Eldbus_Message *
_accessible_get_state(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   Eldbus_Message_Iter *iter = eldbus_message_iter_get(ret);
   Eldbus_Message_Iter *iter_array;

   iter_array = eldbus_message_iter_container_new(iter, 'a', "u");
   /* TODO: states are not implemented yet*/
   eldbus_message_iter_container_close(iter, iter_array);

   return ret;
}

static Eldbus_Message *
_accessible_get_index_in_parent(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   int idx;

   eo_do(obj, elm_atspi_obj_index_in_parent_get(&idx));
   eldbus_message_arguments_append(ret, "i", idx);

   return ret;
}

static Eldbus_Message *
_accessible_child_at_index(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   Elm_Atspi_Object *child;
   int idx;
   Eldbus_Message *ret;
   Eldbus_Message_Iter *iter;

   if (!eldbus_message_arguments_get(msg, "i", &idx))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   ret = eldbus_message_method_return_new(msg);
   iter = eldbus_message_iter_get(ret);
   eo_do(obj, elm_atspi_obj_child_at_index_get(idx, &child));
   object_append_reference(iter, child);

   return ret;
}

static const Eldbus_Method accessible_methods[] = {
   { "GetChildAtIndex", ELDBUS_ARGS({"i", "index"}), ELDBUS_ARGS({"(so)", "Accessible"}), _accessible_child_at_index, 0 },
   { "GetChildren", NULL, ELDBUS_ARGS({"a(so)", "children"}), _accessible_get_children, 0 },
   { "GetIndexInParent", NULL, ELDBUS_ARGS({"i", "index"}), _accessible_get_index_in_parent, 0 },
   { "GetRelationSet", NULL, ELDBUS_ARGS({"a(ua(so))", NULL}), _accessible_get_state, 0 },
   { "GetRole", NULL, ELDBUS_ARGS({"u", "Role"}), _accessible_get_role, 0 },
   { "GetRoleName", NULL, ELDBUS_ARGS({"s", "Name"}), _accessible_get_role_name, 0 },
   { "GetLocalizedRoleName", NULL, ELDBUS_ARGS({"s", "LocalizedName"}), _accessible_get_localized_role_name, 0},
   { "GetState", NULL, ELDBUS_ARGS({"au", NULL}), _accessible_get_state, 0},
   { "GetApplication", NULL, ELDBUS_ARGS({"(so)", NULL}), _accessible_get_application, 0},
   //{ "GetAttributes", NULL, ELDBUS_ARGS({"a{ss}", NULL}), NULL, 0},
   { NULL, NULL, NULL, NULL, 0 }
};

static Elm_Atspi_Object *
_access_object_from_path(const char *path)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);
   unsigned long long eo_ptr = 0;
   Elm_Atspi_Object *eo = NULL;
   const char *tmp = path;

   int len = (sizeof(ELM_ACCESS_OBJECT_PATH_PREFIX) / sizeof(char)) - 1;

   if (strncmp(path, ELM_ACCESS_OBJECT_PATH_PREFIX, len))
     return NULL;

   tmp = path + len; /* Skip over the prefix */
   if (!strcmp(ELM_ACCESS_OBJECT_PATH_ROOT, tmp))
     return _root;

   sscanf(tmp, "%llu", &eo_ptr);
   eo = (Elm_Atspi_Object *)eo_ptr;
   return eo_isa(eo, ELM_ATSPI_CLASS) ? eo : NULL;
}

static char *
_path_from_access_object(Elm_Atspi_Object *eo)
{
   char path[256];

   EINA_SAFETY_ON_NULL_RETURN_VAL(eo, strdup(ATSPI_DBUS_PATH_NULL));
   if (eo == _root)
     snprintf(path, sizeof(path), "%s%s", ELM_ACCESS_OBJECT_PATH_PREFIX, ELM_ACCESS_OBJECT_PATH_ROOT);
   else
     snprintf(path, sizeof(path), ELM_ACCESS_OBJECT_REFERENCE_TEMPLATE, (unsigned long long)eo);
   return strdup(path);
}

static Eina_Bool
_accessible_property_get(const Eldbus_Service_Interface *interface, const char *property,
                         Eldbus_Message_Iter *iter, const Eldbus_Message *request_msg EINA_UNUSED,
                         Eldbus_Message **error EINA_UNUSED)
{
   const char *ret, *obj_path = eldbus_service_object_path_get(interface);

   Elm_Atspi_Object *ret_obj = NULL, *obj = _access_object_from_path(obj_path);

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   if (!strcmp(property, "Name"))
     {
        eo_do(obj, elm_atspi_obj_name_get(&ret));
        if (!ret)
          ret = "";
        eldbus_message_iter_basic_append(iter, 's', ret);
        return EINA_TRUE;
     }
   else if (!strcmp(property, "Description"))
     {
        eo_do(obj, elm_atspi_obj_description_get(&ret));
        if (!ret)
          ret = "";
        eldbus_message_iter_basic_append(iter, 's', ret);
        return EINA_TRUE;
     }
   else if (!strcmp(property, "Parent"))
     {
       eo_do(obj, elm_atspi_obj_parent_get(&ret_obj));
       AtspiRole role;
       eo_do(obj, elm_atspi_obj_role_get(&role));
       if ((!ret_obj) && (ATSPI_ROLE_APPLICATION == role))
         object_append_desktop_reference(iter);
       else
         object_append_reference(iter, ret_obj);
       return EINA_TRUE;
     }
   else if (!strcmp(property, "ChildCount"))
     {
        Eina_List *l = NULL;
        eo_do(obj, elm_atspi_obj_children_get(&l));
        eldbus_message_iter_basic_append(iter, 'i', eina_list_count(l));
        eina_list_free(l);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static const Eldbus_Property accessible_properties[] = {
   { "Name", "s", NULL, NULL, 0 },
   { "Description", "s", NULL, NULL, 0 },
   { "Parent", "(so)", NULL, NULL, 0 },
   { "ChildCount", "i", NULL, NULL, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Service_Interface_Desc accessible_iface_desc = {
   ATSPI_DBUS_INTERFACE_ACCESSIBLE, accessible_methods, NULL, accessible_properties, _accessible_property_get, NULL
};

static void
object_append_reference(Eldbus_Message_Iter *iter, Elm_Atspi_Object *obj){

  Eldbus_Message_Iter *iter_struct = eldbus_message_iter_container_new(iter, 'r', NULL);
  char *path = _path_from_access_object(obj);
  eldbus_message_iter_basic_append(iter_struct, 's', eldbus_connection_unique_name_get(_a11y_bus));
  eldbus_message_iter_basic_append(iter_struct, 'o', path);
  eldbus_message_iter_container_close(iter, iter_struct);
  free(path);
}

static void
object_append_desktop_reference(Eldbus_Message_Iter *iter){

  Eldbus_Message_Iter *iter_struct = eldbus_message_iter_container_new(iter, 'r', NULL);

  eldbus_message_iter_basic_append(iter_struct, 's', ATSPI_DBUS_NAME_REGISTRY);
  eldbus_message_iter_basic_append(iter_struct, 'o', ATSPI_DBUS_PATH_ROOT);
  eldbus_message_iter_container_close(iter, iter_struct);
}

static Eina_Bool
_append_item_fn(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{

  if (!eo_ref_get(data) || eo_destructed_is(data))
    return EINA_TRUE;

  Eldbus_Message_Iter *iter_struct, *iter_sub_array;
  Eldbus_Message_Iter *iter_array = fdata;

  AtspiRole role;
  eo_do(data, elm_atspi_obj_role_get(&role));

  iter_struct = eldbus_message_iter_container_new(iter_array, 'r', NULL);
  /* Marshall object path */
  object_append_reference(iter_struct, data);

  /* Marshall application */
  object_append_reference(iter_struct, _root);

  Elm_Atspi_Object *parent = NULL;
  eo_do(data, elm_atspi_obj_parent_get(&parent));
  /* Marshall parent */
  if ((!parent) && (ATSPI_ROLE_APPLICATION == role))
    object_append_desktop_reference(iter_struct);
  else
    object_append_reference(iter_struct, parent);

  /* Marshall children  */
  Eina_List *children_list = NULL, *l;
  Elm_Atspi_Object *child;

  eo_do(data, elm_atspi_obj_children_get(&children_list));
  iter_sub_array = eldbus_message_iter_container_new(iter_struct, 'a', "(so)");
  EINA_LIST_FOREACH(children_list, l, child)
     object_append_reference(iter_sub_array, child);

  eldbus_message_iter_container_close(iter_struct, iter_sub_array);
  eina_list_free(children_list);

  /* Marshall interfaces */
  iter_sub_array = eldbus_message_iter_container_new(iter_struct, 'a', "s");

  eldbus_message_iter_basic_append(iter_sub_array, 's', ATSPI_DBUS_INTERFACE_ACCESSIBLE);
  if (eo_isa(data, ELM_ATSPI_COMPONENT_INTERFACE))
    eldbus_message_iter_basic_append(iter_sub_array, 's', ATSPI_DBUS_INTERFACE_COMPONENT);

  eldbus_message_iter_container_close(iter_struct, iter_sub_array);

  /* Marshall name */
  const char *name = NULL;
  eo_do(data, elm_atspi_obj_name_get(&name));
  if (!name)
    name = "";
  eldbus_message_iter_basic_append(iter_struct, 's', name);

  /* Marshall role */
  eldbus_message_iter_basic_append(iter_struct, 'u', role);

  /* Marshall description */
  const char* descritpion = NULL;
  eo_do(data, elm_atspi_obj_description_get(&descritpion));
  if (!descritpion)
    descritpion = "";
  eldbus_message_iter_basic_append(iter_struct, 's', descritpion);

  /* Marshall state set */
  iter_sub_array = eldbus_message_iter_container_new(iter_struct, 'a', "u");
  /* TODO: states are not implemented yet*/
  eldbus_message_iter_container_close(iter_struct, iter_sub_array);

  eldbus_message_iter_container_close(iter_array, iter_struct);

  return EINA_TRUE;
}

static Eldbus_Message *
_cache_get_items(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   Eldbus_Message_Iter *iter, *iter_array;
   iter = eldbus_message_iter_get(ret);
   iter_array = eldbus_message_iter_container_new(iter, 'a', CACHE_ITEM_SIGNATURE);
   eina_hash_foreach(_cache, _append_item_fn, iter_array);
   eldbus_message_iter_container_close(iter, iter_array);
   return ret;
}

static const Eldbus_Method cache_methods[] = {
   { "GetItems", NULL, ELDBUS_ARGS({CACHE_ITEM_SIGNATURE, "items"}), _cache_get_items, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Signal cache_signals[] = {
  [ADD_ACCESSIBLE] = { "AddAccessible", ELDBUS_ARGS({"((so)(so)a(so)assusau)", "added"}), 0},
  [REMOVE_ACCESSIBLE] = { "RemoveAccessible", ELDBUS_ARGS({ "(so)", "removed" }), 0},
  {NULL, NULL, 0}
};

static const Eldbus_Service_Interface_Desc cache_iface_desc = {
   ATSPI_DBUS_INTERFACE_CACHE, cache_methods, cache_signals, NULL, NULL, NULL
};

// Component interface
static Eldbus_Message *
_component_contains(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   int x, y;
   Eina_Bool contains;
   AtspiCoordType coord_type;
   Eldbus_Message *ret;

   if (!eldbus_message_arguments_get(msg, "iiu", &x, &y, &coord_type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   eo_do(obj, elm_atspi_component_interface_contains(x, y, coord_type, &contains));

   ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "b", contains);

   return ret;
}

static Eldbus_Message *
_component_get_accessible_at_point(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   int x, y;
   Elm_Atspi_Object *accessible;
   AtspiCoordType coord_type;
   Eldbus_Message *ret;
   Eldbus_Message_Iter *iter;

   if (!eldbus_message_arguments_get(msg, "iiu", &x, &y, &coord_type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   ret = eldbus_message_method_return_new(msg);
   iter = eldbus_message_iter_get(ret);
   eo_do(obj, elm_atspi_component_interface_accessible_at_point_get(x, y, coord_type, &accessible));
   object_append_reference(iter, accessible);

   return ret;
}

static Eldbus_Message *
_component_get_extents(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   int x, y, w, h;
   AtspiCoordType coord_type;
   Eldbus_Message *ret;
   Eldbus_Message_Iter *iter, *iter_struct;

   if (!eldbus_message_arguments_get(msg, "u", &coord_type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   ret = eldbus_message_method_return_new(msg);
   iter = eldbus_message_iter_get(ret);

   eo_do(obj, elm_atspi_component_interface_extents_get(&x, &y, &w, &h, coord_type));
   iter_struct = eldbus_message_iter_container_new(iter, 'r', NULL);
   eldbus_message_iter_basic_append(iter_struct, 'i', x);
   eldbus_message_iter_basic_append(iter_struct, 'i', y);
   eldbus_message_iter_basic_append(iter_struct, 'i', w);
   eldbus_message_iter_basic_append(iter_struct, 'i', h);
   eldbus_message_iter_container_close(iter, iter_struct);

   return ret;
}

static Eldbus_Message *
_component_get_position(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   int x, y;
   AtspiCoordType coord_type;
   Eldbus_Message *ret;

   if (!eldbus_message_arguments_get(msg, "u", &coord_type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   eo_do(obj, elm_atspi_component_interface_position_get(&x, &y, coord_type));

   ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "i", x);
   eldbus_message_arguments_append(ret, "i", y);

   return ret;
}

static Eldbus_Message *
_component_get_size(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   int x, y;
   Eldbus_Message *ret;

   eo_do(obj, elm_atspi_component_interface_size_get(&x, &y));

   ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "i", x);
   eldbus_message_arguments_append(ret, "i", y);

   return ret;
}

static Eldbus_Message *
_component_get_layer(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   int layer;
   Eldbus_Message *ret;

   eo_do(obj, elm_atspi_component_interface_layer_get(&layer));

   ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "u", layer);

   return ret;
}

static Eldbus_Message *
_component_grab_focus(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   Eldbus_Message *ret;
   Eina_Bool focus;

   eo_do(obj, elm_atspi_component_interface_focus_grab(&focus));

   ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "b", focus);

   return ret;
}

static Eldbus_Message *
_component_get_alpha(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   Eldbus_Message *ret;
   double alpha;

   eo_do(obj, elm_atspi_component_interface_alpha_get(&alpha));

   ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "d", alpha);

   return ret;
}

static Eldbus_Message *
_component_set_extends(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   int x, y, w, h;
   Eina_Bool result;
   AtspiCoordType coord_type;
   Eldbus_Message *ret;

   if (!eldbus_message_arguments_get(msg, "iiiiu", &x, &y, &w, &h, &coord_type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   eo_do(obj, elm_atspi_component_interface_extents_set(x, y, w, h, coord_type, &result));

   ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "b", result);

   return ret;
}

static Eldbus_Message *
_component_set_position(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   int x, y;
   Eina_Bool result;
   AtspiCoordType coord_type;
   Eldbus_Message *ret;

   if (!eldbus_message_arguments_get(msg, "iiu", &x, &y, &coord_type))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   eo_do(obj, elm_atspi_component_interface_position_set(x, y, coord_type, &result));

   ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "b", result);

   return ret;
}

static Eldbus_Message *
_component_set_size(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   int w, h;
   Eina_Bool result;
   Eldbus_Message *ret;

   if (!eldbus_message_arguments_get(msg, "ii", &w, &h))
     return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid index type.");

   eo_do(obj, elm_atspi_component_interface_size_set(w, h, &result));

   ret = eldbus_message_method_return_new(msg);
   eldbus_message_arguments_append(ret, "b", result);

   return ret;
}

static const Eldbus_Method component_methods[] = {
   { "Contains", ELDBUS_ARGS({"i", "x"}, {"i", "y"}, {"u", "coord_type"}), ELDBUS_ARGS({"b", "contains"}), _component_contains, 0 },
   { "GetAccessibleAtPoint", ELDBUS_ARGS({"i", "x"}, {"i", "y"}, {"u", "coord_type"}), ELDBUS_ARGS({"(so)", "accessible"}), _component_get_accessible_at_point, 0 },
   { "GetExtents", ELDBUS_ARGS({"u", "coord_type"}), ELDBUS_ARGS({"(iiii)", "extents"}), _component_get_extents, 0 },
   { "GetPosition", ELDBUS_ARGS({"u", "coord_type"}), ELDBUS_ARGS({"i", "x"}, {"i","y"}), _component_get_position, 0 },
   { "GetSize", NULL, ELDBUS_ARGS({"i", "w"}, {"i", "h"}), _component_get_size, 0 },
   { "GetLayer", NULL, ELDBUS_ARGS({"u", "layer"}), _component_get_layer, 0 },
//   { "GetMDIZOrder", NULL, ELDBUS_ARGS({"n", "MDIZOrder"}), _component_get_mdizorder, 0 },
   { "GrabFocus", NULL, ELDBUS_ARGS({"b", "focus"}), _component_grab_focus, 0 },
   { "GetAlpha", NULL, ELDBUS_ARGS({"d", "alpha"}), _component_get_alpha, 0 },
   { "SetExtents", ELDBUS_ARGS({"i", "x"}, {"i", "y"}, {"i", "width"}, {"i", "height"}, {"u", "coord_type"}), ELDBUS_ARGS({"b", "result"}), _component_set_extends, 0 },
   { "SetPosition", ELDBUS_ARGS({"i", "x"}, {"i", "y"}, {"u", "coord_type"}), ELDBUS_ARGS({"b", "result"}), _component_set_position, 0 },
   { "SetSize", ELDBUS_ARGS({"i", "width"}, {"i", "height"}), ELDBUS_ARGS({"b", "result"}), _component_set_size, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Service_Interface_Desc component_iface_desc = {
   ATSPI_DBUS_INTERFACE_COMPONENT, component_methods, NULL, NULL, NULL, NULL
};

static Eina_Bool
_atspi_object_del_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter;
   char* path;

   msg = eldbus_service_signal_new(_cache_interface, REMOVE_ACCESSIBLE);
   iter = eldbus_message_iter_get(msg);
   object_append_reference(iter, obj);
   path = _path_from_access_object(obj);

   //ERR("_atspi_object_del_cbi: %d", eo_ref_get(obj));

   eldbus_service_signal_send(_cache_interface, msg);
   eina_hash_del(_cache, path, obj);

   eldbus_service_object_unregister(data);

   free(path);
   return EO_CALLBACK_CONTINUE;
}

static void
_atspi_object_register_interfaces(const char* path, Elm_Atspi_Object *node)
{
   Eldbus_Service_Interface *accessible = NULL;

   if (eo_isa(node, ELM_ATSPI_CLASS))
     {
        accessible = eldbus_service_interface_register(_a11y_bus, path, &accessible_iface_desc);
        eo_do(node, eo_event_callback_add(EO_EV_DEL, _atspi_object_del_cb, accessible));
     }

   if (eo_isa(node, ELM_ATSPI_COMPONENT_INTERFACE))
     eldbus_service_interface_register(_a11y_bus, path, &component_iface_desc);
}

static void
_atspi_objects_register_rec(Elm_Atspi_Object *node)
{
   EINA_SAFETY_ON_NULL_RETURN(node);
   Eina_List *children_list = NULL, *l;
   Elm_Atspi_Object *child;
   char* path = _path_from_access_object(node);

   // register in cache
   eina_hash_add(_cache, path, node);

   _atspi_object_register_interfaces(path, node);
   free(path);

   eo_do(node, elm_atspi_obj_children_get(&children_list));
   EINA_LIST_FOREACH(children_list, l, child)
      _atspi_objects_register_rec(child);

   eina_list_free(children_list);
}

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

   object_append_reference(iter, _root);

   eldbus_connection_send(a11y_bus, message, _on_app_register, NULL, -1);

   return EINA_TRUE;
}

static void
_bus_objects_register(Eldbus_Connection *a11y_bus)
{
   _cache_interface = eldbus_service_interface_register(a11y_bus, CACHE_INTERFACE_PATH, &cache_iface_desc);
   _atspi_objects_register_rec(_root);
   DBG("%d elements registered in cache", eina_hash_population(_cache));
}

static void
_a11y_bus_initialize(const char *socket_addr)
{
   Eldbus_Connection *a11y_bus = eldbus_address_connection_get(socket_addr);
   _a11y_bus = a11y_bus;
   _bus_objects_register(a11y_bus);
   _app_register(a11y_bus);
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
        _cache = eina_hash_string_superfast_new(NULL);
        _root = elm_atspi_root_object_get();
        _init_count = 1;
     }
}

void
_elm_atspi_bridge_shutdown(void)
{
   if (_init_count)
     {
        eo_unref(_root);

        if (_cache_interface)
          eldbus_service_object_unregister(_cache_interface);
        _cache_interface = NULL;

        if (_a11y_bus)
          eldbus_connection_unref(_a11y_bus);
        _a11y_bus = NULL;

        if (_cache)
          eina_hash_free(_cache);
        _cache = NULL;

        _init_count = 0;
     }
}

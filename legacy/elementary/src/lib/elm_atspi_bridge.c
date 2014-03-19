#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <stdint.h>
#include <Elementary.h>
#include "elm_priv.h"
#include <assert.h>

#include "atspi/atspi-constants.h"

/*
 * Accessibility Bus info not defined in atspi-constants.h
 */
#define A11Y_DBUS_NAME "org.a11y.Bus"
#define A11Y_DBUS_PATH "/org/a11y/bus"
#define A11Y_DBUS_INTERFACE "org.a11y.Bus"
#define ATSPI_DBUS_INTERFACE_EVENT_WINDOW "org.a11y.atspi.Event.Window"

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
static Eldbus_Signal_Handler *_register_hdl;
static Eldbus_Signal_Handler *_unregister_hdl;
static unsigned short _object_property_broadcast_mask;
static unsigned short _object_children_broadcast_mask;
static unsigned long long _object_state_broadcast_mask;
static unsigned long long _window_signal_broadcast_mask;

static void _cache_update(void);
static Elm_Atspi_Object * _access_object_from_path(const char *path);
static char * _path_from_access_object(Elm_Atspi_Object *eo);
static void object_append_reference(Eldbus_Message_Iter *iter,  Elm_Atspi_Object *obj);
static void object_append_desktop_reference(Eldbus_Message_Iter *iter);
static const Eldbus_Service_Interface_Desc accessible_iface_desc;
static void _cache_object_register(Elm_Atspi_Object *node, Eina_Bool rec);


enum _Atspi_Object_Child_Event_Type
{
   ATSPI_OBJECT_CHILD_ADDED = 0,
   ATSPI_OBJECT_CHILD_REMOVED
};

enum _Atspi_Object_Property
{
   ATSPI_OBJECT_PROPERTY_NAME = 0,
   ATSPI_OBJECT_PROPERTY_DESCRIPTION,
   ATSPI_OBJECT_PROPERTY_VALUE,
   ATSPI_OBJECT_PROPERTY_ROLE,
   ATSPI_OBJECT_PROPERTY_PARENT,
};

enum _Atspi_Object_Signals {
   ATSPI_OBJECT_EVENT_PROPERTY_CHANGED,
   ATSPI_OBJECT_EVENT_BOUNDS_CHANGED,
   ATSPI_OBJECT_EVENT_LINK_SELECTED,
   ATSPI_OBJECT_EVENT_STATE_CHANGED,
   ATSPI_OBJECT_EVENT_CHILDREN_CHANGED,
   ATSPI_OBJECT_EVENT_VISIBLE_DATA_CHANGED,
   ATSPI_OBJECT_EVENT_SELECTION_CHANGED,
   ATSPI_OBJECT_EVENT_MODEL_CHANGED,
   ATSPI_OBJECT_EVENT_ACTIVE_DESCENDANT_CHANGED,
   ATSPI_OBJECT_EVENT_ROW_INSERTED,
   ATSPI_OBJECT_EVENT_ROW_REORDERED,
   ATSPI_OBJECT_EVENT_ROW_DELETED,
   ATSPI_OBJECT_EVENT_COLUMN_INSERTED,
   ATSPI_OBJECT_EVENT_COLUMN_REORDERED,
   ATSPI_OBJECT_EVENT_COLUMN_DELETED,
   ATSPI_OBJECT_EVENT_TEXT_BOUNDS_CHANGED,
   ATSPI_OBJECT_EVENT_TEXT_SELECTION_CHANGED,
   ATSPI_OBJECT_EVENT_TEXT_CHANGED,
   ATSPI_OBJECT_EVENT_TEXT_ATTRIBUTES_CHANGED,
   ATSPI_OBJECT_EVENT_TEXT_CARET_MOVED,
   ATSPI_OBJECT_EVENT_ATTRIBUTES_CHANGED,
};

enum _Atspi_Window_Signals
{
   ATSPI_WINDOW_EVENT_PROPERTY_CHANGE,
   ATSPI_WINDOW_EVENT_MINIMIZE,
   ATSPI_WINDOW_EVENT_MAXIMIZE,
   ATSPI_WINDOW_EVENT_RESTORE,
   ATSPI_WINDOW_EVENT_CLOSE,
   ATSPI_WINDOW_EVENT_CREATE,
   ATSPI_WINDOW_EVENT_REPARENT,
   ATSPI_WINDOW_EVENT_DESKTOPCREATE,
   ATSPI_WINDOW_EVENT_DESKTOPDESTROY,
   ATSPI_WINDOW_EVENT_DESTROY,
   ATSPI_WINDOW_EVENT_ACTIVATE,
   ATSPI_WINDOW_EVENT_DEACTIVATE,
   ATSPI_WINDOW_EVENT_RAISE,
   ATSPI_WINDOW_EVENT_LOWER,
   ATSPI_WINDOW_EVENT_MOVE,
   ATSPI_WINDOW_EVENT_RESIZE,
   ATSPI_WINDOW_EVENT_SHADE,
   ATSPI_WINDOW_EVENT_UUSHADE,
   ATSPI_WINDOW_EVENT_RESTYLE,
};

static const Eldbus_Signal _event_obj_signals[] = {
   [ATSPI_OBJECT_EVENT_PROPERTY_CHANGED] = {"PropertyChange", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_BOUNDS_CHANGED] = {"BoundsChange", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_LINK_SELECTED] = {"LinkSelected", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_STATE_CHANGED] = {"StateChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_CHILDREN_CHANGED] = {"ChildrenChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_VISIBLE_DATA_CHANGED] = {"VisibleDataChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_SELECTION_CHANGED] = {"SelectionChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_MODEL_CHANGED] = {"ModelChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_ACTIVE_DESCENDANT_CHANGED] = {"ActiveDescendantsChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_ROW_INSERTED] = {"RowInserted", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_ROW_REORDERED] = {"RowReordered", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_ROW_DELETED] = {"RowDeleted", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_COLUMN_INSERTED] = {"ColumnInserted", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_COLUMN_REORDERED] = {"ColumnReordered", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_COLUMN_DELETED] = {"ColumnDeleted", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_TEXT_BOUNDS_CHANGED] = {"TextBoundsChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_TEXT_SELECTION_CHANGED] = {"SelectionChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_TEXT_CHANGED] = {"TextChaged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_TEXT_ATTRIBUTES_CHANGED] = {"TextAttributesChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_TEXT_CARET_MOVED] = {"TextCaretMoved", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_OBJECT_EVENT_ATTRIBUTES_CHANGED] = {"AttributesChanged", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
};

static const Eldbus_Signal _window_obj_signals[] = {
   [ATSPI_WINDOW_EVENT_PROPERTY_CHANGE] = {"PropertyChange", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_MINIMIZE] = {"Minimize", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_MAXIMIZE] = {"Maximize", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_RESTORE] = {"Restore", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_CLOSE] = {"Close", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_CREATE] = {"Create", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_REPARENT] = {"Reparent", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_DESKTOPCREATE] = {"DesktopCreate", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_DESKTOPDESTROY] = {"DesktopDestroy", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_DESTROY] = {"Destroy", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_ACTIVATE] = {"Activate", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_DEACTIVATE] = {"Deactivate", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_RAISE] = {"Raise", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_LOWER] = {"Lower", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_MOVE] = {"Move", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_RESIZE] = {"Resize", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_SHADE] = {"Shade", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_UUSHADE] = {"uUshade", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
   [ATSPI_WINDOW_EVENT_RESTYLE] = {"Restyle", ELDBUS_ARGS({"siiv(so)", NULL}), 0},
};

static Eldbus_Message *
_accessible_get_role(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   AtspiRole role;

   eo_do(obj, elm_atspi_obj_role_get(&role));
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);
   eldbus_message_arguments_append(ret, "u", role);
   return ret;
}

static Eldbus_Message *
_accessible_get_role_name(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   const char *role_name, *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);

   eo_do(obj, elm_atspi_obj_role_name_get(&role_name));
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);
   eldbus_message_arguments_append(ret, "s", role_name);

   return ret;
}

static Eldbus_Message *
_accessible_get_localized_role_name(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   const char *l_role_name, *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);

   eo_do(obj, elm_atspi_obj_localized_role_name_get(&l_role_name));
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);
   eldbus_message_arguments_append(ret, "s", l_role_name);

   return ret;
}

static Eldbus_Message *
_accessible_get_children(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   Eina_List *children_list = NULL, *l;
   Eldbus_Message *ret;

   Eldbus_Message_Iter *iter, *iter_array;
   Elm_Atspi_Object *children;

   eo_do(obj, elm_atspi_obj_children_get(&children_list));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   iter_array = eldbus_message_iter_container_new(iter, 'a', "(so)");
   EINA_SAFETY_ON_NULL_GOTO(iter_array, fail);

   EINA_LIST_FOREACH(children_list, l, children)
      object_append_reference(iter_array, children);

   eldbus_message_iter_container_close(iter, iter_array);
   eina_list_free(children_list);

   return ret;

fail:
   if (ret) eldbus_message_unref(ret);
   return NULL;
}

static Eldbus_Message *
_accessible_get_application(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);
   Eldbus_Message_Iter *iter = eldbus_message_iter_get(ret);
   object_append_reference(iter, _root);

   return ret;
}

static Eldbus_Message *
_accessible_get_state(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eldbus_Message *ret;
   Eldbus_Message_Iter *iter, *iter_array;
   Elm_Atspi_State states;

   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);

   if (!obj)
     {
        ERR("Atspi Object %s not found in cache!", obj_path);
        return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.InvalidArgs", "Invalid object path.");
     }

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   iter_array = eldbus_message_iter_container_new(iter, 'a', "u");
   EINA_SAFETY_ON_NULL_GOTO(iter_array, fail);

   eo_do(obj, elm_atspi_obj_state_get(&states));

   unsigned int s1 = states & 0xFFFFFFFF;
   unsigned int s2 = (states >> 32) & 0xFFFFFFFF;

   eldbus_message_iter_basic_append(iter_array, 'u', s1);
   eldbus_message_iter_basic_append(iter_array, 'u', s2);
   eldbus_message_iter_container_close(iter, iter_array);

   return ret;

fail:
   if (ret) eldbus_message_unref(ret);
   return NULL;
}

static Eldbus_Message *
_accessible_get_index_in_parent(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *obj_path = eldbus_service_object_path_get(iface);
   Elm_Atspi_Object *obj = _access_object_from_path(obj_path);
   Eldbus_Message *ret;
   int idx;

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

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
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   eo_do(obj, elm_atspi_obj_child_at_index_get(idx, &child));
   object_append_reference(iter, child);

   return ret;
}

static Eldbus_Message *
_accessible_get_relation_set(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.NotSupported", "Relation states not implemented.");
}

static const Eldbus_Method accessible_methods[] = {
   { "GetChildAtIndex", ELDBUS_ARGS({"i", "index"}), ELDBUS_ARGS({"(so)", "Accessible"}), _accessible_child_at_index, 0 },
   { "GetChildren", NULL, ELDBUS_ARGS({"a(so)", "children"}), _accessible_get_children, 0 },
   { "GetIndexInParent", NULL, ELDBUS_ARGS({"i", "index"}), _accessible_get_index_in_parent, 0 },
   { "GetRelationSet", NULL, ELDBUS_ARGS({"a(ua(so))", NULL}), _accessible_get_relation_set, 0 },
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

   int len = strlen(ELM_ACCESS_OBJECT_PATH_PREFIX);

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
   { "Name", "s", _accessible_property_get, NULL, 0 },
   { "Description", "s", _accessible_property_get, NULL, 0 },
   { "Parent", "(so)", _accessible_property_get, NULL, 0 },
   { "ChildCount", "i", _accessible_property_get, NULL, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Service_Interface_Desc accessible_iface_desc = {
   ATSPI_DBUS_INTERFACE_ACCESSIBLE, accessible_methods, NULL, accessible_properties, _accessible_property_get, NULL
};

static const Eldbus_Service_Interface_Desc event_iface_desc = {
   ATSPI_DBUS_INTERFACE_EVENT_OBJECT, NULL, _event_obj_signals, NULL, NULL, NULL
};

static const Eldbus_Service_Interface_Desc window_iface_desc = {
   ATSPI_DBUS_INTERFACE_EVENT_WINDOW, NULL, _window_obj_signals, NULL, NULL, NULL
};

static void
object_append_reference(Eldbus_Message_Iter *iter, Elm_Atspi_Object *obj)
{
  Eldbus_Message_Iter *iter_struct = eldbus_message_iter_container_new(iter, 'r', NULL);
  EINA_SAFETY_ON_NULL_RETURN(iter);
  char *path = _path_from_access_object(obj);
  eldbus_message_iter_basic_append(iter_struct, 's', eldbus_connection_unique_name_get(_a11y_bus));
  eldbus_message_iter_basic_append(iter_struct, 'o', path);
  eldbus_message_iter_container_close(iter, iter_struct);
  free(path);
}

static void
object_append_desktop_reference(Eldbus_Message_Iter *iter)
{
  Eldbus_Message_Iter *iter_struct = eldbus_message_iter_container_new(iter, 'r', NULL);
  EINA_SAFETY_ON_NULL_RETURN(iter);

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
  Elm_Atspi_State states;
  AtspiRole role;

  eo_do(data, elm_atspi_obj_role_get(&role));

  iter_struct = eldbus_message_iter_container_new(iter_array, 'r', NULL);
  EINA_SAFETY_ON_NULL_RETURN_VAL(iter_struct, EINA_TRUE);

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
  EINA_SAFETY_ON_NULL_GOTO(iter_sub_array, fail);

  EINA_LIST_FOREACH(children_list, l, child)
     object_append_reference(iter_sub_array, child);

  eldbus_message_iter_container_close(iter_struct, iter_sub_array);
  eina_list_free(children_list);

  /* Marshall interfaces */
  iter_sub_array = eldbus_message_iter_container_new(iter_struct, 'a', "s");
  EINA_SAFETY_ON_NULL_GOTO(iter_sub_array, fail);

  eldbus_message_iter_basic_append(iter_sub_array, 's', ATSPI_DBUS_INTERFACE_ACCESSIBLE);
  if (eo_isa(data, ELM_INTERFACE_ATSPI_COMPONENT_CLASS))
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
  EINA_SAFETY_ON_NULL_GOTO(iter_sub_array, fail);

  eo_do(data, elm_atspi_obj_state_get(&states));
  unsigned int s1 = states & 0xFFFFFFFF;
  unsigned int s2 = (states >> 32) & 0xFFFFFFFF;
  eldbus_message_iter_basic_append(iter_sub_array, 'u', s1);
  eldbus_message_iter_basic_append(iter_sub_array, 'u', s2);

  eldbus_message_iter_container_close(iter_struct, iter_sub_array);
  eldbus_message_iter_container_close(iter_array, iter_struct);

  return EINA_TRUE;

fail:
  if (iter_struct) eldbus_message_iter_del(iter_struct);
  return EINA_TRUE;
}

static Eldbus_Message *
_cache_get_items(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg)
{
   Eldbus_Message_Iter *iter, *iter_array;
   Eldbus_Message *ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   _cache_update();

   iter = eldbus_message_iter_get(ret);
   iter_array = eldbus_message_iter_container_new(iter, 'a', CACHE_ITEM_SIGNATURE);
   EINA_SAFETY_ON_NULL_GOTO(iter_array, fail);

   eina_hash_foreach(_cache, _append_item_fn, iter_array);
   eldbus_message_iter_container_close(iter, iter_array);

   return ret;
fail:
   if (ret) eldbus_message_unref(ret);
   return NULL;
}

static const Eldbus_Method cache_methods[] = {
   { "GetItems", NULL, ELDBUS_ARGS({CACHE_ITEM_SIGNATURE, "items"}), _cache_get_items, 0 },
   { NULL, NULL, NULL, NULL, 0 }
};

static const Eldbus_Signal cache_signals[] = {
  [ATSPI_OBJECT_CHILD_ADDED] = { "AddAccessible", ELDBUS_ARGS({"((so)(so)a(so)assusau)", "added"}), 0},
  [ATSPI_OBJECT_CHILD_REMOVED] = { "RemoveAccessible", ELDBUS_ARGS({ "(so)", "removed" }), 0},
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

   eo_do(obj, elm_interface_atspi_component_contains(x, y, coord_type, &contains));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

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
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   eo_do(obj, elm_interface_atspi_component_accessible_at_point_get(x, y, coord_type, &accessible));
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
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);

   eo_do(obj, elm_interface_atspi_component_extents_get(&x, &y, &w, &h, coord_type));
   iter_struct = eldbus_message_iter_container_new(iter, 'r', NULL);
   EINA_SAFETY_ON_NULL_GOTO(iter_struct, fail);

   eldbus_message_iter_basic_append(iter_struct, 'i', x);
   eldbus_message_iter_basic_append(iter_struct, 'i', y);
   eldbus_message_iter_basic_append(iter_struct, 'i', w);
   eldbus_message_iter_basic_append(iter_struct, 'i', h);

   eldbus_message_iter_container_close(iter, iter_struct);

   return ret;
fail:
   if (ret) eldbus_message_unref(ret);
   return NULL;
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

   eo_do(obj, elm_interface_atspi_component_position_get(&x, &y, coord_type));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

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

   eo_do(obj, elm_interface_atspi_component_size_get(&x, &y));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

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

   eo_do(obj, elm_interface_atspi_component_layer_get(&layer));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

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

   eo_do(obj, elm_interface_atspi_component_focus_grab(&focus));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

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

   eo_do(obj, elm_interface_atspi_component_alpha_get(&alpha));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

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

   eo_do(obj, elm_interface_atspi_component_extents_set(x, y, w, h, coord_type, &result));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

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

   eo_do(obj, elm_interface_atspi_component_position_set(x, y, coord_type, &result));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

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

   eo_do(obj, elm_interface_atspi_component_size_set(w, h, &result));

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

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
_cache_object_del_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter;
   char* path;

   msg = eldbus_service_signal_new(_cache_interface, ATSPI_OBJECT_CHILD_REMOVED);
   iter = eldbus_message_iter_get(msg);
   object_append_reference(iter, obj);
   path = _path_from_access_object(obj);

   eldbus_service_signal_send(_cache_interface, msg);
   eina_hash_del(_cache, path, obj);

   eldbus_service_object_unregister(data);

   free(path);
   return EO_CALLBACK_CONTINUE;
}

static void
_cache_object_register_interfaces(const char *path, Elm_Atspi_Object *node)
{
   Eldbus_Service_Interface *accessible = NULL;
   Eldbus_Service_Interface *events = NULL;
   Eldbus_Service_Interface *window = NULL;

   if (eina_hash_find(_cache, path))
     return;
   else
     eina_hash_add(_cache, path, node);

   if (!eo_isa(node, ELM_ATSPI_CLASS)) return;

   accessible = eldbus_service_interface_register(_a11y_bus, path, &accessible_iface_desc);
   events = eldbus_service_interface_register(_a11y_bus, path, &event_iface_desc);
   eo_do(node, eo_base_data_set("atspi_event_interface", events, NULL));
   eo_do(node, eo_event_callback_add(EO_EV_DEL, _cache_object_del_cb, accessible));

   if (eo_isa(node, ELM_INTERFACE_ATSPI_COMPONENT_CLASS))
     eldbus_service_interface_register(_a11y_bus, path, &component_iface_desc);

   if (eo_isa(node, ELM_ATSPI_WINDOW_INTERFACE))
     {
        window = eldbus_service_interface_register(_a11y_bus, path, &window_iface_desc);
        eo_do(node, eo_base_data_set("window_event_interface", window, NULL));
     }
}

static void
_cache_object_register(Elm_Atspi_Object *node, Eina_Bool rec)
{
   EINA_SAFETY_ON_NULL_RETURN(node);
   Eina_List *children_list = NULL, *l;
   Elm_Atspi_Object *child;
   char* path = _path_from_access_object(node);

   _cache_object_register_interfaces(path, node);
   free(path);

   if (!rec) return;

   eo_do(node, elm_atspi_obj_children_get(&children_list));
   EINA_LIST_FOREACH(children_list, l, child)
      _cache_object_register(child, rec);

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
_app_register(void)
{
   Eldbus_Message *message = eldbus_message_method_call_new(ATSPI_DBUS_NAME_REGISTRY,
                                    ATSPI_DBUS_PATH_ROOT,
                                    ATSPI_DBUS_INTERFACE_SOCKET,
                                    "Embed");
   Eldbus_Message_Iter *iter = eldbus_message_iter_get(message);

   object_append_reference(iter, _root);
   eldbus_connection_send(_a11y_bus, message, _on_app_register, NULL, -1);

   return EINA_TRUE;
}

static void
_cache_update(void)
{
   _cache_object_register(_root, EINA_TRUE);
}

static void
_bus_objects_register(void)
{
   _cache_interface = eldbus_service_interface_register(_a11y_bus, CACHE_INTERFACE_PATH, &cache_iface_desc);
}

static void
_set_broadcast_flag(const char *event)
{
   char **tokens;

   tokens = eina_str_split(event, ":", 3);

   if (!tokens) return;

   if (!strcmp(tokens[0], "Object"))
     {
        if (!tokens[1] || *tokens[1] == '\0') return; // do not handle "Object:*"
        else if (!strcmp(tokens[1], "StateChanged"))
          {
             if (!tokens[2] || *tokens[2] == '\0')
               _object_state_broadcast_mask = -1; // broadcast all
             else if (!strcmp(tokens[2], "Focused"))
               BIT_FLAG_SET(_object_state_broadcast_mask, ATSPI_STATE_FOCUSED);
             else if (!strcmp(tokens[2], "Showing"))
               BIT_FLAG_SET(_object_state_broadcast_mask, ATSPI_STATE_SHOWING);
          }
        else if (!strcmp(tokens[1], "PropertyChange"))
          {
             if (!tokens[2] || *tokens[2] == '\0')
               _object_property_broadcast_mask = -1; //broadcast all
             else if (!strcmp(tokens[2], "AccessibleValue"))
               BIT_FLAG_SET(_object_property_broadcast_mask, ATSPI_OBJECT_PROPERTY_VALUE);
             else if (!strcmp(tokens[2], "AccessibleName"))
               BIT_FLAG_SET(_object_property_broadcast_mask, ATSPI_OBJECT_PROPERTY_NAME);
             else if (!strcmp(tokens[2], "AccessibleDescription"))
               BIT_FLAG_SET(_object_property_broadcast_mask, ATSPI_OBJECT_PROPERTY_DESCRIPTION);
             else if (!strcmp(tokens[2], "AccessibleParent"))
               BIT_FLAG_SET(_object_property_broadcast_mask, ATSPI_OBJECT_PROPERTY_PARENT);
             else if (!strcmp(tokens[2], "AccessibleRole"))
               BIT_FLAG_SET(_object_property_broadcast_mask, ATSPI_OBJECT_PROPERTY_ROLE);
          }
        else if (!strcmp(tokens[1], "ChildrenChanged"))
          {
             if (!tokens[2] || *tokens[2] == '\0')
               _object_children_broadcast_mask = -1; // broadcast all
             else if (!strcmp(tokens[2], "add"))
               BIT_FLAG_SET(_object_children_broadcast_mask, ATSPI_OBJECT_CHILD_ADDED);
             else if (!strcmp(tokens[2], "remove"))
               BIT_FLAG_SET(_object_children_broadcast_mask, ATSPI_OBJECT_CHILD_ADDED);
          }
     }
   else if (!strcmp(tokens[0], "Window"))
     {
        if (!tokens[1] || *tokens[1] == '\0')
          _window_signal_broadcast_mask = -1; // broadcast all
        else if (!strcmp(tokens[1], "Create"))
          BIT_FLAG_SET(_window_signal_broadcast_mask, ATSPI_WINDOW_EVENT_CREATE);
        else if (!strcmp(tokens[1], "Activate"))
          BIT_FLAG_SET(_window_signal_broadcast_mask, ATSPI_WINDOW_EVENT_ACTIVATE);
        else if (!strcmp(tokens[1], "Deactivate"))
          BIT_FLAG_SET(_window_signal_broadcast_mask, ATSPI_WINDOW_EVENT_DEACTIVATE);
     }

   free(tokens[0]);
   free(tokens);
}

static void
_registered_listeners_get(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   DBG("Updating ATSPI2 clients registered events list.");
   _object_children_broadcast_mask = 0;
   _object_property_broadcast_mask = 0;
   _object_state_broadcast_mask = 0;
   _window_signal_broadcast_mask = 0;

   const char *event, *bus;
   Eldbus_Message_Iter *iter, *siter;
   if (!eldbus_message_arguments_get(msg, "a(ss)", &iter))
     {
        ERR("Invalid answer type from GetRegisteredEvents method call!");
        return;
     }
   while (eldbus_message_iter_get_and_next(iter, 'r', &siter))
     {
        eldbus_message_iter_arguments_get(siter, "ss", &bus, &event);
        _set_broadcast_flag(event);
     }
}

static void
_registered_events_list_update(void)
{
   Eldbus_Message *msg;
   msg = eldbus_message_method_call_new(ATSPI_DBUS_NAME_REGISTRY, ATSPI_DBUS_PATH_REGISTRY, ATSPI_DBUS_INTERFACE_REGISTRY, "GetRegisteredEvents");
   eldbus_connection_send(_a11y_bus, msg, _registered_listeners_get, NULL, -1);
}

static void
_handle_listener_change(void *data EINA_UNUSED, const Eldbus_Message *msg)
{
   const char *bus, *event;
   if (!eldbus_message_arguments_get(msg, "ss", &bus, &event))
     {
        ERR("Invalid org.a11y.Registry signal message args.");
        return;
     }
   _registered_events_list_update();
}

static void
_send_signal_state_changed(Elm_Atspi_Object *obj, AtspiStateType type, Eina_Bool new_value)
{
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *viter;
   Eldbus_Service_Interface *events = NULL;
   char *desc;

   if (!BIT_FLAG_GET(_object_state_broadcast_mask, type))
     {
        DBG("Masking event: %d", type);
        return;
     }

   switch (type) {
        case ATSPI_STATE_FOCUSED:
         desc = "focused";
         break;
        case ATSPI_STATE_SHOWING:
         desc = "showing";
         break;
        case ATSPI_STATE_VISIBLE:
         desc = "visible";
         break;
        default:
         desc = NULL;
   }

   eo_do(obj, eo_base_data_get("atspi_event_interface", (void **)&events));
   if (!events)
     {
        ERR("Atspi object does not have event interface!");
        return;
     }
   msg = eldbus_service_signal_new(events, ATSPI_OBJECT_EVENT_STATE_CHANGED);
   iter = eldbus_message_iter_get(msg);

   eldbus_message_iter_arguments_append(iter, "sii", desc, new_value, 0);

   viter = eldbus_message_iter_container_new(iter, 'v', "i");
   EINA_SAFETY_ON_NULL_RETURN(viter);

   eldbus_message_iter_arguments_append(viter, "i", 0);
   eldbus_message_iter_container_close(iter, viter);

   object_append_reference(iter, obj);

   eldbus_service_signal_send(events, msg);
   DBG("signal sent StateChanged:%s:%d", desc, new_value);
}

static void
_send_signal_property_changed(Elm_Atspi_Object *ao, enum _Atspi_Object_Property prop)
{
   const char *desc;
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *siter, *viter;
   Eldbus_Service_Interface *events = NULL;

   if (!BIT_FLAG_GET(_object_property_broadcast_mask, prop))
     return;

   switch(prop)
    {
     case ATSPI_OBJECT_PROPERTY_NAME:
        desc = "accessible-name";
        break;
     case ATSPI_OBJECT_PROPERTY_DESCRIPTION:
        desc = "accessible-description";
        break;
     case ATSPI_OBJECT_PROPERTY_ROLE:
        desc = "accessible-role";
        break;
     case ATSPI_OBJECT_PROPERTY_PARENT:
        desc = "accessible-parent";
        break;
     case ATSPI_OBJECT_PROPERTY_VALUE:
        desc = "accessible-value";
        break;
     default:
        desc = NULL;
    }
   eo_do(ao, eo_base_data_get("atspi_event_interface", (void**)&events));
   if (!events)
     {
        ERR("Atspi object does not have event interface!");
        return;
     }
   msg = eldbus_service_signal_new(events, ATSPI_OBJECT_EVENT_PROPERTY_CHANGED);
   EINA_SAFETY_ON_NULL_RETURN(msg);

   iter = eldbus_message_iter_get(msg);
   siter = eldbus_message_iter_container_new(iter, 'r', NULL);
   EINA_SAFETY_ON_NULL_RETURN(siter);

   eldbus_message_iter_arguments_append(siter, "suu", desc, 0, 0);

   viter = eldbus_message_iter_container_new(siter, 'v', "s");
   EINA_SAFETY_ON_NULL_RETURN(viter);

   eldbus_message_iter_arguments_append(viter, "s", _path_from_access_object(ao));

   eldbus_message_iter_arguments_append(siter, "v", viter);
   eldbus_message_iter_container_close(siter, viter);

   eldbus_message_iter_container_close(iter, siter);
   eldbus_service_signal_send(events, msg);
   DBG("signal sent PropertyChanged:%s", desc);
}

static void
_send_signal_children_changed(Elm_Atspi_Object *parent, Elm_Atspi_Object *child, enum _Atspi_Object_Child_Event_Type type)
{
   Eldbus_Service_Interface *events = NULL;
   Eldbus_Message_Iter *iter, *viter;
   Eldbus_Message *msg;
   const char *desc = NULL;
   int idx;

   if (!BIT_FLAG_GET(_object_children_broadcast_mask, type))
     return;

   _cache_object_register(parent, EINA_FALSE);
   _cache_object_register(child, EINA_FALSE);

   eo_do(parent, eo_base_data_get("atspi_event_interface", (void **)&events));
   if (!events)
     {
        ERR("Atspi object does not have event interface! %p %p %s", parent, _root, eo_class_name_get(eo_class_get(parent)));
        return;
     }

   switch(type)
    {
     case ATSPI_OBJECT_CHILD_ADDED:
        desc = "add";
        eo_do(child, elm_atspi_obj_index_in_parent_get(&idx));
        break;
     case ATSPI_OBJECT_CHILD_REMOVED:
        desc = "remove";
        idx = -1;
        break;
    }
   msg = eldbus_service_signal_new(events, ATSPI_OBJECT_EVENT_CHILDREN_CHANGED);
   EINA_SAFETY_ON_NULL_RETURN(msg);

   iter = eldbus_message_iter_get(msg);
   eldbus_message_iter_arguments_append(iter, "sii", desc, idx, 0);

   viter = eldbus_message_iter_container_new(iter, 'v', "(so)");
   EINA_SAFETY_ON_NULL_RETURN(viter);

   object_append_reference(viter, child);
   eldbus_message_iter_container_close(iter, viter);

   object_append_reference(iter, _root);

   eldbus_service_signal_send(events, msg);
   DBG("signal sent childrenChanged:%s:%d", desc, idx);
}

static void
_send_signal_window(Elm_Atspi_Object *eo, enum _Atspi_Window_Signals type)
{
   const char *desc;
   Eldbus_Message *msg;
   Eldbus_Message_Iter *iter, *viter;
   Eldbus_Service_Interface *window = NULL;

   if (!BIT_FLAG_GET(_window_signal_broadcast_mask, type))
     return;

   eo_do(eo, eo_base_data_get("window_event_interface", (void**)&window));
   if (!window)
     {
        ERR("Atspi object does not have window interface!");
        return;
     }

   switch(type)
     {
      case ATSPI_WINDOW_EVENT_DEACTIVATE:
         desc = "Deactivate";
         break;
      case ATSPI_WINDOW_EVENT_ACTIVATE:
         desc = "Activate";
         break;
      default:
         desc = "";
     }

   msg = eldbus_service_signal_new(window, type);
   EINA_SAFETY_ON_NULL_RETURN(msg);

   iter = eldbus_message_iter_get(msg);
   eldbus_message_iter_arguments_append(iter, "sii", desc, 0, 0);

   viter = eldbus_message_iter_container_new(iter, 'v', "i");
   EINA_SAFETY_ON_NULL_RETURN(viter);

   eldbus_message_iter_arguments_append(viter, "i", 0);
   eldbus_message_iter_container_close(iter, viter);

   object_append_reference(iter, eo);

   eldbus_service_signal_send(window, msg);
   DBG("signal sent Window:%s", desc);
}

static Eina_Bool
_handle_atspi_event(void *data EINA_UNUSED, Elm_Atspi_Object *ao, const Eo_Event_Description *desc, void *event_info)
{
   if (desc == EV_ATSPI_OBJ_NAME_CHANGED)
     _send_signal_property_changed(ao, ATSPI_OBJECT_PROPERTY_NAME);
   else if (desc == EV_ATSPI_OBJ_STATE_CHANGED)
     {
        int *event_data = event_info;
        _send_signal_state_changed(ao, (AtspiStateType)event_data[0], (Eina_Bool)event_data[1]);
     }
   else if (desc == EV_ATSPI_OBJ_CHILD_ADD)
     _send_signal_children_changed(ao, event_info, ATSPI_OBJECT_CHILD_ADDED);
   else if (desc == EV_ATSPI_OBJ_CHILD_DEL)
     _send_signal_children_changed(ao, event_info, ATSPI_OBJECT_CHILD_REMOVED);
   else if (desc == EV_ATSPI_OBJ_WINDOW_ACTIVATED)
     _send_signal_window(ao, ATSPI_WINDOW_EVENT_ACTIVATE);
   else if (desc == EV_ATSPI_OBJ_WINDOW_DEACTIVATED)
     _send_signal_window(ao, ATSPI_WINDOW_EVENT_DEACTIVATE);

   return EINA_TRUE;
}

static void
_event_handlers_register(void)
{
   _registered_events_list_update();

   // register signal handlers in order to update list of registered listeners of ATSPI-Clients
   _register_hdl = eldbus_signal_handler_add(_a11y_bus, ATSPI_DBUS_NAME_REGISTRY, ATSPI_DBUS_PATH_REGISTRY, ATSPI_DBUS_INTERFACE_REGISTRY, "EventListenerRegistered", _handle_listener_change, NULL);
   _unregister_hdl = eldbus_signal_handler_add(_a11y_bus, ATSPI_DBUS_NAME_REGISTRY, ATSPI_DBUS_PATH_REGISTRY, ATSPI_DBUS_INTERFACE_REGISTRY, "EventListenerDeregistered", _handle_listener_change, NULL);

   // handle incoming events from Elm_Atspi_Objects
   _elm_atspi_object_global_callback_add(_handle_atspi_event, NULL);
}

static void
_a11y_bus_initialize(const char *socket_addr)
{
   _a11y_bus = eldbus_address_connection_get(socket_addr);

   _cache_update();
   _bus_objects_register();
   _app_register();
   _event_handlers_register();
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
        _elm_atspi_object_init();
        session_bus = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
		EINA_SAFETY_ON_NULL_RETURN(session_bus);
        msg = eldbus_message_method_call_new(A11Y_DBUS_NAME, A11Y_DBUS_PATH, A11Y_DBUS_INTERFACE, "GetAddress");
		if (!msg)
		  {
			 eldbus_connection_unref(session_bus);
			 return;
		  }
        eldbus_connection_send(session_bus, msg, _a11y_bus_address_get, session_bus, -1);
        _cache = eina_hash_string_superfast_new(NULL);
        _root = _elm_atspi_root_object_get();
        _init_count = 1;
     }
}

void
_elm_atspi_bridge_shutdown(void)
{
   if (_init_count)
     {
        _elm_atspi_object_shutdown();

        if (_register_hdl)
          eldbus_signal_handler_del(_register_hdl);
        _register_hdl = NULL;

        if (_unregister_hdl)
          eldbus_signal_handler_del(_unregister_hdl);
        _unregister_hdl = NULL;

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

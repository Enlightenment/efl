#include <Elementary.h>
#include <stdint.h>
#include "elm_priv.h"
#include "elm_widget_menu.h"
#include "elm_widget_icon.h"

#ifdef ELM_EDBUS2

#define DBUS_PATH           "/com/canonical/dbusmenu"
#define DBUS_INTERFACE      "com.canonical.dbusmenu"
#define DBUS_MENU_VERSION   3u

#define REGISTRAR_NAME      "com.canonical.AppMenu.Registrar"
#define REGISTRAR_PATH      "/com/canonical/AppMenu/Registrar"
#define REGISTRAR_INTERFACE REGISTRAR_NAME

#define DBUS_DATA_KEY       "_Elm_DBus_Menu"
#endif

struct _Elm_DBus_Menu
{
#ifdef ELM_EDBUS2
   Eo                      *menu;
   EDBus_Connection        *bus;
   EDBus_Service_Interface *iface;
   unsigned                 timestamp;
   Eina_Hash               *elements;
   Ecore_Idler             *signal_idler;
   Ecore_X_Window           xid;
#endif
};

#ifdef ELM_EDBUS2
static const EDBus_Service_Interface_Desc _interface;
static unsigned last_object_path;

typedef enum _Elm_DBus_Property
{
   ELM_DBUS_PROPERTY_LABEL,
   ELM_DBUS_PROPERTY_CHILDREN_DISPLAY,
   ELM_DBUS_PROPERTY_ENABLED,
   ELM_DBUS_PROPERTY_TYPE,
   ELM_DBUS_PROPERTY_ICON_NAME,
   ELM_DBUS_PROPERTY_UNKNOWN,
} Elm_DBus_Property;

enum
{
   ELM_DBUS_SIGNAL_LAYOUT_UPDATED,
   ELM_DBUS_SIGNAL_ITEM_ACTIVATION_REQUESTED,
};

typedef struct _Callback_Data {
  void (*result_cb)(Eina_Bool, void *);
  void *data;
} Callback_Data;

static Eina_Bool
_menu_add_recursive(Elm_DBus_Menu *dbus_menu, Elm_Menu_Item *item)
{
   int32_t id;
   Eina_List *l;
   Elm_Menu_Item *subitem;

   id = ++dbus_menu->timestamp;
   if (!eina_hash_add(dbus_menu->elements, &id, item))
     return EINA_FALSE;

   item->dbus_idx = id;

   EINA_LIST_FOREACH (item->submenu.items, l, subitem)
     {
        if (!_menu_add_recursive(dbus_menu, subitem))
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_app_register_cb(void *data, const EDBus_Message *msg,
		 EDBus_Pending *pending EINA_UNUSED)
{
  Callback_Data *cd = data;

  cd->result_cb(!edbus_message_error_get(msg, NULL, NULL), cd->data);
  free(cd);
}

static Eina_Bool
_layout_idler(void *data)
{
   Elm_DBus_Menu *dbus_menu = data;

   edbus_service_signal_emit(dbus_menu->iface, ELM_DBUS_SIGNAL_LAYOUT_UPDATED,
                             dbus_menu->timestamp, 0);

   dbus_menu->signal_idler = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_layout_signal(Elm_DBus_Menu *dbus_menu)
{
   if (!dbus_menu->bus) return;
   if (dbus_menu->signal_idler) return;

   dbus_menu->signal_idler = ecore_idler_add(_layout_idler, dbus_menu);
}

static Elm_DBus_Property
_str_to_property(const char *str)
{
   if (!strcmp(str, "label"))
     return ELM_DBUS_PROPERTY_LABEL;
   else if (!strcmp(str, "children-display"))
     return ELM_DBUS_PROPERTY_CHILDREN_DISPLAY;
   else if (!strcmp(str, "enabled"))
     return ELM_DBUS_PROPERTY_ENABLED;
   else if (!strcmp(str, "type"))
     return ELM_DBUS_PROPERTY_TYPE;
   else if (!strcmp(str, "icon-name"))
     return ELM_DBUS_PROPERTY_ICON_NAME;

   return ELM_DBUS_PROPERTY_UNKNOWN;
}

static Eina_Bool
_freedesktop_icon_exists(Elm_Menu_Item *item)
{
   if (!item->icon_str) return EINA_FALSE;

#ifdef ELM_EFREET
   ELM_ICON_CHECK(item->content) EINA_FALSE;

   ELM_ICON_DATA_GET(item->content, sd);
   if (sd->freedesktop.use) return EINA_TRUE;
#endif

   return EINA_FALSE;
}

static Eina_Bool
_property_exists(Elm_Menu_Item *item,
                 Elm_DBus_Property property)
{
   Elm_Object_Item *item_obj;

   if (item->separator)
     {
        if (property == ELM_DBUS_PROPERTY_TYPE) return EINA_TRUE;
        return EINA_FALSE;
     }

   switch (property)
     {
      case ELM_DBUS_PROPERTY_LABEL:
        // Allow _property_append to handle the label
        return EINA_TRUE;

      case ELM_DBUS_PROPERTY_CHILDREN_DISPLAY:
        if (eina_list_count(item->submenu.items)) return EINA_TRUE;
        return EINA_FALSE;

      case ELM_DBUS_PROPERTY_ENABLED:
        item_obj = (Elm_Object_Item *)item;
        return elm_object_item_disabled_get(item_obj);

      case ELM_DBUS_PROPERTY_ICON_NAME:
        return _freedesktop_icon_exists(item);

      case ELM_DBUS_PROPERTY_TYPE:
      case ELM_DBUS_PROPERTY_UNKNOWN:
        return EINA_FALSE;
     }

   ERR("Invalid code path");
   return EINA_FALSE;
}

// Ad-hoc dbusmenu property dictionary subset implementation
// Depends on _property_exists results
static void
_property_append(Elm_Menu_Item *item,
                 Elm_DBus_Property property,
                 EDBus_Message_Iter *iter)
{
   EDBus_Message_Iter *variant = NULL;
   Elm_Object_Item *item_obj = (Elm_Object_Item *)item;
   const char *t;

   switch (property)
     {
      case ELM_DBUS_PROPERTY_LABEL:
        variant = edbus_message_iter_container_new(iter, 'v', "s");
        t = elm_object_item_part_text_get(item_obj, NULL);
        if (!t)
          {
             t = elm_object_part_text_get(item->content, NULL);
             if (!t) t = "";
          }

        edbus_message_iter_basic_append(variant, 's', t);
        break;

      case ELM_DBUS_PROPERTY_CHILDREN_DISPLAY:
        variant = edbus_message_iter_container_new(iter, 'v', "s");
        edbus_message_iter_basic_append(variant, 's', "submenu");
        break;

      case ELM_DBUS_PROPERTY_ENABLED:
        variant = edbus_message_iter_container_new(iter, 'v', "b");
        edbus_message_iter_basic_append(variant, 'b', EINA_FALSE);
        break;

      case ELM_DBUS_PROPERTY_TYPE:
        variant = edbus_message_iter_container_new(iter, 'v', "s");
        edbus_message_iter_basic_append(variant, 's', "separator");
        break;

      case ELM_DBUS_PROPERTY_ICON_NAME:
        variant = edbus_message_iter_container_new(iter, 'v', "s");
        edbus_message_iter_basic_append(variant, 's', item->icon_str);
        break;

      case ELM_DBUS_PROPERTY_UNKNOWN:
        ERR("Invalid code path");
        return;
     }

   edbus_message_iter_container_close(iter, variant);
}

static void
_property_dict_build(Elm_Menu_Item *item,
                     Eina_List *property_list, EDBus_Message_Iter *iter)
{
   char *propstr;
   Elm_DBus_Property property;
   EDBus_Message_Iter *array, *pair;
   Eina_List *l;

   array = edbus_message_iter_container_new(iter, 'a', "{sv}");

   EINA_LIST_FOREACH (property_list, l, propstr)
     {
        property = _str_to_property(propstr);

        if (property == ELM_DBUS_PROPERTY_UNKNOWN) continue;
        if (!_property_exists(item, property)) continue;

        pair = edbus_message_iter_container_new(array, 'e', NULL);
        edbus_message_iter_basic_append(pair, 's', propstr);
        _property_append(item, property, pair);
        edbus_message_iter_container_close(array, pair);
     }

   edbus_message_iter_container_close(iter, array);
}

static void
_layout_build_recursive(Elm_Menu_Item *item,
                        Eina_List *property_list, unsigned recursion_depth,
                        EDBus_Message_Iter *iter)
{
   Eina_List *l;
   Elm_Menu_Item *subitem;
   EDBus_Message_Iter *layout, *array, *variant;

   layout = edbus_message_iter_container_new(iter, 'r', NULL);
   edbus_message_iter_basic_append(layout, 'i', item->dbus_idx);
   _property_dict_build(item, property_list, layout);
   array = edbus_message_iter_container_new(layout, 'a', "v");

   if (recursion_depth > 0)
     {
        EINA_LIST_FOREACH (item->submenu.items, l, subitem)
          {
             variant = edbus_message_iter_container_new(array, 'v',
                                                        "(ia{sv}av)");
             _layout_build_recursive(subitem, property_list,
                                     recursion_depth - 1, variant);
             edbus_message_iter_container_close(array, variant);
          }
     }

   edbus_message_iter_container_close(layout, array);
   edbus_message_iter_container_close(iter, layout);
}

static void
_root_layout_build(Elm_DBus_Menu *dbus_menu, Eina_List *property_list,
                   unsigned recursion_depth, EDBus_Message_Iter *iter)
{
   char *property;
   EDBus_Message_Iter *layout, *array, *pair, *variant;
   const Eina_List *ret = NULL;
   Eina_List *items;
   Eina_List *l;
   Elm_Menu_Item *item;

   layout = edbus_message_iter_container_new(iter, 'r', NULL);
   edbus_message_iter_basic_append(layout, 'i', 0);
   array = edbus_message_iter_container_new(layout, 'a', "{sv}");

   EINA_LIST_FOREACH (property_list, l, property)
     {
        if (!strcmp(property, "children-display"))
          {
             pair = edbus_message_iter_container_new(array, 'e', NULL);
             edbus_message_iter_basic_append(pair, 's', property);
             variant = edbus_message_iter_container_new(pair, 'v', "s");
             edbus_message_iter_basic_append(variant, 's', "submenu");
             edbus_message_iter_container_close(pair, variant);
             edbus_message_iter_container_close(array, pair);
             break;
          }
     }

   edbus_message_iter_container_close(layout, array);
   array = edbus_message_iter_container_new(layout, 'a', "v");

   if (recursion_depth > 0)
     {
        eo_do(dbus_menu->menu, elm_obj_menu_items_get(&ret));
        items = (Eina_List *)ret;
        EINA_LIST_FOREACH (items, l, item)
          {
             variant = edbus_message_iter_container_new(array, 'v',
                                                        "(ia{sv}av)");
             _layout_build_recursive(item, property_list,
                                     recursion_depth - 1, variant);
             edbus_message_iter_container_close(array, variant);
          }
     }

   edbus_message_iter_container_close(layout, array);
   edbus_message_iter_container_close(iter, layout);
}

static Eina_List *
_empty_properties_handle(Eina_List *property_list)
{
   if (!eina_list_count(property_list))
     {
        property_list = eina_list_append(property_list, "label");
        property_list = eina_list_append(property_list, "children-display");
        property_list = eina_list_append(property_list, "enabled");
        property_list = eina_list_append(property_list, "type");
        property_list = eina_list_append(property_list, "icon-name");
     }
   return property_list;
}

static Eina_Bool
_event_handle(Elm_DBus_Menu *dbus_menu, EDBus_Message_Iter *iter, int *error_id)
{
   Elm_Menu_Item *item;
   const char *event;
   int id;
   int32_t i;
   EDBus_Message_Iter *data;
   unsigned *timestamp;

   edbus_message_iter_arguments_get(iter, "isvu", &id, &event, &data,
                                    &timestamp);
   i = id;
   item = eina_hash_find(dbus_menu->elements, &i);
   if (!item)
     {
        if (error_id) *error_id = id;
        return EINA_FALSE;
     }

   if (!strcmp(event, "clicked"))
     _elm_dbus_menu_item_select_cb((Elm_Object_Item *)item);

   return EINA_TRUE;
}

static Elm_DBus_Menu *
_elm_dbus_menu_add(Eo *menu)
{
   Elm_DBus_Menu *dbus_menu;
   const Eina_List *ret = NULL;
   Eina_List *items, *l;
   Elm_Menu_Item *item;

   ELM_MENU_CHECK(menu) NULL;

   dbus_menu = calloc(1, sizeof(Elm_DBus_Menu));
   if (!dbus_menu)
     {
        ERR("Unable to allocate D-Bus data");
        return NULL;
     }

   dbus_menu->elements = eina_hash_int32_new(NULL);
   if (!dbus_menu->elements)
     {
        ERR("Unable to allocate hash table");
        goto error_menu;
     }

   dbus_menu->menu = menu;

   eo_do(menu, elm_obj_menu_items_get(&ret));
   items = (Eina_List *)ret;
   EINA_LIST_FOREACH (items, l, item)
     {
        if (!_menu_add_recursive(dbus_menu, item))
          {
             ERR("Unable to add menu item");
             goto error_hash;
          }
     }

   return dbus_menu;

error_hash:
   eina_hash_free(dbus_menu->elements);
error_menu:
   free(dbus_menu);
   return NULL;
}

// =============================================================================
//                            com.canonical.dbusmenu
// =============================================================================
// =============================================================================
// Methods
// =============================================================================
static EDBus_Message *
_method_layout_get(const EDBus_Service_Interface *iface,
                   const EDBus_Message *msg)
{
   int parent_id;
   int32_t id;
   int r;
   unsigned recursion_depth;
   char *property;
   Eina_List *property_list = NULL;
   EDBus_Message *reply;
   EDBus_Message_Iter *iter, *array;
   Elm_DBus_Menu *dbus_menu;
   Elm_Menu_Item *item = NULL;

   dbus_menu = edbus_service_object_data_get(iface, DBUS_DATA_KEY);

   if (!edbus_message_arguments_get(msg, "iias", &parent_id, &r, &array))
     ERR("Invalid arguments in D-Bus message");

   recursion_depth = r;

   while (edbus_message_iter_get_and_next(array, 's', &property))
     property_list = eina_list_append(property_list, property);

   property_list = _empty_properties_handle(property_list);

   if (parent_id)
     {
        id = parent_id;
        item = eina_hash_find(dbus_menu->elements, &id);
        if (!item)
          {
             reply = edbus_message_error_new(msg, DBUS_INTERFACE ".Error",
                                             "Invalid parent");
             return reply;
          }
     }

   reply = edbus_message_method_return_new(msg);
   iter = edbus_message_iter_get(reply);
   edbus_message_iter_basic_append(iter, 'u', dbus_menu->timestamp);

   if (parent_id)
     _layout_build_recursive(item, property_list, recursion_depth, iter);
   else
     _root_layout_build(dbus_menu, property_list, recursion_depth, iter);

   eina_list_free(property_list);
   return reply;
}

static EDBus_Message *
_method_group_properties_get(const EDBus_Service_Interface *iface,
                             const EDBus_Message *msg)
{
   Eina_Iterator *hash_iter;
   EDBus_Message *reply;
   EDBus_Message_Iter *ids, *property_names;
   EDBus_Message_Iter *iter, *array, *tuple;
   Eina_List *property_list = NULL;
   Elm_DBus_Menu *dbus_menu;
   Elm_Menu_Item *item;
   char *property;
   int id;
   int32_t i;
   void *data;

   dbus_menu = edbus_service_object_data_get(iface, DBUS_DATA_KEY);

   if (!edbus_message_arguments_get(msg, "aias", &ids, &property_names))
     ERR("Invalid arguments in D-Bus message");

   while (edbus_message_iter_get_and_next(property_names, 's', &property))
     property_list = eina_list_append(property_list, property);

   property_list = _empty_properties_handle(property_list);

   reply = edbus_message_method_return_new(msg);
   iter = edbus_message_iter_get(reply);
   array = edbus_message_iter_container_new(iter, 'a', "(ia{sv})");

   if (!edbus_message_iter_get_and_next(ids, 'i', &id))
     {
        hash_iter = eina_hash_iterator_data_new(dbus_menu->elements);

        while (eina_iterator_next(hash_iter, &data))
          {
             item = data;
             tuple = edbus_message_iter_container_new(array, 'r', NULL);
             edbus_message_iter_basic_append(tuple, 'i', item->dbus_idx);
             _property_dict_build(item, property_list, tuple);
             edbus_message_iter_container_close(array, tuple);
          }

        eina_iterator_free(hash_iter);
     }
   else
     do
       {
          i = id;
          item = eina_hash_find(dbus_menu->elements, &i);
          if (!item) continue;

          tuple = edbus_message_iter_container_new(array, 'r', NULL);
          edbus_message_iter_basic_append(tuple, 'i', item->dbus_idx);
          _property_dict_build(item, property_list, tuple);
          edbus_message_iter_container_close(array, tuple);
       }
     while (edbus_message_iter_get_and_next(ids, 'i', &id));

   edbus_message_iter_container_close(iter, array);
   eina_list_free(property_list);

   return reply;
}

static EDBus_Message *
_method_property_get(const EDBus_Service_Interface *iface,
                     const EDBus_Message *msg)
{
   EDBus_Message *reply;
   EDBus_Message_Iter *iter, *variant;
   Elm_DBus_Property property;
   Elm_DBus_Menu *dbus_menu;
   Elm_Menu_Item *item;
   int id;
   int32_t i;
   char *name;

   dbus_menu = edbus_service_object_data_get(iface, DBUS_DATA_KEY);

   if (!edbus_message_arguments_get(msg, "is", &id, &name))
     ERR("Invalid arguments in D-Bus message");

   property = _str_to_property(name);

   if (property == ELM_DBUS_PROPERTY_UNKNOWN)
     {
        reply = edbus_message_error_new(msg, DBUS_INTERFACE ".Error",
                                        "Property not found");
        return reply;
     }

   if (!id)
     {
        if (property != ELM_DBUS_PROPERTY_CHILDREN_DISPLAY)
          reply = edbus_message_error_new(msg, DBUS_INTERFACE ".Error",
                                          "Property not found");
        else
          {
             reply = edbus_message_method_return_new(msg);
             iter = edbus_message_iter_get(reply);
             variant = edbus_message_iter_container_new(iter, 'v', "s");
             edbus_message_iter_basic_append(variant, 's', "submenu");
             edbus_message_iter_container_close(iter, variant);
          }

        return reply;
     }

   i = id;
   item = eina_hash_find(dbus_menu->elements, &i);

   if (!item)
     {
        reply = edbus_message_error_new(msg, DBUS_INTERFACE ".Error",
                                        "Invalid menu identifier");
        return reply;
     }

   if (!_property_exists(item, property))
     {
        reply = edbus_message_error_new(msg, DBUS_INTERFACE ".Error",
                                        "Property not found");
        return reply;
     }

   reply = edbus_message_method_return_new(msg);
   iter = edbus_message_iter_get(reply);
   _property_append(item, property, iter);

   return reply;
}

static EDBus_Message *
_method_event(const EDBus_Service_Interface *iface,
              const EDBus_Message *msg)
{
   Elm_DBus_Menu *dbus_menu;
   EDBus_Message *reply;

   reply = edbus_message_method_return_new(msg);
   dbus_menu = edbus_service_object_data_get(iface, DBUS_DATA_KEY);

   if (!_event_handle(dbus_menu, edbus_message_iter_get(msg), NULL))
     reply = edbus_message_error_new(msg, DBUS_INTERFACE ".Error",
                                     "Invalid menu");
   else
     reply = edbus_message_method_return_new(msg);

   return reply;
}

static EDBus_Message *
_method_event_group(const EDBus_Service_Interface *iface,
                    const EDBus_Message *msg)
{
   EDBus_Message *reply;
   EDBus_Message_Iter *iter, *array, *tuple, *errors;
   int id;
   Elm_DBus_Menu *dbus_menu;
   Eina_Bool return_error = EINA_TRUE;

   dbus_menu = edbus_service_object_data_get(iface, DBUS_DATA_KEY);

   if (!edbus_message_arguments_get(msg, "a(isvu)", &array))
     ERR("Invalid arguments in D-Bus message");

   reply = edbus_message_method_return_new(msg);
   iter = edbus_message_iter_get(reply);
   errors = edbus_message_iter_container_new(iter, 'a', "i");

   while (edbus_message_iter_get_and_next(array, 'r', &tuple))
     {
        if (_event_handle(dbus_menu, tuple, &id))
          return_error = EINA_FALSE;
        else
          edbus_message_iter_basic_append(errors, 'i', id);
     }

   if (return_error)
     {
        edbus_message_unref(reply);
        reply = edbus_message_error_new(msg, DBUS_INTERFACE ".Error",
                                        "Invalid menu identifiers");
     }
   else
     edbus_message_iter_container_close(iter, errors);

   return reply;
}

static EDBus_Message *
_method_about_to_show(const EDBus_Service_Interface *iface EINA_UNUSED,
                      const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   edbus_message_arguments_append(reply, "b", EINA_TRUE);

   return reply;
}

static EDBus_Message *
_method_about_to_show_group(const EDBus_Service_Interface *iface EINA_UNUSED,
                            const EDBus_Message *msg)
{
   EDBus_Message *reply = edbus_message_method_return_new(msg);
   EDBus_Message_Iter *iter, *array;

   iter = edbus_message_iter_get(reply);
   array = edbus_message_iter_container_new(iter, 'a', "i");
   edbus_message_iter_container_close(iter, array);
   array = edbus_message_iter_container_new(iter, 'a', "i");
   edbus_message_iter_container_close(iter, array);

   return reply;
}

static const EDBus_Method _methods[] = {
   {
      "GetLayout",
      EDBUS_ARGS({"i", "parentId"},
                 {"i", "recursionDepth"},
                 {"as", "propertyNames"}),
      EDBUS_ARGS({"u", "revision"}, {"(ia{sv}av)", "layout"}),
      _method_layout_get,
      0
   },
   {
      "GetGroupProperties",
      EDBUS_ARGS({"ai", "ids"}, {"as", "propertyNames"}),
      EDBUS_ARGS({"a(ia{sv})", "properties"}),
      _method_group_properties_get,
      0
   },
   {
      "GetProperty",
      EDBUS_ARGS({"i", "id"}, {"s", "name"}),
      EDBUS_ARGS({"v", "value"}),
      _method_property_get,
      0
   },
   {
      "Event",
      EDBUS_ARGS({"i", "id"},
                 {"s", "eventId"},
                 {"v", "data"},
                 {"u", "timestamp"}),
      NULL,
      _method_event,
      0
   },
   {
      "EventGroup",
      EDBUS_ARGS({"a(isvu)", "events"}),
      EDBUS_ARGS({"ai", "idErrors"}),
      _method_event_group,
      0
   },
   {
      "AboutToShow",
      EDBUS_ARGS({"i", "id"}),
      EDBUS_ARGS({"b", "needUpdate"}),
      _method_about_to_show,
      0
   },
   {
      "AboutToShowGroup",
      EDBUS_ARGS({"ai", "ids"}),
      EDBUS_ARGS({"ai", "updatesNeeded"}, {"ai", "idErrors"}),
      _method_about_to_show_group,
      0
   },

   {NULL, NULL, NULL, NULL, 0}
};

// =============================================================================
// Signals
// =============================================================================
static const EDBus_Signal _signals[] = {
   [ELM_DBUS_SIGNAL_LAYOUT_UPDATED] = {
      "LayoutUpdated", EDBUS_ARGS({"u", "revision"}, {"i", "parent"}), 0
   },
   [ELM_DBUS_SIGNAL_ITEM_ACTIVATION_REQUESTED] = {
      "ItemActivationRequested", EDBUS_ARGS({"i", "id"}, {"u", "timestamp"}), 0
   },
   {NULL, NULL, 0}
};

// =============================================================================
// Properties
// =============================================================================
static Eina_Bool
_prop_version_get(const EDBus_Service_Interface *iface EINA_UNUSED,
                  const char *propname EINA_UNUSED,
                  EDBus_Message_Iter *iter,
                  const EDBus_Message *request_msg EINA_UNUSED,
                  EDBus_Message **error EINA_UNUSED)
{
   edbus_message_iter_basic_append(iter, 'u', DBUS_MENU_VERSION);

   return EINA_TRUE;
}

static Eina_Bool
_prop_text_direction_get(const EDBus_Service_Interface *iface EINA_UNUSED,
                         const char *propname EINA_UNUSED,
                         EDBus_Message_Iter *iter,
                         const EDBus_Message *request_msg EINA_UNUSED,
                         EDBus_Message **error EINA_UNUSED)
{
   if (_elm_config->is_mirrored)
     edbus_message_iter_basic_append(iter, 's', "rtl");
   else
     edbus_message_iter_basic_append(iter, 's', "ltr");

   return EINA_TRUE;
}

static Eina_Bool
_prop_status_get(const EDBus_Service_Interface *iface EINA_UNUSED,
                 const char *propname EINA_UNUSED,
                 EDBus_Message_Iter *iter,
                 const EDBus_Message *request_msg EINA_UNUSED,
                 EDBus_Message **error EINA_UNUSED)
{
   static const char *normal = "normal";
   edbus_message_iter_basic_append(iter, 's', normal);

   return EINA_TRUE;
}

static Eina_Bool
_prop_icon_theme_path_get(const EDBus_Service_Interface *iface EINA_UNUSED,
                          const char *propname EINA_UNUSED,
                          EDBus_Message_Iter *iter,
                          const EDBus_Message *request_msg EINA_UNUSED,
                          EDBus_Message **error EINA_UNUSED)
{
   EDBus_Message_Iter *actions;
   edbus_message_iter_arguments_append(iter, "as", &actions);
   edbus_message_iter_arguments_append(actions, "s", ICON_DIR);
   edbus_message_iter_container_close(iter, actions);

   return EINA_TRUE;
}

static const EDBus_Property _properties[] = {
   { "Version", "u", _prop_version_get, NULL, 0 },
   { "TextDirection", "s", _prop_text_direction_get, NULL, 0 },
   { "Status", "s", _prop_status_get, NULL, 0 },
   { "IconThemePath", "as", _prop_icon_theme_path_get, NULL, 0 },
   { NULL, NULL, NULL, NULL, 0 },
};

static const EDBus_Service_Interface_Desc _interface = {
   DBUS_INTERFACE, _methods, _signals, _properties, NULL, NULL
};
// =============================================================================

const char *
_elm_dbus_menu_register(Eo *obj)
{
   char buf[60];
   ELM_MENU_CHECK(obj) NULL;
   ELM_MENU_DATA_GET(obj, sd);

   elm_need_edbus();

   if (sd->dbus_menu)
     goto end;

   sd->dbus_menu = _elm_dbus_menu_add(obj);
   sd->dbus_menu->bus = edbus_connection_get(EDBUS_CONNECTION_TYPE_SESSION);
   snprintf(buf, sizeof(buf), "%s/%u", DBUS_PATH, ++last_object_path);
   sd->dbus_menu->iface = edbus_service_interface_register(sd->dbus_menu->bus,
                                                           buf,
                                                           &_interface);
   edbus_service_object_data_set(sd->dbus_menu->iface, DBUS_DATA_KEY,
                                 sd->dbus_menu);

end:
   return edbus_service_object_path_get(sd->dbus_menu->iface);
}

void
_elm_dbus_menu_unregister(Eo *obj)
{
   // TODO: support refcounting object paths

   ELM_MENU_CHECK(obj);
   ELM_MENU_DATA_GET(obj, sd);

   if (!sd->dbus_menu) return;

   if (sd->dbus_menu->xid)
     _elm_dbus_menu_app_menu_unregister(sd->dbus_menu->menu);
   edbus_service_interface_unregister(sd->dbus_menu->iface);
   edbus_connection_unref(sd->dbus_menu->bus);
   if (sd->dbus_menu->signal_idler)
     ecore_idler_del(sd->dbus_menu->signal_idler);

   eina_hash_free(sd->dbus_menu->elements);
   free(sd->dbus_menu);
   sd->dbus_menu = NULL;
}

void
_elm_dbus_menu_app_menu_register(Ecore_X_Window xid, Eo *obj,
				 void (*result_cb)(Eina_Bool, void *), void *data)
{
   EDBus_Message *msg;
   const char *obj_path;
   Callback_Data *cd;

   ELM_MENU_CHECK(obj);
   ELM_MENU_DATA_GET(obj, sd);

   if (!sd->dbus_menu || !sd->dbus_menu->bus)
     {
        ERR("D-Bus is inactive for menu: %p", obj);
        return;
     }

   msg = edbus_message_method_call_new(REGISTRAR_NAME, REGISTRAR_PATH,
                                       REGISTRAR_INTERFACE, "RegisterWindow");
   cd = malloc(sizeof(Callback_Data));
   cd->result_cb = result_cb;
   cd->data = data;
   obj_path = edbus_service_object_path_get(sd->dbus_menu->iface);
   edbus_message_arguments_append(msg, "uo", (unsigned)xid,
                                  obj_path);
   edbus_connection_send(sd->dbus_menu->bus, msg, _app_register_cb,
			 cd, -1);
   sd->dbus_menu->xid = xid;
}

void
_elm_dbus_menu_app_menu_unregister(Eo *obj)
{
   EDBus_Message *msg;

   ELM_MENU_CHECK(obj);
   ELM_MENU_DATA_GET(obj, sd);

   if (!sd->dbus_menu || !sd->dbus_menu->bus)
     {
        ERR("D-Bus is inactive for menu: %p", obj);
        return;
     }

   if (!sd->dbus_menu->xid) return;

   msg = edbus_message_method_call_new(REGISTRAR_NAME, REGISTRAR_PATH,
                                       REGISTRAR_INTERFACE, "UnregisterWindow");
   edbus_message_arguments_append(msg, "u", (unsigned)sd->dbus_menu->xid);
   edbus_connection_send(sd->dbus_menu->bus, msg, NULL, NULL, -1);
   sd->dbus_menu->xid = 0;
}

int
_elm_dbus_menu_item_add(Elm_DBus_Menu *dbus_menu, Elm_Object_Item *item_obj)
{
   Elm_Menu_Item *item = (Elm_Menu_Item *)item_obj;
   int32_t id = dbus_menu->timestamp + 1;

   if (!eina_hash_add(dbus_menu->elements, &id, item))
     {
        ERR("Unable to add menu");
        return -1;
     }

   _layout_signal(dbus_menu);
   return ++dbus_menu->timestamp;
}

void
_elm_dbus_menu_item_delete(Elm_DBus_Menu *dbus_menu, int id)
{
   int32_t i;

   i = id;

   if (!eina_hash_del_by_key(dbus_menu->elements, &i))
     {
        ERR("Invalid menu ID: %d", id);
        return;
     }

   dbus_menu->timestamp++;
   _layout_signal(dbus_menu);
}

void
_elm_dbus_menu_update(Elm_DBus_Menu *dbus_menu)
{
   _layout_signal(dbus_menu);
}

#else

const char *
_elm_dbus_menu_register(Eo *obj EINA_UNUSED)
{
   return NULL;
}

void
_elm_dbus_menu_unregister(Eo *obj EINA_UNUSED)
{
}

void
_elm_dbus_menu_app_menu_register(Ecore_X_Window xid EINA_UNUSED, Eo *obj EINA_UNUSED)
{
}

void
_elm_dbus_menu_app_menu_unregister(Eo *obj EINA_UNUSED)
{
}

int
_elm_dbus_menu_item_add(Elm_DBus_Menu *dbus_menu EINA_UNUSED,
                        Elm_Object_Item *item_obj EINA_UNUSED)
{
   return -1;
}

void
_elm_dbus_menu_item_delete(Elm_DBus_Menu *dbus_menu EINA_UNUSED, int id EINA_UNUSED)
{
}

void
_elm_dbus_menu_update(Elm_DBus_Menu *dbus_menu EINA_UNUSED)
{
}

#endif

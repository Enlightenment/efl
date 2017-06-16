#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <assert.h>

typedef struct _Elm_Atspi_Cache_Data {
     Eldbus_Connection *conn;
     Eldbus_Service_Interface *cache_ifc;
     Eina_Hash *cache;
} Elm_Atspi_Cache_Data;

#include "atspi/atspi-constants.h"
#include "elm_priv.h"
#include "elm_atspi_cache.eo.h"

#define CACHE_ITEM_SIGNATURE "((so)(so)(so)a(so)assusau)"
#define CACHE_INTERFACE_PATH "/org/a11y/atspi/cache"

typedef enum _Atspi_Cache_Event_Type
{
   ATSPI_CACHE_EVENT_ACCESSIBLE_ADDED = 0,
   ATSPI_CACHE_EVENT_ACCESSIBLE_REMOVED
} Atspi_Cache_Event_Type;

static void
_iter_interfaces_append(Eldbus_Message_Iter *iter, const Eo *obj)
{
}

static void
_cache_item_reference_append(Eldbus_Connection *conn, Elm_Interface_Atspi_Accessible *object, Eldbus_Message_Iter *iter_array)
{
  Eldbus_Message_Iter *iter_struct, *iter_sub_array;
  Elm_Atspi_State_Set states;
  Elm_Atspi_Role role;

  role = elm_interface_atspi_accessible_role_get(object);

  iter_struct = eldbus_message_iter_container_new(iter_array, 'r', NULL);
  EINA_SAFETY_ON_NULL_RETURN(iter_struct);

  /* Marshall object path */
  elm_atspi_dbus_object_reference_append(conn, iter_struct, object);

  /* Marshall root */
  elm_atspi_dbus_object_reference_append(conn, iter_struct, elm_interface_atspi_accessible_root_get(ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN));

  /* Marshall parent */
  Eo *parent = elm_interface_atspi_accessible_parent_get(object);
  elm_atspi_dbus_object_reference_append(conn, iter_struct, parent);

  /* Marshall children  */
  Eina_List *children_list = NULL, *l;
  Eo *child;

  children_list = elm_interface_atspi_accessible_children_get(object);
  iter_sub_array = eldbus_message_iter_container_new(iter_struct, 'a', "(so)");
  EINA_SAFETY_ON_NULL_GOTO(iter_sub_array, fail);

  EINA_LIST_FOREACH(children_list, l, child)
     elm_atspi_dbus_object_reference_append(conn, iter_sub_array, child);

  eldbus_message_iter_container_close(iter_struct, iter_sub_array);
  eina_list_free(children_list);

  /* Marshall interfaces */
  elm_atspi_dbus_object_interfaces_append(iter_struct, object);

  /* Marshall name */
  const char *name = NULL;
  name = elm_interface_atspi_accessible_name_get(object);
  if (!name)
    name = "";

  eldbus_message_iter_basic_append(iter_struct, 's', name);

  /* Marshall role */
  eldbus_message_iter_basic_append(iter_struct, 'u', role);

  /* Marshall description */
  const char* descritpion = NULL;
  descritpion = elm_interface_atspi_accessible_description_get(object);
  if (!descritpion)
    descritpion = "";
  eldbus_message_iter_basic_append(iter_struct, 's', descritpion);

  /* Marshall state set */
  iter_sub_array = eldbus_message_iter_container_new(iter_struct, 'a', "u");
  EINA_SAFETY_ON_NULL_GOTO(iter_sub_array, fail);

  states = elm_interface_atspi_accessible_state_set_get(object);

  unsigned int s1 = states & 0xFFFFFFFF;
  unsigned int s2 = (states >> 32) & 0xFFFFFFFF;
  eldbus_message_iter_basic_append(iter_sub_array, 'u', s1);
  eldbus_message_iter_basic_append(iter_sub_array, 'u', s2);

  eldbus_message_iter_container_close(iter_struct, iter_sub_array);
  eldbus_message_iter_container_close(iter_array, iter_struct);

  return;

fail:
  if (iter_struct) eldbus_message_iter_del(iter_struct);
}

static Eldbus_Message *
_cache_get_items(const Eldbus_Service_Interface *iface, const Eldbus_Message *msg)
{
   Eldbus_Message_Iter *iter, *iter_array;
   Eldbus_Message *ret;
   Eina_Iterator *cache_iterator;
   Elm_Interface_Atspi_Accessible *accessible;

   Elm_Atspi_Cache_Data *cache_data = eldbus_service_object_data_get(iface, "data");
   if (!cache_data) return NULL;

   ret = eldbus_message_method_return_new(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   iter = eldbus_message_iter_get(ret);
   iter_array = eldbus_message_iter_container_new(iter, 'a', CACHE_ITEM_SIGNATURE);
   EINA_SAFETY_ON_NULL_GOTO(iter_array, fail);

   cache_iterator = eina_hash_iterator_data_new(cache_data->cache);
   EINA_SAFETY_ON_NULL_GOTO(iter_array, fail);

   if (eina_iterator_next(cache_iterator, &accessible))
     {
        _cache_item_reference_append(cache_data->conn, accessible, iter_array);
     }

   eina_iterator_free(cache_iterator);
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
  [ATSPI_CACHE_EVENT_ACCESSIBLE_ADDED] = { "AddAccessible", ELDBUS_ARGS({"((so)(so)a(so)assusau)", "added"}), 0},
  [ATSPI_CACHE_EVENT_ACCESSIBLE_REMOVED] = { "RemoveAccessible", ELDBUS_ARGS({ "(so)", "removed" }), 0},
  {NULL, NULL, 0}
};

static const Eldbus_Service_Interface_Desc cache_iface_desc = {
   ATSPI_DBUS_INTERFACE_CACHE, cache_methods, cache_signals, NULL, NULL, NULL
};

EOLIAN static Elm_Atspi_Cache*
_elm_atspi_cache_efl_object_constructor(Elm_Atspi_Cache *cache, Elm_Atspi_Cache_Data *pd)
{
   efl_constructor(efl_super(cache, ELM_ATSPI_CACHE_CLASS));

   pd->cache_ifc = eldbus_service_interface_fallback_register(pd->conn,
                                                              CACHE_INTERFACE_PATH,
                                                              &cache_iface_desc);
   eldbus_service_object_data_set(pd->cache_ifc, "data", pd);

   if (!pd->cache_ifc)
     return NULL;

   pd->cache = eina_hash_pointer_new(NULL);
   assert (pd->cache != NULL);

   return cache;
}

EOLIAN static void
_elm_atspi_cache_constructor(Elm_Atspi_Cache *cache, Elm_Atspi_Cache_Data *pd, Eldbus_Connection *conn)
{
   pd->conn = elbus_connection_ref(conn);
   assert (pd->conn != NULL);
}

EOLIAN static void
_elm_atspi_cache_efl_object_destructor(Elm_Atspi_Cache *cache, Elm_Atspi_Cache_Data *pd)
{
   eina_hash_free(pd->cache);
   eldbus_connection_unref(pd->conn);
   efl_destructor(efl_super(cache, ELM_ATSPI_CACHE_CLASS));
}

EOLIAN static void
_elm_atspi_cache_add(Elm_Atspi_Cache *cache, Elm_Atspi_Cache_Data *pd, Elm_Interface_Atspi_Accessible *object)
{
   eina_hash_add(pd->cache, &object, object);
}

EOLIAN static void
_elm_atspi_cache_remove(Elm_Atspi_Cache *cache, Elm_Atspi_Cache_Data *pd, Elm_Interface_Atspi_Accessible *object)
{
   eina_hash_del(pd->cache, &object, object);
}

EOLIAN static Eina_Bool
_elm_atspi_cache_object_contains(Elm_Atspi_Cache *cache EINA_UNUSED, Elm_Atspi_Cache_Data *pd, Elm_Interface_Atspi_Accessible *obj)
{
   return eina_hash_find(pd->cache, &obj) != NULL ? EINA_TRUE : EINA_FALSE;
}

#include "elm_atspi_cache.eo.c"

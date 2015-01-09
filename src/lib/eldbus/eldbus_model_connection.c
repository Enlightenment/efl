#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eldbus_model_connection_private.h"
#include "eldbus_model_private.h"

#define MY_CLASS ELDBUS_MODEL_CONNECTION_CLASS
#define MY_CLASS_NAME "Eldbus_Model_Connection"

#define UNIQUE_NAME_PROPERTY "unique_name"

static void _eldbus_model_connection_efl_model_base_properties_load(Eo *, Eldbus_Model_Connection_Data *);
static void _eldbus_model_connection_efl_model_base_children_load(Eo *, Eldbus_Model_Connection_Data *);
static void _eldbus_model_connection_names_list_cb(void *, const Eldbus_Message *, Eldbus_Pending *);
static void _eldbus_model_connection_connect(Eldbus_Model_Connection_Data *);
static void _eldbus_model_connection_disconnect(Eldbus_Model_Connection_Data *);
static void _eldbus_model_connection_clear(Eldbus_Model_Connection_Data *);

static Eo_Base*
_eldbus_model_connection_eo_base_constructor(Eo *obj, Eldbus_Model_Connection_Data *pd)
{
   obj = eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());

   pd->obj = obj;
   pd->load.status = EFL_MODEL_LOAD_STATUS_UNLOADED;
   pd->connection = NULL;
   pd->properties_array = NULL;
   pd->children_list = NULL;
   pd->type = ELDBUS_CONNECTION_TYPE_UNKNOWN;
   pd->address = NULL;
   pd->private = false;
   pd->unique_name = NULL;
   pd->pending_list = NULL;

   return obj;
}

static void
_eldbus_model_connection_constructor(Eo *obj EINA_UNUSED,
                                     Eldbus_Model_Connection_Data *pd,
                                     int type,
                                     const char* address,
                                     Eina_Bool private)
{
   pd->type = type;
   pd->address = eina_stringshare_add(address);
   pd->private = private;
}

static void
_eldbus_model_connection_eo_base_destructor(Eo *obj, Eldbus_Model_Connection_Data *pd)
{
   eina_stringshare_del(pd->address);

   _eldbus_model_connection_clear(pd);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Efl_Model_Load_Status
_eldbus_model_connection_efl_model_base_properties_get(Eo *obj EINA_UNUSED,
                                                    Eldbus_Model_Connection_Data *pd,
                                                    Eina_Array * const* properties_array)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EFL_MODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->obj, EFL_MODEL_LOAD_STATUS_ERROR);

   if (pd->properties_array == NULL)
     {
        Eina_Bool ret;

        pd->properties_array = eina_array_new(1);
        EINA_SAFETY_ON_NULL_RETURN_VAL(pd->properties_array, EFL_MODEL_LOAD_STATUS_ERROR);

        ret = eina_array_push(pd->properties_array, UNIQUE_NAME_PROPERTY);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, EFL_MODEL_LOAD_STATUS_ERROR);
     }

   *(Eina_Array**)properties_array = pd->properties_array;
   return pd->load.status;
}

static void
_eldbus_model_connection_efl_model_base_properties_load(Eo *obj EINA_UNUSED, Eldbus_Model_Connection_Data *pd)
{
   const char *unique_name;
   Eina_Bool ret;

   if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES)
     return;

   if (!pd->connection)
     _eldbus_model_connection_connect(pd);

   pd->unique_name = eina_value_new(EINA_VALUE_TYPE_STRING);
   EINA_SAFETY_ON_NULL_RETURN(pd->unique_name);

   unique_name = eldbus_connection_unique_name_get(pd->connection);
   ret = eina_value_set(pd->unique_name, unique_name);
   EINA_SAFETY_ON_FALSE_RETURN(ret);

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);
}

static Efl_Model_Load_Status
_eldbus_model_connection_efl_model_base_property_set(Eo *obj EINA_UNUSED,
                                             Eldbus_Model_Connection_Data *pd EINA_UNUSED,
                                             const char *property EINA_UNUSED,
                                             Eina_Value const* value EINA_UNUSED)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EFL_MODEL_LOAD_STATUS_ERROR);
   DBG("(%p): property=%s", obj, property);
   return EFL_MODEL_LOAD_STATUS_ERROR;
}

static Efl_Model_Load_Status
_eldbus_model_connection_efl_model_base_property_get(Eo *obj EINA_UNUSED,
                                             Eldbus_Model_Connection_Data *pd,
                                             const char *property,
                                             Eina_Value const **value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EFL_MODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EFL_MODEL_LOAD_STATUS_ERROR);
   DBG("(%p): property=%s", obj, property);

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
     return EFL_MODEL_LOAD_STATUS_ERROR;

   if (strcmp(property, UNIQUE_NAME_PROPERTY) != 0)
     return EFL_MODEL_LOAD_STATUS_ERROR;

   *value = pd->unique_name;
   return pd->load.status;
}

static void
_eldbus_model_connection_efl_model_base_load(Eo *obj EINA_UNUSED, Eldbus_Model_Connection_Data *pd)
{
   if (!pd->connection)
     _eldbus_model_connection_connect(pd);

   _eldbus_model_connection_efl_model_base_properties_load(obj, pd);
   _eldbus_model_connection_efl_model_base_children_load(obj, pd);
}

static Efl_Model_Load_Status
_eldbus_model_connection_efl_model_base_load_status_get(Eo *obj EINA_UNUSED, Eldbus_Model_Connection_Data *pd)
{
   return pd->load.status;
}

static void
_eldbus_model_connection_efl_model_base_unload(Eo *obj EINA_UNUSED, Eldbus_Model_Connection_Data *pd)
{
   _eldbus_model_connection_clear(pd);

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_UNLOADED);
}

static Eo *
_eldbus_model_connection_efl_model_base_child_add(Eo *obj EINA_UNUSED, Eldbus_Model_Connection_Data *pd EINA_UNUSED)
{
   return NULL;
}

static Efl_Model_Load_Status
_eldbus_model_connection_efl_model_base_child_del(Eo *obj EINA_UNUSED,
                                          Eldbus_Model_Connection_Data *pd EINA_UNUSED,
                                          Eo *child EINA_UNUSED)
{
   return EFL_MODEL_LOAD_STATUS_ERROR;
}

static Efl_Model_Load_Status
_eldbus_model_connection_efl_model_base_children_slice_get(Eo *obj EINA_UNUSED,
                                                   Eldbus_Model_Connection_Data *pd,
                                                   unsigned start,
                                                   unsigned count,
                                                   Eina_Accessor **children_accessor)
{
   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN))
     {
        WRN("(%p): Children not loaded", obj);
        *children_accessor = NULL;
        return pd->load.status;
     }

   *children_accessor = efl_model_list_slice(pd->children_list, start, count);
   return pd->load.status;
}

static Efl_Model_Load_Status
_eldbus_model_connection_efl_model_base_children_count_get(Eo *obj EINA_UNUSED,
                                                   Eldbus_Model_Connection_Data *pd,
                                                   unsigned *children_count)
{
   *children_count = eina_list_count(pd->children_list);
   return pd->load.status;
}

static void
_eldbus_model_connection_efl_model_base_children_load(Eo *obj EINA_UNUSED, Eldbus_Model_Connection_Data *pd)
{
   Eldbus_Pending *pending;

   if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN)
     return;

   if (!pd->connection)
     _eldbus_model_connection_connect(pd);

   pending = eldbus_names_list(pd->connection, &_eldbus_model_connection_names_list_cb, pd);
   pd->pending_list = eina_list_append(pd->pending_list, pending);

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN);
}

static const char *
_eldbus_model_connection_address_get(Eo *obj EINA_UNUSED, Eldbus_Model_Connection_Data *pd)
{
   return pd->address;
}

static void
_eldbus_model_connection_address_set(Eo *obj EINA_UNUSED, Eldbus_Model_Connection_Data *pd, const char *value)
{
   eina_stringshare_del(pd->address);
   pd->address = eina_stringshare_add(value);
}

static Eina_Bool
_eldbus_model_connection_private_get(Eo *obj EINA_UNUSED, Eldbus_Model_Connection_Data *pd)
{
   return pd->private;
}

static void
_eldbus_model_connection_private_set(Eo *obj EINA_UNUSED, Eldbus_Model_Connection_Data *pd, Eina_Bool value)
{
   pd->private = value;
}

static int
_eldbus_model_connection_type_get(Eo *obj EINA_UNUSED, Eldbus_Model_Connection_Data *pd)
{
   return pd->type;
}

static void
_eldbus_model_connection_type_set(Eo *obj EINA_UNUSED, Eldbus_Model_Connection_Data *pd, int value)
{
   pd->type = value;
}

static void
_eldbus_model_connection_connect(Eldbus_Model_Connection_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);

   if (ELDBUS_CONNECTION_TYPE_ADDRESS == pd->type)
     {
        if (pd->private)
          pd->connection = eldbus_address_connection_get(pd->address);
        else
          pd->connection = eldbus_private_address_connection_get(pd->address);
     }
   else
     {
        if (pd->private)
          pd->connection = eldbus_private_connection_get(pd->type);
        else
          pd->connection = eldbus_connection_get(pd->type);
     }

   // TODO: Register for disconnection event

   EINA_SAFETY_ON_FALSE_RETURN(pd->connection != NULL);
}

static void
_eldbus_model_connection_disconnect(Eldbus_Model_Connection_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);
   eldbus_connection_unref(pd->connection);
   pd->connection = NULL;
}

static void
_eldbus_model_connection_clear(Eldbus_Model_Connection_Data *pd)
{
   Eldbus_Pending *pending;
   Eo *child;

   EINA_SAFETY_ON_NULL_RETURN(pd);

   if (!pd->connection)
     return;

   eina_value_free(pd->unique_name);
   pd->unique_name = NULL;

   EINA_LIST_FREE(pd->children_list, child)
     eo_unref(child);

   EINA_LIST_FREE(pd->pending_list, pending)
     eldbus_pending_cancel(pending);

   if (pd->properties_array)
     {
        eina_array_free(pd->properties_array);
        pd->properties_array = NULL;
     }

   _eldbus_model_connection_disconnect(pd);
}

static void
_eldbus_model_connection_names_list_cb(void *data,
                                       const Eldbus_Message *msg,
                                       Eldbus_Pending *pending)
{
   Eldbus_Model_Connection_Data *pd = (Eldbus_Model_Connection_Data*)data;
   const char *error_name, *error_text;
   Eldbus_Message_Iter *array = NULL;
   const char *bus;
   unsigned int count;

   pd->pending_list = eina_list_remove(pd->pending_list, pending);

   if (eldbus_message_error_get(msg, &error_name, &error_text))
     {
        ERR("%s: %s", error_name, error_text);
        efl_model_error_notify(pd->obj);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "as", &array))
     {
        ERR("%s", "Error getting arguments.");
        return;
     }

   while (eldbus_message_iter_get_and_next(array, 's', &bus))
     {
        DBG("(%p): bus = %s", pd->obj, bus);

        Eo *child = eo_add(ELDBUS_MODEL_OBJECT_CLASS, NULL,
          eldbus_model_object_connection_constructor(pd->connection, bus, "/"));

        pd->children_list = eina_list_append(pd->children_list, child);
     }

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   count = eina_list_count(pd->children_list);
   if (count)
     eo_do(pd->obj, eo_event_callback_call(EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED, &count));
}

#include "eldbus_model_connection.eo.c"

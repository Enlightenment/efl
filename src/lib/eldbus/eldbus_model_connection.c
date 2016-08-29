#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "eldbus_model_connection_private.h"
#include "eldbus_model_private.h"

#define MY_CLASS ELDBUS_MODEL_CONNECTION_CLASS
#define MY_CLASS_NAME "Eldbus_Model_Connection"

#define UNIQUE_NAME_PROPERTY "unique_name"


static void _eldbus_model_connection_names_list_cb(void *, const Eldbus_Message *, Eldbus_Pending *);
static void _eldbus_model_connection_connect(Eldbus_Model_Connection_Data *);
static void _eldbus_model_connection_disconnect(Eldbus_Model_Connection_Data *);
static void _eldbus_model_connection_clear(Eldbus_Model_Connection_Data *);

static Efl_Object*
_eldbus_model_connection_efl_object_constructor(Eo *obj, Eldbus_Model_Connection_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->obj = obj;
   pd->is_listed = EINA_FALSE;
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
_eldbus_model_connection_efl_object_destructor(Eo *obj, Eldbus_Model_Connection_Data *pd)
{
   eina_stringshare_del(pd->address);

   _eldbus_model_connection_clear(pd);

   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Array const *
_eldbus_model_connection_efl_model_properties_get(Eo *obj EINA_UNUSED,
                                                    Eldbus_Model_Connection_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->obj, NULL);

   if (pd->properties_array == NULL)
     {
        Eina_Bool ret;

        pd->properties_array = eina_array_new(1);
        EINA_SAFETY_ON_NULL_RETURN_VAL(pd->properties_array, NULL);

        ret = eina_array_push(pd->properties_array, UNIQUE_NAME_PROPERTY);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, NULL);
     }

   return pd->properties_array;
}


static void
_eldbus_model_connection_efl_model_property_set(Eo *obj EINA_UNUSED,
                                             Eldbus_Model_Connection_Data *pd EINA_UNUSED,
                                             const char *property,
                                             Eina_Value const* value EINA_UNUSED,
                                             Eina_Promise_Owner *promise)
{
   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET((strcmp(property, UNIQUE_NAME_PROPERTY) == 0), promise, EFL_MODEL_ERROR_NOT_FOUND, );
   eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_READ_ONLY);
}

static Eina_Promise*
_eldbus_model_connection_efl_model_property_get(Eo *obj EINA_UNUSED,
                                             Eldbus_Model_Connection_Data *pd,
                                             const char *property)
{
   Eina_Promise_Owner *promise = eina_promise_add();
   Eina_Promise *rpromise = eina_promise_owner_promise_get(promise);

   DBG("(%p): property=%s", obj, property);

   if (!pd->connection)
     _eldbus_model_connection_connect(pd);

   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET((strcmp(property, UNIQUE_NAME_PROPERTY) == 0), promise, EFL_MODEL_ERROR_NOT_FOUND, rpromise);

   if (pd->unique_name == NULL)
     {
        const char *unique_name;

        unique_name = eldbus_connection_unique_name_get(pd->connection);
        ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(unique_name, promise, EFL_MODEL_ERROR_NOT_FOUND, rpromise);
        pd->unique_name = strdup(unique_name);
     }

   Eina_Value* v = eina_value_new(EINA_VALUE_TYPE_STRING);
   eina_value_set(v, pd->unique_name);
   eina_promise_owner_value_set(promise, v, (Eina_Promise_Free_Cb)&eina_value_free);
   return rpromise;
}

static Eo *
_eldbus_model_connection_efl_model_child_add(Eo *obj EINA_UNUSED, Eldbus_Model_Connection_Data *pd EINA_UNUSED)
{
   return NULL;
}

static void
_eldbus_model_connection_efl_model_child_del(Eo *obj EINA_UNUSED,
                                          Eldbus_Model_Connection_Data *pd EINA_UNUSED,
                                          Eo *child EINA_UNUSED)
{
}

static Eina_Promise*
_eldbus_model_connection_efl_model_children_slice_get(Eo *obj EINA_UNUSED,
                                                   Eldbus_Model_Connection_Data *pd,
                                                   unsigned int start,
                                                   unsigned int count)
{
   Eina_Promise_Owner *promise;
   Eina_Promise *rpromise;
   _Eldbus_Children_Slice_Promise* data;
   Eldbus_Pending *pending;

   promise = eina_promise_add();
   rpromise = eina_promise_owner_promise_get(promise);

   if (!pd->connection)
     _eldbus_model_connection_connect(pd);

   if (pd->is_listed)
     {
        Eina_Accessor *ac = efl_model_list_slice(pd->children_list, start, count);
        eina_promise_owner_value_set(promise, ac, (Eina_Promise_Free_Cb)&eina_accessor_free);
        return eina_promise_owner_promise_get(promise);
     }

   data = calloc(1, sizeof(struct _Eldbus_Children_Slice_Promise));
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, rpromise);
   data->promise = promise;
   data->start = start;
   data->count = count;

   pd->children_promises = eina_list_prepend(pd->children_promises, data);

   if (pd->pending_list == NULL)
     {
        pending = eldbus_names_list(pd->connection, &_eldbus_model_connection_names_list_cb, pd);
        pd->pending_list = eina_list_append(pd->pending_list, pending);
     }
   return rpromise;
}

static Eina_Promise*
_eldbus_model_connection_efl_model_children_count_get(Eo *obj EINA_UNUSED,
                                                   Eldbus_Model_Connection_Data *pd)
{
   Eina_Promise_Owner *promise;
   Eldbus_Pending *pending;

   promise = eina_promise_add();
   if (!pd->connection)
     _eldbus_model_connection_connect(pd);

   if (pd->is_listed)
     {
        unsigned int *c = calloc(sizeof(unsigned int), 1);
        *c = eina_list_count(pd->children_list);
        eina_promise_owner_value_set(promise, c, free);
        return eina_promise_owner_promise_get(promise);
     }

   pd->count_promises = eina_list_prepend(pd->count_promises, promise);
   if (pd->pending_list == NULL)
     {
        pending = eldbus_names_list(pd->connection, &_eldbus_model_connection_names_list_cb, pd);
        pd->pending_list = eina_list_append(pd->pending_list, pending);
     }
   return eina_promise_owner_promise_get(promise);
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

   free(pd->unique_name);
   pd->unique_name = NULL;

   EINA_LIST_FREE(pd->children_list, child)
     efl_unref(child);

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
   _Eldbus_Children_Slice_Promise * p;
   const char *error_name, *error_text;
   Eldbus_Message_Iter *array = NULL;
   const char *bus;
   unsigned int count;
   Eina_List* i;

   pd->pending_list = eina_list_remove(pd->pending_list, pending);

   if (eldbus_message_error_get(msg, &error_name, &error_text))
     {
        ERR("%s: %s", error_name, error_text);
        //efl_model_error_notify(pd->obj);
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

        Eo *child = efl_add(ELDBUS_MODEL_OBJECT_CLASS, NULL, eldbus_model_object_connection_constructor(efl_self, pd->connection, bus, "/"));

        pd->children_list = eina_list_append(pd->children_list, child);
     }

   count = eina_list_count(pd->children_list);

   if (count)
     efl_event_callback_call(pd->obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, &count);

   pd->is_listed = EINA_TRUE;

   EINA_LIST_FOREACH(pd->children_promises, i, p)
     {
        Eina_Accessor *ac = efl_model_list_slice(pd->children_list, p->start, p->count);
        eina_promise_owner_value_set(p->promise, ac, (Eina_Promise_Free_Cb)&eina_accessor_free);
        free(p);
     }
   eina_list_free(pd->children_promises);

   Eina_Promise_Owner *ep;
   EINA_LIST_FOREACH(pd->count_promises, i, ep)
     {
       unsigned *c = calloc(sizeof(unsigned), 1);
       *c = eina_list_count(pd->children_list);
       eina_promise_owner_value_set(ep, c, free);
     }
   eina_list_free(pd->count_promises);
}


#include "eldbus_model_connection.eo.c"

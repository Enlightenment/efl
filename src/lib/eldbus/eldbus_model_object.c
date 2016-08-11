#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eldbus_model_object_private.h"
#include "eldbus_model_private.h"

#include <Ecore.h>
#include <Eina.h>

#define MY_CLASS ELDBUS_MODEL_OBJECT_CLASS
#define MY_CLASS_NAME "Eldbus_Model_Object"

#define UNIQUE_NAME_PROPERTY "unique_name"

static bool _eldbus_model_object_introspect(Eldbus_Model_Object_Data *, const char *, const char *);
static void _eldbus_model_object_introspect_cb(void *, const Eldbus_Message *, Eldbus_Pending *);
static void _eldbus_model_object_connect(Eldbus_Model_Object_Data *);
static void _eldbus_model_object_disconnect(Eldbus_Model_Object_Data *);
static void _eldbus_model_object_clear(Eldbus_Model_Object_Data *);
static void _eldbus_model_object_introspect_nodes(Eldbus_Model_Object_Data *, const char *, Eina_List *);
static char *_eldbus_model_object_concatenate_path(const char *, const char *);
static void _eldbus_model_object_create_children(Eldbus_Model_Object_Data *, Eldbus_Object *, Eina_List *);

static Efl_Object*
_eldbus_model_object_efl_object_constructor(Eo *obj, Eldbus_Model_Object_Data *pd)
{
   obj = efl_constructor(eo_super(obj, MY_CLASS));

   pd->obj = obj;
   pd->is_listed = EINA_FALSE;
   pd->connection = NULL;
   pd->object_list = NULL;
   pd->properties_array = NULL;
   pd->children_list = NULL;
   pd->type = ELDBUS_CONNECTION_TYPE_UNKNOWN;
   pd->address = NULL;
   pd->private = false;
   pd->bus = NULL;
   pd->path = NULL;
   pd->unique_name = NULL;
   pd->pending_list = NULL;
   pd->introspection = NULL;

   return obj;
}

static void
_eldbus_model_object_constructor(Eo *obj EINA_UNUSED,
                                 Eldbus_Model_Object_Data *pd,
                                 int type,
                                 const char* address,
                                 Eina_Bool private,
                                 const char* bus,
                                 const char* path)
{
   EINA_SAFETY_ON_NULL_RETURN(bus);
   EINA_SAFETY_ON_NULL_RETURN(path);

   pd->type = type;
   pd->address = eina_stringshare_add(address);
   pd->private = private;
   pd->bus = eina_stringshare_add(bus);
   pd->path = eina_stringshare_add(path);
}

static void
_eldbus_model_object_connection_constructor(Eo *obj EINA_UNUSED,
                                            Eldbus_Model_Object_Data *pd,
                                            Eldbus_Connection *connection,
                                            const char* bus,
                                            const char* path)
{
   EINA_SAFETY_ON_NULL_RETURN(connection);
   EINA_SAFETY_ON_NULL_RETURN(bus);
   EINA_SAFETY_ON_NULL_RETURN(path);

   pd->connection = eldbus_connection_ref(connection);
   pd->bus = eina_stringshare_add(bus);
   pd->path = eina_stringshare_add(path);
}

static void
_eldbus_model_object_efl_object_destructor(Eo *obj, Eldbus_Model_Object_Data *pd)
{
   eina_stringshare_del(pd->address);
   eina_stringshare_del(pd->bus);
   eina_stringshare_del(pd->path);

   _eldbus_model_object_clear(pd);

   efl_destructor(eo_super(obj, MY_CLASS));
}

static Eina_Array const *
_eldbus_model_object_efl_model_properties_get(Eo *obj EINA_UNUSED,
                                         Eldbus_Model_Object_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->obj, NULL);

   if (pd->properties_array == NULL)
     {
        Eina_Bool  ret;

        pd->properties_array = eina_array_new(1);
        EINA_SAFETY_ON_NULL_RETURN_VAL(pd->properties_array, NULL);

        ret = eina_array_push(pd->properties_array, UNIQUE_NAME_PROPERTY);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, NULL);
    }

   return pd->properties_array;
}

static void
_eldbus_model_object_efl_model_property_set(Eo *obj EINA_UNUSED,
                                         Eldbus_Model_Object_Data *pd EINA_UNUSED,
                                         const char *property,
                                         const Eina_Value *value EINA_UNUSED,
                                         Eina_Promise_Owner *promise)
{
   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET((strcmp(property, UNIQUE_NAME_PROPERTY) == 0), promise, EFL_MODEL_ERROR_NOT_FOUND, );
   eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_READ_ONLY);
}

static Eina_Promise*
_eldbus_model_object_efl_model_property_get(Eo *obj EINA_UNUSED,
                                         Eldbus_Model_Object_Data *pd,
                                         const char *property)
{
   Eina_Promise_Owner *promise = eina_promise_add();
   Eina_Promise *rpromise = eina_promise_owner_promise_get(promise);

   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(property, promise, EFL_MODEL_ERROR_INCORRECT_VALUE, rpromise);
   DBG("(%p): property=%s", obj, property);

   if (!pd->connection)
     _eldbus_model_object_connect(pd);

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
   return eina_promise_owner_promise_get(promise);
}

static Eo *
_eldbus_model_object_efl_model_child_add(Eo *obj EINA_UNUSED, Eldbus_Model_Object_Data *pd EINA_UNUSED)
{
   return NULL;
}

static void
_eldbus_model_object_efl_model_child_del(Eo *obj EINA_UNUSED,
                                              Eldbus_Model_Object_Data *pd EINA_UNUSED,
                                              Eo *child EINA_UNUSED)
{
}

static Eina_Promise*
_eldbus_model_object_efl_model_children_slice_get(Eo *obj EINA_UNUSED,
                                                       Eldbus_Model_Object_Data *pd,
                                                       unsigned start,
                                                       unsigned count)
{
   _Eldbus_Children_Slice_Promise* p;
   Eina_Promise_Owner *promise = eina_promise_add();
   Eina_Promise *rpromise = eina_promise_owner_promise_get(promise);

   if (!pd->connection)
     _eldbus_model_object_connect(pd);

   if (pd->is_listed)
     {
        Eina_Accessor* ac = efl_model_list_slice(pd->children_list, start, count);
        eina_promise_owner_value_set(promise, ac, (Eina_Promise_Free_Cb)&eina_accessor_free);
        return eina_promise_owner_promise_get(promise);
     }

   p = calloc(1, sizeof(struct _Eldbus_Children_Slice_Promise));
   EINA_SAFETY_ON_NULL_RETURN_VAL(p, rpromise);
   p->promise = promise;
   p->start = start;
   p->count = count;

   pd->children_promises = eina_list_prepend(pd->children_promises, p);
   if (pd->pending_list == NULL)
     _eldbus_model_object_introspect(pd, pd->bus, pd->path);
   return rpromise;
}

static Eina_Promise*
_eldbus_model_object_efl_model_children_count_get(Eo *obj EINA_UNUSED,
                                               Eldbus_Model_Object_Data *pd)
{
   Eina_Promise_Owner *promise = eina_promise_add();
   if (!pd->connection)
     _eldbus_model_object_connect(pd);

   if (pd->is_listed)
     {
        unsigned int *c = calloc(sizeof(unsigned int), 1);
        *c = eina_list_count(pd->children_list);
        eina_promise_owner_value_set(promise, c, free);
        return eina_promise_owner_promise_get(promise);
     }

   pd->count_promises = eina_list_prepend(pd->count_promises, promise);
   if (pd->pending_list == NULL)
     _eldbus_model_object_introspect(pd, pd->bus, pd->path);
   return eina_promise_owner_promise_get(promise);
}

static const char *
_eldbus_model_object_address_get(Eo *obj EINA_UNUSED, Eldbus_Model_Object_Data *pd)
{
   return pd->address;
}

static void
_eldbus_model_object_address_set(Eo *obj EINA_UNUSED, Eldbus_Model_Object_Data *pd, const char *value)
{
   eina_stringshare_del(pd->address);
   pd->address = eina_stringshare_add(value);
}

static Eina_Bool
_eldbus_model_object_private_get(Eo *obj EINA_UNUSED, Eldbus_Model_Object_Data *pd)
{
   return pd->private;
}

static void
_eldbus_model_object_private_set(Eo *obj EINA_UNUSED, Eldbus_Model_Object_Data *pd, Eina_Bool value)
{
   pd->private = value;
}

static int
_eldbus_model_object_type_get(Eo *obj EINA_UNUSED, Eldbus_Model_Object_Data *pd)
{
   return pd->type;
}

static void
_eldbus_model_object_type_set(Eo *obj EINA_UNUSED, Eldbus_Model_Object_Data *pd, int value)
{
   pd->type = value;
}

static const char *
_eldbus_model_object_bus_get(Eo *obj EINA_UNUSED, Eldbus_Model_Object_Data *pd)
{
   return pd->bus;
}

static void
_eldbus_model_object_bus_set(Eo *obj EINA_UNUSED, Eldbus_Model_Object_Data *pd, const char *value)
{
   eina_stringshare_del(pd->bus);
   pd->bus = eina_stringshare_add(value);
}

static const char *
_eldbus_model_object_path_get(Eo *obj EINA_UNUSED, Eldbus_Model_Object_Data *pd)
{
   return pd->path;
}

static void
_eldbus_model_object_path_set(Eo *obj EINA_UNUSED, Eldbus_Model_Object_Data *pd, const char *value)
{
   eina_stringshare_del(pd->path);
   pd->path = eina_stringshare_add(value);
}

static void
_eldbus_model_object_connect(Eldbus_Model_Object_Data *pd)
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

   EINA_SAFETY_ON_FALSE_RETURN(NULL != pd->connection);
}

static void
_eldbus_model_object_disconnect(Eldbus_Model_Object_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);
   eldbus_connection_unref(pd->connection);
   pd->connection = NULL;
}

static void
_eldbus_model_object_clear(Eldbus_Model_Object_Data *pd)
{
   Eldbus_Pending *pending;
   Eldbus_Object *object;
   Eo *child;

   EINA_SAFETY_ON_NULL_RETURN(pd);
   if (!pd->connection)
     return;

   free(pd->unique_name);
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

   EINA_LIST_FREE(pd->object_list, object)
     eldbus_object_unref(object);

   if (pd->introspection)
     {
        eldbus_introspection_node_free(pd->introspection);
        pd->introspection = NULL;
     }

   _eldbus_model_object_disconnect(pd);
}


static bool
_eldbus_model_object_introspect(Eldbus_Model_Object_Data *pd,
                                const char *bus,
                                const char *path)
{
   Eldbus_Object *object;
   Eldbus_Pending *pending;

   EINA_SAFETY_ON_NULL_RETURN_VAL(bus, false);
   EINA_SAFETY_ON_NULL_RETURN_VAL(path, false);

   DBG("(%p) Introspecting: bus = %s, path = %s", pd->obj, bus, path);

   object = eldbus_object_get(pd->connection, bus, path);
   if (!object)
     {
        ERR("(%p): Cannot get object: bus=%s, path=%s", pd->obj, bus, path);
        return false;
     }
   pd->object_list = eina_list_append(pd->object_list, object);

   // TODO: Register for interface added/removed event

   pending = eldbus_object_introspect(object, &_eldbus_model_object_introspect_cb, pd);
   eldbus_pending_data_set(pending, "object", object);
   pd->pending_list = eina_list_append(pd->pending_list, pending);
   return true;
}

static void
_eldbus_model_object_introspect_cb(void *data,
                                   const Eldbus_Message *msg,
                                   Eldbus_Pending *pending)
{
   Eldbus_Model_Object_Data *pd = (Eldbus_Model_Object_Data*)data;
   Eldbus_Object *object;
   const char *error_name, *error_text;
   const char *xml = NULL;
   const char *current_path;

   pd->pending_list = eina_list_remove(pd->pending_list, pending);
   object = eldbus_pending_data_get(pending, "object");

   if (eldbus_message_error_get(msg, &error_name, &error_text))
     {
        ERR("%s: %s", error_name, error_text);
        //efl_model_error_notify(pd->obj);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "s", &xml))
     {
        ERR("Error getting arguments.");
        return;
     }
   EINA_SAFETY_ON_NULL_RETURN(xml);

   current_path = eldbus_object_path_get(object);
   EINA_SAFETY_ON_NULL_RETURN(current_path);

   DBG("(%p): introspect of bus = %s, path = %s =>\n%s", pd->obj, pd->bus, current_path, xml);

   pd->introspection = eldbus_introspection_parse(xml);
   EINA_SAFETY_ON_NULL_RETURN(pd->introspection);

   _eldbus_model_object_introspect_nodes(pd, current_path, pd->introspection->nodes);
   _eldbus_model_object_create_children(pd, object, pd->introspection->interfaces);

   if (eina_list_count(pd->pending_list) == 0)
     {
        Eina_List* i;

        pd->is_listed = EINA_TRUE;
        _Eldbus_Children_Slice_Promise* p;
        EINA_LIST_FOREACH(pd->children_promises, i, p)
          {
            Eina_Accessor* ac = efl_model_list_slice(pd->children_list, p->start, p->count);
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
}

static void
_eldbus_model_object_introspect_nodes(Eldbus_Model_Object_Data *pd, const char *current_path, Eina_List *nodes)
{
   Eina_List *it;
   Eldbus_Introspection_Node *node;

   EINA_SAFETY_ON_NULL_RETURN(pd);
   EINA_SAFETY_ON_NULL_RETURN(current_path);

   EINA_LIST_FOREACH(nodes, it, node)
     {
        const char *relative_path;
        char *absolute_path;

        relative_path = node->name;
        if (!relative_path) continue;

        absolute_path = _eldbus_model_object_concatenate_path(current_path, relative_path);
        if (!absolute_path) continue;

        _eldbus_model_object_introspect(pd, pd->bus, absolute_path);

        free(absolute_path);
     }
}

static char *
_eldbus_model_object_concatenate_path(const char *root_path, const char *relative_path)
{
   Eina_Strbuf *buffer;
   char *absolute_path = NULL;
   Eina_Bool ret;

   buffer = eina_strbuf_new();
   EINA_SAFETY_ON_NULL_RETURN_VAL(buffer, NULL);

   ret = eina_strbuf_append(buffer, root_path);
   if (strcmp(root_path, "/") != 0)
     ret = ret && eina_strbuf_append_char(buffer, '/');
   ret = ret && eina_strbuf_append(buffer, relative_path);
   EINA_SAFETY_ON_FALSE_GOTO(ret, free_buffer);

   absolute_path = eina_strbuf_string_steal(buffer);

free_buffer:
   eina_strbuf_free(buffer);
   return absolute_path;
}

static void
_eldbus_model_object_create_children(Eldbus_Model_Object_Data *pd, Eldbus_Object *object, Eina_List *interfaces)
{
   Eldbus_Introspection_Interface *interface;
   Eina_List *it;
   const char *current_path;

   current_path = eldbus_object_path_get(object);
   EINA_SAFETY_ON_NULL_RETURN(current_path);

   EINA_LIST_FOREACH(interfaces, it, interface)
     {
        Eo *child;

        WRN("(%p) Creating child: bus = %s, path = %s, interface = %s", pd->obj, pd->bus, current_path, interface->name);

        // TODO: increment reference to keep 'interface' in memory
        child = eo_add_ref(ELDBUS_MODEL_PROXY_CLASS, NULL, eldbus_model_proxy_constructor(eo_self, object, interface));

        pd->children_list = eina_list_append(pd->children_list, child);
     }
}

#include "eldbus_model_object.eo.c"

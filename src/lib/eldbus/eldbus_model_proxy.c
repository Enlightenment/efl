#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eldbus_model_proxy_private.h"
#include "eldbus_model_private.h"

#include <Eina.h>

#define MY_CLASS ELDBUS_MODEL_PROXY_CLASS
#define MY_CLASS_NAME "Eldbus_Model_Proxy"

static void _eldbus_model_proxy_efl_model_base_properties_load(Eo *, Eldbus_Model_Proxy_Data *);
static void _eldbus_model_proxy_efl_model_base_children_load(Eo *, Eldbus_Model_Proxy_Data *);
static bool _eldbus_model_proxy_load(Eldbus_Model_Proxy_Data *);
static void _eldbus_model_proxy_unload(Eldbus_Model_Proxy_Data *);
static void _eldbus_model_proxy_property_get_all_cb(void *, const Eldbus_Message *, Eldbus_Pending *);
static void _eldbus_model_proxy_property_set_cb(void *, const Eldbus_Message *, Eldbus_Pending *);
static void _eldbus_model_proxy_start_monitor(Eldbus_Model_Proxy_Data *);
static void _eldbus_model_proxy_stop_monitor(Eldbus_Model_Proxy_Data *);
static void _eldbus_model_proxy_property_changed_cb(void *, Eldbus_Proxy *, void *);
static void _eldbus_model_proxy_property_invalidated_cb(void *, Eldbus_Proxy *, void *);
static bool _eldbus_model_proxy_is_property_writeable(Eldbus_Model_Proxy_Data *, const char *);
static bool _eldbus_model_proxy_is_property_readable(Eldbus_Model_Proxy_Data *, const char *);
static const char *_eldbus_model_proxy_property_type_get(Eldbus_Model_Proxy_Data *, const char *);
static void _eldbus_model_proxy_create_methods_children(Eldbus_Model_Proxy_Data *);
static void _eldbus_model_proxy_create_signals_children(Eldbus_Model_Proxy_Data *);

typedef struct _Eldbus_Model_Proxy_Property_Set_Data Eldbus_Model_Proxy_Property_Set_Data;

struct _Eldbus_Model_Proxy_Property_Set_Data
{
   Eldbus_Model_Proxy_Data *pd;
   Eina_Stringshare *property;
   Eina_Value value;
};

static Eldbus_Model_Proxy_Property_Set_Data * _eldbus_model_proxy_property_set_data_new(Eldbus_Model_Proxy_Data *, const char *, const Eina_Value *);
static void _eldbus_model_proxy_property_set_data_free(Eldbus_Model_Proxy_Property_Set_Data *);

static void
_eldbus_model_proxy_hash_free(Eina_Value *value)
{
   eina_value_free(value);
}

static Eo_Base*
_eldbus_model_proxy_eo_base_constructor(Eo *obj, Eldbus_Model_Proxy_Data *pd)
{
   obj = eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());

   pd->obj = obj;
   pd->load.status = EFL_MODEL_LOAD_STATUS_UNLOADED;
   pd->object = NULL;
   pd->proxy = NULL;
   pd->properties_array = NULL;
   pd->properties_hash = eina_hash_string_superfast_new(EINA_FREE_CB(_eldbus_model_proxy_hash_free));
   pd->children_list = NULL;
   pd->name = NULL;
   pd->pending_list = NULL;
   pd->monitoring = false;
   pd->interface = NULL;

   return obj;
}

static void
_eldbus_model_proxy_constructor(Eo *obj EINA_UNUSED,
                                Eldbus_Model_Proxy_Data *pd,
                                Eldbus_Object *object,
                                const Eldbus_Introspection_Interface *interface)
{
   EINA_SAFETY_ON_NULL_RETURN(object);
   EINA_SAFETY_ON_NULL_RETURN(interface);

   pd->object = eldbus_object_ref(object);
   pd->name = eina_stringshare_add(interface->name);
   pd->interface = interface;
}

static void
_eldbus_model_proxy_eo_base_destructor(Eo *obj, Eldbus_Model_Proxy_Data *pd)
{
   _eldbus_model_proxy_unload(pd);

   eina_hash_free(pd->properties_hash);

   eina_stringshare_del(pd->name);
   eldbus_object_unref(pd->object);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Efl_Model_Load_Status
_eldbus_model_proxy_efl_model_base_properties_get(Eo *obj EINA_UNUSED,
                                               Eldbus_Model_Proxy_Data *pd,
                                               Eina_Array * const* properties_array)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EFL_MODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->obj, EFL_MODEL_LOAD_STATUS_ERROR);

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
     {
        WRN("%s", "Properties not loaded.");
        return EFL_MODEL_LOAD_STATUS_ERROR;
     }

   *(Eina_Array**)properties_array = pd->properties_array;
   return pd->load.status;
}

static void
_eldbus_model_proxy_efl_model_base_properties_load(Eo *obj EINA_UNUSED, Eldbus_Model_Proxy_Data *pd)
{
   Eldbus_Introspection_Property *property;
   Eina_List *it;
   Eldbus_Pending *pending;

   if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES)
     return;

   if (!_eldbus_model_proxy_load(pd))
     return;

   const unsigned int properties_count = eina_list_count(pd->interface->properties);

   pd->properties_array = eina_array_new(properties_count);
   EINA_SAFETY_ON_NULL_RETURN(pd->properties_array);

   if (!properties_count)
     {
        efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);
        return;
     }

   EINA_LIST_FOREACH(pd->interface->properties, it, property)
     {
        Eina_Stringshare *name;
        Eina_Bool ret;

        name = eina_stringshare_add(property->name);
        ret = eina_array_push(pd->properties_array, name);
        EINA_SAFETY_ON_FALSE_RETURN(ret);
     }

   pending = eldbus_proxy_property_get_all(pd->proxy, _eldbus_model_proxy_property_get_all_cb, pd);
   pd->pending_list = eina_list_append(pd->pending_list, pending);

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADING_PROPERTIES);
}

static Efl_Model_Load_Status
_eldbus_model_proxy_efl_model_base_property_set(Eo *obj EINA_UNUSED,
                                        Eldbus_Model_Proxy_Data *pd,
                                        const char *property,
                                        Eina_Value const* value)
{
   Eldbus_Model_Proxy_Property_Set_Data *data;
   const char *signature;
   Eldbus_Pending *pending;

   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EFL_MODEL_LOAD_STATUS_ERROR);

   DBG("(%p): property=%s", obj, property);

   if (!_eldbus_model_proxy_is_property_writeable(pd, property))
     {
        WRN("Property is read-only: %s", property);
        return EFL_MODEL_LOAD_STATUS_ERROR;
     }

   data = _eldbus_model_proxy_property_set_data_new(pd, property, value);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, EFL_MODEL_LOAD_STATUS_ERROR);

   signature = _eldbus_model_proxy_property_type_get(pd, property);
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EFL_MODEL_LOAD_STATUS_ERROR);

   pending = eldbus_proxy_property_value_set
     (pd->proxy, property, signature, (Eina_Value*)value, _eldbus_model_proxy_property_set_cb, data);
   pd->pending_list = eina_list_append(pd->pending_list, pending);

   return pd->load.status;
}

static Efl_Model_Load_Status
_eldbus_model_proxy_efl_model_base_property_get(Eo *obj EINA_UNUSED,
                                        Eldbus_Model_Proxy_Data *pd,
                                        const char *property,
                                        Eina_Value const **value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EFL_MODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EFL_MODEL_LOAD_STATUS_ERROR);

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
     return EFL_MODEL_LOAD_STATUS_ERROR;

   if (!_eldbus_model_proxy_is_property_readable(pd, property))
     {
        WRN("Property is write-only: %s", property);
        return EFL_MODEL_LOAD_STATUS_ERROR;
     }

   *value = eina_hash_find(pd->properties_hash, property);

   if(!*value)
     {
        *value = &pd->tmp_value;
        return EFL_MODEL_LOAD_STATUS_ERROR;
     }

   return pd->load.status;
}

static void
_eldbus_model_proxy_efl_model_base_load(Eo *obj EINA_UNUSED, Eldbus_Model_Proxy_Data *pd)
{
   if (!_eldbus_model_proxy_load(pd))
     return;

   _eldbus_model_proxy_efl_model_base_properties_load(obj, pd);
   _eldbus_model_proxy_efl_model_base_children_load(obj, pd);
}

static Efl_Model_Load_Status
_eldbus_model_proxy_efl_model_base_load_status_get(Eo *obj EINA_UNUSED, Eldbus_Model_Proxy_Data *pd)
{
   return pd->load.status;
}

static void
_eldbus_model_proxy_efl_model_base_unload(Eo *obj EINA_UNUSED, Eldbus_Model_Proxy_Data *pd)
{
   _eldbus_model_proxy_unload(pd);

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_UNLOADED);
}

static Eo *
_eldbus_model_proxy_efl_model_base_child_add(Eo *obj EINA_UNUSED, Eldbus_Model_Proxy_Data *pd EINA_UNUSED)
{
   return NULL;
}

static Efl_Model_Load_Status
_eldbus_model_proxy_efl_model_base_child_del(Eo *obj EINA_UNUSED,
                                     Eldbus_Model_Proxy_Data *pd EINA_UNUSED,
                                     Eo *child EINA_UNUSED)
{
   return EFL_MODEL_LOAD_STATUS_ERROR;
}

static Efl_Model_Load_Status
_eldbus_model_proxy_efl_model_base_children_slice_get(Eo *obj EINA_UNUSED,
                                              Eldbus_Model_Proxy_Data *pd,
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
_eldbus_model_proxy_efl_model_base_children_count_get(Eo *obj EINA_UNUSED,
                                              Eldbus_Model_Proxy_Data *pd,
                                              unsigned *children_count)
{
   *children_count = eina_list_count(pd->children_list);
   return pd->load.status;
}

static void
_eldbus_model_proxy_efl_model_base_children_load(Eo *obj EINA_UNUSED, Eldbus_Model_Proxy_Data *pd)
{
   unsigned int count;

   if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN)
     return;

   if (!_eldbus_model_proxy_load(pd))
     return;

   _eldbus_model_proxy_create_methods_children(pd);
   _eldbus_model_proxy_create_signals_children(pd);

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);

   count = eina_list_count(pd->children_list);
   if (count)
     eo_do(pd->obj, eo_event_callback_call(EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED, &count));
}

static void
_eldbus_model_proxy_create_methods_children(Eldbus_Model_Proxy_Data *pd)
{
   Eina_List *it;
   Eldbus_Introspection_Method *method;

   EINA_LIST_FOREACH(pd->interface->methods, it, method)
     {
        const char *bus;
        const char *path;
        const char *interface_name;
        const char *method_name;
        Eo *child;

        bus = eldbus_object_bus_name_get(pd->object);
        if (!bus) continue;

        path = eldbus_object_path_get(pd->object);
        if (!path) continue;

        interface_name = pd->interface->name;
        if (!interface_name) continue;

        method_name = method->name;
        if (!method_name) continue;

        INF("(%p) Creating method child: bus = %s, path = %s, method = %s::%s", pd->obj, bus, path, interface_name, method_name);

        child = eo_add(ELDBUS_MODEL_METHOD_CLASS, NULL,
                       eldbus_model_method_constructor(pd->proxy, method));

        pd->children_list = eina_list_append(pd->children_list, child);
     }
}

static void
_eldbus_model_proxy_create_signals_children(Eldbus_Model_Proxy_Data *pd)
{
   Eina_List *it;
   Eldbus_Introspection_Signal *signal;

   EINA_LIST_FOREACH(pd->interface->signals, it, signal)
     {
        const char *bus;
        const char *path;
        const char *interface_name;
        const char *signal_name;
        Eo *child;

        bus = eldbus_object_bus_name_get(pd->object);
        if (!bus) continue;

        path = eldbus_object_path_get(pd->object);
        if (!path) continue;

        interface_name = pd->interface->name;
        if (!interface_name) continue;

        signal_name = signal->name;
        if (!signal_name) continue;

        DBG("(%p) Creating signal child: bus = %s, path = %s, signal = %s::%s", pd->obj, bus, path, interface_name, signal_name);

        child = eo_add(ELDBUS_MODEL_SIGNAL_CLASS, NULL,
                       eldbus_model_signal_constructor(pd->proxy, signal));

        pd->children_list = eina_list_append(pd->children_list, child);
     }
}

static const char *
_eldbus_model_proxy_name_get(Eo *obj EINA_UNUSED, Eldbus_Model_Proxy_Data *pd)
{
   return pd->name;
}

static bool
_eldbus_model_proxy_load(Eldbus_Model_Proxy_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, false);

   if (pd->proxy)
     return true;

   pd->proxy = eldbus_proxy_get(pd->object, pd->name);
   if (!pd->proxy)
     {
        ERR("Cannot get proxy for interface: %s", pd->name);
        return false;
     }

   return true;
}

static void
_eldbus_model_proxy_unload(Eldbus_Model_Proxy_Data *pd)
{
   Eldbus_Pending *pending;
   Eo *child;

   EINA_SAFETY_ON_NULL_RETURN(pd);

   EINA_LIST_FREE(pd->children_list, child)
     eo_unref(child);

   EINA_LIST_FREE(pd->pending_list, pending)
     eldbus_pending_cancel(pending);

   if (pd->properties_array)
     {
        unsigned int i;
        Eina_Stringshare *property;
        Eina_Array_Iterator it;

        EINA_ARRAY_ITER_NEXT(pd->properties_array, i, property, it)
          eina_stringshare_del(property);
        eina_array_free(pd->properties_array);
        pd->properties_array = NULL;
     }

   eina_hash_free_buckets(pd->properties_hash);

   _eldbus_model_proxy_stop_monitor(pd);

   if (pd->proxy)
     {
        eldbus_proxy_unref(pd->proxy);
        pd->proxy = NULL;
     }
}

static void
_eldbus_model_proxy_start_monitor(Eldbus_Model_Proxy_Data *pd)
{
   if (pd->monitoring)
     return;

   pd->monitoring = true;

   eldbus_proxy_event_callback_add(pd->proxy,
                                   ELDBUS_PROXY_EVENT_PROPERTY_CHANGED,
                                   _eldbus_model_proxy_property_changed_cb,
                                   pd);

   eldbus_proxy_event_callback_add(pd->proxy,
                                   ELDBUS_PROXY_EVENT_PROPERTY_REMOVED,
                                   _eldbus_model_proxy_property_invalidated_cb,
                                   pd);
}

static void
_eldbus_model_proxy_stop_monitor(Eldbus_Model_Proxy_Data *pd)
{
   if (!pd->monitoring)
     return;

   pd->monitoring = false;

   eldbus_proxy_event_callback_del(pd->proxy,
                                   ELDBUS_PROXY_EVENT_PROPERTY_CHANGED,
                                   _eldbus_model_proxy_property_changed_cb,
                                   pd);

   eldbus_proxy_event_callback_del(pd->proxy,
                                   ELDBUS_PROXY_EVENT_PROPERTY_REMOVED,
                                   _eldbus_model_proxy_property_invalidated_cb,
                                   pd);
}

static void
_eldbus_model_proxy_property_changed_cb(void *data,
                                        Eldbus_Proxy *proxy EINA_UNUSED,
                                        void *event_info)
{
   Eldbus_Model_Proxy_Data *pd = (Eldbus_Model_Proxy_Data*)data;
   Eldbus_Object_Event_Property_Changed *event = (Eldbus_Object_Event_Property_Changed*)event_info;
   Eina_Value *prop_value;
   Eina_Bool ret;

   prop_value = eina_hash_find(pd->properties_hash, event->name);
   if (!prop_value) return ;

   ret = eina_value_copy(event->value, prop_value);
   if (!ret) return ;

   efl_model_property_changed_notify(pd->obj, event->name);
}

static void
_eldbus_model_proxy_property_invalidated_cb(void *data,
                                            Eldbus_Proxy *proxy EINA_UNUSED,
                                            void *event_info)
{
   Eldbus_Model_Proxy_Data *pd = (Eldbus_Model_Proxy_Data*)data;
   Eldbus_Proxy_Event_Property_Changed *event = (Eldbus_Proxy_Event_Property_Changed*)event_info;

#if 0
   Efl_Model_Property_Event evt = {0};

   // TODO: eldbus_proxy_property_get(event->name) ?

   evt.invalidated_properties = eina_array_new(1);
   EINA_SAFETY_ON_NULL_RETURN(evt.invalidated_properties);

   eina_array_push(evt.invalidated_properties, event->name);
   eina_array_free(evt.invalidated_properties);
#endif

   efl_model_property_invalidated_notify(pd->obj, event->name);
}

static void
_eldbus_model_proxy_property_get_all_cb(void *data,
                                        const Eldbus_Message *msg,
                                        Eldbus_Pending *pending)
{
   Eldbus_Model_Proxy_Data *pd = (Eldbus_Model_Proxy_Data*)data;
   Eldbus_Introspection_Property *prop;
   Eina_Array *changed_properties;
   Eldbus_Message_Iter *values = NULL;
   Eldbus_Message_Iter *entry;
   Eina_List *it;
   const char *error_name, *error_text;

   pd->pending_list = eina_list_remove(pd->pending_list, pending);

   if (eldbus_message_error_get(msg, &error_name, &error_text))
     {
        ERR("%s: %s", error_name, error_text);
        efl_model_error_notify(pd->obj);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "a{sv}", &values))
     {
        ERR("%s", "Error getting arguments.");
        return;
     }

   EINA_LIST_FOREACH(pd->interface->properties, it, prop)
     {
        const Eina_Value_Type *type;
        Eina_Value *value;

        type = _dbus_type_to_eina_value_type(prop->type[0]);
        value = eina_value_new(type);

        eina_hash_add(pd->properties_hash, prop->name, value);
     }

   changed_properties = eina_array_new(1);
   while (eldbus_message_iter_get_and_next(values, 'e', &entry))
     {
        const char *property;
        Eldbus_Message_Iter *variant;
        Eina_Value *struct_value;
        Eina_Value *prop_value;
        Eina_Value arg0;
        Eina_Bool ret;

        if (!eldbus_message_iter_arguments_get(entry, "sv", &property, &variant))
          continue;

        struct_value = eldbus_message_iter_struct_like_to_eina_value(variant);
        EINA_SAFETY_ON_NULL_GOTO(struct_value, on_error);

        ret = eina_value_struct_value_get(struct_value, "arg0", &arg0);
        eina_value_free(struct_value);
        EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);

        prop_value = eina_hash_find(pd->properties_hash, property);
        EINA_SAFETY_ON_NULL_GOTO(prop_value, on_error);

        ret = eina_value_copy(&arg0, prop_value);
        EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);

        eina_value_flush(&arg0);

        ret = eina_array_push(changed_properties, property);
        EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);
     }

   _eldbus_model_proxy_start_monitor(pd);

   if (eina_array_count(changed_properties))
     {
        Efl_Model_Property_Event evt = {
          .changed_properties = changed_properties
        };

        eo_do(pd->obj, eo_event_callback_call(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, &evt));
     }

   efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);

 on_error:
   eina_array_free(changed_properties);
}


static void
_eldbus_model_proxy_property_set_cb(void *data,
                                    const Eldbus_Message *msg,
                                    Eldbus_Pending *pending)
{
   Eldbus_Model_Proxy_Property_Set_Data *property_set_data = (Eldbus_Model_Proxy_Property_Set_Data *)data;
   Eldbus_Model_Proxy_Data *pd = property_set_data->pd;
   const char *error_name, *error_text;
   Eina_Value *prop_value;
   Eina_Bool ret;

   pd->pending_list = eina_list_remove(pd->pending_list, pending);

   if (eldbus_message_error_get(msg, &error_name, &error_text))
     {
        ERR("%s: %s", error_name, error_text);
        efl_model_error_notify(pd->obj);
        goto on_error;
     }

   prop_value = eina_hash_find(pd->properties_hash,
                               property_set_data->property);
   EINA_SAFETY_ON_NULL_GOTO(prop_value, on_error);

   ret = eina_value_copy(&property_set_data->value, prop_value);
   EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);

   {
      Efl_Model_Property_Event evt = {
        .changed_properties = pd->properties_array
      };
      eo_do(pd->obj, eo_event_callback_call(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, &evt));

      efl_model_property_changed_notify(pd->obj, property_set_data->property);
   }

 on_error:
   _eldbus_model_proxy_property_set_data_free(property_set_data);
}

static bool
_eldbus_model_proxy_is_property_writeable(Eldbus_Model_Proxy_Data *pd, const char *property)
{
   Eldbus_Introspection_Property *property_introspection =
     eldbus_introspection_property_find(pd->interface->properties, property);

   if (property_introspection == NULL)
     {
        WRN("Property not found: %s", property);
        return false;
     }

   return ELDBUS_INTROSPECTION_PROPERTY_ACCESS_WRITE == property_introspection->access
       || ELDBUS_INTROSPECTION_PROPERTY_ACCESS_READWRITE == property_introspection->access;
}

static bool
_eldbus_model_proxy_is_property_readable(Eldbus_Model_Proxy_Data *pd, const char *property)
{
   Eldbus_Introspection_Property *property_introspection =
     eldbus_introspection_property_find(pd->interface->properties, property);

   if (property_introspection == NULL)
     {
        WRN("Property not found: %s", property);
        return false;
     }

   return ELDBUS_INTROSPECTION_PROPERTY_ACCESS_READ == property_introspection->access
       || ELDBUS_INTROSPECTION_PROPERTY_ACCESS_READWRITE == property_introspection->access;
}

static const char *
_eldbus_model_proxy_property_type_get(Eldbus_Model_Proxy_Data *pd, const char *property)
{
   Eldbus_Introspection_Property *property_introspection =
     eldbus_introspection_property_find(pd->interface->properties, property);

   if (property_introspection == NULL)
     {
        WRN("Property not found: %s", property);
        return NULL;
     }

   return property_introspection->type;

}

static Eldbus_Model_Proxy_Property_Set_Data *
_eldbus_model_proxy_property_set_data_new(Eldbus_Model_Proxy_Data *pd,
                                          const char *property,
                                          const Eina_Value *value)
{
   Eldbus_Model_Proxy_Property_Set_Data *data = calloc(1, sizeof(Eldbus_Model_Proxy_Property_Set_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);

   data->pd = pd;
   data->property = eina_stringshare_add(property);
   if (!eina_value_copy(value, &data->value))
     goto error;

   return data;

 error:
   eina_stringshare_del(data->property);
   free(data);
   return NULL;
}

static void
_eldbus_model_proxy_property_set_data_free(Eldbus_Model_Proxy_Property_Set_Data *data)
{
   EINA_SAFETY_ON_NULL_RETURN(data);
   eina_stringshare_del(data->property);
   eina_value_flush(&data->value);
   free(data);
}

#include "eldbus_model_proxy.eo.c"

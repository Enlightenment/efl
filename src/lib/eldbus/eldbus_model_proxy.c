#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>

#include "eldbus_model_proxy_private.h"
#include "eldbus_model_private.h"

#define MY_CLASS ELDBUS_MODEL_PROXY_CLASS
#define MY_CLASS_NAME "Eldbus_Model_Proxy"

static Eina_Bool _eldbus_model_proxy_load(Eldbus_Model_Proxy_Data *);
static void _eldbus_model_proxy_unload(Eldbus_Model_Proxy_Data *);
static void _eldbus_model_proxy_property_get_all_cb(void *, const Eldbus_Message *, Eldbus_Pending *);
static void _eldbus_model_proxy_property_set_cb(void *, const Eldbus_Message *, Eldbus_Pending *);
static void _eldbus_model_proxy_property_set_load_cb(void *, const Eldbus_Message *, Eldbus_Pending *);
static void _eldbus_model_proxy_start_monitor(Eldbus_Model_Proxy_Data *);
static void _eldbus_model_proxy_stop_monitor(Eldbus_Model_Proxy_Data *);
static void _eldbus_model_proxy_property_changed_cb(void *, Eldbus_Proxy *, void *);
static void _eldbus_model_proxy_property_invalidated_cb(void *, Eldbus_Proxy *, void *);
static bool _eldbus_model_proxy_is_property_writeable(Eldbus_Model_Proxy_Data *, const char *);
static bool _eldbus_model_proxy_has_property(Eldbus_Model_Proxy_Data *, const char *);
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
   Efl_Promise *promise;
};

static Eldbus_Model_Proxy_Property_Set_Data * _eldbus_model_proxy_property_set_data_new(Eldbus_Model_Proxy_Data *, const char *, const Eina_Value *, Efl_Promise *promise);
static void _eldbus_model_proxy_property_set_data_free(Eldbus_Model_Proxy_Property_Set_Data *);

static void
_eldbus_model_proxy_hash_free(Eina_Value *value)
{
   eina_value_free(value);
}

static Efl_Object*
_eldbus_model_proxy_efl_object_constructor(Eo *obj, Eldbus_Model_Proxy_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->obj = obj;
   pd->object = NULL;
   pd->proxy = NULL;
   pd->is_listed = pd->is_loaded = EINA_FALSE;
   pd->properties_array = NULL;
   pd->properties_hash = eina_hash_string_superfast_new(EINA_FREE_CB(_eldbus_model_proxy_hash_free));
   pd->children_list = NULL;
   pd->name = NULL;
   pd->pending_list = NULL;
   pd->promise_list = NULL;
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
_eldbus_model_proxy_efl_object_destructor(Eo *obj, Eldbus_Model_Proxy_Data *pd)
{
   _eldbus_model_proxy_unload(pd);

   eina_hash_free(pd->properties_hash);

   eina_stringshare_del(pd->name);
   eldbus_object_unref(pd->object);

   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Array const *
_eldbus_model_proxy_efl_model_properties_get(Eo *obj EINA_UNUSED,
                                               Eldbus_Model_Proxy_Data *pd)
{
   Eina_Bool ret;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, NULL);

   ret = _eldbus_model_proxy_load(pd);
   if (!ret) return NULL;

   return pd->properties_array;
}

static Efl_Future*
_eldbus_model_proxy_efl_model_property_set(Eo *obj EINA_UNUSED,
                                        Eldbus_Model_Proxy_Data *pd,
                                        const char *property,
                                           Eina_Value const* value)
{
   Eldbus_Model_Proxy_Property_Set_Data *data;
   const char *signature;
   Eldbus_Pending *pending;
   Eina_Bool ret;
   Efl_Promise* promise = efl_add(EFL_PROMISE_CLASS, obj);
   Efl_Future* future = efl_promise_future_get(promise);

   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(property, promise, EFL_MODEL_ERROR_INCORRECT_VALUE, future);

   DBG("(%p): property=%s", obj, property);
   ret = _eldbus_model_proxy_load(pd);
   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(ret, promise, EFL_MODEL_ERROR_INIT_FAILED, future);

   ret = _eldbus_model_proxy_has_property(pd, property);
   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(ret, promise, EFL_MODEL_ERROR_NOT_FOUND, future);

   ret = _eldbus_model_proxy_is_property_writeable(pd, property);
   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(ret, promise, EFL_MODEL_ERROR_READ_ONLY, future);

   signature = _eldbus_model_proxy_property_type_get(pd, property);
   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(signature, promise, EFL_MODEL_ERROR_UNKNOWN, future);

   data = _eldbus_model_proxy_property_set_data_new(pd, property, value, promise);
   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(data, promise, EFL_MODEL_ERROR_UNKNOWN, future);

   if (!pd->is_loaded)
     {
        _Eldbus_Property_Promise *p = calloc(1, sizeof(_Eldbus_Property_Promise));
        EINA_SAFETY_ON_NULL_RETURN_VAL(p, future);

        p->promise = promise;
        p->property = strdup(property);
        pd->promise_list = eina_list_append(pd->promise_list, p);

        if (!pd->pending_list)
          {
             pending = eldbus_proxy_property_get_all(pd->proxy, _eldbus_model_proxy_property_set_load_cb, data);
             pd->pending_list = eina_list_append(pd->pending_list, pending);
          }
        return future;
     }

   pending = eldbus_proxy_property_value_set
     (pd->proxy, property, signature, (Eina_Value*)value, _eldbus_model_proxy_property_set_cb, data);
   pd->pending_list = eina_list_append(pd->pending_list, pending);
   return future;
}

static Efl_Future*
_eldbus_model_proxy_efl_model_property_get(Eo *obj EINA_UNUSED,
                                        Eldbus_Model_Proxy_Data *pd,
                                        const char *property)
{
   Eina_Bool ret;
   Eina_Value *promise_value;
   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, obj);
   Efl_Future *future = efl_promise_future_get(promise);

   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(property, promise, EFL_MODEL_ERROR_INCORRECT_VALUE, future);

   ret = _eldbus_model_proxy_load(pd);
   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(ret, promise, EFL_MODEL_ERROR_INIT_FAILED, future);

   ret = _eldbus_model_proxy_has_property(pd, property);
   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(ret, promise, EFL_MODEL_ERROR_NOT_FOUND, future);

   if (!pd->is_loaded)
     {
        Eldbus_Pending *pending;
        _Eldbus_Property_Promise *p = calloc(1, sizeof(_Eldbus_Property_Promise));
        EINA_SAFETY_ON_NULL_RETURN_VAL(p, future);

        p->promise = promise;
        p->property = strdup(property);
        pd->promise_list = eina_list_append(pd->promise_list, p);

        if (!pd->pending_list)
          {
             pending = eldbus_proxy_property_get_all(pd->proxy, _eldbus_model_proxy_property_get_all_cb, pd);
             pd->pending_list = eina_list_append(pd->pending_list, pending);
          }
        return future;
     }

   Eina_Value* value = eina_hash_find(pd->properties_hash, property);
   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(value, promise, EFL_MODEL_ERROR_NOT_FOUND, future);

   ret = _eldbus_model_proxy_is_property_writeable(pd, property);
   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(ret, promise, EFL_MODEL_ERROR_READ_ONLY, future);

   promise_value = eina_value_new(eina_value_type_get(value));
   eina_value_copy(value, promise_value);
   efl_promise_value_set(promise, promise_value, (Eina_Free_Cb)&eina_value_free);
   return future;
}

static Eo *
_eldbus_model_proxy_efl_model_child_add(Eo *obj EINA_UNUSED,
                                        Eldbus_Model_Proxy_Data *pd EINA_UNUSED)
{
   return NULL;
}

static void
_eldbus_model_proxy_efl_model_child_del(Eo *obj EINA_UNUSED,
                                     Eldbus_Model_Proxy_Data *pd EINA_UNUSED,
                                     Eo *child EINA_UNUSED)
{
}

static Efl_Future*
_eldbus_model_proxy_efl_model_children_slice_get(Eo *obj EINA_UNUSED,
                                              Eldbus_Model_Proxy_Data *pd,
                                              unsigned start,
                                              unsigned count)
{
   Eina_Bool ret = _eldbus_model_proxy_load(pd);
   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, obj);
   Efl_Future *future = efl_promise_future_get(promise);
   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(ret, promise, EFL_MODEL_ERROR_INIT_FAILED, future);

   if (!pd->is_listed)
     {
        _eldbus_model_proxy_create_methods_children(pd);
        _eldbus_model_proxy_create_signals_children(pd);
        pd->is_listed = EINA_TRUE;
     }

   Eina_Accessor *ac = efl_model_list_slice(pd->children_list, start, count);
   efl_promise_value_set(promise, ac, (Eina_Free_Cb)&eina_accessor_free);
   return future;
}

static Efl_Future*
_eldbus_model_proxy_efl_model_children_count_get(Eo *obj EINA_UNUSED,
                                              Eldbus_Model_Proxy_Data *pd)
{
   Eina_Bool ret = _eldbus_model_proxy_load(pd);
   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, obj);
   Efl_Future *future = efl_promise_future_get(promise);
   ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(ret, promise, EFL_MODEL_ERROR_INIT_FAILED, future);

   if (!pd->is_listed)
     {
        _eldbus_model_proxy_create_methods_children(pd);
        _eldbus_model_proxy_create_signals_children(pd);
        pd->is_listed = EINA_TRUE;
     }

   unsigned int *c = calloc(sizeof(unsigned int), 1);
   *c = eina_list_count(pd->children_list);
   efl_promise_value_set(promise, c, free);
   return future;
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

        INF("(%p) Creating method child: bus = %s, path = %s, method = %s::%s",
                       pd->obj, bus, path, interface_name, method_name);

        child = efl_add(ELDBUS_MODEL_METHOD_CLASS, pd->obj, eldbus_model_method_constructor(efl_added, pd->proxy, method));

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

        DBG("(%p) Creating signal child: bus = %s, path = %s, signal = %s::%s",
                       pd->obj, bus, path, interface_name, signal_name);

        child = efl_add(ELDBUS_MODEL_SIGNAL_CLASS, pd->obj, eldbus_model_signal_constructor(efl_added, pd->proxy, signal));

        pd->children_list = eina_list_append(pd->children_list, child);
     }
}

static const char *
_eldbus_model_proxy_name_get(Eo *obj EINA_UNUSED, Eldbus_Model_Proxy_Data *pd)
{
   return pd->name;
}

static Eina_Bool
_eldbus_model_proxy_load(Eldbus_Model_Proxy_Data *pd)
{
   Eldbus_Introspection_Property *property;
   Eina_List *it;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EINA_FALSE);

   if (pd->proxy)
     return EINA_TRUE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->object, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->name, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->interface, EINA_FALSE);

   pd->proxy = eldbus_proxy_get(pd->object, pd->name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->proxy, EINA_FALSE);

   const unsigned int properties_count = eina_list_count(pd->interface->properties);

   pd->properties_array = eina_array_new(properties_count);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->properties_array, EINA_FALSE);

   if (!properties_count) return EINA_TRUE;

   EINA_LIST_FOREACH(pd->interface->properties, it, property)
     {
        Eina_Stringshare *name;
        Eina_Bool ret;

        name = eina_stringshare_add(property->name);
        ret = eina_array_push(pd->properties_array, name);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, EINA_FALSE);
     }

   return EINA_TRUE;
}

static void
_eldbus_model_proxy_unload(Eldbus_Model_Proxy_Data *pd)
{
   Eldbus_Pending *pending;
   Eo *child;

   EINA_SAFETY_ON_NULL_RETURN(pd);

   EINA_LIST_FREE(pd->children_list, child)
     efl_unref(child);

   EINA_LIST_FREE(pd->pending_list, pending)
   {
     eldbus_pending_cancel(pending);
   }

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

   efl_model_property_invalidated_notify(pd->obj, event->name);
}

static Eina_Array *
_eldbus_model_proxy_property_get_all_load(const Eldbus_Message *msg, Eldbus_Model_Proxy_Data *pd)
{
   Eldbus_Introspection_Property *prop;
   Eldbus_Message_Iter *values = NULL;
   Eldbus_Message_Iter *entry;
   Eina_Array *changed_properties;
   Eina_List *it;
   const char *error_name, *error_text;

   if (eldbus_message_error_get(msg, &error_name, &error_text))
     {
        ERR("%s: %s", error_name, error_text);
        return NULL;
     }

   if (!eldbus_message_arguments_get(msg, "a{sv}", &values))
     {
        ERR("%s", "Error getting arguments.");
        return NULL;
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

   pd->is_loaded = EINA_TRUE;
   return changed_properties;

 on_error:
   eina_array_free(changed_properties);
   return NULL;
}

static void
_eldbus_model_proxy_property_get_all_cb(void *data,
                                        const Eldbus_Message *msg,
                                        Eldbus_Pending *pending)
{
   Eina_Value *promise_value;
   Eldbus_Model_Proxy_Data *pd = (Eldbus_Model_Proxy_Data*)data;

   pd->pending_list = eina_list_remove(pd->pending_list, pending);
   Eina_Array *changed_properties = _eldbus_model_proxy_property_get_all_load(msg, pd);
   if (changed_properties == NULL)
     {
         Eina_List* i;
         _Eldbus_Property_Promise* p;
         EINA_LIST_FOREACH(pd->promise_list, i, p)
           {
               efl_promise_failed_set(p->promise, EFL_MODEL_ERROR_NOT_FOUND);
               free(p->property);
           }
         eina_list_free(pd->promise_list);
         return;
     }

   Eina_List* i;
   _Eldbus_Property_Promise* p;
   EINA_LIST_FOREACH(pd->promise_list, i, p)
     {
        Eina_Value* value = eina_hash_find(pd->properties_hash, p->property);
        if (!value)
          {
             efl_promise_failed_set(p->promise, EFL_MODEL_ERROR_NOT_FOUND);
             free(p->property);
             continue;
          }

        if (!_eldbus_model_proxy_is_property_readable(pd, p->property))
          {
             efl_promise_failed_set(p->promise, EFL_MODEL_ERROR_READ_ONLY);
             free(p->property);
             continue;
          }

        free(p->property);

        promise_value = eina_value_new(eina_value_type_get(value));
        eina_value_copy(value, promise_value);
        efl_promise_value_set(p->promise, promise_value, (Eina_Free_Cb)&eina_value_free);
     }
   eina_list_free(pd->promise_list);


   _eldbus_model_proxy_start_monitor(pd);

   if (eina_array_count(changed_properties))
     {
        Efl_Model_Property_Event evt = {
          .changed_properties = changed_properties
        };

        efl_event_callback_call(pd->obj, EFL_MODEL_EVENT_PROPERTIES_CHANGED, &evt);
     }

   eina_array_free(changed_properties);
}


static void
_eldbus_model_proxy_property_set_load_cb(void *data,
                                        const Eldbus_Message *msg,
                                        Eldbus_Pending *pending)
{
   Eldbus_Model_Proxy_Property_Set_Data *set_data = (Eldbus_Model_Proxy_Property_Set_Data *)data;
   Eldbus_Model_Proxy_Data *pd = set_data->pd;
   const char *signature;

   pd->pending_list = eina_list_remove(pd->pending_list, pending);
   signature = _eldbus_model_proxy_property_type_get(pd, set_data->property);

   Eina_Array *changed_properties = _eldbus_model_proxy_property_get_all_load(msg, pd);
   if (signature == NULL || changed_properties == NULL)
     {
         efl_promise_failed_set(set_data->promise, EFL_MODEL_ERROR_UNKNOWN);

         eina_array_free(changed_properties);
         _eldbus_model_proxy_property_set_data_free(set_data);
        return;
     }

   eina_array_free(changed_properties);
   pending = eldbus_proxy_property_value_set
     (pd->proxy, set_data->property, signature, &set_data->value, _eldbus_model_proxy_property_set_cb, set_data);
   pd->pending_list = eina_list_append(pd->pending_list, pending);
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
   Eina_Value *promise_value;

   pd->pending_list = eina_list_remove(pd->pending_list, pending);

   if (eldbus_message_error_get(msg, &error_name, &error_text))
     {
         ERR("%s: %s", error_name, error_text);
         efl_promise_failed_set(property_set_data->promise, EFL_MODEL_ERROR_UNKNOWN);
         _eldbus_model_proxy_property_set_data_free(property_set_data);
         return;
     }

   prop_value = eina_hash_find(pd->properties_hash, property_set_data->property);
   if (prop_value != NULL)
     {
         if (eina_value_copy(&property_set_data->value, prop_value))
           {
               Efl_Model_Property_Event evt = {
                         .changed_properties = pd->properties_array
                       };

               efl_event_callback_call(pd->obj, EFL_MODEL_EVENT_PROPERTIES_CHANGED, &evt);
               efl_model_property_changed_notify(pd->obj, property_set_data->property);

           }
         promise_value = eina_value_new(eina_value_type_get(prop_value));
         eina_value_copy(prop_value, promise_value);
         efl_promise_value_set(property_set_data->promise, promise_value, (Eina_Free_Cb)&eina_value_free);
     }
   else
     {
         efl_promise_failed_set(property_set_data->promise, EFL_MODEL_ERROR_NOT_FOUND);
     }

   _eldbus_model_proxy_property_set_data_free(property_set_data);
}

static bool
_eldbus_model_proxy_has_property(Eldbus_Model_Proxy_Data *pd, const char *property)
{
   Eldbus_Introspection_Property *property_introspection =
     eldbus_introspection_property_find(pd->interface->properties, property);

   if (property_introspection == NULL)
     {
        return false;
     }

   return true;
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
                                          const Eina_Value *value,
                                          Efl_Promise *promise)
{
   Eldbus_Model_Proxy_Property_Set_Data *data = calloc(1, sizeof(Eldbus_Model_Proxy_Property_Set_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);

   data->pd = pd;
   data->promise = promise;
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

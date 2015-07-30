#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ejson_model_private.h"

#include <stdint.h>
#include <stdlib.h>

#define MY_CLASS EJSON_MODEL_CLASS
#define MY_CLASS_NAME "Ejson_Model"

typedef struct _Ejson_Model_Child_Data
{
   Eina_Stringshare *name;
   int               index;
} Ejson_Model_Child_Data;

static void _properties_load(Ejson_Model_Data *);
static void _children_load(Ejson_Model_Data *);
static bool _init(Ejson_Model_Data *);
static void _clear(Ejson_Model_Data *);
static void _properties_setup(Ejson_Model_Data *);
static Ejson_Model_Type _parent_type_get(Ejson_Model_Data *);
static void _children_hash_free(Ejson_Model_Child_Data *);
static void _properties_hash_free(Eina_Value *);
static bool _name_property_set(Ejson_Model_Data *, const Eina_Value *);
static bool _value_property_set(Ejson_Model_Data *, const Eina_Value *);
static bool _json_property_set(Ejson_Model_Data *, const Eina_Value *);
static void _invalidate(Ejson_Model_Data *);
static bool _stream_load(Ejson_Model_Data *);
static Eina_Bool _eina_value_as(const Eina_Value *, const Eina_Value_Type *, void *);
static Ejson_Model_Type _type_from_json_object(json_object *json);
static bool _object_child_add(Ejson_Model_Data *, Ejson_Model_Data *, const char *);
static bool _array_child_add(Ejson_Model_Data *, Ejson_Model_Data *, int);
static bool _child_add(Ejson_Model_Data *, Ejson_Model_Data *, Ejson_Model_Child_Data *);
static Ejson_Model_Child_Data *_object_child_data_new(const char *);
static Ejson_Model_Child_Data *_array_child_data_new(int*);
typedef Ejson_Model_Child_Data *(*Ejson_Model_Child_Data_New)(void *);
static void _child_new(Ejson_Model_Data *, json_object *, void *, Ejson_Model_Child_Data_New);

static Eo_Base *
_ejson_model_eo_base_constructor(Eo *obj, Ejson_Model_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->load.status = EFL_MODEL_LOAD_STATUS_UNLOADED;
   pd->json = NULL;
   pd->properties_array = NULL;
   pd->properties_hash = eina_hash_string_superfast_new(EINA_FREE_CB(_properties_hash_free));
   pd->children_list = NULL;
   pd->children_hash = eina_hash_pointer_new(EINA_FREE_CB(_children_hash_free));
   pd->stream = NULL;
   pd->type = EJSON_MODEL_TYPE_NULL;
   pd->properties_calc = eina_array_new(1);
   eina_array_push(pd->properties_calc, EJSON_MODEL_JSON_PROPERTY);
   pd->json_property_valid = false;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ejson_model_constructor(Eo *obj EINA_UNUSED,
                         Ejson_Model_Data *pd,
                         Ejson_Model_Type type)
{
   DBG("(%p)", obj);
   pd->type = type;
}

static void
_ejson_model_eo_base_destructor(Eo *obj, Ejson_Model_Data *pd)
{
   DBG("(%p)", obj);

   _clear(pd);

   eina_hash_free(pd->children_hash);
   eina_hash_free(pd->properties_hash);
   eina_array_free(pd->properties_calc);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Efl_Model_Load_Status
_ejson_model_efl_model_base_properties_get(Eo *obj EINA_UNUSED,
                                           Ejson_Model_Data *pd,
                                           Eina_Array * const* properties_array)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EFL_MODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->obj, EFL_MODEL_LOAD_STATUS_ERROR);

   *(Eina_Array**)properties_array = pd->properties_array;
   return pd->load.status;
}

static void
_ejson_model_efl_model_base_properties_load(Eo *obj, Ejson_Model_Data *pd)
{
   DBG("(%p)", obj);

   if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES)
     return;

   if (!_init(pd))
     return;

   efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADING_PROPERTIES);

   _properties_load(pd);

   efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);
}

static void
_properties_load(Ejson_Model_Data *pd)
{
   _properties_setup(pd);
   if (!pd->json)
     return;

   Eina_Array *changed_properties = eina_array_new(1);
   Eina_Value *value = NULL;
   switch (pd->type)
     {
      case EJSON_MODEL_TYPE_NULL:
        break;
      case EJSON_MODEL_TYPE_BOOLEAN:
        {
           value = eina_hash_find(pd->properties_hash, EJSON_MODEL_VALUE_PROPERTY);
           EINA_SAFETY_ON_NULL_GOTO(value, on_error);
           json_bool v = json_object_get_boolean(pd->json);
           Eina_Bool ret = eina_value_set(value, v);
           EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);
           break;
        }
      case EJSON_MODEL_TYPE_DOUBLE:
        {
           value = eina_hash_find(pd->properties_hash, EJSON_MODEL_VALUE_PROPERTY);
           EINA_SAFETY_ON_NULL_GOTO(value, on_error);
           double v = json_object_get_boolean(pd->json);
           Eina_Bool ret = eina_value_set(value, v);
           EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);
           break;
        }
      case EJSON_MODEL_TYPE_INT:
        {
           value = eina_hash_find(pd->properties_hash, EJSON_MODEL_VALUE_PROPERTY);
           EINA_SAFETY_ON_NULL_GOTO(value, on_error);
           int64_t v = json_object_get_int64(pd->json);
           Eina_Bool ret = eina_value_set(value, v);
           EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);
           break;
        }
      case EJSON_MODEL_TYPE_STRING:
        {
           value = eina_hash_find(pd->properties_hash, EJSON_MODEL_VALUE_PROPERTY);
           EINA_SAFETY_ON_NULL_GOTO(value, on_error);
           const char *v = json_object_get_string(pd->json);
           Eina_Bool ret = eina_value_set(value, v);
           EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);
           break;
        }
      case EJSON_MODEL_TYPE_OBJECT:
        break;
      case EJSON_MODEL_TYPE_ARRAY:
        break;
     }

   if (value)
     eina_array_push(changed_properties, EJSON_MODEL_VALUE_PROPERTY);

   if (EJSON_MODEL_TYPE_OBJECT == _parent_type_get(pd))
     {
        Eo *parent = eo_do_ret(pd->obj, parent, eo_parent_get());

        Ejson_Model_Data *parent_pd = eo_data_scope_get(parent, EJSON_MODEL_CLASS);
        EINA_SAFETY_ON_NULL_GOTO(parent_pd, on_error);

        Ejson_Model_Child_Data *child_data = eina_hash_find(parent_pd->children_hash,
                                                            &pd->obj);
        EINA_SAFETY_ON_NULL_GOTO(child_data, on_error);

        value = eina_hash_find(pd->properties_hash, EJSON_MODEL_NAME_PROPERTY);
        EINA_SAFETY_ON_NULL_GOTO(value, on_error);

        Eina_Bool ret = eina_value_set(value, child_data->name);
        EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);

        ret = eina_array_push(changed_properties, EJSON_MODEL_NAME_PROPERTY);
        EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);
     }

   Efl_Model_Property_Event evt = {
     .changed_properties = changed_properties,
     .invalidated_properties = pd->properties_calc
   };
   eo_do(pd->obj,
         eo_event_callback_call(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, &evt));

on_error:
   eina_array_free(changed_properties);
}

static Efl_Model_Load_Status
_ejson_model_efl_model_base_property_set(Eo *obj,
                                         Ejson_Model_Data *pd,
                                         const char *property,
                                         const Eina_Value *value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EFL_MODEL_LOAD_STATUS_ERROR);
   DBG("(%p): property=%s", obj, property);

   Eina_Array *changed_properties = eina_array_new(1);
   const bool is_json_property = strcmp(EJSON_MODEL_JSON_PROPERTY, property) == 0;
   if (is_json_property)
     {
        if (!_json_property_set(pd, value))
          goto on_error;

        // only notify properties_changed if loaded
        if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED)
          eina_array_push(changed_properties, EJSON_MODEL_JSON_PROPERTY);
     }
   else
     {
        if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
          goto on_error;

        const bool is_name_property = strcmp(EJSON_MODEL_NAME_PROPERTY, property) == 0;
        const bool is_value_property = strcmp(EJSON_MODEL_VALUE_PROPERTY, property) == 0;

        if (!is_name_property && !is_value_property)
          {
             ERR("Unrecognized property: %s", property);
             goto on_error;
          }

        if (is_name_property)
          {
             if (!_name_property_set(pd, value))
               goto on_error;
             eina_array_push(changed_properties, EJSON_MODEL_NAME_PROPERTY);
          }
        else if (is_value_property)
          {
             if (!_value_property_set(pd, value))
               goto on_error;
             eina_array_push(changed_properties, EJSON_MODEL_VALUE_PROPERTY);
          }
     }

   if (eina_array_count(changed_properties))
     {
        pd->json_property_valid = false;
        Efl_Model_Property_Event evt = {
          .changed_properties = changed_properties,
          .invalidated_properties = pd->properties_calc
        };
        eo_do(pd->obj, eo_event_callback_call(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, &evt));
     }

   eina_array_free(changed_properties);
   return pd->load.status;

on_error:
   eina_array_free(changed_properties);
   return EFL_MODEL_LOAD_STATUS_ERROR;
}

static bool
_name_property_set(Ejson_Model_Data *pd, const Eina_Value *value)
{
   DBG("(%p)", pd->obj);
   Ejson_Model_Type parent_type = _parent_type_get(pd);
   if (EJSON_MODEL_TYPE_OBJECT != parent_type)
     {
        ERR("Only object types have the '" EJSON_MODEL_NAME_PROPERTY "' property");
        return false;
     }

   Eina_Value *prop_value = eina_hash_find(pd->properties_hash, EJSON_MODEL_NAME_PROPERTY);
   EINA_SAFETY_ON_NULL_RETURN_VAL(prop_value, false);

   eina_value_flush(prop_value);
   Eina_Bool ret = eina_value_copy(value, prop_value);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, false);

   return true;
}

static bool
_value_property_set(Ejson_Model_Data *pd, const Eina_Value *value)
{
   DBG("(%p)", pd->obj);
   const Eina_Value_Type * const value_type = eina_value_type_get(value);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value_type, false);

   json_object *json = NULL;
   switch (pd->type)
     {
      case EJSON_MODEL_TYPE_BOOLEAN:
        {
           int v;
           Eina_Bool ret = _eina_value_as(value, EINA_VALUE_TYPE_INT, &v);
           EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, false);

           json = json_object_new_boolean(v ? TRUE : FALSE);
           break;
        }
      case EJSON_MODEL_TYPE_DOUBLE:
        {
           double v;
           Eina_Bool ret = _eina_value_as(value, EINA_VALUE_TYPE_DOUBLE, &v);
           EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, false);

           json = json_object_new_double(v);
           break;
        }
      case EJSON_MODEL_TYPE_INT:
        {
           int64_t v;
           Eina_Bool ret = _eina_value_as(value, EINA_VALUE_TYPE_INT64, &v);
           EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, false);

           json = json_object_new_int64(v);
           break;
        }
      case EJSON_MODEL_TYPE_STRING:
        {
           const char *v;
           Eina_Bool ret = _eina_value_as(value, EINA_VALUE_TYPE_STRING, &v);
           EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, false);

           json = json_object_new_string(v);
           break;
        }
      case EJSON_MODEL_TYPE_NULL:
        return false;
      case EJSON_MODEL_TYPE_OBJECT:
        return false;
      case EJSON_MODEL_TYPE_ARRAY:
        return false;
     }

   Eina_Value *prop_value = eina_hash_find(pd->properties_hash, EJSON_MODEL_VALUE_PROPERTY);
   EINA_SAFETY_ON_NULL_GOTO(prop_value, on_error);

   eina_value_flush(prop_value);
   Eina_Bool ret = eina_value_copy(value, prop_value);
   EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);

   json_object_put(pd->json);
   pd->json = json;

   return true;

on_error:
   json_object_put(json);
   return false;
}

static bool
_json_property_set(Ejson_Model_Data *pd, const Eina_Value *value)
{
   DBG("(%p)", pd->obj);
   char *stream = eina_value_to_string(value);
   EINA_SAFETY_ON_NULL_RETURN_VAL(stream, false);

   Efl_Model_Load_Status old_status = pd->load.status;
   switch (old_status & EFL_MODEL_LOAD_STATUS_LOADED)
     {
      case EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES:
        efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADING_PROPERTIES);
        break;
      case EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN:
        efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN);
        break;
      case EFL_MODEL_LOAD_STATUS_LOADED:
        efl_model_load_set(pd->obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADING);
        break;
      default: break;
     }

   _clear(pd);

   pd->stream = stream;

   _stream_load(pd);

   if (old_status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES)
     _properties_load(pd);

   if (old_status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN)
     _children_load(pd);

   efl_model_load_set(pd->obj, &pd->load, old_status);
   return true;
}

static Efl_Model_Load_Status
_ejson_model_efl_model_base_property_get(Eo *obj EINA_UNUSED,
                                         Ejson_Model_Data *pd,
                                         const char *property,
                                         const Eina_Value **value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(property, EFL_MODEL_LOAD_STATUS_ERROR);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EFL_MODEL_LOAD_STATUS_ERROR);
   DBG("(%p): property=%s", obj, property);

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
     return EFL_MODEL_LOAD_STATUS_ERROR;

   *value = eina_hash_find(pd->properties_hash, property);
   if (!(*value))
     return EFL_MODEL_LOAD_STATUS_ERROR;

   if ((strcmp(EJSON_MODEL_JSON_PROPERTY, property) == 0) && !pd->json_property_valid)
     {
        const char *json = json_object_to_json_string(pd->json);
        Eina_Bool ret = eina_value_set((Eina_Value*)*value, json);
        pd->json_property_valid = EINA_TRUE == ret;

        if (!pd->json_property_valid)
          return EFL_MODEL_LOAD_STATUS_ERROR;
     }

   return pd->load.status;
}

static void
_ejson_model_efl_model_base_load(Eo *obj, Ejson_Model_Data *pd)
{
   DBG("(%p)", obj);

   if ((pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED) == EFL_MODEL_LOAD_STATUS_LOADED)
     return;

   if (!_init(pd))
     return;

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
     {
        efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADING_PROPERTIES);
        _properties_load(pd);
     }

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN))
     {
        efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN);
        _children_load(pd);
     }

   efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED);
}

static Efl_Model_Load_Status
_ejson_model_efl_model_base_load_status_get(Eo *obj EINA_UNUSED,
                                            Ejson_Model_Data *pd)
{
   DBG("(%p)", obj);
   return pd->load.status;
}

static void
_ejson_model_efl_model_base_unload(Eo *obj, Ejson_Model_Data *pd)
{
   DBG("(%p)", obj);

   _clear(pd);

   efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_UNLOADED);
}

static Eina_Bool
_ejson_model_child_properties_changed_cb(void *data,
                                         Eo *child,
                                         const Eo_Event_Description *desc EINA_UNUSED,
                                         void *event_info)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, EO_CALLBACK_STOP);
   EINA_SAFETY_ON_NULL_RETURN_VAL(event_info, EO_CALLBACK_STOP);
   Ejson_Model_Data *pd = (Ejson_Model_Data*)data;
   DBG("(%p)", pd->obj);

   // Only user changes count
   Efl_Model_Load_Status child_status;
   eo_do(child, child_status = efl_model_load_status_get());
   if (!(child_status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
     return EO_CALLBACK_CONTINUE;

   Ejson_Model_Data *child_pd = eo_data_scope_get(child, EJSON_MODEL_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_pd, EO_CALLBACK_STOP);

   const char *current_name = NULL;
   if (EJSON_MODEL_TYPE_OBJECT == pd->type)
     {
        Eina_Value *value = eina_hash_find(child_pd->properties_hash,
                                           EJSON_MODEL_NAME_PROPERTY);
        EINA_SAFETY_ON_NULL_RETURN_VAL(value, EO_CALLBACK_STOP);

        Eina_Bool ret = eina_value_get(value, &current_name);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, EO_CALLBACK_STOP);

        // Continue only if the name is already assigned
        if (!current_name || !strlen(current_name))
          return EO_CALLBACK_CONTINUE;
     }

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->children_hash, EO_CALLBACK_STOP);
   Ejson_Model_Child_Data *child_data = eina_hash_find(pd->children_hash, &child);
   if (child_data)
     {
        DBG("(%p) Setting child %p value", pd->obj, child_pd->obj);
        switch (pd->type)
          {
           case EJSON_MODEL_TYPE_OBJECT:
             {
                const bool is_name_changed = strcmp(current_name, child_data->name) == 0;
                if (!is_name_changed)
                  {
                     json_object_object_del(pd->json, child_data->name);
                     eina_stringshare_del(child_data->name);
                     child_data->name = eina_stringshare_add(current_name);
                  }

                json_object_object_add(pd->json,
                                          child_data->name,
                                          json_object_get(child_pd->json));
                break;
             }
           case EJSON_MODEL_TYPE_ARRAY:
             json_object_array_put_idx(pd->json,
                                       child_data->index,
                                       json_object_get(child_pd->json));
             break;
           default:
             ERR("Type cannot have children: %d", pd->type);
             return EO_CALLBACK_STOP;
          }
     }
   else
     {

        bool ok = false;
        switch (pd->type)
          {
           case EJSON_MODEL_TYPE_OBJECT:
             json_object_object_add(pd->json,
                                    current_name,
                                    json_object_get(child_pd->json));
             ok = _object_child_add(pd, child_pd, current_name);
             break;
           case EJSON_MODEL_TYPE_ARRAY:
             {
                int index = json_object_array_add(pd->json,
                                                  json_object_get(child_pd->json));
                ok = _array_child_add(pd, child_pd, index);
                break;
             }
           default:
             ERR("Current type cannot have children: %d", pd->type);
          }

        EINA_SAFETY_ON_FALSE_RETURN_VAL(ok, EO_CALLBACK_STOP);

        unsigned int count = eina_list_count(pd->children_list);
        Efl_Model_Children_Event evt = {.child = child, .index = count};
        eo_do(pd->obj,
              eo_event_callback_call(EFL_MODEL_BASE_EVENT_CHILD_ADDED, &evt),
              eo_event_callback_call(EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED, &count));
     }

   _invalidate(pd);

   Efl_Model_Property_Event evt = {
     .invalidated_properties = pd->properties_calc
   };
   eo_do(pd->obj,
         eo_event_callback_call(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, &evt));

   return EO_CALLBACK_CONTINUE;
}

Eo *
_ejson_model_efl_model_base_child_add(Eo *obj, Ejson_Model_Data *pd)
{
   DBG("(%p)", obj);

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN))
     return NULL;

   json_type type = json_object_get_type(pd->json);
   switch (type)
     {
      case json_type_array:
      case json_type_object:
        break;
      default:
        ERR("Value types cannot have children: %d", type);
        return NULL;
     }

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->json, NULL);

   Eo *child = eo_add(EJSON_MODEL_CLASS,
                      obj,
                      ejson_model_constructor(EJSON_MODEL_TYPE_NULL));
   eo_do(child, eo_event_callback_add(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED,
                                      _ejson_model_child_properties_changed_cb,
                                      pd));
   return child;
}

static Efl_Model_Load_Status
_ejson_model_efl_model_base_child_del(Eo *obj,
                                      Ejson_Model_Data *pd,
                                      Eo *child)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child, EFL_MODEL_LOAD_STATUS_ERROR);

   if (!(pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN))
     return EFL_MODEL_LOAD_STATUS_ERROR;

   Ejson_Model_Child_Data *child_data = eina_hash_find(pd->children_hash, &child);
   if (!child_data)
     {
        ERR("Child not found: %p", child);
        return EFL_MODEL_LOAD_STATUS_ERROR;
     }

   switch (pd->type)
     {
      case EJSON_MODEL_TYPE_OBJECT:
        {
           json_object_object_del(pd->json, child_data->name);
           break;
        }
      case EJSON_MODEL_TYPE_ARRAY:
        {
           int index = child_data->index;
           json_object *json = json_object_new_array();

           int length = json_object_array_length(pd->json);

           for (int i = 0; i < index; ++i)
             {
                json_object *item = json_object_array_get_idx(pd->json, i);
                json_object_array_add(json, json_object_get(item));
             }

           for (int i = index + 1; i < length; ++i)
             {
                json_object *item = json_object_array_get_idx(pd->json, i);
                json_object_array_add(json, json_object_get(item));
             }

           Ejson_Model_Child_Data *data;
           Eina_Iterator *it = eina_hash_iterator_data_new(pd->children_hash);
           EINA_ITERATOR_FOREACH(it, data)
             {
                if (data->index > index)
                  --data->index;
             }
           eina_iterator_free(it);

           json_object_put(pd->json);
           pd->json = json;
           break;
        }
      default: return EFL_MODEL_LOAD_STATUS_ERROR;
     }

   pd->children_list = eina_list_remove(pd->children_list, child);
   Eina_Bool ret = eina_hash_del(pd->children_hash, &child, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, EFL_MODEL_LOAD_STATUS_ERROR);
   eo_del(child);

   pd->json_property_valid = false;
   unsigned int count = eina_list_count(pd->children_list);
   Efl_Model_Property_Event prop_event = {
     .changed_properties = pd->properties_array,
     .invalidated_properties = pd->properties_calc
   };
   Efl_Model_Children_Event added_event = {.child = child, .index = count};
   eo_do(obj,
         eo_event_callback_call(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, &prop_event),
         eo_event_callback_call(EFL_MODEL_BASE_EVENT_CHILD_ADDED, &added_event),
         eo_event_callback_call(EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED, &count));

   return pd->load.status;
}

static Efl_Model_Load_Status
_ejson_model_efl_model_base_children_slice_get(Eo *obj EINA_UNUSED,
                                               Ejson_Model_Data *pd,
                                               unsigned start,
                                               unsigned count,
                                               Eina_Accessor **children_accessor)
{
   DBG("(%p)", obj);

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
_ejson_model_efl_model_base_children_count_get(Eo *obj EINA_UNUSED,
                                               Ejson_Model_Data *pd,
                                               unsigned *children_count)
{
   DBG("(%p)", obj);
   *children_count = eina_list_count(pd->children_list);
   return pd->load.status;
}

static void
_ejson_model_efl_model_base_children_load(Eo *obj, Ejson_Model_Data *pd)
{
   DBG("(%p)", obj);

   if (pd->load.status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN)
     return;

   if (!_init(pd))
     return;

   efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADING_CHILDREN);

   _children_load(pd);

   efl_model_load_set(obj, &pd->load, EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN);
}

static void
_children_load(Ejson_Model_Data *pd)
{
   DBG("(%p)", pd->obj);
   if (!pd->json)
     return;

   switch (pd->type)
     {
      case EJSON_MODEL_TYPE_OBJECT:
        {
           Ejson_Model_Child_Data_New cb = (Ejson_Model_Child_Data_New)_object_child_data_new;
           json_object_object_foreach(pd->json, name, json)
             _child_new(pd, json, name, cb);
           break;
        }
      case EJSON_MODEL_TYPE_ARRAY:
        {
           Ejson_Model_Child_Data_New cb = (Ejson_Model_Child_Data_New)_array_child_data_new;
           int length = json_object_array_length(pd->json);
           for (int i = 0; i < length; ++i)
             {
                json_object *json = json_object_array_get_idx(pd->json, i);
                _child_new(pd, json, &i, cb);
             }
           break;
        }
      default:
        ERR("Current type cannot have children: %d", pd->type);
     }

   unsigned int count = eina_list_count(pd->children_list);
   if (count)
     eo_do(pd->obj,
           eo_event_callback_call(EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED, &count));
}

static void
_child_new(Ejson_Model_Data *pd,
           json_object *json,
           void *data,
           Ejson_Model_Child_Data_New child_data_new)
{
   Ejson_Model_Type type = _type_from_json_object(json);

   Eo *child = eo_add(EJSON_MODEL_CLASS,
                      pd->obj,
                      ejson_model_constructor(type));

   Ejson_Model_Data *child_pd = eo_data_scope_get(child, EJSON_MODEL_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(child_pd);

   child_pd->json = json_object_get(json);

   Ejson_Model_Child_Data *child_data = child_data_new(data);
   _child_add(pd, child_pd, child_data);

   eo_do(child, eo_event_callback_add(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED,
                                      _ejson_model_child_properties_changed_cb,
                                      pd));
}

static Ejson_Model_Type
_ejson_model_type_get(Eo *obj EINA_UNUSED, Ejson_Model_Data *pd)
{
   DBG("(%p)", obj);
   return pd->type;
}

static void
_ejson_model_type_set(Eo *obj EINA_UNUSED,
                      Ejson_Model_Data *pd,
                      const Ejson_Model_Type value)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_FALSE_RETURN(value >= 0 && value <= EJSON_MODEL_TYPE_ARRAY);
   pd->type = value;
}

static bool
_init(Ejson_Model_Data *pd)
{
   if (pd->json)
     return true;

   if (pd->stream)
     {
        if (!_stream_load(pd))
          return false;
     }
   else
     {
        switch (pd->type)
          {
           case EJSON_MODEL_TYPE_OBJECT:
             pd->json = json_object_new_object();
             break;
           case EJSON_MODEL_TYPE_ARRAY:
             pd->json = json_object_new_array();
             break;
           default: break;
          }
     }

   return true;
}

static void
_clear(Ejson_Model_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);

   eina_hash_free_buckets(pd->children_hash);

   Eo *child;
   EINA_LIST_FREE(pd->children_list, child)
     eo_del(child);

   eina_hash_free_buckets(pd->properties_hash);

   if (pd->properties_array)
     {
        eina_array_free(pd->properties_array);
        pd->properties_array = NULL;
     }

   json_object_put(pd->json);
   pd->json = NULL;
   pd->json_property_valid = false;

   free(pd->stream);
   pd->stream = NULL;
}

typedef struct
{
   Ejson_Model_Data      *pd;
   const char            *member_name[3];
   const Eina_Value_Type *member_type[3];
} Ejson_Model_Properties_Setup;

static void
_properties_setup(Ejson_Model_Data *pd)
{
   DBG("(%p) Setup type: %d", pd->obj, pd->type);

   Ejson_Model_Properties_Setup setup = {.pd = pd};

   setup.member_name[0] = EJSON_MODEL_JSON_PROPERTY;
   setup.member_type[0] = EINA_VALUE_TYPE_STRING;
   unsigned int member_count = 1;

   switch (pd->type)
     {
      case EJSON_MODEL_TYPE_BOOLEAN:
        setup.member_type[member_count] = EINA_VALUE_TYPE_INT;
        break;
      case EJSON_MODEL_TYPE_INT:
        setup.member_type[member_count] = EINA_VALUE_TYPE_INT64;
        break;
      case EJSON_MODEL_TYPE_DOUBLE:
        setup.member_type[member_count] = EINA_VALUE_TYPE_DOUBLE;
        break;
      case EJSON_MODEL_TYPE_STRING:
        setup.member_type[member_count] = EINA_VALUE_TYPE_STRING;
        break;
      case EJSON_MODEL_TYPE_NULL:
      case EJSON_MODEL_TYPE_OBJECT:
      case EJSON_MODEL_TYPE_ARRAY:
        break;
     }

   switch (pd->type)
     {
      case EJSON_MODEL_TYPE_BOOLEAN:
      case EJSON_MODEL_TYPE_INT:
      case EJSON_MODEL_TYPE_DOUBLE:
      case EJSON_MODEL_TYPE_STRING:
        setup.member_name[member_count] = EJSON_MODEL_VALUE_PROPERTY;
        ++member_count;
        break;
      case EJSON_MODEL_TYPE_NULL:
      case EJSON_MODEL_TYPE_OBJECT:
      case EJSON_MODEL_TYPE_ARRAY:
        break;
     }

   Ejson_Model_Type parent_type = _parent_type_get(pd);

   if (EJSON_MODEL_TYPE_OBJECT == parent_type)
     {
        setup.member_name[member_count] = EJSON_MODEL_NAME_PROPERTY;
        setup.member_type[member_count] = EINA_VALUE_TYPE_STRING;
        ++member_count;
     }

   pd->properties_array = eina_array_new(member_count);
   for (size_t i = 0; i < member_count; ++i)
      {
         Eina_Bool ret = eina_array_push(pd->properties_array,
                                         setup.member_name[i]);
         EINA_SAFETY_ON_FALSE_RETURN(ret);

         const Eina_Value_Type *type = setup.member_type[i];
         Eina_Value *value = eina_value_new(type);
         EINA_SAFETY_ON_NULL_RETURN(value);

         ret = eina_hash_add(pd->properties_hash, setup.member_name[i], value);
         EINA_SAFETY_ON_FALSE_RETURN(ret);
      }

   DBG("(%p) Properties setup done", pd->obj);
}

static Ejson_Model_Type
_parent_type_get(Ejson_Model_Data *pd)
{
   Eo *parent = eo_do_ret(pd->obj, parent, eo_parent_get());
   if (!parent)
     return EJSON_MODEL_TYPE_NULL;

   Ejson_Model_Data *parent_pd = eo_data_scope_get(parent, EJSON_MODEL_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent_pd, EJSON_MODEL_TYPE_NULL);

   return parent_pd->type;
}

static void
_children_hash_free(Ejson_Model_Child_Data *child)
{
   eina_stringshare_del(child->name);
   free(child);
}

static void
_properties_hash_free(Eina_Value *value)
{
   eina_value_free(value);
}

static void
_invalidate(Ejson_Model_Data *pd)
{
   Eo *parent = pd->obj;
   while ((parent = eo_do_ret(parent, parent, eo_parent_get())))
     {
        Ejson_Model_Data *parent_pd = eo_data_scope_get(parent, EJSON_MODEL_CLASS);
        parent_pd->json_property_valid = false;

        Efl_Model_Property_Event evt = {
          .invalidated_properties = parent_pd->properties_calc
        };
        eo_do(parent,
              eo_event_callback_call(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, &evt));
     }

   pd->json_property_valid = false;
}

static bool
_stream_load(Ejson_Model_Data *pd)
{
    enum json_tokener_error error;
    pd->json = json_tokener_parse_verbose(pd->stream, &error);
    if (json_tokener_success != error)
      {
         ERR("Invalid json stream. Error: %d, stream = %s", error, pd->stream);
         return false;
      }

    pd->type = _type_from_json_object(pd->json);
    return true;
}

static Ejson_Model_Type
_type_from_json_object(json_object *json)
{
   json_type type = json_object_get_type(json);
   switch (type)
     {
      case json_type_null: return EJSON_MODEL_TYPE_NULL;
      case json_type_boolean: return EJSON_MODEL_TYPE_BOOLEAN;
      case json_type_double: return EJSON_MODEL_TYPE_DOUBLE;
      case json_type_int: return EJSON_MODEL_TYPE_INT;
      case json_type_object: return EJSON_MODEL_TYPE_OBJECT;
      case json_type_array: return EJSON_MODEL_TYPE_ARRAY;
      case json_type_string: return EJSON_MODEL_TYPE_STRING;
      default:
        ERR("Unrecognized type: %d", type);
        return EJSON_MODEL_TYPE_NULL;
     }
}

static Eina_Bool
_eina_value_as(const Eina_Value *value, const Eina_Value_Type *type, void *mem)
{
   const Eina_Value_Type * const value_type = eina_value_type_get(value);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value_type, false);

   if (value_type == type)
     return eina_value_get(value, mem);

   Eina_Value convert;
   Eina_Bool ret = eina_value_setup(&convert, type)
      && eina_value_convert(value, &convert)
      && eina_value_get(&convert, mem);
   eina_value_flush(&convert);

   return ret;
}

static Ejson_Model_Child_Data *
_object_child_data_new(const char *name)
{
   Ejson_Model_Child_Data *child_data = calloc(1, sizeof(Ejson_Model_Child_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_data, NULL);
   child_data->name = eina_stringshare_add(name);

   return child_data;
}

static Ejson_Model_Child_Data *
_array_child_data_new(int *index)
{
   Ejson_Model_Child_Data *child_data = calloc(1, sizeof(Ejson_Model_Child_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_data, NULL);
   child_data->index = *index;

   return child_data;
}

static bool
_object_child_add(Ejson_Model_Data *pd,
                  Ejson_Model_Data *child_pd,
                  const char *name)
{
   Ejson_Model_Child_Data *child_data = _object_child_data_new(name);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_data, false);

   return _child_add(pd, child_pd, child_data);
}

static bool
_array_child_add(Ejson_Model_Data *pd, Ejson_Model_Data *child_pd, int index)
{
   Ejson_Model_Child_Data *child_data = _array_child_data_new(&index);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_data, false);

   return _child_add(pd, child_pd, child_data);
}

static bool
_child_add(Ejson_Model_Data *pd,
           Ejson_Model_Data *child_pd,
           Ejson_Model_Child_Data *child_data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, false);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_pd, false);
   EINA_SAFETY_ON_NULL_RETURN_VAL(child_data, false);

   DBG("(%p) Adding new child %p of type: %d", pd->obj, child_pd->obj, child_pd->type);
   pd->children_list = eina_list_append(pd->children_list, child_pd->obj);

   Eina_Bool ret = eina_hash_add(pd->children_hash, &child_pd->obj, child_data);
   EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);
   return true;

on_error:
   free(child_data);
   return false;
}

#include "ejson_model.eo.c"

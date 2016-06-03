#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ejson.h"
#include "ejson_model_private.h"

#include <stdint.h>
#include <stdlib.h>

#define MY_CLASS EJSON_MODEL_CLASS
#define MY_CLASS_NAME "Ejson_Model"

typedef struct _Ejson_Model_Child_Data
{
   Ejson_Model_Data *pd;
   Eo *parent;
} Ejson_Model_Child_Data;


static void _ejson_model_clear(Ejson_Model_Data *);
static bool _ejson_model_name_property_set(Ejson_Model_Data *, const Eina_Value *, Eina_Promise_Owner *promise);
static bool _ejson_model_value_property_set(Ejson_Model_Data *, const Eina_Value *, Eina_Promise_Owner *promise);
static bool _ejson_model_stream_load(Ejson_Model_Data *, const char*, unsigned);
static Eina_Bool _ejson_model_eina_value_as(const Eina_Value *, const Eina_Value_Type *, void *);
typedef Ejson_Model_Child_Data *(*Ejson_Model_Child_Data_New)(void *);

static Eina_Bool _parse_simple_obj_cb(void *data, Eina_Simple_JSON_Value_Type type, const char *key, const char *content, unsigned length);

static void _ejson_model_type_set(Eo *obj, Ejson_Model_Data *pd, Ejson_Model_Type type);

void
_accessor_free(void *data)
{
   Eina_Accessor *ac = data;
   if (!ac)
     return;
   Eina_List *list = eina_accessor_container_get(ac);
   eina_accessor_free(ac);
   eina_list_free(list);
}

static Eo_Base*
_ejson_model_eo_base_constructor(Eo *obj, Ejson_Model_Data *pd)
{
   obj = eo_constructor(eo_super(obj, MY_CLASS));

   pd->obj = obj;
   pd->properties_array = eina_array_new(1);
   pd->value = NULL;
   pd->name = NULL;
   pd->children_list = NULL;
   pd->json_property_valid = false;

   _ejson_model_type_set(obj, pd, EJSON_MODEL_TYPE_NULL);

   return obj;
}

static void
_ejson_model_string_set(Eo *obj EINA_UNUSED,
                         Ejson_Model_Data *pd,
                         const char* stream)
{
   _ejson_model_stream_load(pd, stream, strlen(stream));
}

static void
_ejson_model_eo_base_destructor(Eo *obj, Ejson_Model_Data *pd)
{
   _ejson_model_clear(pd);

   eo_destructor(eo_super(obj, MY_CLASS));
}

static void
_fill_default_value(Ejson_Model_Data *pd)
{
   pd->value = eina_value_optional_empty_new();
}

static Eina_Array const *
_ejson_model_efl_model_properties_get(Eo *obj EINA_UNUSED,
                                   Ejson_Model_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->obj, NULL);

   return pd->properties_array;
}

static void
_ejson_model_efl_model_property_set(Eo *obj EINA_UNUSED,
                                 Ejson_Model_Data *pd,
                                 const char *property,
                                 const Eina_Value *value,
                                 Eina_Promise_Owner *promise)
{
   Efl_Model_Property_Event evt;
   if (strcmp(EJSON_MODEL_NAME_PROPERTY, property) == 0)
     {
        if (!_ejson_model_name_property_set(pd, value, promise))
          return;

        evt.changed_properties = eina_array_new(1);
        eina_array_push(evt.changed_properties, EJSON_MODEL_NAME_PROPERTY);
     }
   else if (strcmp(EJSON_MODEL_VALUE_PROPERTY, property) == 0)
     {
        if (!_ejson_model_value_property_set(pd, value, promise))
          return;

        evt.changed_properties = eina_array_new(1);
        eina_array_push(evt.changed_properties, EJSON_MODEL_VALUE_PROPERTY);
     }
   else
     {
        eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
        return;
     }

   if (eina_array_count(evt.changed_properties))
     {
        pd->json_property_valid = false;
        eo_event_callback_call(pd->obj, EFL_MODEL_EVENT_PROPERTIES_CHANGED, &evt);
     }

   eina_array_free(evt.changed_properties);
}

static bool
_ejson_model_name_property_set(Ejson_Model_Data *pd, const Eina_Value *value,
                                                         Eina_Promise_Owner *promise)
{
   const char *name = NULL;

   if (!value || !(name = eina_value_to_string(value)))
     {
         eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_INCORRECT_VALUE);
         return false;
     }

   if (!pd->name)
     eina_array_push(pd->properties_array, "name");

   free((char*)pd->name);
   pd->name = name;

   eina_promise_owner_value_set(promise, value, NULL);

   return true;
}

static bool
_ejson_model_value_property_set(Ejson_Model_Data *pd, const Eina_Value *value,
                                                         Eina_Promise_Owner *promise)
{
   const Eina_Value_Type * const value_type = eina_value_type_get(value);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value_type, false);


   /* json_object *json = NULL; */
   switch (pd->type)
     {
      case EJSON_MODEL_TYPE_BOOLEAN:
        {
           int v;
           Eina_Bool ret = _ejson_model_eina_value_as(value, EINA_VALUE_TYPE_INT, &v);
           EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, false);

           break;
        }
      case EJSON_MODEL_TYPE_DOUBLE:
        {
           double v;
           Eina_Bool ret = _ejson_model_eina_value_as(value, EINA_VALUE_TYPE_DOUBLE, &v);
           EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, false);

           break;
        }
      case EJSON_MODEL_TYPE_INT:
        {
           long int v;
           eina_value_get(value, &v);
           Eina_Bool ret = _ejson_model_eina_value_as(value, EINA_VALUE_TYPE_INT64, &v);
           EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, false);
           eina_value_get(value, &v);

           break;
        }
      case EJSON_MODEL_TYPE_STRING:
        {
           const char *v;
           Eina_Bool ret = _ejson_model_eina_value_as(value, EINA_VALUE_TYPE_STRING, &v);
           EINA_SAFETY_ON_FALSE_RETURN_VAL(ret, false);

           break;
        }
      case EJSON_MODEL_TYPE_NULL:
      case EJSON_MODEL_TYPE_OBJECT:
      case EJSON_MODEL_TYPE_ARRAY:
        eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_INCORRECT_VALUE);
        return false;
     }

   // Copy takes care of flushing the previous values in pd->value.
   if (!pd->value)
     {
        eina_array_push(pd->properties_array, "value");
        _fill_default_value(pd);
     }
   Eina_Bool ret = eina_value_copy(value, pd->value);
   EINA_SAFETY_ON_FALSE_GOTO(ret, on_error);

   eina_promise_owner_value_set(promise, value, NULL);

   return true;

on_error:
   eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_UNKNOWN);
   return false;
}

static void
_ejson_model_efl_model_property_get(Eo *obj EINA_UNUSED,
                                 Ejson_Model_Data *pd,
                                 const char *property,
                                 Eina_Promise_Owner *promise)
{
   if (property == NULL && pd->value == NULL)
     {
         eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_INCORRECT_VALUE);
         return;
     }

   if (strcmp(EJSON_MODEL_NAME_PROPERTY, property) == 0)
     {
        Eina_Value value;
        eina_value_setup(&value, EINA_VALUE_TYPE_STRING);
        eina_value_set(&value, pd->name);

        eina_promise_owner_value_set(promise, &value, (Eina_Promise_Free_Cb)eina_value_flush);
     }
   else if(strcmp(EJSON_MODEL_VALUE_PROPERTY, property) == 0)
     {
         if (!pd->value)
           {
              eina_array_push(pd->properties_array, "value");
              _fill_default_value(pd);
           }
         eina_promise_owner_value_set(promise, pd->value, NULL);
     }
   else
     eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
}

Eo *
_ejson_model_efl_model_child_add(Eo *obj, Ejson_Model_Data *pd)
{
   switch (pd->type)
     {
      case EJSON_MODEL_TYPE_ARRAY:
      case EJSON_MODEL_TYPE_OBJECT:
        break;
      default:
        WRN("Value types cannot have children: %d", pd->type);
        return NULL;
     }

   Eo *child = eo_add(EJSON_MODEL_CLASS, obj,
                      ejson_model_type_set(eo_self, EJSON_MODEL_TYPE_NULL));

   pd->children_list = eina_list_append(pd->children_list, child);

   return child;
}

static void
_ejson_model_efl_model_child_del(Eo *obj,
                              Ejson_Model_Data *pd,
                              Eo *child)
{
   EINA_SAFETY_ON_NULL_RETURN(child);

   int idx = eina_list_data_idx(pd->children_list, child);

   if(idx >= 0)
     {
         pd->children_list = eina_list_remove(pd->children_list, child);
         unsigned int count = eina_list_count(pd->children_list);

         Efl_Model_Children_Event cevt = {.child = child, .index = idx};
         eo_event_callback_call(obj, EFL_MODEL_EVENT_CHILD_REMOVED, &cevt);
         eo_event_callback_call(obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, &count);
         eo_parent_set(child, NULL);
         eo_unref(child);
     }
}

static void
_ejson_model_efl_model_children_slice_get(Eo *obj EINA_UNUSED,
                                       Ejson_Model_Data *pd,
                                       unsigned start,
                                       unsigned count,
                                       Eina_Promise_Owner *promise)
{

   Eina_Accessor *children_accessor = efl_model_list_slice(pd->children_list, start, count);
   if (children_accessor)
     eina_promise_owner_value_set(promise, children_accessor, &_accessor_free);
   else
     eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
}

static void
_ejson_model_efl_model_children_count_get(Eo *obj EINA_UNUSED,
                                       Ejson_Model_Data *pd,
                                       Eina_Promise_Owner *promise)
{
   unsigned int children_count = eina_list_count(pd->children_list);
   eina_promise_owner_value_set(promise, &children_count, NULL);
}



static Ejson_Model_Type
_ejson_model_type_get(Eo *obj EINA_UNUSED, Ejson_Model_Data *pd)
{
   return pd->type;
}

static void
_ejson_model_type_set(Eo *obj EINA_UNUSED, Ejson_Model_Data *pd, const Ejson_Model_Type type)
{
   EINA_SAFETY_ON_FALSE_RETURN(type >= 0 && type <= EJSON_MODEL_TYPE_ARRAY);
   pd->type = type;
}

static void
_ejson_model_clear(Ejson_Model_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN(pd);

   // No need to free each child as they were added through eo_add
   eina_list_free(pd->children_list);

   if (pd->properties_array)
     {
        eina_array_free(pd->properties_array);
        pd->properties_array = NULL;
     }

   pd->json_property_valid = false;

   free((char*)pd->name);
   pd->name = NULL;

   eina_value_free(pd->value);
}

/* JSON SIMPLE PARSE INIT */

static Eina_Bool
_parse_simple_child_setup(Ejson_Model *obj, Eina_Simple_JSON_Value_Type type, const char *content, unsigned length)
{
   Ejson_Model_Type mtype;
   Eina_Value value;
   Eina_Promise *promise = NULL;

   switch(type)
     {
      case EINA_SIMPLE_JSON_VALUE_OBJECT:
        {
            ejson_model_type_set(obj, EJSON_MODEL_TYPE_OBJECT);
            return eina_simple_json_object_parse(content, length, _parse_simple_obj_cb, obj);
        }
      case EINA_SIMPLE_JSON_VALUE_ARRAY:
        {
            ejson_model_type_set(obj, EJSON_MODEL_TYPE_ARRAY);
            return eina_simple_json_object_parse(content, length, _parse_simple_obj_cb, obj);
        }
      case EINA_SIMPLE_JSON_VALUE_STRING:
        {
            char buf[length+1];
            mtype = EJSON_MODEL_TYPE_STRING;
            memcpy(&buf, content, length);
            buf[length] = '\0';
            eina_value_setup(&value, EINA_VALUE_TYPE_STRING);
            eina_value_set(&value, buf);
            break;
        }
      case EINA_SIMPLE_JSON_VALUE_BOOLEAN:
        {
            unsigned int b = !strncmp(content, "true", sizeof("true")-1);
            mtype = EJSON_MODEL_TYPE_BOOLEAN;
            eina_value_setup(&value, EINA_VALUE_TYPE_INT);
            eina_value_set(&value, b);
            break;
        }
      case EINA_SIMPLE_JSON_VALUE_INT:
        {
            char v[length+1];
            mtype = EJSON_MODEL_TYPE_INT;

            memcpy(&v, content, length);
            v[length] = '\0';
            int64_t n = atoi(v);
            eina_value_setup(&value, EINA_VALUE_TYPE_INT64);
            eina_value_set(&value, n);
            break;
        }
      case EINA_SIMPLE_JSON_VALUE_DOUBLE:
        {
            char v[length+1];
            mtype = EJSON_MODEL_TYPE_DOUBLE;

            memcpy(&v, content, length);
            v[length] = '\0';
            double n = atof(v);
            eina_value_setup(&value, EINA_VALUE_TYPE_DOUBLE);
            eina_value_set(&value, &n);
            break;
        }
      case EINA_SIMPLE_JSON_VALUE_NULL:
      default:
        {
            mtype = EJSON_MODEL_TYPE_NULL;
            eina_value_setup(&value, EINA_VALUE_TYPE_STRING);
            eina_value_set(&value, NULL);
        }
     }


   ejson_model_type_set(obj, mtype);
   efl_model_property_set(obj, EJSON_MODEL_VALUE_PROPERTY, &value, &promise);
   eina_value_flush(&value);
   eina_promise_unref(promise);

   return EINA_TRUE;
}

static Eina_Bool
_parse_simple_array_cb(void *data, Eina_Simple_JSON_Value_Type type, const char *content, unsigned length)
{
   Ejson_Model *parent = data;
   Eo* child;
   child = efl_model_child_add(parent);

   return _parse_simple_child_setup(child, type, content, length);
}

static Eina_Bool
_parse_simple_obj_cb(void *data, Eina_Simple_JSON_Value_Type type, const char *key, const char *content, unsigned length)
{
   Ejson_Model *parent = data;
   Eo* child;
   Eina_Promise *promise = NULL;
   child = efl_model_child_add(parent);

   Eina_Value value;
   eina_value_setup(&value, EINA_VALUE_TYPE_STRING);
   eina_value_set(&value, key);
   efl_model_property_set(child, EJSON_MODEL_NAME_PROPERTY, &value, &promise);
   eina_value_flush(&value);

   if (eina_promise_error_get(promise))
     {
        eina_promise_unref(promise);
        return EINA_FALSE;
     }

   eina_promise_unref(promise);

   return _parse_simple_child_setup(child, type, content, length);
}

static Eina_Bool
_parse_simple_load_cb(void *data, Eina_Simple_JSON_Type type, const char *content, unsigned length)
{
   Ejson_Model_Data *pd = data;

   if (type == EINA_SIMPLE_JSON_OBJECT)
     {
       pd->type = EJSON_MODEL_TYPE_OBJECT;
       return eina_simple_json_object_parse(content, length, _parse_simple_obj_cb, pd->obj);
     }
   else if (type == EINA_SIMPLE_JSON_ARRAY)
     {
        pd->type = EJSON_MODEL_TYPE_ARRAY;
        return eina_simple_json_array_parse(content, length, _parse_simple_array_cb, pd->obj);
     }

   return EINA_FALSE;
}

static bool
_ejson_model_stream_load(Ejson_Model_Data *pd, const char* stream, unsigned slen)
{
    if (!stream || !eina_simple_json_parse(stream, slen, _parse_simple_load_cb, pd))
      return false;

    pd->json_property_valid = true;
    return true;
}
/* JSON SIMPLE PARSE END */

static Eina_Bool
_ejson_model_eina_value_as(const Eina_Value *value, const Eina_Value_Type *type, void *mem)
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


#include "ejson_model.eo.c"

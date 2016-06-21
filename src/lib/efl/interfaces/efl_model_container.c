#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

#include "interfaces/efl_model_private.h"

#define MY_CLASS EFL_MODEL_CONTAINER_CLASS

void *
_value_copy_alloc(void *v, const Eina_Value_Type *type)
{
   if (!v)
     return v;

   if (type == EINA_VALUE_TYPE_STRINGSHARE)
     return (void*) eina_stringshare_ref(v);
   else if (type == EINA_VALUE_TYPE_STRING)
     return (void*) strdup(v);
   else
     {
        void *ret = malloc(type->value_size);
        memcpy(ret, v, type->value_size);
        return ret;
     }
}

void
_value_free(void *v, const Eina_Value_Type *type)
{
   if (!v)
     return;

   if (type == EINA_VALUE_TYPE_STRINGSHARE)
     return eina_stringshare_del(v);
   else
     free(v);
}

static void
_values_free(Eina_Array *values, const Eina_Value_Type *type)
{
   unsigned int i;
   void *v;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(values, i, v, it)
     {
        _value_free(v, type);
     }
   eina_array_free(values);
}

static Eina_Bool
_stringshared_keys_free(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data EINA_UNUSED, void *fdata EINA_UNUSED)
{
   eina_stringshare_del(key);
   return EINA_TRUE;
}

static void
_property_data_free_cb(void *data)
{
   Child_Property_Data *cpd = data;
   _values_free(cpd->property_values, cpd->property_type);
   free(cpd);
}

EOLIAN static Eo *
_efl_model_container_eo_base_constructor(Eo *obj, Efl_Model_Container_Data *sd)
{
   obj = eo_constructor(eo_super(obj, MY_CLASS));
   if (!obj)
     return NULL;

   sd->obj = obj;
   sd->property_data = eina_hash_stringshared_new(_property_data_free_cb);
   sd->defined_properties = eina_array_new(8);

   return obj;
}

EOLIAN static void
_efl_model_container_eo_base_destructor(Eo *obj, Efl_Model_Container_Data *sd)
{
   Efl_Model *child;

   EINA_LIST_FREE(sd->children, child)
     {
        if (child)
          {
             efl_model_container_item_invalidate(child);
             eo_parent_set(child, NULL);
          }
     }

   eina_array_free(sd->defined_properties);

   eina_hash_foreach(sd->property_data, _stringshared_keys_free, NULL);
   eina_hash_free(sd->property_data);

   eo_destructor(eo_super(obj, MY_CLASS));
}

EOLIAN static const Eina_Value_Type *
_efl_model_container_child_property_value_type_get(Eo *obj EINA_UNUSED, Efl_Model_Container_Data *sd, const char *property)
{
   Eina_Stringshare *sshared = eina_stringshare_add(property);
   Child_Property_Data *cpd = eina_hash_find(sd->property_data, sshared);
   eina_stringshare_del(sshared);
   if (!cpd)
     return NULL;

   return cpd->property_type;
}

EOLIAN static Eina_Iterator *
_efl_model_container_child_property_values_get(Eo *obj EINA_UNUSED, Efl_Model_Container_Data *sd, const char *property)
{
   Eina_Stringshare *sshared = eina_stringshare_add(property);
   Child_Property_Data *cpd = eina_hash_find(sd->property_data, sshared);
   eina_stringshare_del(sshared);
   if (!cpd)
     return NULL;

   return eina_array_iterator_new(cpd->property_values);
}

EOLIAN static Eina_Bool
_efl_model_container_child_property_add(Eo *obj, Efl_Model_Container_Data *sd, const char *name, const Eina_Value_Type *type, Eina_Iterator *values)
{
   Eina_Array *arr = NULL;
   void *data = NULL;
   Eina_Stringshare *prop_name = NULL;
   Child_Property_Data *cpd = NULL;
   unsigned int i, in_count, children_count;

   if (!type || !values)
     {
        EINA_LOG_WARN("Invalid input data");
        eina_error_set(EFL_MODEL_ERROR_INCORRECT_VALUE);
        return EINA_FALSE;
     }

   arr = eina_array_new(32);
   if (!arr)
     {
        eina_error_set(EFL_MODEL_ERROR_UNKNOWN);
        return EINA_FALSE;
     }

   EINA_ITERATOR_FOREACH(values, data)
     {
        void *new_data = _value_copy_alloc(data, type);
        if ((data && !new_data) || !eina_array_push(arr, new_data))
          {
             if (new_data)
               _value_free(new_data, type);
             goto error;
          }
     }
   eina_iterator_free(values);

   prop_name = eina_stringshare_add(name);
   cpd = eina_hash_find(sd->property_data, prop_name);
   if (!cpd)
     {
        cpd = calloc(1, sizeof(Child_Property_Data));
        if (!cpd)
          goto error;

        cpd->property_type = type;
        cpd->property_values = arr;

        if (!eina_array_push(sd->defined_properties, prop_name))
          goto error;

        if (!eina_hash_direct_add(sd->property_data, prop_name, cpd))
          {
             eina_array_pop(sd->defined_properties);
             goto error;
          }
     }
   else
     {
        eina_stringshare_del(prop_name);
        _values_free(cpd->property_values, cpd->property_type);

        cpd->property_type = type;
        cpd->property_values = arr;
     }

   in_count = eina_array_count(arr);
   children_count = eina_list_count(sd->children);

   for (i = children_count; i < in_count; ++i)
     {
        Efl_Model_Children_Event cevt;
        Efl_Model *child;

        child = eo_add(EFL_MODEL_CONTAINER_ITEM_CLASS, obj,
                       efl_model_container_item_define(eo_self, sd, i));
        sd->children = eina_list_append(sd->children, child);

        cevt.child = child;
        cevt.index = i;
        eo_event_callback_call(obj, EFL_MODEL_EVENT_CHILD_ADDED, &cevt);
     }

   if (in_count > children_count)
     eo_event_callback_call(obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, &in_count);

   return EINA_TRUE;

error:
   if (prop_name)
     eina_stringshare_del(prop_name);
   if (cpd)
     free(cpd);
   if (arr)
     _values_free(arr, type);
   eina_error_set(EFL_MODEL_ERROR_UNKNOWN);
   return EINA_FALSE;
}

EOLIAN static const Eina_Array *
_efl_model_container_efl_model_properties_get(Eo *obj EINA_UNUSED, Efl_Model_Container_Data *sd EINA_UNUSED)
{
   return sd->defined_properties;
}


EOLIAN static void
_efl_model_container_efl_model_property_set(Eo *obj EINA_UNUSED, Efl_Model_Container_Data *sd EINA_UNUSED, const char *property EINA_UNUSED, const Eina_Value *value EINA_UNUSED, Eina_Promise_Owner *promise_owner)
{
   eina_promise_owner_error_set(promise_owner, EFL_MODEL_ERROR_NOT_FOUND);
}

EOLIAN static Eina_Promise *
_efl_model_container_efl_model_property_get(Eo *obj EINA_UNUSED, Efl_Model_Container_Data *sd EINA_UNUSED, const char *property EINA_UNUSED)
{
   Eina_Promise_Owner *promise_owner = eina_promise_add();
   Eina_Promise *rpromise = eina_promise_owner_promise_get(promise_owner);

   eina_promise_owner_error_set(promise_owner, EFL_MODEL_ERROR_NOT_FOUND);

   return rpromise;
}

EOLIAN static Eina_Promise *
_efl_model_container_efl_model_children_slice_get(Eo *obj EINA_UNUSED, Efl_Model_Container_Data *sd, unsigned int start, unsigned int count)
{
   Eina_Promise_Owner *promise_owner = eina_promise_add();
   Eina_Promise *rpromise = eina_promise_owner_promise_get(promise_owner);

   Eina_Accessor* accessor = efl_model_list_slice(sd->children, start, count);
   eina_promise_owner_value_set(promise_owner, accessor, (Eina_Promise_Free_Cb)&eina_accessor_free);

   return rpromise;
}

EOLIAN static Eina_Promise *
_efl_model_container_efl_model_children_count_get(Eo *obj EINA_UNUSED, Efl_Model_Container_Data *sd)
{
   Eina_Promise_Owner *promise_owner = eina_promise_add();
   Eina_Promise *rpromise = eina_promise_owner_promise_get(promise_owner);

   unsigned int *count = calloc(1, sizeof(unsigned int));
   *count = eina_list_count(sd->children);
   eina_promise_owner_value_set(promise_owner, count, &free);

   return rpromise;
}

EOLIAN static Eo *
_efl_model_container_efl_model_child_add(Eo *obj EINA_UNUSED, Efl_Model_Container_Data *sd EINA_UNUSED)
{
   EINA_LOG_WARN("child_add not supported by Efl.Model.Container");
   eina_error_set(EFL_MODEL_ERROR_NOT_SUPPORTED);

   return NULL;
}

EOLIAN static void
_efl_model_container_efl_model_child_del(Eo *obj EINA_UNUSED, Efl_Model_Container_Data *sd EINA_UNUSED, Eo *child EINA_UNUSED)
{
   EINA_LOG_WARN("child_del not supported by Efl.Model.Container");
   eina_error_set(EFL_MODEL_ERROR_NOT_SUPPORTED);
}

#include "interfaces/efl_model_container.eo.c"

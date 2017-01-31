#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>
#include <Ecore.h>

#include "efl_model_container_private.h"

#define MY_CLASS EFL_MODEL_CONTAINER_ITEM_CLASS

static void
_item_value_free_cb(void *data)
{
   eina_value_free(data);
}

EOLIAN static void
_efl_model_container_item_define(Eo *obj EINA_UNUSED, Efl_Model_Container_Item_Data *sd, void *parent_data, unsigned int index)
{
   sd->parent_data = parent_data;
   sd->index = index;
}

EOLIAN static void
_efl_model_container_item_invalidate(Eo *obj EINA_UNUSED, Efl_Model_Container_Item_Data *sd)
{
   sd->parent_data = NULL;
   sd->index = 0;
}

EOLIAN static const Eina_Array *
_efl_model_container_item_efl_model_properties_get(Eo *obj EINA_UNUSED, Efl_Model_Container_Item_Data *sd)
{
   if (!sd->parent_data)
     return NULL;

   return sd->parent_data->defined_properties;
}

Efl_Future *
_efl_model_container_item_efl_model_property_set(Eo *obj EINA_UNUSED, Efl_Model_Container_Item_Data *sd, const char *property, const Eina_Value *value)
{
   Eina_Stringshare *prop_name;
   Child_Property_Data *cpd;
   void *data, *new_data;
   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   Efl_Future* future = efl_promise_future_get(promise);

   if (!sd->parent_data)
     {
        efl_promise_failed_set(promise, EFL_MODEL_ERROR_INVALID_OBJECT);
        return future;
     }

   prop_name = eina_stringshare_add(property);
   cpd = eina_hash_find(sd->parent_data->property_data, prop_name);
   eina_stringshare_del(prop_name);
   if (!cpd || !cpd->property_values ||
       sd->index >= eina_array_count_get(cpd->property_values))
     {
        efl_promise_failed_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
        return future;
     }

   if (cpd->property_type != eina_value_type_get(value))
     {
        efl_promise_failed_set(promise, EFL_MODEL_ERROR_INCORRECT_VALUE);
        return future;
     }

   data = calloc(1, cpd->property_type->value_size);
   if (!data || !eina_value_pget(value, data))
     {
        if (data)
          free(data);
        efl_promise_failed_set(promise, EFL_MODEL_ERROR_UNKNOWN);
        return future;
     }

   new_data = _value_copy_alloc(data, cpd->property_type);
   free(data);

   _value_free(eina_array_data_get(cpd->property_values, sd->index), cpd->property_type);
   eina_array_data_set(cpd->property_values, sd->index, new_data);

   {
      Eina_Value *v = eina_value_new(cpd->property_type);
      if (v && eina_value_copy(value, v))
        efl_promise_value_set(promise, v, _item_value_free_cb);
      else
        {
           if (v)
             eina_value_free(v);
           efl_promise_failed_set(promise, EFL_MODEL_ERROR_UNKNOWN);
        }
   }

   return future;
}

Efl_Future *
_efl_model_container_item_efl_model_property_get(Eo *obj EINA_UNUSED, Efl_Model_Container_Item_Data *sd, const char *property)
{
   Eina_Stringshare *prop_name;
   Child_Property_Data *cpd;
   Eina_Value *value;
   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   Efl_Future* future = efl_promise_future_get(promise);

   if (!sd->parent_data)
     {
        efl_promise_failed_set(promise, EFL_MODEL_ERROR_INVALID_OBJECT);
        return future;
     }

   prop_name = eina_stringshare_add(property);
   cpd = eina_hash_find(sd->parent_data->property_data, prop_name);
   eina_stringshare_del(prop_name);
   if (!cpd || !cpd->property_values ||
       sd->index >= eina_array_count_get(cpd->property_values))
     {
        efl_promise_failed_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
        return future;
     }

   value = eina_value_new(cpd->property_type);
   if (!value)
        efl_promise_failed_set(promise, EFL_MODEL_ERROR_UNKNOWN);
   else
     {
        Eina_Bool r = EINA_FALSE;
        void *data = eina_array_data_get(cpd->property_values, sd->index);

        if (cpd->property_type == EINA_VALUE_TYPE_STRINGSHARE ||
            cpd->property_type == EINA_VALUE_TYPE_STRING)
          r = eina_value_set(value, data);
        else
          r = eina_value_pset(value, data);

        if (r)
          efl_promise_value_set(promise, value, _item_value_free_cb);
        else
          {
             efl_promise_failed_set(promise, EFL_MODEL_ERROR_UNKNOWN);
             eina_value_free(value);
          }
     }
   return future;
}

EOLIAN static Efl_Future *
_efl_model_container_item_efl_model_children_slice_get(Eo *obj EINA_UNUSED, Efl_Model_Container_Item_Data *sd EINA_UNUSED, unsigned int start EINA_UNUSED, unsigned int count EINA_UNUSED)
{
   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   Efl_Future* future = efl_promise_future_get(promise);

   efl_promise_value_set(promise, NULL, NULL);

   return future;
}

EOLIAN static Efl_Future *
_efl_model_container_item_efl_model_children_count_get(Eo *obj EINA_UNUSED, Efl_Model_Container_Item_Data *sd EINA_UNUSED)
{
   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   Efl_Future* future = efl_promise_future_get(promise);

   unsigned int *count = calloc(1, sizeof(unsigned int));
   *count = 0;
   efl_promise_value_set(promise, count, &free);

   return future;
}

EOLIAN static Eo *
_efl_model_container_item_efl_model_child_add(Eo *obj EINA_UNUSED, Efl_Model_Container_Item_Data *sd EINA_UNUSED)
{
   EINA_LOG_WARN("child_add not supported by Efl.Model.Container.Item");
   eina_error_set(EFL_MODEL_ERROR_NOT_SUPPORTED);

   return NULL;
}

EOLIAN static void
_efl_model_container_item_efl_model_child_del(Eo *obj EINA_UNUSED, Efl_Model_Container_Item_Data *sd EINA_UNUSED, Eo *child EINA_UNUSED)
{
   EINA_LOG_WARN("child_del not supported by Efl.Model.Container.Item");
   eina_error_set(EFL_MODEL_ERROR_NOT_SUPPORTED);
}

#include "efl_model_container_item.eo.c"

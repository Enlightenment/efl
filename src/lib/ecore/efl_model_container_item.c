#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>
#include <Ecore.h>

#include "efl_model_container_private.h"

#define MY_CLASS EFL_MODEL_CONTAINER_ITEM_CLASS

static void
_efl_model_container_item_define(Eo *obj EINA_UNUSED, Efl_Model_Container_Item_Data *sd, void *parent_data, unsigned int index)
{
   sd->parent_data = parent_data;
   sd->index = index;
}

EOLIAN static void
_efl_model_container_item_efl_object_invalidate(Eo *obj, Efl_Model_Container_Item_Data *sd)
{
   efl_invalidate(efl_super(obj, MY_CLASS));

   sd->parent_data = NULL;
   sd->index = 0;
}

static Eina_Array *
_efl_model_container_item_efl_model_properties_get(const Eo *obj EINA_UNUSED, Efl_Model_Container_Item_Data *sd)
{
   // FIXME: Not to sure here, shouldn't we extend a child of the parent actually ?
   return efl_model_properties_get(sd->parent_data->obj);
}

static Efl_Object *
_efl_model_container_item_efl_object_finalize(Eo *obj, Efl_Model_Container_Item_Data *pd)
{
   if (!pd->parent_data)
     return NULL;

   return obj;
}

static Eina_Future *
_efl_model_container_item_efl_model_property_set(Eo *obj,
                                                 Efl_Model_Container_Item_Data *sd,
                                                 const char *property, Eina_Value *value)
{
   Eina_Stringshare *name;
   Child_Property_Data *cpd;
   Eina_Value v = EINA_VALUE_EMPTY;
   void *data, *new_data;


   name = eina_stringshare_add(property);
   cpd = eina_hash_find(sd->parent_data->properties, name);
   eina_stringshare_del(name);

   if (!cpd || !cpd->values ||
       sd->index >= eina_array_count_get(cpd->values))
     return eina_future_rejected(efl_loop_future_scheduler_get(obj),
                                 EFL_MODEL_ERROR_NOT_FOUND);

   eina_value_setup(&v,cpd->type);
   if (!eina_value_convert(value, &v))
     return eina_future_rejected(efl_loop_future_scheduler_get(obj),
                                 EFL_MODEL_ERROR_INCORRECT_VALUE);

   // FIXME: This is trying to optimize and avoid the use of Eina_Value,
   // but this put restriction on the type that can be stored in this container.
   data = calloc(1, cpd->type->value_size);
   if (!data || !eina_value_pget(&v, data))
     goto on_error;

   new_data = _value_copy_alloc(data, cpd->type);

   _value_free(eina_array_data_get(cpd->values, sd->index), cpd->type);

   eina_array_data_set(cpd->values, sd->index, new_data);

   free(data);

   return eina_future_resolved(efl_loop_future_scheduler_get(obj), v);

 on_error:
   eina_value_flush(&v);
   free(data);

   return eina_future_rejected(efl_loop_future_scheduler_get(obj),
                               EFL_MODEL_ERROR_UNKNOWN);
}

static Eina_Value *
_efl_model_container_item_efl_model_property_get(const Eo *obj EINA_UNUSED,
                                                 Efl_Model_Container_Item_Data *sd,
                                                 const char *property)
{
   Eina_Stringshare *name;
   Child_Property_Data *cpd;
   Eina_Value *value;
   void *data;
   Eina_Bool r = EINA_FALSE;

   name = eina_stringshare_add(property);
   cpd = eina_hash_find(sd->parent_data->properties, name);
   eina_stringshare_del(name);

   if (!cpd || !cpd->values ||
       sd->index >= eina_array_count_get(cpd->values))
     return eina_value_error_new(EFL_MODEL_ERROR_NOT_FOUND);

   data = eina_array_data_get(cpd->values, sd->index);

   value = eina_value_new(cpd->type);
   if (cpd->type == EINA_VALUE_TYPE_STRINGSHARE ||
       cpd->type == EINA_VALUE_TYPE_STRING)
     r = eina_value_set(value, data);
   else
     r = eina_value_pset(value, data);

   if (!r)
     {
        eina_value_free(value);
        value = eina_value_error_new(EFL_MODEL_ERROR_UNKNOWN);
     }
   return value;
}

EOLIAN static Eina_Future *
_efl_model_container_item_efl_model_children_slice_get(Eo *obj,
                                                       Efl_Model_Container_Item_Data *sd EINA_UNUSED,
                                                       unsigned int start EINA_UNUSED,
                                                       unsigned int count EINA_UNUSED)
{
   return eina_future_resolved(efl_loop_future_scheduler_get(obj), EINA_VALUE_EMPTY);
}

EOLIAN static unsigned int
_efl_model_container_item_efl_model_children_count_get(const Eo *obj EINA_UNUSED,
                                                       Efl_Model_Container_Item_Data *sd EINA_UNUSED)
{
   return 0;
}

EOLIAN static Eo *
_efl_model_container_item_efl_model_child_add(Eo *obj EINA_UNUSED,
                                              Efl_Model_Container_Item_Data *sd EINA_UNUSED)
{
   EINA_LOG_WARN("child_add not supported by Efl.Model.Container.Item");
   eina_error_set(EFL_MODEL_ERROR_NOT_SUPPORTED);

   return NULL;
}

EOLIAN static void
_efl_model_container_item_efl_model_child_del(Eo *obj EINA_UNUSED,
                                              Efl_Model_Container_Item_Data *sd EINA_UNUSED,
                                              Eo *child EINA_UNUSED)
{
   EINA_LOG_WARN("child_del not supported by Efl.Model.Container.Item");
   eina_error_set(EFL_MODEL_ERROR_NOT_SUPPORTED);
}

#include "efl_model_container_item.eo.c"

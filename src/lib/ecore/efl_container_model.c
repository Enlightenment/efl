#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>
#include <Eina.h>
#include <Eo.h>
#include <Ecore.h>

#include "ecore_internal.h"

#include "efl_container_model_private.h"

#define MY_CLASS EFL_CONTAINER_MODEL_CLASS

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

static void
_property_data_free_cb(void *data)
{
   Child_Property_Data *cpd = data;
   _values_free(cpd->values, cpd->type);
   free(cpd);
}

static Efl_Object *
_efl_container_model_efl_object_constructor(Eo *obj,
                                            Efl_Container_Model_Data *sd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   if (!obj)
     return NULL;

   sd->obj = obj;
   sd->properties = eina_hash_stringshared_new(_property_data_free_cb);

   return obj;
}

static void
_efl_container_model_efl_object_destructor(Eo *obj,
                                           Efl_Container_Model_Data *sd)
{
   Eina_Stringshare *key;
   Eina_Iterator *it;

   eina_list_free(sd->childrens);

   it = eina_hash_iterator_key_new(sd->properties);
   EINA_ITERATOR_FOREACH(it, key)
     eina_stringshare_del(key);
   eina_iterator_free(it);

   eina_hash_free(sd->properties);

   efl_destructor(efl_super(obj, MY_CLASS));
}

static const Eina_Value_Type *
_efl_container_model_child_property_value_type_get(Eo *obj EINA_UNUSED,
                                                   Efl_Container_Model_Data *sd,
                                                   const char *property)
{
   Child_Property_Data *cpd;
   Eina_Stringshare *key;

   key = eina_stringshare_add(property);
   cpd = eina_hash_find(sd->properties, key);
   eina_stringshare_del(key);

   if (!cpd) return NULL;
   return cpd->type;
}

static Eina_Iterator *
_efl_container_model_child_property_values_get(Eo *obj EINA_UNUSED,
                                               Efl_Container_Model_Data *sd,
                                               const char *property)
{
   Child_Property_Data *cpd;
   Eina_Stringshare *key;

   key = eina_stringshare_add(property);
   cpd = eina_hash_find(sd->properties, key);
   eina_stringshare_del(key);

   if (!cpd) return NULL;
   return eina_array_iterator_new(cpd->values);
}

static Eina_Bool
_efl_container_model_child_property_add(Eo *obj,
                                        Efl_Container_Model_Data *sd,
                                        const char *name,
                                        const Eina_Value_Type *type,
                                        Eina_Iterator *values)
{
   Eina_Array *arr = NULL;
   void *data = NULL;
   Child_Property_Data *cpd = NULL;
   unsigned int i, in_count, children_count;
   Eina_Error err = EFL_MODEL_ERROR_INCORRECT_VALUE;

   name = eina_stringshare_add(name);

   if (!type || !values)
     {
        EINA_LOG_WARN("Invalid input data");
        goto on_error;
     }

   err = ENOMEM;
   arr = eina_array_new(4);
   if (!arr) goto on_error;

   EINA_ITERATOR_FOREACH(values, data)
     {
        void *new_data = _value_copy_alloc(data, type);
        if ((data && !new_data) || !eina_array_push(arr, new_data))
          {
             if (new_data)
               _value_free(new_data, type);
             goto on_error;
          }
     }
   eina_iterator_free(values);

   err = EFL_MODEL_ERROR_UNKNOWN;

   cpd = eina_hash_find(sd->properties, name);
   if (!cpd)
     {
        cpd = calloc(1, sizeof(Child_Property_Data));
        if (!cpd)
          goto on_error;

        cpd->type = type;
        cpd->values = arr;

        if (!eina_hash_direct_add(sd->properties, name, cpd))
          goto on_error;
     }
   else
     {
        eina_stringshare_del(name);
        _values_free(cpd->values, cpd->type);

        cpd->type = type;
        cpd->values = arr;
     }

   in_count = eina_array_count(arr);
   children_count = eina_list_count(sd->childrens);

   for (i = children_count; i < in_count; ++i)
     {
        Efl_Model_Children_Event cevt = { 0 };
        Efl_Model *child;

        child = efl_add(EFL_CONTAINER_MODEL_ITEM_CLASS, obj,
                       efl_container_model_item_define(efl_added, sd, i));
        sd->childrens = eina_list_append(sd->childrens, child);

        cevt.index = i;

        efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILD_ADDED, &cevt);
     }

   if (in_count > children_count)
     efl_event_callback_call(obj, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, NULL);

   return EINA_TRUE;

 on_error:
   eina_stringshare_del(name);
   if (cpd) free(cpd);
   if (arr)
     _values_free(arr, type);
   eina_error_set(err);
   return EINA_FALSE;
}

static Eina_Iterator *
_efl_container_model_efl_model_properties_get(const Eo *obj EINA_UNUSED, Efl_Container_Model_Data *sd)
{
   return eina_hash_iterator_key_new(sd->properties);
}

static Eina_Future *
_efl_container_model_efl_model_property_set(Eo *obj,
                                            Efl_Container_Model_Data *pd EINA_UNUSED,
                                            const char *property EINA_UNUSED,
                                            Eina_Value *value EINA_UNUSED)
{
   return efl_loop_future_rejected(obj,
                               EFL_MODEL_ERROR_NOT_FOUND);
}

static Eina_Value *
_efl_container_model_efl_model_property_get(const Eo *obj EINA_UNUSED,
                                            Efl_Container_Model_Data *pd EINA_UNUSED,
                                            const char *property EINA_UNUSED)
{
   return eina_value_error_new(EFL_MODEL_ERROR_NOT_FOUND);
}

static Eina_Future *
_efl_container_model_efl_model_children_slice_get(Eo *obj,
                                                  Efl_Container_Model_Data *sd,
                                                  unsigned int start,
                                                  unsigned int count)
{
   Eina_Value v;

   v = efl_model_list_value_get(sd->childrens, start, count);
   return efl_loop_future_resolved(obj, v);
}

static unsigned int
_efl_container_model_efl_model_children_count_get(const Eo *obj EINA_UNUSED, Efl_Container_Model_Data *sd)
{
   return eina_list_count(sd->childrens);
}

static Eo *
_efl_container_model_efl_model_child_add(Eo *obj EINA_UNUSED, Efl_Container_Model_Data *sd EINA_UNUSED)
{
   EINA_LOG_WARN("child_add not supported by Efl.Model.Container");
   eina_error_set(EFL_MODEL_ERROR_NOT_SUPPORTED);

   return NULL;
}

static void
_efl_container_model_efl_model_child_del(Eo *obj EINA_UNUSED, Efl_Container_Model_Data *sd EINA_UNUSED, Eo *child EINA_UNUSED)
{
   EINA_LOG_WARN("child_del not supported by Efl.Model.Container");
   eina_error_set(EFL_MODEL_ERROR_NOT_SUPPORTED);
}

#include "efl_container_model.eo.c"

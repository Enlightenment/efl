#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eina.h"
#include "Efl.h"

#include "interfaces/efl_model_composite_boolean_children.eo.h"

typedef struct _Efl_Model_Hash_Value
{
   unsigned char* buffer;
   unsigned int bits_count;
   Eina_Bool default_value : 1;
} Efl_Model_Hash_Value;

typedef struct _Efl_Model_Composite_Boolean_Data
{
   Efl_Model* composite_model;
   Eina_Array* empty_properties;
   Eina_Hash* values; // [property_name, Efl_Model_Hash_Value*]
} Efl_Model_Composite_Boolean_Data;

typedef struct _Efl_Model_Composite_Boolean_Children_Data
{
   Efl_Model_Composite_Boolean_Data* parent_pd;
   Efl_Model* composite_children;
   Eina_Array* properties_names;
   unsigned int index;
} Efl_Model_Composite_Boolean_Children_Data;

typedef struct _Efl_Model_Iterator_Slice
{
   Eina_Iterator vtable;
   Eina_Iterator* composite_iterator;
   Efl_Model* parent;
   Efl_Model_Composite_Boolean_Data* parent_pd;
   Eina_Promise_Owner* promise;
   unsigned int index;
} Efl_Model_Iterator_Slice;

/**************** efl_mmodel_composite_boolean_children **************/

static const Eina_Array*
_efl_model_composite_boolean_children_efl_model_properties_get(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Boolean_Children_Data *pd)
{
   Eina_Array const* composite_properties;
   unsigned int i;
  
   if(pd->properties_names) return pd->properties_names;
   else
     {
        composite_properties = efl_model_properties_get(pd->parent_pd->composite_model);
        unsigned int composite_count = eina_array_count_get(composite_properties),
          self_count = eina_hash_population(pd->parent_pd->values);
        pd->properties_names = eina_array_new(composite_count + self_count);
        for(i = 0; i != composite_count; ++i)
          {
             const char* name = strdup(eina_array_data_get(composite_properties, i));
             eina_array_push(pd->properties_names, name);
          }
        return pd->properties_names;
     }
}

static Eina_Promise*
_efl_model_composite_boolean_children_efl_model_property_get(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Boolean_Children_Data *pd, const char *property)
{
   Efl_Model_Hash_Value* value = eina_hash_find(pd->parent_pd->values, property);
   if(value)
     {
        Eina_Promise_Owner* promise = eina_promise_add();
        Eina_Value* eina_value = eina_value_new(EINA_VALUE_TYPE_UCHAR);
        if(value->bits_count <= pd->index)
          {
            unsigned char f = value->default_value;
            eina_value_set(eina_value, f);
          }
        else
          {
            eina_value_set(eina_value, (value->buffer[pd->index / 8] >> (pd->index % 8))&1);
          }
        eina_promise_owner_value_set(promise, eina_value, (Eina_Promise_Free_Cb)eina_value_free);
        return eina_promise_owner_promise_get(promise);
     }
   else if(pd->composite_children)
     return efl_model_property_get(pd->composite_children, property);
   else
     {
        Eina_Promise_Owner* promise = eina_promise_add();
        eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
        return eina_promise_owner_promise_get(promise);
     }
}

static void
_efl_model_composite_boolean_children_efl_model_property_set(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Boolean_Children_Data *pd, const char *property, const Eina_Value *value, Eina_Promise_Owner *promise)
{
   
}

static Eina_Promise*
_efl_model_composite_boolean_children_efl_model_children_slice_get(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Boolean_Children_Data *pd, unsigned int start, unsigned int count)
{
   return efl_model_children_slice_get(pd->composite_children, start, count);
}

static Eina_Promise*
_efl_model_composite_boolean_children_efl_model_children_count_get(Eo *obj  EINA_UNUSED,
  Efl_Model_Composite_Boolean_Children_Data *pd)
{
   return efl_model_children_count_get(pd->composite_children);
}

static Eo_Base*
_efl_model_composite_boolean_children_efl_model_child_add(Eo *obj  EINA_UNUSED,
  Efl_Model_Composite_Boolean_Children_Data *pd)
{
   return efl_model_child_add(pd->composite_children);
}

static void
_efl_model_composite_boolean_children_efl_model_child_del(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Boolean_Children_Data *pd, Eo_Base *child)
{
   return efl_model_child_del(pd->composite_children, child);
}

/**************** efl_mmodel_iterator_slice **************/

static Eina_Bool
efl_model_iterator_slice_next(Efl_Model_Iterator_Slice* iterator, void** data)
{
  Efl_Model* child;
  if(eina_iterator_next(iterator->composite_iterator, (void**)&child))
    {
       Efl_Model* model = eo_add(EFL_MODEL_COMPOSITE_BOOLEAN_CHILDREN_CLASS, iterator->parent);
       Efl_Model_Composite_Boolean_Children_Data* pd =
         eo_data_scope_get(model, EFL_MODEL_COMPOSITE_BOOLEAN_CHILDREN_CLASS);
       pd->parent_pd = iterator->parent_pd;
       pd->composite_children = eo_ref(child);
       pd->index = iterator->index++;
       eo_data_ref(iterator->parent, EFL_MODEL_COMPOSITE_BOOLEAN_CLASS);
       eo_data_unref(model, pd);
       *data = model;
       return EINA_TRUE;
    }
  else
    return EINA_FALSE;
}

static void*
efl_model_iterator_slice_container(Efl_Model_Iterator_Slice* iterator)
{
  return iterator->composite_iterator;
}

static void
efl_model_iterator_slice_free(Efl_Model_Iterator_Slice* iterator)
{
  eina_iterator_free(iterator->composite_iterator);
  free(iterator);
}

static Eina_Bool
efl_model_iterator_slice_lock(Efl_Model_Iterator_Slice* iterator EINA_UNUSED)
{
  return EINA_FALSE;
}

static void
efl_model_iterator_slice_setup(Efl_Model_Iterator_Slice* iterator,
   Efl_Model* parent, Efl_Model_Composite_Boolean_Data* parent_pd, Eina_Promise_Owner* promise)
{
   iterator->vtable.version = EINA_ITERATOR_VERSION;
   iterator->vtable.next = FUNC_ITERATOR_NEXT(efl_model_iterator_slice_next);
   iterator->vtable.get_container = FUNC_ITERATOR_GET_CONTAINER(efl_model_iterator_slice_container);
   iterator->vtable.free = FUNC_ITERATOR_FREE(efl_model_iterator_slice_free);
   iterator->vtable.lock = FUNC_ITERATOR_LOCK(efl_model_iterator_slice_lock);
   EINA_MAGIC_SET(&iterator->vtable, EINA_MAGIC_ITERATOR);
   iterator->parent = eo_ref(parent);
   iterator->parent_pd = parent_pd;
   iterator->promise = promise;
}

static void
_efl_model_composite_boolean_slice_then_cb(void* data, void* value)
{
   Efl_Model_Iterator_Slice* slice_iterator = data;
   Eina_Iterator* iterator = value;

   slice_iterator->composite_iterator = eina_iterator_clone(iterator);
   if(slice_iiterator->composite_iterator)
     eina_promise_owner_value_set(slice_iterator->promise, slice_iterator, (Eina_Promise_Free_Cb)&eina_iterator_free);
   else
     {
        eina_promise_owner_error_set(slice_iterator->promise, EFL_MODEL_ERROR_NOT_SUPPORTED);
        free(slice_iterator);
     }
}

static void
_efl_model_composite_boolean_slice_error_cb(void* data, Eina_Error error)
{
   Efl_Model_Iterator_Slice* slice_iterator = data;
   eina_promise_owner_error_set(slice_iterator->promise, error);
   free(slice_iterator);
}

static void efl_model_hash_value_free(void* p)
{
   Efl_Model_Hash_Value* value = p;
   if(value)
     {
        free(value->buffer);
        free(value);
     }
}

/**************** efl_mmodel_composite_boolean **************/

static const Eina_Array *
_efl_model_composite_boolean_efl_model_properties_get(Eo *obj EINA_UNUSED, Efl_Model_Composite_Boolean_Data *pd)
{
  if(pd->composite_model)
    return efl_model_properties_get(pd->composite_model);
  else if(pd->empty_properties)
    return pd->empty_properties;
  else
    return pd->empty_properties = eina_array_new(1);
    
}

static void
_efl_model_composite_boolean_property_add(Eo *obj EINA_UNUSED, Efl_Model_Composite_Boolean_Data *pd,
   const char *name, Eina_Bool initial_value)
{
   if(!pd->values)
     pd->values = eina_hash_string_small_new(&efl_model_hash_value_free);
   Efl_Model_Hash_Value* value = calloc(sizeof(Efl_Model_Hash_Value), 1);
   value->default_value = initial_value;
   eina_hash_add(pd->values, name, value);
}

static Eina_Promise *
_efl_model_composite_boolean_efl_model_property_get(Eo *obj EINA_UNUSED, Efl_Model_Composite_Boolean_Data *pd EINA_UNUSED, const char *property EINA_UNUSED)
{
   if(pd->composite_model)
     return efl_model_property_get(pd->composite_model, property);
   else
     {
        Eina_Promise_Owner* promise = eina_promise_add();
        eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
        return eina_promise_owner_promise_get(promise);
     }
}

static void
_efl_model_composite_boolean_efl_model_property_set(Eo *obj EINA_UNUSED, Efl_Model_Composite_Boolean_Data *pd EINA_UNUSED, const char *property EINA_UNUSED, const Eina_Value *value EINA_UNUSED, Eina_Promise_Owner *promise EINA_UNUSED)
{
  
}

static Eina_Promise*
_efl_model_composite_boolean_efl_model_children_slice_get(Eo *obj, Efl_Model_Composite_Boolean_Data *pd, unsigned int start, unsigned int count)
{
   Eina_Promise_Owner* promise = eina_promise_add();
   if(pd->composite_model)
     {
        Eina_Promise* composite_promise = efl_model_children_slice_get(pd->composite_model, start, count);
        Efl_Model_Iterator_Slice* iterator = calloc(sizeof(*iterator), 1);
        efl_model_iterator_slice_setup(iterator, obj, pd, promise);
        eina_promise_then(composite_promise, &_efl_model_composite_boolean_slice_then_cb,
                          &_efl_model_composite_boolean_slice_error_cb, iterator);
     }
   else
     {
        eina_promise_owner_error_set(promise, EFL_MODEL_ERROR_NOT_SUPPORTED);
     }
  return eina_promise_owner_promise_get(promise);
}

static Eina_Promise*
_efl_model_composite_boolean_efl_model_children_count_get(Eo *obj EINA_UNUSED, Efl_Model_Composite_Boolean_Data *pd)
{
  if(pd->composite_model)
    return efl_model_children_count_get(pd->composite_model);
  else
    {
      Eina_Promise_Owner* promise = eina_promise_add();
      unsigned int* count = malloc(sizeof(unsigned int));
      *count = eina_hash_population(pd->values);
      eina_promise_owner_value_set(promise, count, free);
      return eina_promise_owner_promise_get(promise);
    }
}

static Eo_Base*
_efl_model_composite_boolean_efl_model_child_add(Eo *obj EINA_UNUSED, Efl_Model_Composite_Boolean_Data *pd)
{
  if(pd->composite_model)
    return efl_model_child_add(pd->composite_model);
  else
    {
       eina_error_set(EFL_MODEL_ERROR_NOT_SUPPORTED);
       return NULL;
    }
}

static void
_efl_model_composite_boolean_efl_model_child_del(Eo *obj EINA_UNUSED, Efl_Model_Composite_Boolean_Data *pd, Eo_Base *child)
{
  if(pd->composite_model)
    return efl_model_child_del(pd->composite_model, child);
  else
    {
       eina_error_set(EFL_MODEL_ERROR_NOT_SUPPORTED);
    }
}
                                                                       
#include "interfaces/efl_model_composite_boolean.eo.c"
#include "interfaces/efl_model_composite_boolean_children.eo.c"

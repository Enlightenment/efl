#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eina.h"
#include "Efl.h"
#include <Ecore.h>

#include "efl_model_composite_boolean_children.eo.h"

typedef struct _Efl_Model_Hash_Value
{
   unsigned char *buffer;
   unsigned int   bits_count;
   Eina_Bool      default_value : 1;
} Efl_Model_Hash_Value;

typedef struct _Efl_Model_Composite_Boolean_Data
{
   Efl_Model  *composite_model;
   Eina_Array *bool_children_cache;
   Eina_Array *empty_properties;
   Eina_Hash  *values; // [property_name, Efl_Model_Hash_Value*]
} Efl_Model_Composite_Boolean_Data;

typedef struct _Efl_Model_Composite_Boolean_Children_Data
{
   Efl_Model_Composite_Boolean_Data *parent_pd;
   Efl_Model                        *composite_child;
   Eina_Array                       *properties_names;
   unsigned int                      index;
} Efl_Model_Composite_Boolean_Children_Data;

typedef struct _Efl_Model_Accessor_Slice
{
   Eina_Accessor                     vtable;
   Eina_Accessor                    *real_accessor;
   Efl_Model                        *parent;
   Efl_Model_Composite_Boolean_Data *parent_pd;
   Efl_Promise                      *promise;
   unsigned int                      index;
} Efl_Model_Accessor_Slice;

static Eina_Value *
_value_clone(const Eina_Value *value)
{
   Eina_Value *copy = eina_value_new(eina_value_type_get(value));
   eina_value_copy(value, copy);
   return copy;
}

static void
_future_forward_cb(void *data, Efl_Event const *event)
{
   efl_promise_value_set(
     data, _value_clone(((Efl_Future_Event_Success*)event->info)->value), (Eina_Free_Cb)eina_value_free);
}

static void
_future_error_forward_cb(void *data, Efl_Event const *event)
{
   Eina_Error err = ((Efl_Future_Event_Failure*)event->info)->error;
   efl_promise_failed_set(data, err);
}

static Eina_Bool
_bit_get(const unsigned char *bitstream, unsigned int idx)
{
   return (bitstream[idx / 8] >> (idx % 8)) & 1u;
}

static void
_bit_set(unsigned char *bitstream, unsigned int idx, Eina_Bool v)
{
   if (v)
     bitstream[idx / 8] |= 1u << (idx % 8);
   else
     bitstream[idx / 8] &= ~(1u << (idx % 8));
}

/**************** efl_mmodel_composite_boolean_children **************/

static void
_properties_names_array_free(Eina_Array *properties_names)
{
   Eina_Array_Iterator it;
   void *name;
   unsigned int i;

   EINA_ARRAY_ITER_NEXT(properties_names, i, name, it)
     free(name);

   eina_array_free(properties_names);
}


static void
_efl_model_composite_boolean_children_efl_object_destructor(Eo *obj,
  Efl_Model_Composite_Boolean_Children_Data *pd)
{
   if (pd->properties_names)
     _properties_names_array_free(pd->properties_names);

   if (pd->composite_child)
     efl_unref(pd->composite_child);

   efl_destructor(efl_super(obj, EFL_MODEL_COMPOSITE_BOOLEAN_CHILDREN_CLASS));
}

static Eina_Bool
_properties_names_push(const Eina_Hash *hash EINA_UNUSED,
  const void *key, void *data EINA_UNUSED, void *fdata)
{
   eina_array_push(fdata, key);
   return EINA_TRUE;
}

static const Eina_Array*
_efl_model_composite_boolean_children_efl_model_properties_get(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Boolean_Children_Data *pd)
{
   Eina_Array const *composite_properties;
   unsigned int composite_count, self_count, i;

   if (pd->properties_names)
     {
        _properties_names_array_free(pd->properties_names);
        pd->properties_names = NULL;
     }

   composite_properties = efl_model_properties_get(pd->composite_child);
   if (!composite_properties)
        return pd->properties_names = eina_array_new(1);

   composite_count = eina_array_count_get(composite_properties);
   self_count = eina_hash_population(pd->parent_pd->values);

   pd->properties_names = eina_array_new(composite_count + self_count);
   for (i = 0; i != composite_count; ++i)
     {
        const char *name = strdup(eina_array_data_get(composite_properties, i));
        eina_array_push(pd->properties_names, name);
     }

   eina_hash_foreach(pd->parent_pd->values, _properties_names_push, pd->properties_names);

   return pd->properties_names;
}

static Efl_Future *
_efl_model_composite_boolean_children_efl_model_property_get(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Boolean_Children_Data *pd, const char *property)
{
   Efl_Model_Hash_Value *hv = eina_hash_find(pd->parent_pd->values, property);
   if (hv)
     {
        Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
        Efl_Future *rfuture = efl_promise_future_get(promise);

        Eina_Value *eina_value = eina_value_new(EINA_VALUE_TYPE_UCHAR);
        if (hv->bits_count <= pd->index)
          {
            unsigned char f = hv->default_value;
            eina_value_set(eina_value, f);
          }
        else
          {
             eina_value_set(eina_value, _bit_get(hv->buffer, pd->index));
          }
        efl_promise_value_set(promise, eina_value, (Eina_Free_Cb)eina_value_free);
        return rfuture;
     }
   else if (pd->composite_child)
     return efl_model_property_get(pd->composite_child, property);
   else
     {
        Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
        Efl_Future *rfuture = efl_promise_future_get(promise);

        efl_promise_failed_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
        return rfuture;
     }
}

static Efl_Future *
_efl_model_composite_boolean_children_efl_model_property_set(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Boolean_Children_Data *pd, const char *property, const Eina_Value *value)
{
   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   Efl_Future *rfuture = efl_promise_future_get(promise);

   Efl_Model_Hash_Value *hv = eina_hash_find(pd->parent_pd->values, property);
   if (hv)
     {
        Eina_Bool flag = EINA_FALSE;

        if (eina_value_type_get(value) == EINA_VALUE_TYPE_UCHAR)
          {
             efl_promise_failed_set(promise, EFL_MODEL_ERROR_INCORRECT_VALUE);
             return rfuture;
          }
        if (!eina_value_get(value, &flag))
          {
             efl_promise_failed_set(promise, EFL_MODEL_ERROR_UNKNOWN);
             return rfuture;
          }

        if (pd->index >= hv->bits_count)
          {
             unsigned int bytes = (pd->index / 8) + 1;
             unsigned int buff_size = (hv->bits_count / 8) + (hv->bits_count % 8 ? 1 : 0);
             unsigned char *buff = realloc(hv->buffer, bytes);
             if (!buff)
               {
                  efl_promise_failed_set(promise, EFL_MODEL_ERROR_UNKNOWN);
                  return rfuture;
               }
             memset(
               buff + buff_size,
               hv->default_value ? -1 : 0,
               bytes - buff_size);

             hv->buffer = buff;
             hv->bits_count = bytes * 8;
          }

        _bit_set(hv->buffer, pd->index, flag);

        efl_promise_value_set(
          promise, _value_clone(value), (Eina_Free_Cb)eina_value_free);
     }
   else if (pd->composite_child)
     {
        Efl_Future *f_forward = efl_model_property_set(pd->composite_child, property, value);
        efl_future_then(f_forward, _future_forward_cb, _future_error_forward_cb, NULL, promise);
     }
   else
     {
        efl_promise_failed_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
     }

   return rfuture;
}

static Efl_Future*
_efl_model_composite_boolean_children_efl_model_children_slice_get(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Boolean_Children_Data *pd, unsigned int start, unsigned int count)
{
   return efl_model_children_slice_get(pd->composite_child, start, count);
}

static Efl_Future*
_efl_model_composite_boolean_children_efl_model_children_count_get(Eo *obj  EINA_UNUSED,
  Efl_Model_Composite_Boolean_Children_Data *pd)
{
   return efl_model_children_count_get(pd->composite_child);
}

static Efl_Object*
_efl_model_composite_boolean_children_efl_model_child_add(Eo *obj  EINA_UNUSED,
  Efl_Model_Composite_Boolean_Children_Data *pd)
{
   return efl_model_child_add(pd->composite_child);
}

static void
_efl_model_composite_boolean_children_efl_model_child_del(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Boolean_Children_Data *pd, Efl_Object *child)
{
   efl_model_child_del(pd->composite_child, child);
}

/**************** efl_model_accessor_slice **************/

static void
_bool_children_cache_array_free(Eina_Array *children_cache)
{
   int i, count = eina_array_count_get(children_cache);
   for (i = 0; i < count; ++i)
     efl_unref(eina_array_data_get(children_cache, i));
   eina_array_free(children_cache);
}

static Eina_Bool
efl_model_acessor_slice_get_at(Efl_Model_Accessor_Slice *acc, unsigned int idx, void **data)
{
   Efl_Model *child_bool;
   Eina_Array *children_cache = acc->parent_pd->bool_children_cache;

   /* NOTE: Efl.Model.Composite.Boolean must alloc the cache with the correct size
      and NULL initialized. */
   if (idx >= eina_array_count(children_cache))
     {
        EINA_LOG_WARN("Index bigger than size");
        return EINA_FALSE;
     }

   child_bool = eina_array_data_get(children_cache, idx);

   if (!child_bool)
     {
        Efl_Model *child = NULL;

        if (!eina_accessor_data_get(acc->real_accessor, idx, (void *)&child))
          return EINA_FALSE;

        if (child)
          {
             Efl_Model_Composite_Boolean_Children_Data *pd;
             child_bool = efl_add(EFL_MODEL_COMPOSITE_BOOLEAN_CHILDREN_CLASS, NULL);
             pd = efl_data_scope_get(child_bool, EFL_MODEL_COMPOSITE_BOOLEAN_CHILDREN_CLASS);

             pd->parent_pd = acc->parent_pd;
             pd->composite_child = efl_ref(child);
             pd->index = acc->index++;

             eina_array_data_set(children_cache, idx, child_bool);
          }
     }

   if (data) *data = child_bool;
   return EINA_TRUE;
}

static void *
efl_model_acessor_slice_get_container(Efl_Model_Accessor_Slice *acc)
{
   return eina_accessor_container_get(acc->real_accessor);
}

static void
efl_model_acessor_slice_free(Efl_Model_Accessor_Slice *acc)
{
   if (acc->real_accessor)
     {
        eina_accessor_free(acc->real_accessor);
        acc->real_accessor = NULL;
     }

   if (acc->parent)
     {
        efl_unref(acc->parent);
        acc->parent = NULL;
     }

   acc->parent_pd = NULL;
   acc->promise = NULL;

   free(acc);
}

static Eina_Bool
efl_model_acessor_slice_lock(Efl_Model_Accessor_Slice *acc)
{
   return eina_accessor_lock(acc->real_accessor);
}

static Eina_Bool
efl_model_acessor_slice_unlock(Efl_Model_Accessor_Slice *acc)
{
   return eina_accessor_unlock(acc->real_accessor);
}

static Efl_Model_Accessor_Slice *
efl_model_acessor_slice_clone(Efl_Model_Accessor_Slice *acc EINA_UNUSED)
{
   return NULL;
}

static void
efl_model_accessor_slice_setup(Efl_Model_Accessor_Slice *acc,
   Efl_Model *parent, Efl_Model_Composite_Boolean_Data *parent_pd, Efl_Promise *promise)
{
   acc->vtable.version = EINA_ACCESSOR_VERSION;
   acc->vtable.get_at = FUNC_ACCESSOR_GET_AT(efl_model_acessor_slice_get_at);
   acc->vtable.get_container = FUNC_ACCESSOR_GET_CONTAINER(efl_model_acessor_slice_get_container);
   acc->vtable.free = FUNC_ACCESSOR_FREE(efl_model_acessor_slice_free);

   acc->vtable.lock = FUNC_ACCESSOR_LOCK(efl_model_acessor_slice_lock);
   acc->vtable.unlock = FUNC_ACCESSOR_LOCK(efl_model_acessor_slice_unlock);

   acc->vtable.clone = FUNC_ACCESSOR_CLONE(efl_model_acessor_slice_clone);

   EINA_MAGIC_SET(&acc->vtable, EINA_MAGIC_ACCESSOR);

   acc->parent = efl_ref(parent);
   acc->parent_pd = parent_pd;
   acc->promise = promise;
}

static void
_efl_model_composite_boolean_slice_then_cb(void *data, Efl_Event const *event)
{
   Efl_Model_Accessor_Slice *slice_acc = data;
   Eina_Accessor *value_itt = (Eina_Accessor *)((Efl_Future_Event_Success*)event->info)->value;
   unsigned int *children_count = NULL;
   Eina_Accessor *children_accessor = NULL;

   if (!eina_accessor_data_get(value_itt, 0, (void**)&children_count) ||
       !children_count ||
       !eina_accessor_data_get(value_itt, 1, (void**)&children_accessor) ||
       !children_accessor)
     {
        EINA_LOG_WARN("missing Efl.Model information");
        efl_promise_failed_set(slice_acc->promise, EFL_MODEL_ERROR_UNKNOWN);
        efl_model_acessor_slice_free(slice_acc);
        return;
     }

   slice_acc->real_accessor = eina_accessor_clone(children_accessor);
   if (slice_acc->real_accessor)
     {
        Eina_Array *children_cache;
        unsigned int i;

        if (slice_acc->parent_pd->bool_children_cache)
          _bool_children_cache_array_free(slice_acc->parent_pd->bool_children_cache);

        children_cache = eina_array_new(*children_count);
        for (i = 0 ; i < *children_count; ++i)
          {
             // NOTE: eina_array_push do not accept NULL
             eina_array_push(children_cache, (void*)0x01);
             eina_array_data_set(children_cache, i, NULL);
          }

        slice_acc->parent_pd->bool_children_cache = children_cache;

        efl_promise_value_set(slice_acc->promise, slice_acc, (Eina_Free_Cb)&eina_accessor_free);
     }
   else
     {
        efl_promise_failed_set(slice_acc->promise, EFL_MODEL_ERROR_NOT_SUPPORTED);
        efl_model_acessor_slice_free(slice_acc);
     }
}

static void
_efl_model_composite_boolean_slice_error_cb(void *data,  Efl_Event const*event)
{
   Efl_Model_Accessor_Slice *slice_acc = data;
   Eina_Error error = ((Efl_Future_Event_Failure*)event->info)->error;
   efl_promise_failed_set(slice_acc->promise, error);
   efl_model_acessor_slice_free(slice_acc);
}

/**************** efl_mmodel_composite_boolean **************/

static void
efl_model_hash_value_free(void *p)
{
   Efl_Model_Hash_Value *value = p;
   if (value)
     {
        free(value->buffer);
        free(value);
     }
}

static void
_composite_model_data_reset(Efl_Model_Composite_Boolean_Data *pd)
{
   if (pd->bool_children_cache)
     {
        _bool_children_cache_array_free(pd->bool_children_cache);
        pd->bool_children_cache = NULL;
     }

   if (pd->composite_model)
     {
        efl_unref(pd->composite_model);
        pd->composite_model = NULL;
     }
}

static void
_efl_model_composite_boolean_efl_object_destructor(Eo *obj, Efl_Model_Composite_Boolean_Data *pd)
{
   if (pd->empty_properties)
     {
        eina_array_free(pd->empty_properties);
        pd->empty_properties = NULL;
     }

   if (pd->values)
     {
        eina_hash_free(pd->values);
        pd->values = NULL;
     }

   _composite_model_data_reset(pd);

   efl_destructor(efl_super(obj, EFL_MODEL_COMPOSITE_BOOLEAN_CLASS));
}

static void
_efl_model_composite_boolean_composite_model_set(Eo *obj EINA_UNUSED,
  Efl_Model_Composite_Boolean_Data *pd, Efl_Model *model)
{
   if (pd->composite_model)
     {
        _composite_model_data_reset(pd);
     }

   pd->composite_model = efl_ref(model);
}

static Efl_Model *
_efl_model_composite_boolean_composite_model_get(Eo *obj EINA_UNUSED, Efl_Model_Composite_Boolean_Data *pd)
{
   return pd->composite_model;
}

static void
_efl_model_composite_boolean_property_add(Eo *obj EINA_UNUSED, Efl_Model_Composite_Boolean_Data *pd,
   const char *name, Eina_Bool initial_value)
{
   if (!pd->values)
     pd->values = eina_hash_string_small_new(&efl_model_hash_value_free);
   Efl_Model_Hash_Value *value = calloc(1, sizeof(Efl_Model_Hash_Value));
   value->default_value = initial_value;
   eina_hash_add(pd->values, name, value);
}

static const Eina_Array *
_efl_model_composite_boolean_efl_model_properties_get(Eo *obj EINA_UNUSED, Efl_Model_Composite_Boolean_Data *pd)
{
  if (pd->composite_model)
    return efl_model_properties_get(pd->composite_model);
  else if (pd->empty_properties)
    return pd->empty_properties;
  else
    return pd->empty_properties = eina_array_new(1);
}

Efl_Future *
_efl_model_composite_boolean_efl_model_property_get(Eo *obj EINA_UNUSED,
   Efl_Model_Composite_Boolean_Data *pd, const char *property)
{
   if (pd->composite_model)
     return efl_model_property_get(pd->composite_model, property);

   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   Efl_Future *rfuture = efl_promise_future_get(promise);

   efl_promise_failed_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
   return rfuture;
}

Efl_Future *
_efl_model_composite_boolean_efl_model_property_set(Eo *obj EINA_UNUSED, Efl_Model_Composite_Boolean_Data *pd,
   const char *property, const Eina_Value *value)
{

   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   Efl_Future *rfuture = efl_promise_future_get(promise);

   if (pd->composite_model)
     {
        Efl_Future *f_forward = efl_model_property_set(pd->composite_model, property, value);
        efl_future_then(f_forward, _future_forward_cb, _future_error_forward_cb, NULL, promise);
     }
   else
     {
        efl_promise_failed_set(promise, EFL_MODEL_ERROR_NOT_FOUND);
     }

   return rfuture;
}

static Efl_Future *
_efl_model_composite_boolean_efl_model_children_slice_get(Eo *obj, Efl_Model_Composite_Boolean_Data *pd, unsigned int start, unsigned int count)
{
   Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   Efl_Future *rfuture = efl_promise_future_get(promise);

   if (pd->composite_model)
     {
        Efl_Model_Accessor_Slice *accessor;
        Efl_Future *composite_future;
        Efl_Future *futures[2] = {NULL,};

        futures[0] = efl_model_children_count_get(pd->composite_model);
        futures[1] = efl_model_children_slice_get(pd->composite_model, start, count);

        composite_future = efl_future_all(futures[0], futures[1]);

        accessor = calloc(1, sizeof(Efl_Model_Accessor_Slice));
        efl_model_accessor_slice_setup(accessor, obj, pd, promise);

        efl_future_then(composite_future, &_efl_model_composite_boolean_slice_then_cb,
                          &_efl_model_composite_boolean_slice_error_cb, NULL, accessor);
     }
   else
     {
        efl_promise_failed_set(promise, EFL_MODEL_ERROR_NOT_SUPPORTED);
     }
  return rfuture;
}

static Efl_Future *
_efl_model_composite_boolean_efl_model_children_count_get(Eo *obj EINA_UNUSED, Efl_Model_Composite_Boolean_Data *pd)
{
  if (pd->composite_model)
    return efl_model_children_count_get(pd->composite_model);
  else
    {
      Efl_Promise *promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
      Efl_Future *rfuture = efl_promise_future_get(promise);

      unsigned int *count = malloc(sizeof(unsigned int));
      *count = 0;
      efl_promise_value_set(promise, count, free);
      return rfuture;
    }
}

static Efl_Object*
_efl_model_composite_boolean_efl_model_child_add(Eo *obj EINA_UNUSED, Efl_Model_Composite_Boolean_Data *pd)
{
  if (pd->composite_model)
    return efl_model_child_add(pd->composite_model);
  else
    {
       eina_error_set(EFL_MODEL_ERROR_NOT_SUPPORTED);
       return NULL;
    }
}

static void
_efl_model_composite_boolean_efl_model_child_del(Eo *obj EINA_UNUSED, Efl_Model_Composite_Boolean_Data *pd, Efl_Object *child)
{
  if (pd->composite_model)
    return efl_model_child_del(pd->composite_model, child);
  else
    {
       eina_error_set(EFL_MODEL_ERROR_NOT_SUPPORTED);
    }
}

#include "efl_model_composite_boolean.eo.c"
#include "efl_model_composite_boolean_children.eo.c"

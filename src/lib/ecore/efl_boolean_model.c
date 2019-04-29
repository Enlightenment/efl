#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl_Core.h>

#include "efl_composite_model_private.h"

typedef struct _Efl_Boolean_Model_Data Efl_Boolean_Model_Data;
typedef struct _Efl_Boolean_Model_Value Efl_Boolean_Model_Value;

struct _Efl_Boolean_Model_Data
{
   Efl_Boolean_Model_Data *parent;
   Eina_Hash *values;
};

struct _Efl_Boolean_Model_Value
{
   Eina_Stringshare *property;

   // This is not the best for supporting sparse bitfield with random insertion
   // but will do for now (Would be best to have a tree of fixed size array
   // or something along that line).
   unsigned char *buffer;
   unsigned int   buffer_count;

   Eina_Bool      default_value;
};

static Eina_Iterator *
_efl_boolean_model_efl_model_properties_get(const Eo *obj,
                                            Efl_Boolean_Model_Data *pd)
{
   Eina_Iterator *properties = NULL;

   if (pd->parent)
     properties = eina_hash_iterator_key_new(pd->parent->values);
   EFL_COMPOSITE_MODEL_PROPERTIES_SUPER(props,
                                        obj, EFL_BOOLEAN_MODEL_CLASS,
                                        properties);
   return props;
}

static Eina_Value *
_efl_boolean_model_efl_model_property_get(const Eo *obj,
                                          Efl_Boolean_Model_Data *pd,
                                          const char *property)
{
   Efl_Boolean_Model_Value *v;
   Eina_Stringshare *s;
   Eina_Bool flag;
   unsigned int index;

   if (property == NULL) return NULL;

   // If we do not have a parent set that his a BOOLEAN, then we should just forward up the call
   if (!pd->parent)
     return efl_model_property_get(efl_super(obj, EFL_BOOLEAN_MODEL_CLASS), property);

   // Check if this is requesting a defined boolean property
   // Property are defined and their value are stored on the parent BOOLEAN
   s = eina_stringshare_add(property);
   v = eina_hash_find(pd->parent->values, s);
   eina_stringshare_del(s);

   if (!v) // Not a property handle by this object, forward
     return efl_model_property_get(efl_super(obj, EFL_BOOLEAN_MODEL_CLASS), property);

   index = efl_composite_model_index_get(obj);

   // As an optimization we do optimistically allocate the boolean array
   // Better would be to have a sparse boolean array
   if ((index >> 3) >= v->buffer_count)
     flag = v->default_value;
   else
     flag = v->buffer[index >> 3] & (((unsigned char)1) << (index & 0x7));

   return eina_value_bool_new(!!flag);
}

static Eina_Future *
_efl_boolean_model_efl_model_property_set(Eo *obj,
                                          Efl_Boolean_Model_Data *pd,
                                          const char *property, Eina_Value *value)
{
   Efl_Boolean_Model_Value *v;
   Eina_Stringshare *s;
   Eina_Bool flag;
   unsigned int index;

   if (!property)
     return efl_loop_future_rejected(obj,
                                 EFL_MODEL_ERROR_UNKNOWN);

   // If we do not have a parent set that his a BOOLEAN, then we should just forward up the call
   if (!pd->parent)
     return efl_model_property_set(efl_super(obj, EFL_BOOLEAN_MODEL_CLASS),
                                   property, value);

   // Check if this is requesting a defined boolean property
   // Property are defined and their value are stored on the parent BOOLEAN
   s = eina_stringshare_add(property);
   v = eina_hash_find(pd->parent->values, s);
   eina_stringshare_del(s);

   if (!v)
     return efl_model_property_set(efl_super(obj, EFL_BOOLEAN_MODEL_CLASS),
                                   property, value);

   if (!eina_value_bool_convert(value, &flag))
     return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_UNKNOWN);

   index = efl_composite_model_index_get(obj);

   // We are optimistically allocating the boolean buffer now.
   // Aligning it on 64bits
   if (v->buffer_count < (((index) >> 3) | 0x7) + 1)
     {
        unsigned int rcount = (((index | 0xF) >> 3) | 0x7) + 1;
        unsigned char *tmp;

        tmp = realloc(v->buffer, rcount);
        if (!tmp) return efl_loop_future_rejected(obj, ENOMEM);
        v->buffer = tmp;
        memset(v->buffer + v->buffer_count, 0, rcount - v->buffer_count);
        v->buffer_count = rcount;
     }

   // It is assumed that during slice get the buffer is properly sized
   if (flag)
     v->buffer[index >> 3] |= ((unsigned char)1) << (index & 0x7);
   else
     v->buffer[index >> 3] &= ~(((unsigned char)1) << (index & 0x7));

   // Calling "properties,changed" event
   efl_model_properties_changed(obj, property);

   // Return fulfilled future
   return efl_loop_future_resolved(obj, eina_value_bool_init(!!flag));
}

static void
_boolean_value_free(void *data)
{
   Efl_Boolean_Model_Value *value = data;

   eina_stringshare_del(value->property);
   value->property = NULL;

   free(value->buffer);
   value->buffer = NULL;
   value->buffer_count = 0;

   free(value);
}

static Eo *
_efl_boolean_model_efl_object_constructor(Eo *obj, Efl_Boolean_Model_Data *pd)
{
   Eo *parent;
   obj = efl_constructor(efl_super(obj, EFL_BOOLEAN_MODEL_CLASS));

   if (!obj) return NULL;

   pd->values = eina_hash_stringshared_new(_boolean_value_free);
   // Only add a reference to the parent if it is actually a BOOLEAN_MODEL_CLASS
   // The root typically doesn't have any boolean property, only its child do
   parent = efl_parent_get(obj);
   if (efl_isa(parent, EFL_BOOLEAN_MODEL_CLASS))
     pd->parent = efl_data_scope_get(parent, EFL_BOOLEAN_MODEL_CLASS);

   return obj;
}

static void
_efl_boolean_model_efl_object_destructor(Eo *obj, Efl_Boolean_Model_Data *pd)
{
   eina_hash_free(pd->values);

   efl_destructor(efl_super(obj, EFL_BOOLEAN_MODEL_CLASS));
}

static void
_efl_boolean_model_boolean_add(Eo *obj EINA_UNUSED,
                               Efl_Boolean_Model_Data *pd,
                               const char *name, Eina_Bool default_value)
{
   Efl_Boolean_Model_Value *value;

   if (!name) return ;

   value = calloc(1, sizeof (Efl_Boolean_Model_Value));
   if (!value) return ;

   value->property = eina_stringshare_add(name);
   value->default_value = default_value;

   eina_hash_direct_add(pd->values, value->property, value);
}

static void
_efl_boolean_model_boolean_del(Eo *obj EINA_UNUSED,
                               Efl_Boolean_Model_Data *pd,
                               const char *name)
{
   Eina_Stringshare *s;

   s = eina_stringshare_add(name);
   eina_hash_del(pd->values, s, NULL);
   eina_stringshare_del(s);
}

typedef struct _Eina_Iterator_Boolean Eina_Iterator_Boolean;

struct _Eina_Iterator_Boolean
{
   Eina_Iterator iterator;

   Eo *obj;
   Efl_Boolean_Model_Data *pd;
   Efl_Boolean_Model_Value *v;

   uint64_t index;
   uint64_t total;

   Eina_Bool request;
};

static inline Eina_Bool
_lookup_next_chunk(uint64_t *index, uint64_t total,
                   Efl_Boolean_Model_Value *v, unsigned char pattern)
{
   uint64_t upidx = *index >> 3;

   while (upidx < v->buffer_count &&
          v->buffer[upidx] == pattern)
     upidx++;

   *index = upidx << 3;
   if (upidx == v->buffer_count &&
       *index >= total) return EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
efl_boolean_model_iterator_next(Eina_Iterator_Boolean *it, void **data)
{
   uint64_t upidx;

   *data = &it->index;
   it->index++;

 retry:
   if (it->index >= it->total) return EINA_FALSE;
   if ((it->index >> 3) >= it->v->buffer_count)
     {
        if (it->v->default_value != it->request)
          return EINA_FALSE;
        return EINA_TRUE;
     }

   upidx = it->index >> 3;
   while ((it->index >> 3) == upidx)
     {
        Eina_Bool flag = it->v->buffer[it->index >> 3] &
          (((unsigned char)1) << (it->index & 0x7));

        if (it->request == !!flag)
          break;

        it->index++;
     }

   if ((it->index >> 3) != upidx)
     {
        if (!_lookup_next_chunk(&it->index, it->total, it->v, it->request ? 0x00 : 0xFF))
          return EINA_FALSE;
        goto retry;
     }

   return EINA_TRUE;
}

static Eo *
efl_boolean_model_iterator_get_container(Eina_Iterator_Boolean *it)
{
   return it->obj;
}

static void
efl_boolean_model_iterator_free(Eina_Iterator_Boolean *it)
{
   efl_unref(it->obj);
   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_NONE);
   free(it);
}

static Eina_Iterator *
_efl_boolean_model_boolean_iterator_get(Eo *obj, Efl_Boolean_Model_Data *pd, const char *name, Eina_Bool request)
{
   Eina_Iterator_Boolean *itb;
   Efl_Boolean_Model_Value *v;
   Eina_Stringshare *s;

   s = eina_stringshare_add(name);
   v = eina_hash_find(pd->values, s);
   eina_stringshare_del(s);
   if (!v) return NULL;

   itb = calloc(1, sizeof (Eina_Iterator_Boolean));
   if (!itb) return NULL;

   itb->obj = efl_ref(obj);
   itb->pd = pd;
   itb->v = v;
   itb->index = 0;
   itb->total = efl_model_children_count_get(obj);
   itb->request = !!request;

   itb->iterator.version = EINA_ITERATOR_VERSION;
   itb->iterator.next = FUNC_ITERATOR_NEXT(efl_boolean_model_iterator_next);
   itb->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(efl_boolean_model_iterator_get_container);
   itb->iterator.free = FUNC_ITERATOR_FREE(efl_boolean_model_iterator_free);

   EINA_MAGIC_SET(&itb->iterator, EINA_MAGIC_ITERATOR);
   return &itb->iterator;
}


#include "efl_boolean_model.eo.c"

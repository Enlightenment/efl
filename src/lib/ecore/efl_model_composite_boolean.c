#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl_Core.h>

#include "efl_model_composite_private.h"

typedef struct _Efl_Model_Composite_Boolean_Data Efl_Model_Composite_Boolean_Data;
typedef struct _Efl_Model_Composite_Boolean_Value Efl_Model_Composite_Boolean_Value;

struct _Efl_Model_Composite_Boolean_Data
{
   Efl_Model_Composite_Boolean_Data *parent;
   Eina_Hash *values;
};

struct _Efl_Model_Composite_Boolean_Value
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
_efl_model_composite_boolean_efl_model_properties_get(const Eo *obj,
                                                      Efl_Model_Composite_Boolean_Data *pd)
{
   EFL_MODEL_COMPOSITE_PROPERTIES_SUPER(props,
                                        obj, EFL_MODEL_COMPOSITE_BOOLEAN_CLASS,
                                        eina_hash_iterator_key_new(pd->parent->values));
   return props;
}

static Eina_Value *
_efl_model_composite_boolean_efl_model_property_get(const Eo *obj,
                                                    Efl_Model_Composite_Boolean_Data *pd,
                                                    const char *property)
{
   Efl_Model_Composite_Boolean_Value *v;
   Eina_Stringshare *s;
   Eina_Bool flag;
   unsigned int index;

   if (property == NULL) return NULL;

   // If we do not have a parent set that his a COMPOSITE_BOOLEAN, then we should just forward up the call
   if (!pd->parent)
     return efl_model_property_get(efl_super(obj, EFL_MODEL_COMPOSITE_BOOLEAN_CLASS), property);

   // Check if this is requesting a defined boolean property
   // Property are defined and their value are stored on the parent COMPOSITE_BOOLEAN
   s = eina_stringshare_add(property);
   v = eina_hash_find(pd->parent->values, s);
   eina_stringshare_del(s);

   if (!v) // Not a property handle by this object, forward
     return efl_model_property_get(efl_super(obj, EFL_MODEL_COMPOSITE_BOOLEAN_CLASS), property);

   index = efl_model_composite_index_get(obj);

   // As an optimization we do optimistically allocate the boolean array
   // Better would be to have a sparse boolean array
   if ((index >> 3) >= v->buffer_count)
     flag = v->default_value;
   else
     flag = v->buffer[index >> 3] & (((unsigned char)1) << (index & 0x7));

   return eina_value_bool_new(!!flag);
}

static Eina_Future *
_efl_model_composite_boolean_efl_model_property_set(Eo *obj,
                                                    Efl_Model_Composite_Boolean_Data *pd,
                                                    const char *property, Eina_Value *value)
{
   Efl_Model_Composite_Boolean_Value *v;
   Eina_Stringshare *s;
   Eina_Bool flag;
   unsigned int index;

   if (!property)
     return efl_loop_future_rejected(obj,
                                 EFL_MODEL_ERROR_UNKNOWN);

   // If we do not have a parent set that his a COMPOSITE_BOOLEAN, then we should just forward up the call
   if (!pd->parent)
     return efl_model_property_set(efl_super(obj, EFL_MODEL_COMPOSITE_BOOLEAN_CLASS),
                                   property, value);

   // Check if this is requesting a defined boolean property
   // Property are defined and their value are stored on the parent COMPOSITE_BOOLEAN
   s = eina_stringshare_add(property);
   v = eina_hash_find(pd->parent->values, s);
   eina_stringshare_del(s);

   if (!v)
     return efl_model_property_set(efl_super(obj, EFL_MODEL_COMPOSITE_BOOLEAN_CLASS),
                                   property, value);

   if (!eina_value_bool_convert(value, &flag))
     return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_UNKNOWN);

   index = efl_model_composite_index_get(obj);

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
   Efl_Model_Composite_Boolean_Value *value = data;

   eina_stringshare_del(value->property);
   value->property = NULL;

   free(value->buffer);
   value->buffer = NULL;
   value->buffer_count = 0;

   free(value);
}

static Eo *
_efl_model_composite_boolean_efl_object_constructor(Eo *obj, Efl_Model_Composite_Boolean_Data *pd)
{
   Eo *parent;
   obj = efl_constructor(efl_super(obj, EFL_MODEL_COMPOSITE_BOOLEAN_CLASS));

   if (!obj) return NULL;

   pd->values = eina_hash_stringshared_new(_boolean_value_free);
   // Only add a reference to the parent if it is actually a COMPOSITE_BOOLEAN_CLASS
   // The root typically doesn't have any boolean property, only its child do
   parent = efl_parent_get(obj);
   if (efl_isa(parent, EFL_MODEL_COMPOSITE_BOOLEAN_CLASS))
     pd->parent = efl_data_scope_get(parent, EFL_MODEL_COMPOSITE_BOOLEAN_CLASS);

   return obj;
}

static void
_efl_model_composite_boolean_efl_object_destructor(Eo *obj, Efl_Model_Composite_Boolean_Data *pd)
{
   eina_hash_free(pd->values);

   efl_destructor(efl_super(obj, EFL_MODEL_COMPOSITE_BOOLEAN_CLASS));
}

static void
_efl_model_composite_boolean_boolean_add(Eo *obj EINA_UNUSED,
                                         Efl_Model_Composite_Boolean_Data *pd,
                                         const char *name, Eina_Bool default_value)
{
   Efl_Model_Composite_Boolean_Value *value;

   if (!name) return ;

   value = calloc(1, sizeof (Efl_Model_Composite_Boolean_Value));
   if (!value) return ;

   value->property = eina_stringshare_add(name);
   value->default_value = default_value;

   eina_hash_direct_add(pd->values, value->property, value);
}

static void
_efl_model_composite_boolean_boolean_del(Eo *obj EINA_UNUSED,
                                         Efl_Model_Composite_Boolean_Data *pd,
                                         const char *name)
{
   Eina_Stringshare *s;

   s = eina_stringshare_add(name);
   eina_hash_del(pd->values, s, NULL);
   eina_stringshare_del(s);
}

#include "efl_model_composite_boolean.eo.c"

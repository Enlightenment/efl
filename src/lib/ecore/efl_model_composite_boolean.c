#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl_Core.h>

#include "efl_model_composite_boolean_children.eo.h"

typedef struct _Efl_Model_Composite_Boolean_Data Efl_Model_Composite_Boolean_Data;
typedef struct _Efl_Model_Composite_Boolean_Children_Data Efl_Model_Composite_Boolean_Children_Data;
typedef struct _Efl_Model_Composite_Boolean_Value Efl_Model_Composite_Boolean_Value;

struct _Efl_Model_Composite_Boolean_Data
{
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

struct _Efl_Model_Composite_Boolean_Children_Data
{
   Efl_Model_Composite_Boolean_Data *parent;
   unsigned int                      index;
};

/**************** efl_mmodel_composite_boolean_children **************/

static void
_efl_model_composite_boolean_children_index_set(Eo *obj EINA_UNUSED,
                                                Efl_Model_Composite_Boolean_Children_Data *pd,
                                                unsigned int index)
{
   if (pd->parent) // This is set during finalize
     return ;

   pd->index = index;
}

static unsigned int
_efl_model_composite_boolean_children_index_get(const Eo *obj EINA_UNUSED,
                                                Efl_Model_Composite_Boolean_Children_Data *pd)
{
   return pd->index;
}

static Efl_Object *
_efl_model_composite_boolean_children_efl_object_finalize(Eo *obj,
                                                          Efl_Model_Composite_Boolean_Children_Data *pd)
{
   Eo *parent;

   parent = efl_parent_get(obj);
   if (!parent) return NULL;
   pd->parent = efl_data_scope_get(parent, EFL_MODEL_COMPOSITE_BOOLEAN_CLASS);

   return obj;
}

static Eina_Array *
_efl_model_composite_boolean_children_efl_model_properties_get(const Eo *obj,
                                                               Efl_Model_Composite_Boolean_Children_Data *pd)
{
   Eina_Iterator *it;
   Eina_Stringshare *s;
   Eina_Array *r;

   r = efl_model_properties_get(efl_super(obj, EFL_MODEL_COMPOSITE_BOOLEAN_CHILDREN_CLASS));
   if (!r) r = eina_array_new(1);

   it = eina_hash_iterator_key_new(pd->parent->values);
   EINA_ITERATOR_FOREACH(it, s)
     eina_array_push(r, s);
   eina_array_push(r, "child.index");

   return r;
}

static Eina_Value *
_efl_model_composite_boolean_children_efl_model_property_get(const Eo *obj,
                                                             Efl_Model_Composite_Boolean_Children_Data *pd,
                                                             const char *property)
{
   Efl_Model_Composite_Boolean_Value *v;
   Eina_Stringshare *s;
   Eina_Bool flag;

   if (property == NULL) return NULL;
   if (strcmp(property, "child.index") == 0)
     return eina_value_uint_new(pd->index);

   s = eina_stringshare_add(property);
   v = eina_hash_find(pd->parent->values, s);
   eina_stringshare_del(s);

   if (!v)
     return efl_model_property_get(efl_super(obj, EFL_MODEL_COMPOSITE_BOOLEAN_CHILDREN_CLASS),
                                   property);

   // As an optimization we do optimistically allocate the boolean array
   // Better would be to have a sparse boolean array
   if ((pd->index >> 3) >= v->buffer_count)
     flag = v->default_value;
   else
     flag = v->buffer[pd->index >> 3] & (1 << pd->index & 0x7);

   return eina_value_uchar_new(!!flag);
}

static Eina_Future *
_efl_model_composite_boolean_children_efl_model_property_set(Eo *obj,
                                                             Efl_Model_Composite_Boolean_Children_Data *pd,
                                                             const char *property, Eina_Value *value)
{
   Efl_Model_Composite_Boolean_Value *v;
   Eina_Stringshare *s;
   Eina_Value b = EINA_VALUE_EMPTY;
   Eina_Bool flag;

   if (!property)
     return eina_future_rejected(efl_loop_future_scheduler_get(obj),
                                 EFL_MODEL_ERROR_UNKNOWN);
   if (strcmp(property, "child.index") == 0)
     return eina_future_rejected(efl_loop_future_scheduler_get(obj),
                                 EFL_MODEL_ERROR_READ_ONLY);

   s = eina_stringshare_add(property);
   v = eina_hash_find(pd->parent->values, s);
   eina_stringshare_del(s);

   if (!v)
     return efl_model_property_set(efl_super(obj, EFL_MODEL_COMPOSITE_BOOLEAN_CHILDREN_CLASS),
                                   property, value);

   eina_value_setup(&b, EINA_VALUE_TYPE_UCHAR);
   if (!eina_value_convert(value, &b))
     return eina_future_rejected(efl_loop_future_scheduler_get(obj),
                                 EFL_MODEL_ERROR_UNKNOWN);
   if (!eina_value_get(value, &flag))
     return eina_future_rejected(efl_loop_future_scheduler_get(obj),
                                 EFL_MODEL_ERROR_UNKNOWN);

   eina_value_flush(&b);

   // We are optimistically allocating the boolean buffer now.
   // Aligning it on 64bits
   if (v->buffer_count < (((pd->index) >> 3) | 0x7) + 1)
     {
        unsigned int rcount = (((pd->index | 0xF) >> 3) | 0x7) + 1;
        unsigned char *tmp;

        tmp = realloc(v->buffer, rcount);
        if (!tmp) return eina_future_rejected(efl_loop_future_scheduler_get(obj), ENOMEM);
        v->buffer = tmp;
        memset(v->buffer + v->buffer_count, 0, rcount - v->buffer_count);
        v->buffer_count = rcount;
     }

   // It is assumed that during slice get the buffer is properly sized
   if (flag)
     v->buffer[pd->index >> 3] |= 1 << (pd->index & 0x7);
   else
     v->buffer[pd->index >> 3] &= ~(1 << (pd->index & 0x7));

   return eina_future_resolved(efl_loop_future_scheduler_get(obj),
                               eina_value_uchar_init(!!flag));
}

/**************** efl_model_composite_boolean **************/
typedef struct _Efl_Model_Slice_Request Efl_Model_Slice_Request;
struct _Efl_Model_Slice_Request
{
   Eo *parent;
   unsigned int start;
};

static Eina_Value
_efl_model_composite_boolean_then(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Model_Slice_Request *req = data;
   unsigned int i, len;
   Eina_Value r = EINA_VALUE_EMPTY;
   Eo *target = NULL;

   if (eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY)
     goto on_error;

   eina_value_array_setup(&r, EINA_VALUE_TYPE_OBJECT, 4);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, target)
     {
        Eo *composite;

        // It would have been nice if I could have just overriden the object
        // function, but this would allow only one composite model
        composite = efl_add(EFL_MODEL_COMPOSITE_BOOLEAN_CHILDREN_CLASS, req->parent,
                            efl_model_composite_boolean_children_index_set(efl_added, req->start + i),
                            efl_ui_view_model_set(efl_added, target));

        eina_value_array_append(&r, composite);
     }

 on_error:
   efl_unref(req->parent);
   free(req);

   return r;
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
}

static Eo *
_efl_model_composite_boolean_efl_object_constructor(Eo *obj, Efl_Model_Composite_Boolean_Data *pd)
{
   obj = efl_constructor(efl_super(obj, EFL_MODEL_COMPOSITE_BOOLEAN_CLASS));

   if (!obj) return NULL;

   pd->values = eina_hash_stringshared_new(_boolean_value_free);

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

static Eina_Future *
_efl_model_composite_boolean_efl_model_children_slice_get(Eo *obj,
                                                          Efl_Model_Composite_Boolean_Data *pd EINA_UNUSED,
                                                          unsigned int start, unsigned int count)
{
   Efl_Model_Slice_Request *req;
   Eina_Future *f;

   f = efl_model_children_slice_get(efl_super(obj, EFL_MODEL_COMPOSITE_BOOLEAN_CLASS),
                                    start, count);

   req = malloc(sizeof (Efl_Model_Slice_Request));
   if (!req) return eina_future_rejected(efl_loop_future_scheduler_get(obj),
                                         ENOMEM);
   req->parent = efl_ref(obj);
   req->start = start;

   return efl_future_Eina_FutureXXX_then
     (obj, eina_future_then(f, _efl_model_composite_boolean_then, req, NULL));
}

#include "efl_model_composite_boolean.eo.c"
#include "efl_model_composite_boolean_children.eo.c"

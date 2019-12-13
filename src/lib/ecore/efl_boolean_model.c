#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl_Core.h>

#include "ecore_internal.h"

typedef struct _Efl_Boolean_Model_Data Efl_Boolean_Model_Data;
typedef struct _Efl_Boolean_Model_Value Efl_Boolean_Model_Value;
typedef struct _Efl_Boolean_Model_Storage_Range Efl_Boolean_Model_Storage_Range;

struct _Efl_Boolean_Model_Data
{
   Efl_Boolean_Model_Data *parent;
   Eina_Hash *values;
};

struct _Efl_Boolean_Model_Storage_Range
{
   EINA_RBTREE;

   unsigned int  offset;
   uint16_t      length; // Maximum length of the buffer will be 256, to avoid math error we rely on 16bits here.

   // We over allocate this buffer to have things fitting in one alloc
   unsigned char buffer[256];
};

struct _Efl_Boolean_Model_Value
{
   Eina_Stringshare *property;

   Eina_Rbtree *buffers_root;
   Efl_Boolean_Model_Storage_Range *last;

   Eina_Bool      default_value;
};

static Eina_Rbtree_Direction
_storage_range_cmp(const Efl_Boolean_Model_Storage_Range *left,
                   const Efl_Boolean_Model_Storage_Range *right,
                   void *data EINA_UNUSED)
{
   // We should not have any overlapping range
   if (left->offset < right->offset)
     return EINA_RBTREE_LEFT;
   return EINA_RBTREE_RIGHT;
}

static int
_storage_range_key(const Efl_Boolean_Model_Storage_Range *node,
                   const unsigned int *key, int length EINA_UNUSED, void *data EINA_UNUSED)
{
   if (node->offset > *key) return 1;
   if (node->offset + node->length < *key) return -1;
   // The key is in the range!
   return 0;
}

static void
_storage_range_free(Eina_Rbtree *node, void *data EINA_UNUSED)
{
   free(node);
}

static Efl_Boolean_Model_Storage_Range *
_storage_lookup(Efl_Boolean_Model_Data *pd,
                const char *property,
                unsigned int index,
                Eina_Bool allocate,
                Eina_Bool value,
                Eina_Bool *found,
                Eina_Bool *default_value)
{
   Efl_Boolean_Model_Storage_Range *lookup;
   Efl_Boolean_Model_Value *v;
   Eina_Stringshare *s;

   // Check if this is requesting a defined boolean property
   // Property are defined and their value are stored on the parent BOOLEAN
   s = eina_stringshare_add(property);
   v = eina_hash_find(pd->parent->values, s);
   eina_stringshare_del(s);

   if (!v) return NULL;
   *found = EINA_TRUE;
   *default_value = !!v->default_value;

   lookup = (void*) eina_rbtree_inline_lookup(v->buffers_root, &index, sizeof (unsigned int),
                                              EINA_RBTREE_CMP_KEY_CB(_storage_range_key), NULL);
   if (lookup) return lookup;
   if (!allocate) return NULL;

   // The value is the same as the default value, why bother allocate
   if (*default_value == value) return NULL;

   // For simplicity we do not create a sparse array, every boolean potentially needed are allocated
   // FIXME: keep it a sparse allocated buffer and only allocate needed buffer on demand
   do
     {
        lookup = calloc(1, sizeof (Efl_Boolean_Model_Storage_Range));
        if (!lookup) return NULL;

        lookup->offset = v->last ? v->last->offset + v->last->length + 1 : 0;
        lookup->length = sizeof (lookup->buffer) * 8; // Number of bits in the buffer
        // Initialize the buffer to the right default value
        memset(&lookup->buffer[0], *default_value, sizeof (lookup->buffer));

        v->buffers_root = eina_rbtree_inline_insert(v->buffers_root, EINA_RBTREE_GET(lookup),
                                                    EINA_RBTREE_CMP_NODE_CB(_storage_range_cmp), NULL);
        v->last = lookup;
     }
   while (v->last->offset + v->last->length < index);

   return lookup;
}

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
   Efl_Boolean_Model_Storage_Range *sr;
   Eina_Bool flag;
   unsigned int index;
   unsigned int offset;
   Eina_Bool found = EINA_FALSE;
   Eina_Bool default_value = EINA_FALSE;

   if (property == NULL) return NULL;

   // If we do not have a parent set that his a BOOLEAN, then we should just forward up the call
   if (!pd->parent)
     return efl_model_property_get(efl_super(obj, EFL_BOOLEAN_MODEL_CLASS), property);

   index = efl_composite_model_index_get(obj);


   sr = _storage_lookup(pd, property, index, EINA_FALSE, EINA_FALSE, &found, &default_value);
   if (!found) // Not a property handle by this object, forward
     return efl_model_property_get(efl_super(obj, EFL_BOOLEAN_MODEL_CLASS), property);

   if (!sr) // Not found in storage, should be the default value
     return eina_value_bool_new(default_value);

   // Calculate the matching offset for the requested index
   offset = index - sr->offset;

   flag = sr->buffer[offset >> 3] & (((unsigned char)1) << (offset & 0x7));

   return eina_value_bool_new(!!flag);
}

static Eina_Future *
_efl_boolean_model_efl_model_property_set(Eo *obj,
                                          Efl_Boolean_Model_Data *pd,
                                          const char *property, Eina_Value *value)
{
   Efl_Boolean_Model_Storage_Range *sr;
   unsigned int index;
   unsigned int offset;
   Eina_Bool flag = EINA_FALSE;
   Eina_Bool found = EINA_FALSE;
   Eina_Bool convert_fail = EINA_FALSE;
   Eina_Bool default_value = EINA_FALSE;

   if (!property) return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_UNKNOWN);

   // If we do not have a parent set that his a BOOLEAN, then we should just forward up the call
   if (!pd->parent)
     return efl_model_property_set(efl_super(obj, EFL_BOOLEAN_MODEL_CLASS),
                                   property, value);

   index = efl_composite_model_index_get(obj);
   if (!eina_value_bool_convert(value, &flag))
     convert_fail = EINA_TRUE;

   sr = _storage_lookup(pd, property, index, EINA_TRUE, flag, &found, &default_value);
   if (!found)
     return efl_model_property_set(efl_super(obj, EFL_BOOLEAN_MODEL_CLASS),
                                   property, value);

   // Convert did fail and we actually should have a valid Boolean to put in the buffer
   if (convert_fail)
     return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_UNKNOWN);

   if (!sr)
     return efl_loop_future_resolved(obj, eina_value_bool_init(default_value));

   // Calculate the matching offset for the requested index
   offset = index - sr->offset;

   // It is assumed that during slice get the buffer is properly sized
   if (flag)
     sr->buffer[offset >> 3] |= ((unsigned char)1) << (offset & 0x7);
   else
     sr->buffer[offset >> 3] &= ~(((unsigned char)1) << (offset & 0x7));

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

   eina_rbtree_delete(value->buffers_root, _storage_range_free, NULL);
   value->last = NULL;

   free(value);
}

static void
_mark_greater(Efl_Boolean_Model_Storage_Range *root, Eina_Array *mark, const unsigned int upper)
{
   if (!root) return ;

   if (root->offset > upper)
     {
        eina_array_push(mark, root);
        _mark_greater((void*) EINA_RBTREE_GET(root)->son[0], mark, upper);
        _mark_greater((void*) EINA_RBTREE_GET(root)->son[1], mark, upper);
     }
   else
     {
        _mark_greater((void*) EINA_RBTREE_GET(root)->son[0], mark, upper);
     }
}

static void
_child_removed(void *data, const Efl_Event *event)
{
   Efl_Boolean_Model_Data *pd = data;
   Efl_Model_Children_Event *ev = event->info;
   Efl_Boolean_Model_Value *v;
   Eina_Iterator *it;
   Eina_Array updated;

   if (!pd->values) return;

   eina_array_step_set(&updated, sizeof (Eina_Array), 8);

   it = eina_hash_iterator_data_new(pd->values);
   EINA_ITERATOR_FOREACH(it, v)
     {
        Efl_Boolean_Model_Storage_Range *lookup;
        Eina_Array_Iterator iterator;
        unsigned int i;

        // Remove the data from the buffer it belong to
        lookup = (void*) eina_rbtree_inline_lookup(v->buffers_root, &ev->index, sizeof (unsigned int),
                                                   EINA_RBTREE_CMP_KEY_CB(_storage_range_key), NULL);
        if (lookup)
          {
             unsigned char lower_mask = (((unsigned char)1) << (ev->index & 0x7)) - 1;
             unsigned char upper_mask = (~(((unsigned char)1) << (ev->index & 0x7))) & (~lower_mask);
             uint16_t offset = (ev->index - lookup->offset) >> 3;
             uint16_t byte_length = lookup->length >> 3;

             // Manually shift all the byte in the buffer
             while (offset < byte_length)
               {
                  lookup->buffer[offset] = ((lookup->buffer[offset] & upper_mask) >> 1) |
                    (lookup->buffer[offset] & lower_mask);
                  if (offset + 1 < byte_length)
                    lookup->buffer[offset] |= lookup->buffer[offset + 1] & 0x1;

                  lower_mask = 0;
                  upper_mask = 0xFE;
                  offset++;
               }

             lookup->length--;
             if (lookup->length == 0)
               {
                  v->buffers_root = eina_rbtree_inline_remove(v->buffers_root, EINA_RBTREE_GET(lookup),
                                                              EINA_RBTREE_CMP_NODE_CB(_storage_range_cmp), NULL);
                  free(lookup);

                  if (lookup == v->last)
                    {
                       if (v->buffers_root)
                         {
                            unsigned int last_index = ev->index - 1;

                            lookup = (void*) eina_rbtree_inline_lookup(v->buffers_root, &last_index,
                                                                       sizeof (unsigned int),
                                                                       EINA_RBTREE_CMP_KEY_CB(_storage_range_key),
                                                                       NULL);
                            v->last = lookup;
                         }
                       else
                         {
                            // Nobody left
                            v->last = NULL;
                         }
                    }
               }
          }

        _mark_greater((void*) v->buffers_root, &updated, ev->index);

        // Correct all the buffer after it
        // There is no need to remove and reinsert them as their relative order will not change.
        EINA_ARRAY_ITER_NEXT(&updated, i, lookup, iterator)
          {
             lookup->offset--;
          }

        eina_array_clean(&updated);
     }
   eina_iterator_free(it);

   eina_array_flush(&updated);
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

   efl_event_callback_add(obj, EFL_MODEL_EVENT_CHILD_REMOVED, _child_removed, pd);

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
   Efl_Boolean_Model_Storage_Range *sr;
   Eina_Iterator *infix;

   unsigned int index;
   unsigned int total;

   Eina_Bool request;
};

static Eina_Bool
_efl_boolean_model_iterator_storage_index_find(Eina_Iterator_Boolean *it)
{
   uint16_t offset;
   uint16_t byte_length;

   offset = it->index - it->sr->offset;
   byte_length = it->sr->length >> 3;

   while (offset < it->sr->length)
     {
        unsigned int upidx;

        upidx = offset >> 3;

        // Quickly dismiss byte that really do not match
        while (upidx < byte_length &&
               it->sr->buffer[upidx] == (it->request ? 0x00 : 0xFF))
          upidx++;

        // Make the indexes jump
        if (upidx != (offset >> 3))
          {
             offset = upidx << 3;
             it->index = it->sr->offset + offset;
          }

        // Search inside the said byte
        while (((offset >> 3) == upidx) &&
               (offset < it->sr->length))
          {
             Eina_Bool flag = it->sr->buffer[offset >> 3] &
               (((unsigned char)1) << (offset & 0x7));

             if (it->request == !!flag)
               return EINA_TRUE;

             it->index++;
             offset++;
          }
     }

   return EINA_FALSE;
}

static Eina_Bool
_efl_boolean_model_iterator_index_find(Eina_Iterator_Boolean *it)
{
   while (it->index < it->total)
     {
        // If we are not walking on an existing storage range, look for a new one
        if (!it->sr)
          {
             if (!eina_iterator_next(it->infix, (void**) &it->sr))
               {
                  // All the rest of the data are not allocated and there value is still default
                  if (it->v->default_value != it->request)
                    return EINA_FALSE;
                  return EINA_TRUE;
               }
          }

        if (_efl_boolean_model_iterator_storage_index_find(it))
          return EINA_TRUE;

        // Nothing more to look at in this buffer
        it->sr = NULL;
     }

   return EINA_FALSE;
}

static Eina_Bool
efl_boolean_model_iterator_next(Eina_Iterator_Boolean *it, void **data)
{
   *data = &it->index;
   it->index++;

   return _efl_boolean_model_iterator_index_find(it);
}

static Eo *
efl_boolean_model_iterator_get_container(Eina_Iterator_Boolean *it)
{
   return it->obj;
}

static void
efl_boolean_model_iterator_free(Eina_Iterator_Boolean *it)
{
   eina_iterator_free(it->infix);
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
   itb->infix = eina_rbtree_iterator_infix(v->buffers_root);
   // Search the first index that do have the valid value
   _efl_boolean_model_iterator_index_find(itb);
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

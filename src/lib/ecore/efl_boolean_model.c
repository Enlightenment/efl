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

   // The range holds the bits from offset up to, but not including, offset + length
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
   if (node->offset + node->length <= *key) return -1;
   // The key is in the range!
   return 0;
}

static void
_storage_range_free(Eina_Rbtree *node, void *data EINA_UNUSED)
{
   free(node);
}

static Efl_Boolean_Model_Storage_Range *
_storage_allocate(Efl_Boolean_Model_Value *v, unsigned int index, Eina_Bool default_value)
{
   Efl_Boolean_Model_Storage_Range *lookup = NULL;

   // For simplicity we do not create a sparse array, every boolean potentially needed are allocated
   // FIXME: keep it a sparse allocated buffer and only allocate needed buffer on demand
   do
     {
        lookup = calloc(1, sizeof (Efl_Boolean_Model_Storage_Range));
        if (!lookup) return NULL;

        lookup->offset = v->last ? v->last->offset + v->last->length : 0;
        lookup->length = sizeof (lookup->buffer) * 8; // Number of bits in the buffer
        // Initialize the buffer to the right default value
        memset(&lookup->buffer[0], default_value ? 0xFF : 0x00, sizeof (lookup->buffer));

        v->buffers_root = eina_rbtree_inline_insert(v->buffers_root, EINA_RBTREE_GET(lookup),
                                                    EINA_RBTREE_CMP_NODE_CB(_storage_range_cmp), NULL);
        v->last = lookup;
     }
   while (v->last->offset + v->last->length <= index);

   return lookup;
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

   return _storage_allocate(v, index, *default_value);
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

   // A row with no index (it left while a slice held it) has no bit to set, and the
   // storage is not sparse: allocating up to there would take all the memory
   sr = _storage_lookup(pd, property, index, index != 0xFFFFFFFF, flag, &found, &default_value);
   if (!found)
     return efl_model_property_set(efl_super(obj, EFL_BOOLEAN_MODEL_CLASS),
                                   property, value);

   // Convert did fail and we actually should have a valid Boolean to put in the buffer
   if (convert_fail)
     return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_UNKNOWN);

   if (index == 0xFFFFFFFF)
     return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

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

static Eina_Bool
_value_bit_get(Efl_Boolean_Model_Value *v, unsigned int index)
{
   Efl_Boolean_Model_Storage_Range *sr;
   unsigned int offset;

   sr = (void*) eina_rbtree_inline_lookup(v->buffers_root, &index, sizeof (unsigned int),
                                          EINA_RBTREE_CMP_KEY_CB(_storage_range_key), NULL);
   if (!sr) return !!v->default_value;

   offset = index - sr->offset;
   return !!(sr->buffer[offset >> 3] & (((unsigned char)1) << (offset & 0x7)));
}

static void
_value_bit_set(Efl_Boolean_Model_Value *v, unsigned int index, Eina_Bool flag)
{
   Efl_Boolean_Model_Storage_Range *sr;
   unsigned int offset;

   sr = (void*) eina_rbtree_inline_lookup(v->buffers_root, &index, sizeof (unsigned int),
                                          EINA_RBTREE_CMP_KEY_CB(_storage_range_key), NULL);
   if (!sr)
     {
        // Not stored means default, nothing to do when that is what is wanted
        if (!!v->default_value == !!flag) return;
        sr = _storage_allocate(v, index, v->default_value);
        if (!sr) return;
     }

   offset = index - sr->offset;
   if (flag) sr->buffer[offset >> 3] |= ((unsigned char)1) << (offset & 0x7);
   else sr->buffer[offset >> 3] &= ~(((unsigned char)1) << (offset & 0x7));
}

static void
_child_removed(void *data, const Efl_Event *event)
{
   Efl_Boolean_Model_Data *pd = data;
   Efl_Model_Children_Event *ev = event->info;
   Efl_Boolean_Model_Value *v;
   Eina_Iterator *it;
   unsigned int total;

   if (!pd->values) return;

   // The child is already gone, so the old last index is where the count is now
   total = efl_model_children_count_get(event->object);

   it = eina_hash_iterator_data_new(pd->values);
   EINA_ITERATOR_FOREACH(it, v)
     {
        unsigned int i;

        // Everything after the removed one is one index closer
        for (i = ev->index; i < total; i++)
          {
             _value_bit_set(v, i, _value_bit_get(v, i + 1));
          }
        // And nobody is at the old end any more
        _value_bit_set(v, total, !!v->default_value);
     }
   eina_iterator_free(it);
}

static void
_child_added(void *data, const Efl_Event *event)
{
   Efl_Boolean_Model_Data *pd = data;
   Efl_Model_Children_Event *ev = event->info;
   Efl_Boolean_Model_Value *v;
   Eina_Iterator *it;
   unsigned int total;

   if (!pd->values) return;

   // The child is already counted, so the last index is the new end
   total = efl_model_children_count_get(event->object);
   if (!total) return;

   it = eina_hash_iterator_data_new(pd->values);
   EINA_ITERATOR_FOREACH(it, v)
     {
        unsigned int i;

        // Everything from the insertion point is now one child further
        for (i = total - 1; i > ev->index; i--)
          _value_bit_set(v, i, _value_bit_get(v, i - 1));
        _value_bit_set(v, ev->index, !!v->default_value);
     }
   eina_iterator_free(it);
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
   efl_event_callback_add(obj, EFL_MODEL_EVENT_CHILD_ADDED, _child_added, pd);

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

   unsigned int index;   /* where the next match is, once found */
   unsigned int found;   /* the match being handed out */
   unsigned int total;

   Eina_Bool request;
   Eina_Bool valid;      /* index names a match rather than the end */
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

        /* A buffer holds whole bytes and is as long as it is, so the search
           through one can walk past the last child and find a bit that was
           never anybody's. Bounded here rather than in there: what the
           search knows about is the buffer, and how many children there are
           is not its business. */
        if (_efl_boolean_model_iterator_storage_index_find(it))
          return (it->index < it->total);

        // Nothing more to look at in this buffer
        it->sr = NULL;
     }

   return EINA_FALSE;
}

/* Hand out the match that has been found, then go looking for the one after
 * it. Which way round that goes matters: the search moves it->index onto
 * what it finds, so a walk that searched and then reported would report
 * where it had got to rather than where it was, and the answer would be
 * every match but the first.
 */
static Eina_Bool
efl_boolean_model_iterator_next(Eina_Iterator_Boolean *it, void **data)
{
   if (!it->valid) return EINA_FALSE;

   it->found = it->index;
   *data = &it->found;

   it->index++;
   it->valid = _efl_boolean_model_iterator_index_find(it);

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
   itb->index = 0;
   itb->total = efl_model_children_count_get(obj);
   itb->request = !!request;

   /* Where the first match is. After the fields it reads, not before them:
      searching against a total of zero found nothing every time, and what
      covered for it was the walk starting one short -- which is exactly why
      a match at the first index was never reported. */
   itb->valid = _efl_boolean_model_iterator_index_find(itb);

   itb->iterator.version = EINA_ITERATOR_VERSION;
   itb->iterator.next = FUNC_ITERATOR_NEXT(efl_boolean_model_iterator_next);
   itb->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(efl_boolean_model_iterator_get_container);
   itb->iterator.free = FUNC_ITERATOR_FREE(efl_boolean_model_iterator_free);

   EINA_MAGIC_SET(&itb->iterator, EINA_MAGIC_ITERATOR);
   return &itb->iterator;
}


#include "efl_boolean_model.eo.c"

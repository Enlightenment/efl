#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Efl.h>
#include <Ecore.h>

#include "ecore_private.h"
#include "efl_composite_model.eo.h"

// The index of a row that left while a slice that holds it was pending: it is nowhere.
#define COMPOSITE_GONE 0xFFFFFFFF

typedef struct _Efl_Composite_Model_Data Efl_Composite_Model_Data;
typedef struct _Efl_Composite_Model_Slice_Request Efl_Composite_Model_Slice_Request;

struct _Efl_Composite_Model_Data
{
   EINA_RBTREE;

   Efl_Composite_Model *self;
   Efl_Model *source;
   Eina_Rbtree *indexed;
   Eina_Inlist *pending; // Efl_Composite_Model_Slice_Request not answered yet

   unsigned int index;

   Eina_Bool need_index : 1;
   Eina_Bool set_index : 1;
   Eina_Bool inserted : 1;
   Eina_Bool detached : 1; // never in the parent's index tree
};

struct _Efl_Composite_Model_Slice_Request
{
   EINA_INLIST;

   const Efl_Class *self;
   Eo *parent;
   Efl_Composite_Model_Data *pd;
   unsigned int start;
   unsigned int count; // rows asked for, dummies included
   unsigned int dummy_need;
   // Where each row asked for is now. The source answers with the rows it had when it
   // was asked, so rows that come or go in front of them before the answer move them.
   unsigned int index[];
};

static inline unsigned int
_request_index(const Efl_Composite_Model_Slice_Request *req, unsigned int i)
{
   // (a source may answer more than was asked for, a count of 0 being all of it to some)
   return (i < req->count) ? req->index[i] : (req->start + i);
}

static Eina_Rbtree_Direction
_children_indexed_cmp(const Efl_Composite_Model_Data *left,
                      const Efl_Composite_Model_Data *right,
                      void *data EINA_UNUSED)
{
   if (left->index < right->index)
     return EINA_RBTREE_LEFT;
   return EINA_RBTREE_RIGHT;
}

static int
_children_indexed_key(const Efl_Composite_Model_Data *node,
                      const int *key, int length EINA_UNUSED, void *data EINA_UNUSED)
{
   if (node->index > (unsigned int) *key) return 1;
   if (node->index < (unsigned int) *key) return -1;
   return 0;
}

// Marks every child whose index is at or above lower.
static void
_mark_from(Efl_Composite_Model_Data *root, Eina_Array *mark, const unsigned int lower)
{
   if (!root) return ;

   if (root->index >= lower)
     {
        eina_array_push(mark, root);
        _mark_from((void*) EINA_RBTREE_GET(root)->son[1], mark, lower);
     }
   _mark_from((void*) EINA_RBTREE_GET(root)->son[0], mark, lower);
}

static void
_efl_composite_model_efl_object_invalidate(Eo *obj, Efl_Composite_Model_Data *pd)
{
   if (pd->inserted)
     {
        Eo *parent;

        parent = efl_parent_get(obj);
        if (efl_isa(parent, EFL_COMPOSITE_MODEL_CLASS))
          {
             Efl_Composite_Model_Data *ppd;

             ppd = efl_data_scope_get(parent, EFL_COMPOSITE_MODEL_CLASS);
             ppd->indexed = eina_rbtree_inline_remove(ppd->indexed, EINA_RBTREE_GET(pd),
                                                      EINA_RBTREE_CMP_NODE_CB(_children_indexed_cmp), NULL);
             pd->inserted = EINA_FALSE;
          }
        else
          {
             ERR("Unexpected parent change during the life of object: %s this might lead to crash.", efl_debug_name_get(obj));
          }
     }

   efl_invalidate(efl_super(obj, EFL_COMPOSITE_MODEL_CLASS));
}

static Efl_Object *
_efl_composite_model_efl_object_finalize(Eo *obj, Efl_Composite_Model_Data *pd)
{
   Eo *parent;

   if (pd->source == NULL)
     {
        ERR("Source of the composite model wasn't defined at construction time.");
        return NULL;
     }

   pd->self = obj;

   parent = efl_parent_get(obj);
   if (efl_isa(parent, EFL_COMPOSITE_MODEL_CLASS) && !pd->inserted && !pd->detached)
     {
        Efl_Composite_Model_Data *ppd = efl_data_scope_get(parent, EFL_COMPOSITE_MODEL_CLASS);
        Efl_Composite_Model_Data *lookup;

        lookup = (void*) eina_rbtree_inline_lookup(ppd->indexed, &pd->index, sizeof (int),
                                                   EINA_RBTREE_CMP_KEY_CB(_children_indexed_key), NULL);
        if (lookup)
          {
             // The index is taken: this one stays out of the tree, and the one there
             // keeps following inserts and removals. (Eo keeps the object built here
             // whatever finalize returns, so returning the other one never replaced
             // it.) A model stacked on a composite without an index of its own, at 0,
             // is the usual case.
             pd->detached = EINA_TRUE;
          }
        else
          {
             ppd->indexed = eina_rbtree_inline_insert(ppd->indexed, EINA_RBTREE_GET(pd),
                                                      EINA_RBTREE_CMP_NODE_CB(_children_indexed_cmp), NULL);

             pd->inserted = EINA_TRUE;
          }
     }

   return obj;
}

static void
_efl_composite_model_index_set(Eo *obj EINA_UNUSED, Efl_Composite_Model_Data *pd, unsigned int index)
{
   if (pd->set_index || !pd->source)
     return ;
   pd->index = index;
   pd->set_index = EINA_TRUE;
}

static unsigned int
_efl_composite_model_index_get(const Eo *obj, Efl_Composite_Model_Data *pd)
{
   Eina_Value *fetch = NULL;
   unsigned int r = 0xFFFFFFFF;

   if (pd->set_index)
     return pd->index;
   if (pd->need_index)
     return 0xFFFFFFFF;

   fetch = efl_model_property_get(obj, EFL_COMPOSITE_MODEL_CHILD_INDEX);
   if (!eina_value_uint_convert(fetch, &r))
     return 0xFFFFFFFF;
   eina_value_free(fetch);

   return r;
}

// A row came (added) or went at index at: move the rows that pending slices hold.
static void
_pending_shift(Efl_Composite_Model_Data *pd, unsigned int at, Eina_Bool added)
{
   Efl_Composite_Model_Slice_Request *req;
   unsigned int i;

   EINA_INLIST_FOREACH(pd->pending, req)
     {
        for (i = 0; i < req->count; i++)
          {
             unsigned int *index = &req->index[i];

             if ((*index == COMPOSITE_GONE) || (*index < at)) continue;
             if (added) (*index)++;
             else if (*index == at) *index = COMPOSITE_GONE;
             else (*index)--;
          }
     }
}

static void
_efl_composite_model_child_event(Efl_Composite_Model_Data *pd,
                                 const Efl_Model_Children_Event *ev,
                                 const Efl_Event_Description *description)
{
   Efl_Composite_Model_Data *cpd;
   Efl_Model_Children_Event cev = { 0 };
   Eina_Array mark;
   Eina_Array_Iterator iterator;
   unsigned int i;

   eina_array_step_set(&mark, sizeof (Eina_Array), 8);

   _pending_shift(pd, ev->index, description == EFL_MODEL_EVENT_CHILD_ADDED);

   // A child added at an index pushes the one that was there, and all above it, one further.
   // This has to be done before looking for the new child, or the old one at that index is it.
   if (description == EFL_MODEL_EVENT_CHILD_ADDED)
     {
        _mark_from((void*) pd->indexed, &mark, ev->index);
        EINA_ARRAY_ITER_NEXT(&mark, i, cpd, iterator)
          {
             cpd->index++;
             efl_ref(cpd->self);
          }
     }

   cev.index = ev->index;
   if (ev->child)
     {
        cev.child = _efl_composite_lookup(efl_class_get(pd->self),
                                          pd->self, ev->child, ev->index);
     }
   else
     {
        cpd = (void*) eina_rbtree_inline_lookup(pd->indexed, &cev.index, sizeof (unsigned int),
                                                EINA_RBTREE_CMP_KEY_CB(_children_indexed_key), NULL);
        if (cpd) cev.child = efl_ref(cpd->self);
     }

   if (description == EFL_MODEL_EVENT_CHILD_REMOVED)
     {
        if (cev.child)
          {
             cpd = efl_data_scope_get(cev.child, EFL_COMPOSITE_MODEL_CLASS);

             // Remove child from lookup tree if it exist before triggering anything further
             pd->indexed = eina_rbtree_inline_remove(pd->indexed, EINA_RBTREE_GET(cpd),
                                                     EINA_RBTREE_CMP_NODE_CB(_children_indexed_cmp), NULL);
             cpd->inserted = EINA_FALSE;
             efl_replace(&cpd->source, NULL);
          }

        // Everything above the removed one is one closer
        _mark_from((void*) pd->indexed, &mark, ev->index + 1);
        EINA_ARRAY_ITER_NEXT(&mark, i, cpd, iterator)
          {
             cpd->index--;
             efl_ref(cpd->self);
          }
     }

   efl_event_callback_call(pd->self, description, &cev);

   // Notify of the index change only after notifying of the removal top avoid overlap
   EINA_ARRAY_ITER_NEXT(&mark, i, cpd, iterator)
      {
         efl_model_properties_changed(cpd->self, EFL_COMPOSITE_MODEL_CHILD_INDEX);
         efl_unref(cpd->self);
      }
   eina_array_flush(&mark);

   efl_unref(cev.child);
}

static void
_efl_composite_model_child_added(void *data, const Efl_Event *event)
{
   Efl_Composite_Model_Data *pd = data;
   Efl_Model_Children_Event *ev = event->info;

   _efl_composite_model_child_event(pd, ev, EFL_MODEL_EVENT_CHILD_ADDED);
}

static void
_efl_composite_model_child_removed(void *data, const Efl_Event *event)
{
   Efl_Composite_Model_Data *pd = data;
   Efl_Model_Children_Event *ev = event->info;

   _efl_composite_model_child_event(pd, ev, EFL_MODEL_EVENT_CHILD_REMOVED);
}

EFL_CALLBACKS_ARRAY_DEFINE(composite_callbacks,
                           { EFL_MODEL_EVENT_CHILD_ADDED, _efl_composite_model_child_added },
                           { EFL_MODEL_EVENT_CHILD_REMOVED, _efl_composite_model_child_removed });

static void
_efl_composite_model_efl_ui_view_model_set(Eo *obj EINA_UNUSED, Efl_Composite_Model_Data *pd, Efl_Model *model)
{
   Eina_Iterator *properties;
   const char *property;

   if (pd->source != NULL)
     {
        ERR("Source already set for composite model. It can only be set once.");
        return ;
     }
   pd->source = efl_ref(model);

   efl_event_callback_array_add(model, composite_callbacks(), pd);
   efl_event_callback_forwarder_priority_add(model, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, obj);
   efl_event_callback_forwarder_priority_add(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, obj);

   pd->need_index = EINA_TRUE;

   properties = efl_model_properties_get(pd->source);
   EINA_ITERATOR_FOREACH(properties, property)
     {
        if (eina_streq(property, EFL_COMPOSITE_MODEL_CHILD_INDEX))
          {
             pd->need_index = EINA_FALSE;
             break;
          }
     }
   eina_iterator_free(properties);
}

static Efl_Model *
_efl_composite_model_efl_ui_view_model_get(const Eo *obj EINA_UNUSED, Efl_Composite_Model_Data *pd)
{
   return pd->source;
}

static Eina_Future *
_efl_composite_model_efl_model_property_set(Eo *obj, Efl_Composite_Model_Data *pd,
                                            const char *property, Eina_Value *value)
{
   if (pd->need_index && eina_streq(property, EFL_COMPOSITE_MODEL_CHILD_INDEX))
     {
        if (pd->set_index || !pd->source)
          return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_READ_ONLY);
        if (!eina_value_uint_convert(value, &pd->index))
          return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_UNKNOWN);
        pd->set_index = EINA_TRUE;
        return efl_loop_future_resolved(obj, eina_value_uint_init(pd->index));
     }
   return efl_model_property_set(pd->source, property, value);
}

static Eina_Value *
_efl_composite_model_efl_model_property_get(const Eo *obj EINA_UNUSED, Efl_Composite_Model_Data *pd,
                                            const char *property)
{
   Eina_Value *try;
   if (pd->need_index && eina_streq(property, EFL_COMPOSITE_MODEL_CHILD_INDEX))
     {
        if (pd->set_index)
          return eina_value_uint_new(pd->index);
        return eina_value_error_new(EAGAIN);
     }
   /* The source first: this model is a view on it, and what it would find
      on itself through Efl.Loop_Model's reflection is its own Eo properties,
      such as Efl.Object's "name" and "comment", which would hide a source's
      data of the same name. Only what the source does not have falls back
      to them. */
   if (!pd->source)
     return efl_model_property_get(efl_super(obj, EFL_COMPOSITE_MODEL_CLASS), property);
   try = efl_model_property_get(pd->source, property);
   if (eina_value_type_get(try) == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err = EINA_ERROR_NOT_IMPLEMENTED;

        if (eina_value_error_get(try, &err) &&
            ((err == EINA_ERROR_NOT_IMPLEMENTED) || (err == EFL_MODEL_ERROR_NOT_FOUND)))
          {
             Eina_Value *own;

             own = efl_model_property_get(efl_super(obj, EFL_COMPOSITE_MODEL_CLASS), property);
             /* Neither has it: what the source said is the answer. */
             if (eina_value_type_get(own) == EINA_VALUE_TYPE_ERROR)
               {
                  eina_value_free(own);
                  return try;
               }
             eina_value_free(try);
             return own;
          }
     }
   return try;
}

static Eina_Iterator *
_efl_composite_model_efl_model_properties_get(const Eo *obj EINA_UNUSED, Efl_Composite_Model_Data *pd)
{
   if (pd->need_index)
     {
        static const char *composite_properties[] = {
          EFL_COMPOSITE_MODEL_CHILD_INDEX
        };

        return eina_multi_iterator_new(efl_model_properties_get(pd->source),
                                       EINA_C_ARRAY_ITERATOR_NEW(composite_properties));
     }
   return efl_model_properties_get(pd->source);
}

static unsigned int
_efl_composite_model_efl_model_children_count_get(const Eo *obj EINA_UNUSED, Efl_Composite_Model_Data *pd)
{
   return efl_model_children_count_get(pd->source);
}

static void
_efl_composite_model_detach(Eo *obj)
{
   Efl_Composite_Model_Data *pd = efl_data_scope_get(obj, EFL_COMPOSITE_MODEL_CLASS);

   pd->detached = EINA_TRUE;
}

// A row that left before the slice holding it was answered: still the row, at no index,
// and out of the index tree, so nothing that comes later at its old index is mistaken
// for it.
static Eo *
_efl_composite_gone_add(const Efl_Class *klass, Eo *parent, Efl_Model *view)
{
   return efl_add_ref(klass, parent,
                      _efl_composite_model_detach(efl_added),
                      efl_ui_view_model_set(efl_added, view),
                      efl_composite_model_index_set(efl_added, COMPOSITE_GONE),
                      efl_loop_model_volatile_make(efl_added));
}

static Eina_Value
_efl_composite_model_then(Eo *o EINA_UNUSED, void *data, const Eina_Value v)
{
   Efl_Composite_Model_Slice_Request *req = data;
   unsigned int i, len;
   Eina_Value r = EINA_VALUE_EMPTY;
   Eo *target = NULL;

   eina_value_array_setup(&r, EINA_VALUE_TYPE_OBJECT, 4);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, target)
     {
        Eo *composite;

        // Fetch an existing composite model for this model or create a new one if none exist
        if (_request_index(req, i) == COMPOSITE_GONE)
          composite = _efl_composite_gone_add(req->self, req->parent, target);
        else
          composite = _efl_composite_lookup(req->self, req->parent, target, _request_index(req, i));
        if (!composite) continue;

        eina_value_array_append(&r, composite);
        // Dropping this scope reference
        efl_unref(composite);
     }

   while (req->dummy_need)
     {
        Eo *dummy, *dummy_proxy;

        // Create a dummy object and its proxy
        dummy = efl_add(EFL_GENERIC_MODEL_CLASS, req->parent);
        dummy_proxy = efl_add_ref(req->self, req->parent,
                                  efl_ui_view_model_set(efl_added, dummy),
                                  efl_composite_model_index_set(efl_added, _request_index(req, i)),
                                  efl_loop_model_volatile_make(efl_added));
        efl_parent_set(dummy, dummy_proxy);

        eina_value_array_append(&r, dummy_proxy);
        efl_unref(dummy_proxy);

        req->dummy_need--;
        i++;
     }

   return r;
}

static void
_efl_composite_model_clean(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Composite_Model_Slice_Request *req = data;

   req->pd->pending = eina_inlist_remove(req->pd->pending, EINA_INLIST_GET(req));
   efl_unref(req->parent);
   free(data);
}

static Eina_Future *
_efl_composite_model_efl_model_children_slice_get(Eo *obj,
                                                  Efl_Composite_Model_Data *pd,
                                                  unsigned int start,
                                                  unsigned int count)
{
   Efl_Composite_Model_Slice_Request *req;
   Eina_Future *f;
   unsigned int source_count, self_count;
   unsigned int req_count, i;

   source_count = efl_model_children_count_get(pd->source);
   self_count = efl_model_children_count_get(obj);

   if (start + count > source_count &&
       start + count > self_count)
     return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

   if (start > source_count)
     {
        Eina_Value r = EINA_VALUE_EMPTY;

        i = 0;
        eina_value_array_setup(&r, EINA_VALUE_TYPE_OBJECT, 4);

        while (count)
          {
             Eo *dummy, *dummy_proxy;

             // Create a dummy object and its proxy
             dummy = efl_add(EFL_GENERIC_MODEL_CLASS, obj);
             dummy_proxy = efl_add_ref(efl_class_get(obj), obj,
                                       efl_ui_view_model_set(efl_added, dummy),
                                       efl_composite_model_index_set(efl_added, start + i),
                                       efl_loop_model_volatile_make(efl_added));
             efl_parent_set(dummy, dummy_proxy);

             eina_value_array_append(&r, dummy_proxy);
             efl_unref(dummy_proxy);

             count--;
             i++;
          }

        return efl_loop_future_resolved(obj, r);
     }

   req_count = start + count > source_count ? source_count - start : count;
   f = efl_model_children_slice_get(pd->source, start, req_count);
   if (!f) return NULL;

   req = malloc(sizeof (Efl_Composite_Model_Slice_Request) + count * sizeof (unsigned int));
   if (!req)
     {
        eina_future_cancel(f);
        return efl_loop_future_rejected(obj, ENOMEM);
     }

   req->self = efl_class_get(obj);
   req->parent = efl_ref(obj);
   req->pd = pd;
   req->start = start;
   req->count = count;
   for (i = 0; i < count; i++)
     {
        req->index[i] = start + i;
     }
   if (start + count < source_count)
     req->dummy_need = 0;
   else
     req->dummy_need = count - (source_count - start);
   pd->pending = eina_inlist_append(pd->pending, EINA_INLIST_GET(req));

   return efl_future_then(obj, f, .success_type = EINA_VALUE_TYPE_ARRAY,
                          .success = _efl_composite_model_then,
                          .free = _efl_composite_model_clean,
                          .data = req);
}

static Efl_Object *
_efl_composite_model_efl_model_child_add(Eo *obj EINA_UNUSED,
                                         Efl_Composite_Model_Data *pd)
{
   return efl_model_child_add(pd->source);
}

static void
_efl_composite_model_efl_model_child_del(Eo *obj EINA_UNUSED,
                                         Efl_Composite_Model_Data *pd,
                                         Efl_Object *child)
{
   efl_model_child_del(pd->source, efl_ui_view_model_get(child));
}

static void
_efl_composite_model_efl_object_destructor(Eo *obj, Efl_Composite_Model_Data *pd)
{
   if (pd->source)
     {
        efl_event_callback_array_del(pd->source, composite_callbacks(), pd);
        efl_event_callback_forwarder_del(pd->source, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, obj);
        efl_event_callback_forwarder_del(pd->source, EFL_MODEL_EVENT_PROPERTIES_CHANGED, obj);

        efl_replace(&pd->source, NULL);
     }

   efl_destructor(efl_super(obj, EFL_COMPOSITE_MODEL_CLASS));
}

#include "efl_composite_model.eo.c"

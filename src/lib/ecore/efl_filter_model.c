#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl_Core.h>

#include "efl_composite_model_private.h"

typedef struct _Efl_Filter_Model_Mapping Efl_Filter_Model_Mapping;
struct _Efl_Filter_Model_Mapping
{
   EINA_RBTREE;

   uint64_t original;
   uint64_t mapped;

   EINA_REFCOUNT;
};

typedef struct _Efl_Filter_Model_Data Efl_Filter_Model_Data;
struct _Efl_Filter_Model_Data
{
   Efl_Filter_Model_Mapping *self;

   Eina_Rbtree *mapping;

   struct {
      void *data;
      EflFilterModel cb;
      Eina_Free_Cb free_cb;
      uint64_t count;
   } filter;

   uint64_t counted;
   Eina_Bool counting_started : 1;
   Eina_Bool processed : 1;
};

static Eina_Rbtree_Direction
_filter_mapping_cmp_cb(const Eina_Rbtree *left, const Eina_Rbtree *right, void *data EINA_UNUSED)
{
   const Efl_Filter_Model_Mapping *l, *r;

   l = (const Efl_Filter_Model_Mapping *) left;
   r = (const Efl_Filter_Model_Mapping *) right;

   if (l->mapped < r->mapped)
     return EINA_RBTREE_LEFT;
   return EINA_RBTREE_RIGHT;
}

static int
_filter_mapping_looking_cb(const Eina_Rbtree *node, const void *key,
                           int length EINA_UNUSED, void *data EINA_UNUSED)
{
   const Efl_Filter_Model_Mapping *n = (const Efl_Filter_Model_Mapping *) node;
   const uint64_t *k = key;

   return n->mapped - *k;
}

static void
_efl_filter_model_filter_set(Eo *obj EINA_UNUSED, Efl_Filter_Model_Data *pd,
                             void *filter_data, EflFilterModel filter, Eina_Free_Cb filter_free_cb)
{
   if (pd->filter.cb)
     pd->filter.free_cb(pd->filter.data);
   pd->filter.data = filter_data;
   pd->filter.cb = filter;
   pd->filter.free_cb = filter_free_cb;
}

static void
_rbtree_free_cb(Eina_Rbtree *node, void *data EINA_UNUSED)
{
   Efl_Filter_Model_Mapping *m = (Efl_Filter_Model_Mapping*) node;

   EINA_REFCOUNT_UNREF(m)
     free(m);
}

typedef struct _Efl_Filter_Request Efl_Filter_Request;
struct _Efl_Filter_Request
{
   Efl_Filter_Model_Data *pd;
   Efl_Model *parent;
   Efl_Model *child;
   uint64_t index;
};

static Efl_Filter_Model *
_efl_filter_lookup(const Efl_Class *klass,
                   Efl_Model *parent, Efl_Model *view,
                   Efl_Filter_Model_Mapping *mapping)
{
   Efl_Filter_Model *child;
   Efl_Filter_Model_Data *cpd;

   child = _efl_composite_lookup(klass, parent, view, mapping->mapped);
   if (!child) return NULL;

   cpd = efl_data_scope_get(child, EFL_FILTER_MODEL_CLASS);
   cpd->processed = EINA_TRUE;
   cpd->self = mapping;
   EINA_REFCOUNT_REF(mapping);

   return child;
}

static Eina_Value
_efl_filter_model_filter(Eo *o EINA_UNUSED, void *data, const Eina_Value v)
{
   Efl_Filter_Model_Mapping *mapping;
   Efl_Filter_Model *child;
   Efl_Model_Children_Event cevt = { 0 };
   Efl_Filter_Request *r = data;
   Eina_Value ret = v;
   Eina_Bool result = EINA_FALSE;

   if (!eina_value_bool_get(&v, &result)) goto end;
   if (!result) goto end;

   mapping = calloc(1, sizeof (Efl_Filter_Model_Mapping));
   if (!mapping)
     {
        ret = eina_value_bool_init(EINA_FALSE);
        goto end;
     }
   EINA_REFCOUNT_INIT(mapping);

   mapping->original = r->index;
   mapping->mapped = r->pd->filter.count++;

   r->pd->mapping = eina_rbtree_inline_insert(r->pd->mapping, EINA_RBTREE_GET(mapping),
                                              _filter_mapping_cmp_cb, NULL);

   child = _efl_filter_lookup(efl_class_get(r->parent), r->parent, r->child, mapping);
   if (!child) goto end;

   cevt.index = mapping->mapped;
   cevt.child = child;

   efl_event_callback_call(r->parent, EFL_MODEL_EVENT_CHILD_ADDED, &cevt);
   efl_event_callback_call(r->parent, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, NULL);

   efl_unref(cevt.child);

   ret = eina_value_bool_init(EINA_TRUE);

 end:
   return ret;
}

static void
_efl_filter_model_filter_clean(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Filter_Request *r = data;

   efl_unref(r->parent);
   efl_unref(r->child);
   free(r);
}

static Eina_Value
_efl_filter_model_child_fetch(Eo *o EINA_UNUSED, void *data, const Eina_Value v)
{
   Efl_Filter_Request *r = data;
   unsigned int i, len;
   Eina_Future *f;
   Eo *target = NULL;

   // Get the first and only child in the array
   EINA_VALUE_ARRAY_FOREACH(&v, len, i, target)
     break;

   r->child = efl_ref(target);

   f = r->pd->filter.cb(r->pd->filter.data, r->parent, r->child);
   f = efl_future_then(r->parent, f,
                       .success = _efl_filter_model_filter,
                       .success_type = EINA_VALUE_TYPE_BOOL,
                       .free = _efl_filter_model_filter_clean,
                       .data = r);
   return eina_future_as_value(f);
}

static Eina_Value
_efl_filter_model_child_error(Eo *o, void *data, Eina_Error err)
{
   _efl_filter_model_filter_clean(o, data, NULL);
   return eina_value_error_init(err);
}

static void
_efl_filter_model_child_added(void *data, const Efl_Event *event)
{
   Efl_Filter_Model_Data *pd = data;
   Efl_Model_Children_Event *ev = event->info;
   Efl_Model *child = ev->child;
   Efl_Filter_Request *r;
   Eina_Future *f;

   if (child)
     {
        Efl_Filter_Model_Data *cpd = efl_data_scope_get(child, EFL_FILTER_MODEL_CLASS);

        if (cpd->processed) return ;
     }

   r = calloc(1, sizeof (Efl_Filter_Request));
   if (!r) return ;

   r->pd = pd;
   r->index = ev->index;
   r->parent = efl_ref(event->object);

   if (!child)
     {
        f = efl_model_children_slice_get(efl_ui_view_model_get(r->parent), r->index, 1);
        f = efl_future_then(event->object, f,
                            .success = _efl_filter_model_child_fetch,
                            .success_type = EINA_VALUE_TYPE_ARRAY,
                            .error = _efl_filter_model_child_error,
                            .data = r);
        return ;
     }

   r->child = efl_ref(child);

   f = pd->filter.cb(pd->filter.data, r->parent, r->child);
   f = efl_future_then(event->object, f,
                       .success = _efl_filter_model_filter,
                       .success_type = EINA_VALUE_TYPE_BOOL,
                       .free = _efl_filter_model_filter_clean,
                       .data = r);

   efl_event_callback_stop(event->object);
}

static void
_efl_filter_model_child_removed(void *data, const Efl_Event *event)
{
   Efl_Filter_Model_Mapping *mapping;
   Efl_Filter_Model_Data *pd = data;
   Efl_Model_Children_Event *ev = event->info;
   uint64_t removed = ev->index;

   mapping = (void *)eina_rbtree_inline_lookup(pd->mapping,
                                               &removed, sizeof (uint64_t),
                                               _filter_mapping_looking_cb, NULL);
   if (!mapping) return;

   pd->mapping = eina_rbtree_inline_remove(pd->mapping, EINA_RBTREE_GET(mapping),
                                           _filter_mapping_cmp_cb, NULL);

   EINA_REFCOUNT_UNREF(mapping)
     free(mapping);

   // Update the tree for the index to reflect the removed child
   for (removed++; removed < pd->filter.count; removed++)
     {
        mapping = (void *)eina_rbtree_inline_lookup(pd->mapping,
                                                    &removed, sizeof (uint64_t),
                                                    _filter_mapping_looking_cb, NULL);
        if (!mapping) continue;

        pd->mapping = eina_rbtree_inline_remove(pd->mapping, EINA_RBTREE_GET(mapping),
                                                _filter_mapping_cmp_cb, NULL);
        mapping->mapped--;
        pd->mapping = eina_rbtree_inline_insert(pd->mapping, EINA_RBTREE_GET(mapping),
                                                _filter_mapping_cmp_cb, NULL);
     }
   pd->filter.count--;
   pd->counted--;
}

EFL_CALLBACKS_ARRAY_DEFINE(filters_callbacks,
                           { EFL_MODEL_EVENT_CHILD_ADDED, _efl_filter_model_child_added },
                           { EFL_MODEL_EVENT_CHILD_REMOVED, _efl_filter_model_child_removed });

static void
_efl_filter_model_efl_object_destructor(Eo *obj, Efl_Filter_Model_Data *pd)
{
   eina_rbtree_delete(pd->mapping, _rbtree_free_cb, NULL);

   if (pd->self)
     {
        EINA_REFCOUNT_UNREF(pd->self)
          free(pd->self);
     }
   pd->self = NULL;

   efl_destructor(efl_super(obj, EFL_FILTER_MODEL_CLASS));
}

static Eina_Value
_filter_remove_array(Eo *o EINA_UNUSED, void *data, const Eina_Value v)
{
   Efl_Filter_Model_Mapping *mapping = data;
   unsigned int i, len;
   Eo *target = NULL;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, target)
     break;

   if (efl_isa(target, EFL_FILTER_MODEL_CLASS))
     {
        Efl_Filter_Model_Data *pd = efl_data_scope_get(target, EFL_FILTER_MODEL_CLASS);

        pd->self = mapping;
        EINA_REFCOUNT_REF(pd->self);
     }

   return eina_value_object_init(target);
}

/* Result from eina_future_all_array is an EINA_VALUE_TYPE_ARRAY that contain Eina_Value of
   Eo Model. It is expected that children slice get return an EINA_VALUE_TYPE_ARRAY that
   contain Eo Model directly.
*/
static Eina_Value
_filter_cleanup_array(Eo *o EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   unsigned int i, len;
   Eina_Value created = EINA_VALUE_EMPTY;
   Eina_Value r = EINA_VALUE_EMPTY;

   eina_value_array_setup(&r, EINA_VALUE_TYPE_OBJECT, 4);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, created)
     {
        Eo *target = NULL;

        if (eina_value_type_get(&created) != EINA_VALUE_TYPE_OBJECT)
          goto on_error;

        target = eina_value_object_get(&created);
        if (!target) goto on_error;

        eina_value_array_append(&r, target);
     }

   return r;

 on_error:
   eina_value_flush(&r);
   return eina_value_error_init(EFL_MODEL_ERROR_UNKNOWN);
}

static Eina_Future *
_efl_filter_model_efl_model_children_slice_get(Eo *obj, Efl_Filter_Model_Data *pd,
                                               unsigned int start, unsigned int count)
{
   Efl_Filter_Model_Mapping **mapping = NULL;
   Eina_Future **r = NULL;
   Eina_Future *f;
   unsigned int i;
   Eina_Error err = ENOMEM;

   if ((uint64_t) start + (uint64_t) count > pd->filter.count)
     return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);
   if (count == 0)
     return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

   r = malloc((count + 1) * sizeof (Eina_Future *));
   if (!r) return efl_loop_future_rejected(obj, ENOMEM);

   mapping = calloc(count, sizeof (Efl_Filter_Model_Mapping *));
   if (!mapping) goto on_error;

   for (i = 0; i < count; i++)
     {
        uint64_t lookup = start + i;

        mapping[i] = (void *)eina_rbtree_inline_lookup(pd->mapping,
                                                       &lookup, sizeof (uint64_t),
                                                       _filter_mapping_looking_cb, NULL);
        if (!mapping[i]) goto on_error;
     }

   for (i = 0; i < count; i++)
     {
        r[i] = efl_model_children_slice_get(efl_super(obj, EFL_FILTER_MODEL_CLASS),
                                            mapping[i]->original, 1);
        r[i] = efl_future_then(obj, r[i], .success_type = EINA_VALUE_TYPE_ARRAY,
                               .success = _filter_remove_array,
                               .data = mapping[i]);
        if (!r) goto on_error;
     }
   r[i] = EINA_FUTURE_SENTINEL;

   f = efl_future_then(obj, eina_future_all_array(r), .success = _filter_cleanup_array);
   free(r);
   free(mapping);

   return f;

 on_error:
   free(mapping);

   if (r)
     for (i = 0; i < count; i ++)
       if (r[i]) eina_future_cancel(r[i]);
   free(r);

   return efl_loop_future_rejected(obj, err);
}

typedef struct _Efl_Filter_Model_Result Efl_Filter_Model_Result;
struct _Efl_Filter_Model_Result
{
   Efl_Filter_Model_Data *pd;
   uint64_t count;
   Efl_Model *targets[1];
};

// This future receive an array of boolean that indicate if a fetched object is to be kept
static Eina_Value
_efl_filter_model_array_result_request(Eo *o EINA_UNUSED, void *data, const Eina_Value v)
{
   Efl_Filter_Model_Result *req = data;
   Efl_Filter_Model_Data *pd = req->pd;
   unsigned int i, len;
   Eina_Value request = EINA_VALUE_EMPTY;
   uint64_t pcount = pd->filter.count;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, request)
     {
        Efl_Filter_Model_Mapping *mapping;
        Efl_Model_Children_Event cevt = { 0 };
        Eina_Bool b;

        if (eina_value_type_get(&request) != EINA_VALUE_TYPE_BOOL)
          continue ;

        if (!eina_value_bool_get(&request, &b)) continue;
        if (!b) continue;

        mapping = calloc(1, sizeof (Efl_Filter_Model_Mapping));
        if (!mapping) continue;

        EINA_REFCOUNT_INIT(mapping);
        mapping->original = i;
        mapping->mapped = pd->filter.count++;

        // Insert in tree here
        pd->mapping = eina_rbtree_inline_insert(pd->mapping, EINA_RBTREE_GET(mapping),
                                                _filter_mapping_cmp_cb, NULL);

        cevt.index = mapping->mapped;
        cevt.child = _efl_filter_lookup(efl_class_get(o), o, req->targets[i], mapping);
        if (!cevt.child) continue;

        efl_event_callback_call(o, EFL_MODEL_EVENT_CHILD_ADDED, &cevt);
        efl_unref(cevt.child);
     }

   if (pcount != pd->filter.count)
     efl_event_callback_call(o, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, NULL);

   return v;
}

static void
_efl_filter_model_array_result_free(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Filter_Model_Result *req = data;
   uint64_t i;

   for (i = 0; i < req->count; i++)
     efl_unref(req->targets[i]);
   free(req);
}

// This future receive an array of children object
static Eina_Value
_efl_filter_model_array_fetch(Eo *o, void *data, const Eina_Value v)
{
   Efl_Filter_Model_Result *req;
   Efl_Filter_Model_Data *pd = data;
   unsigned int i, len;
   Eo *target = NULL;
   Eina_Future **array = NULL;
   Eina_Future *r;
   Eina_Value res = v;

   if (!eina_value_array_count(&v)) return v;

   array = malloc((eina_value_array_count(&v) + 1) * sizeof (Eina_Future*));
   if (!array) return eina_value_error_init(ENOMEM);

   req = malloc(sizeof (Efl_Filter_Model_Result) +
                sizeof (Eo*) * (eina_value_array_count(&v) - 1));
   if (!req)
     {
        res = eina_value_error_init(ENOMEM);
        goto on_error;
     }

   req->pd = pd;
   req->count = eina_value_array_count(&v);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, target)
     {
        array[i] = pd->filter.cb(pd->filter.data, o, target);
        req->targets[i] = efl_ref(target);
     }

   array[i] = EINA_FUTURE_SENTINEL;

   r = eina_future_all_array(array);
   r = efl_future_then(o, r, .success_type = EINA_VALUE_TYPE_ARRAY,
                       .success = _efl_filter_model_array_result_request,
                       .free = _efl_filter_model_array_result_free,
                       .data = req);
   res = eina_future_as_value(r);

 on_error:
   free(array);

   return res;
}

static unsigned int
_efl_filter_model_efl_model_children_count_get(const Eo *obj, Efl_Filter_Model_Data *pd)
{
   if (!pd->counting_started && pd->filter.cb)
     {
        pd->counting_started = EINA_TRUE;

        // Start watching for children now
        efl_event_callback_array_add((Eo *)obj, filters_callbacks(), pd);

        // Start counting (which may trigger filter being added asynchronously)
        pd->counted = efl_model_children_count_get(efl_super(obj, EFL_FILTER_MODEL_CLASS));
        if (pd->counted > 0)
          {
             Eina_Future *f;

             f = efl_model_children_slice_get(efl_ui_view_model_get(obj), 0, pd->counted);
             efl_future_then(obj, f, .success_type = EINA_VALUE_TYPE_ARRAY,
                             .success = _efl_filter_model_array_fetch,
                             .data = pd);
          }
     }

   return pd->filter.count;
}

static Eina_Value *
_efl_filter_model_efl_model_property_get(const Eo *obj, Efl_Filter_Model_Data *pd,
                                         const char *property)
{
   if (pd->self && !strcmp(property, EFL_COMPOSITE_MODEL_CHILD_INDEX))
     {
        return eina_value_uint64_new(pd->self->mapped);
     }

   return efl_model_property_get(efl_super(obj, EFL_FILTER_MODEL_CLASS), property);
}

static unsigned int
_efl_filter_model_efl_composite_model_index_get(const Eo *obj, Efl_Filter_Model_Data *pd)
{
   if (pd->self) return pd->self->mapped;
   return efl_composite_model_index_get(efl_super(obj, EFL_FILTER_MODEL_CLASS));
}

#include "efl_filter_model.eo.c"

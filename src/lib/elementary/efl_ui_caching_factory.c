#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_UI_FACTORY_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Efl_Ui_Caching_Factory_Data Efl_Ui_Caching_Factory_Data;
typedef struct _Efl_Ui_Caching_Factory_Request Efl_Ui_Caching_Factory_Request;
typedef struct _Efl_Ui_Caching_Factory_Group_Request Efl_Ui_Caching_Factory_Group_Request;

struct _Efl_Ui_Caching_Factory_Data
{
   const Efl_Class *klass;

   Eina_Stringshare *style;

   // Simple list of ready-to-use objects. They are all equal so it does not matter from which
   // end of the list objects are added and removed.
   Eina_List *cache;
   Eina_Hash *lookup;
   Eina_Future *flush;

   struct {
      unsigned int memory;
      unsigned int items;
   } limit, current;

   Eina_Bool invalidated : 1;
};

struct _Efl_Ui_Caching_Factory_Request
{
   Efl_Ui_Caching_Factory_Data *pd;

   Efl_Ui_Caching_Factory *factory;
   Eo *parent;
};

struct _Efl_Ui_Caching_Factory_Group_Request
{
   Eina_Value done;
};

// Clear the cache until it meet the constraint
static void
_efl_ui_caching_factory_remove(Efl_Ui_Caching_Factory_Data *pd, Eina_List *l, Efl_Gfx_Entity *entity)
{
   pd->cache = eina_list_remove_list(pd->cache, l);
   pd->current.items--;

   pd->current.memory -= efl_class_memory_size_get(entity);
   if (efl_isa(entity, EFL_CACHED_ITEM_INTERFACE))
     pd->current.memory -= efl_cached_item_memory_size_get(entity);
}

static void
_efl_ui_caching_factory_item_del(Eo *obj, Efl_Ui_Caching_Factory_Data *pd,
                                 Eina_Iterator *entities)
{
   if (!pd->klass)
     {
        efl_ui_factory_release(efl_super(obj, EFL_UI_CACHING_FACTORY_CLASS), entities);
     }
   else
     {
        Efl_Gfx_Entity *entity;

        EINA_ITERATOR_FOREACH(entities, entity)
          efl_del(entity);
        eina_iterator_free(entities);
     }
}

static void
_efl_ui_caching_factory_flush(Eo *obj, Efl_Ui_Caching_Factory_Data *pd)
{
   Eina_Array scheduled;

   eina_array_step_set(&scheduled, sizeof (Eina_Array), 8);

   while (pd->limit.items != 0 &&
          pd->current.items > pd->limit.items)
     {
        Efl_Gfx_Entity *entity;

        entity = eina_list_data_get(eina_list_last(pd->cache));

        _efl_ui_caching_factory_remove(pd, eina_list_last(pd->cache), entity);
        if (pd->lookup) eina_hash_del(pd->lookup, efl_ui_widget_style_get(entity), entity);

        eina_array_push(&scheduled, entity);
     }

   while (pd->limit.memory != 0 &&
          pd->current.memory > pd->limit.memory)
     {
        Efl_Gfx_Entity *entity;

        entity = eina_list_data_get(eina_list_last(pd->cache));

        _efl_ui_caching_factory_remove(pd, eina_list_last(pd->cache), entity);
        if (pd->lookup) eina_hash_del(pd->lookup, efl_ui_widget_style_get(entity), entity);

        eina_array_push(&scheduled, entity);
     }

   // We could improve this by doing some limited batch to reduce potential spike usage
   _efl_ui_caching_factory_item_del(obj, pd, eina_array_iterator_new(&scheduled));
   eina_array_flush(&scheduled);
}

static Eina_Value
_efl_ui_caching_factory_uncap_then(Eo *model EINA_UNUSED,
                                   void *data EINA_UNUSED,
                                   const Eina_Value v)
{
   Efl_Ui_Widget *widget = NULL;

   if (eina_value_array_count(&v) != 1) return eina_value_error_init(EINVAL);

   eina_value_array_get(&v, 0, &widget);

   return eina_value_object_init(widget);
}

static Eina_Value
_efl_ui_caching_factory_create_then(Eo *model, void *data, const Eina_Value v)
{
   Efl_Ui_Caching_Factory_Request *r = data;
   Efl_Ui_Widget *w;
   const char *style = NULL;

   if (!eina_value_string_get(&v, &style))
     return eina_value_error_init(EFL_MODEL_ERROR_NOT_SUPPORTED);

   w = eina_hash_find(r->pd->lookup, style);
   if (!w)
     {
        Eina_Future *f;
        Eo *models[1] = { model };

        // No object of that style in the cache, need to create a new one
        // This is not ideal, we would want to gather all the request in one swoop here,
        // left for later improvement.
        f = efl_ui_factory_create(efl_super(r->factory, EFL_UI_CACHING_FACTORY_CLASS),
                                  EINA_C_ARRAY_ITERATOR_NEW(models));
        f = efl_future_then(r->factory, f,
                            .success = _efl_ui_caching_factory_uncap_then,
                            .success_type = EINA_VALUE_TYPE_ARRAY);
        return eina_future_as_value(f);
     }

   eina_hash_del(r->pd->lookup, style, w);
   _efl_ui_caching_factory_remove(r->pd, eina_list_data_find(r->pd->cache, w), w);

   efl_ui_view_model_set(w, model);
   efl_event_callback_call(r->factory, EFL_UI_FACTORY_EVENT_ITEM_BUILDING, w);

   return eina_value_object_init(w);
}

static void
_efl_ui_caching_factory_cleanup(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Ui_Caching_Factory_Request *r = data;

   efl_unref(r->factory);
   efl_unref(r->parent);
   free(r);
}

static Eina_Value
_efl_ui_caching_factory_group_create_then(Eo *obj EINA_UNUSED,
                                          void *data,
                                          const Eina_Value v)
{
   Efl_Ui_Caching_Factory_Group_Request *gr = data;
   int len, i;
   Efl_Ui_Widget *widget;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, widget)
     eina_value_array_append(&gr->done, widget);

   return eina_value_reference_copy(&gr->done);
}

static void
_efl_ui_caching_factory_group_cleanup(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Ui_Caching_Factory_Group_Request *gr = data;

   eina_value_flush(&gr->done);
   free(gr);
}

static Eina_Future *
_efl_ui_caching_factory_efl_ui_factory_create(Eo *obj,
                                              Efl_Ui_Caching_Factory_Data *pd,
                                              Eina_Iterator *models)
{
   Efl_Ui_Caching_Factory_Request *r = NULL;
   Efl_Ui_Caching_Factory_Group_Request *gr = NULL;
   Efl_Gfx_Entity *w = NULL;
   Efl_Model *model = NULL;
   Eina_Future *f = NULL;

   if (pd->cache && pd->style && !pd->klass)
     {
        Eina_Future **all = NULL;
        int count = 0;

        r = calloc(1, sizeof (Efl_Ui_Caching_Factory_Request));
        if (!r) return efl_loop_future_rejected(obj, ENOMEM);

        r->pd = pd;
        r->factory = efl_ref(obj);

        all = calloc(1, sizeof (Eina_Future *));
        if (!all) goto alloc_array_error;

        EINA_ITERATOR_FOREACH(models, model)
          {
             all[count++] = efl_future_then(model,
                                            efl_model_property_ready_get(model, pd->style),
                                            .success = _efl_ui_caching_factory_create_then,
                                            .data = r);

             Eina_Future **tmp = realloc(all, (count + 1) * sizeof (Eina_Future *));
             if (!tmp)
               {
                 free(all);
                 goto alloc_array_error;
               }
             all = tmp;
          }
        eina_iterator_free(models);

        all[count] = EINA_FUTURE_SENTINEL;

        return efl_future_then(obj, eina_future_all_array(all),
                               .data = r,
                               .free = _efl_ui_caching_factory_cleanup);
     }

   gr = calloc(1, sizeof (Efl_Ui_Caching_Factory_Group_Request));
   if (!gr) return efl_loop_future_rejected(obj, ENOMEM);

   eina_value_array_setup(&gr->done, EINA_VALUE_TYPE_OBJECT, 4);

   // First get as much object from the cache as possible
   if (pd->cache)
     EINA_ITERATOR_FOREACH(models, model)
       {
          w = eina_list_data_get(pd->cache);
          _efl_ui_caching_factory_remove(pd, pd->cache, w);

          efl_ui_view_model_set(w, model);

          eina_value_array_append(&gr->done, w);

          if (!pd->cache) break;
       }

   // Now create object on the fly that are missing from the cache
   if (pd->klass)
     {
        Efl_Ui_Widget *widget = efl_ui_widget_factory_widget_get(obj);

        EINA_ITERATOR_FOREACH(models, model)
          {
             w = efl_add(pd->klass, widget,
                         efl_ui_view_model_set(efl_added, model),
                         efl_event_callback_call(obj, EFL_UI_FACTORY_EVENT_ITEM_CONSTRUCTING, efl_added));
             efl_event_callback_call(obj, EFL_UI_FACTORY_EVENT_ITEM_BUILDING, w);
             eina_value_array_append(&gr->done, w);
          }

        f = efl_loop_future_resolved(obj, gr->done);

        eina_value_flush(&gr->done);
        free(gr);

        return f;
     }

   f = efl_ui_factory_create(efl_super(obj, EFL_UI_CACHING_FACTORY_CLASS), models);
   return efl_future_then(obj, f,
                          .success = _efl_ui_caching_factory_group_create_then,
                          .success_type = EINA_VALUE_TYPE_ARRAY,
                          .data = gr,
                          .free = _efl_ui_caching_factory_group_cleanup);

alloc_array_error:
   efl_unref(r->parent);
   efl_unref(r->factory);
   free(r);
   eina_iterator_free(models);
   return efl_loop_future_rejected(obj, ENOMEM);
}

static void
_efl_ui_caching_factory_efl_ui_widget_factory_item_class_set(Eo *obj,
                                                             Efl_Ui_Caching_Factory_Data *pd,
                                                             const Efl_Object *klass)
{
   if (efl_isa(klass, EFL_UI_VIEW_INTERFACE) &&
       !efl_isa(klass, EFL_UI_WIDGET_CLASS))
     {
        if (!efl_isa(klass, EFL_GFX_ENTITY_INTERFACE) ||
            !efl_isa(klass, EFL_UI_VIEW_INTERFACE))
          {
             ERR("Provided class '%s' for factory '%s' doesn't implement '%s' and '%s' interfaces nor '%s' and '%s' interfaces.",
                 efl_class_name_get(klass),
                 efl_class_name_get(obj),
                 efl_class_name_get(EFL_GFX_ENTITY_INTERFACE),
                 efl_class_name_get(EFL_UI_VIEW_INTERFACE),
                 efl_class_name_get(EFL_UI_WIDGET_CLASS),
                 efl_class_name_get(EFL_UI_VIEW_INTERFACE));
             return ;
          }
        pd->klass = klass;
        return;
     }
   efl_ui_widget_factory_item_class_set(efl_super(obj, EFL_UI_CACHING_FACTORY_CLASS), klass);
}

static const Efl_Object *
_efl_ui_caching_factory_efl_ui_widget_factory_item_class_get(const Eo *obj,
                                                             Efl_Ui_Caching_Factory_Data *pd)
{
   if (pd->klass) return pd->klass;
   return efl_ui_widget_factory_item_class_get(efl_super(obj, EFL_UI_CACHING_FACTORY_CLASS));
}

static void
_efl_ui_caching_factory_memory_limit_set(Eo *obj,
                                         Efl_Ui_Caching_Factory_Data *pd,
                                         unsigned int limit)
{
   pd->limit.memory = limit;

   _efl_ui_caching_factory_flush(obj, pd);
}

static unsigned int
_efl_ui_caching_factory_memory_limit_get(const Eo *obj EINA_UNUSED,
                                         Efl_Ui_Caching_Factory_Data *pd)
{
   return pd->limit.memory;
}

static void
_efl_ui_caching_factory_items_limit_set(Eo *obj,
                                        Efl_Ui_Caching_Factory_Data *pd,
                                        unsigned int limit)
{
   pd->limit.items = limit;

   _efl_ui_caching_factory_flush(obj, pd);
}

static unsigned int
_efl_ui_caching_factory_items_limit_get(const Eo *obj EINA_UNUSED,
                                        Efl_Ui_Caching_Factory_Data *pd)
{
   return pd->limit.items;
}

static Eina_Value
_schedule_cache_flush(Eo *obj, void *data, const Eina_Value v)
{
   Efl_Ui_Caching_Factory_Data *pd = data;

   // And check if the cache need some triming
   _efl_ui_caching_factory_flush(obj, pd);

   return v;
}

static void
_schedule_done(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Ui_Caching_Factory_Data *pd = data;

   pd->flush = NULL;
}

static void
_efl_ui_caching_factory_efl_ui_factory_release(Eo *obj,
                                               Efl_Ui_Caching_Factory_Data *pd,
                                               Eina_Iterator *ui_views)
{
   Efl_Gfx_Entity *ui_view;

   // Are we invalidated ?
   if (pd->invalidated)
     {
        _efl_ui_caching_factory_item_del(obj, pd, ui_views);
        return;
     }

   EINA_ITERATOR_FOREACH(ui_views, ui_view)
     {
        // Change parent, disconnect the object and make it invisible
        efl_gfx_entity_visible_set(ui_view, EINA_FALSE);
        efl_event_callback_call(obj, EFL_UI_FACTORY_EVENT_ITEM_RELEASING, ui_view);

        // Add to the cache
        pd->cache = eina_list_prepend(pd->cache, ui_view);
        pd->current.items++;
        pd->current.memory += efl_class_memory_size_get(ui_view);
        if (efl_isa(ui_view, EFL_CACHED_ITEM_INTERFACE))
          pd->current.memory += efl_cached_item_memory_size_get(ui_view);

        // Fill lookup
        if (!pd->klass && efl_ui_widget_style_get(ui_view))
          {
             if (!pd->lookup) pd->lookup = eina_hash_string_djb2_new(NULL);
             eina_hash_direct_add(pd->lookup, efl_ui_widget_style_get(ui_view), ui_view);
          }
     }
   eina_iterator_free(ui_views);

   // Schedule a cache flush if necessary
   if (!pd->flush)
     pd->flush = efl_future_then(obj, efl_loop_job(efl_loop_get(obj)),
                                 .success = _schedule_cache_flush,
                                 .free = _schedule_done,
                                 .data = pd);
}

static void
_efl_ui_caching_factory_pause(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Caching_Factory_Data *pd = data;
   Efl_Gfx_Entity *entity;

   // Application is going into background, let's free resources.
   // Possible improvement would be to delay that by a few seconds.
   EINA_LIST_FREE(pd->cache, entity)
     efl_del(entity);

   pd->current.items = 0;
   pd->current.memory = 0;
}

static void
_invalidate(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Caching_Factory_Data *pd = data;

   // As all the objects in the cache have the factory as parent, there's no need to unparent them
   pd->cache = eina_list_free(pd->cache);
   eina_hash_free(pd->lookup);
   pd->lookup = NULL;
   pd->invalidated = EINA_TRUE;
}

static Efl_Object *
_efl_ui_caching_factory_efl_object_finalize(Eo *obj, Efl_Ui_Caching_Factory_Data *pd)
{
   Efl_App *a;

   obj = efl_finalize(efl_super(obj, EFL_UI_CACHING_FACTORY_CLASS));
   if (!obj) return NULL;

   a = efl_provider_find(obj, EFL_APP_CLASS);
   if (a) efl_event_callback_add(a, EFL_APP_EVENT_PAUSE, _efl_ui_caching_factory_pause, pd);

   // The order of the invalidate event is guaranteed to happen before any children is invalidated
   // this is not the case for the children invalidate function, which can happen in random order.
   efl_event_callback_add(efl_ui_widget_factory_widget_get(obj), EFL_EVENT_INVALIDATE, _invalidate, pd);

   return obj;
}

static void
_efl_ui_caching_factory_efl_object_invalidate(Eo *obj,
                                              Efl_Ui_Caching_Factory_Data *pd)
{
   efl_event_callback_del(efl_ui_widget_factory_widget_get(obj), EFL_EVENT_INVALIDATE, _invalidate, pd);

   efl_invalidate(efl_super(obj, EFL_UI_CACHING_FACTORY_CLASS));
}

static Eina_Error
_efl_ui_caching_factory_efl_ui_property_bind_property_bind(Eo *obj, Efl_Ui_Caching_Factory_Data *pd,
                                                           const char *key, const char *property)
{
   if (!strcmp(key, "style"))
     eina_stringshare_replace(&pd->style, property);

   return efl_ui_property_bind(efl_super(obj, EFL_UI_CACHING_FACTORY_CLASS), key, property);
}

#include "efl_ui_caching_factory.eo.c"

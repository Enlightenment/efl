#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Efl_Ui_Caching_Factory_Data Efl_Ui_Caching_Factory_Data;
typedef struct _Efl_Ui_Caching_Factory_Request Efl_Ui_Caching_Factory_Request;

struct _Efl_Ui_Caching_Factory_Data
{
   const Efl_Class *klass;

   Eina_Stringshare *style;

   // Simple list of ready-to-use objects. They are all equal so it does not matter from which
   // end of the list objects are added and removed.
   Eina_List *cache;
   Eina_Hash *lookup;

   struct {
      unsigned int memory;
      unsigned int items;
   } limit, current;

   Eina_Bool invalidated : 1;
};

struct _Efl_Ui_Caching_Factory_Request
{
   Efl_Ui_Caching_Factory_Data *pd;

   Eo *parent;
   Efl_Model *model;
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
                                 Efl_Gfx_Entity *entity)
{
   if (pd->klass) efl_del(entity);
   else efl_ui_factory_release(efl_super(obj, EFL_UI_CACHING_FACTORY_CLASS), entity);
}

static void
_efl_ui_caching_factory_flush(Eo *obj, Efl_Ui_Caching_Factory_Data *pd)
{
   while (pd->limit.items != 0 &&
          pd->current.items > pd->limit.items)
     {
        Efl_Gfx_Entity *entity;

        entity = eina_list_data_get(eina_list_last(pd->cache));

        _efl_ui_caching_factory_remove(pd, eina_list_last(pd->cache), entity);
        if (pd->lookup) eina_hash_del(pd->lookup, efl_ui_widget_style_get(entity), entity);
        _efl_ui_caching_factory_item_del(obj, pd, entity);
     }

   while (pd->limit.memory != 0 &&
          pd->current.memory > pd->limit.memory)
     {
        Efl_Gfx_Entity *entity;

        entity = eina_list_data_get(eina_list_last(pd->cache));

        _efl_ui_caching_factory_remove(pd, eina_list_last(pd->cache), entity);
        if (pd->lookup) eina_hash_del(pd->lookup, efl_ui_widget_style_get(entity), entity);
        _efl_ui_caching_factory_item_del(obj, pd, entity);
     }
}

static Eina_Value
_efl_ui_caching_factory_create_then(Eo *obj EINA_UNUSED, void *data, const Eina_Value v)
{
   Efl_Ui_Caching_Factory_Request *r = data;
   Efl_Ui_Widget *w;
   const char *string = NULL;

   if (!eina_value_string_get(&v, &string))
     return eina_value_error_init(EFL_MODEL_ERROR_NOT_SUPPORTED);

   w = eina_hash_find(r->pd->lookup, string);
   if (!w)
     {
        Eina_Future *f;

        // No object of that style in the cache, need to create a new one
        f = efl_ui_factory_create(efl_super(obj, EFL_UI_CACHING_FACTORY_CLASS),
                                  r->model, r->parent);
        return eina_future_as_value(f);
     }

   eina_hash_del(r->pd->lookup, string, w);
   _efl_ui_caching_factory_remove(r->pd, r->pd->cache, w);

   efl_parent_set(w, r->parent);
   efl_ui_view_model_set(w, r->model);

   return eina_value_object_init(w);
}

static void
_efl_ui_caching_factory_create_cleanup(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Ui_Caching_Factory_Request *r = data;

   efl_unref(r->model);
   efl_unref(r->parent);
   free(r);
}

static Eina_Future *
_efl_ui_caching_factory_efl_ui_factory_create(Eo *obj,
                                              Efl_Ui_Caching_Factory_Data *pd,
                                              Efl_Model *model, Efl_Gfx_Entity *parent)
{
   Efl_Gfx_Entity *w = NULL;

   if (pd->cache)
     {
        if (pd->style && !pd->klass)
          {
             Efl_Ui_Caching_Factory_Request *r;

             r = calloc(1, sizeof (Efl_Ui_Caching_Factory_Request));
             if (!r) return efl_loop_future_rejected(obj, ENOMEM);

             r->pd = pd;
             r->parent = efl_ref(parent);
             r->model = efl_ref(model);

             return efl_future_then(obj, efl_model_property_ready_get(obj, pd->style),
                                    .success = _efl_ui_caching_factory_create_then,
                                    .data = r,
                                    .free = _efl_ui_caching_factory_create_cleanup);
          }

        w = eina_list_data_get(pd->cache);

        _efl_ui_caching_factory_remove(pd, pd->cache, w);

        efl_parent_set(w, parent);
     }

   if (!w)
     {
        if (pd->klass) w = efl_add(pd->klass, parent);
        else return efl_ui_factory_create(efl_super(obj, EFL_UI_CACHING_FACTORY_CLASS),
                                          model, parent);
     }

   efl_ui_view_model_set(w, model);

   return efl_loop_future_resolved(obj, eina_value_object_init(w));
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

static void
_efl_ui_caching_factory_efl_ui_factory_release(Eo *obj,
                                               Efl_Ui_Caching_Factory_Data *pd,
                                               Efl_Gfx_Entity *ui_view)
{
   // Are we invalidated ?
   if (pd->invalidated)
     {
        _efl_ui_caching_factory_item_del(obj, pd, ui_view);
        return;
     }

   // Change parent, disconnect the object and make it invisible
   efl_parent_set(ui_view, obj);
   efl_gfx_entity_visible_set(ui_view, EINA_FALSE);
   efl_ui_view_model_set(ui_view, NULL);

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

   // And check if the cache need some triming
   _efl_ui_caching_factory_flush(obj, pd);
}

static void
_efl_ui_caching_factory_efl_object_invalidate(Eo *obj EINA_UNUSED,
                                              Efl_Ui_Caching_Factory_Data *pd)
{
   // As all the objects in the cache have the factory as parent, there's no need to unparent them
   pd->cache = eina_list_free(pd->cache);
   eina_hash_free(pd->lookup);
   pd->lookup = NULL;
   pd->invalidated = EINA_TRUE;
}

static Efl_App *
_efl_ui_caching_factory_app_get(Eo *obj)
{
   Efl_Object *p;

   p = efl_parent_get(obj);
   if (!p) return NULL;

   // It is acceptable to just have a loop as parent and not an app
   return efl_provider_find(obj, EFL_APP_CLASS);
}

static void
_efl_ui_caching_factory_pause(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Caching_Factory_Data *pd = data;
   Efl_Gfx_Entity *entity;

   // Application is going into background, let's free ressource
   // Possible improvement would be to delay that by a few second.
   EINA_LIST_FREE(pd->cache, entity)
     efl_del(entity);

   pd->current.items = 0;
   pd->current.memory = 0;
}

static void
_efl_ui_caching_factory_efl_object_parent_set(Eo *obj, Efl_Ui_Caching_Factory_Data *pd, Efl_Object *parent)
{
   Efl_App *a;

   a = _efl_ui_caching_factory_app_get(obj);
   if (a) efl_event_callback_del(a, EFL_APP_EVENT_PAUSE, _efl_ui_caching_factory_pause, pd);

   efl_parent_set(efl_super(obj, EFL_UI_CACHING_FACTORY_CLASS), parent);

   // We are fetching the parent again, just in case the update was denied
   a = _efl_ui_caching_factory_app_get(obj);
   if (a) efl_event_callback_add(a, EFL_APP_EVENT_PAUSE, _efl_ui_caching_factory_pause, pd);
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

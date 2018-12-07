#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Efl_Ui_Caching_Factory_Data Efl_Ui_Caching_Factory_Data;
struct _Efl_Ui_Caching_Factory_Data
{
   const Efl_Class *klass;

   // Simple list of ready-to-use objects. They are all equal so it does not matter from which
   // end of the list objects are added and removed.
   Eina_List *cache;

   struct {
      unsigned int memory;
      unsigned int items;
   } limit, current;
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
_efl_ui_caching_factory_flush(Efl_Ui_Caching_Factory_Data *pd)
{
   while (pd->limit.items != 0 &&
          pd->current.items > pd->limit.items)
     {
        Efl_Gfx_Entity *entity;

        entity = eina_list_data_get(eina_list_last(pd->cache));

        _efl_ui_caching_factory_remove(pd, eina_list_last(pd->cache), entity);

        efl_del(entity);
     }

   while (pd->limit.memory != 0 &&
          pd->current.memory > pd->limit.memory)
     {
        Efl_Gfx_Entity *entity;

        entity = eina_list_data_get(eina_list_last(pd->cache));

        _efl_ui_caching_factory_remove(pd, eina_list_last(pd->cache), entity);

        efl_del(entity);
     }
}

static Eina_Future *
_efl_ui_caching_factory_efl_ui_factory_create(Eo *obj,
                                              Efl_Ui_Caching_Factory_Data *pd,
                                              Efl_Model *model, Efl_Gfx_Entity *parent)
{
   Efl_Gfx_Entity *r;

   if (pd->cache)
     {
        r = eina_list_data_get(pd->cache);

        _efl_ui_caching_factory_remove(pd, pd->cache, r);

        efl_parent_set(r, parent);
     }
   else
     {
        r = efl_add(pd->klass, parent);
     }

   efl_ui_view_model_set(r, model);

   return efl_loop_future_resolved(obj, eina_value_object_init(r));
}

static void
_efl_ui_caching_factory_item_class_set(Eo *obj,
                                       Efl_Ui_Caching_Factory_Data *pd,
                                       const Efl_Object *klass)
{
   if (!efl_isa(klass, EFL_GFX_ENTITY_INTERFACE) ||
       !efl_isa(klass, EFL_UI_VIEW_INTERFACE))
     {
        ERR("Provided class '%s' for factory '%s' doesn't implement '%s' and '%s' interfaces.",
            efl_class_name_get(klass),
            efl_class_name_get(obj),
            efl_class_name_get(EFL_GFX_ENTITY_INTERFACE),
            efl_class_name_get(EFL_UI_VIEW_INTERFACE));
        return ;
     }
   pd->klass = klass;
}

static const Efl_Object *
_efl_ui_caching_factory_item_class_get(const Eo *obj,
                                       Efl_Ui_Caching_Factory_Data *pd)
{
   return pd->klass;
}

static void
_efl_ui_caching_factory_memory_limit_set(Eo *obj,
                                         Efl_Ui_Caching_Factory_Data *pd,
                                         unsigned int limit)
{
   pd->limit.memory = limit;

   _efl_ui_caching_factory_flush(pd);
}

static unsigned int
_efl_ui_caching_factory_memory_limit_get(const Eo *obj,
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

   _efl_ui_caching_factory_flush(pd);
}

static unsigned int
_efl_ui_caching_factory_items_limit_get(const Eo *obj,
                                        Efl_Ui_Caching_Factory_Data *pd)
{
   return pd->limit.items;
}

static void
_efl_ui_caching_factory_efl_ui_factory_release(Eo *obj,
                                               Efl_Ui_Caching_Factory_Data *pd,
                                               Efl_Gfx_Entity *ui_view)
{
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

   // And check if the cache need some triming
   _efl_ui_caching_factory_flush(pd);
}

static void
_efl_ui_caching_factory_efl_object_invalidate(Eo *obj,
                                              Efl_Ui_Caching_Factory_Data *pd)
{
   // As all the objects in the cache have the factory as parent, there's no need to unparent them
   pd->cache = eina_list_free(pd->cache);
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
_efl_ui_caching_factory_pause(void *data, const Efl_Event *event)
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

#include "efl_ui_caching_factory.eo.c"

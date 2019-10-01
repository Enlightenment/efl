#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_FACTORY_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_LAYOUT_FACTORY_CLASS
#define MY_CLASS_NAME "Efl.Ui.Layout_Factory"

typedef struct _Efl_Ui_Layout_Factory_Data
{
    struct {
       Eina_Hash *properties;
       Eina_Hash *factories;
    } bind;
    Eina_Stringshare *klass;
    Eina_Stringshare *group;
    Eina_Stringshare *style;
} Efl_Ui_Layout_Factory_Data;

Eina_Bool
_property_bind(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data, void *fdata)
{
   Eo *layout = fdata;
   Eina_Stringshare *ss_key = key;
   Eina_Stringshare *property = data;

   efl_ui_property_bind(layout, ss_key, property);

   return EINA_TRUE;
}

Eina_Bool
_factory_bind(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data, void *fdata)
{
   Eo *layout = fdata;
   Eina_Stringshare *ss_key = key;
   Efl_Ui_Factory *factory = data;

   efl_ui_factory_bind(layout, ss_key, factory);
   return EINA_TRUE;
}

static void
_efl_ui_layout_factory_building(void *data, const Efl_Event *event)
{
   Efl_Ui_Layout_Factory_Data *pd = data;
   Efl_Gfx_Entity *ui_view = event->info;

   if (pd->klass || pd->group || pd->style)
     efl_ui_layout_theme_set(ui_view, pd->klass, pd->group, pd->style);

   eina_hash_foreach(pd->bind.properties, _property_bind, ui_view);
   eina_hash_foreach(pd->bind.factories, _factory_bind, ui_view);

   efl_gfx_hint_weight_set(ui_view, EFL_GFX_HINT_EXPAND, 0);
   efl_gfx_hint_fill_set(ui_view, EINA_TRUE, EINA_TRUE);
}

EOLIAN static Eo *
_efl_ui_layout_factory_efl_object_constructor(Eo *obj, Efl_Ui_Layout_Factory_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   efl_ui_widget_factory_item_class_set(obj, EFL_UI_LAYOUT_CLASS);

   pd->bind.properties = eina_hash_stringshared_new(EINA_FREE_CB(eina_stringshare_del));
   pd->bind.factories = eina_hash_stringshared_new(EINA_FREE_CB(efl_unref));

   efl_event_callback_add(obj, EFL_UI_FACTORY_EVENT_ITEM_BUILDING, _efl_ui_layout_factory_building, pd);

   return obj;
}

EOLIAN static void
_efl_ui_layout_factory_efl_object_destructor(Eo *obj, Efl_Ui_Layout_Factory_Data *pd)
{
   eina_stringshare_del(pd->klass);
   eina_stringshare_del(pd->group);
   eina_stringshare_del(pd->style);

   eina_hash_free(pd->bind.properties);
   eina_hash_free( pd->bind.factories);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Error
_efl_ui_layout_factory_efl_ui_factory_bind_factory_bind(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Factory_Data *pd,
                                                        const char *key, Efl_Ui_Factory *factory)
{
   Eina_Stringshare *ss_key;
   Efl_Ui_Factory *f_old;
   ss_key = eina_stringshare_add(key);

   if (factory == NULL)
     {
        eina_hash_del(pd->bind.factories, ss_key, NULL);
        return EINA_ERROR_NO_ERROR;
     }

   f_old = eina_hash_set(pd->bind.factories, ss_key, efl_ref(factory));
   if (f_old)
     {
        efl_unref(f_old);
        eina_stringshare_del(ss_key);
     }

   return EINA_ERROR_NO_ERROR;
}

EOLIAN static Eina_Error
_efl_ui_layout_factory_efl_ui_property_bind_property_bind(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Factory_Data *pd,
                                                          const char *key, const char *property)
{
   Eina_Stringshare *ss_key, *ss_prop;
   Eina_Stringshare *ss_old = NULL;
   ss_key = eina_stringshare_add(key);

   if (property == NULL)
     {
        eina_hash_del(pd->bind.properties, ss_key, NULL);
        goto end;
     }

   ss_prop = eina_stringshare_add(property);
   ss_old = eina_hash_set(pd->bind.properties, ss_key, ss_prop);
   if (ss_old) eina_stringshare_del(ss_old);

 end:
   efl_event_callback_call(obj, EFL_UI_PROPERTY_BIND_EVENT_PROPERTY_BOUND, (void*) ss_key);
   // Only delete our key ref it it was already present in the property hash
   if (ss_old) eina_stringshare_del(ss_key);
   return 0;
}

EOLIAN static void
_efl_ui_layout_factory_theme_config(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Factory_Data *pd,
                                    const char *klass, const char *group, const char *style)
{
   eina_stringshare_replace(&pd->klass, klass);
   eina_stringshare_replace(&pd->group, group);
   eina_stringshare_replace(&pd->style, style);
}

#include "efl_ui_layout_factory.eo.c"

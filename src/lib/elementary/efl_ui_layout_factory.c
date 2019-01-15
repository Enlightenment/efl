#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_LAYOUT_FACTORY_CLASS
#define MY_CLASS_NAME "Efl.Ui.Layout_Factory"

typedef struct _Efl_Ui_Layout_Factory_Data
{
    Eina_Hash *connects;
    Eina_Hash *factory_connects;
    Eina_Stringshare *klass;
    Eina_Stringshare *group;
    Eina_Stringshare *style;
} Efl_Ui_Layout_Factory_Data;

Eina_Bool
_model_connect(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data, void *fdata)
{
   Eo *layout = fdata;
   Eina_Stringshare *name = key;
   Eina_Stringshare *property = data;

   efl_ui_model_connect(layout, name, property);

   return EINA_TRUE;
}

Eina_Bool
_factory_model_connect(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data, void *fdata)
{
   Eo *layout = fdata;
   Eina_Stringshare *name = key;
   Efl_Ui_Factory *factory = data;

   efl_ui_factory_model_connect(layout, name, factory);
   return EINA_TRUE;
}

EOLIAN static Eo *
_efl_ui_layout_factory_efl_object_constructor(Eo *obj, Efl_Ui_Layout_Factory_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   efl_ui_caching_factory_item_class_set(obj, EFL_UI_LAYOUT_CLASS);

   pd->connects = eina_hash_stringshared_new(EINA_FREE_CB(eina_stringshare_del));
   pd->factory_connects = eina_hash_stringshared_new(EINA_FREE_CB(efl_unref));

   return obj;
}

EOLIAN static void
_efl_ui_layout_factory_efl_object_destructor(Eo *obj, Efl_Ui_Layout_Factory_Data *pd)
{
   eina_stringshare_del(pd->klass);
   eina_stringshare_del(pd->group);
   eina_stringshare_del(pd->style);

   eina_hash_free(pd->connects);
   eina_hash_free(pd->factory_connects);

   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Value
_efl_ui_layout_factory_connect(Eo *obj EINA_UNUSED, void *data, const Eina_Value value)
{
   Efl_Ui_Layout_Factory_Data *pd = data;
   Efl_Gfx_Entity *layout;

   eina_value_pget(&value, &layout);

   efl_ui_layout_theme_set(layout, pd->klass, pd->group, pd->style);

   eina_hash_foreach(pd->connects, _model_connect, layout);
   eina_hash_foreach(pd->factory_connects, _factory_model_connect, layout);

   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

   return value;
}

EOLIAN static Eina_Future *
_efl_ui_layout_factory_efl_ui_factory_create(Eo *obj, Efl_Ui_Layout_Factory_Data *pd,
                                             Efl_Model *model, Efl_Gfx_Entity *parent)
{
   Eina_Future *f;

   f = efl_ui_factory_create(efl_super(obj, EFL_UI_LAYOUT_FACTORY_CLASS), model, parent);

   return efl_future_then(obj, f,
                          .success_type = EINA_VALUE_TYPE_OBJECT,
                          .success = _efl_ui_layout_factory_connect,
                          .data = pd);
}

EOLIAN static void
_efl_ui_layout_factory_efl_ui_factory_model_connect(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Factory_Data *pd
                                                                        , const char *name, Efl_Ui_Factory *factory)
{
   Eina_Stringshare *ss_name;
   Efl_Ui_Factory *f_old;
   ss_name = eina_stringshare_add(name);

   if (factory == NULL)
     {
        eina_hash_del(pd->factory_connects, ss_name, NULL);
        return;
     }

   f_old = eina_hash_set(pd->factory_connects, ss_name, efl_ref(factory));
   if (f_old)
     {
        efl_unref(f_old);
        eina_stringshare_del(ss_name);
     }
}

EOLIAN static void
_efl_ui_layout_factory_efl_ui_model_connect_connect(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Factory_Data *pd
                                                                        , const char *name, const char *property)
{
   Eina_Stringshare *ss_name, *ss_prop, *ss_old;
   ss_name = eina_stringshare_add(name);

   if (property == NULL)
     {
        eina_hash_del(pd->connects, ss_name, NULL);
        eina_stringshare_del(ss_name);
        return;
     }

   ss_prop = eina_stringshare_add(property);
   ss_old = eina_hash_set(pd->connects, ss_name, ss_prop);
   if (ss_old)
     {
        eina_stringshare_del(ss_old);
        eina_stringshare_del(ss_name);
     }

}

EOLIAN static void
_efl_ui_layout_factory_theme_config(Eo *obj EINA_UNUSED, Efl_Ui_Layout_Factory_Data *pd
                                        , const char *klass, const char *group, const char *style)
{
   eina_stringshare_replace(&pd->klass, klass);
   eina_stringshare_replace(&pd->group, group);
   eina_stringshare_replace(&pd->style, style);
}

#include "efl_ui_layout_factory.eo.c"

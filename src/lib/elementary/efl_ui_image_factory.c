#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_IMAGE_FACTORY_CLASS
#define MY_CLASS_NAME "Efl.Ui.Image_Factory"

typedef struct _Efl_Ui_Image_Factory_Data
{
    Eina_Stringshare *property;
} Efl_Ui_Image_Factory_Data;

EOLIAN static Eo *
_efl_ui_image_factory_efl_object_constructor(Eo *obj, Efl_Ui_Image_Factory_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_ui_widget_factory_item_class_set(obj, EFL_UI_IMAGE_CLASS);

   pd->property = NULL;

   return obj;
}

EOLIAN static void
_efl_ui_image_factory_efl_object_destructor(Eo *obj EINA_UNUSED, Efl_Ui_Image_Factory_Data *pd)
{
   eina_stringshare_del(pd->property);
   pd->property = NULL;

   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Value
_efl_ui_image_factory_bind(Eo *obj EINA_UNUSED, void *data, const Eina_Value value)
{
   Efl_Gfx_Entity *entity = NULL;
   Efl_Ui_Image_Factory_Data *pd = data;

   eina_value_pget(&value, &entity);

   efl_ui_property_bind(entity, "filename", pd->property);

   return value;
}

EOLIAN static Eina_Future *
_efl_ui_image_factory_efl_ui_factory_create(Eo *obj, Efl_Ui_Image_Factory_Data *pd, Efl_Model *model, Efl_Gfx_Entity *parent)
{
   Eina_Future *f;

   if (!parent) return efl_loop_future_rejected(obj, EFL_FACTORY_ERROR_NOT_SUPPORTED);
   if (!pd->property) return efl_loop_future_rejected(obj, EFL_FACTORY_ERROR_NOT_SUPPORTED);

   f = efl_ui_factory_create(efl_super(obj, EFL_UI_IMAGE_FACTORY_CLASS), model, parent);

   return efl_future_then(obj, f,
                          .success_type = EINA_VALUE_TYPE_OBJECT,
                          .success = _efl_ui_image_factory_bind,
                          .data = pd);
}

EOLIAN static Eina_Error
_efl_ui_image_factory_efl_ui_property_bind_property_bind(Eo *obj EINA_UNUSED, Efl_Ui_Image_Factory_Data *pd, const char *key EINA_UNUSED, const char *property)
{
   eina_stringshare_replace(&pd->property, property);
   return 0;
}

#include "efl_ui_image_factory.eo.c"

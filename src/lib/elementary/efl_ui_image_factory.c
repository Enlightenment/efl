#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_FACTORY_PROTECTED

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

static void
_efl_ui_image_factory_efl_ui_factory_building(const Eo *obj EINA_UNUSED, Efl_Ui_Image_Factory_Data *pd, Efl_Gfx_Entity *ui_view)
{
   efl_ui_property_bind(ui_view, "filename", pd->property);

   efl_ui_factory_building(efl_super(obj, EFL_UI_IMAGE_FACTORY_CLASS), ui_view);
}

EOLIAN static Eina_Future *
_efl_ui_image_factory_efl_ui_factory_create(Eo *obj, Efl_Ui_Image_Factory_Data *pd,
                                            Eina_Iterator *models, Efl_Gfx_Entity *parent)
{
   if (!parent) return efl_loop_future_rejected(obj, EFL_FACTORY_ERROR_NOT_SUPPORTED);
   if (!pd->property) return efl_loop_future_rejected(obj, EFL_FACTORY_ERROR_NOT_SUPPORTED);

   return efl_ui_factory_create(efl_super(obj, EFL_UI_IMAGE_FACTORY_CLASS), models, parent);
}

EOLIAN static Eina_Error
_efl_ui_image_factory_efl_ui_property_bind_property_bind(Eo *obj EINA_UNUSED, Efl_Ui_Image_Factory_Data *pd, const char *key EINA_UNUSED, const char *property)
{
   eina_stringshare_replace(&pd->property, property);
   return 0;
}

#include "efl_ui_image_factory.eo.c"

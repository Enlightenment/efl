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

EOLIAN static Efl_Gfx_Entity *
_efl_ui_image_factory_efl_ui_factory_create(Eo *obj EINA_UNUSED, Efl_Ui_Image_Factory_Data *pd, Efl_Model *model, Efl_Gfx_Entity *parent)
{
   Efl_Gfx_Entity *ui_view;

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->property, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   ui_view = efl_add(EFL_UI_IMAGE_CLASS, parent);
   efl_ui_view_model_set(ui_view, model);
   efl_ui_model_connect(ui_view, "filename", pd->property);

   return ui_view;
}

EOLIAN static void
_efl_ui_image_factory_efl_ui_factory_release(Eo *obj EINA_UNUSED, Efl_Ui_Image_Factory_Data *pd EINA_UNUSED, Efl_Gfx_Entity *ui_view)
{
   efl_parent_set(ui_view, NULL);
}

EOLIAN static void
_efl_ui_image_factory_efl_ui_model_connect_connect(Eo *obj EINA_UNUSED, Efl_Ui_Image_Factory_Data *pd, const char *name EINA_UNUSED, const char *property)
{
    eina_stringshare_replace(&pd->property, property);
}

#include "efl_ui_image_factory.eo.c"

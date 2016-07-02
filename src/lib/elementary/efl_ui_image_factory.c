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

static Eo *
_efl_ui_image_factory_eo_base_constructor(Eo *obj, Efl_Ui_Image_Factory_Data *pd)
{
   obj = eo_constructor(eo_super(obj, MY_CLASS));

   pd->property = NULL;

   return obj;
}

static void
_efl_ui_image_factory_eo_base_destructor(Eo *obj EINA_UNUSED, Efl_Ui_Image_Factory_Data *pd)
{
   eina_stringshare_del(pd->property);
   pd->property = NULL;
}

static Efl_Canvas *
_efl_ui_image_factory_efl_ui_factory_create(Eo *obj EINA_UNUSED, Efl_Ui_Image_Factory_Data *pd, Efl_Model *model)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->property, NULL);
   Efl_Canvas *ev = eo_add(EFL_UI_IMAGE_CLASS, eo_parent_get(obj));
   efl_ui_view_model_set(ev, model);
   efl_ui_model_connect(ev, NULL, pd->property);

   return ev;
}

EOLIAN static void
_efl_ui_image_factory_efl_ui_model_connect_connect(Eo *obj EINA_UNUSED, Efl_Ui_Image_Factory_Data *pd, const char *name EINA_UNUSED, const char *property)
{
    eina_stringshare_replace(&pd->property, property);
}

#include "efl_ui_image_factory.eo.c"

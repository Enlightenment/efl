#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_bg_widget_private.h"

#define MY_CLASS EFL_UI_BG_WIDGET_CLASS
#define MY_CLASS_PFX efl_ui_bg_widget

#define MY_CLASS_NAME "Efl.Ui.Bg_Widget"

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"overlay", "elm.swallow.content"},
   {NULL, NULL}
};

EOLIAN static Eo *
_efl_ui_bg_widget_efl_object_constructor(Eo *obj, Efl_Ui_Bg_Widget_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "bg");

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   if (!elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)))
     CRI("Failed to set layout!");

   if (elm_widget_is_legacy(obj))
     {
        pd->rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS, obj,
                           efl_gfx_color_set(efl_added, 0, 0, 0, 0),
                           efl_content_set(efl_part(obj, "elm.swallow.rectangle"), efl_added));

        pd->img = efl_add(EFL_UI_IMAGE_CLASS, obj,
                          efl_gfx_image_scale_type_set(efl_added, EFL_GFX_IMAGE_SCALE_TYPE_FIT_OUTSIDE),
                          efl_content_set(efl_part(obj, "elm.swallow.background"), efl_added));
     }
   else
     {
        pd->rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS, obj,
                           efl_gfx_color_set(efl_added, 0, 0, 0, 0),
                           efl_content_set(efl_part(obj, "efl.rectangle"), efl_added));

        pd->img = efl_add(EFL_UI_IMAGE_CLASS, obj,
                          efl_gfx_image_scale_type_set(efl_added, EFL_GFX_IMAGE_SCALE_TYPE_FIT_OUTSIDE),
                          efl_content_set(efl_part(obj, "efl.background"), efl_added));
     }
   pd->file = NULL;
   pd->key = NULL;

   efl_access_object_access_type_set(obj, EFL_ACCESS_TYPE_DISABLED);

   efl_ui_widget_focus_allow_set(obj, EINA_FALSE);

   return obj;
}

EOLIAN static void
_efl_ui_bg_widget_efl_object_destructor(Eo *obj, Efl_Ui_Bg_Widget_Data *sd)
{
   ELM_SAFE_FREE(sd->file, eina_stringshare_del);
   ELM_SAFE_FREE(sd->key, eina_stringshare_del);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EAPI void
elm_bg_option_set(Evas_Object *obj, Elm_Bg_Option option)
{
   Efl_Gfx_Image_Scale_Type type;

   switch (option)
     {
       case ELM_BG_OPTION_CENTER:
         type = EFL_GFX_IMAGE_SCALE_TYPE_NONE;
         break;
       case ELM_BG_OPTION_SCALE:
         type = EFL_GFX_IMAGE_SCALE_TYPE_FIT_OUTSIDE;
         break;
       case ELM_BG_OPTION_TILE:
         type = EFL_GFX_IMAGE_SCALE_TYPE_TILE;
         break;
       case ELM_BG_OPTION_STRETCH:
         type = EFL_GFX_IMAGE_SCALE_TYPE_FILL;
         break;
       case ELM_BG_OPTION_LAST:
       default:
         type = EFL_GFX_IMAGE_SCALE_TYPE_FIT_OUTSIDE;
     }
   efl_gfx_image_scale_type_set(obj, type);
}

EAPI Elm_Bg_Option
elm_bg_option_get(const Evas_Object *obj)
{
   Efl_Gfx_Image_Scale_Type type;
   Elm_Bg_Option option = ELM_BG_OPTION_LAST;

   type = efl_gfx_image_scale_type_get(obj);
   switch (type)
     {
       case EFL_GFX_IMAGE_SCALE_TYPE_NONE:
         option = ELM_BG_OPTION_CENTER;
        break;
       case EFL_GFX_IMAGE_SCALE_TYPE_FIT_OUTSIDE:
         option = ELM_BG_OPTION_SCALE;
        break;
       case EFL_GFX_IMAGE_SCALE_TYPE_TILE:
         option = ELM_BG_OPTION_TILE;
         break;
       case EFL_GFX_IMAGE_SCALE_TYPE_FILL:
         option = ELM_BG_OPTION_STRETCH;
         break;
       case EFL_GFX_IMAGE_SCALE_TYPE_FIT_INSIDE:
       default:
         ERR("Scale type %d cannot be converted to Elm_Bg_Option", type);
         break;
     }

   return option;
}

EOLIAN static void
_efl_ui_bg_widget_efl_gfx_image_scale_type_set(Eo *obj EINA_UNUSED, Efl_Ui_Bg_Widget_Data *sd, Efl_Gfx_Image_Scale_Type scale_type)
{
   efl_gfx_image_scale_type_set(sd->img, scale_type);
}

EOLIAN static Efl_Gfx_Image_Scale_Type
_efl_ui_bg_widget_efl_gfx_image_scale_type_get(const Eo *obj EINA_UNUSED, Efl_Ui_Bg_Widget_Data *sd)
{
   return efl_gfx_image_scale_type_get(sd->img);
}

EAPI void
elm_bg_color_set(Evas_Object *obj,
                 int r,
                 int g,
                 int b)
{
   int a = 255;
   EFL_UI_BG_WIDGET_CHECK(obj);

   // reset color
   if ((r == -1) && (g == -1) && (b == -1))
   {
      r = g = b = a = 0;
   }
   efl_gfx_color_set(obj, r, g, b, a);
}

EOLIAN static void
_efl_ui_bg_widget_efl_gfx_color_color_set(Eo *obj EINA_UNUSED, Efl_Ui_Bg_Widget_Data *sd, int r, int g, int b, int a)
{
   efl_gfx_color_set(sd->rect, r, g, b, a);
}

EAPI void
elm_bg_color_get(const Evas_Object *obj,
                 int *r,
                 int *g,
                 int *b)
{
   EFL_UI_BG_WIDGET_CHECK(obj);
   efl_gfx_color_get((Eo *) obj, r, g, b, NULL);
}

EOLIAN static void
_efl_ui_bg_widget_efl_gfx_color_color_get(const Eo *obj, Efl_Ui_Bg_Widget_Data *sd, int *r, int *g, int *b, int *a)
{
   if (!sd->rect)
     efl_gfx_color_get(efl_super(obj, MY_CLASS), r, g, b, a);
   else
     efl_gfx_color_get(sd->rect, r, g, b, a);
}

EAPI void
elm_bg_load_size_set(Evas_Object *obj, int w, int h)
{
   EFL_UI_BG_WIDGET_DATA_GET_OR_RETURN(obj, sd);
   efl_gfx_image_load_controller_load_size_set(sd->img, EINA_SIZE2D(w, h));
}

EOLIAN static void
_efl_ui_bg_widget_efl_gfx_image_load_controller_load_size_set(Eo *obj EINA_UNUSED, Efl_Ui_Bg_Widget_Data *sd, Eina_Size2D sz)
{
   efl_gfx_image_load_controller_load_size_set(sd->img, sz);
}

EOLIAN static Eina_Size2D
_efl_ui_bg_widget_efl_gfx_image_load_controller_load_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Bg_Widget_Data *sd)
{
   return efl_gfx_image_load_controller_load_size_get(sd->img);
}

EAPI Eina_Bool
elm_bg_file_set(Eo *obj, const char *file, const char *group)
{
   return efl_file_set((Eo *) obj, file, group);
}

EOLIAN static Eina_Bool
_efl_ui_bg_widget_efl_file_file_set(Eo *obj EINA_UNUSED, Efl_Ui_Bg_Widget_Data *sd, const char *file, const char *key)
{
   eina_stringshare_replace(&sd->file, file);
   eina_stringshare_replace(&sd->key, key);

   return efl_file_set(sd->img, file, key);
}
EAPI void
elm_bg_file_get(const Eo *obj, const char **file, const char **group)
{
   efl_file_get((Eo *) obj, file, group);
}

EOLIAN static void
_efl_ui_bg_widget_efl_file_file_get(const Eo *obj, Efl_Ui_Bg_Widget_Data *sd, const char **file, const char **key)
{
   if (elm_widget_is_legacy(obj))
     {
        if (file) *file = sd->file;
        if (key) *key = sd->key;
        return;
     }

   efl_file_get(sd->img, file, key);
}

EOLIAN static Eina_Bool
_efl_ui_bg_widget_efl_file_mmap_set(Eo *obj EINA_UNUSED, Efl_Ui_Bg_Widget_Data *sd,
                             const Eina_File *file, const char *key)
{
   return efl_file_mmap_set(sd->img, file, key);
}

EOLIAN static void
_efl_ui_bg_widget_efl_file_mmap_get(const Eo *obj EINA_UNUSED, Efl_Ui_Bg_Widget_Data *sd,
                             const Eina_File **file, const char **key)
{
   efl_file_mmap_get(sd->img, file, key);
}

/* Internal EO APIs and hidden overrides */

ELM_LAYOUT_CONTENT_ALIASES_IMPLEMENT(MY_CLASS_PFX)

#define EFL_UI_BG_WIDGET_EXTRA_OPS \
   ELM_LAYOUT_CONTENT_ALIASES_OPS(MY_CLASS_PFX)

#include "efl_ui_bg_widget.eo.c"


#include "efl_ui_bg_widget_legacy.eo.h"

#define MY_CLASS_NAME_LEGACY "elm_bg"

static void
_efl_ui_bg_widget_legacy_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Eo *
_efl_ui_bg_widget_legacy_efl_object_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, EFL_UI_BG_WIDGET_LEGACY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   EFL_UI_BG_WIDGET_DATA_GET_OR_RETURN_VAL(obj, pd, obj);
   efl_gfx_entity_scale_set(pd->img, 1.0);

   return obj;
}

EAPI Evas_Object *
elm_bg_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(EFL_UI_BG_WIDGET_LEGACY_CLASS, parent);
}

#include "efl_ui_bg_widget_legacy.eo.c"

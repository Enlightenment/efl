#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_PROTECTED
#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_bg_widget_private.h"

#define MY_CLASS EFL_UI_BG_WIDGET_CLASS
#define MY_CLASS_PFX efl_ui_bg_widget

#define MY_CLASS_NAME "Efl.Ui.Bg_Widget"
#define MY_CLASS_NAME_LEGACY "elm_bg"

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"overlay", "elm.swallow.content"},
   {NULL, NULL}
};

EOLIAN static void
_efl_ui_bg_widget_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Bg_Widget_Data *sd)
{
   Evas_Coord iw = 0, ih = 0, mw = -1, mh = -1;
   Evas_Coord bx = 0, by = 0, bw = 0, bh = 0;
   Evas_Coord fx = 0, fy = 0, fw = 0, fh = 0;
   const char *p;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if ((!sd->img) || (!sd->file)) return;
   if (((p = strrchr(sd->file, '.'))) && (!strcasecmp(p, ".edj"))) return;

   /* grab image size */
   evas_object_image_size_get(sd->img, &iw, &ih);
   if ((iw < 1) || (ih < 1)) return;

   /* grab base object dimensions */
   evas_object_geometry_get
     (wd->resize_obj, &bx, &by, &bw, &bh);

   switch (sd->scale_type)
     {
       case EFL_IMAGE_SCALE_TYPE_NONE:
         fw = mw = iw;
         fh = mh = ih;
         break;
       case EFL_IMAGE_SCALE_TYPE_FILL:
         fw = bw;
         fh = bh;
         break;
       case EFL_IMAGE_SCALE_TYPE_FIT_INSIDE:
         mw = fw = bw;
         mh = fh = ((ih * fw) / iw);
         if (fh > bh)
           {
              mh = fh = bh;
              mw = fw = ((iw * fh) / ih);
           }
         break;
       case EFL_IMAGE_SCALE_TYPE_FIT_OUTSIDE:
         fw = bw;
         fh = ((ih * fw) / iw);
         if (fh < bh)
           {
              fh = bh;
              fw = ((iw * fh) / ih);
           }
         fx = ((bw - fw) / 2);
         fy = ((bh - fh) / 2);
         break;
       case EFL_IMAGE_SCALE_TYPE_TILE:
         fw = iw;
         fh = ih;
         break;
     }

   evas_object_image_fill_set(sd->img, fx, fy, fw, fh);

   evas_object_size_hint_min_set(sd->img, mw, mh);
   evas_object_size_hint_max_set(sd->img, mw, mh);
}

static void
_on_resize(void *data,
           Evas *e EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

EAPI Evas_Object *
elm_bg_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_efl_ui_bg_widget_efl_object_constructor(Eo *obj, Efl_Ui_Bg_Widget_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "bg");

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_FALSE);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _on_resize, obj);

   if (!elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)))
     CRI("Failed to set layout!");

   pd->scale_type = EFL_IMAGE_SCALE_TYPE_FIT_OUTSIDE;
   pd->rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS, obj,
                      efl_gfx_color_set(efl_added, 0, 0, 0, 0),
                      efl_content_set(efl_part(obj, "elm.swallow.rectangle"), efl_added));

   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   efl_access_type_set(obj, EFL_ACCESS_TYPE_DISABLED);

   efl_ui_widget_focus_allow_set(obj, EINA_FALSE);

   return obj;
}

EOLIAN static Eina_Bool
_efl_ui_bg_widget_efl_file_file_set(Eo *obj, Efl_Ui_Bg_Widget_Data *sd, const char *file, const char *group)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   const char *p;
   Eina_Bool int_ret;

   ELM_SAFE_FREE(sd->img, evas_object_del);
   if (!file)
     {
        ELM_SAFE_FREE(sd->file, eina_stringshare_del);
        ELM_SAFE_FREE(sd->group, eina_stringshare_del);
        return EINA_TRUE;
     }
   eina_stringshare_replace(&sd->file, file);
   eina_stringshare_replace(&sd->group, group);
   if (((p = strrchr(file, '.'))) && (!strcasecmp(p, ".edj")))
     {
        sd->img = edje_object_add
            (evas_object_evas_get(wd->resize_obj));
        int_ret = edje_object_file_set(sd->img, file, group);
     }
   else
     {
        int err;

        sd->img = evas_object_image_add
            (evas_object_evas_get(wd->resize_obj));
        evas_object_image_load_orientation_set(sd->img, EINA_TRUE);
        if ((sd->load_opts.w > 0) && (sd->load_opts.h > 0))
          evas_object_image_load_size_set
            (sd->img, sd->load_opts.w, sd->load_opts.h);
        evas_object_image_file_set(sd->img, file, group);

        err = evas_object_image_load_error_get(sd->img);
        if (err != EVAS_LOAD_ERROR_NONE)
          {
             ERR("Could not load image '%s': %s",
                 file, evas_load_error_str(err));
             int_ret = EINA_FALSE;
          }
        else
          int_ret = EINA_TRUE;
     }

   evas_object_repeat_events_set(sd->img, EINA_TRUE);

   int_ret &= elm_layout_content_set(obj, "elm.swallow.background", sd->img);

   elm_layout_sizing_eval(obj);

   return int_ret;
}

EOLIAN static void
_efl_ui_bg_widget_efl_file_file_get(Eo *obj EINA_UNUSED, Efl_Ui_Bg_Widget_Data *sd, const char **file, const char **group)
{
   if (file) *file = sd->file;
   if (group) *group = sd->group;
}

EAPI void
elm_bg_option_set(Evas_Object *obj, Elm_Bg_Option option)
{
  EFL_UI_BG_WIDGET_DATA_GET(obj, sd);
  switch (option)
    {
      case ELM_BG_OPTION_CENTER:
        sd->scale_type = EFL_IMAGE_SCALE_TYPE_NONE;
        break;
      case ELM_BG_OPTION_SCALE:
        sd->scale_type = EFL_IMAGE_SCALE_TYPE_FIT_OUTSIDE;
        break;
      case ELM_BG_OPTION_TILE:
        sd->scale_type = EFL_IMAGE_SCALE_TYPE_TILE;
        break;
      case ELM_BG_OPTION_STRETCH:
        sd->scale_type = EFL_IMAGE_SCALE_TYPE_FILL;
        break;
      case ELM_BG_OPTION_LAST:
      default:
        sd->scale_type = EFL_IMAGE_SCALE_TYPE_FIT_OUTSIDE;
    }

  elm_layout_sizing_eval(obj);
}

EAPI Elm_Bg_Option
elm_bg_option_get(const Evas_Object *obj)
{
  Elm_Bg_Option option = ELM_BG_OPTION_LAST;
  EFL_UI_BG_WIDGET_DATA_GET(obj, sd);

  switch (sd->scale_type)
  {
    case EFL_IMAGE_SCALE_TYPE_NONE:
      option = ELM_BG_OPTION_CENTER;
      break;
    case EFL_IMAGE_SCALE_TYPE_FIT_OUTSIDE:
      option = ELM_BG_OPTION_SCALE;
      break;
    case EFL_IMAGE_SCALE_TYPE_TILE:
      option = ELM_BG_OPTION_TILE;
      break;
    case EFL_IMAGE_SCALE_TYPE_FILL:
      option = ELM_BG_OPTION_STRETCH;
      break;
    case EFL_IMAGE_SCALE_TYPE_FIT_INSIDE:
    default:
      ERR("Scale type %d cannot be converted to Elm_Bg_Option", sd->scale_type);
      break;
  }
  return option;
}

EOLIAN static void
_efl_ui_bg_widget_efl_image_scale_type_set(Eo *obj EINA_UNUSED, Efl_Ui_Bg_Widget_Data *sd, Efl_Image_Scale_Type scale_type)
{
  sd->scale_type = scale_type;

  elm_layout_sizing_eval(obj);
}

EOLIAN static Efl_Image_Scale_Type
_efl_ui_bg_widget_efl_image_scale_type_get(Eo *obj EINA_UNUSED, Efl_Ui_Bg_Widget_Data *sd)
{
   return sd->scale_type;
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
_efl_ui_bg_widget_efl_gfx_color_set(Eo *obj EINA_UNUSED, Efl_Ui_Bg_Widget_Data *sd, int r, int g, int b, int a)
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
_efl_ui_bg_widget_efl_gfx_color_get(Eo *obj, Efl_Ui_Bg_Widget_Data *sd, int *r, int *g, int *b, int *a)
{
   if (!sd->rect)
     efl_gfx_color_get(efl_super(obj, MY_CLASS), r, g, b, a);
   else
     efl_gfx_color_get(sd->rect, r, g, b, a);
}

EAPI void
elm_bg_load_size_set(Evas_Object *obj, int w, int h)
{
   const char *p;
   EFL_UI_BG_WIDGET_DATA_GET(obj, sd);

   sd->load_opts.w = w;
   sd->load_opts.h = h;
   if (!sd->img) return;

   if (!(((p = strrchr(sd->file, '.'))) && (!strcasecmp(p, ".edj"))))
     evas_object_image_load_size_set(sd->img, w, h);
}

static void
_efl_ui_bg_widget_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EAPI Eina_Bool
elm_bg_file_set(Eo *obj, const char *file, const char *group)
{
   return efl_file_set((Eo *) obj, file, group);
}

EAPI void
elm_bg_file_get(const Eo *obj, const char **file, const char **group)
{
   efl_file_get((Eo *) obj, file, group);
}

/* Internal EO APIs and hidden overrides */

ELM_LAYOUT_CONTENT_ALIASES_IMPLEMENT(MY_CLASS_PFX)

#define EFL_UI_BG_WIDGET_EXTRA_OPS \
   ELM_LAYOUT_CONTENT_ALIASES_OPS(MY_CLASS_PFX), \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_bg_widget)

#include "efl_ui_bg_widget.eo.c"

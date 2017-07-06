#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_bg.h"
#include "elm_widget_layout.h"

#define MY_CLASS ELM_BG_CLASS

#define MY_CLASS_NAME "Elm_Bg"
#define MY_CLASS_NAME_LEGACY "elm_bg"

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"overlay", "elm.swallow.content"},
   {NULL, NULL}
};

EOLIAN static void
_elm_bg_elm_layout_sizing_eval(Eo *obj, Elm_Bg_Data *sd)
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

   switch (sd->option)
     {
      case ELM_BG_OPTION_CENTER:
        fw = mw = iw;
        fh = mh = ih;
        break;

      case ELM_BG_OPTION_SCALE:
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

      case ELM_BG_OPTION_TILE:
        fw = iw;
        fh = ih;
        break;

      case ELM_BG_OPTION_STRETCH:
      default:
        fw = bw;
        fh = bh;
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

EOLIAN static void
_elm_bg_efl_canvas_group_group_add(Eo *obj, Elm_Bg_Data *priv)
{

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   priv->option = ELM_BG_OPTION_SCALE;

   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _on_resize, obj);

   if (!elm_layout_theme_set(obj, "bg", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_bg_elm_layout_content_aliases_get(Eo *obj EINA_UNUSED, Elm_Bg_Data *_pd EINA_UNUSED)
{
   return _content_aliases;
}

EAPI Evas_Object *
elm_bg_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return efl_add(MY_CLASS, parent, efl_canvas_object_legacy_ctor(efl_added));
}

EOLIAN static Eo *
_elm_bg_efl_object_constructor(Eo *obj, Elm_Bg_Data *_pd EINA_UNUSED)
{
   elm_interface_atspi_accessible_type_set(obj, ELM_ATSPI_TYPE_DISABLED);
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);

   return obj;
}

EOLIAN static Eina_Bool
_elm_bg_efl_file_file_set(Eo *obj, Elm_Bg_Data *sd, const char *file, const char *group)
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
             ERR("Could not load image '%s': %s\n",
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
_elm_bg_efl_file_file_get(Eo *obj EINA_UNUSED, Elm_Bg_Data *sd, const char **file, const char **group)
{
   if (file) *file = sd->file;
   if (group) *group = sd->group;
}

EOLIAN static void
_elm_bg_option_set(Eo *obj, Elm_Bg_Data *sd, Elm_Bg_Option option)
{
   sd->option = option;

   elm_layout_sizing_eval(obj);
}

EOLIAN static Elm_Bg_Option
_elm_bg_option_get(Eo *obj EINA_UNUSED, Elm_Bg_Data *sd)
{
   return sd->option;
}

EAPI void
elm_bg_color_set(Evas_Object *obj,
                 int r,
                 int g,
                 int b)
{
   ELM_BG_CHECK(obj);
   elm_obj_bg_color_set(obj, r, g, b, 255);
}

EOLIAN static void
_elm_bg_color_set(Eo *obj, Elm_Bg_Data *sd, int r, int g, int b, int a)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   // reset color
   if ((r == -1) && (g == -1) && (b == -1))
     {
        ELM_SAFE_FREE(sd->rect, evas_object_del);
        return;
     }

   if (!sd->rect)
     {
        sd->rect = evas_object_rectangle_add
            (evas_object_evas_get(wd->resize_obj));

        elm_layout_content_set(obj, "elm.swallow.rectangle", sd->rect);

        elm_layout_sizing_eval(obj);
     }

   evas_object_color_set(sd->rect, r, g, b, a);
}

EAPI void
elm_bg_color_get(const Evas_Object *obj,
                 int *r,
                 int *g,
                 int *b)
{
   ELM_BG_CHECK(obj);
   elm_obj_bg_color_get((Eo *) obj, r, g, b, NULL);
}

EOLIAN static void
_elm_bg_color_get(Eo *obj EINA_UNUSED, Elm_Bg_Data *sd, int *r, int *g, int *b, int *a)
{
   evas_object_color_get(sd->rect, r, g, b, a);
}

EOLIAN static void
_elm_bg_load_size_set(Eo *obj EINA_UNUSED, Elm_Bg_Data *sd, Evas_Coord w, Evas_Coord h)
{
   const char *p;

   sd->load_opts.w = w;
   sd->load_opts.h = h;
   if (!sd->img) return;

   if (!(((p = strrchr(sd->file, '.'))) && (!strcasecmp(p, ".edj"))))
     evas_object_image_load_size_set(sd->img, w, h);
}

static void
_elm_bg_class_constructor(Efl_Class *klass)
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

#define ELM_BG_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(elm_bg)

#include "elm_bg.eo.c"

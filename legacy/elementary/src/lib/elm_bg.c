#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

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
   Evas_Coord nx = 0, ny = 0, nw = 0, nh = 0;
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

   /* set some defaults */
   nx = bx;
   ny = by;
   nw = bw;
   nh = bh;

   switch (sd->option)
     {
      case ELM_BG_OPTION_CENTER:
        fw = nw = iw;
        fh = nh = ih;
        nx = ((bw - fw) / 2);
        ny = ((bh - fh) / 2);
        mw = iw;
        mh = ih;
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

   evas_object_move(sd->img, nx, ny);
   evas_object_resize(sd->img, nw, nh);
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
_elm_bg_evas_object_smart_add(Eo *obj, Elm_Bg_Data *priv)
{

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
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
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_bg_eo_base_constructor(Eo *obj, Elm_Bg_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj, evas_obj_type_set(MY_CLASS_NAME_LEGACY));
}

static void
_elm_bg_file_reload(void *data, Evas_Object *obj,
                    const char *emission EINA_UNUSED,
                    const char *source EINA_UNUSED)
{
   Evas_Object *bg = data;
   const char *file;
   const char *group;

   edje_object_file_get(obj, &file, &group);
   elm_bg_file_set(bg, file, group);
}

EOLIAN static Eina_Bool
_elm_bg_file_set(Eo *obj, Elm_Bg_Data *sd, const char *file, const char *group)
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
        edje_object_signal_callback_del
          (sd->img, "edje,change,file", "edje", _elm_bg_file_reload);
        edje_object_signal_callback_add
          (sd->img, "edje,change,file", "edje", _elm_bg_file_reload, obj);
     }
   else
     {
        int err;

        sd->img = evas_object_image_add
            (evas_object_evas_get(wd->resize_obj));
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
_elm_bg_file_get(Eo *obj EINA_UNUSED, Elm_Bg_Data *sd, const char **file, const char **group)
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
   eo_do(obj, elm_obj_bg_color_set(r, g, b, 255));
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
   eo_do((Eo *) obj, elm_obj_bg_color_get(r, g, b, NULL));
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
_elm_bg_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_bg.eo.c"

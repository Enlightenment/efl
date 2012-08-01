#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_bg.h"

EAPI const char ELM_BG_SMART_NAME[] = "elm_bg";

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"overlay", "elm.swallow.content"},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_BG_SMART_NAME, _elm_bg, Elm_Bg_Smart_Class, Elm_Layout_Smart_Class,
  elm_layout_smart_class_get, NULL);

static void
_elm_bg_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord iw = 0, ih = 0, mw = -1, mh = -1;
   Evas_Coord bx = 0, by = 0, bw = 0, bh = 0;
   Evas_Coord fx = 0, fy = 0, fw = 0, fh = 0;
   Evas_Coord nx = 0, ny = 0, nw = 0, nh = 0;
   const char *p;

   ELM_BG_DATA_GET(obj, sd);

   if ((!sd->img) || (!sd->file)) return;
   if (((p = strrchr(sd->file, '.'))) && (!strcasecmp(p, ".edj"))) return;

   /* grab image size */
   evas_object_image_size_get(sd->img, &iw, &ih);
   if ((iw < 1) || (ih < 1)) return;

   /* grab base object dimensions */
   evas_object_geometry_get
     (ELM_WIDGET_DATA(sd)->resize_obj, &bx, &by, &bw, &bh);

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
           Evas *e __UNUSED__,
           Evas_Object *obj __UNUSED__,
           void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static void
_elm_bg_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Bg_Smart_Data);

   ELM_WIDGET_CLASS(_elm_bg_parent_sc)->base.add(obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   priv->option = ELM_BG_OPTION_SCALE;

   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _on_resize, obj);

   elm_layout_theme_set(obj, "bg", "base", elm_widget_style_get(obj));
}

static void
_elm_bg_smart_set_user(Elm_Bg_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_bg_smart_add;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_bg_smart_sizing_eval;

   ELM_LAYOUT_CLASS(sc)->content_aliases = _content_aliases;
}

EAPI const Elm_Bg_Smart_Class *
elm_bg_smart_class_get(void)
{
   static Elm_Bg_Smart_Class _sc =
     ELM_BG_SMART_CLASS_INIT_NAME_VERSION(ELM_BG_SMART_NAME);
   static const Elm_Bg_Smart_Class *class = NULL;

   if (class)
     return class;

   _elm_bg_smart_set(&_sc);
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_bg_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_bg_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

static void
_elm_bg_file_reload(void *data, Evas_Object *obj,
                    const char *emission __UNUSED__,
                    const char *source __UNUSED__)
{
   Evas_Object *bg = data;
   const char *file;
   const char *group;

   edje_object_file_get(obj, &file, &group);
   elm_bg_file_set(bg, file, group);
}

EAPI Eina_Bool
elm_bg_file_set(Evas_Object *obj,
                const char *file,
                const char *group)
{
   ELM_BG_CHECK(obj) EINA_FALSE;
   ELM_BG_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   const char *p;
   Eina_Bool ret;

   if (sd->img)
     {
        evas_object_del(sd->img);
        sd->img = NULL;
     }
   if (!file)
     {
        eina_stringshare_del(sd->file);
        sd->file = NULL;
        eina_stringshare_del(sd->group);
        sd->group = NULL;
        return EINA_TRUE;
     }
   eina_stringshare_replace(&sd->file, file);
   eina_stringshare_replace(&sd->group, group);
   if (((p = strrchr(file, '.'))) && (!strcasecmp(p, ".edj")))
     {
        sd->img = edje_object_add
            (evas_object_evas_get(ELM_WIDGET_DATA(sd)->resize_obj));
        ret = edje_object_file_set(sd->img, file, group);
        edje_object_signal_callback_del
          (sd->img, "edje,change,file", "edje", _elm_bg_file_reload);
        edje_object_signal_callback_add
          (sd->img, "edje,change,file", "edje", _elm_bg_file_reload, obj);
     }
   else
     {
        int err;

        sd->img = evas_object_image_add
            (evas_object_evas_get(ELM_WIDGET_DATA(sd)->resize_obj));
        if ((sd->load_opts.w > 0) && (sd->load_opts.h > 0))
          evas_object_image_load_size_set
            (sd->img, sd->load_opts.w, sd->load_opts.h);
        evas_object_image_file_set(sd->img, file, group);

        err = evas_object_image_load_error_get(sd->img);
        if (err != EVAS_LOAD_ERROR_NONE)
          {
             ERR("Could not load image '%s': %s\n",
                 file, evas_load_error_str(err));
             ret = EINA_FALSE;
          }
        else
          ret = EINA_TRUE;
     }

   evas_object_repeat_events_set(sd->img, EINA_TRUE);

   ret &= elm_layout_content_set(obj, "elm.swallow.background", sd->img);

   elm_layout_sizing_eval(obj);

   return ret;
}

EAPI void
elm_bg_file_get(const Evas_Object *obj,
                const char **file,
                const char **group)
{
   ELM_BG_CHECK(obj);
   ELM_BG_DATA_GET_OR_RETURN(obj, sd);

   if (file) *file = sd->file;
   if (group) *group = sd->group;
}

EAPI void
elm_bg_option_set(Evas_Object *obj,
                  Elm_Bg_Option option)
{
   ELM_BG_CHECK(obj);
   ELM_BG_DATA_GET_OR_RETURN(obj, sd);

   sd->option = option;

   elm_layout_sizing_eval(obj);
}

EAPI Elm_Bg_Option
elm_bg_option_get(const Evas_Object *obj)
{
   ELM_BG_CHECK(obj) EINA_FALSE;
   ELM_BG_DATA_GET_OR_RETURN_VAL(obj, sd, ELM_BG_OPTION_LAST);

   return sd->option;
}

EAPI void
elm_bg_color_set(Evas_Object *obj,
                 int r,
                 int g,
                 int b)
{
   ELM_BG_CHECK(obj);
   ELM_BG_DATA_GET_OR_RETURN(obj, sd);

   if (!sd->rect)
     {
        sd->rect = evas_object_rectangle_add
            (evas_object_evas_get(ELM_WIDGET_DATA(sd)->resize_obj));

        elm_layout_content_set(obj, "elm.swallow.rectangle", sd->rect);

        elm_layout_sizing_eval(obj);
     }

   evas_object_color_set(sd->rect, r, g, b, 255);
}

EAPI void
elm_bg_color_get(const Evas_Object *obj,
                 int *r,
                 int *g,
                 int *b)
{
   ELM_BG_CHECK(obj);
   ELM_BG_DATA_GET_OR_RETURN(obj, sd);

   evas_object_color_get(sd->rect, r, g, b, NULL);
}

EAPI void
elm_bg_load_size_set(Evas_Object *obj,
                     Evas_Coord w,
                     Evas_Coord h)
{
   ELM_BG_CHECK(obj);
   ELM_BG_DATA_GET_OR_RETURN(obj, sd);
   const char *p;

   sd->load_opts.w = w;
   sd->load_opts.h = h;
   if (!sd->img) return;

   if (!(((p = strrchr(sd->file, '.'))) && (!strcasecmp(p, ".edj"))))
     evas_object_image_load_size_set(sd->img, w, h);
}

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_bg.h"
#include "elm_widget_layout.h"

EAPI Eo_Op ELM_OBJ_BG_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_BG_CLASS

#define MY_CLASS_NAME "Elm_Bg"
#define MY_CLASS_NAME_LEGACY "elm_bg"

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"overlay", "elm.swallow.content"},
   {NULL, NULL}
};

static void
_elm_bg_smart_sizing_eval(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Coord iw = 0, ih = 0, mw = -1, mh = -1;
   Evas_Coord bx = 0, by = 0, bw = 0, bh = 0;
   Evas_Coord fx = 0, fy = 0, fw = 0, fh = 0;
   Evas_Coord nx = 0, ny = 0, nw = 0, nh = 0;
   const char *p;

   Elm_Bg_Smart_Data *sd = _pd;
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
           Evas *e __UNUSED__,
           Evas_Object *obj __UNUSED__,
           void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static void
_elm_bg_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Bg_Smart_Data *priv = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   priv->option = ELM_BG_OPTION_SCALE;

   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _on_resize, obj);

   if (!elm_layout_theme_set(obj, "bg", "base", elm_widget_style_get(obj)))
     CRITICAL("Failed to set layout!");
}

static void
_elm_bg_smart_content_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _content_aliases;
}

EAPI Evas_Object *
elm_bg_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj, evas_obj_type_set(MY_CLASS_NAME_LEGACY));
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
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_bg_file_set(file, group, &ret));
   return ret;
}

static void
_file_set(Eo *obj, void *_pd, va_list *list)
{
   const char *file = va_arg(*list, const char *);
   const char *group = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Bg_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   const char *p;
   Eina_Bool int_ret;

   if (sd->img)
     {
        evas_object_del(sd->img);
        sd->img = NULL;
     }
   if (!file)
     {
        ELM_SAFE_FREE(sd->file, eina_stringshare_del);
        ELM_SAFE_FREE(sd->group, eina_stringshare_del);
        if (ret) *ret = EINA_TRUE;
        return;
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

   if (ret) *ret = int_ret;
}

EAPI void
elm_bg_file_get(const Evas_Object *obj,
                const char **file,
                const char **group)
{
   ELM_BG_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_bg_file_get(file, group));
}

static void
_file_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **file = va_arg(*list, const char **);
   const char **group = va_arg(*list, const char **);
   Elm_Bg_Smart_Data *sd = _pd;

   if (file) *file = sd->file;
   if (group) *group = sd->group;
}

EAPI void
elm_bg_option_set(Evas_Object *obj,
                  Elm_Bg_Option option)
{
   ELM_BG_CHECK(obj);
   eo_do(obj, elm_obj_bg_option_set(option));
}

static void
_option_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Bg_Option option = va_arg(*list, Elm_Bg_Option);
   Elm_Bg_Smart_Data *sd = _pd;

   sd->option = option;

   elm_layout_sizing_eval(obj);
}

EAPI Elm_Bg_Option
elm_bg_option_get(const Evas_Object *obj)
{
   ELM_BG_CHECK(obj) ELM_BG_OPTION_LAST;
   Elm_Bg_Option ret = ELM_BG_OPTION_LAST;
   eo_do((Eo *) obj, elm_obj_bg_option_get(&ret));
   return ret;
}

static void
_option_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Bg_Option *ret = va_arg(*list, Elm_Bg_Option *);
   Elm_Bg_Smart_Data *sd = _pd;

   *ret = sd->option;
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

static void
_color_set(Eo *obj, void *_pd, va_list *list)
{
   int r = va_arg(*list, int);
   int g = va_arg(*list, int);
   int b = va_arg(*list, int);
   int a = va_arg(*list, int);
   Elm_Bg_Smart_Data *sd = _pd;
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

static void
_color_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *r = va_arg(*list, int *);
   int *g = va_arg(*list, int *);
   int *b = va_arg(*list, int *);
   int *a = va_arg(*list, int *);
   Elm_Bg_Smart_Data *sd = _pd;

   evas_object_color_get(sd->rect, r, g, b, a);
}

EAPI void
elm_bg_load_size_set(Evas_Object *obj,
                     Evas_Coord w,
                     Evas_Coord h)
{
   ELM_BG_CHECK(obj);
   eo_do(obj, elm_obj_bg_load_size_set(w, h));
}

static void
_load_size_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   Elm_Bg_Smart_Data *sd = _pd;
   const char *p;

   sd->load_opts.w = w;
   sd->load_opts.h = h;
   if (!sd->img) return;

   if (!(((p = strrchr(sd->file, '.'))) && (!strcasecmp(p, ".edj"))))
     evas_object_image_load_size_set(sd->img, w, h);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_bg_smart_add),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_bg_smart_sizing_eval),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET), _elm_bg_smart_content_aliases_get),

        EO_OP_FUNC(ELM_OBJ_BG_ID(ELM_OBJ_BG_SUB_ID_FILE_SET), _file_set),
        EO_OP_FUNC(ELM_OBJ_BG_ID(ELM_OBJ_BG_SUB_ID_FILE_GET), _file_get),
        EO_OP_FUNC(ELM_OBJ_BG_ID(ELM_OBJ_BG_SUB_ID_OPTION_SET), _option_set),
        EO_OP_FUNC(ELM_OBJ_BG_ID(ELM_OBJ_BG_SUB_ID_OPTION_GET), _option_get),
        EO_OP_FUNC(ELM_OBJ_BG_ID(ELM_OBJ_BG_SUB_ID_COLOR_SET), _color_set),
        EO_OP_FUNC(ELM_OBJ_BG_ID(ELM_OBJ_BG_SUB_ID_COLOR_GET), _color_get),
        EO_OP_FUNC(ELM_OBJ_BG_ID(ELM_OBJ_BG_SUB_ID_LOAD_SIZE_SET), _load_size_set),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_BG_SUB_ID_FILE_SET, "Set the file (image or edje collection) to give life for the background."),
     EO_OP_DESCRIPTION(ELM_OBJ_BG_SUB_ID_FILE_GET, "Get the file (image or edje collection) set on a given background."),
     EO_OP_DESCRIPTION(ELM_OBJ_BG_SUB_ID_OPTION_SET, "Set the mode of display for a given background widget's image."),
     EO_OP_DESCRIPTION(ELM_OBJ_BG_SUB_ID_OPTION_GET, "Get the mode of display for a given background widget's image."),
     EO_OP_DESCRIPTION(ELM_OBJ_BG_SUB_ID_COLOR_SET, "Set the color on a given background widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_BG_SUB_ID_COLOR_GET, "Get the color set on a given background widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_BG_SUB_ID_LOAD_SIZE_SET, "Set the size of the pixmap representation of the image set on a given background widget."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_BG_BASE_ID, op_desc, ELM_OBJ_BG_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Bg_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_bg_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);


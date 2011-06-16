#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *base, *rect, *img, *overlay;
   const char  *file, *group;
   Elm_Bg_Option option;
   struct {
      Evas_Coord w, h;
   } load_opts;
};

static const char *widtype = NULL;

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _custom_resize(void *data, Evas *a, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord w, h;

   _elm_theme_object_set(obj, wd->base, "bg", "base",
                         elm_widget_style_get(obj));

   if (wd->rect)
     edje_object_part_swallow(wd->base, "elm.swallow.rectangle", wd->rect);
   if (wd->img)
     edje_object_part_swallow(wd->base, "elm.swallow.background", wd->img);
   if (wd->overlay)
     edje_object_part_swallow(wd->base, "elm.swallow.content", wd->overlay);

   // FIXME: if i don't do this, bg doesnt calc correctly. why?
   evas_object_geometry_get(wd->base, NULL, NULL, &w, &h);
   evas_object_resize(wd->base, w, h);
}

static void
_custom_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = data;
   Evas_Coord bx = 0, by = 0, bw = 0, bh = 0;
   Evas_Coord iw = 0, ih = 0, mw = -1, mh = -1;
   Evas_Coord fx = 0, fy = 0, fw = 0, fh = 0;
   Evas_Coord nx = 0, ny = 0, nw = 0, nh = 0;
   const char *p;

   if ((!wd->img) || (!wd->file)) return;
   if (((p = strrchr(wd->file, '.'))) && (!strcasecmp(p, ".edj"))) return;

   /* grab image size */
   evas_object_image_size_get(wd->img, &iw, &ih);
   if ((iw < 1) || (ih < 1)) return;

   /* grab base object dimensions */
   evas_object_geometry_get(wd->base, &bx, &by, &bw, &bh);

   /* set some defaults */
   nx = bx;
   ny = by;
   nw = bw;
   nh = bh;

   switch (wd->option)
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

   evas_object_move(wd->img, nx, ny);
   evas_object_resize(wd->img, nw, nh);
   evas_object_image_fill_set(wd->img, fx, fy, fw, fh);

   evas_object_size_hint_min_set(wd->img, mw, mh);
   evas_object_size_hint_max_set(wd->img, mw, mh);
}

EAPI Evas_Object *
elm_bg_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "bg");
   elm_widget_type_set(obj, "bg");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   wd->base = edje_object_add(e);
   _elm_theme_object_set(obj, wd->base, "bg", "base", "default");
   elm_widget_resize_object_set(obj, wd->base);

   evas_object_event_callback_add(wd->base, EVAS_CALLBACK_RESIZE,
                                  _custom_resize, wd);

   wd->option = ELM_BG_OPTION_SCALE;
   return obj;
}

EAPI void
elm_bg_file_set(Evas_Object *obj, const char *file, const char *group)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *p;

   if (wd->img)
     {
        evas_object_del(wd->img);
        wd->img = NULL;
     }
   if (!file)
     {
        eina_stringshare_del(wd->file);
        wd->file = NULL;
        eina_stringshare_del(wd->group);
        wd->group = NULL;
        return;
     }
   eina_stringshare_replace(&wd->file, file);
   eina_stringshare_replace(&wd->group, group);
   if (((p = strrchr(file, '.'))) && (!strcasecmp(p, ".edj")))
     {
        wd->img = edje_object_add(evas_object_evas_get(wd->base));
        edje_object_file_set(wd->img, file, group);
     }
   else
     {
        wd->img = evas_object_image_add(evas_object_evas_get(wd->base));
        if ((wd->load_opts.w > 0) && (wd->load_opts.h > 0))
          evas_object_image_load_size_set(wd->img, wd->load_opts.w, wd->load_opts.h);
        evas_object_image_file_set(wd->img, file, group);
     }
   evas_object_repeat_events_set(wd->img, EINA_TRUE);
   edje_object_part_swallow(wd->base, "elm.swallow.background", wd->img);
   elm_widget_sub_object_add(obj, wd->img);
   _custom_resize(wd, NULL, NULL, NULL);
}

EAPI void
elm_bg_file_get(const Evas_Object *obj, const char **file, const char **group)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (file) *file = wd->file;
   if (group) *group = wd->group;
}

EAPI void
elm_bg_option_set(Evas_Object *obj, Elm_Bg_Option option)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   wd->option = option;
   _custom_resize(wd, NULL, NULL, NULL);
}

EAPI Elm_Bg_Option
elm_bg_option_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   return wd->option;
}

EAPI void
elm_bg_color_set(Evas_Object *obj, int r, int g, int b)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd->rect)
     {
        wd->rect = evas_object_rectangle_add(evas_object_evas_get(wd->base));
        edje_object_part_swallow(wd->base, "elm.swallow.rectangle", wd->rect);
        elm_widget_sub_object_add(obj, wd->rect);
        _custom_resize(wd, NULL, NULL, NULL);
     }
   evas_object_color_set(wd->rect, r, g, b, 255);
}

EAPI void
elm_bg_color_get(const Evas_Object *obj, int *r, int *g, int *b)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   evas_object_color_get(wd->rect, r, g, b, NULL);
}

EAPI void
elm_bg_overlay_set(Evas_Object *obj, Evas_Object *overlay)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->overlay)
     {
        evas_object_del(wd->overlay);
        wd->overlay = NULL;
     }
   if (overlay)
     {
        wd->overlay = overlay;
        edje_object_part_swallow(wd->base, "elm.swallow.content", wd->overlay);
        elm_widget_sub_object_add(obj, wd->overlay);
     }

   _custom_resize(wd, NULL, NULL, NULL);
}

EAPI Evas_Object *
elm_bg_overlay_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->overlay;
}

EAPI Evas_Object *
elm_bg_overlay_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *overlay;
   if (!wd) return NULL;
   if (!wd->overlay) return NULL;
   overlay = wd->overlay;
   elm_widget_sub_object_del(obj, wd->overlay);
   edje_object_part_unswallow(wd->base, wd->overlay);
   wd->overlay = NULL;
   _custom_resize(wd, NULL, NULL, NULL);
   return overlay;
}

EAPI void
elm_bg_load_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *p;
   if (!wd) return;
   wd->load_opts.w = w;
   wd->load_opts.h = h;
   if (!wd->img) return;
   if (!(((p = strrchr(wd->file, '.'))) && (!strcasecmp(p, ".edj"))))
     evas_object_image_load_size_set(wd->img, w, h);
}


#include <Elementary.h>
#include "elm_priv.h"

static void _elm_bg_file_set(Elm_Bg *bg, const char *file, const char *group);
static void _elm_bg_del(Elm_Bg *bg);

Elm_Bg_Class _elm_bg_class =
{
   &_elm_widget_class,
     ELM_OBJ_BG,
     _elm_bg_file_set
};

static void
_elm_bg_custom_resize(void *data, Evas *a, Evas_Object *obj, void *event_info)
{
   Elm_Bg *bg = data;
   int iw = 0, ih = 0;
   Evas_Coord x, y, w, h, ow = 0, oh = 0;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   evas_object_image_size_get(obj, &iw, &ih);

   if ((iw < 1) || (ih < 1)) return;
   w = ow;
   h = (ih * w) / iw;
   if (h < oh)
     {
	h = oh;
	w = (iw * h) / ih;
     }
   x = (ow - w) / 2;
   y = (oh - h) / 2;
   evas_object_image_fill_set(obj, x, y, w, h);
}

static void
_elm_bg_file_set(Elm_Bg *bg, const char *file, const char *group)
{
   const char *p;
   
   if (bg->custom_bg)
     {
	evas_object_del(bg->custom_bg);
	bg->custom_bg = NULL;
     }
   if (!file) return;
   if (bg->file) evas_stringshare_del(bg->file);
   if (file) bg->file = evas_stringshare_add(file);
   else bg->file = NULL;
   if (bg->group) evas_stringshare_del(bg->group);
   if (group) bg->group = evas_stringshare_add(group);
   else bg->group = NULL;
   if (((p = strrchr(file, '.'))) && (!strcasecmp(p, ".edj")))
     {
	bg->custom_bg = edje_object_add(evas_object_evas_get(bg->base));
	edje_object_file_set(bg->custom_bg, file, group);
     }
   else 
     {
	bg->custom_bg = evas_object_image_add(evas_object_evas_get(bg->base));
	evas_object_event_callback_add(bg->custom_bg, EVAS_CALLBACK_RESIZE, _elm_bg_custom_resize, bg);
	evas_object_image_file_set(bg->custom_bg, file, group);
     }
   evas_object_repeat_events_set(bg->custom_bg, 1);
   edje_object_part_swallow(bg->base, "elm.swallow.background", bg->custom_bg);
   evas_object_show(bg->custom_bg);
}

static void
_elm_bg_del(Elm_Bg *bg)
{
   if (bg->custom_bg) evas_object_del(bg->custom_bg);
   if (bg->group) evas_stringshare_del(bg->group);
   if (bg->file) evas_stringshare_del(bg->file);
   ((Elm_Obj_Class *)(((Elm_Bg_Class *)(bg->clas))->parent))->del(ELM_OBJ(bg));
}

EAPI Elm_Bg *
elm_bg_new(Elm_Win *win)
{
   Elm_Bg *bg;
   
   bg = ELM_NEW(Elm_Bg);
   
   _elm_widget_init(bg);
   bg->clas = &_elm_bg_class;
   bg->type = ELM_OBJ_BG;

   bg->del = _elm_bg_del;
   
   bg->file_set = _elm_bg_file_set;

   bg->base = edje_object_add(win->evas);
   _elm_theme_set(bg->base, "bg", "bg");
   _elm_widget_post_init(bg);
   win->child_add(win, bg);
   return bg;
}

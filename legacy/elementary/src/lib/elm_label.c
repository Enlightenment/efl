#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *lbl;
   const char *label;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->label) eina_stringshare_del(wd->label);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _elm_theme_set(wd->lbl, "label", "base", "default");
   edje_object_part_text_set(wd->lbl, "elm.text", wd->label);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   
   edje_object_size_min_calc(wd->lbl, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

EAPI Evas_Object *
elm_label_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, 0);
   
   wd->lbl = edje_object_add(e);
   _elm_theme_set(wd->lbl, "label", "base", "default");
   wd->label = eina_stringshare_add("<br>");
   edje_object_part_text_set(wd->lbl, "elm.text", "<br>");
   elm_widget_resize_object_set(obj, wd->lbl);
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_label_label_set(Evas_Object *obj, const char *label)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!label) label = "";
   if (wd->label) eina_stringshare_del(wd->label);
   wd->label = eina_stringshare_add(label);
   edje_object_part_text_set(wd->lbl, "elm.text", label);
   _sizing_eval(obj);
}

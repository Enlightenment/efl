#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *ent;
};

static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   
   edje_object_size_min_calc(wd->ent, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_on_focus_hook(void *data, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   evas_object_focus_set(wd->ent, 1);
}

EAPI Evas_Object *
elm_entry_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_can_focus_set(obj, 1);
   
   wd->ent = edje_object_add(e);
   _elm_theme_set(wd->ent, "entry", "base", "default");
   elm_widget_resize_object_set(obj, wd->ent);
   return obj;
}

EAPI void
elm_entry_entry_set(Evas_Object *obj, const char *entry)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_part_text_set(wd->ent, "elm.text", entry);
   _sizing_eval(obj);
}

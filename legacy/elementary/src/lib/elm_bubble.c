#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *bbl;
   Evas_Object *content, *icon;
   const char *label, *info;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->label) eina_stringshare_del(wd->label);
   if (wd->info) eina_stringshare_del(wd->info);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _elm_theme_set(wd->bbl, "bubble", "base", "default");
   edje_object_part_text_set(wd->bbl, "elm.text", wd->label);
   edje_object_part_text_set(wd->bbl, "elm.info", wd->info);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   
   edje_object_size_min_calc(wd->bbl, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (wd->content)
     edje_object_part_swallow(wd->bbl, "elm.swallow.content", wd->content);
   if (wd->icon)
     edje_object_part_swallow(wd->bbl, "elm.swallow.icon", wd->icon);
   _sizing_eval(data);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (sub == wd->content)
     {
	evas_object_event_callback_del
	  (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
	wd->content = NULL;
	_sizing_eval(obj);
     }
   if (sub == wd->icon)
     {
	evas_object_event_callback_del
	  (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
	wd->icon = NULL;
	_sizing_eval(obj);
     }
}

EAPI Evas_Object *
elm_bubble_add(Evas_Object *parent)
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
   
   wd->bbl = edje_object_add(e);
   _elm_theme_set(wd->bbl, "bubble", "base", "default");
   elm_widget_resize_object_set(obj, wd->bbl);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_bubble_label_set(Evas_Object *obj, const char *label)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->label) eina_stringshare_del(wd->label);
   if (label) wd->label = eina_stringshare_add(label);
   else wd->label = NULL;
   edje_object_part_text_set(wd->bbl, "elm.text", label);
   _sizing_eval(obj);
}

EAPI void
elm_bubble_info_set(Evas_Object *obj, const char *info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->info) eina_stringshare_del(wd->info);
   if (info) wd->info = eina_stringshare_add(info);
   else wd->info = NULL;
   edje_object_part_text_set(wd->bbl, "elm.info", info);
   _sizing_eval(obj);
}

EAPI void
elm_bubble_content_set(Evas_Object *obj, Evas_Object *content)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((wd->content != content) && (wd->content))
     elm_widget_sub_object_del(obj, wd->content);
   wd->content = content;
   if (content)
     {
	elm_widget_sub_object_add(obj, content);
	edje_object_part_swallow(wd->bbl, "elm.swallow.content", content);
	evas_object_event_callback_add(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	_sizing_eval(obj);
     }
}

EAPI void
elm_bubble_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((wd->icon != icon) && (wd->icon))
     elm_widget_sub_object_del(obj, wd->icon);
   wd->icon = icon;
   if (icon)
     {
	elm_widget_sub_object_add(obj, icon);
	edje_object_part_swallow(wd->bbl, "elm.swallow.icon", icon);
	evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	_sizing_eval(obj);
     }
}

EAPI void
elm_bubble_corner_set(Evas_Object *obj, const char *corner)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   
   _elm_theme_set(wd->bbl, "bubble", corner, "default");
   if (wd->icon)
     edje_object_part_swallow(wd->bbl, "elm.swallow.icon", wd->icon);
   if (wd->content)
     edje_object_part_swallow(wd->bbl, "elm.swallow.content", wd->content);
   // FIXME: fix label etc.
   _sizing_eval(obj);
}

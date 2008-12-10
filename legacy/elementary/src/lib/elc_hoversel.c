#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *btn, *hover;
   Evas_Object *hover_parent;
   const char *hover_style;
};

static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void
_del_pre_hook(Evas_Object *obj)
{
   elm_hoversel_hover_end(obj);
   elm_hoversel_hover_parent_set(obj, NULL);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->hover_style) eina_stringshare_del(wd->hover_style);
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   evas_object_size_hint_min_get(wd->btn, &minw, &minh);
   evas_object_size_hint_max_get(wd->btn, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_hover_clicked(void *data, Evas_Object *obj, void *event_info)
{
   elm_hoversel_hover_end(data);
}

static void
_button_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Object *bt, *bx, *ic;
   wd->hover = elm_hover_add(data);
   elm_hover_style_set(wd->hover, "hoversel_vertical");
   evas_object_smart_callback_add(wd->hover, "clicked", _hover_clicked, data);
   elm_hover_parent_set(wd->hover, wd->hover_parent);
   elm_hover_target_set(wd->hover, wd->btn);

   bx = elm_box_add(wd->hover);
   elm_box_homogenous_set(bx, 1);
   
   bt = elm_button_add(wd->hover);
   elm_button_style_set(bt, "hoversel_vertical_entry");
   elm_button_label_set(bt, "Forward");
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   
   // FIXME: fill.

   elm_hover_content_set
     (wd->hover,
      elm_hover_best_content_location_get(wd->hover, ELM_HOVER_AXIS_VERTICAL),
      bx);
   evas_object_show(bx);
   
   evas_object_show(wd->hover);
}

static void
_parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->hover_parent = NULL;
}

EAPI Evas_Object *
elm_hoversel_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   
   wd->btn = elm_button_add(parent);
   elm_button_style_set(wd->btn, "hoversel_vertical");
   elm_widget_resize_object_set(obj, wd->btn);
   evas_object_event_callback_add(wd->btn, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _changed_size_hints, obj);
   evas_object_smart_callback_add(wd->btn, "clicked", _button_clicked, obj);
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_hoversel_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->hover_parent)
     evas_object_event_callback_del(wd->hover_parent, EVAS_CALLBACK_DEL, _parent_del);
   wd->hover_parent = parent;
   if (wd->hover_parent)
     evas_object_event_callback_add(wd->hover_parent, EVAS_CALLBACK_DEL, _parent_del, obj);
}

EAPI void
elm_hoversel_label_set(Evas_Object *obj, const char *label)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_button_label_set(wd->btn, label);
}

EAPI void
elm_hoversel_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_button_icon_set(wd->btn, icon);
}

EAPI void
elm_hoversel_hover_end(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->hover) evas_object_del(wd->hover);
   wd->hover = NULL;
}

EAPI void * // FIXME: return some sort of handle other than void *
elm_hoversel_item_add(Evas_Object *obj, const char *label, const char *icon_file, int icon_type, void (*func) (void *data, Evas_Object *obj, void *event_into), const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   // FIXME: implement
}

EAPI void
elm_hoversel_item_del(void *item)
{
   // FIXME: implement
}

EAPI void
elm_hoversel_item_enabled_set(void *item, Evas_Bool enabled)
{
   // FIXME: implement
}

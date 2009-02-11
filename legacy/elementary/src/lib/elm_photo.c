#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *frm;
   Evas_Object *img;
   int size;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _mouse_up(void *data, Evas_Object *obj, void *event_info);

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
   edje_object_scale_set(wd->frm, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
 
   evas_object_size_hint_min_set(wd->img, 
                                 wd->size * elm_widget_scale_get(obj) * _elm_config->scale,
                                 wd->size * elm_widget_scale_get(obj) * _elm_config->scale);
   edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->img);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->frm, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   maxw = minw;
   maxh = minh;
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_mouse_up(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "clicked", NULL);
}

EAPI Evas_Object *
elm_photo_add(Evas_Object *parent)
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

   wd->frm = edje_object_add(e);
   _elm_theme_set(wd->frm, "photo", "base", "default");
   elm_widget_resize_object_set(obj, wd->frm);
   
   wd->img = _els_smart_icon_add(e);
   _els_smart_icon_scale_up_set(wd->img, 1);
   _els_smart_icon_scale_down_set(wd->img, 1);
   _els_smart_icon_smooth_scale_set(wd->img, 1);
   _els_smart_icon_fill_inside_set(wd->img, 0);
   _els_smart_icon_scale_set(wd->img, elm_widget_scale_get(obj) * _elm_config->scale);
   evas_object_event_callback_add(wd->img, EVAS_CALLBACK_MOUSE_UP,
				  _mouse_up, obj);
   evas_object_repeat_events_set(wd->img, 1);
   edje_object_part_swallow(wd->frm, "elm.swallow.content", wd->img);
   evas_object_show(wd->img);
   
   wd->size = 40;
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_photo_file_set(Evas_Object *obj, const char *file)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _els_smart_icon_file_key_set(wd->img, file, NULL);
   _sizing_eval(obj);
}

EAPI void
elm_photo_size_set(Evas_Object *obj, int size)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (size < 0) size = 0;
   wd->size = size;
   _sizing_eval(obj);
}

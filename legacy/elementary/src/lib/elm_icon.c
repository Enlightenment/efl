#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *img;
   Evas_Bool    scale_up : 1;
   Evas_Bool    scale_down : 1;
   Evas_Bool    smooth : 1;
   Evas_Bool    fill_outside : 1;
   Evas_Bool    no_scale : 1;
};

static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _mouse_up(void *data, Evas_Object *obj, void *event_info);

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
   int w, h;
 
   _els_smart_icon_size_get(wd->img, &w, &h);
   _els_smart_icon_scale_up_set(wd->img, wd->scale_up);
   _els_smart_icon_scale_down_set(wd->img, wd->scale_down);
   _els_smart_icon_smooth_scale_set(wd->img, wd->smooth);
   _els_smart_icon_fill_inside_set(wd->img, !(wd->fill_outside));
   if (wd->no_scale) _els_smart_icon_scale_set(wd->img, 1.0);
   else
     {
	_els_smart_icon_scale_set(wd->img, _elm_config->scale);
	w *= _elm_config->scale;
	h *= _elm_config->scale;
     }
   if (!wd->scale_down)
     {
	minw = w;
	minh = h;
     }
   if (!wd->scale_up)
     {
	maxw = w;
	maxh = h;
     }
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_mouse_up(void *data, Evas_Object *obj, void *event_info)
{
   evas_object_smart_callback_call(data, "clicked", NULL);
}

EAPI Evas_Object *
elm_icon_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_can_focus_set(obj, 0);
   
   wd->img = _els_smart_icon_add(e);
   evas_object_event_callback_add(wd->img, EVAS_CALLBACK_MOUSE_UP,
				  _mouse_up, obj);
   evas_object_repeat_events_set(wd->img, 1);
   elm_widget_resize_object_set(obj, wd->img);   

   wd->smooth = 1;
   wd->scale_up = 1;
   wd->scale_down = 1;
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_icon_file_set(Evas_Object *obj, const char *file, const char *group)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *p;
   
   if (!file) return;
   if (((p = strrchr(file, '.'))) && (!strcasecmp(p, ".edj")))
     _els_smart_icon_file_edje_set(wd->img, file, group);
   else 
     _els_smart_icon_file_key_set(wd->img, file, group);
   _sizing_eval(obj);
}

EAPI void
elm_icon_standard_set(Evas_Object *obj, const char *name)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   
   if (!name) return;
   _elm_theme_icon_set(wd->img, name, "default");
   _sizing_eval(obj);
}

EAPI void
elm_icon_smooth_set(Evas_Object *obj, Evas_Bool smooth)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->smooth = smooth;
   _sizing_eval(obj);
}

EAPI void
elm_icon_no_scale_set(Evas_Object *obj, Evas_Bool no_scale)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->no_scale = no_scale;
   _sizing_eval(obj);
}

EAPI void
elm_icon_scale_set(Evas_Object *obj, Evas_Bool scale_up, Evas_Bool scale_down)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->scale_up = scale_up;
   wd->scale_down = scale_down;
   _sizing_eval(obj);
}

EAPI void
elm_icon_fill_outside_set(Evas_Object *obj, Evas_Bool fill_outside)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->fill_outside = fill_outside;
   _sizing_eval(obj);
}

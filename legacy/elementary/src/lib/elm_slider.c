#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *slider;
   Evas_Object *icon;
   Evas_Object *spacer;
   const char *label;
   const char *units;
   const char *indicator;
   Evas_Bool horizontal : 1;
   Evas_Bool inverted : 1;
   double val, val_min, val_max;
   Ecore_Timer *delay;
   Evas_Coord size;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _units_set(Evas_Object *obj);
static void _indicator_set(Evas_Object *obj);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->label) eina_stringshare_del(wd->label);
   if (wd->indicator) eina_stringshare_del(wd->units);
   if (wd->delay) ecore_timer_del(wd->delay);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->horizontal)
     _elm_theme_set(wd->slider, "slider", "horizontal", "default");
   else
     _elm_theme_set(wd->slider, "slider", "vertical", "default");
   if (wd->inverted)
     edje_object_signal_emit(wd->slider, "elm,state,inverted,on", "elm");
   else
     edje_object_signal_emit(wd->slider, "elm,state,inverted,off", "elm");
   if (wd->icon)
     edje_object_signal_emit(wd->slider, "elm,state,icon,visible", "elm");
   else
     edje_object_signal_emit(wd->slider, "elm,state,icon,hidden", "elm");
   if (wd->label)
     edje_object_signal_emit(wd->slider, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->slider, "elm,state,text,hidden", "elm");
   edje_object_part_text_set(wd->slider, "elm.text", wd->label);
   if (wd->units)
     edje_object_signal_emit(wd->slider, "elm,state,units,visible", "elm");
   else
     edje_object_signal_emit(wd->slider, "elm,state,units,hidden", "elm");
   if (wd->horizontal)
     evas_object_size_hint_min_set(wd->spacer, (double)wd->size * elm_widget_scale_get(obj) * _elm_config->scale, 1);
   else 
     evas_object_size_hint_min_set(wd->spacer, 1, (double)wd->size * elm_widget_scale_get(obj) * _elm_config->scale);
   edje_object_part_swallow(wd->slider, "elm.swallow.bar", wd->spacer);
   _units_set(obj);
   edje_object_message_signal_process(wd->slider);
   edje_object_scale_set(wd->slider, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   
   edje_object_size_min_calc(wd->slider, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (obj != wd->icon) return;
   edje_object_part_swallow(wd->slider, "elm.swallow.content", obj);
   _sizing_eval(data);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (sub == wd->icon)
     {
	edje_object_signal_emit(wd->slider, "elm,state,icon,hidden", "elm");
	evas_object_event_callback_del
	  (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
	wd->icon = NULL;
	_sizing_eval(obj);
     }
}

static int
_delay_change(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->delay = NULL;
   evas_object_smart_callback_call(data, "delay,changed", NULL);
   return 0;
}

static void
_val_fetch(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   double posx = 0.0, posy = 0.0, pos = 0.0, val;
   
   edje_object_part_drag_value_get(wd->slider, "elm.dragable.slider", 
                                   &posx, &posy);
   if (wd->horizontal) pos = posx;
   else pos = posy;
   if (wd->inverted) pos = 1.0 - pos;
   val = (pos * (wd->val_max - wd->val_min)) + wd->val_min;
   if (val != wd->val)
     {
        wd->val = val;
        evas_object_smart_callback_call(obj, "changed", NULL);
        if (wd->delay) ecore_timer_del(wd->delay);
        wd->delay = ecore_timer_add(0.2, _delay_change, obj);
     }
}

static void
_val_set(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   double pos;
   if (wd->val_max > wd->val_min)
     pos = (wd->val - wd->val_min) / (wd->val_max - wd->val_min);
   else
     pos = 0.0;
   if (pos < 0.0) pos = 0.0;
   else if (pos > 1.0) pos = 1.0;
   if (wd->inverted) pos = 1.0 - pos;
   edje_object_part_drag_value_set(wd->slider, "elm.dragable.slider", pos, pos);
}

static void
_units_set(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->units)
     {
        char buf[1024];
        
        snprintf(buf, sizeof(buf), wd->units, wd->val);
        edje_object_part_text_set(wd->slider, "elm.units", buf);
     }
   else
     edje_object_part_text_set(wd->slider, "elm.units", NULL);
}

static void
_indicator_set(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->indicator)
     {
        char buf[1024];
        
        snprintf(buf, sizeof(buf), wd->indicator, wd->val);
        edje_object_part_text_set(wd->slider, "elm.indicator", buf);
     }
   else
     edje_object_part_text_set(wd->slider, "elm.indicator", NULL);
}

static void 
_drag(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   _val_fetch(data);
   _units_set(data);
   _indicator_set(data);
}

static void 
_drag_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{    
   _val_fetch(data);
   _units_set(data);
   _indicator_set(data);
}

static void 
_drag_stop(void *data, Evas_Object *obj, const char *emission, const char *source)
{    
   _val_fetch(data);
   _units_set(data);
   _indicator_set(data);
}

EAPI Evas_Object *
elm_slider_add(Evas_Object *parent)
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
   
   wd->horizontal = 1;
   wd->val = 0.0;
   wd->val_min = 0.0;
   wd->val_max = 1.0;
   
   wd->slider = edje_object_add(e);
   _elm_theme_set(wd->slider, "slider", "horizontal", "default");
   elm_widget_resize_object_set(obj, wd->slider);
   edje_object_signal_callback_add(wd->slider, "drag", "*", _drag, obj);
   edje_object_signal_callback_add(wd->slider, "drag,start", "*", _drag_start, obj);
   edje_object_signal_callback_add(wd->slider, "drag,stop", "*", _drag_stop, obj);
   edje_object_signal_callback_add(wd->slider, "drag,step", "*", _drag_stop, obj);
   edje_object_signal_callback_add(wd->slider, "drag,set", "*", _drag_stop, obj);
   edje_object_part_drag_value_set(wd->slider, "elm.dragable.slider", 0.0, 0.0);
   
   wd->spacer = evas_object_rectangle_add(e);
   evas_object_color_set(wd->spacer, 0, 0, 0, 0);
   evas_object_pass_events_set(wd->spacer, 1);
   elm_widget_sub_object_add(obj, wd->spacer);
   edje_object_part_swallow(wd->slider, "elm.swallow.bar", wd->spacer);
   
   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_slider_label_set(Evas_Object *obj, const char *label)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (wd->label) eina_stringshare_del(wd->label);
   if (label)
     {
        wd->label = eina_stringshare_add(label);
	edje_object_signal_emit(wd->slider, "elm,state,text,visible", "elm");
	edje_object_message_signal_process(wd->slider);
     }
   else
     {
        wd->label = NULL;
	edje_object_signal_emit(wd->slider, "elm,state,text,hidden", "elm");
	edje_object_message_signal_process(wd->slider);
     }
   edje_object_part_text_set(wd->slider, "elm.text", label);
   _sizing_eval(obj);
}

EAPI void
elm_slider_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((wd->icon != icon) && (wd->icon))
     elm_widget_sub_object_del(obj, wd->icon);
   wd->icon = icon;
   if (icon)
     {
	elm_widget_sub_object_add(obj, icon);
	edje_object_part_swallow(wd->slider, "elm.swallow.content", icon);
	edje_object_signal_emit(wd->slider, "elm,state,icon,visible", "elm");
	evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	_sizing_eval(obj);
     }
}

EAPI void
elm_slider_span_size_set(Evas_Object *obj, Evas_Coord size)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->size == size) return;
   wd->size = size;
   if (wd->horizontal)
     evas_object_size_hint_min_set(wd->spacer, (double)wd->size * elm_widget_scale_get(obj) * _elm_config->scale, 1);
   else 
     evas_object_size_hint_min_set(wd->spacer, 1, (double)wd->size * elm_widget_scale_get(obj) * _elm_config->scale);
   edje_object_part_swallow(wd->slider, "elm.swallow.bar", wd->spacer);
   _sizing_eval(obj);
}

EAPI void
elm_slider_unit_format_set(Evas_Object *obj, const char *units)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->units) eina_stringshare_del(wd->units);
   if (units)
     {
        wd->units = eina_stringshare_add(units);
	edje_object_signal_emit(wd->slider, "elm,state,units,visible", "elm");
	edje_object_message_signal_process(wd->slider);
     }
   else
     {
        wd->units = NULL;
	edje_object_signal_emit(wd->slider, "elm,state,units,hidden", "elm");
	edje_object_message_signal_process(wd->slider);
     }
   _units_set(obj);
   _sizing_eval(obj);
}

EAPI void
elm_slider_indicator_format_set(Evas_Object *obj, const char *indicator)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->indicator) eina_stringshare_del(wd->indicator);
   if (indicator) wd->indicator = eina_stringshare_add(indicator);
   else wd->indicator = NULL;
   _indicator_set(obj);
}

EAPI void
elm_slider_horizontal_set(Evas_Object *obj, Evas_Bool horizontal)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   horizontal = !!horizontal;
   if (wd->horizontal == horizontal) return;
   wd->horizontal = horizontal;
   _theme_hook(obj);
}

EAPI void
elm_slider_min_max_set(Evas_Object *obj, double min, double max)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((wd->val_min == min) && (wd->val_max == max)) return;
   wd->val_min = min;
   wd->val_max = max;
   if (wd->val < wd->val_min) wd->val = wd->val_min;
   if (wd->val > wd->val_max) wd->val = wd->val_max;
   _val_set(obj);
   _units_set(obj);
   _indicator_set(obj);
}

EAPI void
elm_slider_value_set(Evas_Object *obj, double val)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->val == val) return;
   wd->val = val;
   if (wd->val < wd->val_min) wd->val = wd->val_min;
   if (wd->val > wd->val_max) wd->val = wd->val_max;
   _val_set(obj);
   _units_set(obj);
   _indicator_set(obj);
}

EAPI double
elm_slider_value_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->val;
}

EAPI void
elm_slider_inverted_set(Evas_Object *obj, Evas_Bool inverted)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   inverted = !!inverted;
   if (wd->inverted == inverted) return;
   wd->inverted = inverted;
   if (wd->inverted)
     edje_object_signal_emit(wd->slider, "elm,state,inverted,on", "elm");
   else
     edje_object_signal_emit(wd->slider, "elm,state,inverted,off", "elm");
   edje_object_message_signal_process(wd->slider);
   _val_set(obj);
   _units_set(obj);
   _indicator_set(obj);
}

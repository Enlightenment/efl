#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *tgl;
   Evas_Object *icon;
   Evas_Bool state;
   Evas_Bool *statep;
   const char *label;
   const char *ontext, *offtext;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _signal_toggle_off(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_toggle_on(void *data, Evas_Object *obj, const char *emission, const char *source);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->label) eina_stringshare_del(wd->label);
   if (wd->ontext) eina_stringshare_del(wd->ontext);
   if (wd->offtext) eina_stringshare_del(wd->offtext);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _elm_theme_set(wd->tgl, "toggle", "base", "default");
   if (wd->icon)
     edje_object_signal_emit(wd->tgl, "elm,state,icon,visible", "elm");
   else
     edje_object_signal_emit(wd->tgl, "elm,state,icon,hidden", "elm");
   if (wd->state)
     edje_object_signal_emit(wd->tgl, "elm,state,toggle,on", "elm");
   else
     edje_object_signal_emit(wd->tgl, "elm,state,toggle,off", "elm");
   if (wd->label)
     edje_object_signal_emit(wd->tgl, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->tgl, "elm,state,text,hidden", "elm");
   edje_object_part_text_set(wd->tgl, "elm.text", wd->label);
   edje_object_part_text_set(wd->tgl, "elm.ontext", wd->ontext);
   edje_object_part_text_set(wd->tgl, "elm.offtext", wd->offtext);
   edje_object_message_signal_process(wd->tgl);
   edje_object_scale_set(wd->tgl, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->tgl, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (obj != wd->icon) return;
   edje_object_part_swallow(wd->tgl, "elm.swallow.content", obj);
   Evas_Coord mw, mh;
   evas_object_size_hint_min_get(obj, &mw, &mh);
   _sizing_eval(data);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (sub == wd->icon)
     {
	edje_object_signal_emit(wd->tgl, "elm,state,icon,hidden", "elm");
	evas_object_event_callback_del
	  (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
	wd->icon = NULL;
	_sizing_eval(obj);
     }
}

static void
_signal_toggle_off(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->state = 0;
   if (wd->statep) *wd->statep = wd->state;
   evas_object_smart_callback_call(data, "changed", NULL);
}

static void
_signal_toggle_on(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->state = 1;
   if (wd->statep) *wd->statep = wd->state;
   evas_object_smart_callback_call(data, "changed", NULL);
}

EAPI Evas_Object *
elm_toggle_add(Evas_Object *parent)
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
   
   wd->tgl = edje_object_add(e);
   _elm_theme_set(wd->tgl, "toggle", "base", "default");
   wd->ontext = eina_stringshare_add("ON");
   wd->offtext = eina_stringshare_add("OFF");
   edje_object_signal_callback_add(wd->tgl, "elm,action,toggle,on", "", _signal_toggle_on, obj);
   edje_object_signal_callback_add(wd->tgl, "elm,action,toggle,off", "", _signal_toggle_off, obj);
   elm_widget_resize_object_set(obj, wd->tgl);
   edje_object_part_text_set(wd->tgl, "elm.ontext", wd->ontext);
   edje_object_part_text_set(wd->tgl, "elm.offtext", wd->offtext);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_toggle_label_set(Evas_Object *obj, const char *label)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord mw, mh;

   if (wd->label) eina_stringshare_del(wd->label);
   if (label)
     {
        wd->label = eina_stringshare_add(label);
	edje_object_signal_emit(wd->tgl, "elm,state,text,visible", "elm");
	edje_object_message_signal_process(wd->tgl);
     }
   else
     {
        wd->label = NULL;
	edje_object_signal_emit(wd->tgl, "elm,state,text,hidden", "elm");
	edje_object_message_signal_process(wd->tgl);
     }
   edje_object_part_text_set(wd->tgl, "elm.text", label);
   _sizing_eval(obj);
}

EAPI void
elm_toggle_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((wd->icon != icon) && (wd->icon))
     elm_widget_sub_object_del(obj, wd->icon);
   wd->icon = icon;
   if (icon)
     {
	elm_widget_sub_object_add(obj, icon);
	edje_object_part_swallow(wd->tgl, "elm.swallow.content", icon);
	edje_object_signal_emit(wd->tgl, "elm,state,icon,visible", "elm");
	evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	_sizing_eval(obj);
     }
}

EAPI void
elm_toggle_states_labels_set(Evas_Object *obj, const char *onlabel, const char *offlabel)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->ontext) eina_stringshare_del(wd->ontext);
   if (wd->offtext) eina_stringshare_del(wd->offtext);
   if (onlabel) wd->ontext = eina_stringshare_add(onlabel);
   else wd->ontext = NULL;
   if (offlabel) wd->offtext = eina_stringshare_add(offlabel);
   else wd->offtext = NULL;
   edje_object_part_text_set(wd->tgl, "elm.ontext", onlabel);
   edje_object_part_text_set(wd->tgl, "elm.offtext", offlabel);
   _sizing_eval(obj);
}

EAPI void
elm_toggle_state_set(Evas_Object *obj, Evas_Bool state)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (state != wd->state)
     {
	wd->state = state;
	if (wd->statep) *wd->statep = wd->state;
	if (wd->state)
	  edje_object_signal_emit(wd->tgl, "elm,state,toggle,on", "elm");
	else
	  edje_object_signal_emit(wd->tgl, "elm,state,toggle,off", "elm");
     }
}

EAPI Evas_Bool
elm_toggle_state_get(const Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->state;
}

EAPI void
elm_toggle_state_pointer_set(Evas_Object *obj, Evas_Bool *statep)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (statep)
     {
	wd->statep = statep;
	if (*wd->statep != wd->state)
	  {
	     wd->state = *wd->statep;
	     if (wd->state)
	       edje_object_signal_emit(wd->tgl, "elm,state,toggle,on", "elm");
	     else
	       edje_object_signal_emit(wd->tgl, "elm,state,toggle,off", "elm");
	  }
     }
   else
     {
        wd->statep = NULL;
     }
}

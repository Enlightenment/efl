#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Group Group;

struct _Group
{
   int value;
   int *valuep;
   Eina_List *radios;
};

struct _Widget_Data
{
   Evas_Object *chk;
   Evas_Object *icon;
   int value;
   const char *label;
   Evas_Bool state;
   Group *group;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _signal_radio_on(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_radio_toggle(void *data, Evas_Object *obj, const char *emission, const char *source);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->label) eina_stringshare_del(wd->label);
   wd->group->radios = eina_list_remove(wd->group->radios, obj);
   if (!wd->group->radios) free(wd->group);
   wd->group = NULL;
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _elm_theme_set(wd->chk, "radio", "base", "default");
   if (wd->icon)
     edje_object_signal_emit(wd->chk, "elm,state,icon,visible", "elm");
   else
     edje_object_signal_emit(wd->chk, "elm,state,icon,hidden", "elm");
   if (wd->state)
     edje_object_signal_emit(wd->chk, "elm,state,radio,on", "elm");
   else
     edje_object_signal_emit(wd->chk, "elm,state,radio,off", "elm");
   if (wd->label)
     edje_object_signal_emit(wd->chk, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->chk, "elm,state,text,hidden", "elm");
   edje_object_part_text_set(wd->chk, "elm.text", wd->label);
   edje_object_message_signal_process(wd->chk);
   edje_object_scale_set(wd->chk, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->chk, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (obj != wd->icon) return;
   edje_object_part_swallow(wd->chk, "elm.swallow.content", obj);
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
	edje_object_signal_emit(wd->chk, "elm,state,icon,hidden", "elm");
	evas_object_event_callback_del
	  (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
	wd->icon = NULL;
	_sizing_eval(obj);
     }
}

static void
_state_set(Evas_Object *obj, Evas_Bool state)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (state != wd->state)
     {
        wd->state = state;
        if (wd->state)
          edje_object_signal_emit(wd->chk, "elm,state,radio,on", "elm");
        else
          edje_object_signal_emit(wd->chk, "elm,state,radio,off", "elm");
     }
}

static void
_state_set_all(Widget_Data *wd)
{
   const Eina_List *l;
   Evas_Object *child;

   EINA_LIST_FOREACH(wd->group->radios, l, child)
     {
        Widget_Data *wd2 = elm_widget_data_get(child);
        if (wd2->value == wd->group->value) _state_set(child, 1);
        else _state_set(child, 0);
     }
}

static void
_signal_radio_on(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (wd->group->value == wd->value) return;
   wd->group->value = wd->value;
   if (wd->group->valuep) *(wd->group->valuep) = wd->group->value;
   _state_set_all(wd);
   evas_object_smart_callback_call(data, "changed", NULL);
}

EAPI Evas_Object *
elm_radio_add(Evas_Object *parent)
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
   
   wd->chk = edje_object_add(e);
   _elm_theme_set(wd->chk, "radio", "base", "default");
   edje_object_signal_callback_add(wd->chk, "elm,action,radio,on", "", _signal_radio_on, obj);
   edje_object_signal_callback_add(wd->chk, "elm,action,radio,toggle", "", _signal_radio_on, obj);
   elm_widget_resize_object_set(obj, wd->chk);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   wd->group = calloc(1, sizeof(Group));
   wd->group->radios = eina_list_append(wd->group->radios, obj);
   wd->state = 0;
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_radio_label_set(Evas_Object *obj, const char *label)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord mw, mh;

   if (wd->label) eina_stringshare_del(wd->label);
   if (label)
     {
        wd->label = eina_stringshare_add(label);
	edje_object_signal_emit(wd->chk, "elm,state,text,visible", "elm");
	edje_object_message_signal_process(wd->chk);
     }
   else
     {
        wd->label = NULL;
	edje_object_signal_emit(wd->chk, "elm,state,text,hidden", "elm");
	edje_object_message_signal_process(wd->chk);
     }
   edje_object_part_text_set(wd->chk, "elm.text", label);
   _sizing_eval(obj);
}

EAPI void
elm_radio_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((wd->icon != icon) && (wd->icon))
     elm_widget_sub_object_del(obj, wd->icon);
   wd->icon = icon;
   if (icon)
     {
	elm_widget_sub_object_add(obj, icon);
	edje_object_part_swallow(wd->chk, "elm.swallow.content", icon);
	edje_object_signal_emit(wd->chk, "elm,state,icon,visible", "elm");
	evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				       _changed_size_hints, obj);
	_sizing_eval(obj);
     }
}

EAPI void
elm_radio_group_add(Evas_Object *obj, Evas_Object *group)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Widget_Data *wd2 = elm_widget_data_get(group);
   Evas_Bool state = 0;
   if (wd->group == wd2->group) return;
   wd->group->radios = eina_list_remove(wd->group->radios, obj);
   if (!wd->group->radios) free(wd->group);
   wd->group = wd2->group;
   wd->group->radios = eina_list_append(wd->group->radios, obj);
   if (wd->value == wd->group->value) _state_set(obj, 1);
   else _state_set(obj, 0);
}

EAPI void
elm_radio_state_value_set(Evas_Object *obj, int value)
{
   Widget_Data *wd = elm_widget_data_get(obj);
//   if (wd->value == value) return;
   wd->value = value;
   if (wd->value == wd->group->value) _state_set(obj, 1);
   else _state_set(obj, 0);
}

EAPI void
elm_radio_value_set(Evas_Object *obj, int value)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const Eina_List *l;
   Evas_Object *child;
   if (value == wd->group->value) return;
   wd->group->value = value;
   if (wd->group->valuep) *(wd->group->valuep) = wd->group->value;
   _state_set_all(wd);
}

EAPI int
elm_radio_value_get(const Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->group->value;
}

EAPI void
elm_radio_value_pointer_set(Evas_Object *obj, int *valuep)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (valuep)
     {
        wd->group->valuep = valuep;
	if (*(wd->group->valuep) != wd->group->value)
	  {
             const Eina_List *l;
	     Evas_Object *child;

             wd->group->value = *(wd->group->valuep);
	     _state_set_all(wd);
          }
     }
   else
     {
        wd->group->valuep = NULL;
     }
}

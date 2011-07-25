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
   Evas_Object *radio;
   Evas_Object *icon;
   int value;
   const char *label;
   Eina_Bool state;
   Group *group;
};

static const char *widtype = NULL;
static void _state_set(Evas_Object *obj, Eina_Bool state);
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _signal_radio_on(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _on_focus_hook(void *data, Evas_Object *obj);
static void _activate(Evas_Object *obj);
static void _activate_hook(Evas_Object *obj);
static Eina_Bool _event_hook(Evas_Object *obj, Evas_Object *src,
                             Evas_Callback_Type type, void *event_info);

static const char SIG_CHANGED[] = "changed";
static const Evas_Smart_Cb_Description _signals[] = {
  {SIG_CHANGED, ""},
  {NULL, NULL}
};

static Eina_Bool
_event_hook(Evas_Object *obj, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   Evas_Event_Key_Down *ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   if ((strcmp(ev->keyname, "Return")) &&
       (strcmp(ev->keyname, "KP_Enter")) &&
       (strcmp(ev->keyname, "space")))
     return EINA_FALSE;
   _activate(obj);
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   wd->group->radios = eina_list_remove(wd->group->radios, obj);
   if (!wd->group->radios) free(wd->group);
   wd->group = NULL;
   free(wd);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit(wd->radio, "elm,action,focus", "elm");
        evas_object_focus_set(wd->radio, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit(wd->radio, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->radio, EINA_FALSE);
     }
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->radio, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->radio, "radio", "base", elm_widget_style_get(obj));
   if (wd->icon)
     edje_object_signal_emit(wd->radio, "elm,state,icon,visible", "elm");
   else
     edje_object_signal_emit(wd->radio, "elm,state,icon,hidden", "elm");
   if (wd->state)
     edje_object_signal_emit(wd->radio, "elm,state,radio,on", "elm");
   else
     edje_object_signal_emit(wd->radio, "elm,state,radio,off", "elm");
   if (wd->label)
     edje_object_signal_emit(wd->radio, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->radio, "elm,state,text,hidden", "elm");
   edje_object_part_text_set(wd->radio, "elm.text", wd->label);
   if (elm_widget_disabled_get(obj))
     {
        edje_object_signal_emit(wd->radio, "elm,state,disabled", "elm");
        if (wd->state) _state_set(obj, 0);
     }
   edje_object_message_signal_process(wd->radio);
   edje_object_scale_set(wd->radio, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_disabled_get(obj))
     {
        edje_object_signal_emit(wd->radio, "elm,state,disabled", "elm");
        if (wd->state) _state_set(obj, 0);
     }
   else
     edje_object_signal_emit(wd->radio, "elm,state,enabled", "elm");
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->radio, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (obj != wd->icon) return;
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (!wd) return;
   if (sub == wd->icon)
     {
        edje_object_signal_emit(wd->radio, "elm,state,icon,hidden", "elm");
        evas_object_event_callback_del_full
           (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, obj);
        wd->icon = NULL;
        _sizing_eval(obj);
     }
}

static void
_state_set(Evas_Object *obj, Eina_Bool state)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if ((state != wd->state) && (!elm_widget_disabled_get(obj)))
     {
        wd->state = state;
        if (wd->state)
          edje_object_signal_emit(wd->radio, "elm,state,radio,on", "elm");
        else
          edje_object_signal_emit(wd->radio, "elm,state,radio,off", "elm");
     }
}

static void
_state_set_all(Widget_Data *wd)
{
   const Eina_List *l;
   Evas_Object *child, *selected = NULL;
   Eina_Bool disabled = EINA_FALSE;
   EINA_LIST_FOREACH(wd->group->radios, l, child)
     {
        Widget_Data *wd2 = elm_widget_data_get(child);
        if (wd2->state) selected = child;
        if (wd2->value == wd->group->value)
          {
             _state_set(child, 1);
             if (!wd2->state) disabled = EINA_TRUE;
          }
        else _state_set(child, 0);
     }
   if ((disabled) && (selected)) _state_set(selected, 1);
}

static void
_activate(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->group->value == wd->value) return;
   wd->group->value = wd->value;
   if (wd->group->valuep) *(wd->group->valuep) = wd->group->value;
   _state_set_all(wd);
   evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
}

static void
_activate_hook(Evas_Object *obj)
{
   _activate(obj);
}

static void
_signal_radio_on(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _activate(data);
}

static void
_elm_radio_label_set(Evas_Object *obj, const char *item, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item && strcmp(item, "default")) return;
   if (!wd) return;
   eina_stringshare_replace(&wd->label, label);
   if (label)
     {
        edje_object_signal_emit(wd->radio, "elm,state,text,visible", "elm");
        edje_object_message_signal_process(wd->radio);
     }
   else
     {
        edje_object_signal_emit(wd->radio, "elm,state,text,hidden", "elm");
        edje_object_message_signal_process(wd->radio);
     }
   edje_object_part_text_set(wd->radio, "elm.text", label);
   _sizing_eval(obj);
}

static const char *
_elm_radio_label_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item && strcmp(item, "default")) return NULL;
   if (!wd) return NULL;
   return wd->label;
}

EAPI Evas_Object *
elm_radio_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "radio");
   elm_widget_type_set(obj, "radio");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_activate_hook_set(obj, _activate_hook);
   elm_widget_event_hook_set(obj, _event_hook);
   elm_widget_text_set_hook_set(obj, _elm_radio_label_set);
   elm_widget_text_get_hook_set(obj, _elm_radio_label_get);

   wd->radio = edje_object_add(e);
   _elm_theme_object_set(obj, wd->radio, "radio", "base", "default");
   edje_object_signal_callback_add(wd->radio, "elm,action,radio,on", "", _signal_radio_on, obj);
   edje_object_signal_callback_add(wd->radio, "elm,action,radio,toggle", "", _signal_radio_on, obj);
   elm_widget_resize_object_set(obj, wd->radio);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   wd->group = calloc(1, sizeof(Group));
   wd->group->radios = eina_list_append(wd->group->radios, obj);
   wd->state = 0;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);

   // TODO: convert Elementary to subclassing of Evas_Smart_Class
   // TODO: and save some bytes, making descriptions per-class and not instance!
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

EAPI void
elm_radio_label_set(Evas_Object *obj, const char *label)
{
   _elm_radio_label_set(obj, NULL, label);
}

EAPI const char *
elm_radio_label_get(const Evas_Object *obj)
{
   return _elm_radio_label_get(obj, NULL);
}

EAPI void
elm_radio_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->icon == icon) return;
   if (wd->icon) evas_object_del(wd->icon);
   wd->icon = icon;
   if (icon)
     {
        elm_widget_sub_object_add(obj, icon);
        evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        edje_object_part_swallow(wd->radio, "elm.swallow.content", icon);
        edje_object_signal_emit(wd->radio, "elm,state,icon,visible", "elm");
        edje_object_message_signal_process(wd->radio);
     }
   _sizing_eval(obj);
}

EAPI Evas_Object *
elm_radio_icon_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->icon;
}

EAPI Evas_Object *
elm_radio_icon_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->icon) return NULL;
   Evas_Object *icon = wd->icon;
   elm_widget_sub_object_del(obj, wd->icon);
   edje_object_part_unswallow(wd->radio, wd->icon);
   wd->icon = NULL;
   return icon;
}

EAPI void
elm_radio_group_add(Evas_Object *obj, Evas_Object *group)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Widget_Data *wd2 = elm_widget_data_get(group);
   if (!wd) return;
   if (!wd2)
     {
        if (eina_list_count(wd->group->radios) == 1)
          return;
        wd->group->radios = eina_list_remove(wd->group->radios, obj);
        wd->group = calloc(1, sizeof(Group));
        wd->group->radios = eina_list_append(wd->group->radios, obj);
     }
   else if (wd->group == wd2->group) return;
   else
     {
        wd->group->radios = eina_list_remove(wd->group->radios, obj);
        if (!wd->group->radios) free(wd->group);
        wd->group = wd2->group;
        wd->group->radios = eina_list_append(wd->group->radios, obj);
     }
   if (wd->value == wd->group->value) _state_set(obj, 1);
   else _state_set(obj, 0);
}

EAPI void
elm_radio_state_value_set(Evas_Object *obj, int value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->value = value;
   if (wd->value == wd->group->value) _state_set(obj, 1);
   else _state_set(obj, 0);
}

EAPI int
elm_radio_state_value_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->value;
}

EAPI void
elm_radio_value_set(Evas_Object *obj, int value)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (value == wd->group->value) return;
   wd->group->value = value;
   if (wd->group->valuep) *(wd->group->valuep) = wd->group->value;
   _state_set_all(wd);
}

EAPI int
elm_radio_value_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->group->value;
}

EAPI void
elm_radio_value_pointer_set(Evas_Object *obj, int *valuep)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (valuep)
     {
        wd->group->valuep = valuep;
        if (*(wd->group->valuep) != wd->group->value)
          {
             wd->group->value = *(wd->group->valuep);
             _state_set_all(wd);
          }
     }
   else
     {
        wd->group->valuep = NULL;
     }
}

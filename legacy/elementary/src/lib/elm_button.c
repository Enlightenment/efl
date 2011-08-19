#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *btn, *icon;
   const char *label;
   Eina_Bool autorepeat;
   Eina_Bool repeating;
   double ar_threshold;
   double ar_interval;
   Ecore_Timer *timer;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _signal_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_pressed(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_unpressed(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _on_focus_hook(void *data, Evas_Object *obj);
static void _activate(Evas_Object *obj);
static void _activate_hook(Evas_Object *obj);
static Eina_Bool _event_hook(Evas_Object *obj, Evas_Object *src,
                             Evas_Callback_Type type, void *event_info);

static const char SIG_CLICKED[] = "clicked";
static const char SIG_REPEATED[] = "repeated";
static const char SIG_PRESSED[] = "pressed";
static const char SIG_UNPRESSED[] = "unpressed";
static const Evas_Smart_Cb_Description _signals[] = {
       {SIG_CLICKED, ""},
       {SIG_REPEATED, ""},
       {SIG_PRESSED, ""},
       {SIG_UNPRESSED, ""},
       {NULL, NULL}
};

static Eina_Bool
_event_hook(Evas_Object *obj, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   Evas_Event_Key_Down *ev = event_info;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if ((strcmp(ev->keyname, "Return")) &&
       (strcmp(ev->keyname, "KP_Enter")) &&
       (strcmp(ev->keyname, "space")))
     return EINA_FALSE;
   _activate(obj);
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   edje_object_signal_emit(wd->btn, "elm,anim,activate", "elm");
   return EINA_TRUE;
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   free(wd);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit(wd->btn, "elm,action,focus", "elm");
        evas_object_focus_set(wd->btn, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit(wd->btn, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->btn, EINA_FALSE);
     }
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->btn, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *str;
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->btn, "button", "base", elm_widget_style_get(obj));
   if (wd->icon)
     edje_object_part_swallow(wd->btn, "elm.swallow.content", wd->icon);
   if (wd->label)
     edje_object_signal_emit(wd->btn, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->btn, "elm,state,text,hidden", "elm");
   if (wd->icon)
     edje_object_signal_emit(wd->btn, "elm,state,icon,visible", "elm");
   else
     edje_object_signal_emit(wd->btn, "elm,state,icon,hidden", "elm");
   edje_object_part_text_set(wd->btn, "elm.text", wd->label);
   if (elm_object_disabled_get(obj))
     edje_object_signal_emit(wd->btn, "elm,state,disabled", "elm");
   edje_object_message_signal_process(wd->btn);
   edje_object_scale_set(wd->btn, elm_widget_scale_get(obj) * _elm_config->scale);
   str = edje_object_data_get(wd->btn, "focus_highlight");
   if ((str) && (!strcmp(str, "on")))
     elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
   else
     elm_widget_highlight_in_theme_set(obj, EINA_FALSE);
   _sizing_eval(obj);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(wd->btn, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(wd->btn, "elm,state,enabled", "elm");
}

static void
_signal_emit_hook(Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_emit(wd->btn, emission, source);
}

static void
_signal_callback_add_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_callback_add(wd->btn, emission, source, func_cb, data);
}

static void
_signal_callback_del_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_signal_callback_del_full(wd->btn, emission, source, func_cb,
                                        data);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;

   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->btn, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
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
        edje_object_signal_emit(wd->btn, "elm,state,icon,hidden", "elm");
        evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
        wd->icon = NULL;
        edje_object_message_signal_process(wd->btn);
        _sizing_eval(obj);
     }
}

static void
_activate(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->timer)
     {
        ecore_timer_del(wd->timer);
        wd->timer = NULL;
     }
   wd->repeating = EINA_FALSE;
   if ((_elm_config->access_mode == ELM_ACCESS_MODE_OFF) ||
       (_elm_access_2nd_click_timeout(obj)))
     {
        if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
           _elm_access_say(E_("Clicked"));
        evas_object_smart_callback_call(obj, SIG_CLICKED, NULL);
     }
}

static void
_activate_hook(Evas_Object *obj)
{
   _activate(obj);
}

static void
_signal_clicked(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _activate(data);
}

static Eina_Bool
_autorepeat_send(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return ECORE_CALLBACK_CANCEL;

   evas_object_smart_callback_call(data, SIG_REPEATED, NULL);
   if (!wd->repeating)
     {
        wd->timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_autorepeat_initial_send(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return ECORE_CALLBACK_CANCEL;

   if (wd->timer) ecore_timer_del(wd->timer);
   wd->repeating = EINA_TRUE;
   _autorepeat_send(data);
   wd->timer = ecore_timer_add(wd->ar_interval, _autorepeat_send, data);

   return ECORE_CALLBACK_CANCEL;
}

static void
_signal_pressed(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   if ((wd->autorepeat) && (!wd->repeating))
     {
        if (wd->ar_threshold <= 0.0)
          _autorepeat_initial_send(data); /* call immediately */
        else
          wd->timer = ecore_timer_add(wd->ar_threshold, _autorepeat_initial_send, data);
     }

   evas_object_smart_callback_call(data, SIG_PRESSED, NULL);
}

static void
_signal_unpressed(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   if (wd->timer)
     {
        ecore_timer_del(wd->timer);
        wd->timer = NULL;
     }
   wd->repeating = EINA_FALSE;
   evas_object_smart_callback_call(data, SIG_UNPRESSED, NULL);
}

static void
_elm_button_label_set(Evas_Object *obj, const char *item, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item && strcmp(item, "default")) return;
   if (!wd) return;
   eina_stringshare_replace(&wd->label, label);
   if (label)
     edje_object_signal_emit(wd->btn, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(wd->btn, "elm,state,text,hidden", "elm");
   edje_object_message_signal_process(wd->btn);
   edje_object_part_text_set(wd->btn, "elm.text", label);
   _sizing_eval(obj);
}

static const char *
_elm_button_label_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item && strcmp(item, "default")) return NULL;
   if (!wd) return NULL;
   return wd->label;
}

static char *
_access_info_cb(const void *data __UNUSED__, Evas_Object *obj, Elm_Widget_Item *item __UNUSED__)
{
   char *txt = (char *)elm_widget_access_info_get(obj);
   if (!txt) txt = (char *)_elm_button_label_get(obj, NULL);
   if (txt) return strdup(txt);
   return txt;
}

static char *
_access_state_cb(const void *data __UNUSED__, Evas_Object *obj, Elm_Widget_Item *item __UNUSED__)
{
   if (elm_widget_disabled_get(obj))
      return strdup(E_("State: Disabled"));
   return NULL;
}

EAPI Evas_Object *
elm_button_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "button");
   elm_widget_type_set(obj, "button");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_activate_hook_set(obj, _activate_hook);
   elm_widget_event_hook_set(obj, _event_hook);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_signal_callback_add_hook_set(obj, _signal_callback_add_hook);
   elm_widget_signal_callback_del_hook_set(obj, _signal_callback_del_hook);
   elm_widget_text_set_hook_set(obj, _elm_button_label_set);
   elm_widget_text_get_hook_set(obj, _elm_button_label_get);

   wd->btn = edje_object_add(e);
   _elm_theme_object_set(obj, wd->btn, "button", "base", "default");
   edje_object_signal_callback_add(wd->btn, "elm,action,click", "",
                                   _signal_clicked, obj);
   edje_object_signal_callback_add(wd->btn, "elm,action,press", "",
                                   _signal_pressed, obj);
   edje_object_signal_callback_add(wd->btn, "elm,action,unpress", "",
                                   _signal_unpressed, obj);
   elm_widget_resize_object_set(obj, wd->btn);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _theme_hook(obj);

   // TODO: convert Elementary to subclassing of Evas_Smart_Class
   // TODO: and save some bytes, making descriptions per-class and not instance!
   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   _elm_access_object_register(obj, wd->btn);
   _elm_access_text_set(_elm_access_object_get(obj),
                        ELM_ACCESS_TYPE, E_("Button"));
   _elm_access_callback_set(_elm_access_object_get(obj),
                            ELM_ACCESS_INFO, _access_info_cb, obj);
   _elm_access_callback_set(_elm_access_object_get(obj),
                            ELM_ACCESS_STATE, _access_state_cb, obj);
   return obj;
}

EAPI void
elm_button_label_set(Evas_Object *obj, const char *label)
{
   _elm_button_label_set(obj, NULL, label);
}

EAPI const char *
elm_button_label_get(const Evas_Object *obj)
{
   return _elm_button_label_get(obj, NULL);
}

EAPI void
elm_button_icon_set(Evas_Object *obj, Evas_Object *icon)
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
        edje_object_part_swallow(wd->btn, "elm.swallow.content", icon);
        edje_object_signal_emit(wd->btn, "elm,state,icon,visible", "elm");
        edje_object_message_signal_process(wd->btn);
     }
   _sizing_eval(obj);
}

EAPI Evas_Object *
elm_button_icon_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->icon;
}

EAPI Evas_Object *
elm_button_icon_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->icon) return NULL;
   Evas_Object *icon = wd->icon;
   elm_widget_sub_object_del(obj, wd->icon);
   edje_object_part_unswallow(wd->btn, wd->icon);
   wd->icon = NULL;
   return icon;
}

EAPI void
elm_button_autorepeat_set(Evas_Object *obj, Eina_Bool on)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->timer)
     {
        ecore_timer_del(wd->timer);
        wd->timer = NULL;
     }
   wd->autorepeat = on;
   wd->repeating = EINA_FALSE;
}

EAPI Eina_Bool
elm_button_autorepeat_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->autorepeat;
}

EAPI void
elm_button_autorepeat_initial_timeout_set(Evas_Object *obj, double t)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->ar_threshold == t) return;
   if (wd->timer)
     {
        ecore_timer_del(wd->timer);
        wd->timer = NULL;
     }
   wd->ar_threshold = t;
}

EAPI double
elm_button_autorepeat_initial_timeout_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0.0;
   return wd->ar_threshold;
}

EAPI void
elm_button_autorepeat_gap_timeout_set(Evas_Object *obj, double t)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->ar_interval == t) return;

   wd->ar_interval = t;
   if ((wd->repeating) && (wd->timer)) ecore_timer_interval_set(wd->timer, t);
}

EAPI double
elm_button_autorepeat_gap_timeout_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0.0;
   return wd->ar_interval;
}

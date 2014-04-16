#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_button.h"
#include "elm_widget_layout.h"

#define MY_CLASS ELM_OBJ_BUTTON_CLASS

#define MY_CLASS_NAME "Elm_Button"
#define MY_CLASS_NAME_LEGACY "elm_button"

static const char SIG_CLICKED[] = "clicked";
static const char SIG_REPEATED[] = "repeated";
static const char SIG_PRESSED[] = "pressed";
static const char SIG_UNPRESSED[] = "unpressed";

/* smart callbacks coming from elm button objects (besides the ones
 * coming from elm layout): */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_REPEATED, ""},
   {SIG_PRESSED, ""},
   {SIG_UNPRESSED, ""},
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"icon", "elm.swallow.content"},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {NULL, NULL}
};

static Eina_Bool _key_action_activate(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"activate", _key_action_activate},
   {NULL, NULL}
};

static void
_activate(Evas_Object *obj)
{
   ELM_BUTTON_DATA_GET_OR_RETURN(obj, sd);

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   sd->repeating = EINA_FALSE;

   if ((_elm_config->access_mode == ELM_ACCESS_MODE_OFF) ||
       (_elm_access_2nd_click_timeout(obj)))
     {
        if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
          _elm_access_say(E_("Clicked"));
        if (!elm_widget_disabled_get(obj) &&
            !evas_object_freeze_events_get(obj))
          evas_object_smart_callback_call(obj, SIG_CLICKED, NULL);
     }
}

EOLIAN static void
_elm_button_elm_layout_sizing_eval(Eo *obj, Elm_Button_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
}

EOLIAN static Eina_Bool
_elm_button_elm_widget_activate(Eo *obj, Elm_Button_Data *_pd EINA_UNUSED, Elm_Activate act)
{
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (act != ELM_ACTIVATE_DEFAULT) return EINA_FALSE;
   if (evas_object_freeze_events_get(obj)) return EINA_FALSE;

   evas_object_smart_callback_call(obj, SIG_CLICKED, NULL);
   elm_layout_signal_emit(obj, "elm,anim,activate", "elm");

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
_icon_signal_emit(Evas_Object *obj)
{
   char buf[64];

   snprintf(buf, sizeof(buf), "elm,state,icon,%s",
            elm_layout_content_get(obj, "icon") ? "visible" : "hidden");

   elm_layout_signal_emit(obj, buf, "elm");
   edje_object_message_signal_process(elm_layout_edje_get(obj));
   eo_do(obj, elm_obj_layout_sizing_eval());
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Eina_Bool
_elm_button_elm_widget_theme_apply(Eo *obj, Elm_Button_Data *_pd EINA_UNUSED)
{
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;
   _icon_signal_emit(obj);

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Eina_Bool
_elm_button_elm_widget_sub_object_del(Eo *obj, Elm_Button_Data *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_sub_object_del(sobj));
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Eina_Bool
_elm_button_elm_container_content_set(Eo *obj, Elm_Button_Data *_pd EINA_UNUSED, const char *part, Evas_Object *content)
{
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_container_content_set(part, content));
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   elm_layout_signal_emit(obj, "elm,anim,activate", "elm");
   _activate(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_button_elm_widget_event(Eo *obj, Elm_Button_Data *_pd EINA_UNUSED, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   (void) src;
   Evas_Event_Key_Down *ev = event_info;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_on_clicked_signal(void *data,
                   Evas_Object *obj EINA_UNUSED,
                   const char *emission EINA_UNUSED,
                   const char *source EINA_UNUSED)
{
   _activate(data);
}

static Eina_Bool
_autorepeat_send(void *data)
{
   ELM_BUTTON_DATA_GET_OR_RETURN_VAL(data, sd, ECORE_CALLBACK_CANCEL);

   evas_object_smart_callback_call(data, SIG_REPEATED, NULL);
   if (!sd->repeating)
     {
        sd->timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_autorepeat_initial_send(void *data)
{
   ELM_BUTTON_DATA_GET_OR_RETURN_VAL(data, sd, ECORE_CALLBACK_CANCEL);

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   sd->repeating = EINA_TRUE;
   _autorepeat_send(data);
   sd->timer = ecore_timer_add(sd->ar_interval, _autorepeat_send, data);

   return ECORE_CALLBACK_CANCEL;
}

static void
_on_pressed_signal(void *data,
                   Evas_Object *obj EINA_UNUSED,
                   const char *emission EINA_UNUSED,
                   const char *source EINA_UNUSED)
{
   ELM_BUTTON_DATA_GET_OR_RETURN(data, sd);

   if ((sd->autorepeat) && (!sd->repeating))
     {
        if (sd->ar_threshold <= 0.0)
          _autorepeat_initial_send(data);  /* call immediately */
        else
          sd->timer = ecore_timer_add
              (sd->ar_threshold, _autorepeat_initial_send, data);
     }

   evas_object_smart_callback_call(data, SIG_PRESSED, NULL);
}

static void
_on_unpressed_signal(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     const char *emission EINA_UNUSED,
                     const char *source EINA_UNUSED)
{
   ELM_BUTTON_DATA_GET_OR_RETURN(data, sd);

   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   sd->repeating = EINA_FALSE;
   evas_object_smart_callback_call(data, SIG_UNPRESSED, NULL);
}

static char *
_access_info_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   const char *txt = elm_widget_access_info_get(obj);

   if (!txt) txt = elm_layout_text_get(obj, NULL);
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   if (elm_widget_disabled_get(obj))
     return strdup(E_("State: Disabled"));

   return NULL;
}

EOLIAN static void
_elm_button_evas_smart_add(Eo *obj, Elm_Button_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,click", "*",
     _on_clicked_signal, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,press", "*",
     _on_pressed_signal, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,unpress", "*",
     _on_unpressed_signal, obj);

   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("Button"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);
   if (!elm_layout_theme_set(obj, "button", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_button_elm_layout_content_aliases_get(Eo *obj EINA_UNUSED, Elm_Button_Data *_pd EINA_UNUSED)
{
   return _content_aliases;
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_button_elm_layout_text_aliases_get(Eo *obj EINA_UNUSED, Elm_Button_Data *_pd EINA_UNUSED)
{
   return _text_aliases;
}

EAPI Evas_Object *
elm_button_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_button_eo_base_constructor(Eo *obj, Elm_Button_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks));
}

EOLIAN static void
_elm_button_autorepeat_set(Eo *obj EINA_UNUSED, Elm_Button_Data *sd, Eina_Bool on)
{
   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   sd->autorepeat = on;
   sd->repeating = EINA_FALSE;
}

#define _AR_CAPABLE(obj) \
  (_internal_elm_button_admits_autorepeat_get(obj))

static Eina_Bool
_internal_elm_button_admits_autorepeat_get(const Evas_Object *obj)
{
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, ret = elm_obj_button_admits_autorepeat_get());
   return ret;
}

EOLIAN static Eina_Bool
_elm_button_admits_autorepeat_get(Eo *obj EINA_UNUSED, Elm_Button_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_button_autorepeat_get(Eo *obj, Elm_Button_Data *sd)
{
   return (_AR_CAPABLE(obj) & sd->autorepeat);
}

EOLIAN static void
_elm_button_autorepeat_initial_timeout_set(Eo *obj, Elm_Button_Data *sd, double t)
{
   if (!_AR_CAPABLE(obj))
     {
        ERR("this widget does not support auto repetition of clicks.");
        return;
     }

   if (sd->ar_threshold == t) return;
   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   sd->ar_threshold = t;
}

EOLIAN static double
_elm_button_autorepeat_initial_timeout_get(Eo *obj, Elm_Button_Data *sd)
{
   if (!_AR_CAPABLE(obj))
      return 0.0;
   else
      return sd->ar_threshold;
}

EOLIAN static void
_elm_button_autorepeat_gap_timeout_set(Eo *obj, Elm_Button_Data *sd, double t)
{
   if (!_AR_CAPABLE(obj))
     {
        ERR("this widget does not support auto repetition of clicks.");
        return;
     }

   if (sd->ar_interval == t) return;

   sd->ar_interval = t;
   if ((sd->repeating) && (sd->timer)) ecore_timer_interval_set(sd->timer, t);
}

EOLIAN static double
_elm_button_autorepeat_gap_timeout_get(Eo *obj EINA_UNUSED, Elm_Button_Data *sd)
{
   return sd->ar_interval;
}

EOLIAN static Eina_Bool
_elm_button_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Button_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_button_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Button_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

static void
_elm_button_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_button.eo.c"

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define EFL_ACCESS_OBJECT_PROTECTED
#define ELM_LAYOUT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_button_private.h"
#include "elm_widget_layout.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_BUTTON_CLASS
#define MY_CLASS_PFX efl_ui_button

#define MY_CLASS_NAME "Efl.Ui.Button"

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

static Eina_Bool _key_action_activate(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"activate", _key_action_activate},
   {NULL, NULL}
};

#define MY_CLASS_NAME_LEGACY "elm_button"

static void
_efl_ui_button_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

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
          efl_event_callback_legacy_call
            (obj, EFL_UI_EVENT_CLICKED, NULL);
     }
}

EOLIAN static void
_efl_ui_button_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Button_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
}

EOLIAN static Eina_Bool
_efl_ui_button_efl_ui_widget_on_access_activate(Eo *obj, Efl_Ui_Button_Data *_pd EINA_UNUSED, Efl_Ui_Activate act)
{
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (act != EFL_UI_ACTIVATE_DEFAULT) return EINA_FALSE;
   if (evas_object_freeze_events_get(obj)) return EINA_FALSE;

   efl_event_callback_legacy_call
      (obj, EFL_UI_EVENT_CLICKED, NULL);

   if (elm_widget_is_legacy(obj))
     elm_layout_signal_emit(obj, "elm,anim,activate", "elm");
   else
     elm_layout_signal_emit(obj, "efl,anim,activate", "efl");

   return EINA_TRUE;
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   if (elm_widget_is_legacy(obj))
     elm_layout_signal_emit(obj, "elm,anim,activate", "elm");
   else
     elm_layout_signal_emit(obj, "efl,anim,activate", "efl");
   _activate(obj);
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

   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_REPEATED, NULL);
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
   sd->timer = ecore_timer_add(sd->ar_gap_timeout, _autorepeat_send, data);

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
        if (sd->ar_initial_timeout <= 0.0)
          _autorepeat_initial_send(data);  /* call immediately */
        else
          sd->timer = ecore_timer_add
              (sd->ar_initial_timeout, _autorepeat_initial_send, data);
     }

   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_PRESSED, NULL);
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
   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_UNPRESSED, NULL);
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
_efl_ui_button_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Button_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "button");
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   if (elm_widget_is_legacy(obj))
     {
        edje_object_signal_callback_add
           (wd->resize_obj, "elm,action,click", "*",
            _on_clicked_signal, obj);
        edje_object_signal_callback_add
           (wd->resize_obj, "elm,action,press", "*",
            _on_pressed_signal, obj);
        edje_object_signal_callback_add
           (wd->resize_obj, "elm,action,unpress", "*",
            _on_unpressed_signal, obj);
     }
   else
     {
        edje_object_signal_callback_add
           (wd->resize_obj, "efl,action,click", "*",
            _on_clicked_signal, obj);
        edje_object_signal_callback_add
           (wd->resize_obj, "efl,action,press", "*",
            _on_pressed_signal, obj);
        edje_object_signal_callback_add
           (wd->resize_obj, "efl,action,unpress", "*",
            _on_unpressed_signal, obj);
     }

   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("Button"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   if (!elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)))
     CRI("Failed to set layout!");
}

EOLIAN static Eo *
_efl_ui_button_efl_object_constructor(Eo *obj, Efl_Ui_Button_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_PUSH_BUTTON);

   return obj;
}

EOLIAN static void
_efl_ui_button_efl_ui_autorepeat_autorepeat_enabled_set(Eo *obj EINA_UNUSED, Efl_Ui_Button_Data *sd, Eina_Bool on)
{
   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   sd->autorepeat = on;
   sd->repeating = EINA_FALSE;
}

#define _AR_CAPABLE(obj) \
  (_internal_efl_ui_button_autorepeat_supported_get(obj))

static Eina_Bool
_internal_efl_ui_button_autorepeat_supported_get(const Evas_Object *obj)
{
   Eina_Bool ret = EINA_FALSE;
   ret = efl_ui_autorepeat_supported_get(obj);
   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_button_efl_ui_autorepeat_autorepeat_supported_get(const Eo *obj EINA_UNUSED, Efl_Ui_Button_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_button_efl_ui_autorepeat_autorepeat_enabled_get(const Eo *obj, Efl_Ui_Button_Data *sd)
{
   return (_AR_CAPABLE(obj) & sd->autorepeat);
}

EOLIAN static void
_efl_ui_button_efl_ui_autorepeat_autorepeat_initial_timeout_set(Eo *obj, Efl_Ui_Button_Data *sd, double t)
{
   if (!_AR_CAPABLE(obj))
     {
        ERR("this widget does not support auto repetition of clicks.");
        return;
     }

   if (EINA_DBL_EQ(sd->ar_initial_timeout, t)) return;
   ELM_SAFE_FREE(sd->timer, ecore_timer_del);
   sd->ar_initial_timeout = t;
}

EOLIAN static double
_efl_ui_button_efl_ui_autorepeat_autorepeat_initial_timeout_get(const Eo *obj, Efl_Ui_Button_Data *sd)
{
   if (!_AR_CAPABLE(obj))
      return 0.0;
   else
      return sd->ar_initial_timeout;
}

EOLIAN static void
_efl_ui_button_efl_ui_autorepeat_autorepeat_gap_timeout_set(Eo *obj, Efl_Ui_Button_Data *sd, double t)
{
   if (!_AR_CAPABLE(obj))
     {
        ERR("this widget does not support auto repetition of clicks.");
        return;
     }

   if (EINA_DBL_EQ(sd->ar_gap_timeout, t)) return;

   sd->ar_gap_timeout = t;
   if ((sd->repeating) && (sd->timer)) ecore_timer_interval_set(sd->timer, t);
}

EOLIAN static double
_efl_ui_button_efl_ui_autorepeat_autorepeat_gap_timeout_get(const Eo *obj EINA_UNUSED, Efl_Ui_Button_Data *sd)
{
   return sd->ar_gap_timeout;
}

EOLIAN const Efl_Access_Action_Data *
_efl_ui_button_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Efl_Ui_Button_Data *pd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "activate", "activate", NULL, _key_action_activate },
          { NULL, NULL, NULL, NULL}
   };
   return &atspi_actions[0];
}

/* Standard widget overrides */

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(efl_ui_button, Efl_Ui_Button_Data)
ELM_PART_TEXT_DEFAULT_IMPLEMENT(efl_ui_button, Efl_Ui_Button_Data)
ELM_PART_CONTENT_DEFAULT_IMPLEMENT(efl_ui_button, Efl_Ui_Button_Data)

EAPI void
elm_button_autorepeat_initial_timeout_set(Evas_Object *obj, double t)
{
   efl_ui_autorepeat_initial_timeout_set(obj, t);
}

EAPI double
elm_button_autorepeat_initial_timeout_get(const Evas_Object *obj)
{
   return efl_ui_autorepeat_initial_timeout_get(obj);
}

EAPI void
elm_button_autorepeat_gap_timeout_set(Evas_Object *obj, double t)
{
   efl_ui_autorepeat_gap_timeout_set(obj, t);
}

EAPI double
elm_button_autorepeat_gap_timeout_get(const Evas_Object *obj)
{
   return efl_ui_autorepeat_gap_timeout_get(obj);
}

EAPI void
elm_button_autorepeat_set(Evas_Object *obj, Eina_Bool on)
{
   efl_ui_autorepeat_enabled_set(obj, on);
}

EAPI Eina_Bool
elm_button_autorepeat_get(const Evas_Object *obj)
{
   return efl_ui_autorepeat_enabled_get(obj);
}

/* Internal EO APIs and hidden overrides */

ELM_LAYOUT_CONTENT_ALIASES_IMPLEMENT(MY_CLASS_PFX)

#define EFL_UI_BUTTON_EXTRA_OPS \
   ELM_LAYOUT_CONTENT_ALIASES_OPS(MY_CLASS_PFX), \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_button), \
   EFL_CANVAS_GROUP_ADD_OPS(efl_ui_button)

#include "efl_ui_button.eo.c"

#include "efl_ui_button_legacy.eo.h"
#include "efl_ui_button_legacy_part.eo.h"

EOLIAN static Eo *
_efl_ui_button_legacy_efl_object_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, EFL_UI_BUTTON_LEGACY_CLASS));
   legacy_object_focus_handle(obj);
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   return obj;
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
_icon_signal_emit(Evas_Object *obj)
{
   char buf[64];

   if (!elm_widget_resize_object_get(obj)) return;
   snprintf(buf, sizeof(buf), "elm,state,icon,%s",
            elm_layout_content_get(obj, "icon") ? "visible" : "hidden");

   elm_layout_signal_emit(obj, buf, "elm");
   edje_object_message_signal_process(elm_layout_edje_get(obj));
   elm_layout_sizing_eval(obj);
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Efl_Ui_Theme_Apply
_efl_ui_button_legacy_efl_ui_widget_theme_apply(Eo *obj, void *_pd EINA_UNUSED)
{
   Efl_Ui_Theme_Apply int_ret = EFL_UI_THEME_APPLY_FAILED;

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, EFL_UI_BUTTON_LEGACY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_FAILED;
   _icon_signal_emit(obj);

   return int_ret;
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Eina_Bool
_efl_ui_button_legacy_efl_ui_widget_widget_sub_object_del(Eo *obj, void *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = elm_widget_sub_object_del(efl_super(obj, EFL_UI_BUTTON_LEGACY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because button's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static Eina_Bool
_efl_ui_button_legacy_content_set(Eo *obj, void *_pd EINA_UNUSED, const char *part, Evas_Object *content)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_content_set(efl_part(efl_super(obj, EFL_UI_BUTTON_LEGACY_CLASS), part), content);
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

/* Efl.Part begin */

static Eina_Bool
_part_is_efl_ui_button_legacy_part(const Eo *obj EINA_UNUSED, const char *part)
{
   return eina_streq(part, "elm.swallow.content");
}

ELM_PART_OVERRIDE_PARTIAL(efl_ui_button_legacy, EFL_UI_BUTTON_LEGACY, void, _part_is_efl_ui_button_legacy_part)
ELM_PART_OVERRIDE_CONTENT_SET_NO_SD(efl_ui_button_legacy)
#include "efl_ui_button_legacy_part.eo.c"

/* Efl.Part end */

EAPI Evas_Object *
elm_button_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(EFL_UI_BUTTON_LEGACY_CLASS, parent);
}

#include "efl_ui_button_legacy.eo.c"

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_check.h"
#include "elm_widget_layout.h"

 
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED
#include "elm_interface_atspi_widget_action.h"
#include "elm_interface_atspi_widget_action.eo.h"

#define MY_CLASS ELM_CHECK_CLASS

#define MY_CLASS_NAME "Elm_Check"
#define MY_CLASS_NAME_LEGACY "elm_check"

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"icon", "elm.swallow.content"},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {"on", "elm.ontext"},
   {"off", "elm.offtext"},
   {NULL, NULL}
};

static const char SIG_CHANGED[] = "changed";

/* smart callbacks coming from elm check objects: */
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
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
   ELM_CHECK_DATA_GET(obj, sd);

   sd->state = !sd->state;
   if (sd->statep) *sd->statep = sd->state;
   if (sd->state)
     {
        elm_layout_signal_emit(obj, "elm,state,check,on", "elm");
        if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
             _elm_access_say(E_("State: On"));
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
        if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
             _elm_access_say(E_("State: Off"));
     }

   evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);

   if (_elm_config->atspi_mode)
       elm_interface_atspi_accessible_state_changed_signal_emit(obj,
                                                                ELM_ATSPI_STATE_CHECKED,
                                                                sd->state);
}

/* FIXME: replicated from elm_layout just because check's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
_icon_signal_emit(Evas_Object *obj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   char buf[64];

   snprintf(buf, sizeof(buf), "elm,state,icon,%s",
            elm_layout_content_get(obj, "icon") ? "visible" : "hidden");

   elm_layout_signal_emit(obj, buf, "elm");
   edje_object_message_signal_process(wd->resize_obj);
}

EOLIAN static Elm_Atspi_State_Set
_elm_check_elm_interface_atspi_accessible_state_set_get(Eo *obj, Elm_Check_Data *_pd EINA_UNUSED)
{
   Elm_Atspi_State_Set states = 0;

   eo_do_super(obj, ELM_CHECK_CLASS, states = elm_interface_atspi_accessible_state_set_get());

   if (elm_check_state_get(obj))
       STATE_TYPE_SET(states, ELM_ATSPI_STATE_CHECKED);

   return states;
}

/* FIXME: replicated from elm_layout just because check's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Eina_Bool
_elm_check_elm_widget_sub_object_del(Eo *obj, Elm_Check_Data *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_sub_object_del(sobj));
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   eo_do(obj, elm_obj_layout_sizing_eval());

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_check_elm_widget_activate(Eo *obj EINA_UNUSED, Elm_Check_Data *_pd EINA_UNUSED, Elm_Activate act)
{
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (act != ELM_ACTIVATE_DEFAULT) return EINA_FALSE;

   _activate(obj);

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because check's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Eina_Bool
_elm_check_elm_container_content_set(Eo *obj, Elm_Check_Data *_pd EINA_UNUSED, const char *part, Evas_Object *content)
{
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_container_content_set(part, content));
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   eo_do(obj, elm_obj_layout_sizing_eval());

   return EINA_TRUE;
}

EOLIAN static void
_elm_check_elm_layout_sizing_eval(Eo *obj, Elm_Check_Data *_pd EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   _activate(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_check_elm_widget_event(Eo *obj, Elm_Check_Data *_pd EINA_UNUSED, Evas_Object *src, Evas_Callback_Type type, void *event_info)
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

EOLIAN static Eina_Bool
_elm_check_elm_widget_theme_apply(Eo *obj, Elm_Check_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   if (!sd->state) elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
   else elm_layout_signal_emit(obj, "elm,state,check,on", "elm");

   edje_object_message_signal_process(wd->resize_obj);

   /* FIXME: replicated from elm_layout just because check's icon spot
    * is elm.swallow.content, not elm.swallow.icon. Fix that whenever
    * we can changed the theme API */
   _icon_signal_emit(obj);

   eo_do(obj, elm_obj_layout_sizing_eval());

   return EINA_TRUE;
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
_access_state_cb(void *data, Evas_Object *obj)
{
   Elm_Check_Data *sd = eo_data_scope_get(data, MY_CLASS);
   const char *on_text, *off_text;

   if (elm_widget_disabled_get(obj))
     return strdup(E_("State: Disabled"));
   if (sd->state)
     {
        on_text = elm_layout_text_get(data, "on");

        if (on_text)
          {
             char buf[1024];

             snprintf(buf, sizeof(buf), "%s: %s", E_("State"), on_text);
             return strdup(buf);
          }
        else
          return strdup(E_("State: On"));
     }

   off_text = elm_layout_text_get(data, "off");

   if (off_text)
     {
        char buf[1024];

        snprintf(buf, sizeof(buf), "%s: %s", E_("State"), off_text);
        return strdup(buf);
     }
   return strdup(E_("State: Off"));
}

static void
_on_check_off(void *data,
              Evas_Object *o EINA_UNUSED,
              const char *emission EINA_UNUSED,
              const char *source EINA_UNUSED)
{
   Evas_Object *obj = data;

   ELM_CHECK_DATA_GET(obj, sd);

   sd->state = EINA_FALSE;
   if (sd->statep) *sd->statep = sd->state;

   elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
   evas_object_smart_callback_call(data, SIG_CHANGED, NULL);

   if (_elm_config->atspi_mode)
       elm_interface_atspi_accessible_state_changed_signal_emit(data,
                                                                ELM_ATSPI_STATE_CHECKED,
                                                                sd->state);
}

static void
_on_check_on(void *data,
             Evas_Object *o EINA_UNUSED,
             const char *emission EINA_UNUSED,
             const char *source EINA_UNUSED)
{
   Evas_Object *obj = data;

   ELM_CHECK_DATA_GET(obj, sd);

   sd->state = EINA_TRUE;
   if (sd->statep) *sd->statep = sd->state;
   elm_layout_signal_emit(obj, "elm,state,check,on", "elm");
   evas_object_smart_callback_call(data, SIG_CHANGED, NULL);

   if (_elm_config->atspi_mode)
       elm_interface_atspi_accessible_state_changed_signal_emit(data,
                                                                ELM_ATSPI_STATE_CHECKED,
                                                                sd->state);
}

static void
_on_check_toggle(void *data,
                 Evas_Object *o EINA_UNUSED,
                 const char *emission EINA_UNUSED,
                 const char *source EINA_UNUSED)
{
   _activate(data);
}

EOLIAN static void
_elm_check_evas_object_smart_add(Eo *obj, Elm_Check_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,check,on", "*",
     _on_check_on, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,check,off", "*",
     _on_check_off, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,check,toggle", "*",
     _on_check_toggle, obj);

   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("Check"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, obj);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   if (!elm_layout_theme_set(obj, "check", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_sizing_eval(obj);
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_check_elm_layout_content_aliases_get(Eo *obj EINA_UNUSED, Elm_Check_Data *_pd EINA_UNUSED)
{
   return _content_aliases;
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_check_elm_layout_text_aliases_get(Eo *obj EINA_UNUSED, Elm_Check_Data *_pd EINA_UNUSED)
{
   return _text_aliases;
}

EAPI Evas_Object *
elm_check_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_check_eo_base_constructor(Eo *obj, Elm_Check_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks),
         elm_interface_atspi_accessible_role_set(ELM_ATSPI_ROLE_CHECK_BOX));
}

EOLIAN static void
_elm_check_state_set(Eo *obj, Elm_Check_Data *sd, Eina_Bool state)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (state != sd->state)
     {
        sd->state = state;
        if (sd->statep) *sd->statep = sd->state;
        if (sd->state)
          elm_layout_signal_emit(obj, "elm,state,check,on", "elm");
        else
          elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
     }

   edje_object_message_signal_process(wd->resize_obj);
}

EOLIAN static Eina_Bool
_elm_check_state_get(Eo *obj EINA_UNUSED, Elm_Check_Data *sd)
{
   return sd->state;
}

EOLIAN static void
_elm_check_state_pointer_set(Eo *obj, Elm_Check_Data *sd, Eina_Bool *statep)
{
   if (statep)
     {
        sd->statep = statep;
        if (*sd->statep != sd->state)
          {
             sd->state = *sd->statep;
             if (sd->state)
               elm_layout_signal_emit(obj, "elm,state,check,on", "elm");
             else
               elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
          }
     }
   else
     sd->statep = NULL;
}

EOLIAN static Eina_Bool
_elm_check_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Check_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_check_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Check_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN const Elm_Atspi_Action *
_elm_check_elm_interface_atspi_widget_action_elm_actions_get(Eo *obj EINA_UNUSED, Elm_Check_Data *pd EINA_UNUSED)
{
   static Elm_Atspi_Action atspi_action[] = {
          { "activate", "activate", NULL, _key_action_activate },
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_action[0];
}

static void
_elm_check_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_check.eo.c"

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define ELM_LAYOUT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_check_private.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_CHECK_CLASS
#define MY_CLASS_PFX efl_ui_check

#define MY_CLASS_NAME "Efl.Ui.Check"

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {"on", "elm.ontext"},
   {"off", "elm.offtext"},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"icon", "elm.swallow.content"},
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
_check_legacy_event(Eo *obj)
{
   EFL_UI_CHECK_DATA_GET(obj, pd);
   if (pd->selected)
     {
        if (pd->legacy_changed_emitted_select) return;
        pd->legacy_changed_emitted_select = EINA_TRUE;
        pd->legacy_changed_emitted_unselect = EINA_FALSE;
     }
   else
     {
        if (pd->legacy_changed_emitted_unselect) return;
        pd->legacy_changed_emitted_unselect = EINA_TRUE;
        pd->legacy_changed_emitted_select = EINA_FALSE;
     }
   evas_object_smart_callback_call(obj, "changed", NULL);
}

static void
_activate(Evas_Object *obj)
{
   // state will be changed by the later call to the selected_set call
   if (!efl_ui_selectable_selected_get(obj))
     {
        // FIXME: to do animation during state change , we need different signal
        // so that we can distinguish between state change by user or state change
        // by calling state_change() api. Keep both the signal for backward compatibility
        // and remove "elm,state,check,on" signal emission when we can break ABI.
        // efl_ui_selectable_selected_set below will emit "elm,state,check,*" or "efl,state,*selected"
        if (elm_widget_is_legacy(obj))
          {
             elm_layout_signal_emit(obj, "elm,activate,check,on", "elm");
          }
        else
          {
             elm_layout_signal_emit(obj, "efl,state,selected", "efl");
          }

        if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
             _elm_access_say(E_("State: On"));
     }
   else
     {
        // FIXME: to do animation during state change , we need different signal
        // so that we can distinguish between state change by user or state change
        // by calling state_change() api. Keep both the signal for backward compatibility
        // and remove "elm,state,check,off" signal emission when we can break ABI.
        // efl_ui_selectable_selected_set below will emit "elm,state,check,*" or "efl,state,*selected"
        if (elm_widget_is_legacy(obj))
          {
             elm_layout_signal_emit(obj, "elm,activate,check,off", "elm");
          }
        else
          {
             elm_layout_signal_emit(obj, "efl,state,unselected", "efl");
          }

        if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
             _elm_access_say(E_("State: Off"));
     }
   //This commit will update the theme with the correct signals
   // "elm,state,check,on" or "elm,state,check,off" for legacy
   // "efl,state,selected" or "efl,state,unselected" for eo-api
   efl_ui_selectable_selected_set(obj, !efl_ui_selectable_selected_get(obj));
   if (elm_widget_is_legacy(obj))
     _check_legacy_event(obj);
   if (_elm_config->atspi_mode)
     efl_access_state_changed_signal_emit(obj,
                                          EFL_ACCESS_STATE_TYPE_CHECKED,
                                          efl_ui_selectable_selected_get(obj));
}

EOLIAN static Efl_Access_State_Set
_efl_ui_check_efl_access_object_state_set_get(const Eo *obj, Efl_Ui_Check_Data *_pd EINA_UNUSED)
{
   Efl_Access_State_Set states = 0;

   states = efl_access_object_state_set_get(efl_super(obj, EFL_UI_CHECK_CLASS));

   if (elm_check_state_get(obj))
       STATE_TYPE_SET(states, EFL_ACCESS_STATE_TYPE_CHECKED);

   return states;
}

EOLIAN static Eina_Bool
_efl_ui_check_efl_ui_widget_on_access_activate(Eo *obj EINA_UNUSED, Efl_Ui_Check_Data *_pd EINA_UNUSED, Efl_Ui_Activate act)
{
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (act != EFL_UI_ACTIVATE_DEFAULT) return EINA_FALSE;

   _activate(obj);

   return EINA_TRUE;
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   _activate(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Error
_efl_ui_check_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Check_Data *sd EINA_UNUSED)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_ERROR_GENERIC);

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   if (elm_widget_is_legacy(obj))
     {
        if (!efl_ui_selectable_selected_get(obj))
          elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
        else
          elm_layout_signal_emit(obj, "elm,state,check,on", "elm");
     }
   else
     {
        if (!efl_ui_selectable_selected_get(obj))
          elm_layout_signal_emit(obj, "efl,state,unselected", "efl");
        else
          elm_layout_signal_emit(obj, "efl,state,selected", "efl");
     }

   edje_object_message_signal_process(wd->resize_obj);

   return int_ret;
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
   const char *on_text, *off_text;

   if (elm_widget_disabled_get(obj))
     return strdup(E_("State: Disabled"));
   if (efl_ui_selectable_selected_get(obj))
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
_flush_selected(Eo *obj, Eina_Bool sel)
{
   efl_ui_selectable_selected_set(obj, sel);

   if (_elm_config->atspi_mode)
     efl_access_state_changed_signal_emit(obj,
                                          EFL_ACCESS_STATE_TYPE_CHECKED,
                                          efl_ui_selectable_selected_get(obj));
}

static void
_on_check_off(void *data,
              Evas_Object *o EINA_UNUSED,
              const char *emission EINA_UNUSED,
              const char *source EINA_UNUSED)
{
   Evas_Object *obj = data;

   _flush_selected(obj, EINA_FALSE);
   if (elm_widget_is_legacy(obj))
     _check_legacy_event(obj);
}

static void
_on_check_on(void *data,
             Evas_Object *o EINA_UNUSED,
             const char *emission EINA_UNUSED,
             const char *source EINA_UNUSED)
{
   Evas_Object *obj = data;

   _flush_selected(obj, EINA_TRUE);
   if (elm_widget_is_legacy(obj))
     _check_legacy_event(obj);
}

static void
_on_check_toggle(void *data,
                 Evas_Object *o EINA_UNUSED,
                 const char *emission EINA_UNUSED,
                 const char *source EINA_UNUSED)
{
   _activate(data);
}

static void
_clicked_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   _activate(data);
}

EOLIAN static Eina_Bool
_efl_ui_check_efl_ui_selectable_selected_get(const Eo *obj EINA_UNUSED, Efl_Ui_Check_Data *pd EINA_UNUSED)
{
   return pd->selected;
}

EOLIAN static void
_efl_ui_check_efl_ui_selectable_selected_set(Eo *obj, Efl_Ui_Check_Data *pd, Eina_Bool value)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (pd->selected == value) return;

   if (elm_widget_is_legacy(obj))
     {
        if (pd->statep)
          *pd->statep = value;
     }

   if (elm_widget_is_legacy(obj))
     {
        if (value == 1)
          elm_layout_signal_emit(obj, "elm,state,check,on", "elm");
        else
          elm_layout_signal_emit(obj, "elm,state,check,off", "elm");
     }
   else
     {
        if (value == 1)
          elm_layout_signal_emit(obj, "efl,state,selected", "efl");
        else
          elm_layout_signal_emit(obj, "efl,state,unselected", "efl");
     }

   edje_object_message_signal_process(wd->resize_obj);

   pd->selected = value;

   pd->legacy_changed_emitted_select = !pd->selected;
   pd->legacy_changed_emitted_unselect = pd->selected;

   if (!elm_widget_is_legacy(obj))
     efl_event_callback_call(obj, EFL_UI_EVENT_SELECTED_CHANGED, &pd->selected);
}

EOLIAN static Eo *
_efl_ui_check_efl_object_constructor(Eo *obj, Efl_Ui_Check_Data *pd EINA_UNUSED)
{
   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "check");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);
   efl_ui_action_connector_bind_clickable_to_theme(wd->resize_obj, obj);
   if (elm_widget_is_legacy(obj))
     {
        efl_layout_signal_callback_add
          (wd->resize_obj, "elm,action,check,on", "*", obj, _on_check_on, NULL);
        efl_layout_signal_callback_add
          (wd->resize_obj, "elm,action,check,off", "*", obj, _on_check_off, NULL);
        efl_layout_signal_callback_add
          (wd->resize_obj, "elm,action,check,toggle", "*", obj, _on_check_toggle, NULL);
     }
   else
     {
        efl_layout_signal_callback_add
          (wd->resize_obj, "efl,action,select", "*", obj, _on_check_on, NULL);
        efl_layout_signal_callback_add
          (wd->resize_obj, "efl,action,unselect", "*", obj, _on_check_off, NULL);
        efl_event_callback_add(obj, EFL_INPUT_EVENT_CLICKED, _clicked_cb, obj);
     }

   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_CHECK_BOX);
   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("Check"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, obj);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   return obj;
}

EAPI void
elm_check_state_set(Evas_Object *obj, Eina_Bool state)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   EFL_UI_CHECK_DATA_GET_OR_RETURN(obj, sd);

   efl_ui_selectable_selected_set(obj, state);
}

EAPI Eina_Bool
elm_check_state_get(const Evas_Object *obj)
{
   return !!efl_ui_selectable_selected_get(obj);
}

EAPI void
elm_check_state_pointer_set(Eo *obj, Eina_Bool *statep)
{
   EFL_UI_CHECK_DATA_GET_OR_RETURN(obj, sd);

   if (!statep)
     {
        sd->statep = NULL;
        return;
     }

   sd->statep = statep;
   if (*sd->statep != sd->selected)
     {
        efl_ui_selectable_selected_set(obj, *sd->statep);
     }
}

EOLIAN const Efl_Access_Action_Data *
_efl_ui_check_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Efl_Ui_Check_Data *pd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_action[] = {
          { "activate", "activate", NULL, _key_action_activate },
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_action[0];
}

/* Standard widget overrides */

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(efl_ui_check, Efl_Ui_Check_Data)
ELM_PART_TEXT_DEFAULT_IMPLEMENT(efl_ui_check, Efl_Ui_Check_Data)
ELM_PART_CONTENT_DEFAULT_IMPLEMENT(efl_ui_check, Efl_Ui_Check_Data)
EFL_UI_LAYOUT_CONTENT_ALIASES_IMPLEMENT(MY_CLASS_PFX)

/* Internal EO APIs and hidden overrides */

EFL_UI_LAYOUT_TEXT_ALIASES_IMPLEMENT(MY_CLASS_PFX)

#define EFL_UI_CHECK_EXTRA_OPS \
   EFL_UI_LAYOUT_CONTENT_ALIASES_OPS(MY_CLASS_PFX), \
   EFL_UI_LAYOUT_TEXT_ALIASES_OPS(MY_CLASS_PFX)


#include "efl_ui_check.eo.c"
#include "efl_ui_check_eo.legacy.c"

#include "efl_ui_check_legacy_eo.h"
#include "efl_ui_check_legacy_part.eo.h"

#define MY_CLASS_NAME_LEGACY "elm_check"

static void
_efl_ui_check_legacy_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Eo *
_efl_ui_check_legacy_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, EFL_UI_CHECK_LEGACY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   legacy_object_focus_handle(obj);
   return obj;
}

/* FIXME: replicated from elm_layout just because check's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Eina_Error
_efl_ui_check_legacy_efl_ui_widget_theme_apply(Eo *obj, void *_pd EINA_UNUSED)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, EFL_UI_CHECK_LEGACY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   if (efl_finalized_get(obj)) _elm_layout_legacy_icon_signal_emit(obj);

   return int_ret;
}

/* FIXME: replicated from elm_layout just because check's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Eina_Bool
_efl_ui_check_legacy_efl_ui_widget_widget_sub_object_del(Eo *obj, void *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = elm_widget_sub_object_del(efl_super(obj, EFL_UI_CHECK_LEGACY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   _elm_layout_legacy_icon_signal_emit(obj);

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because check's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static Eina_Bool
_efl_ui_check_legacy_content_set(Eo *obj, void *_pd EINA_UNUSED, const char *part, Evas_Object *content)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_content_set(efl_part(efl_super(obj, EFL_UI_CHECK_LEGACY_CLASS), part), content);
   if (!int_ret) return EINA_FALSE;

   _elm_layout_legacy_icon_signal_emit(obj);

   return EINA_TRUE;
}

/* Efl.Part begin */

static Eina_Bool
_part_is_efl_ui_check_legacy_part(const Eo *obj EINA_UNUSED, const char *part)
{
   return eina_streq(part, "elm.swallow.content");
}

ELM_PART_OVERRIDE_PARTIAL(efl_ui_check_legacy, EFL_UI_CHECK_LEGACY, void, _part_is_efl_ui_check_legacy_part)
ELM_PART_OVERRIDE_CONTENT_SET_NO_SD(efl_ui_check_legacy)
#include "efl_ui_check_legacy_part.eo.c"

/* Efl.Part end */

EAPI Evas_Object *
elm_check_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(EFL_UI_CHECK_LEGACY_CLASS, parent);
}

#include "efl_ui_check_legacy_eo.c"

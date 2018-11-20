#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define ELM_LAYOUT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_radio_private.h"
#include "elm_widget_layout.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_RADIO_CLASS
#define MY_CLASS_PFX efl_ui_radio

#define MY_CLASS_NAME "Efl.Ui.Radio"

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {NULL, NULL}
};

static const char SIG_CHANGED[] = "changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""}, /**< handled by efl_ui_check */
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
_state_set(Evas_Object *obj, Eina_Bool state, Eina_Bool activate)
{
   ELM_RADIO_DATA_GET(obj, sd);

   if (state != sd->state)
     {
        sd->state = state;
        if (sd->state)
          {
             // FIXME: to do animation during state change , we need different signal
             // so that we can distinguish between state change by user or state change
             // by calling state_change() api. Keep both the signal for backward compatibility
             // and only emit "elm,state,radio,on" when activate is false  when we can break ABI.
             if (elm_widget_is_legacy(obj))
               {
                  if (activate) elm_layout_signal_emit(obj, "elm,activate,radio,on", "elm");
                  elm_layout_signal_emit(obj, "elm,state,radio,on", "elm");
               }
             else
               {
                  if (activate) elm_layout_signal_emit(obj, "efl,activate,radio,on", "efl");
                  elm_layout_signal_emit(obj, "efl,state,radio,on", "efl");
               }
          }
        else
          {
             // FIXME: to do animation during state change , we need different signal
             // so that we can distinguish between state change by user or state change
             // by calling state_change() api. Keep both the signal for backward compatibility
             // and only emit "elm,state,radio,off"when activate is false when we can break ABI.
             if (elm_widget_is_legacy(obj))
               {
                  if (activate) elm_layout_signal_emit(obj, "elm,activate,radio,off", "elm");
                  elm_layout_signal_emit(obj, "elm,state,radio,off", "elm");
               }
             else
               {
                  if (activate) elm_layout_signal_emit(obj, "efl,activate,radio,off", "efl");
                  elm_layout_signal_emit(obj, "efl,state,radio,off", "efl");
               }
          }
        if (_elm_config->atspi_mode)
          {
             if (sd->state)
               {
                  efl_access_state_changed_signal_emit(obj, EFL_ACCESS_STATE_CHECKED, EINA_TRUE);
               }
          }
     }
}

static void
_state_set_all(Efl_Ui_Radio_Data *sd, Eina_Bool activate)
{
   const Eina_List *l;
   Eina_Bool disabled = EINA_FALSE;
   Evas_Object *child, *selected = NULL;

   EINA_LIST_FOREACH(sd->group->radios, l, child)
     {
        ELM_RADIO_DATA_GET(child, sdc);

        if (sdc->state) selected = child;
        if (sdc->value == sd->group->value)
          {
             _state_set(child, EINA_TRUE, activate);
             if (!sdc->state) disabled = EINA_TRUE;
          }
        else _state_set(child, EINA_FALSE, activate);
     }

   if ((disabled) && (selected)) _state_set(selected, 1, activate);
}

static void
_activate(Evas_Object *obj)
{
   ELM_RADIO_DATA_GET(obj, sd);

   if (sd->group->value == sd->value) return;

   if ((!_elm_config->access_mode) ||
       (_elm_access_2nd_click_timeout(obj)))
     {
        sd->group->value = sd->value;
        if (sd->group->valuep) *(sd->group->valuep) = sd->group->value;

        _state_set_all(sd, EINA_TRUE);

        if (_elm_config->access_mode)
          _elm_access_say(E_("State: On"));
        efl_event_callback_legacy_call
          (obj, EFL_UI_RADIO_EVENT_CHANGED, NULL);

     }
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   _activate(obj);
   return EINA_TRUE;
}

EOLIAN static Efl_Ui_Theme_Apply_Result
_efl_ui_radio_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Radio_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_RESULT_FAIL);
   Efl_Ui_Theme_Apply_Result int_ret = EFL_UI_THEME_APPLY_RESULT_FAIL;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, EFL_UI_CHECK_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_RESULT_FAIL;

   if (elm_widget_is_legacy(obj))
     {
        if (sd->state) elm_layout_signal_emit(obj, "elm,state,radio,on", "elm");
        else elm_layout_signal_emit(obj, "elm,state,radio,off", "elm");
     }
   else
     {
        if (sd->state) elm_layout_signal_emit(obj, "efl,state,radio,on", "efl");
        else elm_layout_signal_emit(obj, "efl,state,radio,off", "efl");
     }

   edje_object_message_signal_process(wd->resize_obj);

   elm_layout_sizing_eval(obj);

   return int_ret;
}

static void
_radio_on_cb(void *data,
             Evas_Object *obj EINA_UNUSED,
             const char *emission EINA_UNUSED,
             const char *source EINA_UNUSED)
{
   _activate(data);
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
   ELM_RADIO_DATA_GET(obj, sd);

   if (elm_widget_disabled_get(obj)) return strdup(E_("State: Disabled"));
   if (sd->state) return strdup(E_("State: On"));

   return strdup(E_("State: Off"));
}

EOLIAN static Eo *
_efl_ui_radio_efl_object_constructor(Eo *obj, Efl_Ui_Radio_Data *pd)
{
   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "radio");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (elm_widget_is_legacy(obj))
     elm_layout_signal_callback_add
        (obj, "elm,action,radio,toggle", "*", _radio_on_cb, obj);
   else
     elm_layout_signal_callback_add
        (obj, "efl,action,radio,toggle", "*", _radio_on_cb, obj);

   pd->group = calloc(1, sizeof(Group));
   pd->group->radios = eina_list_append(pd->group->radios, obj);

   elm_layout_sizing_eval(obj);

   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_RADIO_BUTTON);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("Radio"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, obj);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, obj);

   return obj;
}

EOLIAN static void
_efl_ui_radio_efl_object_destructor(Eo *obj EINA_UNUSED, Efl_Ui_Radio_Data *pd)
{
   pd->group->radios = eina_list_remove(pd->group->radios, obj);
   if (!pd->group->radios) free(pd->group);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_radio_group_add(Eo *obj, Efl_Ui_Radio_Data *sd, Evas_Object *group)
{
   ELM_RADIO_DATA_GET(group, sdg);

   if (!sdg)
     {
        if (eina_list_count(sd->group->radios) == 1) return;
        sd->group->radios = eina_list_remove(sd->group->radios, obj);
        sd->group = calloc(1, sizeof(Group));
        sd->group->radios = eina_list_append(sd->group->radios, obj);
     }
   else if (sd->group == sdg->group)
     return;
   else
     {
        sd->group->radios = eina_list_remove(sd->group->radios, obj);
        if (!sd->group->radios) free(sd->group);
        sd->group = sdg->group;
        sd->group->radios = eina_list_append(sd->group->radios, obj);
     }
   if (sd->value == sd->group->value) _state_set(obj, EINA_TRUE, EINA_FALSE);
   else _state_set(obj, EINA_FALSE, EINA_FALSE);
}

EOLIAN static void
_efl_ui_radio_state_value_set(Eo *obj, Efl_Ui_Radio_Data *sd, int value)
{
   sd->value = value;
   if (sd->value == sd->group->value) _state_set(obj, EINA_TRUE, EINA_FALSE);
   else _state_set(obj, EINA_FALSE, EINA_FALSE);
}

EOLIAN static int
_efl_ui_radio_state_value_get(const Eo *obj EINA_UNUSED, Efl_Ui_Radio_Data *sd)
{
   return sd->value;
}

EOLIAN static void
_efl_ui_radio_efl_ui_nstate_value_set(Eo *obj EINA_UNUSED, Efl_Ui_Radio_Data *sd, int value)
{
   if (value == sd->group->value) return;
   sd->group->value = value;
   if (sd->group->valuep) *(sd->group->valuep) = sd->group->value;
   _state_set_all(sd, EINA_FALSE);
}

EOLIAN static int
_efl_ui_radio_efl_ui_nstate_value_get(const Eo *obj EINA_UNUSED, Efl_Ui_Radio_Data *sd)
{
   return sd->group->value;
}

EOLIAN static void
_efl_ui_radio_value_pointer_set(Eo *obj EINA_UNUSED, Efl_Ui_Radio_Data *sd, int *valuep)
{
   if (valuep)
     {
        sd->group->valuep = valuep;
        if (*(sd->group->valuep) != sd->group->value)
          {
             sd->group->value = *(sd->group->valuep);
             _state_set_all(sd, EINA_FALSE);
          }
     }
   else sd->group->valuep = NULL;
}

EOLIAN static Evas_Object*
_efl_ui_radio_selected_object_get(const Eo *obj EINA_UNUSED, Efl_Ui_Radio_Data *sd)
{
   Eina_List *l;
   Evas_Object *child;

   EINA_LIST_FOREACH(sd->group->radios, l, child)
     {
        ELM_RADIO_DATA_GET(child, sdc);

        if (sdc->value == sd->group->value) return child;
     }

   return NULL;
}

EOLIAN static Eina_Bool
_efl_ui_radio_efl_ui_widget_on_access_activate(Eo *obj, Efl_Ui_Radio_Data *_pd EINA_UNUSED, Efl_Ui_Activate act)
{
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (act != EFL_UI_ACTIVATE_DEFAULT) return EINA_FALSE;

   _activate(obj);

   return EINA_TRUE;
}

EOLIAN const Efl_Access_Action_Data *
_efl_ui_radio_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Efl_Ui_Radio_Data *pd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "activate", "activate", NULL, _key_action_activate},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

EOLIAN Efl_Access_State_Set
_efl_ui_radio_efl_access_object_state_set_get(const Eo *obj, Efl_Ui_Radio_Data *pd EINA_UNUSED)
{
   Efl_Access_State_Set ret;

   ret = efl_access_object_state_set_get(efl_super(obj, EFL_UI_RADIO_CLASS));
   if (obj == elm_radio_selected_object_get(obj))
     STATE_TYPE_SET(ret, EFL_ACCESS_STATE_CHECKED);

   return ret;
}

/* Internal EO APIs and hidden overrides */

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(efl_ui_radio, Efl_Ui_Radio_Data)
ELM_LAYOUT_TEXT_ALIASES_IMPLEMENT(MY_CLASS_PFX)

#define EFL_UI_RADIO_EXTRA_OPS \
   ELM_LAYOUT_TEXT_ALIASES_OPS(MY_CLASS_PFX)

#include "efl_ui_radio.eo.c"

#include "efl_ui_radio_legacy.eo.h"
#include "efl_ui_radio_legacy_part.eo.h"

#define MY_CLASS_NAME_LEGACY "elm_radio"
/* Legacy APIs */

static void
_efl_ui_radio_legacy_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Eo *
_efl_ui_radio_legacy_efl_object_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, EFL_UI_RADIO_LEGACY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   legacy_object_focus_handle(obj);
   return obj;
}

/* FIXME: replicated from elm_layout just because radio's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static void
_icon_signal_emit(Evas_Object *obj)
{
   char buf[63];
   Eo *edje;

   edje = elm_widget_resize_object_get(obj);
   if (!edje) return;
   snprintf(buf, sizeof(buf), "elm,state,icon,%s",
            elm_layout_content_get(obj, "icon") ? "visible" : "hidden");

   elm_layout_signal_emit(obj, buf, "elm");
   edje_object_message_signal_process(edje);
   elm_layout_sizing_eval(obj);
}

EOLIAN static Efl_Ui_Theme_Apply_Result
_efl_ui_radio_legacy_efl_ui_widget_theme_apply(Eo *obj, void *_pd EINA_UNUSED)
{
   Efl_Ui_Theme_Apply_Result int_ret = EFL_UI_THEME_APPLY_RESULT_FAIL;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, EFL_UI_RADIO_LEGACY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_RESULT_FAIL;

   /* FIXME: replicated from elm_layout just because radio's icon
    * spot is elm.swallow.content, not elm.swallow.icon. Fix that
    * whenever we can changed the theme API */
   _icon_signal_emit(obj);

   return int_ret;
}

/* FIXME: replicated from elm_layout just because radio's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Eina_Bool
_efl_ui_radio_legacy_efl_ui_widget_widget_sub_object_del(Eo *obj, void *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = elm_widget_sub_object_del(efl_super(obj, EFL_UI_RADIO_LEGACY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because radio's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
static Eina_Bool
_efl_ui_radio_legacy_content_set(Eo *obj, void *_pd EINA_UNUSED, const char *part, Evas_Object *content)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_content_set(efl_part(efl_super(obj, EFL_UI_RADIO_LEGACY_CLASS), part), content);
   if (!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   return EINA_TRUE;
}

/* Efl.Part begin */

static Eina_Bool
_part_is_efl_ui_radio_legacy_part(const Eo *obj EINA_UNUSED, const char *part)
{
   return eina_streq(part, "elm.swallow.content");
}

ELM_PART_OVERRIDE_PARTIAL(efl_ui_radio_legacy, EFL_UI_RADIO_LEGACY, void, _part_is_efl_ui_radio_legacy_part)
ELM_PART_OVERRIDE_CONTENT_SET_NO_SD(efl_ui_radio_legacy)
#include "efl_ui_radio_legacy_part.eo.c"

/* Efl.Part end */

EAPI Evas_Object *
elm_radio_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(EFL_UI_RADIO_LEGACY_CLASS, parent);
}

EAPI void
elm_radio_value_set(Evas_Object *obj, int value)
{
   efl_ui_nstate_value_set(obj, value);
}

EAPI int
elm_radio_value_get(const Evas_Object *obj)
{
   return efl_ui_nstate_value_get(obj);
}

#include "efl_ui_radio_legacy.eo.c"

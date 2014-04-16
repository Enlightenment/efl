#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_radio.h"
#include "elm_widget_layout.h"

#define MY_CLASS ELM_OBJ_RADIO_CLASS

#define MY_CLASS_NAME "Elm_Radio"
#define MY_CLASS_NAME_LEGACY "elm_radio"

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

static const char SIG_CHANGED[] = "changed";
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
_state_set(Evas_Object *obj, Eina_Bool state)
{
   ELM_RADIO_DATA_GET(obj, sd);

   if ((state != sd->state) && (!elm_widget_disabled_get(obj)))
     {
        sd->state = state;
        if (sd->state)
          elm_layout_signal_emit(obj, "elm,state,radio,on", "elm");
        else
          elm_layout_signal_emit(obj, "elm,state,radio,off", "elm");
     }
}

static void
_state_set_all(Elm_Radio_Data *sd)
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
             _state_set(child, EINA_TRUE);
             if (!sdc->state) disabled = EINA_TRUE;
          }
        else _state_set(child, EINA_FALSE);
     }

   if ((disabled) && (selected)) _state_set(selected, 1);
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

        _state_set_all(sd);

        if (_elm_config->access_mode)
          _elm_access_say(E_("State: On"));
        evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
     }
}

/* FIXME: replicated from elm_layout just because radio's icon spot
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

/* FIXME: replicated from elm_layout just because radio's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Eina_Bool
_elm_radio_elm_widget_sub_object_del(Eo *obj, Elm_Radio_Data *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_sub_object_del(sobj));
   if(!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   eo_do(obj, elm_obj_layout_sizing_eval());

   return EINA_TRUE;
}

/* FIXME: replicated from elm_layout just because radio's icon spot
 * is elm.swallow.content, not elm.swallow.icon. Fix that whenever we
 * can changed the theme API */
EOLIAN static Eina_Bool
_elm_radio_elm_container_content_set(Eo *obj, Elm_Radio_Data *_pd EINA_UNUSED, const char *part, Evas_Object *content)
{
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_container_content_set(part, content));
   if(!int_ret) return EINA_FALSE;

   _icon_signal_emit(obj);

   eo_do(obj, elm_obj_layout_sizing_eval());

   return EINA_TRUE;
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   _activate(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_radio_elm_widget_event(Eo *obj, Elm_Radio_Data *_pd EINA_UNUSED, Evas_Object *src, Evas_Callback_Type type, void *event_info)
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
_elm_radio_elm_widget_theme_apply(Eo *obj, Elm_Radio_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   if (sd->state) elm_layout_signal_emit(obj, "elm,state,radio,on", "elm");
   else elm_layout_signal_emit(obj, "elm,state,radio,off", "elm");

   if (elm_widget_disabled_get(obj) && sd->state) _state_set(obj, EINA_FALSE);

   edje_object_message_signal_process(wd->resize_obj);

   /* FIXME: replicated from elm_layout just because radio's icon
    * spot is elm.swallow.content, not elm.swallow.icon. Fix that
    * whenever we can changed the theme API */
   _icon_signal_emit(obj);

   eo_do(obj, elm_obj_layout_sizing_eval());

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_radio_elm_widget_disable(Eo *obj, Elm_Radio_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_disable());
   if (!int_ret) return EINA_FALSE;

   if (elm_widget_disabled_get(obj) && sd->state) _state_set(obj, EINA_FALSE);

   return EINA_TRUE;
}

EOLIAN static void
_elm_radio_elm_layout_sizing_eval(Eo *obj, Elm_Radio_Data *_pd EINA_UNUSED)
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

EOLIAN static void
_elm_radio_evas_smart_add(Eo *obj, Elm_Radio_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set(obj, "radio", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_signal_callback_add
     (obj, "elm,action,radio,toggle", "*", _radio_on_cb, obj);

   priv->group = calloc(1, sizeof(Group));
   priv->group->radios = eina_list_append(priv->group->radios, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_sizing_eval(obj);

   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("Radio"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, obj);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, obj);
}

EOLIAN static void
_elm_radio_evas_smart_del(Eo *obj, Elm_Radio_Data *sd)
{
   sd->group->radios = eina_list_remove(sd->group->radios, obj);
   if (!sd->group->radios) free(sd->group);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_radio_elm_layout_text_aliases_get(Eo *obj EINA_UNUSED, Elm_Radio_Data *_pd EINA_UNUSED)
{
   return _text_aliases;
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_radio_elm_layout_content_aliases_get(Eo *obj EINA_UNUSED, Elm_Radio_Data *_pd EINA_UNUSED)
{
   return _content_aliases;
}

EAPI Evas_Object *
elm_radio_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_radio_eo_base_constructor(Eo *obj, Elm_Radio_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks));
}

EOLIAN static void
_elm_radio_group_add(Eo *obj, Elm_Radio_Data *sd, Evas_Object *group)
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
   if (sd->value == sd->group->value) _state_set(obj, EINA_TRUE);
   else _state_set(obj, EINA_FALSE);
}

EOLIAN static void
_elm_radio_state_value_set(Eo *obj, Elm_Radio_Data *sd, int value)
{
   sd->value = value;
   if (sd->value == sd->group->value) _state_set(obj, EINA_TRUE);
   else _state_set(obj, EINA_FALSE);
}

EOLIAN static int
_elm_radio_state_value_get(Eo *obj EINA_UNUSED, Elm_Radio_Data *sd)
{
   return sd->value;
}

EOLIAN static void
_elm_radio_value_set(Eo *obj EINA_UNUSED, Elm_Radio_Data *sd, int value)
{
   if (value == sd->group->value) return;
   sd->group->value = value;
   if (sd->group->valuep) *(sd->group->valuep) = sd->group->value;
   _state_set_all(sd);
}

EOLIAN static int
_elm_radio_value_get(Eo *obj EINA_UNUSED, Elm_Radio_Data *sd)
{
   return sd->group->value;
}

EOLIAN static void
_elm_radio_value_pointer_set(Eo *obj EINA_UNUSED, Elm_Radio_Data *sd, int *valuep)
{
   if (valuep)
     {
        sd->group->valuep = valuep;
        if (*(sd->group->valuep) != sd->group->value)
          {
             sd->group->value = *(sd->group->valuep);
             _state_set_all(sd);
          }
     }
   else sd->group->valuep = NULL;
}

EOLIAN static Evas_Object*
_elm_radio_selected_object_get(Eo *obj EINA_UNUSED, Elm_Radio_Data *sd)
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
_elm_radio_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Radio_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_radio_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Radio_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_radio_elm_widget_activate(Eo *obj, Elm_Radio_Data *_pd EINA_UNUSED, Elm_Activate act)
{
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (act != ELM_ACTIVATE_DEFAULT) return EINA_FALSE;

   _activate(obj);

   return EINA_TRUE;
}

EOLIAN static void
_elm_radio_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_radio.eo.c"

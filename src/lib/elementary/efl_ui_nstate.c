#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_NSTATE_PROTECTED
#include "Elementary.h"
#include "elm_priv.h"
#include "efl_ui_nstate.eo.h"
#include "efl_ui_button_private.h"

#define MY_CLASS EFL_UI_NSTATE_CLASS
#define MY_CLASS_NAME "Efl.Ui.Nstate"
#define MY_CLASS_NAME_LEGACY "efl_ui_nstate"


typedef struct
{
   int nstate;
   int state;
} Efl_Ui_Nstate_Data;

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {NULL, NULL}
};

static Eina_Bool _key_action_activate(Evas_Object *obj, const char *params);
static void _state_active(Evas_Object *obj, Efl_Ui_Nstate_Data *sd);

static const Elm_Action key_actions[] = {
   {"activate", _key_action_activate},
   {NULL, NULL}
};

EOLIAN static Efl_Object *
_efl_ui_nstate_efl_object_constructor(Eo *obj, Efl_Ui_Nstate_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   //TODO: Add ATSPI call here

   return obj;
}

static void
_next_state_set(Efl_Ui_Nstate_Data *sd)
{
   ++sd->state;
   if (sd->state == sd->nstate) sd->state = 0;
}

static void
_state_active(Evas_Object *obj, Efl_Ui_Nstate_Data *sd)
{
   char buf[64];

   sprintf(buf, "elm,state,changed,%d", sd->state);
   elm_layout_signal_emit(obj, buf, "elm");
   edje_object_message_signal_process(elm_layout_edje_get(obj));
   elm_layout_sizing_eval(obj);
   efl_event_callback_legacy_call(obj, EFL_UI_NSTATE_EVENT_STATE_CHANGED, NULL);
}

static void
_on_state_changed(void *data,
                  Evas_Object *o EINA_UNUSED,
                  const char *emission EINA_UNUSED,
                  const char *source EINA_UNUSED)
{
   efl_ui_nstate_activate(data);
}

EOLIAN static void
_efl_ui_nstate_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Nstate_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   pd->state = 0;
   // Default: 2 states
   pd->nstate = 2;

   if (!elm_layout_theme_set(obj, "nstate", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   edje_object_signal_callback_add(wd->resize_obj, "elm,action,state,changed",
                                   "*", _on_state_changed, obj);
}

EOLIAN static void
_efl_ui_nstate_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Nstate_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_signal_callback_del_full(wd->resize_obj, "elm,action,state,changed",
                                        "*", _on_state_changed, obj);
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static int
_efl_ui_nstate_count_get(Eo *obj EINA_UNUSED, Efl_Ui_Nstate_Data *pd)
{
   return pd->nstate;
}

EOLIAN static void
_efl_ui_nstate_count_set(Eo *obj EINA_UNUSED, Efl_Ui_Nstate_Data *pd, int nstate)
{
   if (pd->nstate == nstate) return;

   pd->nstate = nstate;
   pd->state = 0;
}

EOLIAN static int
_efl_ui_nstate_value_get(Eo *obj EINA_UNUSED, Efl_Ui_Nstate_Data *pd)
{
   return pd->state;
}

static Eina_Bool
_is_valid_state(Efl_Ui_Nstate_Data *sd, int state)
{
   if (sd->state == state || (state < 0 || state >= sd->nstate))
     return EINA_FALSE;

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_nstate_value_set(Eo *obj, Efl_Ui_Nstate_Data *pd, int state)
{
   if (!_is_valid_state(pd, state)) return;

   pd->state = state;
   _state_active(obj, pd);
}

EOLIAN static Elm_Theme_Apply
_efl_ui_nstate_elm_widget_theme_apply(Eo *obj, Efl_Ui_Nstate_Data *pd)
{
   Elm_Theme_Apply int_ret = ELM_THEME_APPLY_FAILED;

   int_ret = elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return ELM_THEME_APPLY_FAILED;

   _state_active(obj, pd);

   return int_ret;
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   efl_ui_nstate_activate(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_nstate_elm_widget_widget_event(Eo *obj, Efl_Ui_Nstate_Data *_pd EINA_UNUSED, Evas_Object *src EINA_UNUSED, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, MY_CLASS_NAME_LEGACY, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_nstate_activate(Eo *obj, Efl_Ui_Nstate_Data *_pd)
{
   _next_state_set(_pd);
   _state_active(obj, _pd);
}

EOLIAN static void
_efl_ui_nstate_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME, klass);
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_NSTATE_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_nstate)

#include "efl_ui_nstate.eo.c"

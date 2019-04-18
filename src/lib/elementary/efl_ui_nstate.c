#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_NSTATE_PROTECTED
#include "Elementary.h"
#include "elm_priv.h"
#include "efl_ui_nstate.eo.h"
#include "efl_ui_button_private.h"
#include "efl_ui_nstate_private.h"

#define MY_CLASS EFL_UI_NSTATE_CLASS
#define MY_CLASS_NAME "Efl.Ui.Nstate"


static Eina_Bool _key_action_activate(Evas_Object *obj, const char *params);
static void _state_active(Evas_Object *obj, Efl_Ui_Nstate_Data *sd);

static const Elm_Action key_actions[] = {
   {"activate", _key_action_activate},
   {NULL, NULL}
};

static void
_on_state_changed(void *data,
                  Evas_Object *o EINA_UNUSED,
                  const char *emission EINA_UNUSED,
                  const char *source EINA_UNUSED)
{
   efl_ui_nstate_activate(data);
}

EOLIAN static Efl_Object *
_efl_ui_nstate_efl_object_constructor(Eo *obj, Efl_Ui_Nstate_Data *pd)
{
   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "nstate");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);

   pd->state = 0;
   // Default: 2 states
   pd->nstate = 2;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);
   efl_layout_signal_callback_add
     (wd->resize_obj, "efl,action,state,changed", "*", obj, _on_state_changed, NULL);

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
_state_signal_emit(Evas_Object *obj, Efl_Ui_Nstate_Data *sd)
{
   char buf[64];

   sprintf(buf, "efl,state,changed,%d", sd->state);
   elm_layout_signal_emit(obj, buf, "efl");
   edje_object_message_signal_process(elm_layout_edje_get(obj));
   elm_layout_sizing_eval(obj);
}

static void
_state_active(Evas_Object *obj, Efl_Ui_Nstate_Data *sd)
{
   _state_signal_emit(obj, sd);
   efl_event_callback_legacy_call(obj, EFL_UI_NSTATE_EVENT_CHANGED, NULL);
}

EOLIAN static int
_efl_ui_nstate_count_get(const Eo *obj EINA_UNUSED, Efl_Ui_Nstate_Data *pd)
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
_efl_ui_nstate_value_get(const Eo *obj EINA_UNUSED, Efl_Ui_Nstate_Data *pd)
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

EOLIAN static Eina_Error
_efl_ui_nstate_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Nstate_Data *pd)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   _state_signal_emit(obj, pd);

   return int_ret;
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   efl_ui_nstate_activate(obj);
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

/* Standard widget overrides */

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(efl_ui_nstate, Efl_Ui_Nstate_Data)

#include "efl_ui_nstate.eo.c"

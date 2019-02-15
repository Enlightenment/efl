EWAPI const Efl_Event_Description _ELM_SPINNER_EVENT_CHANGED =
   EFL_EVENT_DESCRIPTION("changed");
EWAPI const Efl_Event_Description _ELM_SPINNER_EVENT_DELAY_CHANGED =
   EFL_EVENT_DESCRIPTION("delay,changed");
EWAPI const Efl_Event_Description _ELM_SPINNER_EVENT_SPINNER_DRAG_START =
   EFL_EVENT_DESCRIPTION("spinner,drag,start");
EWAPI const Efl_Event_Description _ELM_SPINNER_EVENT_SPINNER_DRAG_STOP =
   EFL_EVENT_DESCRIPTION("spinner,drag,stop");
EWAPI const Efl_Event_Description _ELM_SPINNER_EVENT_MIN_REACHED =
   EFL_EVENT_DESCRIPTION("min,reached");
EWAPI const Efl_Event_Description _ELM_SPINNER_EVENT_MAX_REACHED =
   EFL_EVENT_DESCRIPTION("max,reached");

void _elm_spinner_wrap_set(Eo *obj, Elm_Spinner_Data *pd, Eina_Bool wrap);


static Eina_Error
__eolian_elm_spinner_wrap_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_spinner_wrap_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_spinner_wrap_set, EFL_FUNC_CALL(wrap), Eina_Bool wrap);

Eina_Bool _elm_spinner_wrap_get(const Eo *obj, Elm_Spinner_Data *pd);


static Eina_Value
__eolian_elm_spinner_wrap_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_spinner_wrap_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_spinner_wrap_get, Eina_Bool, 0);

void _elm_spinner_interval_set(Eo *obj, Elm_Spinner_Data *pd, double interval);


static Eina_Error
__eolian_elm_spinner_interval_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_spinner_interval_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_spinner_interval_set, EFL_FUNC_CALL(interval), double interval);

double _elm_spinner_interval_get(const Eo *obj, Elm_Spinner_Data *pd);


static Eina_Value
__eolian_elm_spinner_interval_get_reflect(const Eo *obj)
{
   double val = elm_obj_spinner_interval_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_spinner_interval_get, double, 0);

void _elm_spinner_round_set(Eo *obj, Elm_Spinner_Data *pd, int rnd);


static Eina_Error
__eolian_elm_spinner_round_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_spinner_round_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_spinner_round_set, EFL_FUNC_CALL(rnd), int rnd);

int _elm_spinner_round_get(const Eo *obj, Elm_Spinner_Data *pd);


static Eina_Value
__eolian_elm_spinner_round_get_reflect(const Eo *obj)
{
   int val = elm_obj_spinner_round_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_spinner_round_get, int, 0);

void _elm_spinner_editable_set(Eo *obj, Elm_Spinner_Data *pd, Eina_Bool editable);


static Eina_Error
__eolian_elm_spinner_editable_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_spinner_editable_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_spinner_editable_set, EFL_FUNC_CALL(editable), Eina_Bool editable);

Eina_Bool _elm_spinner_editable_get(const Eo *obj, Elm_Spinner_Data *pd);


static Eina_Value
__eolian_elm_spinner_editable_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_spinner_editable_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_spinner_editable_get, Eina_Bool, 0);

void _elm_spinner_base_set(Eo *obj, Elm_Spinner_Data *pd, double base);


static Eina_Error
__eolian_elm_spinner_base_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_spinner_base_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_spinner_base_set, EFL_FUNC_CALL(base), double base);

double _elm_spinner_base_get(const Eo *obj, Elm_Spinner_Data *pd);


static Eina_Value
__eolian_elm_spinner_base_get_reflect(const Eo *obj)
{
   double val = elm_obj_spinner_base_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_spinner_base_get, double, 0);

void _elm_spinner_label_format_set(Eo *obj, Elm_Spinner_Data *pd, const char *fmt);


static Eina_Error
__eolian_elm_spinner_label_format_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_spinner_label_format_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_spinner_label_format_set, EFL_FUNC_CALL(fmt), const char *fmt);

const char *_elm_spinner_label_format_get(const Eo *obj, Elm_Spinner_Data *pd);


static Eina_Value
__eolian_elm_spinner_label_format_get_reflect(const Eo *obj)
{
   const char *val = elm_obj_spinner_label_format_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_spinner_label_format_get, const char *, NULL);

void _elm_spinner_special_value_add(Eo *obj, Elm_Spinner_Data *pd, double value, const char *label);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_spinner_special_value_add, EFL_FUNC_CALL(value, label), double value, const char *label);

Efl_Object *_elm_spinner_efl_object_constructor(Eo *obj, Elm_Spinner_Data *pd);


Eina_Error _elm_spinner_efl_ui_widget_theme_apply(Eo *obj, Elm_Spinner_Data *pd);


void _elm_spinner_efl_ui_widget_on_access_update(Eo *obj, Elm_Spinner_Data *pd, Eina_Bool enable);


Eina_Bool _elm_spinner_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Spinner_Data *pd);


Eina_Bool _elm_spinner_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Spinner_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


void _elm_spinner_efl_ui_range_display_range_min_max_set(Eo *obj, Elm_Spinner_Data *pd, double min, double max);


void _elm_spinner_efl_ui_range_display_range_min_max_get(const Eo *obj, Elm_Spinner_Data *pd, double *min, double *max);


void _elm_spinner_efl_ui_range_interactive_range_step_set(Eo *obj, Elm_Spinner_Data *pd, double step);


double _elm_spinner_efl_ui_range_interactive_range_step_get(const Eo *obj, Elm_Spinner_Data *pd);


void _elm_spinner_efl_ui_range_display_range_value_set(Eo *obj, Elm_Spinner_Data *pd, double val);


double _elm_spinner_efl_ui_range_display_range_value_get(const Eo *obj, Elm_Spinner_Data *pd);


const char *_elm_spinner_efl_access_object_i18n_name_get(const Eo *obj, Elm_Spinner_Data *pd);


Eina_Bool _elm_spinner_efl_access_value_value_and_text_set(Eo *obj, Elm_Spinner_Data *pd, double value, const char *text);


void _elm_spinner_efl_access_value_value_and_text_get(const Eo *obj, Elm_Spinner_Data *pd, double *value, const char **text);


void _elm_spinner_efl_access_value_range_get(const Eo *obj, Elm_Spinner_Data *pd, double *lower_limit, double *upper_limit, const char **description);


double _elm_spinner_efl_access_value_increment_get(const Eo *obj, Elm_Spinner_Data *pd);


const Efl_Access_Action_Data *_elm_spinner_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Spinner_Data *pd);


static Eina_Bool
_elm_spinner_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_SPINNER_EXTRA_OPS
#define ELM_SPINNER_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_spinner_wrap_set, _elm_spinner_wrap_set),
      EFL_OBJECT_OP_FUNC(elm_obj_spinner_wrap_get, _elm_spinner_wrap_get),
      EFL_OBJECT_OP_FUNC(elm_obj_spinner_interval_set, _elm_spinner_interval_set),
      EFL_OBJECT_OP_FUNC(elm_obj_spinner_interval_get, _elm_spinner_interval_get),
      EFL_OBJECT_OP_FUNC(elm_obj_spinner_round_set, _elm_spinner_round_set),
      EFL_OBJECT_OP_FUNC(elm_obj_spinner_round_get, _elm_spinner_round_get),
      EFL_OBJECT_OP_FUNC(elm_obj_spinner_editable_set, _elm_spinner_editable_set),
      EFL_OBJECT_OP_FUNC(elm_obj_spinner_editable_get, _elm_spinner_editable_get),
      EFL_OBJECT_OP_FUNC(elm_obj_spinner_base_set, _elm_spinner_base_set),
      EFL_OBJECT_OP_FUNC(elm_obj_spinner_base_get, _elm_spinner_base_get),
      EFL_OBJECT_OP_FUNC(elm_obj_spinner_label_format_set, _elm_spinner_label_format_set),
      EFL_OBJECT_OP_FUNC(elm_obj_spinner_label_format_get, _elm_spinner_label_format_get),
      EFL_OBJECT_OP_FUNC(elm_obj_spinner_special_value_add, _elm_spinner_special_value_add),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_spinner_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_spinner_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_spinner_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_on_focus_update, _elm_spinner_efl_ui_focus_object_on_focus_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_spinner_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(efl_ui_range_min_max_set, _elm_spinner_efl_ui_range_display_range_min_max_set),
      EFL_OBJECT_OP_FUNC(efl_ui_range_min_max_get, _elm_spinner_efl_ui_range_display_range_min_max_get),
      EFL_OBJECT_OP_FUNC(efl_ui_range_step_set, _elm_spinner_efl_ui_range_interactive_range_step_set),
      EFL_OBJECT_OP_FUNC(efl_ui_range_step_get, _elm_spinner_efl_ui_range_interactive_range_step_get),
      EFL_OBJECT_OP_FUNC(efl_ui_range_value_set, _elm_spinner_efl_ui_range_display_range_value_set),
      EFL_OBJECT_OP_FUNC(efl_ui_range_value_get, _elm_spinner_efl_ui_range_display_range_value_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_i18n_name_get, _elm_spinner_efl_access_object_i18n_name_get),
      EFL_OBJECT_OP_FUNC(efl_access_value_and_text_set, _elm_spinner_efl_access_value_value_and_text_set),
      EFL_OBJECT_OP_FUNC(efl_access_value_and_text_get, _elm_spinner_efl_access_value_value_and_text_get),
      EFL_OBJECT_OP_FUNC(efl_access_value_range_get, _elm_spinner_efl_access_value_range_get),
      EFL_OBJECT_OP_FUNC(efl_access_value_increment_get, _elm_spinner_efl_access_value_increment_get),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_spinner_efl_access_widget_action_elm_actions_get),
      ELM_SPINNER_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"wrap", __eolian_elm_spinner_wrap_set_reflect, __eolian_elm_spinner_wrap_get_reflect},
      {"interval", __eolian_elm_spinner_interval_set_reflect, __eolian_elm_spinner_interval_get_reflect},
      {"round", __eolian_elm_spinner_round_set_reflect, __eolian_elm_spinner_round_get_reflect},
      {"editable", __eolian_elm_spinner_editable_set_reflect, __eolian_elm_spinner_editable_get_reflect},
      {"base", __eolian_elm_spinner_base_set_reflect, __eolian_elm_spinner_base_get_reflect},
      {"label_format", __eolian_elm_spinner_label_format_set_reflect, __eolian_elm_spinner_label_format_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_spinner_class_desc = {
   EO_VERSION,
   "Elm.Spinner",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Spinner_Data),
   _elm_spinner_class_initializer,
   _elm_spinner_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_spinner_class_get, &_elm_spinner_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_RANGE_INTERACTIVE_INTERFACE, EFL_UI_FOCUS_COMPOSITION_MIXIN, EFL_ACCESS_VALUE_INTERFACE, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_spinner_eo.legacy.c"

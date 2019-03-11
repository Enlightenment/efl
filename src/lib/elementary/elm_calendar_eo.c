EWAPI const Efl_Event_Description _ELM_CALENDAR_EVENT_CHANGED =
   EFL_EVENT_DESCRIPTION("changed");
EWAPI const Efl_Event_Description _ELM_CALENDAR_EVENT_DISPLAY_CHANGED =
   EFL_EVENT_DESCRIPTION("display,changed");

void _elm_calendar_first_day_of_week_set(Eo *obj, Elm_Calendar_Data *pd, Elm_Calendar_Weekday day);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_calendar_first_day_of_week_set, EFL_FUNC_CALL(day), Elm_Calendar_Weekday day);

Elm_Calendar_Weekday _elm_calendar_first_day_of_week_get(const Eo *obj, Elm_Calendar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_calendar_first_day_of_week_get, Elm_Calendar_Weekday, 0);

void _elm_calendar_selectable_set(Eo *obj, Elm_Calendar_Data *pd, Elm_Calendar_Selectable selectable);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_calendar_selectable_set, EFL_FUNC_CALL(selectable), Elm_Calendar_Selectable selectable);

Elm_Calendar_Selectable _elm_calendar_selectable_get(const Eo *obj, Elm_Calendar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_calendar_selectable_get, Elm_Calendar_Selectable, 0);

void _elm_calendar_interval_set(Eo *obj, Elm_Calendar_Data *pd, double interval);


static Eina_Error
__eolian_elm_calendar_interval_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_calendar_interval_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_calendar_interval_set, EFL_FUNC_CALL(interval), double interval);

double _elm_calendar_interval_get(const Eo *obj, Elm_Calendar_Data *pd);


static Eina_Value
__eolian_elm_calendar_interval_get_reflect(Eo *obj)
{
   double val = elm_obj_calendar_interval_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_calendar_interval_get, double, 0);

void _elm_calendar_weekdays_names_set(Eo *obj, Elm_Calendar_Data *pd, const char **weekdays);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_calendar_weekdays_names_set, EFL_FUNC_CALL(weekdays), const char **weekdays);

const char **_elm_calendar_weekdays_names_get(const Eo *obj, Elm_Calendar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_calendar_weekdays_names_get, const char **, NULL);

void _elm_calendar_select_mode_set(Eo *obj, Elm_Calendar_Data *pd, Elm_Calendar_Select_Mode mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_calendar_select_mode_set, EFL_FUNC_CALL(mode), Elm_Calendar_Select_Mode mode);

Elm_Calendar_Select_Mode _elm_calendar_select_mode_get(const Eo *obj, Elm_Calendar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_calendar_select_mode_get, Elm_Calendar_Select_Mode, 0);

void _elm_calendar_format_function_set(Eo *obj, Elm_Calendar_Data *pd, Elm_Calendar_Format_Cb format_function);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_calendar_format_function_set, EFL_FUNC_CALL(format_function), Elm_Calendar_Format_Cb format_function);

const Eina_List *_elm_calendar_marks_get(const Eo *obj, Elm_Calendar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_calendar_marks_get, const Eina_List *, NULL);

void _elm_calendar_date_min_set(Eo *obj, Elm_Calendar_Data *pd, const Efl_Time *min);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_calendar_date_min_set, EFL_FUNC_CALL(min), const Efl_Time *min);

const Efl_Time *_elm_calendar_date_min_get(const Eo *obj, Elm_Calendar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_calendar_date_min_get, const Efl_Time *, NULL);

void _elm_calendar_date_max_set(Eo *obj, Elm_Calendar_Data *pd, const Efl_Time *max);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_calendar_date_max_set, EFL_FUNC_CALL(max), const Efl_Time *max);

const Efl_Time *_elm_calendar_date_max_get(const Eo *obj, Elm_Calendar_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_calendar_date_max_get, const Efl_Time *, NULL);

void _elm_calendar_selected_time_set(Eo *obj, Elm_Calendar_Data *pd, Efl_Time *selected_time);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_calendar_selected_time_set, EFL_FUNC_CALL(selected_time), Efl_Time *selected_time);

Eina_Bool _elm_calendar_selected_time_get(const Eo *obj, Elm_Calendar_Data *pd, Efl_Time *selected_time);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_calendar_selected_time_get, Eina_Bool, 0, EFL_FUNC_CALL(selected_time), Efl_Time *selected_time);

Elm_Calendar_Mark *_elm_calendar_mark_add(Eo *obj, Elm_Calendar_Data *pd, const char *mark_type, Efl_Time *mark_time, Elm_Calendar_Mark_Repeat_Type repeat);

EOAPI EFL_FUNC_BODYV(elm_obj_calendar_mark_add, Elm_Calendar_Mark *, NULL, EFL_FUNC_CALL(mark_type, mark_time, repeat), const char *mark_type, Efl_Time *mark_time, Elm_Calendar_Mark_Repeat_Type repeat);

void _elm_calendar_mark_del(Eo *obj, Elm_Calendar_Data *pd, Elm_Calendar_Mark *mark);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_calendar_mark_del, EFL_FUNC_CALL(mark), Elm_Calendar_Mark *mark);

void _elm_calendar_marks_clear(Eo *obj, Elm_Calendar_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_calendar_marks_clear);

void _elm_calendar_marks_draw(Eo *obj, Elm_Calendar_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_calendar_marks_draw);

Eina_Bool _elm_calendar_displayed_time_get(const Eo *obj, Elm_Calendar_Data *pd, Efl_Time *displayed_time);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_calendar_displayed_time_get, Eina_Bool, 0, EFL_FUNC_CALL(displayed_time), Efl_Time *displayed_time);

Efl_Object *_elm_calendar_efl_object_constructor(Eo *obj, Elm_Calendar_Data *pd);


void _elm_calendar_efl_canvas_group_group_calculate(Eo *obj, Elm_Calendar_Data *pd);


Eina_Error _elm_calendar_efl_ui_widget_theme_apply(Eo *obj, Elm_Calendar_Data *pd);


void _elm_calendar_efl_ui_widget_on_access_update(Eo *obj, Elm_Calendar_Data *pd, Eina_Bool enable);


Eina_Bool _elm_calendar_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Calendar_Data *pd);


Eina_Bool _elm_calendar_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Calendar_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


const Efl_Access_Action_Data *_elm_calendar_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Calendar_Data *pd);


static Eina_Bool
_elm_calendar_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_CALENDAR_EXTRA_OPS
#define ELM_CALENDAR_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_first_day_of_week_set, _elm_calendar_first_day_of_week_set),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_first_day_of_week_get, _elm_calendar_first_day_of_week_get),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_selectable_set, _elm_calendar_selectable_set),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_selectable_get, _elm_calendar_selectable_get),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_interval_set, _elm_calendar_interval_set),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_interval_get, _elm_calendar_interval_get),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_weekdays_names_set, _elm_calendar_weekdays_names_set),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_weekdays_names_get, _elm_calendar_weekdays_names_get),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_select_mode_set, _elm_calendar_select_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_select_mode_get, _elm_calendar_select_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_format_function_set, _elm_calendar_format_function_set),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_marks_get, _elm_calendar_marks_get),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_date_min_set, _elm_calendar_date_min_set),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_date_min_get, _elm_calendar_date_min_get),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_date_max_set, _elm_calendar_date_max_set),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_date_max_get, _elm_calendar_date_max_get),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_selected_time_set, _elm_calendar_selected_time_set),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_selected_time_get, _elm_calendar_selected_time_get),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_mark_add, _elm_calendar_mark_add),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_mark_del, _elm_calendar_mark_del),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_marks_clear, _elm_calendar_marks_clear),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_marks_draw, _elm_calendar_marks_draw),
      EFL_OBJECT_OP_FUNC(elm_obj_calendar_displayed_time_get, _elm_calendar_displayed_time_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_calendar_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_calculate, _elm_calendar_efl_canvas_group_group_calculate),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_calendar_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_calendar_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_on_focus_update, _elm_calendar_efl_ui_focus_object_on_focus_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_calendar_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_calendar_efl_access_widget_action_elm_actions_get),
      ELM_CALENDAR_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"interval", __eolian_elm_calendar_interval_set_reflect, __eolian_elm_calendar_interval_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_calendar_class_desc = {
   EO_VERSION,
   "Elm.Calendar",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Calendar_Data),
   _elm_calendar_class_initializer,
   _elm_calendar_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_calendar_class_get, &_elm_calendar_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_FOCUS_COMPOSITION_MIXIN, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_calendar_eo.legacy.c"


ELM_API void
elm_calendar_first_day_of_week_set(Elm_Calendar *obj, Elm_Calendar_Weekday day)
{
   elm_obj_calendar_first_day_of_week_set(obj, day);
}

ELM_API Elm_Calendar_Weekday
elm_calendar_first_day_of_week_get(const Elm_Calendar *obj)
{
   return elm_obj_calendar_first_day_of_week_get(obj);
}

ELM_API void
elm_calendar_selectable_set(Elm_Calendar *obj, Elm_Calendar_Selectable selectable)
{
   elm_obj_calendar_selectable_set(obj, selectable);
}

ELM_API Elm_Calendar_Selectable
elm_calendar_selectable_get(const Elm_Calendar *obj)
{
   return elm_obj_calendar_selectable_get(obj);
}

ELM_API void
elm_calendar_interval_set(Elm_Calendar *obj, double interval)
{
   elm_obj_calendar_interval_set(obj, interval);
}

ELM_API double
elm_calendar_interval_get(const Elm_Calendar *obj)
{
   return elm_obj_calendar_interval_get(obj);
}

ELM_API void
elm_calendar_weekdays_names_set(Elm_Calendar *obj, const char **weekdays)
{
   elm_obj_calendar_weekdays_names_set(obj, weekdays);
}

ELM_API const char **
elm_calendar_weekdays_names_get(const Elm_Calendar *obj)
{
   return elm_obj_calendar_weekdays_names_get(obj);
}

ELM_API void
elm_calendar_select_mode_set(Elm_Calendar *obj, Elm_Calendar_Select_Mode mode)
{
   elm_obj_calendar_select_mode_set(obj, mode);
}

ELM_API Elm_Calendar_Select_Mode
elm_calendar_select_mode_get(const Elm_Calendar *obj)
{
   return elm_obj_calendar_select_mode_get(obj);
}

ELM_API void
elm_calendar_format_function_set(Elm_Calendar *obj, Elm_Calendar_Format_Cb format_function)
{
   elm_obj_calendar_format_function_set(obj, format_function);
}

ELM_API const Eina_List *
elm_calendar_marks_get(const Elm_Calendar *obj)
{
   return elm_obj_calendar_marks_get(obj);
}

ELM_API void
elm_calendar_date_min_set(Elm_Calendar *obj, const Efl_Time *min)
{
   elm_obj_calendar_date_min_set(obj, min);
}

ELM_API const Efl_Time *
elm_calendar_date_min_get(const Elm_Calendar *obj)
{
   return elm_obj_calendar_date_min_get(obj);
}

ELM_API void
elm_calendar_date_max_set(Elm_Calendar *obj, const Efl_Time *max)
{
   elm_obj_calendar_date_max_set(obj, max);
}

ELM_API const Efl_Time *
elm_calendar_date_max_get(const Elm_Calendar *obj)
{
   return elm_obj_calendar_date_max_get(obj);
}

ELM_API void
elm_calendar_selected_time_set(Elm_Calendar *obj, Efl_Time *selected_time)
{
   elm_obj_calendar_selected_time_set(obj, selected_time);
}

ELM_API Eina_Bool
elm_calendar_selected_time_get(const Elm_Calendar *obj, Efl_Time *selected_time)
{
   return elm_obj_calendar_selected_time_get(obj, selected_time);
}

ELM_API Elm_Calendar_Mark *
elm_calendar_mark_add(Elm_Calendar *obj, const char *mark_type, Efl_Time *mark_time, Elm_Calendar_Mark_Repeat_Type repeat)
{
   return elm_obj_calendar_mark_add(obj, mark_type, mark_time, repeat);
}

ELM_API void
elm_calendar_marks_clear(Elm_Calendar *obj)
{
   elm_obj_calendar_marks_clear(obj);
}

ELM_API void
elm_calendar_marks_draw(Elm_Calendar *obj)
{
   elm_obj_calendar_marks_draw(obj);
}

ELM_API Eina_Bool
elm_calendar_displayed_time_get(const Elm_Calendar *obj, Efl_Time *displayed_time)
{
   return elm_obj_calendar_displayed_time_get(obj, displayed_time);
}

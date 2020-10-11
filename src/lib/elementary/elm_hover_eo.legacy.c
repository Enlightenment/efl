
ELM_API void
elm_hover_target_set(Elm_Hover *obj, Efl_Canvas_Object *target)
{
   elm_obj_hover_target_set(obj, target);
}

ELM_API Efl_Canvas_Object *
elm_hover_target_get(const Elm_Hover *obj)
{
   return elm_obj_hover_target_get(obj);
}

ELM_API const char *
elm_hover_best_content_location_get(const Elm_Hover *obj, Elm_Hover_Axis pref_axis)
{
   return elm_obj_hover_best_content_location_get(obj, pref_axis);
}

ELM_API void
elm_hover_dismiss(Elm_Hover *obj)
{
   elm_obj_hover_dismiss(obj);
}

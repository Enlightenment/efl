
ELM_API void
elm_label_wrap_width_set(Elm_Label *obj, int w)
{
   elm_obj_label_wrap_width_set(obj, w);
}

ELM_API int
elm_label_wrap_width_get(const Elm_Label *obj)
{
   return elm_obj_label_wrap_width_get(obj);
}

ELM_API void
elm_label_slide_speed_set(Elm_Label *obj, double speed)
{
   elm_obj_label_slide_speed_set(obj, speed);
}

ELM_API double
elm_label_slide_speed_get(const Elm_Label *obj)
{
   return elm_obj_label_slide_speed_get(obj);
}

ELM_API void
elm_label_slide_mode_set(Elm_Label *obj, Elm_Label_Slide_Mode mode)
{
   elm_obj_label_slide_mode_set(obj, mode);
}

ELM_API Elm_Label_Slide_Mode
elm_label_slide_mode_get(const Elm_Label *obj)
{
   return elm_obj_label_slide_mode_get(obj);
}

ELM_API void
elm_label_slide_duration_set(Elm_Label *obj, double duration)
{
   elm_obj_label_slide_duration_set(obj, duration);
}

ELM_API double
elm_label_slide_duration_get(const Elm_Label *obj)
{
   return elm_obj_label_slide_duration_get(obj);
}

ELM_API void
elm_label_line_wrap_set(Elm_Label *obj, Elm_Wrap_Type wrap)
{
   elm_obj_label_line_wrap_set(obj, wrap);
}

ELM_API Elm_Wrap_Type
elm_label_line_wrap_get(const Elm_Label *obj)
{
   return elm_obj_label_line_wrap_get(obj);
}

ELM_API void
elm_label_ellipsis_set(Elm_Label *obj, Eina_Bool ellipsis)
{
   elm_obj_label_ellipsis_set(obj, ellipsis);
}

ELM_API Eina_Bool
elm_label_ellipsis_get(const Elm_Label *obj)
{
   return elm_obj_label_ellipsis_get(obj);
}

ELM_API void
elm_label_slide_go(Elm_Label *obj)
{
   elm_obj_label_slide_go(obj);
}

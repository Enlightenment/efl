
EAPI void
elm_animation_view_auto_play_set(Efl_Ui_Animation_View *obj, Eina_Bool auto_play)
{
   efl_ui_animation_view_auto_play_set(obj, auto_play);
}

EAPI Eina_Bool
elm_animation_view_auto_play_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_auto_play_get(obj);
}

EAPI void
elm_animation_view_auto_repeat_set(Efl_Ui_Animation_View *obj, Eina_Bool auto_repeat)
{
   efl_ui_animation_view_auto_repeat_set(obj, auto_repeat);
}

EAPI Eina_Bool
elm_animation_view_auto_repeat_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_auto_repeat_get(obj);
}

EAPI Eina_Bool
elm_animation_view_speed_set(Efl_Ui_Animation_View *obj, double speed)
{
   return efl_ui_animation_view_speed_set(obj, speed);
}

EAPI double
elm_animation_view_speed_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_speed_get(obj);
}

EAPI double
elm_animation_view_duration_time_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_duration_time_get(obj);
}

EAPI void
elm_animation_view_progress_set(Efl_Ui_Animation_View *obj, double progress)
{
   efl_ui_animation_view_progress_set(obj, progress);
}

EAPI double
elm_animation_view_progress_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_progress_get(obj);
}

EAPI void
elm_animation_view_frame_set(Efl_Ui_Animation_View *obj, int frame_num)
{
   efl_ui_animation_view_frame_set(obj, frame_num);
}

EAPI int
elm_animation_view_frame_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_frame_get(obj);
}

EAPI Eina_Bool
elm_animation_view_play(Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_play(obj);
}

EAPI Eina_Bool
elm_animation_view_play_back(Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_play_back(obj);
}

EAPI Eina_Bool
elm_animation_view_pause(Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_pause(obj);
}

EAPI Eina_Bool
elm_animation_view_resume(Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_resume(obj);
}

EAPI Eina_Bool
elm_animation_view_stop(Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_stop(obj);
}

EAPI Eina_Size2D
elm_animation_view_default_size_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_default_size_get(obj);
}

EAPI Eina_Bool
elm_animation_view_is_playing_back(Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_is_playing_back(obj);
}

EAPI int
elm_animation_view_frame_count_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_frame_count_get(obj);
}

EAPI void
elm_animation_view_min_progress_set(Efl_Ui_Animation_View *obj, double min_progress)
{
   efl_ui_animation_view_min_progress_set(obj, min_progress);
}

EAPI double
elm_animation_view_min_progress_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_min_progress_get(obj);
}

EAPI void
elm_animation_view_max_progress_set(Efl_Ui_Animation_View *obj, double max_progress)
{
   efl_ui_animation_view_max_progress_set(obj, max_progress);
}

EAPI double
elm_animation_view_max_progress_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_max_progress_get(obj);
}

EAPI void
elm_animation_view_min_frame_set(Efl_Ui_Animation_View *obj, int min_frame)
{
   efl_ui_animation_view_min_frame_set(obj, min_frame);
}

EAPI int
elm_animation_view_min_frame_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_min_frame_get(obj);
}

EAPI void
elm_animation_view_max_frame_set(Efl_Ui_Animation_View *obj, int max_frame)
{
   efl_ui_animation_view_max_frame_set(obj, max_frame);
}

EAPI int
elm_animation_view_max_frame_get(const Efl_Ui_Animation_View *obj)
{
   return efl_ui_animation_view_max_frame_get(obj);
}


EAPI void
emotion_object_module_option_set(Efl_Canvas_Video *obj, const char *opt, const char *val)
{
   efl_canvas_video_option_set(obj, opt, val);
}

EAPI Eina_Bool
emotion_object_init(Efl_Canvas_Video *obj, const char *module_filename)
{
   return efl_canvas_video_engine_set(obj, module_filename);
}

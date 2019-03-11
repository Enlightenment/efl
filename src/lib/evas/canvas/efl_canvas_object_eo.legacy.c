
EAPI Eina_Bool
evas_object_pointer_mode_set(Efl_Canvas_Object *obj, Efl_Input_Object_Pointer_Mode pointer_mode)
{
   return efl_canvas_object_pointer_mode_set(obj, pointer_mode);
}

EAPI Efl_Input_Object_Pointer_Mode
evas_object_pointer_mode_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_pointer_mode_get(obj);
}

EAPI void
evas_object_clip_set(Efl_Canvas_Object *obj, Efl_Canvas_Object *clipper)
{
   efl_canvas_object_clipper_set(obj, clipper);
}

EAPI Efl_Canvas_Object *
evas_object_clip_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_clipper_get(obj);
}

EAPI void
evas_object_repeat_events_set(Efl_Canvas_Object *obj, Eina_Bool repeat)
{
   efl_canvas_object_repeat_events_set(obj, repeat);
}

EAPI Eina_Bool
evas_object_repeat_events_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_repeat_events_get(obj);
}

EAPI void
evas_object_focus_set(Efl_Canvas_Object *obj, Eina_Bool focus)
{
   efl_canvas_object_key_focus_set(obj, focus);
}

EAPI Eina_Bool
evas_object_focus_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_key_focus_get(obj);
}

EAPI void
evas_object_precise_is_inside_set(Efl_Canvas_Object *obj, Eina_Bool precise)
{
   efl_canvas_object_precise_is_inside_set(obj, precise);
}

EAPI Eina_Bool
evas_object_precise_is_inside_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_precise_is_inside_get(obj);
}

EAPI void
evas_object_propagate_events_set(Efl_Canvas_Object *obj, Eina_Bool propagate)
{
   efl_canvas_object_propagate_events_set(obj, propagate);
}

EAPI Eina_Bool
evas_object_propagate_events_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_propagate_events_get(obj);
}

EAPI void
evas_object_pass_events_set(Efl_Canvas_Object *obj, Eina_Bool pass)
{
   efl_canvas_object_pass_events_set(obj, pass);
}

EAPI Eina_Bool
evas_object_pass_events_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_pass_events_get(obj);
}

EAPI void
evas_object_anti_alias_set(Efl_Canvas_Object *obj, Eina_Bool anti_alias)
{
   efl_canvas_object_anti_alias_set(obj, anti_alias);
}

EAPI Eina_Bool
evas_object_anti_alias_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_anti_alias_get(obj);
}

EAPI Efl_Canvas_Object *
evas_object_smart_parent_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_render_parent_get(obj);
}

EAPI void
evas_object_paragraph_direction_set(Efl_Canvas_Object *obj, Efl_Text_Bidirectional_Type dir)
{
   efl_canvas_object_paragraph_direction_set(obj, dir);
}

EAPI Efl_Text_Bidirectional_Type
evas_object_paragraph_direction_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_paragraph_direction_get(obj);
}


EVAS_API Eina_Bool
evas_object_pointer_mode_set(Efl_Canvas_Object *obj, Evas_Object_Pointer_Mode pointer_mode)
{
   return efl_canvas_object_pointer_mode_set(obj, (Efl_Input_Object_Pointer_Mode)pointer_mode);
}

EVAS_API Evas_Object_Pointer_Mode
evas_object_pointer_mode_get(const Efl_Canvas_Object *obj)
{
   return (Evas_Object_Pointer_Mode)efl_canvas_object_pointer_mode_get(obj);
}

EVAS_API void
evas_object_clip_set(Efl_Canvas_Object *obj, Efl_Canvas_Object *clipper)
{
   efl_canvas_object_clipper_set(obj, clipper);
}

EVAS_API Efl_Canvas_Object *
evas_object_clip_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_clipper_get(obj);
}

EVAS_API void
evas_object_repeat_events_set(Efl_Canvas_Object *obj, Eina_Bool repeat)
{
   efl_canvas_object_repeat_events_set(obj, repeat);
}

EVAS_API Eina_Bool
evas_object_repeat_events_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_repeat_events_get(obj);
}

EVAS_API void
evas_object_focus_set(Efl_Canvas_Object *obj, Eina_Bool focus)
{
   efl_canvas_object_key_focus_set(obj, focus);
}

EVAS_API Eina_Bool
evas_object_focus_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_key_focus_get(obj);
}

EVAS_API void
evas_object_precise_is_inside_set(Efl_Canvas_Object *obj, Eina_Bool precise)
{
   efl_canvas_object_precise_is_inside_set(obj, precise);
}

EVAS_API Eina_Bool
evas_object_precise_is_inside_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_precise_is_inside_get(obj);
}

EVAS_API void
evas_object_propagate_events_set(Efl_Canvas_Object *obj, Eina_Bool propagate)
{
   efl_canvas_object_propagate_events_set(obj, propagate);
}

EVAS_API Eina_Bool
evas_object_propagate_events_get(const Efl_Canvas_Object *obj)
{
   if (!efl_isa(obj, EFL_CANVAS_OBJECT_CLASS)) return EINA_FALSE;
   return efl_canvas_object_propagate_events_get(obj);
}

EVAS_API void
evas_object_pass_events_set(Efl_Canvas_Object *obj, Eina_Bool pass)
{
   efl_canvas_object_pass_events_set(obj, pass);
}

EVAS_API Eina_Bool
evas_object_pass_events_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_pass_events_get(obj);
}

EVAS_API void
evas_object_anti_alias_set(Efl_Canvas_Object *obj, Eina_Bool anti_alias)
{
   efl_canvas_object_anti_alias_set(obj, anti_alias);
}

EVAS_API Eina_Bool
evas_object_anti_alias_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_anti_alias_get(obj);
}

EVAS_API Efl_Canvas_Object *
evas_object_smart_parent_get(const Efl_Canvas_Object *obj)
{
   return efl_canvas_object_render_parent_get(obj);
}

static inline Efl_Text_Bidirectional_Type
_evas_bidi_direction_type_to_efl_text_bidirectional_type(Evas_BiDi_Direction type)
{
   switch (type)
     {
#define CONVERT_TYPE(TYPE) case EVAS_BIDI_DIRECTION_##TYPE: return EFL_TEXT_BIDIRECTIONAL_TYPE_##TYPE
      CONVERT_TYPE(NATURAL); //Neutral text type, same as natural
      CONVERT_TYPE(LTR);
      CONVERT_TYPE(RTL);
      CONVERT_TYPE(INHERIT);
      default: break;
     }
   return EFL_TEXT_BIDIRECTIONAL_TYPE_NATURAL;
#undef CONVERT_TYPE
}

static inline Evas_BiDi_Direction
_efl_text_bidirectional_type_to_evas_bidi_direction_type(Efl_Text_Bidirectional_Type type)
{
   switch (type)
     {
#define CONVERT_TYPE(TYPE) case EFL_TEXT_BIDIRECTIONAL_TYPE_##TYPE: return EVAS_BIDI_DIRECTION_##TYPE
      CONVERT_TYPE(NATURAL); //Neutral text type, same as natural
      CONVERT_TYPE(LTR);
      CONVERT_TYPE(RTL);
      CONVERT_TYPE(INHERIT);
      default: break;
     }
   return EVAS_BIDI_DIRECTION_NATURAL;
#undef CONVERT_TYPE
}

EVAS_API void
evas_object_paragraph_direction_set(Efl_Canvas_Object *obj, Evas_BiDi_Direction dir)
{
   efl_canvas_object_paragraph_direction_set(obj, _evas_bidi_direction_type_to_efl_text_bidirectional_type(dir));
}

EVAS_API Evas_BiDi_Direction
evas_object_paragraph_direction_get(const Efl_Canvas_Object *obj)
{
   return _efl_text_bidirectional_type_to_evas_bidi_direction_type(efl_canvas_object_paragraph_direction_get(obj));
}

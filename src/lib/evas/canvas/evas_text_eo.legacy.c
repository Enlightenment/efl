
EVAS_API void
evas_object_text_shadow_color_set(Evas_Text *obj, int r, int g, int b, int a)
{
   evas_obj_text_shadow_color_set(obj, r, g, b, a);
}

EVAS_API void
evas_object_text_shadow_color_get(const Evas_Text *obj, int *r, int *g, int *b, int *a)
{
   evas_obj_text_shadow_color_get(obj, r, g, b, a);
}

EVAS_API void
evas_object_text_ellipsis_set(Evas_Text *obj, double ellipsis)
{
   evas_obj_text_ellipsis_set(obj, ellipsis);
}

EVAS_API double
evas_object_text_ellipsis_get(const Evas_Text *obj)
{
   return evas_obj_text_ellipsis_get(obj);
}

EVAS_API void
evas_object_text_bidi_delimiters_set(Evas_Text *obj, const char *delim)
{
   evas_obj_text_bidi_delimiters_set(obj, delim);
}

EVAS_API const char *
evas_object_text_bidi_delimiters_get(const Evas_Text *obj)
{
   return evas_obj_text_bidi_delimiters_get(obj);
}

EVAS_API void
evas_object_text_outline_color_set(Evas_Text *obj, int r, int g, int b, int a)
{
   evas_obj_text_outline_color_set(obj, r, g, b, a);
}

EVAS_API void
evas_object_text_outline_color_get(const Evas_Text *obj, int *r, int *g, int *b, int *a)
{
   evas_obj_text_outline_color_get(obj, r, g, b, a);
}

EVAS_API void
evas_object_text_glow2_color_set(Evas_Text *obj, int r, int g, int b, int a)
{
   evas_obj_text_glow2_color_set(obj, r, g, b, a);
}

EVAS_API void
evas_object_text_glow2_color_get(const Evas_Text *obj, int *r, int *g, int *b, int *a)
{
   evas_obj_text_glow2_color_get(obj, r, g, b, a);
}

EVAS_API void
evas_object_text_style_set(Evas_Text *obj, Evas_Text_Style_Type style)
{
   evas_obj_text_style_set(obj, style);
}

EVAS_API Evas_Text_Style_Type
evas_object_text_style_get(const Evas_Text *obj)
{
   return evas_obj_text_style_get(obj);
}

EVAS_API void
evas_object_text_glow_color_set(Evas_Text *obj, int r, int g, int b, int a)
{
   evas_obj_text_glow_color_set(obj, r, g, b, a);
}

EVAS_API void
evas_object_text_glow_color_get(const Evas_Text *obj, int *r, int *g, int *b, int *a)
{
   evas_obj_text_glow_color_get(obj, r, g, b, a);
}

EVAS_API int
evas_object_text_max_descent_get(const Evas_Text *obj)
{
   return evas_obj_text_max_descent_get(obj);
}

EVAS_API void
evas_object_text_style_pad_get(const Evas_Text *obj, int *l, int *r, int *t, int *b)
{
   evas_obj_text_style_pad_get(obj, l, r, t, b);
}

EVAS_API Efl_Text_Bidirectional_Type
evas_object_text_direction_get(const Evas_Text *obj)
{
   return evas_obj_text_direction_get(obj);
}

EVAS_API int
evas_object_text_ascent_get(const Evas_Text *obj)
{
   return evas_obj_text_ascent_get(obj);
}

EVAS_API int
evas_object_text_horiz_advance_get(const Evas_Text *obj)
{
   return evas_obj_text_horiz_advance_get(obj);
}

EVAS_API int
evas_object_text_inset_get(const Evas_Text *obj)
{
   return evas_obj_text_inset_get(obj);
}

EVAS_API int
evas_object_text_max_ascent_get(const Evas_Text *obj)
{
   return evas_obj_text_max_ascent_get(obj);
}

EVAS_API int
evas_object_text_vert_advance_get(const Evas_Text *obj)
{
   return evas_obj_text_vert_advance_get(obj);
}

EVAS_API int
evas_object_text_descent_get(const Evas_Text *obj)
{
   return evas_obj_text_descent_get(obj);
}

EVAS_API int
evas_object_text_last_up_to_pos(const Evas_Text *obj, int x, int y)
{
   return evas_obj_text_last_up_to_pos(obj, x, y);
}

EVAS_API int
evas_object_text_char_coords_get(const Evas_Text *obj, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   return evas_obj_text_char_coords_get(obj, x, y, cx, cy, cw, ch);
}

EVAS_API Eina_Bool
evas_object_text_char_pos_get(const Evas_Text *obj, int pos, int *cx, int *cy, int *cw, int *ch)
{
   return evas_obj_text_char_pos_get(obj, pos, cx, cy, cw, ch);
}

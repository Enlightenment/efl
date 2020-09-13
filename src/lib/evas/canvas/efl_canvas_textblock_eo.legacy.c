
EVAS_API Eina_Bool
evas_object_textblock_visible_range_get(Efl_Canvas_Textblock *obj EINA_UNUSED, Efl_Text_Cursor_Handle *start, Efl_Text_Cursor_Handle *end)
{
   return evas_textblock_cursor_visible_range_get(start, end);
}

EVAS_API void
evas_object_textblock_style_insets_get(const Efl_Canvas_Textblock *obj, int *l, int *r, int *t, int *b)
{
   efl_canvas_textblock_style_insets_get(obj, l, r, t, b);
}

EVAS_API void
evas_object_textblock_bidi_delimiters_set(Efl_Canvas_Textblock *obj, const char *delim)
{
   efl_canvas_textblock_bidi_delimiters_set(obj, delim);
}

EVAS_API const char *
evas_object_textblock_bidi_delimiters_get(const Efl_Canvas_Textblock *obj)
{
   return efl_canvas_textblock_bidi_delimiters_get(obj);
}

EVAS_API void
evas_object_textblock_legacy_newline_set(Efl_Canvas_Textblock *obj, Eina_Bool mode)
{
   efl_canvas_textblock_newline_as_paragraph_separator_set(obj, mode);
}

EVAS_API Eina_Bool
evas_object_textblock_legacy_newline_get(const Efl_Canvas_Textblock *obj)
{
   return efl_canvas_textblock_newline_as_paragraph_separator_get(obj);
}

EVAS_API void
evas_object_textblock_size_formatted_get(const Efl_Canvas_Textblock *obj, int *w, int *h)
{
   Eina_Size2D size;
   size = efl_canvas_textblock_size_formatted_get(obj);
   if (w) *w = size.w;
   if (h) *h = size.h;
}

EVAS_API void
evas_object_textblock_size_native_get(const Efl_Canvas_Textblock *obj, int *w, int *h)
{
   Eina_Size2D size;
   size = efl_canvas_textblock_size_native_get(obj);
   if (w) *w = size.w;
   if (h) *h = size.h;
}

EVAS_API Eina_Bool
evas_object_textblock_obstacle_add(Efl_Canvas_Textblock *obj, Efl_Canvas_Object *eo_obs)
{
   return efl_canvas_textblock_obstacle_add(obj, eo_obs);
}

EVAS_API Eina_Bool
evas_object_textblock_obstacle_del(Efl_Canvas_Textblock *obj, Efl_Canvas_Object *eo_obs)
{
   return efl_canvas_textblock_obstacle_del(obj, eo_obs);
}

EVAS_API void
evas_object_textblock_obstacles_update(Efl_Canvas_Textblock *obj)
{
   efl_canvas_textblock_obstacles_update(obj);
}

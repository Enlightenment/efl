
EAPI Eina_Bool
evas_object_textblock_visible_range_get(Efl_Canvas_Text *obj, Efl_Text_Cursor_Cursor *start, Efl_Text_Cursor_Cursor *end)
{
   return efl_canvas_text_visible_range_get(obj, start, end);
}

EAPI void
evas_object_textblock_style_insets_get(const Efl_Canvas_Text *obj, int *l, int *r, int *t, int *b)
{
   efl_canvas_text_style_insets_get(obj, l, r, t, b);
}

EAPI void
evas_object_textblock_bidi_delimiters_set(Efl_Canvas_Text *obj, const char *delim)
{
   efl_canvas_text_bidi_delimiters_set(obj, delim);
}

EAPI const char *
evas_object_textblock_bidi_delimiters_get(const Efl_Canvas_Text *obj)
{
   return efl_canvas_text_bidi_delimiters_get(obj);
}

EAPI void
evas_object_textblock_legacy_newline_set(Efl_Canvas_Text *obj, Eina_Bool mode)
{
   efl_canvas_text_legacy_newline_set(obj, mode);
}

EAPI Eina_Bool
evas_object_textblock_legacy_newline_get(const Efl_Canvas_Text *obj)
{
   return efl_canvas_text_legacy_newline_get(obj);
}

EAPI void
evas_object_textblock_size_formatted_get(const Efl_Canvas_Text *obj, int *w, int *h)
{
   efl_canvas_text_size_formatted_get(obj, w, h);
}

EAPI void
evas_object_textblock_size_native_get(const Efl_Canvas_Text *obj, int *w, int *h)
{
   efl_canvas_text_size_native_get(obj, w, h);
}

EAPI Eina_Bool
evas_object_textblock_obstacle_add(Efl_Canvas_Text *obj, Efl_Canvas_Object *eo_obs)
{
   return efl_canvas_text_obstacle_add(obj, eo_obs);
}

EAPI Eina_Bool
evas_object_textblock_obstacle_del(Efl_Canvas_Text *obj, Efl_Canvas_Object *eo_obs)
{
   return efl_canvas_text_obstacle_del(obj, eo_obs);
}

EAPI void
evas_object_textblock_obstacles_update(Efl_Canvas_Text *obj)
{
   efl_canvas_text_obstacles_update(obj);
}


EAPI Eina_Bool
evas_object_textblock_visible_range_get(Evas_Textblock_Legacy *obj, Efl_Text_Cursor_Cursor *start, Efl_Text_Cursor_Cursor *end)
{
   return evas_textblock_legacy_visible_range_get(obj, start, end);
}

EAPI void
evas_object_textblock_style_insets_get(const Evas_Textblock_Legacy *obj, int *l, int *r, int *t, int *b)
{
   evas_textblock_legacy_style_insets_get(obj, l, r, t, b);
}

EAPI void
evas_object_textblock_bidi_delimiters_set(Evas_Textblock_Legacy *obj, const char *delim)
{
   evas_textblock_legacy_bidi_delimiters_set(obj, delim);
}

EAPI const char *
evas_object_textblock_bidi_delimiters_get(const Evas_Textblock_Legacy *obj)
{
   return evas_textblock_legacy_bidi_delimiters_get(obj);
}

EAPI void
evas_object_textblock_legacy_newline_set(Evas_Textblock_Legacy *obj, Eina_Bool mode)
{
   evas_textblock_legacy_newline_set(obj, mode);
}

EAPI Eina_Bool
evas_object_textblock_legacy_newline_get(const Evas_Textblock_Legacy *obj)
{
   return evas_textblock_legacy_newline_get(obj);
}

EAPI void
evas_object_textblock_size_formatted_get(const Evas_Textblock_Legacy *obj, int *w, int *h)
{
   evas_textblock_legacy_size_formatted_get(obj, w, h);
}

EAPI void
evas_object_textblock_size_native_get(const Evas_Textblock_Legacy *obj, int *w, int *h)
{
   evas_textblock_legacy_size_native_get(obj, w, h);
}

EAPI Eina_Bool
evas_object_textblock_obstacle_add(Evas_Textblock_Legacy *obj, Efl_Canvas_Object *eo_obs)
{
   return evas_textblock_legacy_obstacle_add(obj, eo_obs);
}

EAPI Eina_Bool
evas_object_textblock_obstacle_del(Evas_Textblock_Legacy *obj, Efl_Canvas_Object *eo_obs)
{
   return evas_textblock_legacy_obstacle_del(obj, eo_obs);
}

EAPI void
evas_object_textblock_obstacles_update(Evas_Textblock_Legacy *obj)
{
   evas_textblock_legacy_obstacles_update(obj);
}

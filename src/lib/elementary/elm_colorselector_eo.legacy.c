
EAPI void
elm_colorselector_color_set(Elm_Colorselector *obj, int r, int g, int b, int a)
{
   elm_obj_colorselector_picked_color_set(obj, r, g, b, a);
}

EAPI void
elm_colorselector_color_get(const Elm_Colorselector *obj, int *r, int *g, int *b, int *a)
{
   elm_obj_colorselector_picked_color_get(obj, r, g, b, a);
}

EAPI void
elm_colorselector_palette_name_set(Elm_Colorselector *obj, const char *palette_name)
{
   elm_obj_colorselector_palette_name_set(obj, palette_name);
}

EAPI const char *
elm_colorselector_palette_name_get(const Elm_Colorselector *obj)
{
   return elm_obj_colorselector_palette_name_get(obj);
}

EAPI void
elm_colorselector_mode_set(Elm_Colorselector *obj, Elm_Colorselector_Mode mode)
{
   elm_obj_colorselector_mode_set(obj, mode);
}

EAPI Elm_Colorselector_Mode
elm_colorselector_mode_get(const Elm_Colorselector *obj)
{
   return elm_obj_colorselector_mode_get(obj);
}

EAPI const Eina_List *
elm_colorselector_palette_items_get(const Elm_Colorselector *obj)
{
   return elm_obj_colorselector_palette_items_get(obj);
}

EAPI Elm_Widget_Item *
elm_colorselector_palette_selected_item_get(const Elm_Colorselector *obj)
{
   return elm_obj_colorselector_palette_selected_item_get(obj);
}

EAPI Elm_Widget_Item *
elm_colorselector_palette_color_add(Elm_Colorselector *obj, int r, int g, int b, int a)
{
   return elm_obj_colorselector_palette_color_add(obj, r, g, b, a);
}

EAPI void
elm_colorselector_palette_clear(Elm_Colorselector *obj)
{
   elm_obj_colorselector_palette_clear(obj);
}

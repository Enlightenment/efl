
EAPI void
evas_object_textgrid_supported_font_styles_set(Evas_Textgrid *obj, Evas_Textgrid_Font_Style styles)
{
   evas_obj_textgrid_supported_font_styles_set(obj, styles);
}

EAPI Evas_Textgrid_Font_Style
evas_object_textgrid_supported_font_styles_get(const Evas_Textgrid *obj)
{
   return evas_obj_textgrid_supported_font_styles_get(obj);
}

EAPI void
evas_object_textgrid_size_set(Evas_Textgrid *obj, int w, int h)
{
   evas_obj_textgrid_grid_size_set(obj, w, h);
}

EAPI void
evas_object_textgrid_size_get(const Evas_Textgrid *obj, int *w, int *h)
{
   evas_obj_textgrid_grid_size_get(obj, w, h);
}

EAPI void
evas_object_textgrid_cell_size_get(const Evas_Textgrid *obj, int *width, int *height)
{
   evas_obj_textgrid_cell_size_get(obj, width, height);
}

EAPI void
evas_object_textgrid_update_add(Evas_Textgrid *obj, int x, int y, int w, int h)
{
   evas_obj_textgrid_update_add(obj, x, y, w, h);
}

EAPI void
evas_object_textgrid_cellrow_set(Evas_Textgrid *obj, int y, const Evas_Textgrid_Cell *row)
{
   evas_obj_textgrid_cellrow_set(obj, y, row);
}

EAPI Evas_Textgrid_Cell *
evas_object_textgrid_cellrow_get(const Evas_Textgrid *obj, int y)
{
   return evas_obj_textgrid_cellrow_get(obj, y);
}

EAPI void
evas_object_textgrid_palette_set(Evas_Textgrid *obj, Evas_Textgrid_Palette pal, int idx, int r, int g, int b, int a)
{
   evas_obj_textgrid_palette_set(obj, pal, idx, r, g, b, a);
}

EAPI void
evas_object_textgrid_palette_get(const Evas_Textgrid *obj, Evas_Textgrid_Palette pal, int idx, int *r, int *g, int *b, int *a)
{
   evas_obj_textgrid_palette_get(obj, pal, idx, r, g, b, a);
}

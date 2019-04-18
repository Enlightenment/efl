
EAPI void
evas_object_line_xy_set(Evas_Line *obj, int x1, int y1, int x2, int y2)
{
   evas_obj_line_xy_set(obj, x1, y1, x2, y2);
}

EAPI void
evas_object_line_xy_get(const Evas_Line *obj, int *x1, int *y1, int *x2, int *y2)
{
   evas_obj_line_xy_get(obj, x1, y1, x2, y2);
}

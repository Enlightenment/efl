
EAPI void
elm_pan_pos_set(Elm_Pan *obj, int x, int y)
{
   elm_obj_pan_pos_set(obj, x, y);
}

EAPI void
elm_pan_pos_get(const Elm_Pan *obj, int *x, int *y)
{
   elm_obj_pan_pos_get(obj, x, y);
}

EAPI void
elm_pan_content_size_get(const Elm_Pan *obj, int *w, int *h)
{
   elm_obj_pan_content_size_get(obj, w, h);
}

EAPI void
elm_pan_pos_min_get(const Elm_Pan *obj, int *x, int *y)
{
   elm_obj_pan_pos_min_get(obj, x, y);
}

EAPI void
elm_pan_pos_max_get(const Elm_Pan *obj, int *x, int *y)
{
   elm_obj_pan_pos_max_get(obj, x, y);
}

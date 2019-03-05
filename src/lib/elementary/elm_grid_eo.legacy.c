
EAPI void
elm_grid_size_set(Elm_Grid *obj, int w, int h)
{
   elm_obj_grid_size_set(obj, w, h);
}

EAPI void
elm_grid_size_get(const Elm_Grid *obj, int *w, int *h)
{
   elm_obj_grid_size_get(obj, w, h);
}

EAPI Eina_List *
elm_grid_children_get(const Elm_Grid *obj)
{
   return elm_obj_grid_children_get(obj);
}

EAPI void
elm_grid_clear(Elm_Grid *obj, Eina_Bool clear)
{
   elm_obj_grid_clear(obj, clear);
}

EAPI void
elm_grid_unpack(Elm_Grid *obj, Efl_Canvas_Object *subobj)
{
   elm_obj_grid_unpack(obj, subobj);
}

EAPI void
elm_grid_pack(Elm_Grid *obj, Efl_Canvas_Object *subobj, int x, int y, int w, int h)
{
   elm_obj_grid_pack(obj, subobj, x, y, w, h);
}

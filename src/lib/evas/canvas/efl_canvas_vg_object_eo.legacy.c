
EVAS_API void
evas_object_vg_viewbox_set(Evas_Object *obj, Eina_Rect viewbox)
{
   efl_canvas_vg_object_viewbox_set(obj, viewbox);
}

EVAS_API Eina_Rect
evas_object_vg_viewbox_get(const Evas_Object *obj)
{
   return efl_canvas_vg_object_viewbox_get(obj);
}

EVAS_API void
evas_object_vg_viewbox_align_set(Evas_Object *obj, double align_x, double align_y)
{
   efl_canvas_vg_object_viewbox_align_set(obj, align_x, align_y);
}

EVAS_API void
evas_object_vg_viewbox_align_get(const Evas_Object *obj, double *align_x, double *align_y)
{
   efl_canvas_vg_object_viewbox_align_get(obj, align_x, align_y);
}

EVAS_API void
evas_object_vg_root_node_set(Evas_Object *obj, Efl_Canvas_Vg_Node *root)
{
   efl_canvas_vg_object_root_node_set(obj, root);
}

EVAS_API Efl_Canvas_Vg_Node *
evas_object_vg_root_node_get(const Evas_Object *obj)
{
   return efl_canvas_vg_object_root_node_get(obj);
}

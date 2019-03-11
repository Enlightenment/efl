
EAPI void
evas_object_vg_fill_mode_set(Efl_Canvas_Vg_Object *obj, Efl_Canvas_Vg_Fill_Mode fill_mode)
{
   efl_canvas_vg_object_fill_mode_set(obj, fill_mode);
}

EAPI Efl_Canvas_Vg_Fill_Mode
evas_object_vg_fill_mode_get(const Efl_Canvas_Vg_Object *obj)
{
   return efl_canvas_vg_object_fill_mode_get(obj);
}

EAPI void
evas_object_vg_viewbox_set(Efl_Canvas_Vg_Object *obj, Eina_Rect viewbox)
{
   efl_canvas_vg_object_viewbox_set(obj, viewbox);
}

EAPI Eina_Rect
evas_object_vg_viewbox_get(const Efl_Canvas_Vg_Object *obj)
{
   return efl_canvas_vg_object_viewbox_get(obj);
}

EAPI void
evas_object_vg_viewbox_align_set(Efl_Canvas_Vg_Object *obj, double align_x, double align_y)
{
   efl_canvas_vg_object_viewbox_align_set(obj, align_x, align_y);
}

EAPI void
evas_object_vg_viewbox_align_get(const Efl_Canvas_Vg_Object *obj, double *align_x, double *align_y)
{
   efl_canvas_vg_object_viewbox_align_get(obj, align_x, align_y);
}

EAPI void
evas_object_vg_root_node_set(Efl_Canvas_Vg_Object *obj, Efl_Canvas_Vg_Node *root)
{
   efl_canvas_vg_object_root_node_set(obj, root);
}

EAPI Efl_Canvas_Vg_Node *
evas_object_vg_root_node_get(const Efl_Canvas_Vg_Object *obj)
{
   return efl_canvas_vg_object_root_node_get(obj);
}

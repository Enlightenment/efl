
EAPI void
evas_vg_node_transformation_set(Efl_Canvas_Vg_Node *obj, const Eina_Matrix3 *m)
{
   efl_canvas_vg_node_transformation_set(obj, m);
}

EAPI const Eina_Matrix3 *
evas_vg_node_transformation_get(const Efl_Canvas_Vg_Node *obj)
{
   return efl_canvas_vg_node_transformation_get(obj);
}

EAPI void
evas_vg_node_origin_set(Efl_Canvas_Vg_Node *obj, double x, double y)
{
   efl_canvas_vg_node_origin_set(obj, x, y);
}

EAPI void
evas_vg_node_origin_get(const Efl_Canvas_Vg_Node *obj, double *x, double *y)
{
   efl_canvas_vg_node_origin_get(obj, x, y);
}

EAPI void
evas_vg_node_mask_set(Efl_Canvas_Vg_Node *obj, Efl_Canvas_Vg_Node *mask, int op)
{
   efl_canvas_vg_node_mask_set(obj, mask, op);
}


EAPI void
evas_vg_node_transformation_set(Evas_Vg_Node *obj, const Eina_Matrix3 *m)
{
   efl_canvas_vg_node_transformation_set(obj, m);
}

EAPI const Eina_Matrix3 *
evas_vg_node_transformation_get(const Evas_Vg_Node *obj)
{
   return efl_canvas_vg_node_transformation_get(obj);
}

EAPI void
evas_vg_node_origin_set(Evas_Vg_Node *obj, double x, double y)
{
   efl_canvas_vg_node_origin_set(obj, x, y);
}

EAPI void
evas_vg_node_origin_get(const Evas_Vg_Node *obj, double *x, double *y)
{
   efl_canvas_vg_node_origin_get(obj, x, y);
}

EAPI void
evas_vg_node_mask_set(Evas_Vg_Node *obj, Evas_Vg_Node *mask, int op EINA_UNUSED)
{
   efl_canvas_vg_node_comp_method_set(obj, mask, 0);
}

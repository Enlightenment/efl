#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EFL_CANVAS_VG_SHAPE_CLASS

typedef struct _Efl_Canvas_Vg_Shape_Data Efl_Canvas_Vg_Shape_Data;
struct _Efl_Canvas_Vg_Shape_Data
{
   Efl_Canvas_Vg_Node *fill;

   struct {
      Efl_Canvas_Vg_Node *fill;
      Efl_Canvas_Vg_Node *marker;
   } stroke;
};

// FIXME: Use the renderer bounding box when it has been created instead of an estimation

static void
_efl_canvas_vg_shape_fill_set(Eo *obj EINA_UNUSED,
                       Efl_Canvas_Vg_Shape_Data *pd,
                       Efl_Canvas_Vg_Node *f)
{
   if (pd->fill == f) return;

   Efl_Canvas_Vg_Node *tmp = pd->fill;

   pd->fill = efl_ref(f);
   efl_unref(tmp);
}

static Efl_Canvas_Vg_Node *
_efl_canvas_vg_shape_fill_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Vg_Shape_Data *pd)
{
   return pd->fill;
}

static void
_efl_canvas_vg_shape_stroke_fill_set(Eo *obj EINA_UNUSED,
                              Efl_Canvas_Vg_Shape_Data *pd,
                              Efl_Canvas_Vg_Node *f)
{
   if (pd->stroke.fill == f) return;

   Efl_Canvas_Vg_Node *tmp = pd->stroke.fill;
   pd->stroke.fill = efl_ref(f);
   efl_unref(tmp);
}

static Efl_Canvas_Vg_Node *
_efl_canvas_vg_shape_stroke_fill_get(const Eo *obj EINA_UNUSED,
                              Efl_Canvas_Vg_Shape_Data *pd)
{
   return pd->stroke.fill;
}

static void
_efl_canvas_vg_shape_stroke_marker_set(Eo *obj EINA_UNUSED,
                                Efl_Canvas_Vg_Shape_Data *pd,
                                Efl_Canvas_Vg_Shape *m)
{
   Efl_Canvas_Vg_Node *tmp = pd->stroke.marker;

   pd->stroke.marker = efl_ref(m);
   efl_unref(tmp);
}

static Efl_Canvas_Vg_Shape *
_efl_canvas_vg_shape_stroke_marker_get(const Eo *obj EINA_UNUSED,
                                Efl_Canvas_Vg_Shape_Data *pd)
{
   return pd->stroke.marker;
}

static void
_efl_canvas_vg_shape_render_pre(Evas_Object_Protected_Data *vg_pd,
                                Efl_VG *obj,
                                Efl_Canvas_Vg_Node_Data *nd,
                                void *engine, void *output, void *context,
                                Ector_Surface *surface,
                                Eina_Matrix3 *ptransform,
                                int p_opacity,
                                Ector_Buffer *comp,
                                Efl_Gfx_Vg_Composite_Method comp_method,
                                void *data)
{
   Efl_Canvas_Vg_Shape_Data *pd = data;
   Efl_Canvas_Vg_Node_Data *fill, *stroke_fill, *stroke_marker;

   if (nd->flags == EFL_GFX_CHANGE_FLAG_NONE) return;

   nd->flags = EFL_GFX_CHANGE_FLAG_NONE;

   EFL_CANVAS_VG_COMPUTE_MATRIX(ctransform, ptransform, nd);
   EFL_CANVAS_VG_COMPUTE_ALPHA(c_r, c_g, c_b, c_a, p_opacity, nd);

   fill = _evas_vg_render_pre(vg_pd, pd->fill,
                              engine, output, context,
                              surface, ctransform, c_a, comp, comp_method);
   stroke_fill = _evas_vg_render_pre(vg_pd, pd->stroke.fill,
                                     engine, output, context,
                                     surface, ctransform, c_a, comp, comp_method);
   stroke_marker = _evas_vg_render_pre(vg_pd, pd->stroke.marker,
                                       engine, output, context,
                                       surface, ctransform, c_a, comp, comp_method);

   if (!nd->renderer)
     {
        efl_domain_current_push(EFL_ID_DOMAIN_SHARED);
        nd->renderer = ector_surface_renderer_factory_new(surface, ECTOR_RENDERER_SHAPE_MIXIN);
        efl_domain_current_pop();
     }
   ector_renderer_transformation_set(nd->renderer, ctransform);
   ector_renderer_origin_set(nd->renderer, nd->x, nd->y);
   ector_renderer_color_set(nd->renderer, c_r, c_g, c_b, c_a);
   ector_renderer_visibility_set(nd->renderer, nd->visibility);
   ector_renderer_shape_fill_set(nd->renderer, fill ? fill->renderer : NULL);
   ector_renderer_shape_stroke_fill_set(nd->renderer, stroke_fill ? stroke_fill->renderer : NULL);
   ector_renderer_shape_stroke_marker_set(nd->renderer, stroke_marker ? stroke_marker->renderer : NULL);
   efl_gfx_path_copy_from(nd->renderer, obj);
   efl_gfx_path_commit(nd->renderer);
   ector_renderer_prepare(nd->renderer);
   ector_renderer_comp_method_set(nd->renderer, comp, comp_method);
}

static Eo *
_efl_canvas_vg_shape_efl_object_constructor(Eo *obj, Efl_Canvas_Vg_Shape_Data *pd)
{
   Efl_Canvas_Vg_Node_Data *nd;

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   efl_gfx_shape_stroke_scale_set(obj, 1);
   efl_gfx_shape_stroke_location_set(obj, 0.5);
   efl_gfx_shape_stroke_cap_set(obj, EFL_GFX_CAP_BUTT);
   efl_gfx_shape_stroke_join_set(obj, EFL_GFX_JOIN_MITER);

   //NOTE: The default value is 4. It only refers to the standard of web svg.
   //      https://developer.mozilla.org/en-US/docs/Web/SVG/Attribute/stroke-miterlimit
   efl_gfx_shape_stroke_miterlimit_set(obj, 4);

   nd = efl_data_scope_get(obj, EFL_CANVAS_VG_NODE_CLASS);
   nd->render_pre = _efl_canvas_vg_shape_render_pre;
   nd->data = pd;

   return obj;
}

static void
_efl_canvas_vg_shape_efl_object_destructor(Eo *obj, Efl_Canvas_Vg_Shape_Data *pd)
{
   if (pd->fill) efl_unref(pd->fill);
   if (pd->stroke.fill) efl_unref(pd->stroke.fill);
   if (pd->stroke.marker) efl_unref(pd->stroke.marker);

   efl_gfx_path_reset(obj);
   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Bool
_efl_canvas_vg_shape_efl_gfx_path_interpolate(Eo *obj,
                                              Efl_Canvas_Vg_Shape_Data *pd,
                                              const Efl_Canvas_Vg_Node *from,
                                              const Efl_Canvas_Vg_Node *to,
                                              double pos_map)
{
   Efl_Canvas_Vg_Shape_Data *fromd, *tod;
   Eina_Bool r = EINA_TRUE;

   //Check if both objects have same type
   if (!(efl_isa(from, MY_CLASS) && efl_isa(to, MY_CLASS)))
     return EINA_FALSE;

   //Is this the best way?
   r &= efl_gfx_path_interpolate(efl_cast(obj, EFL_CANVAS_VG_NODE_CLASS),
                                 from, to, pos_map);
   r &= efl_gfx_path_interpolate(efl_super(obj, MY_CLASS), from, to, pos_map);

   fromd = efl_data_scope_get(from, MY_CLASS);
   tod = efl_data_scope_get(to, MY_CLASS);

   //Fill
   if (fromd->fill && tod->fill && pd->fill)
     r &= efl_gfx_path_interpolate(pd->fill, fromd->fill, tod->fill, pos_map);

   //Stroke Fill
   if (fromd->stroke.fill && tod->stroke.fill && pd->stroke.fill)
     r &= efl_gfx_path_interpolate(pd->stroke.fill, fromd->stroke.fill, tod->stroke.fill, pos_map);

   //Stroke Marker
   if (fromd->stroke.marker && tod->stroke.marker && pd->stroke.marker)
     r &= efl_gfx_path_interpolate(pd->stroke.marker, fromd->stroke.marker, tod->stroke.marker, pos_map);

   return r;
}

EOLIAN static void
_efl_canvas_vg_shape_efl_gfx_path_commit(Eo *obj,
                                         Efl_Canvas_Vg_Shape_Data *pd EINA_UNUSED)
{
   efl_canvas_vg_node_change(obj);
}

EOLIAN static Efl_Canvas_Vg_Node *
_efl_canvas_vg_shape_efl_duplicate_duplicate(const Eo *obj, Efl_Canvas_Vg_Shape_Data *pd)
{
   Efl_Canvas_Vg_Node *node;
   Efl_Canvas_Vg_Shape_Data *sd;

   node = efl_duplicate(efl_super(obj, MY_CLASS));
   sd = efl_data_scope_get(node, MY_CLASS);

   if (pd->fill)
     {
        sd->fill = efl_duplicate(pd->fill);
        efl_parent_set(sd->fill, efl_parent_get(node));
     }

   if (pd->stroke.fill)
     {
        sd->stroke.fill = efl_duplicate(pd->stroke.fill);
        efl_parent_set(sd->stroke.fill, efl_parent_get(node));
     }

   if (pd->stroke.marker)
     {
        sd->stroke.marker = efl_duplicate(pd->stroke.marker);
        efl_parent_set(sd->stroke.marker, efl_parent_get(node));
     }

   efl_gfx_path_copy_from(node, obj);

   return node;
}

EAPI double
evas_vg_shape_stroke_scale_get(Evas_Vg_Shape *obj)
{
   return efl_gfx_shape_stroke_scale_get(obj);
}

EAPI void
evas_vg_shape_stroke_scale_set(Evas_Vg_Shape *obj, double s)
{
   efl_gfx_shape_stroke_scale_set(obj, s);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_stroke_color_get(Evas_Vg_Shape *obj, int *r, int *g, int *b, int *a)
{
   efl_gfx_shape_stroke_color_get(obj, r, g, b, a);
}

EAPI void
evas_vg_shape_stroke_color_set(Evas_Vg_Shape *obj, int r, int g, int b, int a)
{
   efl_gfx_shape_stroke_color_set(obj, r, g, b, a);
   efl_canvas_vg_node_change(obj);
}

EAPI double
evas_vg_shape_stroke_width_get(Evas_Vg_Shape *obj)
{
   return efl_gfx_shape_stroke_width_get(obj);
}

EAPI void
evas_vg_shape_stroke_width_set(Evas_Vg_Shape *obj, double w)
{
   efl_gfx_shape_stroke_width_set(obj, w);
   efl_canvas_vg_node_change(obj);
}

EAPI double
evas_vg_shape_stroke_location_get(Evas_Vg_Shape *obj)
{
   return efl_gfx_shape_stroke_location_get(obj);
}

EAPI void
evas_vg_shape_stroke_location_set(Evas_Vg_Shape *obj, double centered)
{
   efl_gfx_shape_stroke_location_set(obj, centered);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_stroke_dash_get(Evas_Vg_Shape *obj, const Evas_Vg_Dash **dash, unsigned int *length)
{
   efl_gfx_shape_stroke_dash_get(obj, (const Efl_Gfx_Dash **)dash, length);
}

EAPI void
evas_vg_shape_stroke_dash_set(Evas_Vg_Shape *obj, const Evas_Vg_Dash *dash, unsigned int length)
{
   efl_gfx_shape_stroke_dash_set(obj, (const Efl_Gfx_Dash *)dash, length);
   efl_canvas_vg_node_change(obj);
}

EAPI Evas_Vg_Cap
evas_vg_shape_stroke_cap_get(Evas_Vg_Shape *obj)
{
   return (Evas_Vg_Cap)efl_gfx_shape_stroke_cap_get(obj);
}

EAPI void
evas_vg_shape_stroke_cap_set(Evas_Vg_Shape *obj, Evas_Vg_Cap c)
{
   efl_gfx_shape_stroke_cap_set(obj, (Efl_Gfx_Cap)c);
   efl_canvas_vg_node_change(obj);
}

EAPI Evas_Vg_Join
evas_vg_shape_stroke_join_get(Evas_Vg_Shape *obj)
{
   return (Evas_Vg_Join)efl_gfx_shape_stroke_join_get(obj);
}

EAPI void
evas_vg_shape_stroke_join_set(Evas_Vg_Shape *obj, Evas_Vg_Join j)
{
   efl_gfx_shape_stroke_join_set(obj, (Efl_Gfx_Join)j);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_path_set(Evas_Vg_Shape *obj, const Evas_Vg_Path_Command *op, const double *points)
{
   efl_gfx_path_set(obj, (const Efl_Gfx_Path_Command *)op, points);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_path_get(Evas_Vg_Shape *obj, const Evas_Vg_Path_Command **op, const double **points)
{
   efl_gfx_path_get(obj, (const Efl_Gfx_Path_Command **)op, points);
}

EAPI void
evas_vg_shape_path_length_get(Evas_Vg_Shape *obj, unsigned int *commands, unsigned int *points)
{
   efl_gfx_path_length_get(obj, commands, points);
}

EAPI void
evas_vg_shape_current_get(Evas_Vg_Shape *obj, double *x, double *y)
{
   efl_gfx_path_current_get(obj, x, y);
}

EAPI void
evas_vg_shape_current_ctrl_get(Evas_Vg_Shape *obj, double *x, double *y)
{
   efl_gfx_path_current_ctrl_get(obj, x, y);
}

EAPI void
evas_vg_shape_dup(Evas_Vg_Shape *obj, Evas_Vg_Shape *dup_from)
{
   efl_gfx_path_copy_from(obj, dup_from);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_reset(Evas_Vg_Shape *obj)
{
   efl_gfx_path_reset(obj);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_append_move_to(Evas_Vg_Shape *obj, double x, double y)
{
   efl_gfx_path_append_move_to(obj, x, y);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_append_line_to(Evas_Vg_Shape *obj, double x, double y)
{
   efl_gfx_path_append_line_to(obj, x, y);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_append_quadratic_to(Evas_Vg_Shape *obj, double x, double y, double ctrl_x, double ctrl_y)
{
   efl_gfx_path_append_quadratic_to(obj, x, y, ctrl_x, ctrl_y);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_append_squadratic_to(Evas_Vg_Shape *obj, double x, double y)
{
   efl_gfx_path_append_squadratic_to(obj, x, y);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_append_cubic_to(Evas_Vg_Shape *obj, double x, double y, double ctrl_x0, double ctrl_y0, double ctrl_x1, double ctrl_y1)
{
   efl_gfx_path_append_cubic_to(obj, ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1, x, y);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_append_scubic_to(Evas_Vg_Shape *obj, double x, double y, double ctrl_x, double ctrl_y)
{
   efl_gfx_path_append_scubic_to(obj, x, y, ctrl_x, ctrl_y);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_append_arc_to(Evas_Vg_Shape *obj, double x, double y, double rx, double ry, double angle, Eina_Bool large_arc, Eina_Bool sweep)
{
   efl_gfx_path_append_arc_to(obj, x, y, rx, ry, angle, large_arc, sweep);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_append_arc(Evas_Vg_Shape *obj, double x, double y, double w, double h, double start_angle, double sweep_length)
{
   efl_gfx_path_append_arc(obj, x, y, w, h, start_angle, sweep_length);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_append_close(Evas_Vg_Shape *obj)
{
   efl_gfx_path_append_close(obj);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_append_circle(Evas_Vg_Shape *obj, double x, double y, double radius)
{
   efl_gfx_path_append_circle(obj, x, y, radius);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_append_rect(Evas_Vg_Shape *obj, double x, double y, double w, double h, double rx, double ry)
{
   efl_gfx_path_append_rect(obj, x, y, w, h, rx, ry);
   efl_canvas_vg_node_change(obj);
}

EAPI void
evas_vg_shape_append_svg_path(Evas_Vg_Shape *obj, const char *svg_path_data)
{
   efl_gfx_path_append_svg_path(obj, svg_path_data);
   efl_canvas_vg_node_change(obj);
}

EAPI Eina_Bool
evas_vg_shape_interpolate(Evas_Vg_Shape *obj, const Evas_Vg_Shape *from, const Evas_Vg_Shape *to, double pos_map)
{
   Eina_Bool ret = efl_gfx_path_interpolate(obj, from, to, pos_map);
   efl_canvas_vg_node_change(obj);
   return ret;
}

EAPI Eina_Bool
evas_vg_shape_equal_commands(Evas_Vg_Shape *obj, const Evas_Vg_Shape *with)
{
   return efl_gfx_path_equal_commands(obj, with);
}

EAPI Efl_Canvas_Vg_Shape*
evas_vg_shape_add(Efl_Canvas_Vg_Node *parent)
{
   /* Warn it because the usage has been changed.
      We can remove this message after v1.21. */
   if (!parent)
     {
        ERR("Efl_Canvas_Vg_Shape only allow Efl_Canvas_Vg_Node as the parent");
        return NULL;
     }

   return efl_add(MY_CLASS, parent);
}

#include "efl_canvas_vg_shape.eo.c"
#include "efl_canvas_vg_shape_eo.legacy.c"

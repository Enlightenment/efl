#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EFL_VG_SHAPE_CLASS

typedef struct _Efl_VG_Shape_Data Efl_VG_Shape_Data;
struct _Efl_VG_Shape_Data
{
   Efl_VG *fill;

   struct {
      Efl_VG *fill;
      Efl_VG *marker;
   } stroke;
};

static void
_efl_vg_shape_efl_vg_bounds_get(Eo *obj,
                                    Efl_VG_Shape_Data *pd EINA_UNUSED,
                                    Eina_Rectangle *r)
{
   // FIXME: Use the renderer bounding box when it has been created instead of an estimation
   efl_gfx_shape_bounds_get(obj, r);
}

static void
_efl_vg_shape_fill_set(Eo *obj EINA_UNUSED,
                       Efl_VG_Shape_Data *pd,
                       Efl_VG *f)
{
   Efl_VG *tmp = pd->fill;

   pd->fill = efl_ref(f);
   efl_unref(tmp);

   _efl_vg_changed(obj);
}

static Efl_VG *
_efl_vg_shape_fill_get(Eo *obj EINA_UNUSED, Efl_VG_Shape_Data *pd)
{
   return pd->fill;
}

static Eina_Bool
_efl_vg_shape_efl_gfx_color_part_set(Eo *obj, Efl_VG_Shape_Data *pd EINA_UNUSED,
                                          const char * part,
                                          int r, int g, int b, int a)
{
   if (part && !strcmp(part, "stroke"))
     {
         efl_gfx_shape_stroke_color_set(obj, r, g, b, a);
        return EINA_TRUE;
     }

   return efl_gfx_color_part_set(efl_super(obj, EFL_VG_SHAPE_CLASS), part, r, g, b, a);
}

static Eina_Bool
_efl_vg_shape_efl_gfx_color_part_get(Eo *obj, Efl_VG_Shape_Data *pd EINA_UNUSED,
                                          const char * part,
                                          int *r, int *g, int *b, int *a)
{
   if (part && !strcmp(part, "stroke"))
     {
        efl_gfx_shape_stroke_color_get(obj, r, g, b, a);
        return EINA_TRUE;
     }

   return efl_gfx_color_part_get(efl_super(obj, EFL_VG_SHAPE_CLASS), part, r, g, b, a);
}

static void
_efl_vg_shape_stroke_fill_set(Eo *obj EINA_UNUSED,
                              Efl_VG_Shape_Data *pd,
                              Efl_VG *f)
{
   Efl_VG *tmp = pd->fill;

   pd->stroke.fill = efl_ref(f);
   efl_unref(tmp);

   _efl_vg_changed(obj);
}

static Efl_VG *
_efl_vg_shape_stroke_fill_get(Eo *obj EINA_UNUSED,
                              Efl_VG_Shape_Data *pd)
{
   return pd->stroke.fill;
}

static void
_efl_vg_shape_stroke_marker_set(Eo *obj EINA_UNUSED,
                                Efl_VG_Shape_Data *pd,
                                Efl_VG_Shape *m)
{
   Efl_VG *tmp = pd->stroke.marker;

   pd->stroke.marker = efl_ref(m);
   efl_unref(tmp);

   _efl_vg_changed(obj);
}

static Efl_VG_Shape *
_efl_vg_shape_stroke_marker_get(Eo *obj EINA_UNUSED,
                                Efl_VG_Shape_Data *pd)
{
   return pd->stroke.marker;
}

static void
_efl_vg_shape_render_pre(Eo *obj EINA_UNUSED,
                         Eina_Matrix3 *parent,
                         Ector_Surface *s,
                         void *data,
                         Efl_VG_Data *nd)
{
   Efl_VG_Shape_Data *pd = data;
   Efl_VG_Data *fill, *stroke_fill, *stroke_marker, *mask;

   if (nd->flags == EFL_GFX_CHANGE_FLAG_NONE) return ;

   nd->flags = EFL_GFX_CHANGE_FLAG_NONE;

   EFL_VG_COMPUTE_MATRIX(current, parent, nd);

   fill = _evas_vg_render_pre(pd->fill, s, current);
   stroke_fill = _evas_vg_render_pre(pd->stroke.fill, s, current);
   stroke_marker = _evas_vg_render_pre(pd->stroke.marker, s, current);
   mask = _evas_vg_render_pre(nd->mask, s, current);

   if (!nd->renderer)
     {
        nd->renderer = ector_surface_renderer_factory_new(s, ECTOR_RENDERER_SHAPE_MIXIN);
     }

   ector_renderer_transformation_set(nd->renderer, current);
   ector_renderer_origin_set(nd->renderer, nd->x, nd->y);
   ector_renderer_color_set(nd->renderer, nd->r, nd->g, nd->b, nd->a);
   ector_renderer_visibility_set(nd->renderer, nd->visibility);
   ector_renderer_mask_set(nd->renderer, mask ? mask->renderer : NULL);
   ector_renderer_shape_fill_set(nd->renderer, fill ? fill->renderer : NULL);
   ector_renderer_shape_stroke_fill_set(nd->renderer, stroke_fill ? stroke_fill->renderer : NULL);
   ector_renderer_shape_stroke_marker_set(nd->renderer, stroke_marker ? stroke_marker->renderer : NULL);
   efl_gfx_shape_dup(nd->renderer, obj);
   ector_renderer_prepare(nd->renderer);
}

static Eo *
_efl_vg_shape_efl_object_constructor(Eo *obj, Efl_VG_Shape_Data *pd)
{
   Efl_VG_Data *nd;

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   efl_gfx_shape_stroke_scale_set(obj, 1);
   efl_gfx_shape_stroke_location_set(obj, 0.5);
   efl_gfx_shape_stroke_cap_set(obj, EFL_GFX_CAP_BUTT);
   efl_gfx_shape_stroke_join_set(obj, EFL_GFX_JOIN_MITER);

   nd = efl_data_scope_get(obj, EFL_VG_CLASS);
   nd->render_pre = _efl_vg_shape_render_pre;
   nd->data = pd;

   return obj;
}

static void
_efl_vg_shape_efl_object_destructor(Eo *obj, Efl_VG_Shape_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Bool
_efl_vg_shape_efl_vg_interpolate(Eo *obj,
                                      Efl_VG_Shape_Data *pd,
                                      const Efl_VG *from, const Efl_VG *to,
                                      double pos_map)
{
   Efl_VG_Shape_Data *fromd, *tod;
   Eina_Bool r;

   fromd = efl_data_scope_get(from, EFL_VG_SHAPE_CLASS);
   tod = efl_data_scope_get(to, EFL_VG_SHAPE_CLASS);

   r = efl_vg_interpolate(efl_super(obj, MY_CLASS), from, to, pos_map);

   r &= efl_gfx_shape_interpolate(obj, from, to, pos_map);

   if (fromd->fill && tod->fill && pd->fill)
     {
        r &= efl_vg_interpolate(pd->fill, fromd->fill, tod->fill, pos_map);
     }
   if (fromd->stroke.fill && tod->stroke.fill && pd->stroke.fill)
     {
        r &= efl_vg_interpolate(pd->stroke.fill, fromd->stroke.fill, tod->stroke.fill, pos_map);
     }
   if (fromd->stroke.marker && tod->stroke.marker && pd->stroke.marker)
     {
        r &= efl_vg_interpolate(pd->stroke.marker, fromd->stroke.marker, tod->stroke.marker, pos_map);
     }

   return r;
}

static void
_efl_vg_shape_efl_vg_dup(Eo *obj, Efl_VG_Shape_Data *pd EINA_UNUSED, const Efl_VG *from)
{
   Efl_VG_Shape_Data *fromd;
   Eo *fill = NULL, *stroke_fill = NULL, *stroke_marker = NULL;

   efl_vg_dup(efl_super(obj, MY_CLASS), from);

   fromd = efl_data_scope_get(from, MY_CLASS);

   if (fromd->fill)
     {
        fill = efl_add(efl_class_get(fromd->fill), NULL, efl_vg_dup(efl_added, fromd->fill));
        efl_vg_shape_fill_set(obj, fill);
        efl_unref(fill);
     }

   if (fromd->stroke.fill)
     {
        stroke_fill = efl_add(efl_class_get(fromd->stroke.fill), NULL, efl_vg_dup(efl_added, fromd->stroke.fill));
        efl_vg_shape_stroke_fill_set(obj, stroke_fill);
        efl_unref(stroke_fill);
     }

   if (fromd->stroke.marker)
     {
        stroke_marker = efl_add(efl_class_get(fromd->stroke.marker), NULL, efl_vg_dup(efl_added, fromd->stroke.marker));
        efl_vg_shape_stroke_marker_set(obj, stroke_marker);
        efl_unref(stroke_marker);
     }

   efl_gfx_shape_dup(obj, from);
}

EAPI double
evas_vg_shape_stroke_scale_get(Eo *obj)
{
   return efl_gfx_shape_stroke_scale_get(obj);
}

EAPI void
evas_vg_shape_stroke_scale_set(Eo *obj, double s)
{
   efl_gfx_shape_stroke_scale_set(obj, s);
}

EAPI void
evas_vg_shape_stroke_color_get(Eo *obj, int *r, int *g, int *b, int *a)
{
   efl_gfx_shape_stroke_color_get(obj, r, g, b, a);
}

EAPI void
evas_vg_shape_stroke_color_set(Eo *obj, int r, int g, int b, int a)
{
   efl_gfx_shape_stroke_color_set(obj, r, g, b, a);
}

EAPI double
evas_vg_shape_stroke_width_get(Eo *obj)
{
   return efl_gfx_shape_stroke_width_get(obj);
}

EAPI void
evas_vg_shape_stroke_width_set(Eo *obj, double w)
{
   efl_gfx_shape_stroke_width_set(obj, w);
}

EAPI double
evas_vg_shape_stroke_location_get(Eo *obj)
{
   return efl_gfx_shape_stroke_location_get(obj);
}

EAPI void
evas_vg_shape_stroke_location_set(Eo *obj, double centered)
{
   efl_gfx_shape_stroke_location_set(obj, centered);
}

EAPI void
evas_vg_shape_stroke_dash_get(Eo *obj, const Efl_Gfx_Dash **dash, unsigned int *length)
{
   efl_gfx_shape_stroke_dash_get(obj, dash, length);
}

EAPI void
evas_vg_shape_stroke_dash_set(Eo *obj, const Efl_Gfx_Dash *dash, unsigned int length)
{
   efl_gfx_shape_stroke_dash_set(obj, dash, length);
}

EAPI Efl_Gfx_Cap
evas_vg_shape_stroke_cap_get(Eo *obj)
{
   return efl_gfx_shape_stroke_cap_get(obj);
}

EAPI void
evas_vg_shape_stroke_cap_set(Eo *obj, Efl_Gfx_Cap c)
{
   efl_gfx_shape_stroke_cap_set(obj, c);
}

EAPI Efl_Gfx_Join
evas_vg_shape_stroke_join_get(Eo *obj)
{
   return efl_gfx_shape_stroke_join_get(obj);
}

EAPI void
evas_vg_shape_stroke_join_set(Eo *obj, Efl_Gfx_Join j)
{
   efl_gfx_shape_stroke_join_set(obj, j);
}

EAPI void
evas_vg_shape_shape_path_set(Eo *obj, const Efl_Gfx_Path_Command *op, const double *points)
{
   efl_gfx_shape_path_set(obj, op, points);
}

EAPI void
evas_vg_shape_shape_path_get(Eo *obj, const Efl_Gfx_Path_Command **op, const double **points)
{
   efl_gfx_shape_path_get(obj, op, points);
}

EAPI void
evas_vg_shape_shape_path_length_get(Eo *obj, unsigned int *commands, unsigned int *points)
{
   efl_gfx_shape_path_length_get(obj, commands, points);
}

EAPI void
evas_vg_shape_shape_current_get(Eo *obj, double *x, double *y)
{
   efl_gfx_shape_current_get(obj, x, y);
}

EAPI void
evas_vg_shape_shape_current_ctrl_get(Eo *obj, double *x, double *y)
{
   efl_gfx_shape_current_ctrl_get(obj, x, y);
}

EAPI void
evas_vg_shape_shape_dup(Eo *obj, Eo *dup_from)
{
   efl_gfx_shape_dup(obj, dup_from);
}

EAPI void
evas_vg_shape_shape_reset(Eo *obj)
{
   efl_gfx_shape_reset(obj);
}

EAPI void
evas_vg_shape_shape_append_move_to(Eo *obj, double x, double y)
{
   efl_gfx_shape_append_move_to(obj, x, y);
}

EAPI void
evas_vg_shape_shape_append_line_to(Eo *obj, double x, double y)
{
   efl_gfx_shape_append_line_to(obj, x, y);
}

EAPI void
evas_vg_shape_shape_append_quadratic_to(Eo *obj, double x, double y, double ctrl_x, double ctrl_y)
{
   efl_gfx_shape_append_quadratic_to(obj, x, y, ctrl_x, ctrl_y);
}

EAPI void
evas_vg_shape_shape_append_squadratic_to(Eo *obj, double x, double y)
{
   efl_gfx_shape_append_squadratic_to(obj, x, y);
}

EAPI void
evas_vg_shape_shape_append_cubic_to(Eo *obj, double x, double y, double ctrl_x0, double ctrl_y0, double ctrl_x1, double ctrl_y1)
{
   efl_gfx_shape_append_cubic_to(obj, x, y, ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1);
}

EAPI void
evas_vg_shape_shape_append_scubic_to(Eo *obj, double x, double y, double ctrl_x, double ctrl_y)
{
   efl_gfx_shape_append_scubic_to(obj, x, y, ctrl_x, ctrl_y);
}

EAPI void
evas_vg_shape_shape_append_arc_to(Eo *obj, double x, double y, double rx, double ry, double angle, Eina_Bool large_arc, Eina_Bool sweep)
{
   efl_gfx_shape_append_arc_to(obj, x, y, rx, ry, angle, large_arc, sweep);
}

EAPI void
evas_vg_shape_shape_append_arc(Eo *obj, double x, double y, double w, double h, double start_angle, double sweep_length)
{
   efl_gfx_shape_append_arc(obj, x, y, w, h, start_angle, sweep_length);
}

EAPI void
evas_vg_shape_shape_append_close(Eo *obj)
{
   efl_gfx_shape_append_close(obj);
}

EAPI void
evas_vg_shape_shape_append_circle(Eo *obj, double x, double y, double radius)
{
   efl_gfx_shape_append_circle(obj, x, y, radius);
}

EAPI void
evas_vg_shape_shape_append_rect(Eo *obj, double x, double y, double w, double h, double rx, double ry)
{
   efl_gfx_shape_append_rect(obj, x, y, w, h, rx, ry);
}

EAPI void
evas_vg_shape_shape_append_svg_path(Eo *obj, const char *svg_path_data)
{
   efl_gfx_shape_append_svg_path(obj, svg_path_data);
}

EAPI Eina_Bool
evas_vg_shape_shape_interpolate(Eo *obj, const Eo *from, const Eo *to, double pos_map)
{
   return efl_gfx_shape_interpolate(obj, from, to, pos_map);
}

EAPI Eina_Bool
evas_vg_shape_shape_equal_commands(Eo *obj, const Eo *with)
{
   return efl_gfx_shape_equal_commands(obj, with);
}

EAPI Efl_VG*
evas_vg_shape_add(Efl_VG *parent)
{
   return efl_add(EFL_VG_SHAPE_CLASS, parent);
}

#include "efl_vg_shape.eo.c"

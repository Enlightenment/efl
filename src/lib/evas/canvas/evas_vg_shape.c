#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EFL_VG_SHAPE_CLASS

typedef struct _Efl_VG_Shape_Data Efl_VG_Shape_Data;
struct _Efl_VG_Shape_Data
{
   Efl_VG *fill;

   struct {
      Efl_Gfx_Dash *dash;
      Efl_VG *fill;
      Efl_VG *marker;

      double scale;
      double width;
      double centered; // from 0 to 1

      int r, g, b, a;

      unsigned int dash_count;

      Efl_Gfx_Cap cap;
      Efl_Gfx_Join join;
   } stroke;
};

static Eina_Bool
_efl_vg_shape_efl_vg_base_bound_get(Eo *obj,
                                    Efl_VG_Shape_Data *pd EINA_UNUSED,
                                    Eina_Rectangle *r)
{
   // FIXME: Use the renderer bounding box when it has been created instead of an estimation
   eo_do(obj, efl_gfx_shape_bounding_box_get(r));
   return EINA_TRUE;
}

static void
_efl_vg_shape_fill_set(Eo *obj EINA_UNUSED,
                       Efl_VG_Shape_Data *pd,
                       Efl_VG *f)
{
   Efl_VG *tmp = pd->fill;

   pd->fill = eo_ref(f);
   eo_unref(tmp);

   _efl_vg_base_changed(obj);
}

static Efl_VG *
_efl_vg_shape_fill_get(Eo *obj EINA_UNUSED, Efl_VG_Shape_Data *pd)
{
   return pd->fill;
}

static void
_efl_vg_shape_efl_gfx_shape_stroke_scale_set(Eo *obj EINA_UNUSED,
                                             Efl_VG_Shape_Data *pd,
                                             double s)
{
   pd->stroke.scale = s;

   _efl_vg_base_changed(obj);
}

static double
_efl_vg_shape_efl_gfx_shape_stroke_scale_get(Eo *obj EINA_UNUSED,
                                             Efl_VG_Shape_Data *pd)
{
   return pd->stroke.scale;
}

static void
_efl_vg_shape_efl_gfx_shape_stroke_color_set(Eo *obj EINA_UNUSED,
                                             Efl_VG_Shape_Data *pd,
                                             int r, int g, int b, int a)
{
   pd->stroke.r = r;
   pd->stroke.g = g;
   pd->stroke.b = b;
   pd->stroke.a = a;

   _efl_vg_base_changed(obj);
}

static Eina_Bool
_efl_vg_shape_efl_gfx_base_color_part_set(Eo *obj, Efl_VG_Shape_Data *pd,
                                          const char * part,
                                          int r, int g, int b, int a)
{
   Eina_Bool ret;

   if (part && !strcmp(part, "stroke"))
     {
        _efl_vg_shape_efl_gfx_shape_stroke_color_set(obj, pd, r, g, b, a);
        return EINA_TRUE;
     }

   eo_do_super(obj, EFL_VG_SHAPE_CLASS,
               ret = efl_gfx_color_part_set(part, r, g, b, a));

   return ret;
}

static void
_efl_vg_shape_efl_gfx_shape_stroke_color_get(Eo *obj EINA_UNUSED,
                                             Efl_VG_Shape_Data *pd,
                                             int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->stroke.r;
   if (g) *g = pd->stroke.g;
   if (b) *b = pd->stroke.b;
   if (a) *a = pd->stroke.a;
}

static Eina_Bool
_efl_vg_shape_efl_gfx_base_color_part_get(Eo *obj, Efl_VG_Shape_Data *pd,
                                          const char * part,
                                          int *r, int *g, int *b, int *a)
{
   Eina_Bool ret;

   if (part && !strcmp(part, "stroke"))
     {
        _efl_vg_shape_efl_gfx_shape_stroke_color_get(obj, pd, r, g, b, a);
        return EINA_TRUE;
     }

   eo_do_super(obj, EFL_VG_SHAPE_CLASS,
               ret = efl_gfx_color_part_get(part, r, g, b, a));

   return ret;
}

static void
_efl_vg_shape_stroke_fill_set(Eo *obj EINA_UNUSED,
                              Efl_VG_Shape_Data *pd,
                              Efl_VG *f)
{
   Efl_VG *tmp = pd->fill;

   pd->stroke.fill = eo_ref(f);
   eo_unref(tmp);

   _efl_vg_base_changed(obj);
}

static Efl_VG *
_efl_vg_shape_stroke_fill_get(Eo *obj EINA_UNUSED,
                              Efl_VG_Shape_Data *pd)
{
   return pd->stroke.fill;
}

static void
_efl_vg_shape_efl_gfx_shape_stroke_width_set(Eo *obj EINA_UNUSED,
                                             Efl_VG_Shape_Data *pd,
                                             double w)
{
   pd->stroke.width = w;

   _efl_vg_base_changed(obj);
}

static double
_efl_vg_shape_efl_gfx_shape_stroke_width_get(Eo *obj EINA_UNUSED,
                                             Efl_VG_Shape_Data *pd)
{
   return pd->stroke.width;
}

static void
_efl_vg_shape_efl_gfx_shape_stroke_location_set(Eo *obj EINA_UNUSED,
                                                Efl_VG_Shape_Data *pd,
                                                double centered)
{
   pd->stroke.centered = centered;

   _efl_vg_base_changed(obj);
}

static double
_efl_vg_shape_efl_gfx_shape_stroke_location_get(Eo *obj EINA_UNUSED,
                                                Efl_VG_Shape_Data *pd)
{
   return pd->stroke.centered;
}

static void
_efl_vg_shape_efl_gfx_shape_stroke_dash_set(Eo *obj EINA_UNUSED,
                                            Efl_VG_Shape_Data *pd,
                                            const Efl_Gfx_Dash *dash,
                                            unsigned int length)
{
   free(pd->stroke.dash);
   pd->stroke.dash = NULL;
   pd->stroke.dash_count = 0;

   pd->stroke.dash = malloc(sizeof (Efl_Gfx_Dash) * length);
   if (!pd->stroke.dash) return ;

   memcpy(pd->stroke.dash, dash, sizeof (Efl_Gfx_Dash) * length);
   pd->stroke.dash_count = length;

   _efl_vg_base_changed(obj);
}

static void
_efl_vg_shape_efl_gfx_shape_stroke_dash_get(Eo *obj EINA_UNUSED,
                                            Efl_VG_Shape_Data *pd,
                                            const Efl_Gfx_Dash **dash,
                                            unsigned int *length)
{
   if (dash) *dash = pd->stroke.dash;
   if (length) *length = pd->stroke.dash_count;
}

static void
_efl_vg_shape_stroke_marker_set(Eo *obj EINA_UNUSED,
                                Efl_VG_Shape_Data *pd,
                                Efl_VG_Shape *m)
{
   Efl_VG *tmp = pd->stroke.marker;

   pd->stroke.marker = eo_ref(m);
   eo_unref(tmp);

   _efl_vg_base_changed(obj);
}

static Efl_VG_Shape *
_efl_vg_shape_stroke_marker_get(Eo *obj EINA_UNUSED,
                                Efl_VG_Shape_Data *pd)
{
   return pd->stroke.marker;
}

static void
_efl_vg_shape_efl_gfx_shape_stroke_cap_set(Eo *obj EINA_UNUSED,
                                           Efl_VG_Shape_Data *pd,
                                           Efl_Gfx_Cap c)
{
   pd->stroke.cap = c;

   _efl_vg_base_changed(obj);
}

static Efl_Gfx_Cap
_efl_vg_shape_efl_gfx_shape_stroke_cap_get(Eo *obj EINA_UNUSED,
                                           Efl_VG_Shape_Data *pd)
{
   return pd->stroke.cap;
}

static void
_efl_vg_shape_efl_gfx_shape_stroke_join_set(Eo *obj EINA_UNUSED,
                                            Efl_VG_Shape_Data *pd,
                                            Efl_Gfx_Join j)
{
   pd->stroke.join = j;

   _efl_vg_base_changed(obj);
}

static Efl_Gfx_Join
_efl_vg_shape_efl_gfx_shape_stroke_join_get(Eo *obj EINA_UNUSED,
                                            Efl_VG_Shape_Data *pd)
{
   return pd->stroke.join;
}

static void
_efl_vg_shape_render_pre(Eo *obj EINA_UNUSED,
                         Eina_Matrix3 *parent,
                         Ector_Surface *s,
                         void *data,
                         Efl_VG_Base_Data *nd)
{
   Efl_VG_Shape_Data *pd = data;
   Efl_VG_Base_Data *fill, *stroke_fill, *stroke_marker, *mask;
   double xn = nd->x, yn = nd->y ;

   if (!nd->changed) return ;
   nd->changed = EINA_FALSE;

   if(parent) eina_matrix3_point_transform(parent, nd->x, nd->y, &xn, &yn);

   EFL_VG_COMPUTE_MATRIX(current, parent, nd);

   fill = _evas_vg_render_pre(pd->fill, s, current);
   stroke_fill = _evas_vg_render_pre(pd->stroke.fill, s, current);
   stroke_marker = _evas_vg_render_pre(pd->stroke.marker, s, current);
   mask = _evas_vg_render_pre(nd->mask, s, current);

   if (!nd->renderer)
     {
        eo_do(s, nd->renderer = ector_surface_renderer_factory_new(ECTOR_RENDERER_GENERIC_SHAPE_MIXIN));
     }

   eo_do(nd->renderer,
         ector_renderer_transformation_set(current),
         ector_renderer_origin_set(xn, yn),
         ector_renderer_color_set(nd->r, nd->g, nd->b, nd->a),
         ector_renderer_visibility_set(nd->visibility),
         ector_renderer_mask_set(mask ? mask->renderer : NULL),
         ector_renderer_shape_fill_set(fill ? fill->renderer : NULL),
         ector_renderer_shape_stroke_fill_set(stroke_fill ? stroke_fill->renderer : NULL),
         ector_renderer_shape_stroke_marker_set(stroke_marker ? stroke_marker->renderer : NULL),
         efl_gfx_shape_dup(obj),
         ector_renderer_prepare());
}

static void
_efl_vg_shape_eo_base_constructor(Eo *obj, Efl_VG_Shape_Data *pd)
{
   Efl_VG_Base_Data *nd;

   eo_do_super(obj, MY_CLASS, eo_constructor());

   pd->stroke.cap = EFL_GFX_CAP_BUTT;
   pd->stroke.join = EFL_GFX_JOIN_MITER;
   pd->stroke.scale = 1;
   pd->stroke.centered = 0.5;

   nd = eo_data_scope_get(obj, EFL_VG_BASE_CLASS);
   nd->render_pre = _efl_vg_shape_render_pre;
   nd->data = pd;
}

static void
_efl_vg_shape_eo_base_destructor(Eo *obj, Efl_VG_Shape_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

EAPI double
evas_vg_shape_stroke_scale_get(Eo *obj)
{
   double ret;

   return eo_do_ret(obj, ret, efl_gfx_shape_stroke_scale_get());
}

EAPI void
evas_vg_shape_stroke_scale_set(Eo *obj, double s)
{
   eo_do(obj, efl_gfx_shape_stroke_scale_set(s));
}

EAPI void
evas_vg_shape_stroke_color_get(Eo *obj, int *r, int *g, int *b, int *a)
{
   eo_do(obj, efl_gfx_shape_stroke_color_get(r, g, b, a));
}

EAPI void
evas_vg_shape_stroke_color_set(Eo *obj, int r, int g, int b, int a)
{
   eo_do(obj, efl_gfx_shape_stroke_color_set(r, g, b, a));
}

EAPI double
evas_vg_shape_stroke_width_get(Eo *obj)
{
   double ret;

   return eo_do_ret(obj, ret, efl_gfx_shape_stroke_width_get());
}

EAPI void
evas_vg_shape_stroke_width_set(Eo *obj, double w)
{
   eo_do(obj, efl_gfx_shape_stroke_width_set(w));
}

EAPI double
evas_vg_shape_stroke_location_get(Eo *obj)
{
   double ret;

   return eo_do_ret(obj, ret, efl_gfx_shape_stroke_location_get());
}

EAPI void
evas_vg_shape_stroke_location_set(Eo *obj, double centered)
{
   eo_do(obj, efl_gfx_shape_stroke_location_set(centered));
}

EAPI void
evas_vg_shape_stroke_dash_get(Eo *obj, const Efl_Gfx_Dash **dash, unsigned int *length)
{
   eo_do(obj, efl_gfx_shape_stroke_dash_get(dash, length));
}

EAPI void
evas_vg_shape_stroke_dash_set(Eo *obj, const Efl_Gfx_Dash *dash, unsigned int length)
{
   eo_do(obj, efl_gfx_shape_stroke_dash_set(dash, length));
}

EAPI Efl_Gfx_Cap
evas_vg_shape_stroke_cap_get(Eo *obj)
{
   Efl_Gfx_Cap ret;

   return eo_do_ret(obj, ret, efl_gfx_shape_stroke_cap_get());
}

EAPI void
evas_vg_shape_stroke_cap_set(Eo *obj, Efl_Gfx_Cap c)
{
   eo_do(obj, efl_gfx_shape_stroke_cap_set(c));
}

EAPI Efl_Gfx_Join
evas_vg_shape_stroke_join_get(Eo *obj)
{
   Efl_Gfx_Join ret;

   return eo_do_ret(obj, ret, efl_gfx_shape_stroke_join_get());
}

EAPI void
evas_vg_shape_stroke_join_set(Eo *obj, Efl_Gfx_Join j)
{
   eo_do(obj, efl_gfx_shape_stroke_join_set(j));
}

EAPI void
evas_vg_shape_shape_path_set(Eo *obj, const Efl_Gfx_Path_Command *op, const double *points)
{
   eo_do(obj, efl_gfx_shape_path_set(op, points));
}

EAPI void
evas_vg_shape_shape_path_get(Eo *obj, const Efl_Gfx_Path_Command **op, const double **points)
{
   eo_do(obj, efl_gfx_shape_path_get(op, points));
}

EAPI void
evas_vg_shape_shape_path_length_get(Eo *obj, unsigned int *commands, unsigned int *points)
{
   eo_do(obj, efl_gfx_shape_path_length_get(commands, points));
}

EAPI void
evas_vg_shape_shape_current_get(Eo *obj, double *x, double *y)
{
   eo_do(obj, efl_gfx_shape_current_get(x, y));
}

EAPI void
evas_vg_shape_shape_current_ctrl_get(Eo *obj, double *x, double *y)
{
   eo_do(obj, efl_gfx_shape_current_ctrl_get(x, y));
}

EAPI void
evas_vg_shape_shape_dup(Eo *obj, Eo *dup_from)
{
   eo_do(obj, efl_gfx_shape_dup(dup_from));
}

EAPI void
evas_vg_shape_shape_reset(Eo *obj)
{
   eo_do(obj, efl_gfx_shape_reset());
}

EAPI void
evas_vg_shape_shape_append_move_to(Eo *obj, double x, double y)
{
   eo_do(obj, efl_gfx_shape_append_move_to(x, y));
}

EAPI void
evas_vg_shape_shape_append_line_to(Eo *obj, double x, double y)
{
   eo_do(obj, efl_gfx_shape_append_line_to(x, y));
}

EAPI void
evas_vg_shape_shape_append_quadratic_to(Eo *obj, double x, double y, double ctrl_x, double ctrl_y)
{
   eo_do(obj, efl_gfx_shape_append_quadratic_to(x, y, ctrl_x, ctrl_y));
}

EAPI void
evas_vg_shape_shape_append_squadratic_to(Eo *obj, double x, double y)
{
   eo_do(obj, efl_gfx_shape_append_squadratic_to(x, y));
}

EAPI void
evas_vg_shape_shape_append_cubic_to(Eo *obj, double x, double y, double ctrl_x0, double ctrl_y0, double ctrl_x1, double ctrl_y1)
{
   eo_do(obj, efl_gfx_shape_append_cubic_to(x, y, ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1));
}

EAPI void
evas_vg_shape_shape_append_scubic_to(Eo *obj, double x, double y, double ctrl_x, double ctrl_y)
{
   eo_do(obj, efl_gfx_shape_append_scubic_to(x, y, ctrl_x, ctrl_y));
}

EAPI void
evas_vg_shape_shape_append_arc_to(Eo *obj, double x, double y, double rx, double ry, double angle, Eina_Bool large_arc, Eina_Bool sweep)
{
   eo_do(obj, efl_gfx_shape_append_arc_to(x, y, rx, ry, angle, large_arc, sweep));
}

EAPI void
evas_vg_shape_shape_append_close(Eo *obj)
{
   eo_do(obj, efl_gfx_shape_append_close());
}

EAPI void
evas_vg_shape_shape_append_circle(Eo *obj, double x, double y, double radius)
{
   eo_do(obj, efl_gfx_shape_append_circle(x, y, radius));
}

EAPI void
evas_vg_shape_shape_append_rect(Eo *obj, double x, double y, double w, double h, double rx, double ry)
{
   eo_do(obj, efl_gfx_shape_append_rect(x, y, w, h, rx, ry));
}

EAPI void
evas_vg_shape_shape_append_svg_path(Eo *obj, const char *svg_path_data)
{
   eo_do(obj, efl_gfx_shape_append_svg_path(svg_path_data));
}

EAPI Eina_Bool
evas_vg_shape_shape_interpolate(Eo *obj, const Eo *from, const Eo *to, double pos_map)
{
   Eina_Bool ret;

   return eo_do_ret(obj, ret, efl_gfx_shape_interpolate(from, to, pos_map));
}

EAPI Eina_Bool
evas_vg_shape_shape_equal_commands(Eo *obj, const Eo *with)
{
   Eina_Bool ret;

   return eo_do_ret(obj, ret, efl_gfx_shape_equal_commands(with));
}

EAPI Efl_VG*
evas_vg_shape_add(Efl_VG *parent)
{
   return eo_add(EFL_VG_SHAPE_CLASS, parent);
}

#include "efl_vg_shape.eo.c"

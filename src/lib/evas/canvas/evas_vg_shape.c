#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EVAS_VG_SHAPE_CLASS

typedef struct _Evas_VG_Shape_Data Evas_VG_Shape_Data;
struct _Evas_VG_Shape_Data
{
   Efl_Graphics_Path_Command *ops;
   double *points;

   Evas_VG_Node *fill;

   struct {
      Efl_Graphics_Dash *dash;
      Evas_VG_Node *fill;
      Evas_VG_Node *marker;

      double scale;
      double width;
      double centered; // from 0 to 1

      int r, g, b, a;

      unsigned int dash_count;

      Efl_Graphics_Cap cap;
      Efl_Graphics_Join join;
   } stroke;
};

static Eina_Bool
_evas_vg_shape_efl_graphics_shape_path_set(Eo *obj EINA_UNUSED,
                                           Evas_VG_Shape_Data *pd,
                                           const Efl_Graphics_Path_Command *ops,
                                           const double *points)
{
   free(pd->points);
   pd->points = NULL;
   free(pd->ops);
   pd->ops = NULL;

   return efl_graphics_path_dup(&pd->ops, &pd->points, ops, points);
}

static Eina_Bool
_evas_vg_shape_evas_vg_node_bound_get(Eo *obj,
                                      Evas_VG_Shape_Data *pd,
                                      Eina_Rectangle *r)
{
   return EINA_FALSE;
}

static void
_evas_vg_shape_fill_set(Eo *obj EINA_UNUSED,
                        Evas_VG_Shape_Data *pd,
                        Evas_VG_Node *f)
{
   Evas_VG_Node *tmp = pd->fill;

   pd->fill = eo_ref(f);
   eo_unref(tmp);
}

static Evas_VG_Node *
_evas_vg_shape_fill_get(Eo *obj EINA_UNUSED, Evas_VG_Shape_Data *pd)
{
   return pd->fill;
}

static void
_evas_vg_shape_efl_graphics_shape_stroke_scale_set(Eo *obj EINA_UNUSED,
                                                   Evas_VG_Shape_Data *pd,
                                                   double s)
{
   pd->stroke.scale = s;
}

static double
_evas_vg_shape_efl_graphics_shape_stroke_scale_get(Eo *obj EINA_UNUSED,
                                                   Evas_VG_Shape_Data *pd)
{
   return pd->stroke.scale;
}

static void
_evas_vg_shape_efl_graphics_shape_stroke_color_set(Eo *obj EINA_UNUSED,
                                                   Evas_VG_Shape_Data *pd,
                                                   int r, int g, int b, int a)
{
   pd->stroke.r = r;
   pd->stroke.g = g;
   pd->stroke.b = b;
   pd->stroke.a = a;
}

static void
_evas_vg_shape_efl_graphics_shape_stroke_color_get(Eo *obj EINA_UNUSED,
                                                   Evas_VG_Shape_Data *pd,
                                                   int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->stroke.r;
   if (g) *g = pd->stroke.g;
   if (b) *b = pd->stroke.b;
   if (a) *a = pd->stroke.a;
}

static void
_evas_vg_shape_stroke_fill_set(Eo *obj EINA_UNUSED,
                               Evas_VG_Shape_Data *pd,
                               Evas_VG_Node *f)
{
   Evas_VG_Node *tmp = pd->fill;

   pd->stroke.fill = eo_ref(f);
   eo_unref(tmp);
}

static Evas_VG_Node *
_evas_vg_shape_stroke_fill_get(Eo *obj EINA_UNUSED,
                               Evas_VG_Shape_Data *pd)
{
   return pd->stroke.fill;
}

static void
_evas_vg_shape_efl_graphics_shape_stroke_width_set(Eo *obj EINA_UNUSED,
                                                   Evas_VG_Shape_Data *pd,
                                                   double w)
{
   pd->stroke.width = w;
}

static double
_evas_vg_shape_efl_graphics_shape_stroke_width_get(Eo *obj EINA_UNUSED,
                                                   Evas_VG_Shape_Data *pd)
{
   return pd->stroke.width;
}

static void
_evas_vg_shape_efl_graphics_shape_stroke_location_set(Eo *obj EINA_UNUSED,
                                                      Evas_VG_Shape_Data *pd,
                                                      double centered)
{
   pd->stroke.centered = centered;
}

static double
_evas_vg_shape_efl_graphics_shape_stroke_location_get(Eo *obj EINA_UNUSED,
                                                      Evas_VG_Shape_Data *pd)
{
   return pd->stroke.centered;
}

static void
_evas_vg_shape_efl_graphics_shape_stroke_dash_set(Eo *obj EINA_UNUSED,
                                                  Evas_VG_Shape_Data *pd,
                                                  const Efl_Graphics_Dash *dash,
                                                  unsigned int length)
{
   free(pd->stroke.dash);
   pd->stroke.dash = NULL;
   pd->stroke.dash_count = 0;

   pd->stroke.dash = malloc(sizeof (Efl_Graphics_Dash) * length);
   if (!pd->stroke.dash) return ;

   memcpy(pd->stroke.dash, dash, sizeof (Efl_Graphics_Dash) * length);
   pd->stroke.dash_count = length;
}

static void
_evas_vg_shape_efl_graphics_shape_stroke_dash_get(Eo *obj EINA_UNUSED,
                                                  Evas_VG_Shape_Data *pd,
                                                  const Efl_Graphics_Dash **dash,
                                                  unsigned int *length)
{
   if (dash) *dash = pd->stroke.dash;
   if (length) *length = pd->stroke.dash_count;
}

static void
_evas_vg_shape_stroke_marker_set(Eo *obj EINA_UNUSED,
                                 Evas_VG_Shape_Data *pd,
                                 Evas_VG_Shape *m)
{
   Evas_VG_Node *tmp = pd->stroke.marker;

   pd->stroke.marker = eo_ref(m);
   eo_unref(tmp);
}

static Evas_VG_Shape *
_evas_vg_shape_stroke_marker_get(Eo *obj EINA_UNUSED,
                                 Evas_VG_Shape_Data *pd)
{
   return pd->stroke.marker;
}

static void
_evas_vg_shape_efl_graphics_shape_stroke_cap_set(Eo *obj EINA_UNUSED,
                                                 Evas_VG_Shape_Data *pd,
                                                 Efl_Graphics_Cap c)
{
   pd->stroke.cap = c;
}

static Efl_Graphics_Cap
_evas_vg_shape_efl_graphics_shape_stroke_cap_get(Eo *obj EINA_UNUSED,
                                                 Evas_VG_Shape_Data *pd)
{
   return pd->stroke.cap;
}

static void
_evas_vg_shape_efl_graphics_shape_stroke_join_set(Eo *obj EINA_UNUSED,
                                                  Evas_VG_Shape_Data *pd,
                                                  Efl_Graphics_Join j)
{
   pd->stroke.join = j;
}

static Efl_Graphics_Join
_evas_vg_shape_efl_graphics_shape_stroke_join_get(Eo *obj EINA_UNUSED,
                                                  Evas_VG_Shape_Data *pd)
{
   return pd->stroke.join;
}

static void
_evas_vg_shape_render_pre(Eo *obj EINA_UNUSED,
                          Eina_Matrix3 *parent,
                          Ector_Surface *s,
                          void *data,
                          Evas_VG_Node_Data *nd)
{
   Evas_VG_Shape_Data *pd = data;
   Evas_VG_Node_Data *fill, *stroke_fill, *stroke_marker, *mask;
   EVAS_VG_COMPUTE_MATRIX(current, parent, nd);

   fill = _evas_vg_render_pre(pd->fill, s, current);
   stroke_fill = _evas_vg_render_pre(pd->stroke.fill, s, current);
   stroke_marker = _evas_vg_render_pre(pd->stroke.marker, s, current);
   mask = _evas_vg_render_pre(nd->mask, s, current);

   if (!nd->renderer)
     {
        eo_do(s, nd->renderer = ector_surface_renderer_factory_new(ECTOR_RENDERER_GENERIC_SHAPE_CLASS));
     }

   eo_do(nd->renderer,
         ector_renderer_transformation_set(current),
         ector_renderer_origin_set(nd->x, nd->y),
         ector_renderer_color_set(nd->r, nd->g, nd->b, nd->a),
         ector_renderer_visibility_set(nd->visibility),
         ector_renderer_mask_set(mask->renderer),
         ector_renderer_shape_fill_set(fill->renderer),
         ector_renderer_shape_stroke_fill_set(stroke_fill->renderer),
         ector_renderer_shape_stroke_marker_set(stroke_marker->renderer),
         efl_graphics_shape_stroke_scale_set(pd->stroke.scale),
         efl_graphics_shape_stroke_color_set(pd->stroke.r,
                                             pd->stroke.g,
                                             pd->stroke.b,
                                             pd->stroke.a),
         efl_graphics_shape_stroke_width_set(pd->stroke.width),
         efl_graphics_shape_stroke_location_set(pd->stroke.centered),
         efl_graphics_shape_stroke_dash_set(pd->stroke.dash, pd->stroke.dash_count),
         efl_graphics_shape_stroke_cap_set(pd->stroke.cap),
         efl_graphics_shape_stroke_join_set(pd->stroke.join),
         efl_graphics_shape_path_set(pd->ops, pd->points),
         ector_renderer_prepare());
}

static void
_evas_vg_shape_eo_base_constructor(Eo *obj, Evas_VG_Shape_Data *pd)
{
   Evas_VG_Node_Data *nd;

   eo_do_super(obj, MY_CLASS, eo_constructor());

   pd->stroke.cap = EFL_GRAPHICS_CAP_BUTT;
   pd->stroke.join = EFL_GRAPHICS_JOIN_MITER;
   pd->stroke.scale = 1;
   pd->stroke.a = 1;
   pd->stroke.centered = 0.5;

   nd = eo_data_scope_get(obj, EVAS_VG_NODE_CLASS);
   nd->render_pre = _evas_vg_shape_render_pre;
   nd->data = pd;
}

static void
_evas_vg_shape_eo_base_destructor(Eo *obj, Evas_VG_Shape_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

#include "evas_vg_shape.eo.c"

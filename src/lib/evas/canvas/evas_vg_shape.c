#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EVAS_VG_SHAPE_CLASS

typedef struct _Evas_VG_Shape_Data Evas_VG_Shape_Data;
struct _Evas_VG_Shape_Data
{
   Evas_VG_Node *fill;

   struct {
      Efl_Gfx_Dash *dash;
      Evas_VG_Node *fill;
      Evas_VG_Node *marker;

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

   _evas_vg_node_changed(obj);
}

static Evas_VG_Node *
_evas_vg_shape_fill_get(Eo *obj EINA_UNUSED, Evas_VG_Shape_Data *pd)
{
   return pd->fill;
}

static void
_evas_vg_shape_efl_gfx_shape_stroke_scale_set(Eo *obj EINA_UNUSED,
                                              Evas_VG_Shape_Data *pd,
                                              double s)
{
   pd->stroke.scale = s;

   _evas_vg_node_changed(obj);
}

static double
_evas_vg_shape_efl_gfx_shape_stroke_scale_get(Eo *obj EINA_UNUSED,
                                              Evas_VG_Shape_Data *pd)
{
   return pd->stroke.scale;
}

static void
_evas_vg_shape_efl_gfx_shape_stroke_color_set(Eo *obj EINA_UNUSED,
                                              Evas_VG_Shape_Data *pd,
                                              int r, int g, int b, int a)
{
   pd->stroke.r = r;
   pd->stroke.g = g;
   pd->stroke.b = b;
   pd->stroke.a = a;

   _evas_vg_node_changed(obj);
}

static Eina_Bool
_evas_vg_shape_efl_gfx_base_color_part_set(Eo *obj, Evas_VG_Shape_Data *pd,
                                           const char * part,
                                           int r, int g, int b, int a)
{
   Eina_Bool ret;

   if (part && !strcmp(part, "stroke"))
     {
        _evas_vg_shape_efl_gfx_shape_stroke_color_set(obj, pd, r, g, b, a);
        return EINA_TRUE;
     }

   eo_do_super(obj, EFL_VG_SHAPE_CLASS,
               ret = efl_gfx_color_part_set(part, r, g, b, a));

   return ret;
}

static void
_evas_vg_shape_efl_gfx_shape_stroke_color_get(Eo *obj EINA_UNUSED,
                                              Evas_VG_Shape_Data *pd,
                                              int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->stroke.r;
   if (g) *g = pd->stroke.g;
   if (b) *b = pd->stroke.b;
   if (a) *a = pd->stroke.a;
}

static Eina_Bool
_evas_vg_shape_efl_gfx_base_color_part_get(Eo *obj, Evas_VG_Shape_Data *pd,
                                           const char * part,
                                           int *r, int *g, int *b, int *a)
{
   Eina_Bool ret;

   if (part && !strcmp(part, "stroke"))
     {
        _evas_vg_shape_efl_gfx_shape_stroke_color_get(obj, pd, r, g, b, a);
        return EINA_TRUE;
     }

   eo_do_super(obj, EFL_VG_SHAPE_CLASS,
               ret = efl_gfx_color_part_get(part, r, g, b, a));

   return ret;
}

static void
_evas_vg_shape_stroke_fill_set(Eo *obj EINA_UNUSED,
                               Evas_VG_Shape_Data *pd,
                               Evas_VG_Node *f)
{
   Evas_VG_Node *tmp = pd->fill;

   pd->stroke.fill = eo_ref(f);
   eo_unref(tmp);

   _evas_vg_node_changed(obj);
}

static Evas_VG_Node *
_evas_vg_shape_stroke_fill_get(Eo *obj EINA_UNUSED,
                               Evas_VG_Shape_Data *pd)
{
   return pd->stroke.fill;
}

static void
_evas_vg_shape_efl_gfx_shape_stroke_width_set(Eo *obj EINA_UNUSED,
                                              Evas_VG_Shape_Data *pd,
                                              double w)
{
   pd->stroke.width = w;

   _evas_vg_node_changed(obj);
}

static double
_evas_vg_shape_efl_gfx_shape_stroke_width_get(Eo *obj EINA_UNUSED,
                                              Evas_VG_Shape_Data *pd)
{
   return pd->stroke.width;
}

static void
_evas_vg_shape_efl_gfx_shape_stroke_location_set(Eo *obj EINA_UNUSED,
                                                 Evas_VG_Shape_Data *pd,
                                                 double centered)
{
   pd->stroke.centered = centered;

   _evas_vg_node_changed(obj);
}

static double
_evas_vg_shape_efl_gfx_shape_stroke_location_get(Eo *obj EINA_UNUSED,
                                                 Evas_VG_Shape_Data *pd)
{
   return pd->stroke.centered;
}

static void
_evas_vg_shape_efl_gfx_shape_stroke_dash_set(Eo *obj EINA_UNUSED,
                                             Evas_VG_Shape_Data *pd,
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

   _evas_vg_node_changed(obj);
}

static void
_evas_vg_shape_efl_gfx_shape_stroke_dash_get(Eo *obj EINA_UNUSED,
                                             Evas_VG_Shape_Data *pd,
                                             const Efl_Gfx_Dash **dash,
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

   _evas_vg_node_changed(obj);
}

static Evas_VG_Shape *
_evas_vg_shape_stroke_marker_get(Eo *obj EINA_UNUSED,
                                 Evas_VG_Shape_Data *pd)
{
   return pd->stroke.marker;
}

static void
_evas_vg_shape_efl_gfx_shape_stroke_cap_set(Eo *obj EINA_UNUSED,
                                            Evas_VG_Shape_Data *pd,
                                            Efl_Gfx_Cap c)
{
   pd->stroke.cap = c;

   _evas_vg_node_changed(obj);
}

static Efl_Gfx_Cap
_evas_vg_shape_efl_gfx_shape_stroke_cap_get(Eo *obj EINA_UNUSED,
                                            Evas_VG_Shape_Data *pd)
{
   return pd->stroke.cap;
}

static void
_evas_vg_shape_efl_gfx_shape_stroke_join_set(Eo *obj EINA_UNUSED,
                                             Evas_VG_Shape_Data *pd,
                                             Efl_Gfx_Join j)
{
   pd->stroke.join = j;

   _evas_vg_node_changed(obj);
}

static Efl_Gfx_Join
_evas_vg_shape_efl_gfx_shape_stroke_join_get(Eo *obj EINA_UNUSED,
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

   if (!nd->changed) return ;
   nd->changed = EINA_FALSE;

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
         ector_renderer_mask_set(mask ? mask->renderer : NULL),
         ector_renderer_shape_fill_set(fill ? fill->renderer : NULL),
         ector_renderer_shape_stroke_fill_set(stroke_fill ? stroke_fill->renderer : NULL),
         ector_renderer_shape_stroke_marker_set(stroke_marker ? stroke_marker->renderer : NULL),
         efl_gfx_shape_dup(obj),
         ector_renderer_prepare());
}

static void
_evas_vg_shape_eo_base_constructor(Eo *obj, Evas_VG_Shape_Data *pd)
{
   Evas_VG_Node_Data *nd;

   eo_do_super(obj, MY_CLASS, eo_constructor());

   pd->stroke.cap = EFL_GFX_CAP_BUTT;
   pd->stroke.join = EFL_GFX_JOIN_MITER;
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

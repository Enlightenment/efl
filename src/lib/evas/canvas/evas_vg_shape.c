#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EVAS_VG_SHAPE_CLASS

typedef struct _Evas_VG_Shape_Data Evas_VG_Shape_Data;
struct _Evas_VG_Shape_Data
{
   Efl_Geometry_Path_Command *op;
   double *points;

   Evas_VG_Node *fill;

   struct {
      Efl_Geometry_Dash *dash;
      Evas_VG_Node *fill;
      Evas_VG_Node *marker;

      double scale;
      double width;
      double centered; // from 0 to 1

      int r, g, b, a;

      unsigned int dash_count;

      Efl_Geometry_Cap cap;
      Efl_Geometry_Join join;
   } stroke;

   unsigned int op_count;
   unsigned int pts_counts;
};

Eina_Bool
_evas_vg_shape_efl_geometry_shape_path_set(Eo *obj EINA_UNUSED,
                                           Evas_VG_Shape_Data *pd,
                                           const Efl_Geometry_Path_Command *op,
                                           const double *points)
{
   free(pd->points);
   pd->points = NULL;
   free(pd->op);
   pd->op = NULL;

   return efl_geometry_path_dup(&pd->op, &pd->points, op, points);
}

Eina_Bool
_evas_vg_shape_bounds_get(Eo *obj, Evas_VG_Shape_Data *pd, Eina_Rectangle *r)
{
   (void) obj; (void) pd; (void) r;
   return EINA_FALSE;
}

void
_evas_vg_shape_eo_base_constructor(Eo *obj, Evas_VG_Shape_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   pd->stroke.cap = EFL_GEOMETRY_CAP_BUTT;
   pd->stroke.join = EFL_GEOMETRY_JOIN_MITER;
   pd->stroke.scale = 1;
   pd->stroke.a = 1;
   pd->stroke.centered = 0.5;
}

void
_evas_vg_shape_eo_base_destructor(Eo *obj, Evas_VG_Shape_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

void
_evas_vg_shape_fill_set(Eo *obj EINA_UNUSED,
                        Evas_VG_Shape_Data *pd,
                        Evas_VG_Node *f)
{
   Evas_VG_Node *tmp = pd->fill;

   pd->fill = eo_ref(f);
   eo_unref(tmp);
}

Evas_VG_Node *
_evas_vg_shape_fill_get(Eo *obj EINA_UNUSED, Evas_VG_Shape_Data *pd)
{
   return pd->fill;
}

void
_evas_vg_shape_efl_geometry_shape_stroke_scale_set(Eo *obj EINA_UNUSED,
                                                   Evas_VG_Shape_Data *pd,
                                                   double s)
{
   pd->stroke.scale = s;
}

double
_evas_vg_shape_efl_geometry_shape_stroke_scale_get(Eo *obj EINA_UNUSED,
                                                   Evas_VG_Shape_Data *pd)
{
   return pd->stroke.scale;
}

void
_evas_vg_shape_efl_geometry_shape_stroke_color_set(Eo *obj EINA_UNUSED,
                                                   Evas_VG_Shape_Data *pd,
                                                   int r, int g, int b, int a)
{
   pd->stroke.r = r;
   pd->stroke.g = g;
   pd->stroke.b = b;
   pd->stroke.a = a;
}

void
_evas_vg_shape_efl_geometry_shape_stroke_color_get(Eo *obj EINA_UNUSED,
                                                   Evas_VG_Shape_Data *pd,
                                                   int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->stroke.r;
   if (g) *g = pd->stroke.g;
   if (b) *b = pd->stroke.b;
   if (a) *a = pd->stroke.a;
}

void
_evas_vg_shape_stroke_fill_set(Eo *obj EINA_UNUSED,
                               Evas_VG_Shape_Data *pd,
                               Evas_VG_Node *f)
{
   Evas_VG_Node *tmp = pd->fill;

   pd->stroke.fill = eo_ref(f);
   eo_unref(tmp);
}

Evas_VG_Node *
_evas_vg_shape_stroke_fill_get(Eo *obj EINA_UNUSED,
                               Evas_VG_Shape_Data *pd)
{
   return pd->stroke.fill;
}

void
_evas_vg_shape_efl_geometry_shape_stroke_width_set(Eo *obj EINA_UNUSED,
                                                   Evas_VG_Shape_Data *pd,
                                                   double w)
{
   pd->stroke.width = w;
}

double
_evas_vg_shape_efl_geometry_shape_stroke_width_get(Eo *obj EINA_UNUSED,
                                                   Evas_VG_Shape_Data *pd)
{
   return pd->stroke.width;
}

void
_evas_vg_shape_efl_geometry_shape_stroke_location_set(Eo *obj EINA_UNUSED,
                                                      Evas_VG_Shape_Data *pd,
                                                      double centered)
{
   pd->stroke.centered = centered;
}

double
_evas_vg_shape_efl_geometry_shape_stroke_location_get(Eo *obj EINA_UNUSED,
                                                      Evas_VG_Shape_Data *pd)
{
   return pd->stroke.centered;
}

void
_evas_vg_shape_efl_geometry_shape_stroke_dash_set(Eo *obj EINA_UNUSED,
                                                  Evas_VG_Shape_Data *pd,
                                                  const Efl_Geometry_Dash *dash,
                                                  unsigned int length)
{
   free(pd->stroke.dash);
   pd->stroke.dash = NULL;
   pd->stroke.dash_count = 0;

   pd->stroke.dash = malloc(sizeof (Efl_Geometry_Dash) * length);
   if (!pd->stroke.dash) return ;

   memcpy(pd->stroke.dash, dash, sizeof (Efl_Geometry_Dash) * length);
   pd->stroke.dash_count = length;
}

void
_evas_vg_shape_efl_geometry_shape_stroke_dash_get(Eo *obj EINA_UNUSED,
                                                  Evas_VG_Shape_Data *pd,
                                                  const Efl_Geometry_Dash **dash,
                                                  unsigned int *length)
{
   if (dash) *dash = pd->stroke.dash;
   if (length) *length = pd->stroke.dash_count;
}

void
_evas_vg_shape_stroke_marker_set(Eo *obj EINA_UNUSED,
                                 Evas_VG_Shape_Data *pd,
                                 Evas_VG_Shape *m)
{
   Evas_VG_Node *tmp = pd->stroke.marker;

   pd->stroke.marker = eo_ref(m);
   eo_unref(tmp);
}

Evas_VG_Shape *
_evas_vg_shape_stroke_marker_get(Eo *obj EINA_UNUSED,
                                 Evas_VG_Shape_Data *pd)
{
   return pd->stroke.marker;
}

void
_evas_vg_shape_efl_geometry_shape_stroke_cap_set(Eo *obj EINA_UNUSED,
                                                 Evas_VG_Shape_Data *pd,
                                                 Efl_Geometry_Cap c)
{
   pd->stroke.cap = c;
}

Efl_Geometry_Cap
_evas_vg_shape_efl_geometry_shape_stroke_cap_get(Eo *obj EINA_UNUSED,
                                                 Evas_VG_Shape_Data *pd)
{
   return pd->stroke.cap;
}

void
_evas_vg_shape_efl_geometry_shape_stroke_join_set(Eo *obj EINA_UNUSED,
                                                  Evas_VG_Shape_Data *pd,
                                                  Efl_Geometry_Join j)
{
   pd->stroke.join = j;
}

Efl_Geometry_Join
_evas_vg_shape_efl_geometry_shape_stroke_join_get(Eo *obj EINA_UNUSED,
                                                  Evas_VG_Shape_Data *pd)
{
   return pd->stroke.join;
}

#include "evas_vg_shape.eo.c"

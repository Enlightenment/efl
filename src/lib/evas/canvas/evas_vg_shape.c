#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EVAS_VG_SHAPE_CLASS

typedef struct _Evas_VG_Shape_Data Evas_VG_Shape_Data;
struct _Evas_VG_Shape_Data
{
   Evas_VG_Path_Command *op;
   double *points;
   unsigned int op_count;
   unsigned int pts_counts;
};

Eina_Bool
_evas_vg_shape_path_set(Eo *obj EINA_UNUSED, Evas_VG_Shape_Data *pd,
                        const Evas_VG_Path_Command *op, const double *points)
{
   free(pd->points);
   pd->points = NULL;
   free(pd->op);
   pd->op = NULL;

   return evas_vg_path_dup(&pd->op, &pd->points, op, points);
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
}

void
_evas_vg_shape_eo_base_destructor(Eo *obj, Evas_VG_Shape_Data *pd)
{
   (void) obj; (void) pd;
}

void
_evas_vg_shape_fill_set(Eo *obj, Evas_VG_Shape_Data *pd, Evas_VG_Node *f)
{
   (void) obj; (void) pd; (void) f;
}

Evas_VG_Node *
_evas_vg_shape_fill_get(Eo *obj, Evas_VG_Shape_Data *pd)
{
   (void) obj; (void) pd;
   return NULL;
}

void
_evas_vg_shape_stroke_scale_set(Eo *obj, Evas_VG_Shape_Data *pd, double s)
{
   (void) obj; (void) pd; (void) s;
}

double
_evas_vg_shape_stroke_scale_get(Eo *obj, Evas_VG_Shape_Data *pd)
{
   (void) obj; (void) pd;
   return 0.0;
}

void
_evas_vg_shape_stroke_color_set(Eo *obj, Evas_VG_Shape_Data *pd,
                                int r, int g, int b, int a)
{
   (void) obj; (void) pd; (void) r; (void) g; (void) b; (void) a;
}

void
_evas_vg_shape_stroke_color_get(Eo *obj, Evas_VG_Shape_Data *pd,
                                int *r, int *g, int *b, int *a)
{
   (void) obj; (void) pd; (void) r; (void) g; (void) b; (void) a;
}

void
_evas_vg_shape_stroke_fill_set(Eo *obj, Evas_VG_Shape_Data *pd, Evas_VG_Node *f)
{
   (void) obj; (void) pd; (void) f;
}

Evas_VG_Node *
_evas_vg_shape_stroke_fill_get(Eo *obj, Evas_VG_Shape_Data *pd)
{
   (void) obj; (void) pd;
   return NULL;
}

void
_evas_vg_shape_stroke_width_set(Eo *obj, Evas_VG_Shape_Data *pd, double w)
{
   (void) obj; (void) pd; (void) w;
}

double
_evas_vg_shape_stroke_width_get(Eo *obj, Evas_VG_Shape_Data *pd)
{
   (void) obj; (void) pd;
   return 0.0;
}

void
_evas_vg_shape_stroke_location_set(Eo *obj, Evas_VG_Shape_Data *pd,
                                   double centered)
{
   (void) obj; (void) pd; (void) centered;
}

double
_evas_vg_shape_stroke_location_get(Eo *obj, Evas_VG_Shape_Data *pd)
{
   (void) obj; (void) pd;
   return 0.0;
}

void
_evas_vg_shape_stroke_dash_set(Eo *obj, Evas_VG_Shape_Data *pd,
                               const Evas_VG_Dash *dash, unsigned int length)
{
   (void) obj; (void) pd; (void) dash; (void) length;
}

void
_evas_vg_shape_stroke_dash_get(Eo *obj, Evas_VG_Shape_Data *pd,
                               const Evas_VG_Dash **dash, unsigned int *length)
{
   (void) obj; (void) pd; (void) dash; (void) length;
}

void
_evas_vg_shape_stroke_marker_set(Eo *obj, Evas_VG_Shape_Data *pd,
                                 Evas_VG_Shape *m)
{
   (void) obj; (void) pd; (void) m;
}

Evas_VG_Shape *
_evas_vg_shape_stroke_marker_get(Eo *obj, Evas_VG_Shape_Data *pd)
{
   (void) obj; (void) pd;
   return NULL;
}

void
_evas_vg_shape_stroke_cap_set(Eo *obj, Evas_VG_Shape_Data *pd, Evas_VG_Cap c)
{
   (void) obj; (void) pd; (void) c;
}

Evas_VG_Cap
_evas_vg_shape_stroke_cap_get(Eo *obj, Evas_VG_Shape_Data *pd)
{
   (void) obj; (void) pd;
   return EVAS_VG_CAP_LAST;
}

void
_evas_vg_shape_stroke_join_set(Eo *obj, Evas_VG_Shape_Data *pd, Evas_VG_Join j)
{
   (void) obj; (void) pd; (void) j;
}

Evas_VG_Join
_evas_vg_shape_stroke_join_get(Eo *obj, Evas_VG_Shape_Data *pd)
{
   (void) obj; (void) pd;
   return EVAS_VG_JOIN_LAST;
}

#include "evas_vg_shape.eo.c"

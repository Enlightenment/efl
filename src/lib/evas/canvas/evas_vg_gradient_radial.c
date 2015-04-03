#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EVAS_VG_GRADIENT_RADIAL_CLASS

typedef struct _Evas_VG_Gradient_Radial_Data Evas_VG_Gradient_Radial_Data;
struct _Evas_VG_Gradient_Radial_Data
{
   struct {
      double x, y;
   } center, focal;
   double radius;
};

static void
_evas_vg_gradient_radial_efl_gfx_gradient_radial_center_set(Eo *obj EINA_UNUSED,
                                                            Evas_VG_Gradient_Radial_Data *pd,
                                                            double x, double y)
{
   pd->center.x = x;
   pd->center.y = y;
}

static void
_evas_vg_gradient_radial_efl_gfx_gradient_radial_center_get(Eo *obj EINA_UNUSED,
                                                            Evas_VG_Gradient_Radial_Data *pd,
                                                            double *x, double *y)
{
   if (x) *x = pd->center.x;
   if (y) *y = pd->center.y;
}

static void
_evas_vg_gradient_radial_efl_gfx_gradient_radial_radius_set(Eo *obj EINA_UNUSED,
                                                            Evas_VG_Gradient_Radial_Data *pd,
                                                            double r)
{
   pd->radius = r;
}

static double
_evas_vg_gradient_radial_efl_gfx_gradient_radial_radius_get(Eo *obj EINA_UNUSED,
                                                            Evas_VG_Gradient_Radial_Data *pd)
{
   return pd->radius;
}

static void
_evas_vg_gradient_radial_efl_gfx_gradient_radial_focal_set(Eo *obj EINA_UNUSED,
                                                           Evas_VG_Gradient_Radial_Data *pd,
                                                           double x, double y)
{
   pd->focal.x = x;
   pd->focal.y = y;
}

static void
_evas_vg_gradient_radial_efl_gfx_gradient_radial_focal_get(Eo *obj EINA_UNUSED,
                                                           Evas_VG_Gradient_Radial_Data *pd,
                                                           double *x, double *y)
{
   if (x) *x = pd->focal.x;
   if (y) *y = pd->focal.y;
}

static void
_evas_vg_gradient_radial_render_pre(Eo *obj,
                                    Eina_Matrix3 *parent,
                                    Ector_Surface *s,
                                    void *data,
                                    Evas_VG_Node_Data *nd)
{
   Evas_VG_Gradient_Radial_Data *pd = data;
   Evas_VG_Gradient_Data *gd = eo_data_scope_get(obj, EVAS_VG_GRADIENT_CLASS);
   EVAS_VG_COMPUTE_MATRIX(current, parent, nd);

   if (!nd->renderer)
     {
        eo_do(s, nd->renderer = ector_surface_renderer_factory_new(ECTOR_RENDERER_GENERIC_GRADIENT_RADIAL_CLASS));
     }

   eo_do(nd->renderer,
         ector_renderer_transformation_set(current),
         ector_renderer_origin_set(nd->x, nd->y),
         ector_renderer_color_set(nd->r, nd->g, nd->b, nd->a),
         ector_renderer_visibility_set(nd->visibility),
         efl_gfx_gradient_stop_set(gd->colors, gd->colors_count),
         efl_gfx_gradient_spread_set(gd->s),
         efl_gfx_gradient_radial_center_set(pd->center.x, pd->center.y),
         efl_gfx_gradient_radial_focal_set(pd->focal.x, pd->focal.y),
         efl_gfx_gradient_radial_radius_set(pd->radius),
         ector_renderer_prepare());
}

static void
_evas_vg_gradient_radial_eo_base_constructor(Eo *obj, Evas_VG_Gradient_Radial_Data *pd)
{
   Evas_VG_Node_Data *nd;

   eo_do_super(obj, MY_CLASS, eo_constructor());

   nd = eo_data_scope_get(obj, EVAS_VG_NODE_CLASS);
   nd->render_pre = _evas_vg_gradient_radial_render_pre;
   nd->data = pd;
}

static void
_evas_vg_gradient_radial_eo_base_destructor(Eo *obj,
                                            Evas_VG_Gradient_Radial_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

#include "evas_vg_gradient_radial.eo.c"

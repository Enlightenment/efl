#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#define MY_CLASS EFL_VG_GRADIENT_RADIAL_CLASS

typedef struct _Efl_VG_Gradient_Radial_Data Efl_VG_Gradient_Radial_Data;
struct _Efl_VG_Gradient_Radial_Data
{
   struct {
      double x, y;
   } center, focal;
   double radius;
};

static void
_efl_vg_gradient_radial_efl_gfx_gradient_radial_center_set(Eo *obj EINA_UNUSED,
                                                           Efl_VG_Gradient_Radial_Data *pd,
                                                           double x, double y)
{
   pd->center.x = x;
   pd->center.y = y;

   _efl_vg_base_changed(obj);
}

static void
_efl_vg_gradient_radial_efl_gfx_gradient_radial_center_get(Eo *obj EINA_UNUSED,
                                                           Efl_VG_Gradient_Radial_Data *pd,
                                                           double *x, double *y)
{
   if (x) *x = pd->center.x;
   if (y) *y = pd->center.y;
}

static void
_efl_vg_gradient_radial_efl_gfx_gradient_radial_radius_set(Eo *obj EINA_UNUSED,
                                                           Efl_VG_Gradient_Radial_Data *pd,
                                                           double r)
{
   pd->radius = r;

   _efl_vg_base_changed(obj);
}

static double
_efl_vg_gradient_radial_efl_gfx_gradient_radial_radius_get(Eo *obj EINA_UNUSED,
                                                           Efl_VG_Gradient_Radial_Data *pd)
{
   return pd->radius;
}

static void
_efl_vg_gradient_radial_efl_gfx_gradient_radial_focal_set(Eo *obj EINA_UNUSED,
                                                          Efl_VG_Gradient_Radial_Data *pd,
                                                          double x, double y)
{
   pd->focal.x = x;
   pd->focal.y = y;

   _efl_vg_base_changed(obj);
}

static void
_efl_vg_gradient_radial_efl_gfx_gradient_radial_focal_get(Eo *obj EINA_UNUSED,
                                                          Efl_VG_Gradient_Radial_Data *pd,
                                                          double *x, double *y)
{
   if (x) *x = pd->focal.x;
   if (y) *y = pd->focal.y;
}

static void
_efl_vg_gradient_radial_render_pre(Eo *obj,
                                    Eina_Matrix3 *parent,
                                    Ector_Surface *s,
                                    void *data,
                                    Efl_VG_Base_Data *nd)
{
   Efl_VG_Gradient_Radial_Data *pd = data;
   Efl_VG_Gradient_Data *gd;

   if (!nd->changed) return ;
   nd->changed = EINA_FALSE;

   gd = eo_data_scope_get(obj, EFL_VG_GRADIENT_CLASS);
   EFL_VG_COMPUTE_MATRIX(current, parent, nd);

   if (!nd->renderer)
     {
        eo_do(s, nd->renderer = ector_surface_renderer_factory_new(ECTOR_RENDERER_GENERIC_GRADIENT_RADIAL_MIXIN));
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

static Eo *
_efl_vg_gradient_radial_eo_base_constructor(Eo *obj, Efl_VG_Gradient_Radial_Data *pd)
{
   Efl_VG_Base_Data *nd;

   obj = eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());

   nd = eo_data_scope_get(obj, EFL_VG_BASE_CLASS);
   nd->render_pre = _efl_vg_gradient_radial_render_pre;
   nd->data = pd;

   return obj;
}

static void
_efl_vg_gradient_radial_eo_base_destructor(Eo *obj,
                                           Efl_VG_Gradient_Radial_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_efl_vg_gradient_radial_efl_vg_base_bounds_get(Eo *obj, Efl_VG_Gradient_Radial_Data *pd, Eina_Rectangle *r)
{
   Efl_VG_Base_Data *nd;

   nd = eo_data_scope_get(obj, EFL_VG_BASE_CLASS);
   EINA_RECTANGLE_SET(r,
                      nd->x + pd->center.x - pd->radius,
                      nd->y + pd->center.y - pd->radius,
                      pd->radius * 2, pd->radius * 2);
}

static Eina_Bool
_efl_vg_gradient_radial_efl_vg_base_interpolate(Eo *obj,
                                                Efl_VG_Gradient_Radial_Data *pd,
                                                const Efl_VG_Base *from, const Efl_VG_Base *to,
                                                double pos_map)
{
   Efl_VG_Gradient_Radial_Data *fromd, *tod;
   double from_map;
   Eina_Bool r;

   eo_do_super(obj, EFL_VG_GRADIENT_RADIAL_CLASS, r = efl_vg_interpolate(from, to, pos_map));

   if (!r) return EINA_FALSE;

   fromd = eo_data_scope_get(from, EFL_VG_GRADIENT_RADIAL_CLASS);
   tod = eo_data_scope_get(to, EFL_VG_GRADIENT_RADIAL_CLASS);
   from_map = 1.0 - pos_map;

#define INTP(Pd, From, To, Member, From_Map, Pos_Map)   \
   Pd->Member = From->Member * From_Map + To->Member * Pos_Map

   INTP(pd, fromd, tod, focal.x, from_map, pos_map);
   INTP(pd, fromd, tod, focal.y, from_map, pos_map);
   INTP(pd, fromd, tod, center.x, from_map, pos_map);
   INTP(pd, fromd, tod, center.y, from_map, pos_map);
   INTP(pd, fromd, tod, radius, from_map, pos_map);

#undef INTP

   return EINA_TRUE;
}

EAPI void
evas_vg_gradient_radial_center_set(Eo *obj, double x, double y)
{
   eo_do(obj, efl_gfx_gradient_radial_center_set(x, y));
}

EAPI void
evas_vg_gradient_radial_center_get(Eo *obj, double *x, double *y)
{
   eo_do(obj, efl_gfx_gradient_radial_center_get(x, y));
}

EAPI void
evas_vg_gradient_radial_radius_set(Eo *obj, double r)
{
   eo_do(obj, efl_gfx_gradient_radial_radius_set(r));
}

EAPI double
evas_vg_gradient_radial_radius_get(Eo *obj)
{
   double ret;

   return eo_do_ret(obj, ret, efl_gfx_gradient_radial_radius_get());
}

EAPI void
evas_vg_gradient_radial_focal_set(Eo *obj, double x, double y)
{
   eo_do(obj, efl_gfx_gradient_radial_focal_set(x, y));
}

EAPI void
evas_vg_gradient_radial_focal_get(Eo *obj, double *x, double *y)
{
   eo_do(obj, efl_gfx_gradient_radial_focal_get(x, y));
}

#include "efl_vg_gradient_radial.eo.c"

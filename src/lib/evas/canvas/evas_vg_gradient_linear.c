#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#include <strings.h>

#define MY_CLASS EFL_VG_GRADIENT_LINEAR_CLASS

typedef struct _Efl_VG_Gradient_Linear_Data Efl_VG_Gradient_Linear_Data;
struct _Efl_VG_Gradient_Linear_Data
{
   struct {
      double x, y;
   } start, end;
};

static void
_efl_vg_gradient_linear_efl_gfx_gradient_linear_start_set(Eo *obj EINA_UNUSED,
                                                          Efl_VG_Gradient_Linear_Data *pd,
                                                          double x, double y)
{
   pd->start.x = x;
   pd->start.y = y;

   _efl_vg_base_changed(obj);
}

static void
_efl_vg_gradient_linear_efl_gfx_gradient_linear_start_get(Eo *obj EINA_UNUSED,
                                                          Efl_VG_Gradient_Linear_Data *pd,
                                                          double *x, double *y)
{
   if (x) *x = pd->start.x;
   if (y) *y = pd->start.y;
}

static void
_efl_vg_gradient_linear_efl_gfx_gradient_linear_end_set(Eo *obj EINA_UNUSED,
                                                        Efl_VG_Gradient_Linear_Data *pd,
                                                        double x, double y)
{
   pd->end.x = x;
   pd->end.y = y;

   _efl_vg_base_changed(obj);
}

static void
_efl_vg_gradient_linear_efl_gfx_gradient_linear_end_get(Eo *obj EINA_UNUSED,
                                                        Efl_VG_Gradient_Linear_Data *pd,
                                                        double *x, double *y)
{
   if (x) *x = pd->end.x;
   if (y) *y = pd->end.y;
}

static void
_efl_vg_gradient_linear_render_pre(Eo *obj,
                                   Eina_Matrix3 *parent,
                                   Ector_Surface *s,
                                   void *data,
                                   Efl_VG_Base_Data *nd)
{
   Efl_VG_Gradient_Linear_Data *pd = data;
   Efl_VG_Gradient_Data *gd;

   if (!nd->changed) return ;
   nd->changed = EINA_FALSE;

   gd = eo_data_scope_get(obj, EFL_VG_GRADIENT_CLASS);
   EFL_VG_COMPUTE_MATRIX(current, parent, nd);

   if (!nd->renderer)
     {
        eo_do(s, nd->renderer = ector_surface_renderer_factory_new(ECTOR_RENDERER_GENERIC_GRADIENT_LINEAR_MIXIN));
     }

   eo_do(nd->renderer,
         ector_renderer_transformation_set(current),
         ector_renderer_origin_set(nd->x, nd->y),
         ector_renderer_color_set(nd->r, nd->g, nd->b, nd->a),
         ector_renderer_visibility_set(nd->visibility),
         efl_gfx_gradient_stop_set(gd->colors, gd->colors_count),
         efl_gfx_gradient_spread_set(gd->s),
         efl_gfx_gradient_linear_start_set(pd->start.x, pd->start.y),
         efl_gfx_gradient_linear_end_set(pd->end.x, pd->end.y),
         ector_renderer_prepare());
}

static Eo *
_efl_vg_gradient_linear_eo_base_constructor(Eo *obj,
                                            Efl_VG_Gradient_Linear_Data *pd)
{
   Efl_VG_Base_Data *nd;

   obj = eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());

   nd = eo_data_scope_get(obj, EFL_VG_BASE_CLASS);
   nd->render_pre = _efl_vg_gradient_linear_render_pre;
   nd->data = pd;

   return obj;
}

static void
_efl_vg_gradient_linear_eo_base_destructor(Eo *obj, Efl_VG_Gradient_Linear_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_efl_vg_gradient_linear_efl_vg_base_bounds_get(Eo *obj, Efl_VG_Gradient_Linear_Data *pd, Eina_Rectangle *r)
{
   Efl_VG_Base_Data *nd;

   nd = eo_data_scope_get(obj, EFL_VG_BASE_CLASS);
   EINA_RECTANGLE_SET(r,
                      nd->x + pd->start.x, nd->y + pd->start.y,
                      pd->end.x - pd->start.x, pd->end.y - pd->start.x);
}

EAPI void
evas_vg_gradient_linear_start_set(Eo *obj, double x, double y)
{
   eo_do(obj, efl_gfx_gradient_linear_start_set(x, y));
}

EAPI void
evas_vg_gradient_linear_start_get(Eo *obj, double *x, double *y)
{
   eo_do(obj, efl_gfx_gradient_linear_start_get(x, y));
}

EAPI void
evas_vg_gradient_linear_end_set(Eo *obj, double x, double y)
{
   eo_do(obj, efl_gfx_gradient_linear_end_set(x, y));
}

EAPI void
evas_vg_gradient_linear_end_get(Eo *obj, double *x, double *y)
{
   eo_do(obj, efl_gfx_gradient_linear_end_get(x, y));
}

#include "efl_vg_gradient_linear.eo.c"

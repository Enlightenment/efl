#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#include <strings.h>

#define MY_CLASS EVAS_VG_GRADIENT_LINEAR_CLASS

typedef struct _Evas_VG_Gradient_Linear_Data Evas_VG_Gradient_Linear_Data;
struct _Evas_VG_Gradient_Linear_Data
{
   struct {
      double x, y;
   } start, end;
};

static void
_evas_vg_gradient_linear_efl_graphics_gradient_linear_start_set(Eo *obj EINA_UNUSED,
                                   Evas_VG_Gradient_Linear_Data *pd,
                                   double x, double y)
{
   pd->start.x = x;
   pd->start.y = y;
}

static void
_evas_vg_gradient_linear_efl_graphics_gradient_linear_start_get(Eo *obj EINA_UNUSED,
                                   Evas_VG_Gradient_Linear_Data *pd,
                                   double *x, double *y)
{
   if (x) *x = pd->start.x;
   if (y) *y = pd->start.y;
}

static void
_evas_vg_gradient_linear_efl_graphics_gradient_linear_end_set(Eo *obj EINA_UNUSED,
                                 Evas_VG_Gradient_Linear_Data *pd,
                                 double x, double y)
{
   pd->end.x = x;
   pd->end.y = y;
}

static void
_evas_vg_gradient_linear_efl_graphics_gradient_linear_end_get(Eo *obj EINA_UNUSED,
                                 Evas_VG_Gradient_Linear_Data *pd,
                                 double *x, double *y)
{
   if (x) *x = pd->end.x;
   if (y) *y = pd->end.y;
}

static void
_evas_vg_gradient_linear_render_pre(Eo *obj,
                                    Ector_Surface *s,
                                    void *data,
                                    Evas_VG_Node_Data *nd)
{
   Evas_VG_Gradient_Linear_Data *pd = data;
   Evas_VG_Gradient_Data *gd = eo_data_scope_get(obj, EVAS_VG_GRADIENT_CLASS);

   if (!nd->renderer)
     {
        eo_do(s, nd->renderer = ector_surface_renderer_factory_new(ECTOR_RENDERER_GENERIC_GRADIENT_LINEAR_CLASS));
     }

   eo_do(nd->renderer,
         ector_renderer_transformation_set(nd->m),
         ector_renderer_origin_set(nd->x, nd->y),
         ector_renderer_color_set(nd->r, nd->g, nd->b, nd->a),
         ector_renderer_visibility_set(nd->visibility),
         efl_graphics_gradient_stop_set(gd->colors, gd->colors_count),
         efl_graphics_gradient_spread_set(gd->s),
         efl_graphics_gradient_linear_start_set(pd->start.x, pd->start.y),
         efl_graphics_gradient_linear_end_set(pd->end.x, pd->end.y),
         ector_renderer_prepare());
}

static void
_evas_vg_gradient_linear_eo_base_constructor(Eo *obj,
                                             Evas_VG_Gradient_Linear_Data *pd)
{
   Evas_VG_Node_Data *nd;

   eo_do_super(obj, MY_CLASS, eo_constructor());

   nd = eo_data_scope_get(obj, EVAS_VG_NODE_CLASS);
   nd->render_pre = _evas_vg_gradient_linear_render_pre;
   nd->data = pd;
}

void
_evas_vg_gradient_linear_eo_base_destructor(Eo *obj, Evas_VG_Gradient_Linear_Data *pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

#include "evas_vg_gradient_linear.eo.c"

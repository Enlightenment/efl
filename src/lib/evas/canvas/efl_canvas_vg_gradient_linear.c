#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_vg_private.h"

#include <strings.h>

#define MY_CLASS EFL_CANVAS_VG_GRADIENT_LINEAR_CLASS

typedef struct _Efl_Canvas_Vg_Gradient_Linear_Data Efl_Canvas_Vg_Gradient_Linear_Data;
struct _Efl_Canvas_Vg_Gradient_Linear_Data
{
   struct {
      double x, y;
   } start, end;
};

static void
_efl_canvas_vg_gradient_linear_efl_gfx_gradient_linear_start_set(Eo *obj EINA_UNUSED,
                                                          Efl_Canvas_Vg_Gradient_Linear_Data *pd,
                                                          double x, double y)
{
   pd->start.x = x;
   pd->start.y = y;

   _efl_canvas_vg_node_changed(obj);
}

static void
_efl_canvas_vg_gradient_linear_efl_gfx_gradient_linear_start_get(const Eo *obj EINA_UNUSED,
                                                          Efl_Canvas_Vg_Gradient_Linear_Data *pd,
                                                          double *x, double *y)
{
   if (x) *x = pd->start.x;
   if (y) *y = pd->start.y;
}

static void
_efl_canvas_vg_gradient_linear_efl_gfx_gradient_linear_end_set(Eo *obj EINA_UNUSED,
                                                        Efl_Canvas_Vg_Gradient_Linear_Data *pd,
                                                        double x, double y)
{
   pd->end.x = x;
   pd->end.y = y;

   _efl_canvas_vg_node_changed(obj);
}

static void
_efl_canvas_vg_gradient_linear_efl_gfx_gradient_linear_end_get(const Eo *obj EINA_UNUSED,
                                                        Efl_Canvas_Vg_Gradient_Linear_Data *pd,
                                                        double *x, double *y)
{
   if (x) *x = pd->end.x;
   if (y) *y = pd->end.y;
}

static void
_efl_canvas_vg_gradient_linear_render_pre(Eo *obj,
                                   Eina_Matrix3 *parent,
                                   Ector_Surface *s,
                                   void *data,
                                   Efl_Canvas_Vg_Node_Data *nd)
{
   Efl_Canvas_Vg_Gradient_Linear_Data *pd = data;
   Efl_Canvas_Vg_Gradient_Data *gd;

   if (nd->flags == EFL_GFX_CHANGE_FLAG_NONE) return ;

   nd->flags = EFL_GFX_CHANGE_FLAG_NONE;

   gd = efl_data_scope_get(obj, EFL_CANVAS_VG_GRADIENT_CLASS);
   EFL_CANVAS_VG_COMPUTE_MATRIX(current, parent, nd);

   if (!nd->renderer)
     {
        efl_domain_current_push(EFL_ID_DOMAIN_SHARED);
        nd->renderer = ector_surface_renderer_factory_new(s, ECTOR_RENDERER_GRADIENT_LINEAR_MIXIN);
        efl_domain_current_pop();
     }

   ector_renderer_transformation_set(nd->renderer, current);
   ector_renderer_origin_set(nd->renderer, nd->x, nd->y);
   ector_renderer_color_set(nd->renderer, nd->r, nd->g, nd->b, nd->a);
   ector_renderer_visibility_set(nd->renderer, nd->visibility);
   efl_gfx_gradient_stop_set(nd->renderer, gd->colors, gd->colors_count);
   efl_gfx_gradient_spread_set(nd->renderer, gd->s);
   efl_gfx_gradient_linear_start_set(nd->renderer, pd->start.x, pd->start.y);
   efl_gfx_gradient_linear_end_set(nd->renderer, pd->end.x, pd->end.y);

   //Prepare renderer triggered by ector shape this gradient applied to.
   //ector_renderer_prepare(nd->renderer);
}

static Eo *
_efl_canvas_vg_gradient_linear_efl_object_constructor(Eo *obj,
                                            Efl_Canvas_Vg_Gradient_Linear_Data *pd)
{
   Efl_Canvas_Vg_Node_Data *nd;

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   nd = efl_data_scope_get(obj, EFL_CANVAS_VG_NODE_CLASS);
   nd->render_pre = _efl_canvas_vg_gradient_linear_render_pre;
   nd->data = pd;

   return obj;
}

static void
_efl_canvas_vg_gradient_linear_efl_object_destructor(Eo *obj, Efl_Canvas_Vg_Gradient_Linear_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

static void
_efl_canvas_vg_gradient_linear_efl_gfx_path_bounds_get(const Eo *obj, Efl_Canvas_Vg_Gradient_Linear_Data *pd, Eina_Rect *r)
{
   Efl_Canvas_Vg_Node_Data *nd;

   nd = efl_data_scope_get(obj, EFL_CANVAS_VG_NODE_CLASS);
   EINA_RECTANGLE_SET(r,
                      nd->x + pd->start.x, nd->y + pd->start.y,
                      pd->end.x - pd->start.x, pd->end.y - pd->start.x);
}

static Eina_Bool
_efl_canvas_vg_gradient_linear_efl_gfx_path_interpolate(Eo *obj,
                                                Efl_Canvas_Vg_Gradient_Linear_Data *pd,
                                                const Efl_VG *from, const Efl_VG *to,
                                                double pos_map)
{
   Efl_Canvas_Vg_Gradient_Linear_Data *fromd, *tod;
   double from_map;
   Eina_Bool r;

   r = efl_gfx_path_interpolate(efl_super(obj, EFL_CANVAS_VG_GRADIENT_LINEAR_CLASS), from, to, pos_map);

   if (!r) return EINA_FALSE;

   fromd = efl_data_scope_get(from, EFL_CANVAS_VG_GRADIENT_LINEAR_CLASS);
   tod = efl_data_scope_get(to, EFL_CANVAS_VG_GRADIENT_LINEAR_CLASS);
   from_map = 1.0 - pos_map;

#define INTP(Pd, From, To, Member, From_Map, Pos_Map)   \
   Pd->Member = From->Member * From_Map + To->Member * Pos_Map

   INTP(pd, fromd, tod, start.x, from_map, pos_map);
   INTP(pd, fromd, tod, start.y, from_map, pos_map);
   INTP(pd, fromd, tod, end.x, from_map, pos_map);
   INTP(pd, fromd, tod, end.y, from_map, pos_map);

#undef INTP

   return EINA_TRUE;
}

EOLIAN static Efl_VG *
_efl_canvas_vg_gradient_linear_efl_duplicate_duplicate(const Eo *obj, Efl_Canvas_Vg_Gradient_Linear_Data *pd)
{
   Efl_VG *cn = NULL;

   cn = efl_duplicate(efl_super(obj, MY_CLASS));
   efl_gfx_gradient_linear_start_set(cn, pd->start.x, pd->start.y);
   efl_gfx_gradient_linear_end_set(cn, pd->end.x, pd->end.y);
   return cn;
}

EAPI void
evas_vg_gradient_linear_start_set(Eo *obj, double x, double y)
{
   efl_gfx_gradient_linear_start_set(obj, x, y);
}

EAPI void
evas_vg_gradient_linear_start_get(Eo *obj, double *x, double *y)
{
   efl_gfx_gradient_linear_start_get(obj, x, y);
}

EAPI void
evas_vg_gradient_linear_end_set(Eo *obj, double x, double y)
{
   efl_gfx_gradient_linear_end_set(obj, x, y);
}

EAPI void
evas_vg_gradient_linear_end_get(Eo *obj, double *x, double *y)
{
   efl_gfx_gradient_linear_end_get(obj, x, y);
}

EAPI Efl_VG*
evas_vg_gradient_linear_add(Efl_VG *parent)
{
   return efl_add(EFL_CANVAS_VG_GRADIENT_LINEAR_CLASS, parent);
}

#include "efl_canvas_vg_gradient_linear.eo.c"

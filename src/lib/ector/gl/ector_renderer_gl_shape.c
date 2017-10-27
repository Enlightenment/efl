#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>
#include <float.h>

#include <Eina.h>
#include <Ector.h>
#include <gl/Ector_Gl.h>

#include "ector_private.h"
#include "ector_gl_private.h"
#include "triangulator_stroker.h"
#include "triangulator_simple.h"
#include "draw.h"

typedef struct _Ector_Renderer_Gl_Shape_Data Ector_Renderer_Gl_Shape_Data;
struct _Ector_Renderer_Gl_Shape_Data
{
   Efl_Gfx_Shape_Public                *public_shape;

   Ector_Renderer_Shape_Data           *shape;
   Ector_Renderer_Data                 *base;

   Triangulator_Simple                 *filler;
   Triangulator_Stroker                *stroker;
};

static Eina_Bool
_ector_renderer_gl_shape_ector_renderer_prepare(Eo *obj EINA_UNUSED,
                                                             Ector_Renderer_Gl_Shape_Data *pd)
{
   // FIXME: shouldn't that be part of the shape generic implementation ?
   if (pd->shape->fill)
     ector_renderer_prepare(pd->shape->fill);
   if (pd->shape->stroke.fill)
     ector_renderer_prepare(pd->shape->stroke.fill);

   return EINA_TRUE;
}

static Eina_Bool
_generate_stroke_data(Ector_Renderer_Gl_Shape_Data *pd)
{
   if (pd->stroker) return EINA_FALSE;

   if (!pd->shape->stroke.fill &&
       ((pd->public_shape->stroke.color.a == 0) ||
        (pd->public_shape->stroke.width < 0.01)))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_generate_shape_data(Ector_Renderer_Gl_Shape_Data *pd)
{
   if (pd->filler) return EINA_FALSE;

   if (!pd->shape->fill && (pd->base->color.a == 0)) return EINA_FALSE;

   return EINA_TRUE;
}


static void _shape_transform(double *pts, int count , Eina_Matrix3 *m)
{
   int i;
   double x, y;
   if (m && (eina_matrix3_type_get(m) != EINA_MATRIX_TYPE_IDENTITY))
     {
        for (i = 0; i < count; i= i+2)
          {
             eina_matrix3_point_transform(m,
                                          pts[i],
                                          pts[i+1],
                                          &x, &y);

             pts[i] = x;
             pts[i+1] = y;
          }
     }
}

static void
_update_triangulators(Eo *obj, Ector_Renderer_Gl_Shape_Data *pd)
{
   const Efl_Gfx_Path_Command *cmds = NULL;
   const double *pts;
   double *new_pts;
   unsigned int cmd_count = 0, pt_count = 0;
   Eina_Bool cleanup = EINA_FALSE;

   efl_gfx_path_get(obj, &cmds, &pts);
   if (cmds && (_generate_stroke_data(pd) || _generate_shape_data(pd)))
     {
        efl_gfx_path_length_get(obj, &cmd_count, &pt_count);

        if (pd->base->m &&
            (eina_matrix3_type_get(pd->base->m) != EINA_MATRIX_TYPE_IDENTITY))
          {
             new_pts =  malloc(sizeof(double) * pt_count);
             memcpy(new_pts, pts, sizeof(double) * pt_count);
             _shape_transform(new_pts, pt_count, pd->base->m);
             cleanup = EINA_TRUE;
          }
        else
          {
             new_pts = (double *)pts;
          }

        //shape triangulator creation
        if (_generate_shape_data(pd))
          {
             pd->filler = triangulator_simple_new();
             triangulator_simple_process(pd->filler, cmds, new_pts, 0);
          }

        //stroke triangulator creation
        if ( _generate_stroke_data(pd))
          {
             pd->stroker = triangulator_stroker_new();
             triangulator_stroker_stroke_set(pd->stroker,
                                             (pd->public_shape->stroke.width * pd->public_shape->stroke.scale),
                                             pd->public_shape->stroke.cap,
                                             pd->public_shape->stroke.join,
                                             pd->base->m);

             triangulator_stroker_process(pd->stroker, cmds, new_pts, cmd_count, pt_count);
          }
        if (cleanup) free(new_pts);
     }
}


static Eina_Bool
_ector_renderer_gl_shape_ector_renderer_draw(Eo *obj EINA_UNUSED,
                                             Ector_Renderer_Gl_Shape_Data *pd EINA_UNUSED,
                                             Efl_Gfx_Render_Op op EINA_UNUSED, Eina_Array *clips EINA_UNUSED,
                                             unsigned int mul_col EINA_UNUSED)
{
   float *vertex;
   int *stops , count, stop_count;
   unsigned int color;

   _update_triangulators(obj, pd);

   ector_gl_engine_param_offset(pd->base->origin.x, pd->base->origin.y);

   if (pd->filler)
     {
        if (pd->shape->fill)
          ector_renderer_gl_fill(pd->shape->fill);
        else
          {
             color = DRAW_ARGB_JOIN(pd->base->color.a,
                                    pd->base->color.r,
                                    pd->base->color.g,
                                    pd->base->color.b);
             color = DRAW_MUL4_SYM(color, mul_col);
             ector_gl_engine_param_color(color);
          }

        vertex = eina_inarray_nth(pd->filler->vertices, 0);
        count = eina_inarray_count(pd->filler->vertices);
        stop_count = eina_inarray_count(pd->filler->stops);
        stops = eina_inarray_nth(pd->filler->stops, 0);

        ector_gl_engine_path_fill(vertex, count, stops, stop_count);
     }

  if (pd->stroker)
    {
        if (pd->shape->stroke.fill)
          ector_renderer_gl_fill(pd->shape->stroke.fill);
        else
          {
             color = DRAW_ARGB_JOIN(pd->public_shape->stroke.color.a,
                                    pd->public_shape->stroke.color.r,
                                    pd->public_shape->stroke.color.g,
                                    pd->public_shape->stroke.color.b);
             color = DRAW_MUL4_SYM(color, mul_col);
             ector_gl_engine_param_color(color);
          }

        vertex = eina_inarray_nth(pd->stroker->vertices, 0);
        count = eina_inarray_count(pd->stroker->vertices);

        ector_gl_engine_path_stroke(vertex, count);
    }

   return EINA_TRUE;
}

static void
_clear_path_compute_data(Ector_Renderer_Gl_Shape_Data *pd)
{
   if (pd->filler) triangulator_simple_free(pd->filler);
   if (pd->stroker) triangulator_stroker_free(pd->stroker);

   pd->filler = NULL;
   pd->stroker = NULL;
}

static Eina_Bool
_ector_renderer_gl_shape_ector_renderer_gl_fill(Eo *obj EINA_UNUSED,
                                                Ector_Renderer_Gl_Shape_Data *pd EINA_UNUSED)
{
   // FIXME: let's find out how to fill a shape with a shape later.
   // I need to read SVG specification and see how to map that with software.
   return EINA_FALSE;
}

static void
_ector_renderer_gl_shape_efl_gfx_path_path_set(Eo *obj,
                                               Ector_Renderer_Gl_Shape_Data *pd EINA_UNUSED,
                                               const Efl_Gfx_Path_Command *op,
                                               const double *points)
{

   efl_gfx_path_set(efl_super(obj, ECTOR_RENDERER_GL_SHAPE_CLASS), op, points);

   _clear_path_compute_data(pd);
}


static void
_ector_renderer_gl_shape_path_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ector_Renderer_Gl_Shape_Data *pd = data;

   _clear_path_compute_data(pd);
}

static Efl_Object *
_ector_renderer_gl_shape_efl_object_constructor(Eo *obj, Ector_Renderer_Gl_Shape_Data *pd)
{
   obj = efl_constructor(efl_super(obj, ECTOR_RENDERER_GL_SHAPE_CLASS));

   if (!obj) return NULL;

   pd->public_shape = efl_data_xref(obj, EFL_GFX_SHAPE_MIXIN, obj);
   pd->shape = efl_data_xref(obj, ECTOR_RENDERER_SHAPE_MIXIN, obj);
   pd->base = efl_data_xref(obj, ECTOR_RENDERER_CLASS, obj);

   efl_event_callback_add(obj, EFL_GFX_PATH_EVENT_CHANGED, _ector_renderer_gl_shape_path_changed, pd);

   return obj;
}

void
_ector_renderer_gl_shape_efl_object_destructor(Eo *obj, Ector_Renderer_Gl_Shape_Data *pd)
{
   //FIXME, As base class  destructor can't call destructor of mixin class.
   // call explicit API to free shape data.
   efl_gfx_path_reset(obj);

   _clear_path_compute_data(pd);

   efl_data_xunref(obj, pd->public_shape, obj);
   efl_data_xunref(obj, pd->shape, obj);
   efl_data_xunref(obj, pd->base, obj);

   efl_destructor(efl_super(obj, ECTOR_RENDERER_GL_SHAPE_CLASS));
}


unsigned int
_ector_renderer_gl_shape_ector_renderer_base_crc_get(Eo *obj EINA_UNUSED,
                                                             Ector_Renderer_Gl_Shape_Data *pd EINA_UNUSED)
{
   return 0;
}

#include "ector_renderer_gl_shape.eo.c"

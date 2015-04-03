#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>
#include <float.h>

#include <Eina.h>
#include <Ector.h>
#include <cairo/Ector_Cairo.h>

#include "ector_private.h"
#include "ector_cairo_private.h"

typedef struct _cairo_path_t cairo_path_t;

static void (*cairo_move_to)(cairo_t *cr, double x, double y) = NULL;
static void (*cairo_line_to)(cairo_t *cr, double x, double y) = NULL;
static void (*cairo_curve_to)(cairo_t *cr,
                              double x1, double y1,
                              double x2, double y2,
                              double x3, double y3) = NULL;
static void (*cairo_close_path)(cairo_t *cr) = NULL;

static void (*cairo_fill)(cairo_t *cr) = NULL;
static void (*cairo_fill_preserve)(cairo_t *cr) = NULL;
static void (*cairo_stroke)(cairo_t *cr) = NULL;

static void (*cairo_set_source_rgba)(cairo_t *cr,
                                     double red, double green,
                                     double blue, double alpha) = NULL;


static cairo_path_t *(*cairo_copy_path)(cairo_t *cr) = NULL;
static void (*cairo_path_destroy)(cairo_path_t *path) = NULL;
static void (*cairo_new_path)(cairo_t *cr) = NULL;
static void (*cairo_append_path)(cairo_t *cr, const cairo_path_t *path) = NULL;

typedef struct _Ector_Renderer_Cairo_Shape_Data Ector_Renderer_Cairo_Shape_Data;
struct _Ector_Renderer_Cairo_Shape_Data
{
   Ector_Cairo_Surface_Data *parent;
   Ector_Renderer_Generic_Shape_Data *shape;
   Ector_Renderer_Generic_Base_Data *base;
   cairo_path_t *path;
};

static Eina_Bool
_ector_renderer_cairo_shape_ector_renderer_generic_base_prepare(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd)
{
   // FIXME: shouldn't that be part of the shape generic implementation ?
   if (pd->shape->fill)
     eo_do(pd->shape->fill, ector_renderer_prepare());
   if (pd->shape->stroke.fill)
     eo_do(pd->shape->stroke.fill, ector_renderer_prepare());
   if (pd->shape->stroke.marker)
     eo_do(pd->shape->stroke.marker, ector_renderer_prepare());
   eo_do_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS, ector_renderer_prepare());

   // shouldn't that be moved to the cairo base object
   if (!pd->parent)
     {
        Eo *parent;

        eo_do(obj, parent = eo_parent_get());
        if (!parent) return EINA_FALSE;
        pd->parent = eo_data_xref(parent, ECTOR_CAIRO_SURFACE_CLASS, obj);
        if (!pd->parent) return EINA_FALSE;
     }

   if (!pd->path && pd->shape->path.cmd)
     {
        double *pts;
        unsigned int i;

        USE(obj, cairo_new_path, EINA_FALSE);

        cairo_new_path(pd->parent->cairo);

        pts = pd->shape->path.pts;
        for (i = 0; pd->shape->path.cmd[i] != EFL_GRAPHICS_PATH_COMMAND_TYPE_END; i++)
          {
             switch (pd->shape->path.cmd[i])
               {
                case EFL_GRAPHICS_PATH_COMMAND_TYPE_MOVE_TO:
                   USE(obj, cairo_move_to, EINA_FALSE);

                   cairo_move_to(pd->parent->cairo, pts[0], pts[1]);

                   pts += 2;
                   break;
                case EFL_GRAPHICS_PATH_COMMAND_TYPE_LINE_TO:
                   USE(obj, cairo_line_to, EINA_FALSE);

                   cairo_line_to(pd->parent->cairo, pts[0], pts[1]);

                   pts += 2;
                   break;
                case EFL_GRAPHICS_PATH_COMMAND_TYPE_CUBIC_TO:
                   USE(obj, cairo_curve_to, EINA_FALSE);

                   // Be careful, we do have a different order than
                   // cairo, first is destination point, followed by
                   // the control point. The opposite of cairo.
                   cairo_curve_to(pd->parent->cairo,
                                  pts[2], pts[3], pts[4], pts[5], // control points
                                  pts[0], pts[1]); // destination point

                   pts += 6;
                   break;
                case EFL_GRAPHICS_PATH_COMMAND_TYPE_CLOSE:
                   USE(obj, cairo_close_path, EINA_FALSE);

                   cairo_close_path(pd->parent->cairo);
                   break;
                case EFL_GRAPHICS_PATH_COMMAND_TYPE_LAST:
                case EFL_GRAPHICS_PATH_COMMAND_TYPE_END:
                   break;
               }
          }

        USE(obj, cairo_copy_path, EINA_FALSE);

        pd->path = cairo_copy_path(pd->parent->cairo);
     }

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_cairo_shape_ector_renderer_generic_base_draw(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd, Ector_Rop op, Eina_Array *clips, int x, int y, unsigned int mul_col)
{
   if (pd->path == NULL) return EINA_FALSE;

   // FIXME: find a way to offset the drawing and setting multiple clips

   eo_do_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS, ector_renderer_draw(op, clips, x, y, mul_col));

   USE(obj, cairo_new_path, EINA_FALSE);
   USE(obj, cairo_append_path, EINA_FALSE);

   cairo_new_path(pd->parent->cairo);
   cairo_append_path(pd->parent->cairo, pd->path);

   if (pd->shape->fill)
     eo_do(pd->shape->fill, ector_renderer_cairo_base_fill());

   if (pd->shape->stroke.color.a > 0)
     {
        USE(obj, cairo_fill_preserve, EINA_FALSE);
        USE(obj, cairo_set_source_rgba, EINA_FALSE);
        USE(obj, cairo_stroke, EINA_FALSE);

        cairo_fill_preserve(pd->parent->cairo);

        cairo_set_source_rgba(pd->parent->cairo,
                              pd->shape->stroke.color.r / 255.0,
                              pd->shape->stroke.color.g / 255.0,
                              pd->shape->stroke.color.b / 255.0,
                              pd->shape->stroke.color.a / 255.0);

        if (pd->shape->stroke.fill)
          eo_do(pd->shape->stroke.fill, ector_renderer_cairo_base_fill());
        // Set dash, cap and join
        cairo_stroke(pd->parent->cairo);
     }
   else
     {
        USE(obj, cairo_fill, EINA_FALSE);
        cairo_fill(pd->parent->cairo);
     }

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_cairo_shape_ector_renderer_cairo_base_fill(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd)
{
   // FIXME: let's find out how to fill a shape with a shape later.
   // I need to read SVG specification and see how to map that with cairo.
}

static void
_ector_renderer_cairo_shape_efl_gfx_shape_path_set(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd,
                                                   const Efl_Gfx_Path_Command *op, const double *points)
{
   USE(obj, cairo_path_destroy, );

   if (pd->path) cairo_path_destroy(pd->path);
   pd->path = NULL;

   eo_do_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS, efl_graphics_shape_path_set(op, points));
}


void
_ector_renderer_cairo_shape_eo_base_constructor(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd)
{
   eo_do_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS, eo_constructor());
   pd->shape = eo_data_xref(obj, ECTOR_RENDERER_GENERIC_SHAPE_CLASS, obj);
   pd->base = eo_data_xref(obj, ECTOR_RENDERER_GENERIC_BASE_CLASS, obj);
}

void
_ector_renderer_cairo_shape_eo_base_destructor(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd)
{
   Eo *parent;

   USE(obj, cairo_path_destroy, );
   if (pd->path) cairo_path_destroy(pd->path);

   eo_do(obj, parent = eo_parent_get());
   eo_data_xunref(parent, pd->parent, obj);

   eo_data_xunref(obj, pd->shape, obj);
   eo_data_xunref(obj, pd->base, obj);
   eo_do_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS, eo_destructor());
}


#include "ector_renderer_cairo_shape.eo.c"

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

// This function come from librsvg rsvg-path.c
static void
_ector_arc_segment(Eo *obj, cairo_t* ctx,
                   double xc, double yc,
                   double th0, double th1, double rx, double ry,
                   double x_axis_rotation)
{
   double x1, y1, x2, y2, x3, y3;
   double t;
   double th_half;
   double f, sinf, cosf;

   f = x_axis_rotation * M_PI / 180.0;
   sinf = sin(f);
   cosf = cos(f);

   th_half = 0.5 * (th1 - th0);
   t = (8.0 / 3.0) * sin(th_half * 0.5) * sin(th_half * 0.5) / sin(th_half);
   x1 = rx * (cos(th0) - t * sin(th0));
   y1 = ry * (sin(th0) + t * cos(th0));
   x3 = rx* cos(th1);
   y3 = ry* sin(th1);
   x2 = x3 + rx * (t * sin(th1));
   y2 = y3 + ry * (-t * cos(th1));

   USE(obj, cairo_curve_to, );

   cairo_curve_to(ctx,
                  xc + cosf * x1 - sinf * y1,
                  yc + sinf * x1 + cosf * y1,
                  xc + cosf * x2 - sinf * y2,
                  yc + sinf * x2 + cosf * y2,
                  xc + cosf * x3 - sinf * y3,
                  yc + sinf * x3 + cosf * y3);
}

// This function come from librsvg rsvg-path.c
static void
_ector_arc_to(Eo *obj, cairo_t* ctx,
              double *current_x, double *current_y,
              double rx, double ry, double x_axis_rotation,
              Eina_Bool large_arc_flag, Eina_Bool sweep_flag,
              double x, double y)
{
   /* See Appendix F.6 Elliptical arc implementation notes
      http://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes */

   double f, sinf, cosf;
   double x1, y1, x2, y2;
   double x1_, y1_;
   double cx_, cy_, cx, cy;
   double gamma;
   double theta1, delta_theta;
   double k1, k2, k3, k4, k5;

   int i, n_segs;

   /* Start and end of path segment */
   x1 = *current_x;
   y1 = *current_y;

   x2 = x;
   y2 = y;

   if (x1 == x2 && y1 == y2)
     return;

   /* X-axis */
   f = x_axis_rotation * M_PI / 180.0;
   sinf = sin(f);
   cosf = cos(f);

   /* Check the radius against floading point underflow.
      See http://bugs.debian.org/508443 */
   if ((fabs(rx) < DBL_EPSILON) || (fabs(ry) < DBL_EPSILON)) {
      USE(obj, cairo_line_to, );

      cairo_line_to(ctx, x, y);

      *current_x = x;
      *current_y = y;
      return;
   }

   if (rx < 0) rx = -rx;
   if (ry < 0) ry = -ry;

   k1 = (x1 - x2) / 2;
   k2 = (y1 - y2) / 2;

   x1_ = cosf * k1 + sinf * k2;
   y1_ = -sinf * k1 + cosf * k2;

   gamma = (x1_ * x1_) / (rx * rx) + (y1_ * y1_) / (ry * ry);
   if (gamma > 1) {
      rx *= sqrt(gamma);
      ry *= sqrt(gamma);
   }

   /* Compute the center */
   k1 = rx * rx * y1_ * y1_ + ry * ry * x1_ * x1_;
   if (k1 == 0)
     return;

   k1 = sqrt(fabs((rx * rx * ry * ry) / k1 - 1));
   if (sweep_flag == large_arc_flag)
     k1 = -k1;

   cx_ = k1 * rx * y1_ / ry;
   cy_ = -k1 * ry * x1_ / rx;

   cx = cosf * cx_ - sinf * cy_ + (x1 + x2) / 2;
   cy = sinf * cx_ + cosf * cy_ + (y1 + y2) / 2;

   /* Compute start angle */
   k1 = (x1_ - cx_) / rx;
   k2 = (y1_ - cy_) / ry;
   k3 = (-x1_ - cx_) / rx;
   k4 = (-y1_ - cy_) / ry;

   k5 = sqrt(fabs(k1 * k1 + k2 * k2));
   if (k5 == 0) return;

   k5 = k1 / k5;
   if (k5 < -1) k5 = -1;
   else if(k5 > 1) k5 = 1;

   theta1 = acos(k5);
   if(k2 < 0) theta1 = -theta1;

   /* Compute delta_theta */
   k5 = sqrt(fabs((k1 * k1 + k2 * k2) * (k3 * k3 + k4 * k4)));
   if (k5 == 0) return;

   k5 = (k1 * k3 + k2 * k4) / k5;
   if (k5 < -1) k5 = -1;
   else if (k5 > 1) k5 = 1;
   delta_theta = acos(k5);
   if(k1 * k4 - k3 * k2 < 0) delta_theta = -delta_theta;

   if (sweep_flag && delta_theta < 0)
     delta_theta += M_PI*2;
   else if (!sweep_flag && delta_theta > 0)
     delta_theta -= M_PI*2;

   /* Now draw the arc */
   n_segs = ceil (fabs (delta_theta / (M_PI * 0.5 + 0.001)));

   for (i = 0; i < n_segs; i++)
     _ector_arc_segment(obj, ctx,
                        cx, cy,
                        theta1 + i * delta_theta / n_segs,
                        theta1 + (i + 1) * delta_theta / n_segs,
                        rx, ry, x_axis_rotation);

   *current_x = x;
   *current_y = y;
}

static Eina_Bool
_ector_renderer_cairo_shape_ector_renderer_generic_base_prepare(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd, Ector_Surface *s)
{
   // FIXME: shouldn't that be part of the shape generic implementation ?
   if (pd->shape->fill)
     eo_do(pd->shape->fill, ector_renderer_prepare(s));
   if (pd->shape->stroke.fill)
     eo_do(pd->shape->stroke.fill, ector_renderer_prepare(s));
   if (pd->shape->stroke.marker)
     eo_do(pd->shape->stroke.marker, ector_renderer_prepare(s));
   eo_do_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS, ector_renderer_prepare(s));

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
        double current_x = 0, current_y = 0;
        double current_ctrl_x = 0, current_ctrl_y = 0;
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

                   current_ctrl_x = current_x = pts[0];
                   current_ctrl_y = current_y = pts[1];

                   pts += 2;
                   break;
                case EFL_GRAPHICS_PATH_COMMAND_TYPE_LINE_TO:
                   USE(obj, cairo_line_to, EINA_FALSE);

                   cairo_line_to(pd->parent->cairo, pts[0], pts[1]);

                   current_ctrl_x = current_x = pts[0];
                   current_ctrl_y = current_y = pts[1];

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

                   current_ctrl_x = pts[4];
                   current_ctrl_y = pts[5];
                   current_x = pts[0];
                   current_y = pts[1];

                   pts += 6;
                   break;
                case EFL_GRAPHICS_PATH_COMMAND_TYPE_ARC_TO:
                   _ector_arc_to(obj, pd->parent->cairo,
                                 &current_x, &current_y,
                                 pts[2], pts[3], pts[4],
                                 0, 0, // FIXME: need to get the large arc and sweep flag
                                 pts[0], pts[1]);

                   pts += 5;
                   break;
                case EFL_GRAPHICS_PATH_COMMAND_TYPE_CLOSE:
                   USE(obj, cairo_close_path, EINA_FALSE);

                   cairo_close_path(pd->parent->cairo);
                   break;
                case EFL_GRAPHICS_PATH_COMMAND_TYPE_QUADRATIC_TO:
                  {
                     double x1, y1, x2, y2, x3, y3;
                     // This code come from librsvg rsvg-path.c
                     // Be careful, we do have a different order than
                     // cairo, first is destination point, followed by
                     // the control point. The opposite of cairo.
                     /* raise quadratic bezier to cubic */
                     x1 = (current_x + 2 * pts[2]) * (1.0 / 3.0);
                     y1 = (current_y + 2 * pts[3]) * (1.0 / 3.0);
                     x3 = pts[0];
                     y3 = pts[1];
                     x2 = (x3 + 2 * pts[2]) * (1.0 / 3.0);
                     y2 = (y3 + 2 * pts[3]) * (1.0 / 3.0);

                     cairo_curve_to(pd->parent->cairo,
                                    x1, y1, x2, y2, // control points
                                    x3, y3); // destination point

                     current_ctrl_x = pts[2];
                     current_ctrl_y = pts[3];

                     current_x = x3;
                     current_y = y3;
                     break;
                  }
                case EFL_GRAPHICS_PATH_COMMAND_TYPE_SQUADRATIC_TO:
                  {
                     // This code come from librsvg rsvg-path.c
                     // Smooth quadratic basically reusing the last control
                     // point in a meaningful way.
                     double xc, yc; /* quadratic control point */
                     double x1, y1, x2, y2, x3, y3;

                     xc = 2 * current_x - current_ctrl_x;
                     yc = 2 * current_y - current_ctrl_y;
                     /* generate a quadratic bezier with control point = xc, yc */
                     x1 = (current_x + 2 * xc) * (1.0 / 3.0);
                     y1 = (current_y + 2 * yc) * (1.0 / 3.0);
                     x3 = pts[0];
                     y3 = pts[1];
                     x2 = (x3 + 2 * xc) * (1.0 / 3.0);
                     y2 = (y3 + 2 * yc) * (1.0 / 3.0);

                     USE(obj, cairo_curve_to, EINA_FALSE);

                     cairo_curve_to(pd->parent->cairo,
                                    x1, y1, x2, y2, x3, y3);

                     current_ctrl_x = xc;
                     current_ctrl_y = yc;

                     current_x = x3;
                     current_y = y3;

                     break;
                  }
                case EFL_GRAPHICS_PATH_COMMAND_TYPE_SCUBIC_TO:
                  {
                     // This code come from librsvg rsvg-path.c
                     // Smooth cubic basically reusing the last control point
                     // in a meaningful way.
                     double x1, y1, x2, y2, x3, y3;

                     x1 = 2 * current_x - current_ctrl_x;
                     y1 = 2 * current_y - current_ctrl_y;
                     x2 = pts[2];
                     y2 = pts[3];
                     x3 = pts[0];
                     y3 = pts[1];

                     USE(obj, cairo_curve_to, EINA_FALSE);

                     cairo_curve_to(pd->parent->cairo,
                                    x1, y1, x2, y2, x3, y3);

                     current_ctrl_x = x2;
                     current_ctrl_y = y2;
                     current_x = x3;
                     current_y = y3;
                     break;
                  }
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
_ector_renderer_cairo_shape_ector_renderer_generic_base_draw(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd, Ector_Rop op, Eina_Array *clips, int x, int y)
{
   if (pd->path == NULL) return EINA_FALSE;

   // FIXME: find a way to offset the drawing and setting multiple clips

   eo_do_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS, ector_renderer_draw(op, clips, x, y));

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

static Eina_Bool
_ector_renderer_cairo_shape_efl_graphics_shape_path_set(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd,
                                                        const Efl_Graphics_Path_Command *op, const double *points)
{
   Eina_Bool r;

   USE(obj, cairo_path_destroy, EINA_FALSE);

   if (pd->path) cairo_path_destroy(pd->path);
   pd->path = NULL;

   eo_do_super(obj, ECTOR_RENDERER_CAIRO_SHAPE_CLASS, r = efl_graphics_shape_path_set(op, points));

   return r;
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

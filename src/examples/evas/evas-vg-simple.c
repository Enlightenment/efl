/**
 * Simple Evas example illustrating a custom Evas box object
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one). See stdout/stderr for output.
 *
 * @verbatim
 * gcc -o evas-box evas-box.c `pkg-config --libs --cflags evas ecore ecore-evas eina ector eo efl`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#endif

#define WIDTH 400
#define HEIGHT 400

#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT 1
#endif

#ifndef EFL_EO_API_SUPPORT
#define EFL_EO_API_SUPPORT 1
#endif

#include <Eo.h>
#include <Efl.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>


#include <math.h>
#include <Eina.h>

#define PATH_KAPPA 0.5522847498
#define PI         3.1415926535

typedef struct _Bezier
{
float x1, y1, x2, y2, x3, y3, x4, y4;
}Bezier;

typedef struct _Point
{
    int x;
    int y;
}Point;

static
Bezier bezierFromPoints(Point p1, Point p2,
                            Point p3, Point p4)
{
    Bezier b;
    b.x1 = p1.x;
    b.y1 = p1.y;
    b.x2 = p2.x;
    b.y2 = p2.y;
    b.x3 = p3.x;
    b.y3 = p3.y;
    b.x4 = p4.x;
    b.y4 = p4.y;
    return b;
}

inline void
parameterSplitLeft(Bezier *b, float t, Bezier *left)
{
    left->x1 = b->x1;
    left->y1 = b->y1;

    left->x2 = b->x1 + t * ( b->x2 - b->x1 );
    left->y2 = b->y1 + t * ( b->y2 - b->y1 );

    left->x3 = b->x2 + t * ( b->x3 - b->x2 ); // temporary holding spot
    left->y3 = b->y2 + t * ( b->y3 - b->y2 ); // temporary holding spot

    b->x3 = b->x3 + t * ( b->x4 - b->x3 );
    b->y3 = b->y3 + t * ( b->y4 - b->y3 );

    b->x2 = left->x3 + t * ( b->x3 - left->x3);
    b->y2 = left->y3 + t * ( b->y3 - left->y3);

    left->x3 = left->x2 + t * ( left->x3 - left->x2 );
    left->y3 = left->y2 + t * ( left->y3 - left->y2 );

    left->x4 = b->x1 = left->x3 + t * (b->x2 - left->x3);
    left->y4 = b->y1 = left->y3 + t * (b->y2 - left->y3);
}
static
Bezier bezierOnInterval(Bezier *b, float t0, float t1)
{
    if (t0 == 0 && t1 == 1)
        return *b;

    Bezier result;
    parameterSplitLeft(b, t0, &result);
    float trueT = (t1-t0)/(1-t0);
    parameterSplitLeft(b, trueT, &result);

    return result;
}

inline void
_bezier_coefficients(float t, float *ap, float *bp, float *cp, float *dp)
{
    float a,b,c,d;
    float m_t = 1. - t;
    b = m_t * m_t;
    c = t * t;
    d = c * t;
    a = b * m_t;
    b *= 3. * t;
    c *= 3. * m_t;
    *ap = a;
    *bp = b;
    *cp = c;
    *dp = d;
}

static
float _t_for_arc_angle(float angle)
{
    if (angle < 0.00001)
        return 0;

    if (angle == 90.0)
        return 1;

    float radians = PI * angle / 180;
    float cosAngle = cos(radians);
    float sinAngle = sin(radians);

    // initial guess
    float tc = angle / 90;
    // do some iterations of newton's method to approximate cosAngle
    // finds the zero of the function b.pointAt(tc).x() - cosAngle
    tc -= ((((2-3*PATH_KAPPA) * tc + 3*(PATH_KAPPA-1)) * tc) * tc + 1 - cosAngle) // value
         / (((6-9*PATH_KAPPA) * tc + 6*(PATH_KAPPA-1)) * tc); // derivative
    tc -= ((((2-3*PATH_KAPPA) * tc + 3*(PATH_KAPPA-1)) * tc) * tc + 1 - cosAngle) // value
         / (((6-9*PATH_KAPPA) * tc + 6*(PATH_KAPPA-1)) * tc); // derivative

    // initial guess
    float ts = tc;
    // do some iterations of newton's method to approximate sinAngle
    // finds the zero of the function b.pointAt(tc).y() - sinAngle
    ts -= ((((3*PATH_KAPPA-2) * ts -  6*PATH_KAPPA + 3) * ts + 3*PATH_KAPPA) * ts - sinAngle)
         / (((9*PATH_KAPPA-6) * ts + 12*PATH_KAPPA - 6) * ts + 3*PATH_KAPPA);
    ts -= ((((3*PATH_KAPPA-2) * ts -  6*PATH_KAPPA + 3) * ts + 3*PATH_KAPPA) * ts - sinAngle)
         / (((9*PATH_KAPPA-6) * ts + 12*PATH_KAPPA - 6) * ts + 3*PATH_KAPPA);

    // use the average of the t that best approximates cosAngle
    // and the t that best approximates sinAngle
    float t = 0.5 * (tc + ts);
    return t;
}

static void
_find_ellipse_coords(int x, int y, int w, int h, float angle, float length,
                            Point* startPoint, Point *endPoint)
{
    if (!w || !h ) {
        if (startPoint)
            startPoint->x = 0 , startPoint->y = 0;
        if (endPoint)
            endPoint->x = 0 , endPoint->y = 0;
        return;
    }

    int w2 = w / 2;
    int h2 = h / 2;

    float angles[2] = { angle, angle + length };
    Point *points[2] = { startPoint, endPoint };
    int i =0;
    for (i = 0; i < 2; ++i) {
        if (!points[i])
            continue;

        float theta = angles[i] - 360 * floor(angles[i] / 360);
        float t = theta / 90;
        // truncate
        int quadrant = (int)t;
        t -= quadrant;

        t = _t_for_arc_angle(90 * t);

        // swap x and y?
        if (quadrant & 1)
            t = 1 - t;

        float a, b, c, d;
        _bezier_coefficients(t, &a, &b, &c, &d);
        float px = a + b + c*PATH_KAPPA;
        float py = d + c + b*PATH_KAPPA;

        // left quadrants
        if (quadrant == 1 || quadrant == 2)
            px = -px;

        // top quadrants
        if (quadrant == 0 || quadrant == 1)
            py = -py;
           int cx = x+w/2;
           int cy = y+h/2;
         points[i]->x = cx + w2 * px;
         points[i]->y = cy + h2 * py;
    }
}


//// The return value is the starting point of the arc
static
Point _curves_for_arc(int x, int y, int w, int h,
                      float startAngle, float sweepLength,
                      Point *curves, int *point_count)
{
    *point_count = 0;
    int w2 = w / 2;
    int w2k = w2 * PATH_KAPPA;

    int h2 = h / 2;
    int h2k = h2 * PATH_KAPPA;

    Point points[16] =
    {
      // start point
      { x + w, y + h2 },

      // 0 -> 270 degrees
      { x + w, y + h2 + h2k },
      { x + w2 + w2k, y + h },
      { x + w2, y + h },

      // 270 -> 180 degrees
      { x + w2 - w2k, y + h },
      { x, y + h2 + h2k },
      { x, y + h2 },

      // 180 -> 90 degrees
      { x, y + h2 - h2k },
      { x + w2 - w2k, y },
      { x + w2, y },

      // 90 -> 0 degrees
      { x + w2 + w2k, y },
      { x + w, y + h2 - h2k },
      { x + w, y + h2 }
    };

    if (sweepLength > 360) sweepLength = 360;
    else if (sweepLength < -360) sweepLength = -360;

    // Special case fast paths
    if (startAngle == 0) {
        if (sweepLength == 360) {
	    int i;
            for (i = 11; i >= 0; --i)
                curves[(*point_count)++] = points[i];
            return points[12];
        } else if (sweepLength == -360) {
	    int i ;
            for (i = 1; i <= 12; ++i)
                curves[(*point_count)++] = points[i];
            return points[0];
        }
    }

    int startSegment = (int)(floor(startAngle / 90));
    int endSegment = (int)(floor((startAngle + sweepLength) / 90));

    float startT = (startAngle - startSegment * 90) / 90;
    float endT = (startAngle + sweepLength - endSegment * 90) / 90;

    int delta = sweepLength > 0 ? 1 : -1;
    if (delta < 0) {
        startT = 1 - startT;
        endT = 1 - endT;
    }

    // avoid empty start segment
    if (startT == 1.0) {
        startT = 0;
        startSegment += delta;
    }

    // avoid empty end segment
    if (endT == 0) {
        endT = 1;
        endSegment -= delta;
    }

    startT = _t_for_arc_angle(startT * 90);
    endT = _t_for_arc_angle(endT * 90);

    Eina_Bool splitAtStart = !(fabs(startT) <= 0.00001f);
    Eina_Bool splitAtEnd = !(fabs(endT - 1.0) <= 0.00001f);

    const int end = endSegment + delta;

    // empty arc?
    if (startSegment == end) {
        const int quadrant = 3 - ((startSegment % 4) + 4) % 4;
        const int j = 3 * quadrant;
        return delta > 0 ? points[j + 3] : points[j];
    }


    Point startPoint, endPoint;
    _find_ellipse_coords(x, y, w, h, startAngle, sweepLength, &startPoint, &endPoint);
    int i;
    for (i = startSegment; i != end; i += delta) {
        const int quadrant = 3 - ((i % 4) + 4) % 4;
        const int j = 3 * quadrant;

        Bezier b;
        if (delta > 0)
            b = bezierFromPoints(points[j + 3], points[j + 2], points[j + 1], points[j]);
        else
            b = bezierFromPoints(points[j], points[j + 1], points[j + 2], points[j + 3]);

        // empty arc?
        if (startSegment == endSegment && (startT == endT))
            return startPoint;

        if (i == startSegment) {
            if (i == endSegment && splitAtEnd)
                b = bezierOnInterval(&b, startT, endT);
            else if (splitAtStart)
                b = bezierOnInterval(&b, startT, 1);
        } else if (i == endSegment && splitAtEnd) {
            b = bezierOnInterval(&b, 0, endT);
        }

        // push control points
        curves[(*point_count)].x = b.x2;
        curves[(*point_count)++].y = b.y2;
        curves[(*point_count)].x = b.x3;
        curves[(*point_count)++].y = b.y3;
        curves[(*point_count)].x = b.x4;
        curves[(*point_count)++].y = b.y4;
    }

    curves[*(point_count)-1] = endPoint;

    return startPoint;
}

void _arcto(Evas_VG_Node *obj, int x, int y, int width, int height, int startAngle, int sweepLength)
{
    int point_count;

    Point pts[15];
    Point curve_start = _curves_for_arc(x, y, width, height, startAngle, sweepLength, pts, &point_count);
    int cx = x + (width)/2;
    int cy = y + (height)/2;
    int i;

    eo_do(obj,
          efl_gfx_shape_append_move_to(cx, cy);

          efl_gfx_shape_append_line_to(curve_start.x, curve_start.y);
          for (i = 0; i < point_count; i += 3)
            {
               efl_gfx_shape_append_cubic_to(pts[i+2].x, pts[i+2].y,
                                             pts[i].x, pts[i].y,
                                             pts[i+1].x, pts[i+1].y);
            }
          efl_gfx_shape_append_close());
}

void _rect_add(Evas_VG_Node *obj, int x, int y, int w, int h)
{
   eo_do(obj,
         efl_gfx_shape_append_move_to(x, y);
         efl_gfx_shape_append_line_to(x + w, y);
         efl_gfx_shape_append_line_to(x + w, y +h);
         efl_gfx_shape_append_line_to(x, y +h);
         efl_gfx_shape_append_close());
}


struct example_data
{
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
   Evas_Object *vg;
};

static struct example_data d;

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void /* adjust canvas' contents on resizes */
_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(d.bg, w, h);
   evas_object_resize(d.vg, w, h);
}

static void
vector_set(int x, int y, int w, int h)
{
   int vg_w = w, vg_h = h;

   //Create VG Object

   Evas_Object *tmp = evas_object_rectangle_add(d.evas);
   evas_object_resize(tmp, vg_w, vg_h);
   evas_object_color_set(tmp, 100, 100, 50, 100);
   evas_object_move(tmp, x,y);
   evas_object_show(tmp);

   d.vg = evas_object_vg_add(d.evas);
   evas_object_resize(d.vg, vg_w, vg_h);
   evas_object_move(d.vg, x,y);
   evas_object_show(d.vg);
   evas_object_clip_set(d.vg, tmp);

     // Applying map on the evas_object_vg
//   Evas_Map *m = evas_map_new(4);
//   evas_map_smooth_set(m, EINA_TRUE);
//   evas_map_util_points_populate_from_object_full(m, d.vg, 0);
//   evas_map_util_rotate(m, 10, 0,0);
//   evas_object_map_enable_set(d.vg, EINA_TRUE);
//   evas_object_map_set(d.vg, m);

   // apply some transformation
   double radian = 30.0 * 2 * 3.141 / 360.0;
   Eina_Matrix3 matrix;
   eina_matrix3_rotate(&matrix, radian);

   Evas_VG_Node *root = evas_object_vg_root_node_get(d.vg);
   //eo_do(root, evas_vg_node_transformation_set(&matrix));

   Evas_VG_Node *bg = eo_add(EVAS_VG_SHAPE_CLASS, root);
   _rect_add(bg, 0, 0 , vg_w, vg_h);
   eo_do(bg,
         evas_vg_node_origin_set(0, 0),
         efl_gfx_shape_stroke_width_set(1.0),
         efl_gfx_color_set(128, 128, 128, 80));

   Evas_VG_Node *shape = eo_add(EVAS_VG_SHAPE_CLASS, root);
   Evas_VG_Node *rgradient = eo_add(EVAS_VG_GRADIENT_RADIAL_CLASS, root);
   Evas_VG_Node *lgradient = eo_add(EVAS_VG_GRADIENT_LINEAR_CLASS, root);

   _arcto(shape, 0, 0, 100, 100, 25, 330);

   Efl_Gfx_Gradient_Stop stops[3];
   stops[0].r = 255;
   stops[0].g = 0;
   stops[0].b = 0;
   stops[0].a = 255;
   stops[0].offset = 0;
   stops[1].r = 0;
   stops[1].g = 255;
   stops[1].b = 0;
   stops[1].a = 255;
   stops[1].offset = 0.5;
   stops[2].r = 0;
   stops[2].g = 0;
   stops[2].b = 255;
   stops[2].a = 255;
   stops[2].offset = 1;

   eo_do(rgradient,
         evas_vg_node_origin_set(10,10),
         efl_gfx_gradient_stop_set(stops, 3),
         efl_gfx_gradient_spread_set(EFL_GFX_GRADIENT_SPREAD_REFLECT),
         efl_gfx_gradient_stop_set(stops, 3),
         efl_gfx_gradient_radial_center_set(30, 30),
         efl_gfx_gradient_radial_radius_set(80));

   eo_do(lgradient,
         evas_vg_node_origin_set(10,10),
         efl_gfx_gradient_stop_set(stops, 3),
         efl_gfx_gradient_spread_set(EFL_GFX_GRADIENT_SPREAD_REFLECT),
         efl_gfx_gradient_stop_set(stops, 3),
         efl_gfx_gradient_linear_start_set(10,10),
         efl_gfx_gradient_linear_end_set(50,50));

   eo_do(shape,
         evas_vg_node_origin_set(10, 10),
         evas_vg_shape_fill_set(rgradient),
         efl_gfx_shape_stroke_scale_set(2.0),
         efl_gfx_shape_stroke_width_set(1.0),
         efl_gfx_color_set(0, 0, 255, 255),
         efl_gfx_shape_stroke_color_set(0, 0, 255, 128));

   Evas_VG_Node *rect = eo_add(EVAS_VG_SHAPE_CLASS, root);
   _rect_add(rect, 0, 0, 100, 100);
   eo_do(rect,
         evas_vg_node_origin_set(100, 100),
         evas_vg_shape_fill_set(lgradient),
         efl_gfx_shape_stroke_width_set(2.0),
         efl_gfx_shape_stroke_join_set(EFL_GFX_JOIN_ROUND),
         efl_gfx_shape_stroke_color_set(255, 255, 255, 255));


   Evas_VG_Node *rect1 = eo_add(EVAS_VG_SHAPE_CLASS, root);
   _rect_add(rect1, 0, 0, 70, 70);
   eo_do(rect1,
         evas_vg_node_origin_set(50, 70),
         efl_gfx_shape_stroke_scale_set(2),
         efl_gfx_shape_stroke_width_set(8.0),
         efl_gfx_shape_stroke_join_set(EFL_GFX_JOIN_ROUND),
         efl_gfx_shape_stroke_color_set(0, 100, 80, 100));

   Evas_VG_Node *circle = eo_add(EVAS_VG_SHAPE_CLASS, root);
   _arcto(circle, 0, 0, 250, 100, 30, 300);
   eo_do(circle,
         evas_vg_shape_fill_set(lgradient),
         //evas_vg_node_transformation_set(&matrix),
         evas_vg_node_origin_set(50,50),
         efl_gfx_color_set(50, 0, 0, 50));

   // Foreground
   Evas_VG_Node *fg = eo_add(EVAS_VG_SHAPE_CLASS, root);
   _rect_add(fg, 0, 0, vg_w, vg_h);
   eo_do(fg,
         evas_vg_node_origin_set(0, 0),
         efl_gfx_shape_stroke_width_set(5.0),
         efl_gfx_shape_stroke_join_set(EFL_GFX_JOIN_ROUND),
         efl_gfx_shape_stroke_color_set(70, 70, 0, 70));
}

int
main(void)
{
   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   d.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!d.ee)
     goto error;

   ecore_evas_callback_delete_request_set(d.ee, _on_delete);
   ecore_evas_callback_resize_set(d.ee, _canvas_resize_cb);
   ecore_evas_show(d.ee);

   d.evas = ecore_evas_get(d.ee);

   d.bg = evas_object_rectangle_add(d.evas);
   evas_object_color_set(d.bg, 70, 70, 70, 255); /* white bg */
   evas_object_show(d.bg);

   _canvas_resize_cb(d.ee);

   vector_set(50, 50, 300 ,300);
   //vector_set(30, 90, 300 ,300);

   ecore_main_loop_begin();
   ecore_evas_shutdown();
   return 0;

error:
   ecore_evas_shutdown();
   return -1;
}


/**
 * Example of Evas' vector graphics object and API.
 *
 * Vector graphics describe an image with distinct shapes - lines,
 * rectangles, circles, curves; this distinguishes it from raster
 * graphics where the image is comprised of a grid of colored pixels.
 *
 * Just as there are many different file and data formats for raster
 * graphics, there are also many data formats for vector graphics.  A
 * well-known one is Scalable Vector Graphics (SVG).  This example uses
 * SVG-like path definitions to create a shape in evas_vg and then
 * modify and animate it using the evas_vg API; there is a second test
 * mode to demonstrate a timeline animation of morphing between two
 * shapes using evas_vg's interpolation API.
 *
 * Internally, evas_vg uses EFL's Ector component to interface with the
 * underlying graphics rendering subsystems in a two step process.
 * First is a 'prepare' phase where CPU intensive operations are
 * performed (ideally spawned on a separate thread), second is the
 * 'render' phase where memory intensive work is done.  The reason for
 * this split is to enable pushing work to the GPU in order to attain
 * higher performance.  Ector currently supports use of Cairo, GL, and a
 * software backend renderer.
 *
 * You'll need at least one engine built for this example (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * gcc -o evas_vg_simple evas-vg-simple.c `pkg-config --libs --cflags evas ecore ecore-evas eina ector eo efl` -lm
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#endif

#define WIDTH 600
#define HEIGHT 600

#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
#include <Efl.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Eina.h>

struct example_data
{
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
   Evas_Object *vg;
   Eina_List   *shape_list;

};

static struct example_data d;
Ecore_Animator *animator;

/* These data strings follows an SVG-like convention for describing the
 * nodes of a path.  'M x,y' indicates a move-to operation that sets
 * where to start drawing.  'L x,y' draws a straight line that ends at
 * the given point.  'C x1,y1 x2,y2 x,y' defines a Bezier curve's two
 * control points and the x,y coordinate where the curve ends.  'Z' marks
 * the end of the path.
 */
static const char *batman = "M 256,213 C 245,181 206,187 234,262 147,181 169,71.2 233,18 220,56 235,81 283,88 285,78.7 286,69.3 288,60 289,61.3 290,62.7 291,64 291,64 297,63 300,63 303,63 309,64 309,64 310,62.7 311,61.3 312,60 314,69.3 315,78.7 317,88 365,82 380,56 367,18 431,71 453,181 366,262 394,187 356,181 344,213 328,185 309,184 300,284 291,184 272,185 256,213 Z";
static const char *morph1[2] = {"M 0,0 L 0,0 L 100,0 L 100,0 L 100,100 L 100,100 L 0,100 L 0,100 L 0,0",
                                "M 0,0 L 50,-80 L 100,0 L 180,50 L 100,100 L 50,180 L 0,100 L -80,50 L 0,0"};

static void _main_menu();
static void _main_menu_key_handle(void *data, Evas *evas, Evas_Object *o, void *einfo);

static const char *main_menu = \
  "Main Menu:\n"
  "\t1    - Basic Shape test\n"
  "\t2    - Interpolation\n"
  "\te    - Exit\n"
  "\th    - Print help\n";

static const char *basic_shape_menu = \
  "Basic Shape Menu:\n"
  "\tup   - Increase Stroke Width by 0.5\n"
  "\tdown - Decrease Stroke Width by 0.5\n"
  "\tr    - +10 degree rotation\n"
  "\tR    - -10 degree rotation\n"
  "\ts    - +(.1, .1) scale\n"
  "\tS    - -(.1, .1) scale\n"
  "\tt    - +(10,10) translation\n"
  "\tT    - +(10,10) translation\n"
  "\td    - Reset path data\n"
  "\te    - Exit\n"
  "\tb    - Back to Main Menu\n"
  "\th    - Print help\n";

static const char *interpolation_menu = \
  "Interpolation Menu:\n"
  "\te    - Exit\n"
  "\tb    - Back to Main Menu\n"
  "\th    - print help\n";

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   if (animator) ecore_animator_del(animator);
   ecore_main_loop_quit();
}

static void
_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(d.bg, w, h);
   evas_object_resize(d.vg, w, h);
}

static void
reset_test()
{
  if(d.vg) evas_object_del(d.vg);
  d.shape_list = eina_list_free(d.shape_list);

  d.vg = evas_object_vg_add(d.evas);
  evas_object_show(d.vg);
  evas_object_focus_set(d.vg, 1);
  _canvas_resize_cb(d.ee);
}



// 2. Basic shape  Test Case START

/* Applies a matrix transformation to a shape.  If there is already a
 * matrix transformation applied, the new tranformation is matrix
 * multiplied with the existing one so both transformations are used.
 */
static void
_added_transformation(Efl_VG *shape, Eina_Matrix3 *m)
{
   Eina_Matrix3 new_m;
   const Eina_Matrix3 *old_m;

   old_m = evas_vg_node_transformation_get(shape);
   if (old_m)
     {
        eina_matrix3_compose(m, old_m, &new_m);
        evas_vg_node_transformation_set(shape, &new_m);
     }
   else
     {
        evas_vg_node_transformation_set(shape, m);
     }
}

/*
 * Applies various modifications to the canvas objects as directed by
 * the user.
 */
static void
_basic_shape_key_handle(void        *data EINA_UNUSED,
                      Evas        *evas EINA_UNUSED,
                      Evas_Object *o EINA_UNUSED,
                      void        *einfo)
{
   Evas_Event_Key_Down *ev = einfo;
   Efl_VG *shape;
   Eina_List *l;
   Eina_Matrix3 m;
   double stroke_w;

   if (strcmp(ev->key, "h") == 0)
     {
        /* h - Print help */
        puts(basic_shape_menu);
        return;
     }
   if (strcmp(ev->key, "e") == 0)
     {
        /* e    - Exit */
        _on_delete(d.ee);
        return;
     }

   if (strcmp(ev->key, "b") == 0)
     {
        /* b - Back to Main Menu */
        _main_menu();
        return;
     }
   if (strcmp(ev->key, "Up") == 0)
     {
        /* up - Increase Stroke Width by 0.5 */
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          stroke_w = evas_vg_shape_stroke_width_get(shape);
          evas_vg_shape_stroke_width_set(shape, stroke_w + 0.5);
        }
        return;
     }
   if (strcmp(ev->key, "Down") == 0)
     {
        /* down - Decrease Stroke Width by 0.5 */
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          stroke_w = evas_vg_shape_stroke_width_get(shape);
          if (stroke_w <= 0.5) stroke_w = 1;
          evas_vg_shape_stroke_width_set(shape, stroke_w - 0.5);
        }
        return;
     }
   if (strcmp(ev->key, "r") == 0)
     {
        /* r - Rotate the shapes +10 degrees */
        eina_matrix3_identity(&m);
        eina_matrix3_rotate(&m, 10.0 * 2 * 3.141 / 360.0);
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          _added_transformation(shape, &m);
        }
        return;
     }
   if (strcmp(ev->key, "R") == 0)
     {
        /* R - Rotate the shapes -10 degrees */
        eina_matrix3_identity(&m);
        eina_matrix3_rotate(&m, -10.0 * 2 * 3.141 / 360.0);
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          _added_transformation(shape, &m);
        }
        return;
     }
   if (strcmp(ev->key, "s") == 0)
     {
        /* s - Scale the shapes +(.1, .1) */
        eina_matrix3_identity(&m);
        eina_matrix3_scale(&m, 1.1, 1.1);
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          _added_transformation(shape, &m);
        }
        return;
     }
   if (strcmp(ev->key, "S") == 0)
     {
        /* S - Scale the shapes -(.1, .1) */
        eina_matrix3_identity(&m);
        eina_matrix3_scale(&m, .9, .9);
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          _added_transformation(shape, &m);
        }
        return;
     }
   if (strcmp(ev->key, "t") == 0)
     {
        /* t - Translate the shapes +(10,10) */
        eina_matrix3_identity(&m);
        eina_matrix3_translate(&m, 10, 10);
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          _added_transformation(shape, &m);
        }
        return;
     }
   if (strcmp(ev->key, "T") == 0)
     {
        /* T - Translate the shapes +(10,10) */
        eina_matrix3_identity(&m);
        eina_matrix3_translate(&m, -10, -10);
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          _added_transformation(shape, &m);
        }
        return;
     }
   if (strcmp(ev->key, "d") == 0)
     {
        /* d - Reset path data */
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          evas_vg_shape_reset(shape);
        }
        return;
     }
}

/* Create several different geometric objects - a line, rectangle,
 * circle, and arc, each with distinct style and placement.
 */
static void
_1_basic_shape_test()
{
  Efl_VG *container, *shape, *new_shape;

  reset_test();
  evas_object_event_callback_add(d.vg, EVAS_CALLBACK_KEY_DOWN, _basic_shape_key_handle, NULL);
  puts(basic_shape_menu);

  container = evas_vg_container_add(d.vg);

  // Line
  shape = evas_vg_shape_add(container);
  evas_vg_shape_append_move_to(shape, 0, 0);
  evas_vg_shape_append_line_to(shape, 100, 0);
  evas_vg_shape_stroke_color_set(shape, 255, 0, 0, 255);
  evas_vg_shape_stroke_width_set(shape, 5);
  evas_vg_node_origin_set(shape, 50, 50);
  evas_vg_shape_stroke_cap_set(shape, EFL_GFX_CAP_ROUND);
  d.shape_list = eina_list_append(d.shape_list, shape);

  new_shape = evas_vg_shape_add(container);
  evas_vg_shape_dup(new_shape, shape);
  evas_vg_shape_stroke_color_set(new_shape, 0, 100, 0, 100);
  evas_vg_shape_append_line_to(new_shape, 100, 50);
  evas_vg_node_origin_set(new_shape, 200, 50);
  evas_vg_shape_stroke_cap_set(new_shape, EFL_GFX_CAP_SQUARE);
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  new_shape = evas_vg_shape_add(container);
  evas_vg_shape_dup(new_shape, shape);
  evas_vg_shape_stroke_color_set(new_shape, 0, 0, 255, 255);
  evas_vg_shape_append_line_to(new_shape, 50, 50);
  evas_vg_node_origin_set(new_shape, 350, 50);
  evas_vg_shape_stroke_cap_set(new_shape, EFL_GFX_CAP_ROUND);
  evas_vg_shape_stroke_join_set(new_shape, EFL_GFX_JOIN_ROUND);
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  // Rect
  shape = evas_vg_shape_add(container);
  evas_vg_shape_append_rect(shape, 0, 0, 100 , 100, 10, 10);
  evas_vg_shape_stroke_color_set(shape, 255, 0, 0, 255);
  evas_vg_shape_stroke_width_set(shape, 5);
  evas_vg_node_origin_set(shape, 50, 150);
  d.shape_list = eina_list_append(d.shape_list, shape);

  new_shape = evas_vg_shape_add(container);
  evas_vg_shape_dup(new_shape, shape);
  evas_vg_shape_stroke_color_set(new_shape, 0, 0, 0, 0);
  evas_vg_node_color_set(new_shape, 0, 0, 100, 100);
  evas_vg_node_origin_set(new_shape, 200, 150);
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  new_shape = evas_vg_shape_add(container);
  evas_vg_shape_dup(new_shape, shape);
  evas_vg_shape_stroke_color_set(new_shape, 100, 0, 0, 100);
  evas_vg_node_color_set(new_shape, 0, 100, 100, 100);
  evas_vg_node_origin_set(new_shape, 350, 150);
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  // Circle
  shape = evas_vg_shape_add(container);
  evas_vg_shape_append_circle(shape, 50, 50, 50);
  evas_vg_shape_stroke_color_set(shape, 255, 0, 0, 255);
  evas_vg_shape_stroke_width_set(shape, 5);
  evas_vg_node_origin_set(shape, 50, 300);
  d.shape_list = eina_list_append(d.shape_list, shape);

  new_shape = evas_vg_shape_add(container);
  evas_vg_shape_dup(new_shape, shape);
  evas_vg_shape_stroke_color_set(new_shape, 0, 0, 0, 0);
  evas_vg_node_color_set(new_shape, 0, 0, 255, 255);
  evas_vg_node_origin_set(new_shape, 200, 300);
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  new_shape = evas_vg_shape_add(container);
  evas_vg_shape_dup(new_shape, shape);
  evas_vg_shape_stroke_color_set(new_shape, 150, 0, 0, 150);
  evas_vg_node_color_set(new_shape, 0, 0, 200, 200);
  evas_vg_node_origin_set(new_shape, 350, 300);
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  // Arc
  shape = evas_vg_shape_add(container);
  evas_vg_shape_append_arc(shape, 0, 0, 100, 100, 45, -200);
  evas_vg_shape_append_line_to(shape, 50, 50);
  evas_vg_shape_stroke_cap_set(shape, EFL_GFX_CAP_ROUND);
  evas_vg_shape_stroke_color_set(shape, 255, 0, 0, 255);
  evas_vg_shape_stroke_width_set(shape, 5);
  evas_vg_node_origin_set(shape, 50, 450);
  d.shape_list = eina_list_append(d.shape_list, shape);

  new_shape = evas_vg_shape_add(container);
  evas_vg_shape_append_arc(new_shape, 0, 0, 100, 100, 90, 200);
  evas_vg_shape_stroke_color_set(new_shape, 0, 0, 0, 0);
  evas_vg_node_color_set(new_shape, 0, 0, 200, 200);
  evas_vg_node_origin_set(new_shape, 200, 450);
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  new_shape = evas_vg_shape_add(container);
  evas_vg_shape_append_arc(new_shape, 0, 0, 100, 100, 90, 200);
  evas_vg_shape_append_line_to(new_shape, 50, 50);
  evas_vg_shape_stroke_color_set(new_shape, 0, 0, 0, 0);
  evas_vg_node_color_set(new_shape, 0, 0, 200, 200);
  evas_vg_node_origin_set(new_shape, 350, 450);
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  evas_object_vg_root_node_set(d.vg, container);
}

// 2. Basic shape  Test Case END

// 2. Interpolation Test Case START

static void
_interpolation_key_handle(void  *data EINA_UNUSED,
                          Evas        *evas EINA_UNUSED,
                          Evas_Object *o EINA_UNUSED,
                          void        *einfo)
{
   Evas_Event_Key_Down *ev = einfo;

   if (strcmp(ev->key, "h") == 0)
     {
        /* h - Print help */
        puts(basic_shape_menu);
        return;
     }
   if (strcmp(ev->key, "e") == 0)
     {
        /* e - Exit */
        _on_delete(d.ee);
        return;
     }

   if (strcmp(ev->key, "b") == 0)
     {
        /* b - Back to main menu */
        _main_menu();
        return;
     }
}

static int anim_index = 0;
static Eina_Bool
_interpolation_keyframe(void *data EINA_UNUSED, double pos)
{
  int next = (anim_index == 1) ? 0 : 1;

  evas_vg_shape_reset(eina_list_nth(d.shape_list, 2));
  evas_vg_shape_interpolate(eina_list_nth(d.shape_list, 2),
                             eina_list_nth(d.shape_list, anim_index),
                             eina_list_nth(d.shape_list, next),
                             ecore_animator_pos_map(pos, ECORE_POS_MAP_SINUSOIDAL, 0.0, 0.0));

   if (pos == 1.0)
     {
        anim_index = (anim_index == 1) ? 0 : 1;
        animator = ecore_animator_timeline_add(1, _interpolation_keyframe, NULL);
     }
   return EINA_TRUE;
}

static void
_2_interpolation_test()
{
  anim_index = 0;

  Efl_VG *shape;

  reset_test();

  evas_object_event_callback_add(d.vg, EVAS_CALLBACK_KEY_DOWN, _interpolation_key_handle, NULL);
  animator = ecore_animator_timeline_add(1, _interpolation_keyframe, NULL);
  puts(interpolation_menu);

  shape = evas_vg_shape_add(d.vg);
  evas_vg_shape_append_svg_path(shape, morph1[0]);
  evas_vg_shape_stroke_color_set(shape, 255, 0, 0, 255);
  evas_vg_shape_stroke_width_set(shape, 5);
  evas_vg_node_origin_set(shape, 100, 100);
  d.shape_list = eina_list_append(d.shape_list, shape);

  shape = evas_vg_shape_add(d.vg);
  evas_vg_shape_append_svg_path(shape, morph1[1]);
  evas_vg_shape_stroke_color_set(shape, 0, 0, 255, 255);
  evas_vg_shape_stroke_width_set(shape, 10);
  evas_vg_node_origin_set(shape, 150, 150);
  d.shape_list = eina_list_append(d.shape_list, shape);

  shape = evas_vg_shape_add(d.vg);
  evas_vg_node_origin_set(shape, 150, 150);
  d.shape_list = eina_list_append(d.shape_list, shape);

  evas_object_vg_root_node_set(d.vg, shape);
}

// 2. Interpolation Test Case END

// Main Menu START

static void
_main_menu()
{
  Efl_VG *shape;
  if (animator) ecore_animator_del(animator);
  animator = NULL;
  if(d.vg) evas_object_del(d.vg);
  if(d.bg) evas_object_del(d.bg);
   d.bg = evas_object_rectangle_add(d.evas);
   evas_object_color_set(d.bg, 255, 255, 255, 255);
   evas_object_focus_set(d.bg, 1);
   evas_object_show(d.bg);
   evas_object_event_callback_add(d.bg, EVAS_CALLBACK_KEY_DOWN, _main_menu_key_handle, NULL);

   // create the initial screen
   d.vg = evas_object_vg_add(d.evas);
   evas_object_show(d.vg);

   shape = evas_vg_shape_add(d.vg);
   evas_vg_shape_append_svg_path(shape, batman);
   evas_vg_node_color_set(shape, 10, 0, 0, 10);
   evas_vg_node_origin_set(shape, 0, 100);

   evas_object_vg_root_node_set(d.vg, shape);

   _canvas_resize_cb(d.ee);
   puts(main_menu);
}

static void
_main_menu_key_handle(void        *data EINA_UNUSED,
                      Evas        *evas EINA_UNUSED,
                      Evas_Object *o EINA_UNUSED,
                      void        *einfo)
{
   Evas_Event_Key_Down *ev = einfo;

   if (strcmp(ev->key, "h") == 0)
     {
        /* h - Help menu */
        puts(main_menu);
        return;
     }
   if (strcmp(ev->key, "e") == 0)
     {
        /* e - Exit */
        _on_delete(d.ee);
        return;
     }
   if (strcmp(ev->key, "1") == 0)
     {
        /* 1 - Basic Shape test */
        _1_basic_shape_test();
        return;
     }
   if (strcmp(ev->key, "2") == 0)
     {
        /* 2 - Interpolation */
        _2_interpolation_test();
        return;
     }
}

// Main Menu END

int
main(void)
{
   if (!ecore_evas_init())
     return EXIT_FAILURE;

   d.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!d.ee)
     goto error;

   ecore_evas_callback_delete_request_set(d.ee, _on_delete);
   ecore_evas_callback_resize_set(d.ee, _canvas_resize_cb);
   ecore_evas_show(d.ee);

   d.evas = ecore_evas_get(d.ee);

   _main_menu();

   ecore_main_loop_begin();
   ecore_evas_shutdown();
   return 0;

error:
   ecore_evas_shutdown();
   return -1;
}

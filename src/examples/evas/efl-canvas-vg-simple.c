/**
 * Example of Evas' vector graphics object and API.
 *
 * @verbatim
 * gcc -o efl-canvas-vg-simple efl-canvas-vg-simple.c `pkg-config --libs --cflags evas ecore ecore-evas eina ector eo efl` -lm
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#endif

#define WIDTH 600
#define HEIGHT 800

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
   Efl_Canvas_Rectangle *bg;
   Efl_VG      *vg;
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
static const unsigned int batman_commandCnt = 17;
static const unsigned int batman_pointCnt = 86;
static const char *morph1[2] = {"M 0,0 L 0,0 L 100,0 L 100,0 L 100,100 L 100,100 L 0,100 L 0,100 L 0,0",
                                "M 0,0 L 50,-80 L 100,0 L 180,50 L 100,100 L 50,180 L 0,100 L -80,50 L 0,0"};

static void _main_menu();
static void _main_menu_key_handle(void *data EINA_UNUSED, const Efl_Event *ev);

static const char *main_menu = \
  "Main Menu:\n"
  "\t1    - Basic Shape test\n"
  "\t2    - Interpolation\n"
  "\t3    - Gradient\n"
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

static const char *gradient_menu = \
  "Gradient Menu:\n"
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

   efl_gfx_entity_geometry_set(d.bg, EINA_RECT(0, 0, w, h));
   efl_gfx_entity_geometry_set(d.vg, EINA_RECT(0, 0, w, h));
}

static void
reset_test()
{
  if(d.vg) efl_del(d.vg);
  d.shape_list = eina_list_free(d.shape_list);

  d.vg = efl_add(EFL_CANVAS_VG_OBJECT_CLASS, d.evas,
     efl_canvas_object_key_focus_set(efl_added, EINA_TRUE),
     efl_gfx_entity_visible_set(efl_added, EINA_TRUE));

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

   old_m = efl_canvas_vg_node_transformation_get(shape);
   if (old_m)
     {
        eina_matrix3_compose(m, old_m, &new_m);
        efl_canvas_vg_node_transformation_set(shape, &new_m);
     }
   else
     {
        efl_canvas_vg_node_transformation_set(shape, m);
     }
}

/*
 * Applies various modifications to the canvas objects as directed by
 * the user.
 */
static void
_basic_shape_key_handle(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_VG *shape;
   Eina_List *l;
   Eina_Matrix3 m;
   double stroke_w;

   if (!strcmp(efl_input_key_sym_get(ev->info) , "h"))
     {
        /* h - Print help */
        puts(basic_shape_menu);
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "e"))
     {
        /* e    - Exit */
        _on_delete(d.ee);
     }

   else if (!strcmp(efl_input_key_sym_get(ev->info) , "b"))
     {
        /* b - Back to Main Menu */
        _main_menu();
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "Up"))
     {
        /* up - Increase Stroke Width by 0.5 */
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          stroke_w = efl_gfx_shape_stroke_width_get(shape);
          efl_gfx_shape_stroke_width_set(shape, stroke_w + 0.5);
          efl_gfx_path_commit(shape);
        }
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "Down"))
     {
        /* down - Decrease Stroke Width by 0.5 */
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          stroke_w = efl_gfx_shape_stroke_width_get(shape);
          if (stroke_w <= 0.5) stroke_w = 1;
          efl_gfx_shape_stroke_width_set(shape, stroke_w - 0.5);
          efl_gfx_path_commit(shape);
        }
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "r"))
     {
        /* r - Rotate the shapes +10 degrees */
        eina_matrix3_identity(&m);
        eina_matrix3_rotate(&m, 10.0 * 2 * 3.141 / 360.0);
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          _added_transformation(shape, &m);
        }
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "R"))
     {
        /* R - Rotate the shapes -10 degrees */
        eina_matrix3_identity(&m);
        eina_matrix3_rotate(&m, -10.0 * 2 * 3.141 / 360.0);
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          _added_transformation(shape, &m);
        }
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "s"))
     {
        /* s - Scale the shapes +(.1, .1) */
        eina_matrix3_identity(&m);
        eina_matrix3_scale(&m, 1.1, 1.1);
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          _added_transformation(shape, &m);
        }
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "S"))
     {
        /* S - Scale the shapes -(.1, .1) */
        eina_matrix3_identity(&m);
        eina_matrix3_scale(&m, .9, .9);
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          _added_transformation(shape, &m);
        }
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "t"))
     {
        /* t - Translate the shapes +(10,10) */
        eina_matrix3_identity(&m);
        eina_matrix3_translate(&m, 10, 10);
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          _added_transformation(shape, &m);
        }
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "T"))
     {
        /* T - Translate the shapes +(10,10) */
        eina_matrix3_identity(&m);
        eina_matrix3_translate(&m, -10, -10);
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          _added_transformation(shape, &m);
        }
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "d"))
     {
        /* d - Reset path data */
        EINA_LIST_FOREACH(d.shape_list, l, shape)
        {
          efl_gfx_path_reset(shape);
        }
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
  efl_event_callback_add(d.vg, EFL_EVENT_KEY_DOWN, _basic_shape_key_handle, NULL);
  puts(basic_shape_menu);

  container = efl_add(EFL_CANVAS_VG_CONTAINER_CLASS, d.vg);

  // Line
  shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_path_append_move_to(efl_added, 0, 0),  // Move strating point path
    efl_gfx_path_append_line_to(efl_added, 100, 0), // Add line form move point
    efl_gfx_shape_stroke_color_set(efl_added, 255, 0, 0, 255),
    efl_gfx_shape_stroke_width_set(efl_added, 5),
    efl_canvas_vg_node_origin_set(efl_added, 50, 50),
    efl_gfx_shape_stroke_cap_set(efl_added, EFL_GFX_CAP_ROUND)); // Set stroke cap
  d.shape_list = eina_list_append(d.shape_list, shape);

  new_shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_path_copy_from(efl_added, shape), // Path duplicate.
    efl_gfx_shape_stroke_color_set(efl_added, 0, 100, 0, 100),
    efl_gfx_path_append_line_to(efl_added, 100, 50),
    efl_canvas_vg_node_origin_set(efl_added, 200, 50),
    efl_gfx_shape_stroke_cap_set(efl_added, EFL_GFX_CAP_SQUARE));
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  new_shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_path_copy_from(efl_added, shape),
    efl_gfx_shape_stroke_color_set(efl_added, 0, 0, 255, 255),
    efl_gfx_path_append_line_to(efl_added, 50, 50),
    efl_canvas_vg_node_origin_set(efl_added, 350, 50),
    efl_gfx_shape_stroke_cap_set(efl_added, EFL_GFX_CAP_ROUND),
    efl_gfx_shape_stroke_join_set(efl_added, EFL_GFX_JOIN_ROUND)); // Set stroke join
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  // Rect
  shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_path_append_rect(efl_added, 0, 0, 100 , 100, 10, 10), // Draw Rectangle
    efl_gfx_shape_stroke_color_set(efl_added, 255, 0, 0, 255),
    efl_gfx_shape_stroke_width_set(efl_added, 5),
    efl_canvas_vg_node_origin_set(efl_added, 50, 150));
  d.shape_list = eina_list_append(d.shape_list, shape);

  new_shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_path_copy_from(efl_added, shape),
    efl_gfx_shape_stroke_color_set(efl_added, 0, 0, 0, 0),
    efl_gfx_color_set(efl_added, 0, 0, 100, 100), // Set fill color.
    efl_canvas_vg_node_origin_set(efl_added, 200, 150));
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  new_shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_path_copy_from(efl_added, shape),
    efl_gfx_shape_stroke_color_set(efl_added, 100, 0, 0, 100),
    efl_gfx_color_set(efl_added, 0, 100, 100, 100),
    efl_canvas_vg_node_origin_set(efl_added, 350, 150));
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  // Circle
  shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_path_append_circle(efl_added, 50, 50, 50),  //Draw Circle
    efl_gfx_shape_stroke_color_set(efl_added, 255, 0, 0, 255),
    efl_gfx_shape_stroke_width_set(efl_added, 5),
    efl_canvas_vg_node_origin_set(efl_added, 50, 300));
  d.shape_list = eina_list_append(d.shape_list, shape);

  new_shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_path_copy_from(efl_added, shape),
    efl_gfx_shape_stroke_color_set(efl_added, 0, 0, 0, 0),
    efl_gfx_color_set(efl_added, 0, 0, 255, 255),
    efl_canvas_vg_node_origin_set(efl_added, 200, 300));
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  new_shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_path_copy_from(efl_added, shape),
    efl_gfx_shape_stroke_color_set(efl_added, 150, 0, 0, 150),
    efl_gfx_color_set(efl_added, 0, 0, 200, 200),
    efl_canvas_vg_node_origin_set(efl_added, 350, 300));
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  // Arc
  shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_path_append_arc(efl_added, 0, 0, 100, 100, 45, -200), // Draw Arc
    efl_gfx_path_append_line_to(efl_added, 50, 50),
    efl_gfx_shape_stroke_cap_set(efl_added, EFL_GFX_CAP_ROUND),
    efl_gfx_shape_stroke_color_set(efl_added, 255, 0, 0, 255),
    efl_gfx_shape_stroke_width_set(efl_added, 5),
    efl_canvas_vg_node_origin_set(efl_added, 50, 450));
  d.shape_list = eina_list_append(d.shape_list, shape);

  new_shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_path_append_arc(efl_added, 0, 0, 100, 100, 90, 200),
    efl_gfx_shape_stroke_color_set(efl_added, 0, 0, 0, 0),
    efl_gfx_color_set(efl_added, 0, 0, 200, 200),
    efl_canvas_vg_node_origin_set(efl_added, 200, 450));
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  new_shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_path_append_arc(efl_added, 0, 0, 100, 100, 90, 200),
    efl_gfx_path_append_line_to(efl_added, 50, 50),
    efl_gfx_shape_stroke_color_set(efl_added, 0, 0, 0, 0),
    efl_gfx_color_set(efl_added, 0, 0, 200, 200),
    efl_canvas_vg_node_origin_set(efl_added, 350, 450));
  d.shape_list = eina_list_append(d.shape_list, new_shape);

  efl_canvas_vg_object_root_node_set(d.vg, container);
}

// 2. Basic shape  Test Case END

// 2. Interpolation Test Case START

static void
_interpolation_key_handle(void *data EINA_UNUSED, const Efl_Event *ev)
{
   if (!strcmp(efl_input_key_sym_get(ev->info) , "h"))
     {
        /* h - Print help */
        puts(basic_shape_menu);
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "e"))
     {
        /* e - Exit */
        _on_delete(d.ee);
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "b"))
     {
        /* b - Back to main menu */
        _main_menu();
     }
}

static int anim_index = 0;
static Eina_Bool
_interpolation_keyframe(void *data EINA_UNUSED, double pos)
{
  int next = (anim_index == 1) ? 0 : 1;

  // Reset Base Object's path
  efl_gfx_path_reset(eina_list_nth(d.shape_list, 2));
  // Set Path that interpolated from 'from' Object and 'to' Object.
  efl_gfx_path_interpolate(eina_list_nth(d.shape_list, 2),
                           eina_list_nth(d.shape_list, anim_index),
                           eina_list_nth(d.shape_list, next),
                           ecore_animator_pos_map(pos, ECORE_POS_MAP_SINUSOIDAL, 0.0, 0.0));

   if (EINA_DBL_EQ(pos, 1.0))
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

  Efl_VG *shape, *container;

  reset_test();

  efl_event_callback_add(d.vg, EFL_EVENT_KEY_DOWN, _interpolation_key_handle, NULL);
  animator = ecore_animator_timeline_add(1, _interpolation_keyframe, NULL);
  puts(interpolation_menu);

  // Make Efl.Canvas.Vg.Container Object
  container = efl_add(EFL_CANVAS_VG_CONTAINER_CLASS, d.vg);
  // Set the root node of Efl.Canvas.Vg.Object
  // The Efl.Canvas.Vg.Object searches and renders
  // Efl.Canvas.Vg.Node(Shape, Container, etc...) from set root node.
  efl_canvas_vg_object_root_node_set(d.vg, container);

  //Interpolation 'from' and 'to' Object
  shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, d.vg,
    efl_gfx_path_append_svg_path(efl_added, morph1[0]),           // Set SVG path
    efl_gfx_shape_stroke_color_set(efl_added, 255, 0, 0, 255),
    efl_gfx_shape_stroke_width_set(efl_added, 5),
    efl_canvas_vg_node_origin_set(efl_added, 100, 100));
  d.shape_list = eina_list_append(d.shape_list, shape);

  shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, d.vg,
    efl_gfx_path_append_svg_path(efl_added, morph1[1]),           // Set SVG path
    efl_gfx_shape_stroke_color_set(efl_added, 0, 0, 255, 255),
    efl_gfx_shape_stroke_width_set(efl_added, 10),
    efl_canvas_vg_node_origin_set(efl_added, 150, 150));
  d.shape_list = eina_list_append(d.shape_list, shape);

  // Base Object
  shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container);
  d.shape_list = eina_list_append(d.shape_list, shape);

}

// 2. Interpolation Test Case END


// 3. Gradient Test Case START
static void
_gradient_key_handle(void *data EINA_UNUSED, const Efl_Event *ev)
{
   if (!strcmp(efl_input_key_sym_get(ev->info) , "h"))
     {
        /* h - Print help */
        puts(basic_shape_menu);
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "e"))
     {
        /* e - Exit */
        _on_delete(d.ee);
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "b"))
     {
        /* b - Back to main menu */
        _main_menu();
     }
}

static void
_3_gradient_test()
{
  Efl_VG *container, *shape;
  Efl_Canvas_Vg_Gradient *grad_fill, *grad_stroke;

  reset_test();
  efl_event_callback_add(d.vg, EFL_EVENT_KEY_DOWN, _gradient_key_handle, NULL);
  puts(gradient_menu);

  container = efl_add(EFL_CANVAS_VG_CONTAINER_CLASS, d.vg);

  // Rect Shape
  shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_path_append_rect(efl_added, 0, 0, 100 , 100, 10, 10),
    efl_gfx_shape_stroke_color_set(efl_added, 255, 0, 0, 255),
    efl_gfx_shape_stroke_width_set(efl_added, 5),
    efl_canvas_vg_node_origin_set(efl_added, 50, 50));

  //Make Linear gradient for fill
  grad_stroke = efl_add(EFL_CANVAS_VG_GRADIENT_LINEAR_CLASS, container,
     efl_gfx_gradient_linear_start_set(efl_added, 0, 0),
     efl_gfx_gradient_linear_end_set(efl_added, 100, 100),
     efl_gfx_entity_visible_set(efl_added, EINA_TRUE));
  //Make Stops
  float pa;
  Efl_Gfx_Gradient_Stop stops[2];
  stops[0].offset = 0;
  pa = 100.0 / 255.0;
  stops[0].r = (int)(255.0 * pa);
  stops[0].g = 0;
  stops[0].b = 0;
  stops[0].a = 100;

  stops[1].offset = 1;
  pa = 200.0 / 255;
  stops[1].r = 0;
  stops[1].g = 0;
  stops[1].b = (int)(255.0 * pa);
  stops[1].a = 200.0;
  efl_gfx_gradient_stop_set(grad_stroke, stops, 2);
  //Set Gradient
  efl_canvas_vg_shape_stroke_fill_set(shape, grad_stroke);

  shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_path_append_rect(efl_added, 0, 0, 100 , 100, 10, 10),
    efl_gfx_shape_stroke_width_set(efl_added, 5),
    efl_gfx_color_set(efl_added, 0, 0, 100, 100),
    efl_canvas_vg_node_origin_set(efl_added, 50, 200));
  //Make Radial Gradient for Stroke
  grad_fill = efl_add(EFL_CANVAS_VG_GRADIENT_RADIAL_CLASS, container,
     efl_gfx_gradient_radial_center_set(efl_added, 50, 50),
     efl_gfx_gradient_radial_focal_set(efl_added, 10, 10),
     efl_gfx_gradient_radial_radius_set(efl_added, 100),
     efl_gfx_entity_visible_set(efl_added, EINA_TRUE));
  //Make Stops
  stops[0].offset = 0;
  pa = 100.0 / 255.0;
  stops[0].r = (int)(255.0 * pa);
  stops[0].g = 0;
  stops[0].b = 0;
  stops[0].a = 100;

  stops[1].offset = 1;
  pa = 100.0 / 255;
  stops[1].r = 0;
  stops[1].g = (int)(255.0 * pa);
  stops[1].b = 0;
  stops[1].a = 100.0;
  efl_gfx_gradient_stop_set(grad_fill, stops, 2);
  //Set Gradient
  efl_canvas_vg_shape_fill_set(shape, grad_fill);

  efl_canvas_vg_object_root_node_set(d.vg, container);
}
// 3. Gradient Test Case END

// Main Menu START
static void
_main_menu()
{
  Efl_VG *shape, *container;
  if (animator) ecore_animator_del(animator);
  animator = NULL;
  if(d.vg) efl_del(d.vg);
  if(d.bg) efl_del(d.bg);
  d.bg = efl_add(EFL_CANVAS_RECTANGLE_CLASS, d.evas,
    efl_gfx_color_set(efl_added, 255, 255, 255, 255),
    efl_gfx_entity_visible_set(efl_added, EINA_TRUE),
    efl_canvas_object_key_focus_set(efl_added, EINA_TRUE),
    efl_event_callback_add(efl_added, EFL_EVENT_KEY_DOWN, _main_menu_key_handle, NULL));

  // Create the initial screen - Yellow color bat
  // Make Efl.Canvas.Vg.Object object
  d.vg = efl_add(EFL_CANVAS_VG_OBJECT_CLASS, d.evas,
    efl_gfx_entity_visible_set(efl_added, EINA_TRUE));

  // Make Efl.Canvas.Vg.Container Object
  container = efl_add(EFL_CANVAS_VG_CONTAINER_CLASS, d.vg);
  // Set the root node of Efl.Canvas.Vg.Object
  // The Efl.Canvas.Vg.Object searches and renders
  // Efl.Canvas.Vg.Node(Shape, Container, etc...) from set root node.
  efl_canvas_vg_object_root_node_set(d.vg, container);

  // Make Efl.Canvas.Vg.Shape
  shape = efl_add(EFL_CANVAS_VG_SHAPE_CLASS, container,
    efl_gfx_shape_stroke_color_set(efl_added, 255, 0, 0, 255), // Set Stroke color
    efl_gfx_shape_stroke_width_set(efl_added, 5),              // Set Stroke width
    efl_gfx_color_set(efl_added, 255, 255, 0, 255),            // Set Fill color
    efl_canvas_vg_node_origin_set(efl_added, 25, 100));         // Set Position

  // Reserve memory for the number of commands and pointers
  efl_gfx_path_reserve(shape, batman_commandCnt, batman_pointCnt);
  efl_gfx_path_append_svg_path(shape, batman);           // Set SVG path

  _canvas_resize_cb(d.ee);
  puts(main_menu);
}

static void
_main_menu_key_handle(void *data EINA_UNUSED, const Efl_Event *ev)
{

   if (!strcmp(efl_input_key_sym_get(ev->info) , "h"))
     {
        /* h - Help menu */
        puts(main_menu);
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "e"))
     {
        /* e - Exit */
        _on_delete(d.ee);
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "1"))
     {
        /* 1 - Basic Shape test */
        _1_basic_shape_test();
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "2"))
     {
        /* 2 - Interpolation */
        _2_interpolation_test();
     }
   else if (!strcmp(efl_input_key_sym_get(ev->info) , "3"))
     {
        /* 3 - Gradient */
        _3_gradient_test();
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

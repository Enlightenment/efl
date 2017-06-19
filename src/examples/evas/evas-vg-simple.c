/**
 * Example of basic nodes in Evas_VG.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one). See stdout/stderr for output.
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

#define WIDTH 400
#define HEIGHT 400

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <math.h>

#include <Eo.h>
#include <Efl.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Eina.h>

#define PATH_KAPPA 0.5522847498
#define PI         3.1415926535

static Efl_VG *beginning = NULL;
static Efl_VG *end = NULL;
static Efl_VG *root = NULL;
static double start_time = 0;
static Ecore_Animator *anim = NULL;

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
   Efl_VG *root_node, *tmp_vg;

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
   eina_matrix3_identity(&matrix);
   eina_matrix3_rotate(&matrix, radian);

   root = efl_add(EFL_VG_CONTAINER_CLASS, NULL);
   //evas_vg_node_transformation_set(root, &matrix);

   Efl_VG *bg = efl_add(EFL_VG_SHAPE_CLASS, root, efl_vg_name_set(efl_added, "bg"));
   evas_vg_shape_append_rect(bg, 0, 0 , vg_w, vg_h, 0, 0);
   evas_vg_node_origin_set(bg, 0,0);
   evas_vg_shape_stroke_width_set(bg, 1.0);
   evas_vg_node_color_set(bg, 80, 80, 80, 80);

   Efl_VG *shape = efl_add(EFL_VG_SHAPE_CLASS, root, efl_vg_name_set(efl_added, "shape"));
   Efl_VG *rgradient = efl_add(EFL_VG_GRADIENT_RADIAL_CLASS, NULL, efl_vg_name_set(efl_added, "rgradient"));
   Efl_VG *lgradient = efl_add(EFL_VG_GRADIENT_LINEAR_CLASS, NULL, efl_vg_name_set(efl_added, "lgradient"));

   evas_vg_shape_append_arc(shape, 0, 0, 100, 100, 25, 330);

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

   evas_vg_node_origin_set(rgradient, 10, 10);
   evas_vg_gradient_spread_set(rgradient, EFL_GFX_GRADIENT_SPREAD_REFLECT);
   evas_vg_gradient_stop_set(rgradient, stops, 3);
   evas_vg_gradient_radial_center_set(rgradient, 30, 30);
   evas_vg_gradient_radial_radius_set(rgradient, 80);

   evas_vg_node_origin_set(lgradient, 10, 10);
   evas_vg_gradient_stop_set(lgradient, stops, 3);
   evas_vg_gradient_spread_set(lgradient, EFL_GFX_GRADIENT_SPREAD_REFLECT);
   evas_vg_gradient_stop_set(lgradient, stops, 3);
   evas_vg_gradient_linear_start_set(lgradient, 10, 10);
   evas_vg_gradient_linear_end_set(lgradient, 50, 50);

   evas_vg_node_origin_set(shape, 10, 10);
   evas_vg_shape_fill_set(shape, rgradient);
   evas_vg_shape_stroke_scale_set(shape, 2.0);
   evas_vg_shape_stroke_width_set(shape, 1.0);
   evas_vg_node_color_set(shape, 0, 0, 255, 255);
   evas_vg_shape_stroke_color_set(shape, 0, 0, 255, 128);

   Efl_VG *rect = efl_add(EFL_VG_SHAPE_CLASS, root, efl_vg_name_set(efl_added, "rect"));
   evas_vg_shape_append_rect(rect, 0, 0, 100, 100, 0, 0);
   evas_vg_node_origin_set(rect, 100, 100);
   evas_vg_shape_fill_set(rect, lgradient);
   evas_vg_shape_stroke_width_set(rect, 2.0);
   evas_vg_shape_stroke_join_set(rect, EFL_GFX_JOIN_ROUND);
   evas_vg_shape_stroke_color_set(rect, 255, 255, 255, 255);

   Efl_VG *rect1 = efl_add(EFL_VG_SHAPE_CLASS, root, efl_vg_name_set(efl_added, "rect1"));
   evas_vg_shape_append_rect(rect1, 0, 0, 70, 70, 0, 0);
   evas_vg_node_origin_set(rect1, 50, 70);
   evas_vg_shape_stroke_scale_set(rect1, 2);
   evas_vg_shape_stroke_width_set(rect1, 8.0);
   evas_vg_shape_stroke_join_set(rect1, EFL_GFX_JOIN_ROUND);
   evas_vg_shape_stroke_color_set(rect1, 0, 100, 80, 100);

   Efl_VG *circle = efl_add(EFL_VG_SHAPE_CLASS, root, efl_vg_name_set(efl_added, "circle"));
   evas_vg_shape_append_arc(circle, 0, 0, 250, 100, 30, 300);
   evas_vg_shape_fill_set(circle, lgradient);
   //evas_vg_node_transformation_set(&matrix),
   evas_vg_node_origin_set(circle, 50,50);
   evas_vg_node_color_set(circle, 50, 0, 0, 50);

   // Foreground
   Efl_VG *fg = efl_add(EFL_VG_SHAPE_CLASS, root, efl_vg_name_set(efl_added, "fg"));
   evas_vg_shape_append_rect(fg, 0, 0, vg_w, vg_h, 0, 0);
   evas_vg_node_origin_set(fg, 0, 0);
   evas_vg_shape_stroke_width_set(fg, 5.0);
   evas_vg_shape_stroke_join_set(fg, EFL_GFX_JOIN_ROUND);
   evas_vg_shape_stroke_color_set(fg, 70, 70, 0, 70);

   Efl_VG *tst = efl_add(EFL_VG_SHAPE_CLASS, root, efl_vg_name_set(efl_added, "tst"));
   evas_vg_shape_append_rect(tst, 50, 25, 200, 200, 3, 5);
   evas_vg_node_color_set(tst, 0, 0, 200, 200);
   evas_vg_shape_stroke_width_set(tst, 2);
   evas_vg_shape_stroke_color_set(tst, 255, 0, 0, 255);

   Efl_VG *vc = efl_add(EFL_VG_SHAPE_CLASS, root, efl_vg_name_set(efl_added, "vc"));
   evas_vg_shape_append_circle(vc, 100, 100, 23);
   evas_vg_node_color_set(vc, 0, 200, 0, 255);
   evas_vg_shape_stroke_width_set(vc, 4);
   evas_vg_shape_stroke_color_set(vc, 255, 0, 0, 255);

   beginning = efl_add(EFL_VG_CONTAINER_CLASS, NULL, efl_vg_dup(efl_added, root));
   end = efl_add(EFL_VG_CONTAINER_CLASS, NULL, efl_vg_dup(efl_added, root));

   circle = efl_vg_container_child_get(end, "circle");
   efl_vg_transformation_set(circle, &matrix);

   root_node = evas_object_vg_root_node_get(d.vg);
   // check if the dupe is working properly or not
   efl_parent_set(beginning, root_node);

   tmp_vg = root;
   root = beginning;
   beginning = tmp_vg;
}

static Eina_Bool
_anim(void *data EINA_UNUSED)
{
   double pos, now;

   now = ecore_loop_time_get();

   if (now - start_time > 3)
     {
        Efl_VG *tmp = beginning;

        beginning = end;
        end = tmp;
        start_time = now;
     }

   pos = ecore_animator_pos_map((now - start_time) / 3, ECORE_POS_MAP_SINUSOIDAL, 0, 0);

   efl_vg_interpolate(root, beginning, end, pos);

   return EINA_TRUE;
}

static void
_keydown(void *data EINA_UNUSED, Evas *evas EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *einfo)
{
   Evas_Event_Key_Down *ev = einfo;

   if (strcmp(ev->key, "a") == 0)
     {
        if (!anim)
          {
             anim = ecore_animator_add(_anim, NULL);
             start_time = ecore_loop_time_get();
          }
        else
          {
             ecore_animator_del(anim);
             anim = NULL;
          }
        /* efl_vg_interpolate(root, beginning, end, 0.5); */
     }
   fprintf(stderr, "key: [%s]\n", ev->key);
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
   evas_object_focus_set(d.bg, 1);
   evas_object_event_callback_add(d.bg, EVAS_CALLBACK_KEY_DOWN, _keydown, NULL);
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

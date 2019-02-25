/**
 * Example comparing map with and without anti-aliasing (AA) with an Eo-styled Evas API.
 *
 * @verbatim
 * gcc -o evas_map_aa_eo evas-map-aa-eo.c `pkg-config --libs --cflags evas ecore ecore-evas eina`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#endif

#ifndef EFL_BETA_API_SUPPORT
# define EFL_BETA_API_SUPPORT
#endif

#ifndef EFL_EO_API_SUPPORT
# define EFL_EO_API_SUPPORT
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "evas-common.h"

#define WIDTH  (800)
#define HEIGHT (400)

#define IMAGE_SIZE_W 256
#define IMAGE_SIZE_H 256

struct example_data
{
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
   Evas_Object *target1;
   Evas_Object *target2;
   Evas_Coord px1, py1;
   Evas_Coord px2, py2;
   Evas_Coord px3, py3;
   Evas_Coord px4, py4;
   Eina_Bool r1_down;
   Eina_Bool r2_down;
   Eina_Bool r3_down;
   Eina_Bool r4_down;
   Eina_Bool image;
};

static const char *img_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/cube1.png";

static struct example_data d =
{ NULL, NULL, NULL, NULL, NULL, 62, 62, 262, 62, 262, 262, 62, 262,
  EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE };

static void
update(void)
{
   efl_gfx_mapping_coord_absolute_set(d.target1, 0, d.px1, d.py1, 0);
   efl_gfx_mapping_coord_absolute_set(d.target1, 1, d.px2, d.py2, 0);
   efl_gfx_mapping_coord_absolute_set(d.target1, 2, d.px3, d.py3, 0);
   efl_gfx_mapping_coord_absolute_set(d.target1, 3, d.px4, d.py4, 0);

   efl_gfx_mapping_coord_absolute_set(d.target2, 0, d.px1 + 400, d.py1, 0);
   efl_gfx_mapping_coord_absolute_set(d.target2, 1, d.px2 + 400, d.py2, 0);
   efl_gfx_mapping_coord_absolute_set(d.target2, 2, d.px3 + 400, d.py3, 0);
   efl_gfx_mapping_coord_absolute_set(d.target2, 3, d.px4 + 400, d.py4, 0);
}

static void
r1_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   d.r1_down = EINA_TRUE;
}

static void
r1_up_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
         Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   d.r1_down = EINA_FALSE;
}

static void
r1_move_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
           void *event_info)
{
   Evas_Coord x, y, w, h;

   if (!d.r1_down) return;

   Evas_Event_Mouse_Move *ev = event_info;

   evas_object_geometry_get(obj, &x, &y, &w, &h);

   evas_object_move(obj, ev->cur.canvas.x - (w/2), ev->cur.canvas.y - (h/2));

   d.px1 = ev->cur.canvas.x;
   d.py1 = ev->cur.canvas.y;

   update();
}

static void
r2_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   d.r2_down = EINA_TRUE;
}

static void
r2_up_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
         Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   d.r2_down = EINA_FALSE;
}

static void
r2_move_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
           void *event_info)
{
   Evas_Coord x, y, w, h;

   if (!d.r2_down) return;

   Evas_Event_Mouse_Move *ev = event_info;

   evas_object_geometry_get(obj, &x, &y, &w, &h);

   evas_object_move(obj, ev->cur.canvas.x - (w/2), ev->cur.canvas.y - (h/2));

   d.px2 = ev->cur.canvas.x;
   d.py2 = ev->cur.canvas.y;

   update();
}

static void
r3_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   d.r3_down = EINA_TRUE;
}

static void
r3_up_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
         Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   d.r3_down = EINA_FALSE;
}

static void
r3_move_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
           void *event_info)
{
   Evas_Coord x, y, w, h;

   if (!d.r3_down) return;

   Evas_Event_Mouse_Move *ev = event_info;

   evas_object_geometry_get(obj, &x, &y, &w, &h);

   evas_object_move(obj, ev->cur.canvas.x - (w/2), ev->cur.canvas.y - (h/2));

   d.px3 = ev->cur.canvas.x;
   d.py3 = ev->cur.canvas.y;

   update();
}

static void
r4_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   d.r4_down = EINA_TRUE;
}

static void
r4_up_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
         Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   d.r4_down = EINA_FALSE;
}

static void
r4_move_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
           void *event_info)
{
   if (!d.r4_down) return;

   Evas_Event_Mouse_Move *ev = event_info;

   Evas_Coord x, y, w, h;
   evas_object_geometry_get(obj, &x, &y, &w, &h);

   evas_object_move(obj, ev->cur.canvas.x - (w/2), ev->cur.canvas.y - (h/2));

   d.px4 = ev->cur.canvas.x;
   d.py4 = ev->cur.canvas.y;

   update();
}

static void
_on_keydown(void *data EINA_UNUSED, Evas *e, Evas_Object *obj EINA_UNUSED,
            void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   if (!strcmp(ev->keyname, "i") || !strcmp(ev->keyname, "I"))
     {
        d.image = !d.image;

        if (d.image)
          {
             evas_object_del(d.target1);
             d.target1 = evas_object_image_filled_add(e);
             evas_object_image_file_set(d.target1, img_path, NULL);
             evas_object_image_smooth_scale_set(d.target1, 0);
             evas_object_resize(d.target1, 200, 200);
             evas_object_show(d.target1);

             evas_object_del(d.target2);
             d.target2 = evas_object_image_filled_add(e);
             evas_object_image_file_set(d.target2, img_path, NULL);
             evas_object_image_smooth_scale_set(d.target2, 0);
             evas_object_anti_alias_set(d.target2, EINA_TRUE);
             evas_object_resize(d.target2, 200, 200);
             evas_object_show(d.target2);
          }
        else
          {
             evas_object_del(d.target1);
             d.target1 = evas_object_rectangle_add(e);
             evas_object_color_set(d.target1, 255, 0, 0, 255);
             evas_object_resize(d.target1, 200, 200);
             evas_object_move(d.target1, 64, 64);
             evas_object_show(d.target1);

             evas_object_del(d.target2);
             d.target2 = evas_object_rectangle_add(e);
             evas_object_anti_alias_set(d.target2, EINA_TRUE);
             evas_object_color_set(d.target2, 255, 0, 0, 255);
             evas_object_resize(d.target2, 200, 200);
             evas_object_move(d.target2, 464, 64);
             evas_object_show(d.target2);
          }
        update();
     }
}

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

int
main(void)
{
   if (!ecore_evas_init())
     return EXIT_FAILURE;

   d.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!d.ee)
     goto panic;

   ecore_evas_callback_delete_request_set(d.ee, _on_delete);
   ecore_evas_show(d.ee);

   d.evas = ecore_evas_get(d.ee);

   d.bg = evas_object_rectangle_add(d.evas);
   evas_object_resize(d.bg, WIDTH, HEIGHT);
   evas_object_color_set(d.bg, 255, 255, 255, 255);
   evas_object_show(d.bg);

   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(d.bg, EVAS_CALLBACK_KEY_DOWN,
                                  _on_keydown, NULL);

   Evas_Object *r1 = evas_object_rectangle_add(d.evas);
   evas_object_event_callback_add(r1, EVAS_CALLBACK_MOUSE_DOWN, r1_down_cb, 0);
   evas_object_event_callback_add(r1, EVAS_CALLBACK_MOUSE_MOVE, r1_move_cb, 0);
   evas_object_event_callback_add(r1, EVAS_CALLBACK_MOUSE_UP, r1_up_cb, 0);
   evas_object_layer_set(r1, EVAS_LAYER_MAX);
   evas_object_color_set(r1, 125, 0, 125, 125);
   evas_object_move(r1, 50, 50);
   evas_object_resize(r1, 25, 25);
   evas_object_show(r1);

   Evas_Object *r2 = evas_object_rectangle_add(d.evas);
   evas_object_event_callback_add(r2, EVAS_CALLBACK_MOUSE_DOWN, r2_down_cb, 0);
   evas_object_event_callback_add(r2, EVAS_CALLBACK_MOUSE_MOVE, r2_move_cb, 0);
   evas_object_event_callback_add(r2, EVAS_CALLBACK_MOUSE_UP, r2_up_cb, 0);
   evas_object_layer_set(r2, EVAS_LAYER_MAX);
   evas_object_color_set(r2, 0, 0, 0, 125);
   evas_object_move(r2, 250, 50);
   evas_object_resize(r2, 25, 25);
   evas_object_show(r2);

   Evas_Object *r3 = evas_object_rectangle_add(d.evas);
   evas_object_event_callback_add(r3, EVAS_CALLBACK_MOUSE_DOWN, r3_down_cb, 0);
   evas_object_event_callback_add(r3, EVAS_CALLBACK_MOUSE_MOVE, r3_move_cb, 0);
   evas_object_event_callback_add(r3, EVAS_CALLBACK_MOUSE_UP, r3_up_cb, 0);
   evas_object_color_set(r3, 0, 0, 125, 125);
   evas_object_layer_set(r3, EVAS_LAYER_MAX);
   evas_object_move(r3, 250, 250);
   evas_object_resize(r3, 25, 25);
   evas_object_show(r3);

   Evas_Object *r4 = evas_object_rectangle_add(d.evas);
   evas_object_event_callback_add(r4, EVAS_CALLBACK_MOUSE_DOWN, r4_down_cb, 0);
   evas_object_event_callback_add(r4, EVAS_CALLBACK_MOUSE_MOVE, r4_move_cb, 0);
   evas_object_event_callback_add(r4, EVAS_CALLBACK_MOUSE_UP, r4_up_cb, 0);
   evas_object_color_set(r4, 0, 125, 0, 125);
   evas_object_layer_set(r4, EVAS_LAYER_MAX);
   evas_object_move(r4, 50, 250);
   evas_object_resize(r4, 25, 25);
   evas_object_show(r4);

   d.target1 = evas_object_rectangle_add(d.evas);
   evas_object_color_set(d.target1, 255, 0, 0, 255);
   evas_object_resize(d.target1, 200, 200);
   evas_object_move(d.target1, 64, 64);
   evas_object_show(d.target1);

   d.target2 = evas_object_rectangle_add(d.evas);
   evas_object_anti_alias_set(d.target2, EINA_TRUE);
   evas_object_color_set(d.target2, 255, 0, 0, 255);
   evas_object_resize(d.target2, 200, 200);
   evas_object_move(d.target2, 464, 64);
   evas_object_show(d.target2);

   ecore_main_loop_begin();
   ecore_evas_shutdown();
   return 0;

panic:
   fprintf(stderr, "error: Requires at least one Evas engine built and linked"
                   " up to ecore-evas for this example to run properly.\n");
   return -2;
}

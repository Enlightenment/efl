/**
 * Simple Evas example illustrating usage of table object.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader also built.
 *
 * @verbatim
 * gcc -o evas-table evas-table.c `pkg-config --libs --cflags ecore-evas`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define __UNUSED__
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>

#include <stdlib.h>

#define WIDTH  100
#define HEIGHT 150

struct test_data
{
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
};

static struct test_data d = {0};

static void
_on_destroy(Ecore_Evas *ee __UNUSED__)
{
   ecore_main_loop_quit();
}

/* here just to keep our example's window size and background image's
 * size in synchrony */
static void
_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(d.bg, w, h);
}

int
main(void)
{
   Evas_Object *table, *rect;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   d.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!d.ee)
     goto error;

   ecore_evas_callback_destroy_set(d.ee, _on_destroy);
   ecore_evas_callback_resize_set(d.ee, _canvas_resize_cb);
   ecore_evas_show(d.ee);

   /* the canvas pointer, de facto */
   d.evas = ecore_evas_get(d.ee);

   d.bg = evas_object_rectangle_add(d.evas);
   evas_object_color_set(d.bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(d.bg, 0, 0); /* at canvas' origin */
   evas_object_resize(d.bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(d.bg);

   table = evas_object_table_add(d.evas);
   evas_object_table_homogeneous_set(table, EVAS_OBJECT_TABLE_HOMOGENEOUS_NONE);
   evas_object_table_padding_set(table, 0, 0);
   evas_object_resize(table, WIDTH, HEIGHT);
   evas_object_show(table);

   rect = evas_object_rectangle_add(d.evas);
   evas_object_color_set(rect, 255, 0, 0, 255);
   evas_object_size_hint_min_set(rect, 100, 50);
   evas_object_show(rect);
   evas_object_table_pack(table, rect, 1, 1, 2, 1);

   rect = evas_object_rectangle_add(d.evas);
   evas_object_color_set(rect, 0, 255, 0, 255);
   evas_object_size_hint_min_set(rect, 50, 100);
   evas_object_show(rect);
   evas_object_table_pack(table, rect, 1, 2, 1, 2);

   rect = evas_object_rectangle_add(d.evas);
   evas_object_color_set(rect, 0, 0, 255, 255);
   evas_object_size_hint_min_set(rect, 50, 50);
   evas_object_show(rect);
   evas_object_table_pack(table, rect, 2, 2, 1, 1);

   rect = evas_object_rectangle_add(d.evas);
   evas_object_color_set(rect, 255, 255, 0, 255);
   evas_object_size_hint_min_set(rect, 50, 50);
   evas_object_show(rect);
   evas_object_table_pack(table, rect, 2, 3, 1, 1);

   ecore_main_loop_begin();

   ecore_evas_free(d.ee);
   ecore_evas_shutdown();
   return 0;

error:
   fprintf(stderr, "you got to have at least one evas engine built and linked"
                   " up to ecore-evas for this example to run properly.\n");
   ecore_evas_shutdown();
   return -1;
}

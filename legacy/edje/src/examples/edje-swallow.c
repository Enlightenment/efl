/**
 * Simple Edje example illustrating swallow functions.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc swallow.edc && gcc -o edje-swallow edje-swallow.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define __UNUSED__
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  (300)
#define HEIGHT (300)

static const char *edje_file_path = PACKAGE_EXAMPLES_DIR "/swallow.edj";

static Ecore_Evas *ee;
static Evas_Object *bg;

static void
_on_delete(Ecore_Evas *ee __UNUSED__)
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
   evas_object_resize(bg, w, h);
}

int
main(void)
{
   Evas_Object *edje_obj, *rect, *obj;
   Evas *evas;

   ecore_evas_init();
   edje_init();

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);

   ecore_evas_callback_delete_request_set(ee, _on_delete);
   ecore_evas_callback_resize_set(ee, _canvas_resize_cb);
   ecore_evas_title_set(ee, "Edje Swallow Example");
   ecore_evas_show(ee);

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);

   edje_obj = edje_object_add(evas);

   edje_object_file_set(edje_obj, edje_file_path, "example_group");
   evas_object_move(edje_obj, 20, 20);
   evas_object_resize(edje_obj, WIDTH - 40, HEIGHT - 40);
   evas_object_show(edje_obj);

   rect = evas_object_rectangle_add(evas);
   evas_object_color_set(rect, 255, 0, 0, 255);
   edje_object_part_swallow(edje_obj, "part_one", rect);

   obj = edje_object_part_swallow_get(edje_obj, "part_one");
   if(obj == rect)
      printf("Swallowing worked!\n");

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();
   edje_shutdown();
   return 0;
}

/**
 * Example of handling events for image objects in Evas.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader/saver also built. See stdout/stderr
 * for output.
 *
 * @verbatim
 * gcc -o evas-images6 evas-images6.c `pkg-config --libs --cflags evas ecore ecore-evas`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <stdio.h>
#include <errno.h>
#include "evas-common.h"

#define WIDTH  (960)
#define HEIGHT (540)

static const char *img_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/scale_down.png";

struct test_data
{
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg, *img;
};

static struct test_data d = {0};


static void
_on_destroy(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

/* Keep the example's window size in sync with the background image's size */
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
   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   d.ee = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);
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

   d.img = evas_object_image_filled_add(d.evas);
   evas_object_image_file_set(d.img, img_path, NULL);
   evas_object_image_load_scale_down_set(d.img, 2);
   evas_object_resize(d.img, WIDTH, HEIGHT);
   evas_object_show(d.img);

   ecore_main_loop_begin();

   ecore_evas_free(d.ee);
   ecore_evas_shutdown();
   return 0;

error:
   fprintf(stderr, "error: Requires at least one Evas engine built and linked"
                   " to ecore-evas for this example to run properly.\n");
   ecore_evas_shutdown();
   return -1;
}

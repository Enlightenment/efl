/**
 * Simple Ecore_Evas example on the Evas buffer engine wrapper
 * functions.
 *
 * You must have Evas compiled with the buffer engine.
 *
 * Compile with:
 *
 * @verbatim
 * gcc -o evas-buffer-simple evas-buffer-simple.c `pkg-config --libs --cflags evas evas-software-buffer`
 * @endverbatim
 *
 */

#ifdef HAVE_CONFIG_H

#include "config.h"
#else
#define __UNUSED__
#define PACKAGE_EXAMPLES_DIR "."
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <stdio.h>

#define WIDTH  (320)
#define HEIGHT (240)

static Ecore_Evas *ee;
static const char *border_img_path = PACKAGE_EXAMPLES_DIR "/red.png";

static void
_on_destroy(Ecore_Evas *ee __UNUSED__)
{
   ecore_main_loop_quit();
}

int
main(void)
{
   Evas *canvas, *sub_canvas;
   Evas_Object *bg, *r1, *r2, *r3; /* "sub" canvas objects */
   Evas_Object *border, *img; /* canvas objects */
   Ecore_Evas *sub_ee;

   ecore_evas_init();

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee) goto error;

   ecore_evas_size_min_set(ee, WIDTH, HEIGHT);
   ecore_evas_size_max_set(ee, WIDTH, HEIGHT);

   ecore_evas_callback_delete_request_set(ee, _on_destroy);
   ecore_evas_title_set(ee, "Ecore_Evas buffer (image) example");
   ecore_evas_show(ee);

   canvas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(canvas);
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);

   /* this is a border around the image containing a scene of another
    * canvas */
   border = evas_object_image_filled_add(canvas);
   evas_object_image_file_set(border, border_img_path, NULL);
   evas_object_image_border_set(border, 3, 3, 3, 3);
   evas_object_image_border_center_fill_set(border, EVAS_BORDER_FILL_NONE);

   evas_object_move(border, WIDTH / 6, HEIGHT / 6);
   evas_object_resize(border, (2 * WIDTH) / 3, (2 * HEIGHT) / 3);
   evas_object_show(border);

   img = ecore_evas_object_image_new(ee);
   evas_object_image_filled_set(img, EINA_TRUE);
   evas_object_image_size_set(
       img, ((2 * WIDTH) / 3) - 6, ((2 * HEIGHT) / 3) - 6);
   sub_ee = ecore_evas_object_ecore_evas_get(img);
   sub_canvas = ecore_evas_object_evas_get(img);

   evas_object_move(img, (WIDTH / 6) + 3, (HEIGHT / 6) + 3);

   /* apply the same size on both! */
   evas_object_resize(img, ((2 * WIDTH) / 3) - 6, ((2 * HEIGHT) / 3) - 6);
   ecore_evas_resize(sub_ee, ((2 * WIDTH) / 3) - 6, ((2 * HEIGHT) / 3) - 6);

   r1 = evas_object_rectangle_add(sub_canvas);
   evas_object_color_set(r1, 255, 0, 0, 255); /* 100% opaque red */
   evas_object_move(r1, 10, 10);
   evas_object_resize(r1, 100, 100);
   evas_object_show(r1);

   r2 = evas_object_rectangle_add(sub_canvas);
   evas_object_color_set(r2, 0, 128, 0, 128); /* 50% opaque green */
   evas_object_move(r2, 10, 10);
   evas_object_resize(r2, 50, 50);
   evas_object_show(r2);

   r3 = evas_object_rectangle_add(sub_canvas);
   evas_object_color_set(r3, 0, 128, 0, 255); /* 100% opaque dark green */
   evas_object_move(r3, 60, 60);
   evas_object_resize(r3, 50, 50);
   evas_object_show(r3);

   evas_object_show(img);
   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();

   return 0;

error:
   fprintf(stderr, "You got to have at least one Evas engine built"
                   " and linked up to ecore-evas for this example to run"
                   " properly.\n");
   ecore_evas_shutdown();
   return -1;
}


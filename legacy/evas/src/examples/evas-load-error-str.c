/**
 * Simple Evas example illustrating evas_load_error_str()'s usage.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader also built. See stdout/stderr for
 * output.
 *
 * @verbatim
 * gcc -o evas-load-error-str evas-load-error-str.c `pkg-config --libs \
 * --cflags ecore-evas`
 * @endverbatim
 *
 */

#ifdef HAVE_CONFIG_H

#include "config.h"
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <stdio.h>
#include <errno.h>

#define WIDTH  (320)
#define HEIGHT (240)

static const char *valid_path = PACKAGE_EXAMPLES_DIR "/enlightenment.png";
static const char *bogus_path = "/tmp/non-existent-220986.png";

int
main(void)
{
   Evas *evas;
   Ecore_Evas *ee;
   Evas_Object *img1, *img2, *bg;
   int err;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);
   if (!ee)
     goto error;

   ecore_evas_show(ee);

   /* the canvas pointer, de facto */
   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);

   img1 = evas_object_image_filled_add(evas);
   evas_object_image_file_set(img1, valid_path, NULL);
   err = evas_object_image_load_error_get(img1);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        fprintf(stderr, "could not load image '%s'. error string is \"%s\"\n",
                valid_path, evas_load_error_str(err));
     }
   else
     {
        fprintf(stdout,
                "loaded image '%s' with succes! error string is \"%s\"\n",
                valid_path, evas_load_error_str(err));

        evas_object_move(img1, 0, 0);
        evas_object_resize(img1, WIDTH / 2, HEIGHT / 2);
        evas_object_show(img1);
     }

   /* image loading will fail for this one -- unless one cheats and
    * puts a valid image on that path */
   img2 = evas_object_image_filled_add(evas);
   evas_object_image_file_set(img2, bogus_path, NULL);
   err = evas_object_image_load_error_get(img2);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        fprintf(stderr, "could not load image '%s': error string is \"%s\"\n",
                bogus_path, evas_load_error_str(err));
     }
   else
     {
        evas_object_move(img2, WIDTH / 2, HEIGHT / 2);
        evas_object_resize(img2, WIDTH / 2, HEIGHT / 2);
        evas_object_show(img2);
     }

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();
   return 0;

  error:
   fprintf(stderr, "you got to have at least one evas engine built and linked"
           " up to ecore-evas for this example to run properly.\n");
   ecore_evas_shutdown();
   return -1;
}


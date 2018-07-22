/**
 * Example of rotating and flipping image objects in Evas.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader/saver also built. See stdout/stderr
 * for output.
 *
 * @verbatim
 * gcc -o evas-images5 evas-images5.c `pkg-config --libs --cflags evas ecore ecore-evas efl`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define EFL_BETA_API_SUPPORT
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <stdio.h>
#include <errno.h>
#include <Efl.h>
#include "evas-common.h"

#define WIDTH  (320)
#define HEIGHT (240)

static const char *img_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/im1.png";
static const char *commands = \
  "commands are:\n"
  "\tp - print image fill property\n"
  "\t0 - rotate by 0\n"
  "\t1 - rotate by 90\n"
  "\t2 - rotate by 180\n"
  "\t3 - rotate by 270\n"
  "\t4 - flip horizontal\n"
  "\t5 - flip vertical\n"
  "\t6 - flip none\n"
  "\ts - save noise image to disk (/tmp dir)\n"
  "\th - print help\n";

const char *file_path = "/tmp/evas-images4-example.png";
const char *quality_str = "quality=100";

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

static void
_on_keydown(void        *data EINA_UNUSED,
            Evas        *evas EINA_UNUSED,
            Evas_Object *o EINA_UNUSED,
            void        *einfo)
{
   Evas_Event_Key_Down *ev = einfo;

   if (strcmp(ev->key, "h") == 0) /* print help */
     {
        puts(commands);
        return;
     }

   if (strcmp(ev->key, "s") == 0) /* save noise image to disk */
     {
        if (!evas_object_image_save(d.img, file_path, NULL, quality_str))
          fprintf(stderr, "Cannot save image to '%s' (flags '%s')\n",
                  file_path, quality_str);
        else
          printf("Image saved to '%s' (flags '%s'), check it out with "
                 "an image viewer\n", file_path, quality_str);

        return;
     }

   if (strcmp(ev->key, "p") == 0) /* print image size*/
     {
        Evas_Coord w, h;

        evas_object_image_size_get(d.img, &w, &h);
        printf("Image has size set to: w=%d, h=%d\n", w, h);
        return;
     }

   int key_val = ev->key[0] - '0';

   switch (key_val)
     {
      case 0:
         efl_orientation_set(d.img, EFL_ORIENT_0);
         break;
      case 1:
         efl_orientation_set(d.img, EFL_ORIENT_90);
         break;
      case 2:
         efl_orientation_set(d.img, EFL_ORIENT_180);
         break;
      case 3:
         efl_orientation_set(d.img, EFL_ORIENT_270);
         break;
      case 4:
         efl_orientation_flip_set(d.img, EFL_FLIP_HORIZONTAL);
         break;
      case 5:
         efl_orientation_flip_set(d.img, EFL_FLIP_VERTICAL);
         break;
      case 6:
         efl_orientation_flip_set(d.img, EFL_FLIP_NONE);
         break;
     }
}

int
main(void)
{
   int err;

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

   d.img = evas_object_image_add(d.evas);
   evas_object_image_file_set(d.img, img_path, NULL);
   err = evas_object_image_load_error_get(d.img);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        fprintf(stderr, "could not load image '%s'. error string is \"%s\"\n",
                img_path, evas_load_error_str(err));
     }
   else
     {
        printf("loaded image '%s' with success! error string is \"%s\"\n",
               img_path, evas_load_error_str(err));

        evas_object_move(d.img, WIDTH / 2, HEIGHT / 2);
        evas_object_image_fill_set(d.img, 0, 0, WIDTH / 2, HEIGHT / 2);
        evas_object_resize(d.img, WIDTH / 2, HEIGHT / 2);
        evas_object_show(d.img);

        evas_object_focus_set(d.bg, EINA_TRUE);
        evas_object_event_callback_add(
          d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);
     }

   puts(commands);
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

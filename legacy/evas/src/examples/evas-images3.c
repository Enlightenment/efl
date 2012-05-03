/**
 * Simple Evas example illustrating some image objects functions
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader/saver also built. See stdout/stderr
 * for output.
 *
 * @verbatim
 * gcc -o evas-images2 evas-images2.c `pkg-config --libs --cflags evas ecore ecore-evas`
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
#include <stdio.h>
#include <errno.h>

#define WIDTH  (320)
#define HEIGHT (240)

static const char *img_path = PACKAGE_EXAMPLES_DIR "/enlightenment.png";
static const char *commands = \
  "commands are:\n"
  "\tw - write new pixel data to image\n"
  "\ti - print image info\n"
  "\ta - save noise image to disk (/tmp dir)\n"
  "\th - print help\n";

const char *file_path = "/tmp/evas-images2-example.png";
const char *quality_str = "quality=100";

struct test_data
{
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
   Evas_Object *logo, *logo1;
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
   evas_object_resize(d.logo, w / 2, h);
   evas_object_move(d.logo1, w / 2, 0);
   evas_object_resize(d.logo1, w / 2, h);
}

static void
_on_keydown(void        *data __UNUSED__,
            Evas        *evas __UNUSED__,
            Evas_Object *o __UNUSED__,
            void        *einfo)
{
   Evas_Event_Key_Down *ev = einfo;

   if (strcmp(ev->keyname, "h") == 0) /* print help */
     {
        fprintf(stdout, commands);
        return;
     }

   if (strcmp(ev->keyname, "i") == 0) /* change proxy's source */
     {
        int stride = evas_object_image_stride_get(d.logo);
        int w, h;

        evas_object_image_size_get(d.logo, &w, &h);

        printf("image size: %dx%d; stride: %d\n", w, h, stride);

        return;
     }

   if (strcmp(ev->keyname, "w") == 0) /* save noise image to disk */
     {
        int i;
        char *pixels = evas_object_image_data_get(d.logo, EINA_FALSE);
        char *bufpixels;
        int w, h;
        int stride;
        Eina_Bool equal = EINA_TRUE;

        evas_object_image_size_get(d.logo, &w, &h);
        stride = evas_object_image_stride_get(d.logo);

        bufpixels = malloc(sizeof(char) * stride * h);
        memcpy(bufpixels, pixels, sizeof(char) * stride * h);

        pixels = evas_object_image_data_get(d.logo, EINA_TRUE);

        for (i = 0; i < (stride * h); i++)
          {
             if (bufpixels[i] != pixels[i])
               {
                  equal = EINA_FALSE;
                  break;
               }
          }

        free(bufpixels);

        if (!equal)
          printf("write pixels different from readonly pixels.\n");

        for (i = ((stride * h) / 4) ; i < ((stride * h) / 2) ; i++)
          {
             pixels[i] = 0;
          }

        // evas_object_image_data_set(d.logo, pixels);
        evas_object_image_data_update_add(d.logo, 0, 0, w, h);
        return;
     }
}

int
main(void)
{
   // unsigned int i;
   // unsigned int pixels[(WIDTH / 4) * (HEIGHT / 4)];

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

   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(
     d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

   d.logo = evas_object_image_filled_add(d.evas);

   evas_object_image_file_set(d.logo, img_path, NULL);
   evas_object_resize(d.logo, WIDTH / 2, HEIGHT);
   evas_object_show(d.logo);

   d.logo1 = evas_object_image_filled_add(d.evas);
   evas_object_image_file_set(d.logo1, img_path, NULL);
   evas_object_resize(d.logo1, WIDTH / 2, HEIGHT);
   evas_object_move(d.logo1, WIDTH / 2, 0);
   evas_object_show(d.logo1);

   fprintf(stdout, commands);
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

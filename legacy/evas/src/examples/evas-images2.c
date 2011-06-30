/**
 * Simple Evas example illustrating some image objects functions
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader also built. See stdout/stderr for
 * output.
 *
 * @verbatim
 * gcc -o evas-images3 evas-images3.c `pkg-config --libs --cflags ecore-evas`
 * @endverbatim
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

static const char *img_path = PACKAGE_EXAMPLES_DIR "/enlightenment.png";
static const char *commands = \
  "commands are:\n"
  "\tp - change proxy image's source\n"
  "\ts - print noise image's stride value\n"
  "\th - print help\n";

struct test_data
{
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *logo, *noise_img, *proxy_img, *bg;
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

   if (strcmp(ev->keyname, "s") == 0) /* print proxy image' stride value */
     {
        int stride = evas_object_image_stride_get(d.noise_img);

        fprintf(stdout,"Image has row stride value of %d, which accounts"
                " for %d pixels\n", stride, stride / 4);

        return;
     }

   if (strcmp(ev->keyname, "p") == 0) /* change proxy's source */
     {
        Evas_Object *source = evas_object_image_source_get(d.proxy_img);

        if (source == d.logo) source = d.noise_img;
        else source = d.logo;

        evas_object_image_source_set(d.proxy_img, source);

        fprintf(stdout, "Proxy image's source changed\n");

        return;
     }
}

int
main(void)
{
   unsigned int i;
   unsigned int pixels[(WIDTH / 4) * (HEIGHT / 4)];

   srand(time(NULL));

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
   evas_object_image_fill_set(d.logo, 0, 0, WIDTH / 2, HEIGHT / 2);
   evas_object_resize(d.logo, WIDTH / 2, HEIGHT / 2);
   evas_object_show(d.logo);

   /* creating noise image */
   for (i = 0; i < sizeof(pixels) / sizeof(pixels[0]); i++)
     pixels[i] = rand();

   d.noise_img = evas_object_image_add(d.evas);
   evas_object_image_size_set(d.noise_img, WIDTH / 4, HEIGHT / 4);
   evas_object_image_data_set(d.noise_img, pixels);
   evas_object_image_filled_set(d.noise_img, EINA_TRUE);
   evas_object_move(d.noise_img, (WIDTH * 3)/ 4, 0);
   evas_object_resize(d.noise_img, WIDTH / 4, HEIGHT / 4);
   evas_object_show(d.noise_img);
   fprintf(stdout, "Creating noise image with size %d, %d\n",
           WIDTH / 4, HEIGHT / 4);

   /* todo: option to save noise image to /tmp dir */

   d.proxy_img = evas_object_image_filled_add(d.evas);
   evas_object_image_source_set(d.proxy_img, d.logo);
   evas_object_move(d.proxy_img, WIDTH / 4, HEIGHT / 2);
   evas_object_resize(d.proxy_img, WIDTH / 2, HEIGHT / 2);
   evas_object_show(d.proxy_img);

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

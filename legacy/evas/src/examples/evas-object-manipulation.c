/**
 * Simple Evas example illustrating basic objects manipulation.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader also built. See stdout/stderr for
 * output.
 *
 * @verbatim
 * gcc -o evas-events evas-events.c `pkg-config --libs --cflags ecore-evas`
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
static const char *border_img_path = PACKAGE_EXAMPLES_DIR "/red.png";

struct test_data
{
   Ecore_Evas  *ee;
   Evas        *canvas;
   Evas_Object *img, *bg, *clipper, *clipper_border, *text;
};

static struct test_data d = {0};

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
        fprintf(stdout, "commands are:\n"
                        "\to - change clipper's opacity\n"
                        "\tr - toggle clipper's color between red and white\n"
                        "\tc - toggle clipper's clipping function\n"
                        "\tv - toggle clipper's visibility\n");
        return;
     }

   if (strcmp(ev->keyname, "o") == 0) /* change clipper's opacity */
     {
        int alpha, r, g, b;

        evas_object_color_get(d.clipper, &r, &g, &b, &alpha);
        alpha -= 20;
        if (alpha < 0)
          alpha = 255;

        evas_object_color_set(d.clipper, r, g, b, alpha);

        fprintf(stdout, "Changing clipper's opacity: %d%%\n",
                (int)((alpha / 255.0) * 100));
        return;
     }

   if (strcmp(ev->keyname, "r") == 0) /* toggle clipper's color
                                       * between red and white */
     {
        int alpha, r, g, b;

        fprintf(stdout, "Changing clipper's color to");

        evas_object_color_get(d.clipper, &r, &g, &b, &alpha);
        if (g > 0)
          {
             fprintf(stdout, "red\n");
             g = b = 0;
          }
        else
          {
             fprintf(stdout, "white\n");
             g = b = 255;
          }

        evas_object_color_set(d.clipper, r, g, b, alpha);
        return;
     }

   if (strcmp(ev->keyname, "c") == 0) /* toggle clipper's clipping function */
     {
        fprintf(stdout, "Toggling clipping ");

        if (evas_object_clip_get(d.img) == d.clipper)
          {
             evas_object_clip_unset(d.img);
             fprintf(stdout, "off\n");
          }
        else
          {
             evas_object_clip_set(d.img, d.clipper);
             fprintf(stdout, "on\n");
          }
        return;
     }

   if (strcmp(ev->keyname, "v") == 0) /* toggle clipper's visibility */
     {
        fprintf(stdout, "Clipper is now ");

        if (evas_object_visible_get(d.clipper))
          {
             evas_object_hide(d.clipper);
             fprintf(stdout, "hidden\n");
          }
        else
          {
             evas_object_show(d.clipper);
             fprintf(stdout, "visible\n");
          }
        return;
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

   ecore_evas_callback_resize_set(d.ee, _canvas_resize_cb);
   ecore_evas_show(d.ee);

   /* the canvas pointer, de facto */
   d.canvas = ecore_evas_get(d.ee);

   d.bg = evas_object_rectangle_add(d.canvas);
   evas_object_name_set(d.bg, "background rectangle");
   evas_object_color_set(d.bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(d.bg, 0, 0); /* at canvas' origin */
   evas_object_resize(d.bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(d.bg);

   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(
     d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

   d.img = evas_object_image_filled_add(d.canvas);
   evas_object_image_file_set(d.img, img_path, NULL);
   err = evas_object_image_load_error_get(d.img);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        goto panic;
     }
   else
     {
        evas_object_move(d.img, 0, 0);
        evas_object_resize(d.img, WIDTH, HEIGHT);
        evas_object_show(d.img);

        fprintf(stdout, "Image object added, type is: %s\n",
                evas_object_type_get(d.img));
     }

   /* border on the image's clipper, here just to emphasize its position */
   d.clipper_border = evas_object_image_filled_add(d.canvas);
   evas_object_image_file_set(d.clipper_border, border_img_path, NULL);
   err = evas_object_image_load_error_get(d.clipper_border);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        goto panic;
     }
   else
     {
        evas_object_image_border_set(d.clipper_border, 3, 3, 3, 3);
        evas_object_image_border_center_fill_set(
          d.clipper_border, EVAS_BORDER_FILL_NONE);
        evas_object_move(d.clipper_border, (WIDTH / 4) - 3, (HEIGHT / 4) - 3);
        evas_object_resize(
          d.clipper_border, (WIDTH / 2) + 6, (HEIGHT / 2) + 6);
        evas_object_show(d.clipper_border);
     }

   /* solid white clipper (note that it's the default color for a
    * rectangle) - it won't change clippees' colors, then (multiplying
    * by 255) */
   d.clipper = evas_object_rectangle_add(d.canvas);
   evas_object_move(d.clipper, WIDTH / 4, HEIGHT / 4);
   evas_object_resize(d.clipper, WIDTH / 2, HEIGHT / 2);
   evas_object_clip_set(d.img, d.clipper);
   evas_object_show(d.clipper);

   ecore_main_loop_begin();

   ecore_evas_free(d.ee);
   ecore_evas_shutdown();
   return 0;

error:
   fprintf(stderr, "you got to have at least one evas engine built and linked"
                   " up to ecore-evas for this example to run properly.\n");
panic:
   ecore_evas_free(d.ee);
   ecore_evas_shutdown();
   return -1;
}

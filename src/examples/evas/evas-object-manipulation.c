/**
 * Example of basic object manipulation in Evas.
 *
 * This example shows how to manipulate objects within an Ecore-Evas
 * canvas.
 *
 * Please note that this example uses Evas' legacy API.  Compare this
 * implementation with evas-object-manipulation-eo.c to learn how the
 * new Eo API is used.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader also built. See stdout/stderr for
 * output.
 *
 * @verbatim
 * gcc -o evas-object-manipulation evas-object-manipulation.c `pkg-config --libs --cflags ecore evas ecore-evas`
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

#define WIDTH  (320)
#define HEIGHT (240)

static const char *img_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/enlightenment.png";
static const char *border_img_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/red.png";

struct test_data
{
   Ecore_Evas  *ee;
   Evas        *canvas;
   Evas_Object *img, *bg, *clipper, *clipper_border, *text;
};

static struct test_data d = {0};

/* Keep the example's window size in sync with the background image's size */
static void
_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(d.bg, w, h);
}

/* Keyboard event callback routine, to enable the user to toggle various
 * object properties on the clipper object.
 */
static void
_on_keydown(void        *data EINA_UNUSED,
            Evas        *evas EINA_UNUSED,
            Evas_Object *o EINA_UNUSED,
            void        *einfo)
{
   Evas_Event_Key_Down *ev = einfo;

   if (strcmp(ev->key, "h") == 0)
     {
        /* h - print help */
        printf("commands are:\n"
               "\to - change clipper's opacity\n"
               "\tr - toggle clipper's color between red and white\n"
               "\tc - toggle clipper's clipping function\n"
               "\tv - toggle clipper's visibility\n");
        return;
     }

   if (strcmp(ev->key, "o") == 0)
     {
        /* o - Change clipper's opacity */
        int alpha, r, g, b;

        evas_object_color_get(d.clipper, &r, &g, &b, &alpha);
        evas_color_argb_unpremul(alpha, &r, &g, &b);

        alpha -= 20;
        if (alpha < 0)
          alpha = 255;

        evas_color_argb_premul(alpha, &r, &g, &b);
        evas_object_color_set(d.clipper, r, g, b, alpha);

        printf("Changing clipper's opacity: %d%%\n",
                (int)((alpha / 255.0) * 100));
        return;
     }

   if (strcmp(ev->key, "r") == 0)
     {
        /* r - Toggle clipper's color between red and white */
        int alpha, r, g, b;

        printf("Changing clipper's color to");

        evas_object_color_get(d.clipper, &r, &g, &b, &alpha);
        evas_color_argb_unpremul(alpha, &r, &g, &b);

        if (g > 0)
          {
             printf("red\n");
             g = b = 0;
          }
        else
          {
             printf("white\n");
             g = b = 255;
          }

        evas_color_argb_premul(alpha, &r, &g, &b);
        evas_object_color_set(d.clipper, r, g, b, alpha);
        return;
     }

   if (strcmp(ev->key, "c") == 0)
     {
        /* o - Toggle clipper's clipping function */
        printf("Toggling clipping ");

        if (evas_object_clip_get(d.img) == d.clipper)
          {
             evas_object_clip_unset(d.img);
             printf("off\n");
          }
        else
          {
             evas_object_clip_set(d.img, d.clipper);
             printf("on\n");
          }
        return;
     }

   if (strcmp(ev->key, "v") == 0)
     {
        /* v - Toggle clipper's visibility */
        printf("Clipper is now ");

        if (evas_object_visible_get(d.clipper))
          {
             evas_object_hide(d.clipper);
             printf("hidden\n");
          }
        else
          {
             evas_object_show(d.clipper);
             printf("visible\n");
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

   d.ee = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);
   if (!d.ee)
     goto error;

   ecore_evas_callback_resize_set(d.ee, _canvas_resize_cb);
   ecore_evas_show(d.ee);

   d.canvas = ecore_evas_get(d.ee);

   /* Create background.  As mentioned earlier, the evas_object_*
    * routines are part of the legacy Evas API; with the new API
    * you should use code as shown in evas-object-manipulation-eo.c.
    */
   d.bg = evas_object_rectangle_add(d.canvas);
   evas_object_name_set(d.bg, "background rectangle");
   evas_object_color_set(d.bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(d.bg, 0, 0); /* at canvas' origin */
   evas_object_resize(d.bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(d.bg);

   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(
     d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

   /* Load enlightenment.png as an image object, then make it fill the
    * whole canvas area.
    */
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

        printf("Image object added, type is: %s\n",
                evas_object_type_get(d.img));
     }

   /* Add a second image to the canvas - a red square this time.  It
    * will be given a border to emphasize its position.
    */
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

   /* Lastly, add a rectangle.  It will be white (the default color for
    * rectangles) and so won't change the color of anything it clips.
    */
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
   fprintf(stderr, "error: Requires at least one Evas engine built and linked"
                   " to ecore-evas for this example to run properly.\n");
panic:
   ecore_evas_free(d.ee);
   ecore_evas_shutdown();
   return -1;
}

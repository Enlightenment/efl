/**
 * Porting evas-object-manipulation.c example
 * to use Eo-styled Evas API.
 *
 * Simple Evas example illustrating basic objects manipulation.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader also built. See stdout/stderr for
 * output.
 *
 * @verbatim
 * gcc -o evas-object-manipulation-eo evas-object-manipulation-eo.c `pkg-config --libs --cflags ecore evas ecore-evas eo`
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

#define WIDTH  (320)
#define HEIGHT (240)

static const char *img_path = PACKAGE_EXAMPLES_DIR "/enlightenment.png";
static const char *border_img_path = PACKAGE_EXAMPLES_DIR "/red.png";

static const char *commands = \
  "commands are:\n"
  "\to - change clipper's opacity\n"
  "\tr - toggle clipper's color between red and white\n"
  "\tc - toggle clipper's clipping function\n"
  "\tv - toggle clipper's visibility"
  "\th - print help\n";

struct test_data
{
   Ecore_Evas  *ee;
   Evas        *canvas;
   Eo *img, *bg, *clipper, *clipper_border;
};

static struct test_data d = {0};

/* here just to keep our example's window size and background image's
 * size in synchrony */
static void
_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   eo_do(d.bg, evas_obj_size_set(w, h));
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
        fprintf(stdout, "%s", commands);
        return;
     }

   if (strcmp(ev->key, "o") == 0) /* change clipper's opacity */
     {
        int alpha, r, g, b;

        eo_do(d.clipper, evas_obj_color_get(&r, &g, &b, &alpha));
        evas_color_argb_unpremul(alpha, &r, &g, &b);

        alpha -= 20;
        if (alpha < 0)
          alpha = 255;

        evas_color_argb_premul(alpha, &r, &g, &b);
        eo_do(d.clipper, evas_obj_color_set(r, g, b, alpha));

        fprintf(stdout, "Changing clipper's opacity: %d%%\n",
                (int)((alpha / 255.0) * 100));
        return;
     }

   if (strcmp(ev->key, "r") == 0) /* toggle clipper's color
                                       * between red and white */
     {
        int alpha, r, g, b;

        fprintf(stdout, "Changing clipper's color to");

        eo_do(d.clipper, evas_obj_color_get(&r, &g, &b, &alpha));
        evas_color_argb_unpremul(alpha, &r, &g, &b);

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

        evas_color_argb_premul(alpha, &r, &g, &b);
        eo_do(d.clipper, evas_obj_color_set(r, g, b, alpha));
        return;
     }

   if (strcmp(ev->key, "c") == 0) /* toggle clipper's clipping function */
     {
        fprintf(stdout, "Toggling clipping ");

        Evas_Object *clip = NULL;
        clip = eo_do(d.img, evas_obj_clip_get());
        if (clip == d.clipper)
          {
             eo_do(d.img, evas_obj_clip_unset());
             fprintf(stdout, "off\n");
          }
        else
          {
             eo_do(d.img, evas_obj_clip_set(d.clipper));
             fprintf(stdout, "on\n");
          }
        return;
     }

   if (strcmp(ev->key, "v") == 0) /* toggle clipper's visibility */
     {
        Eina_Bool visibility;
        /* Don't use "get"-"set" expressions in one eo_do call,
         * if you pass parameter to "set" by value. */
        visibility = eo_do(d.clipper, evas_obj_visibility_get());
        eo_do(d.clipper, evas_obj_visibility_set(!visibility));
        fprintf(stdout, "Clipper is now %s\n", visibility ? "hidden" : "visible");
        return;
     }
}

int
main(void)
{
   Evas_Load_Error err;

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

   /* Creating object with Eo.
    * Object must be deleted explixitly at the end of program.*/
   d.bg = eo_add(EVAS_RECTANGLE_CLASS, d.canvas);

   /* Eo-styled way to perform actions on an object*/
   eo_do(d.bg, evas_obj_name_set("background rectangle"),
               evas_obj_color_set(255, 255, 255, 255), /* white bg */
               evas_obj_position_set(0, 0), /* at canvas' origin */
               evas_obj_size_set(WIDTH, HEIGHT), /* covers full canvas */
               evas_obj_visibility_set(EINA_TRUE),
               evas_obj_focus_set(EINA_TRUE));

   evas_object_event_callback_add(
     d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

   d.img = eo_add(EVAS_IMAGE_CLASS, d.canvas);

   /* As soon as 'canvas' object is a parent for 'image' object,
    * 'canvas' keeps reference to 'image'.
    * So it's possible to decrement refcount, and 'image' object
    * will be deleted automatically by parent.*/
   eo_unref(d.img);

   eo_do(d.img, evas_obj_image_filled_set(EINA_TRUE),
                efl_file_set(img_path, NULL),
                err = evas_obj_image_load_error_get());

   if (err != EVAS_LOAD_ERROR_NONE)
     {
        goto panic;
     }
   else
     {
        eo_do(d.img, evas_obj_position_set(0, 0),
                     evas_obj_size_set(WIDTH, HEIGHT),
                     evas_obj_visibility_set(EINA_TRUE));

        const char *type = NULL;
        eo_do(d.img, type = evas_obj_type_get());
        fprintf(stdout, "Image object added, type is: %s\n", type);
     }

   /* border on the image's clipper, here just to emphasize its position */
   d.clipper_border = eo_add(EVAS_IMAGE_CLASS, d.canvas);
   eo_do(d.clipper_border, evas_obj_image_filled_set(EINA_TRUE),
                           efl_file_set(border_img_path, NULL),
                           err = evas_obj_image_load_error_get());

   if (err != EVAS_LOAD_ERROR_NONE)
     {
        goto panic;
     }
   else
     {
        eo_do(d.clipper_border,
              evas_obj_image_border_set(3, 3, 3, 3),
              evas_obj_image_border_center_fill_set(EVAS_BORDER_FILL_NONE),
              evas_obj_position_set((WIDTH / 4) -3, (HEIGHT / 4) - 3),
              evas_obj_size_set((WIDTH / 2) + 6, (HEIGHT / 2) + 6),
              evas_obj_visibility_set(EINA_TRUE));
     }
   /* solid white clipper (note that it's the default color for a
    * rectangle) - it won't change clippees' colors, then (multiplying
    * by 255) */
   d.clipper = eo_add(EVAS_RECTANGLE_CLASS, d.canvas);
   eo_unref(d.clipper);

   eo_do(d.clipper,
         evas_obj_position_set( WIDTH / 4, HEIGHT / 4),
         evas_obj_size_set(WIDTH / 2, HEIGHT / 2),
         evas_obj_visibility_set(EINA_TRUE));

   eo_do(d.img, evas_obj_clip_set(d.clipper));

   fprintf(stdout, "%s", commands);

   ecore_main_loop_begin();

   /* Decrementing object's reference count.*/
   eo_unref(d.bg);
   eo_unref(d.clipper_border);

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

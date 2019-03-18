/**
 * Example of basic object manipulation with an Eo-styled Evas API.
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
#define EFL_BETA_API_SUPPORT
#endif

#include <Efl.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <stdio.h>
#include <errno.h>
#include "evas-common.h"

#define WIDTH  (320)
#define HEIGHT (240)

static const char *img_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/enlightenment.png";
static const char *border_img_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/red.png";

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

/* Keep the example's window size in sync with the background image's size */
static void
_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   efl_gfx_entity_size_set(d.bg, EINA_SIZE2D(w,  h));
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
        printf("%s", commands);
        return;
     }

   if (strcmp(ev->key, "o") == 0) /* change clipper's opacity */
     {
        int alpha, r, g, b;

        efl_gfx_color_get(d.clipper, &r, &g, &b, &alpha);
        evas_color_argb_unpremul(alpha, &r, &g, &b);

        alpha -= 20;
        if (alpha < 0)
          alpha = 255;

        evas_color_argb_premul(alpha, &r, &g, &b);
        efl_gfx_color_set(d.clipper, r, g, b, alpha);

        printf("Changing clipper's opacity: %d%%\n",
               (int)((alpha / 255.0) * 100));
        return;
     }

   if (strcmp(ev->key, "r") == 0) /* toggle clipper's color
                                       * between red and white */
     {
        int alpha, r, g, b;

        printf("Changing clipper's color to");

        efl_gfx_color_get(d.clipper, &r, &g, &b, &alpha);
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
        efl_gfx_color_set(d.clipper, r, g, b, alpha);
        return;
     }

   if (strcmp(ev->key, "c") == 0) /* toggle clipper's clipping function */
     {
        printf("Toggling clipping ");

        Evas_Object *clip = NULL;
        clip = efl_canvas_object_clipper_get(d.img);
        if (clip == d.clipper)
          {
             efl_canvas_object_clipper_set(d.img, NULL);
             printf("off\n");
          }
        else
          {
             efl_canvas_object_clipper_set(d.img, d.clipper);
             printf("on\n");
          }
        return;
     }

   if (strcmp(ev->key, "v") == 0) /* toggle clipper's visibility */
     {
        Eina_Bool visibility;
        /* Don't use "get"-"set" expressions in one eo_do call,
         * if you pass parameter to "set" by value. */
        visibility = efl_gfx_entity_visible_get(d.clipper);
        efl_gfx_entity_visible_set(d.clipper, !visibility);
        printf("Clipper is now %s\n", visibility ? "hidden" : "visible");
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
    * Object must be deleted explicitly at the end of program.*/
   d.bg = efl_add(EFL_CANVAS_RECTANGLE_CLASS, d.canvas);

   /* Eo-styled way to perform actions on an object */
   evas_object_name_set(d.bg, "background rectangle");
   efl_gfx_color_set(d.bg, 255, 255, 255, 255);
   /* white bg */
   efl_gfx_entity_position_set(d.bg, EINA_POSITION2D(0, 0));
   /* at canvas' origin */
   efl_gfx_entity_size_set(d.bg, EINA_SIZE2D(WIDTH,  HEIGHT));
   /* covers full canvas */
   efl_gfx_entity_visible_set(d.bg, EINA_TRUE);
   evas_object_focus_set(d.bg, EINA_TRUE);

   evas_object_event_callback_add(
     d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

   d.img = efl_add(EFL_CANVAS_IMAGE_CLASS, d.canvas);

   /* As soon as 'canvas' object is a parent for 'image' object,
    * 'canvas' keeps reference to 'image'.
    * So it's possible to decrement refcount, and 'image' object
    * will be deleted automatically by parent.*/

   if (efl_file_set(d.img, img_path)) goto panic;
   
   err = efl_file_load(d.img);

   if (err != EVAS_LOAD_ERROR_NONE)
     {
        goto panic;
     }
   else
     {
        efl_gfx_entity_position_set(d.img, EINA_POSITION2D(0, 0));
        efl_gfx_entity_size_set(d.img, EINA_SIZE2D(WIDTH,  HEIGHT));
        efl_gfx_entity_visible_set(d.img, EINA_TRUE);
        printf("Image object added, class name is: %s\n",
               efl_class_name_get(d.img));
     }

   /* border on the image's clipper, here just to emphasize its position */
   d.clipper_border = efl_add(EFL_CANVAS_IMAGE_CLASS, d.canvas);
   if (efl_file_set(d.clipper_border, border_img_path)) goto panic;
   err = efl_file_load(d.clipper_border);

   if (err != EVAS_LOAD_ERROR_NONE)
     {
        goto panic;
     }
   else
     {
        efl_gfx_image_border_set(d.clipper_border, 3, 3, 3, 3);
        efl_gfx_image_border_center_fill_set(d.clipper_border, EFL_GFX_BORDER_FILL_MODE_NONE);
        efl_gfx_entity_position_set(d.clipper_border, EINA_POSITION2D((WIDTH / 4) -3, (HEIGHT / 4) - 3));
        efl_gfx_entity_size_set(d.clipper_border, EINA_SIZE2D((WIDTH / 2) + 6,  (HEIGHT / 2) + 6));
        efl_gfx_entity_visible_set(d.clipper_border, EINA_TRUE);
     }

   /* solid white clipper (note that it's the default color for a
    * rectangle) - it won't change clippees' colors, then (multiplying
    * by 255) */
   d.clipper = efl_add(EFL_CANVAS_RECTANGLE_CLASS, d.canvas);

   efl_gfx_entity_position_set(d.clipper, EINA_POSITION2D(WIDTH / 4, HEIGHT / 4));
   efl_gfx_entity_size_set(d.clipper, EINA_SIZE2D(WIDTH / 2,  HEIGHT / 2));
   efl_gfx_entity_visible_set(d.clipper, EINA_TRUE);

   efl_canvas_object_clipper_set(d.img, d.clipper);

   printf("%s", commands);

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

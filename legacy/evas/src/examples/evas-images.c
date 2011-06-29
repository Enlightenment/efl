/**
 * Simple Evas example illustrating some image objects functions and  evas_load_error_str()'s usage.
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

static const char *border_img_path = PACKAGE_EXAMPLES_DIR "/red.png";
static const char *valid_path = PACKAGE_EXAMPLES_DIR "/enlightenment.png";
static const char *bogus_path = "/tmp/non-existent-220986.png";
static const char *commands = \
    "commands are:\n"
    "\tx - change image's x fill coordinate\n"
    "\ty - change image's y fill coordinate\n"
    "\tw - change image's w fill size\n"
    "\te - change image's h fill size\n"
    "\tf - toggle image filled property (overrides fill)\n"
    "\ta - toggle image's alpha channel usage\n"
    "\tt - change border's thickness\n"
    "\tb - change border's center region aspect\n"
    "\tc - change border's scaling factor\n"
    "\ts - print image's fill property status\n"
    "\th - print help\n";

struct test_data
{
   Evas *evas;
   Ecore_Evas *ee;
   Evas_Object *img1, *img2, *bg, *border;
};

static struct test_data d = {0};

static const char*
_border_fill_mode_to_str(Evas_Border_Fill_Mode mode)
{
    switch (mode)
        {
        case EVAS_BORDER_FILL_NONE:
            return "none";
        case EVAS_BORDER_FILL_DEFAULT:
            return "default";
        case EVAS_BORDER_FILL_SOLID:
            return "solid";
        default:
            return "invalid";
        }
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

   if (strcmp(ev->keyname, "t") == 0) /* change border's thickness */
     {
         int l, r, t, b;

         evas_object_image_border_get(d.border, &l, &r, &t, &b);

         l = (l + 3) % 9;
         r = (r + 3) % 9;
         t = (t + 3) % 9;
         b = (b + 3) % 9;

         evas_object_image_border_set(d.border, l, r, t, b);

        fprintf(stdout, "Image's border thickness is now %d\n", l);

        return;
     }

   if (strcmp(ev->keyname, "c") == 0) /* change border's scaling factor */
     {
        double scale = evas_object_image_border_scale_get(d.border);

        scale *= 2;
        if (scale > 4.0) scale = 1.0;

        evas_object_image_border_scale_set(d.border, scale);

        fprintf(stdout, "Image's border scaling factor is now %f\n", scale);

        return;
     }

   if (strcmp(ev->keyname, "b") == 0) /* change border's center
                                       * region's aspect */
     {
        Eina_Bool fill = \
            evas_object_image_border_center_fill_get(d.border);

        fill = (fill + 1) % 3;

        evas_object_image_border_center_fill_set(d.border, fill);

        fprintf(stdout, "Image's border center region aspect is now \"%s\"\n",
                _border_fill_mode_to_str(fill));

        return;
     }

   if (strcmp(ev->keyname, "a") == 0) /* toggle alpha channel usage */
     {
        Eina_Bool alpha = evas_object_image_alpha_get(d.img1);

        evas_object_image_alpha_set(d.img1, !alpha);

        fprintf(stdout, "Image's alpha channel is now %s\n",
                alpha ? "off" : "on");

        return;
     }

   if (strcmp(ev->keyname, "f") == 0) /* toggle filled property */
     {
        Eina_Bool filled = evas_object_image_filled_get(d.img1);

        evas_object_image_filled_set(d.img1, !filled);

        fprintf(stdout, "Image's x filled property is now %s\n",
                filled ? "off" : "on");

        return;
     }

   if (strcmp(ev->keyname, "x") == 0) /* change x fill coordinate */
     {
        Evas_Coord x, y, w, h;

        evas_object_image_fill_get(d.img1, &x, &y, &w, &h);
        x = (x + 20) % (WIDTH / 2);
        evas_object_image_fill_set(d.img1, x, y, w, h);

        fprintf(stdout, "Image's x fill coordinate changed to %d\n", x);

        return;
     }

   if (strcmp(ev->keyname, "y") == 0) /* change y fill coordinate */
     {
        Evas_Coord x, y, w, h;

        evas_object_image_fill_get(d.img1, &x, &y, &w, &h);
        y = (y + 20) % (HEIGHT / 2);
        evas_object_image_fill_set(d.img1, x, y, w, h);

        fprintf(stdout, "Image's y fill coordinate changed to %d\n", y);

        return;
     }

   if (strcmp(ev->keyname, "w") == 0) /* change w fill size */
     {
        Evas_Coord x, y, w, h;

        evas_object_image_fill_get(d.img1, &x, &y, &w, &h);
        if (w > (WIDTH / 2)) w = (WIDTH / 2);
        else w = WIDTH;
        evas_object_image_fill_set(d.img1, x, y, w, h);

        fprintf(stdout, "Image's w fill size changed to %d\n", w);

        return;
     }

   if (strcmp(ev->keyname, "e") == 0) /* change h fill size */
     {
        Evas_Coord x, y, w, h;

        evas_object_image_fill_get(d.img1, &x, &y, &w, &h);
        if (h > (HEIGHT / 2)) h = (HEIGHT / 2);
        else h = HEIGHT;
        evas_object_image_fill_set(d.img1, x, y, w, h);

        fprintf(stdout, "Image's h fill size changed to %d\n", h);

        return;
     }

   if (strcmp(ev->keyname, "s") == 0) /* status */
     {
        Evas_Coord x, y, w, h;

        evas_object_image_fill_get(d.img1, &x, &y, &w, &h);

        fprintf(stdout, "Image has fill properties set to: %d, %d, %d, %d\n",
                x, y, w, h);

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

   ecore_evas_show(d.ee);

   /* the canvas pointer, de facto */
   d.evas = ecore_evas_get(d.ee);

   d.bg = evas_object_rectangle_add(d.evas);
   evas_object_color_set(d.bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(d.bg, 0, 0); /* at canvas' origin */
   evas_object_resize(d.bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(d.bg);

   d.img1 = evas_object_image_add(d.evas);
   evas_object_image_file_set(d.img1, valid_path, NULL);
   err = evas_object_image_load_error_get(d.img1);
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

        evas_object_move(d.img1, 3, 3);
        evas_object_image_fill_set(d.img1, 0, 0, WIDTH / 2, HEIGHT / 2);
        evas_object_resize(d.img1, WIDTH / 2, HEIGHT / 2);
        evas_object_show(d.img1);

        evas_object_focus_set(d.bg, EINA_TRUE);
        evas_object_event_callback_add(
          d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);
     }

   /* this is a border around the image above, here just to emphasize
    * its geometry */
   d.border = evas_object_image_filled_add(d.evas);
   evas_object_image_file_set(d.border, border_img_path, NULL);
   evas_object_image_border_set(d.border, 3, 3, 3, 3);
   evas_object_image_border_center_fill_set(d.border, EVAS_BORDER_FILL_NONE);

   evas_object_move(d.border, 0, 0);
   evas_object_resize(d.border, (WIDTH / 2) + 6, (HEIGHT / 2) + 6);
   evas_object_show(d.border);

   /* image loading will fail for this one -- unless one cheats and
    * puts a valid image on that path */
   d.img2 = evas_object_image_add(d.evas);
   evas_object_image_file_set(d.img2, bogus_path, NULL);
   err = evas_object_image_load_error_get(d.img2);
   if (err != EVAS_LOAD_ERROR_NONE)
     {
        fprintf(stderr, "could not load image '%s': error string is \"%s\"\n",
                bogus_path, evas_load_error_str(err));
     }
   else
     {
        evas_object_move(d.img2, WIDTH / 2, HEIGHT / 2);
        evas_object_image_fill_set(d.img2, 0, 0, WIDTH / 2, HEIGHT / 2);
        evas_object_resize(d.img2, WIDTH / 2, HEIGHT / 2);
        evas_object_show(d.img2);
     }

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

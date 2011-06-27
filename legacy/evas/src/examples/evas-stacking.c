/**
 * Simple Evas example illustrating <b>objects stacking</b> and
 * <b>canvas layers</b>.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader also built. See stdout/stderr for
 * output.
 *
 * @verbatim
 * gcc -o evas-stacking evas-stacking.c `pkg-config --libs --cflags ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WIDTH  320
#define HEIGHT 320

struct test_data
{
   Ecore_Evas  *ee;
   Evas        *canvas;
   Evas_Object *bg;
   Evas_Object *rects[3]; /* red, green, blue */
   int          layers[3]; /* default, below it, above it */
   int          cur_rect, cur_layer;
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

/* use the following commands to interact with this example - 'h' is
 * the key for help */
static void
_on_keydown(void        *data __UNUSED__,
            Evas        *evas __UNUSED__,
            Evas_Object *o __UNUSED__,
            void        *einfo)
{
   Evas_Event_Key_Down *ev = einfo;
   const char *name = evas_object_name_get(d.rects[d.cur_rect]);

   if (strcmp(ev->keyname, "h") == 0)  /* print help */
     {
        fprintf(stdout,
                "commands are:\n"
                "\tc - change the target rectangle to operate on\n"
                "\ta - stack target rectangle one level above\n"
                "\tb - stack target rectangle one level below\n"
                "\tt - stack target rectangle up to the top of its layer\n"
                "\tm - stack target rectangle down to the bottom of its layer\n"
                "\ts - print current stacking information\n"
                "\tl - change background rectangle's layer\n"
                "\th - print help\n");
        return;
     }

   if (strcmp(ev->keyname, "s") == 0)  /* get status of the
                                        * rectangles WRT size
                                        * hints */
     {
        Evas_Object *rect;

        fprintf(stdout, "Order of stacking, from top to bottom, is: ");

        rect = evas_object_top_get(evas);
        fprintf(stdout, "%s", evas_object_name_get(rect));

        rect = evas_object_below_get(rect);
        while (rect)
          {
             fprintf(stdout, ", %s", evas_object_name_get(rect));
             rect = evas_object_below_get(rect);
          }

        fprintf(stdout, ".\n");

        fprintf(stdout, "Current target rectangle is %s\n",
                evas_object_name_get(d.rects[d.cur_rect]));

        fprintf(stdout, "Background rectangle's layer is %d\n",
                evas_object_layer_get(d.bg));

        return;
     }

   if (strcmp(ev->keyname, "l") == 0)  /* change background rectangle's layer */
     {
        d.cur_layer = (d.cur_layer + 1) % 3;
        evas_object_layer_set(d.bg, d.layers[d.cur_layer]);

        fprintf(stdout, "Changing background rectangle's layer to %d\n",
                d.layers[d.cur_layer]);
        return;
     }

   if (strcmp(ev->keyname, "c") == 0)  /* change rectangle to operate on */
     {
        d.cur_rect = (d.cur_rect + 1) % 3;

        fprintf(stdout, "Changing target rectangle to the %s one\n",
                evas_object_name_get(d.rects[d.cur_rect]));
        return;
     }

   if (strcmp(ev->keyname, "t") == 0)  /* bring target to top */
     {
        Evas_Object *neighbour;

        evas_object_raise(d.rects[d.cur_rect]);

        fprintf(stdout, "%s rectangle was re-stacked to the top if its layer\n",
                name);

        neighbour = evas_object_below_get(d.rects[d.cur_rect]);
        fprintf(stdout, "Below of %s rect is %s\n", name,
                neighbour ? evas_object_name_get(neighbour) : "no object");
        return;
     }

   if (strcmp(ev->keyname, "m") == 0)  /* bring target to bottom */
     {
        Evas_Object *neighbour;

        evas_object_lower(d.rects[d.cur_rect]);

        fprintf(stdout,
                "%s rectangle was re-stacked to the bottom if its layer\n",
                name);

        neighbour = evas_object_below_get(d.rects[d.cur_rect]);
        fprintf(stdout, "Below of %s rect is %s\n", name,
                neighbour ? evas_object_name_get(neighbour) : "no object");
        return;
     }

   if (strcmp(ev->keyname, "a") == 0)  /* stack target above */
     {
        Evas_Object *neighbour = evas_object_above_get(d.rects[d.cur_rect]);

        if (!neighbour || (evas_object_layer_get(d.rects[d.cur_rect]) !=
                           evas_object_layer_get(neighbour)))
          return;

        evas_object_stack_above(d.rects[d.cur_rect], neighbour);

        fprintf(stdout, "%s rectangle was re-stacked one level above\n", name);

        neighbour = evas_object_above_get(d.rects[d.cur_rect]);
        fprintf(stdout, "Above of %s rect is %s\n", name,
                neighbour ? evas_object_name_get(neighbour) : "no object");

        neighbour = evas_object_below_get(d.rects[d.cur_rect]);
        fprintf(stdout, "Below of %s rect is %s\n", name,
                neighbour ? evas_object_name_get(neighbour) : "no object");
        return;
     }

   if (strcmp(ev->keyname, "b") == 0)  /* stack target below */
     {
        Evas_Object *neighbour = evas_object_below_get(d.rects[d.cur_rect]);

        if (!neighbour || (evas_object_layer_get(d.rects[d.cur_rect]) !=
                           evas_object_layer_get(neighbour)))
          return;

        evas_object_stack_below(d.rects[d.cur_rect], neighbour);

        fprintf(stdout, "%s rectangle was re-stacked one level below\n", name);

        neighbour = evas_object_above_get(d.rects[d.cur_rect]);
        fprintf(stdout, "Above of %s rect is %s\n", name,
                neighbour ? evas_object_name_get(neighbour) : "no object");

        neighbour = evas_object_below_get(d.rects[d.cur_rect]);

        fprintf(stdout, "Below of %s rect is %s\n", name,
                neighbour ? evas_object_name_get(neighbour) : "no object");
        return;
     }
}

static void
_on_destroy(Ecore_Evas *ee __UNUSED__)
{
   ecore_main_loop_quit();
}

int
main(void)
{
   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   d.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!d.ee)
     goto error;

   ecore_evas_callback_destroy_set(d.ee, _on_destroy);
   ecore_evas_callback_resize_set(d.ee, _canvas_resize_cb);
   ecore_evas_show(d.ee);

   /* the canvas pointer, de facto */
   d.canvas = ecore_evas_get(d.ee);

   d.bg = evas_object_rectangle_add(d.canvas);
   evas_object_name_set(d.bg, "background");  /* white bg */
   evas_object_color_set(d.bg, 255, 255, 255, 255);
   evas_object_move(d.bg, 0, 0);
   evas_object_resize(d.bg, WIDTH, HEIGHT);

   d.layers[0] = evas_object_layer_get(d.bg);
   d.layers[1] = d.layers[0] - 1;
   d.layers[2] = d.layers[0] + 1;

   d.cur_layer = 1;
   evas_object_layer_set(d.bg, d.layers[d.cur_layer]);  /* let's start with it
                                                         * below the default
                                                         * layer */

   evas_object_show(d.bg);

   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(
     d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

   d.rects[2] = evas_object_rectangle_add(d.canvas);
   evas_object_name_set(d.rects[2], "blue");
   evas_object_color_set(d.rects[2], 0, 0, 255, 255);

   evas_object_resize(d.rects[2], WIDTH / 2.2, WIDTH / 2.2);
   evas_object_move(d.rects[2], WIDTH / 6, WIDTH / 4.5);
   evas_object_show(d.rects[2]);

   d.rects[1] = evas_object_rectangle_add(d.canvas);
   evas_object_name_set(d.rects[1], "green");
   evas_object_color_set(d.rects[1], 0, 255, 0, 255);

   evas_object_resize(d.rects[1], WIDTH / 2.2, WIDTH / 2.2);
   evas_object_move(d.rects[1], WIDTH / 2.5, WIDTH / 7);
   evas_object_show(d.rects[1]);

   d.rects[0] = evas_object_rectangle_add(d.canvas);
   evas_object_name_set(d.rects[0], "red");
   evas_object_color_set(d.rects[0], 255, 0, 0, 255);

   evas_object_resize(d.rects[0], WIDTH / 2.2, WIDTH / 2.2);
   evas_object_move(d.rects[0], WIDTH / 3, WIDTH / 2.5);
   evas_object_show(d.rects[0]);

   ecore_main_loop_begin();

   ecore_evas_shutdown();
   return 0;

error:
   fprintf(stderr, "you got to have at least one evas engine built and linked"
                   " up to ecore-evas for this example to run properly.\n");
   return -1;
}

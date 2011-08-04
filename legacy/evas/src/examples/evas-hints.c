/**
 * Simple Evas example illustrating <b>alignment, minimum size, maximum
 * size, padding and weight</b> hints on objects.
 *
 * To exemplify those hints, whe use the Evas box object, one of the
 * managers using size hints to layout its children.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WIDTH  320
#define HEIGHT 480

static const char commands[] = \
  "commands are:\n"
  "\tShift + a - change alignment hints on top rectangle\n"
  "\tShift + m - change min. size hint on top rectangle\n"
  "\tShift + n - change max. size hint on top rectangle\n"
  "\tShift + p - change padding hints on top rectangle\n"
  "\tShift + w - change weight hints on top rectangle\n\n"
  "\tControl + a - change alignment hints on bottom rectangle\n"
  "\tControl + m - change min. size hint on bottom rectangle\n"
  "\tControl + n - change max. size hint on bottom rectangle\n"
  "\tControl + p - change padding hints on bottom rectangle\n"
  "\tControl + w - change weight hints on bottom rectangle\n\n"
  "\ts - print current hints information\n"
  "\th - print help\n";

static const char *border_img_path = PACKAGE_EXAMPLES_DIR "/red.png";

struct coord_tuple
{
   Evas_Coord w, h;
};

struct weight_tuple
{
   double x, y;
};

struct padding_tuple
{
   Evas_Coord l, r, t, b;
};

struct rect_data
{
   struct coord_tuple   *min_ptr;
   struct coord_tuple    min[4];

   struct coord_tuple   *max_ptr;
   struct coord_tuple    max[4];

   struct weight_tuple  *align_ptr;
   struct weight_tuple   align[3];

   struct weight_tuple  *weight_ptr;
   struct weight_tuple   weight[3];

   struct padding_tuple *padding_ptr;
   struct padding_tuple  padding[3];
};

struct test_data
{
   Ecore_Evas      *ee;
   Evas            *canvas;
   struct rect_data t_data, b_data;
   Evas_Object     *bg, *box, *t_rect, *b_rect, *border;
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

   evas_object_move(d.box, (w / 4), (h / 4));
   evas_object_resize(d.box, (w / 2), (h / 2));

   evas_object_move(d.border, (w / 4) - 3, (h / 4) - 3);
   evas_object_resize(d.border, (w / 2) + 6, (h / 2) + 6);
}

static void
_print_rect_stats(Evas_Object *rect)
{
   Evas_Coord w, h, l, r, t, b;
   double x, y;

   evas_object_size_hint_align_get(rect, &x, &y);
   fprintf(stdout, "\talign hints: h(%f), v(%f)\n", x, y);

   evas_object_size_hint_min_get(rect, &w, &h);
   fprintf(stdout, "\tmin. size hints: h(%d), v(%d)\n", w, h);

   evas_object_size_hint_max_get(rect, &w, &h);
   fprintf(stdout, "\tmax. size hints: h(%d), v(%d)\n", w, h);

   evas_object_size_hint_padding_get(rect, &l, &r, &t, &b);
   fprintf(stdout, "\tpadding hints: l(%d), r(%d), t(%d), b(%d)\n",
           l, r, t, b);

   evas_object_size_hint_weight_get(rect, &x, &y);
   fprintf(stdout, "\tweight hints: h(%f), v(%f)\n", x, y);
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
   struct rect_data *r_data = NULL;
   const Evas_Modifier *mods;
   Evas_Object *rect = NULL;
   const char *name = NULL;

   mods = evas_key_modifier_get(evas);
   if (evas_key_modifier_is_set(mods, "Shift"))
     {
        rect = d.t_rect;
        r_data = &d.t_data;
        name = "top";
     }
   else if (evas_key_modifier_is_set(mods, "Control"))
     {
        rect = d.b_rect;
        r_data = &d.b_data;
        name = "bottom";
     }
   else if (strcmp(ev->keyname, "h") == 0) /* print help */
     {
        fprintf(stdout, commands);
        return;
     }
   else if (strcmp(ev->keyname, "s") == 0) /* get aspect status of the
                                            * rectangles WRT size
                                            * hints */
     {
        fprintf(stdout, "Top rectangle:\n");
        _print_rect_stats(d.t_rect);

        fprintf(stdout, "\nBottom rectangle:\n");
        _print_rect_stats(d.b_rect);

        return;
     }

   if (!rect) return;

   if (strcmp(ev->keyname, "a") == 0) /* alignment hints */
     {
        (r_data->align_ptr)++;

        if ((unsigned)
            (((void *)(r_data->align_ptr)) - ((void *)(r_data->align))) >=
            sizeof(r_data->align))
          r_data->align_ptr = r_data->align;

        evas_object_size_hint_align_set(
          rect, r_data->align_ptr->x, r_data->align_ptr->y);

        fprintf(stdout, "Changing align hints for %s rect. to (%f, %f)\n",
                name, r_data->align_ptr->x, r_data->align_ptr->y);
        return;
     }

   if (strcmp(ev->keyname, "m") == 0) /* min. size hints */
     {
        (r_data->min_ptr)++;

        if ((unsigned)
            (((void *)(r_data->min_ptr)) - ((void *)(r_data->min))) >=
            sizeof(r_data->min))
          r_data->min_ptr = r_data->min;

        evas_object_size_hint_min_set(
          rect, r_data->min_ptr->w, r_data->min_ptr->h);

        fprintf(stdout, "Changing min. size hints for %s rect. to (%d, %d)\n",
                name, r_data->min_ptr->w, r_data->min_ptr->h);
        return;
     }

   if (strcmp(ev->keyname, "n") == 0) /* max. size hints */
     {
        (r_data->max_ptr)++;

        if ((unsigned)
            (((void *)(r_data->max_ptr)) - ((void *)(r_data->max))) >=
            sizeof(r_data->max))
          r_data->max_ptr = r_data->max;

        evas_object_size_hint_max_set(
          rect, r_data->max_ptr->w, r_data->max_ptr->h);

        fprintf(stdout, "Changing max. size hints for %s rect. to (%d, %d)\n",
                name, r_data->max_ptr->w, r_data->max_ptr->h);
        return;
     }

   if (strcmp(ev->keyname, "p") == 0) /* padding size hints */
     {
        (r_data->padding_ptr)++;

        if ((unsigned)
            (((void *)(r_data->padding_ptr)) - ((void *)(r_data->padding))) >=
            sizeof(r_data->padding))
          r_data->padding_ptr = r_data->padding;

        evas_object_size_hint_padding_set(
          rect, r_data->padding_ptr->l, r_data->padding_ptr->r,
          r_data->padding_ptr->t, r_data->padding_ptr->b);

        fprintf(stdout, "Changing padding size hints for %s rect."
                        " to (%d, %d, %d, %d)\n",
                name, r_data->padding_ptr->l, r_data->padding_ptr->r,
                r_data->padding_ptr->t, r_data->padding_ptr->b);
        return;
     }

   /* experiment with weights here. keep in mind that, for the box
    * object, only if all the children have non zero weights this hint
    * will have an effect */
   if (strcmp(ev->keyname, "w") == 0) /* weight hints */
     {
        (r_data->weight_ptr)++;

        if ((unsigned)
            (((void *)(r_data->weight_ptr)) - ((void *)(r_data->weight))) >=
            sizeof(r_data->weight))
          r_data->weight_ptr = r_data->weight;

        evas_object_size_hint_weight_set(
          rect, r_data->weight_ptr->x, r_data->weight_ptr->y);

        fprintf(stdout, "Changing weight hints for %s rect. to (%f, %f)\n",
                name, r_data->weight_ptr->x, r_data->weight_ptr->y);
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

   /* init values one is going to cycle through while running this
    * example */
   struct rect_data init_data = \
   {
      .min = {{0, 0}, {30, 30}, {100, 70}, {200, 200}},
      .max = {{0, 0}, {100, 100}, {100, 70}, {300, 300}},
      .align = {{0.0, 0.0}, {0.5, 0.5}, {1.0, 0.5}},
      .weight = {{0.0, 0.0}, {3, 6}, {10, 100}},
      .padding = {{0, 0, 0, 0}, {3, 6, 9, 12}, {10, 20, 0, 30}}
   };

   d.t_data = init_data;

   d.t_data.min_ptr = d.t_data.min + 1;
   d.t_data.max_ptr = d.t_data.max + 1;
   d.t_data.align_ptr = d.t_data.align;
   d.t_data.weight_ptr = d.t_data.weight;
   d.t_data.padding_ptr = d.t_data.padding;

   d.b_data = init_data;

   d.b_data.min_ptr = d.b_data.min + 1;
   d.b_data.max_ptr = d.b_data.max + 1;
   d.b_data.align_ptr = d.b_data.align;
   d.b_data.weight_ptr = d.b_data.weight;
   d.b_data.padding_ptr = d.b_data.padding;

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
   evas_object_color_set(d.bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(d.bg, 0, 0); /* at canvas' origin */
   evas_object_resize(d.bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(d.bg);

   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(
     d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

   /* Evas box with vertical layout */
   d.box = evas_object_box_add(d.canvas);
   evas_object_box_layout_set(
     d.box, evas_object_box_layout_vertical, NULL, NULL);
   evas_object_show(d.box);

   /* this is a border around the box, container of the rectangles we
    * are going to experiment with (changing some size hints). this
    * way you can see how the container relates to the children */
   d.border = evas_object_image_filled_add(d.canvas);
   evas_object_image_file_set(d.border, border_img_path, NULL);
   evas_object_image_border_set(d.border, 3, 3, 3, 3);
   evas_object_image_border_center_fill_set(d.border, EVAS_BORDER_FILL_NONE);
   evas_object_show(d.border);

   d.t_rect = evas_object_rectangle_add(d.canvas);
   evas_object_color_set(d.t_rect, 0, 0, 255, 255);

   evas_object_size_hint_min_set(
          d.t_rect, d.t_data.min_ptr->w, d.t_data.min_ptr->h);
   evas_object_show(d.t_rect);
   evas_object_box_append(d.box, d.t_rect);

   d.b_rect = evas_object_rectangle_add(d.canvas);
   evas_object_color_set(d.b_rect, 0, 255, 0, 255);

   evas_object_size_hint_min_set(
          d.b_rect, d.b_data.min_ptr->w, d.b_data.min_ptr->h);
   evas_object_show(d.b_rect);
   evas_object_box_append(d.box, d.b_rect);

   _canvas_resize_cb(d.ee);

   fprintf(stdout, commands);
   ecore_main_loop_begin();
   ecore_evas_shutdown();
   return 0;

error:
   fprintf(stderr, "You got to have at least one evas engine built and linked"
                   " up to ecore-evas for this example to run properly.\n");
   return -1;
}


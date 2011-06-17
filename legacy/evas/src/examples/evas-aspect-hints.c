/**
 * Simple Evas example illustrating aspect control hints on objects.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader also built. See stdout/stderr for
 * output.
 *
 * You'll also need @b Edje for this one, as it has the only smart
 * object implementing aspect control for children.
 *
 * @verbatim
 * gcc -o evas-events evas-events.c `pkg-config --libs --cflags ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H

#include "config.h"
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include <stdio.h>
#include <errno.h>

#define WIDTH         320
#define HEIGHT        480

static const char *border_img_path = PACKAGE_EXAMPLES_DIR "/red.png";
static const char *edje_file_path = PACKAGE_EXAMPLES_DIR "/aspect.edj";

struct test_data
{
   Ecore_Evas  *ee;
   Evas        *canvas;
   Evas_Object *bg, *rect, *container, *border;
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

static const char *
_get_aspect_name(Evas_Aspect_Control aspect)
{
   switch (aspect)
     {
      case 0:
        return "NONE";

      case 1:
        return "NEITHER";

      case 2:
        return "HORIZONTAL";

      case 3:
        return "VERTICAL";

      case 4:
        return "BOTH";

      default:
        return "INVALID";
     }
}

static void
_on_keydown(void        *data __UNUSED__,
            Evas        *evas __UNUSED__,
            Evas_Object *o,
            void        *einfo)
{
   const Evas_Modifier *mods;
   Evas_Event_Key_Down *ev = einfo;

   mods = evas_key_modifier_get(evas_object_evas_get(o));

   if (evas_key_modifier_is_set(mods, "Shift") &&
       strcmp(ev->keyname, "h") == 0) /* print help */
     {
        fprintf(stdout, "commands are:\n"
                        "\tc - cycle aspect control on object\n"
                        "\th - change horizontal aspect component\n"
                        "\tv - change vertical aspect component\n"
                        "\ts - print current object's status\n"
                        "\tH - print help\n");
        return;
     }

   if (strcmp(ev->keyname, "s") == 0) /* get aspect status of the obj */
     {
        Evas_Coord w, h;
        Evas_Aspect_Control aspect;

        evas_object_size_hint_aspect_get(d.rect, &aspect, &w, &h);

        fprintf(stdout, "Object has aspect %s, with horizontal compontent %d"
                        " and vertical compontent %d\n",
                _get_aspect_name(aspect), w, h);

        return;
     }

   if (strcmp(ev->keyname, "c") == 0) /* cycle aspect control on obj */
     {
        Evas_Coord w, h;
        Evas_Aspect_Control aspect;

        evas_object_size_hint_aspect_get(d.rect, &aspect, &w, &h);

        aspect = (aspect + 1) % 5;

        evas_object_size_hint_aspect_set(d.rect, aspect, w, h);

        fprintf(stdout, "Changing aspect control to %s\n",
                _get_aspect_name(aspect));

        return;
     }

   if (strcmp(ev->keyname, "h") == 0) /* change horizontal aspect component */
     {
        Evas_Coord w, h;
        Evas_Aspect_Control aspect;

        evas_object_size_hint_aspect_get(d.rect, &aspect, &w, &h);

        w = (w + 1) % 3;

        evas_object_size_hint_aspect_set(d.rect, aspect, w, h);

        fprintf(stdout, "Changing horizontal aspect component to %d\n", w);

        return;
     }

   if (strcmp(ev->keyname, "v") == 0) /* change vertical aspect component */
     {
        Evas_Coord w, h;
        Evas_Aspect_Control aspect;

        evas_object_size_hint_aspect_get(d.rect, &aspect, &w, &h);

        h = (h + 1) % 3;

        evas_object_size_hint_aspect_set(d.rect, aspect, w, h);

        fprintf(stdout, "Changing vertical aspect component to %d\n", h);

        return;
     }
}

int
main(void)
{
   Eina_Bool ret;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   if (!edje_init())
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
   evas_object_color_set(d.bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(d.bg, 0, 0); /* at canvas' origin */
   evas_object_resize(d.bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(d.bg);

   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(
     d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

   d.container = edje_object_add(d.canvas);
   ret = edje_object_file_set(d.container, edje_file_path, "main");
   if (!ret)
     {
        Edje_Load_Error err = edje_object_load_error_get(d.container);
        const char *msg = edje_load_error_str(err);
        fprintf(stderr, "could not load 'main' from %s: %s",
                edje_file_path, msg);

        goto panic;
     }

   evas_object_move(d.container, (WIDTH / 4), (HEIGHT / 4));
   evas_object_resize(d.container, (WIDTH / 2), (HEIGHT / 2));
   evas_object_show(d.container);

   d.rect = evas_object_rectangle_add(d.canvas);
   evas_object_color_set(d.rect, 0, 0, 255, 255);
   evas_object_size_hint_aspect_set(d.rect, EVAS_ASPECT_CONTROL_NONE, 1, 1);
   evas_object_show(d.rect);

   edje_object_part_swallow(d.container, "content", d.rect);
   evas_object_smart_changed(d.container);

   /* this is a border around the edje object, container of the
    * rectangle we are going to experiment with (change its aspect
    * hints). this way you can see how their sizes relate */
   d.border = evas_object_image_filled_add(d.canvas);
   evas_object_image_file_set(d.border, border_img_path, NULL);
   evas_object_image_border_set(d.border, 3, 3, 3, 3);
   evas_object_image_border_center_fill_set(d.border, EVAS_BORDER_FILL_NONE);
   evas_object_move(d.border, (WIDTH / 4) - 3, (HEIGHT / 4) - 3);
   evas_object_resize(d.border, (WIDTH / 2) + 6, (HEIGHT / 2) + 6);
   evas_object_show(d.border);

   ecore_main_loop_begin();

   ecore_evas_free(d.ee);
   ecore_evas_shutdown();
   edje_shutdown();
   return 0;

error:
   fprintf(stderr, "you got to have at least one evas engine built and linked"
                   " up to ecore-evas for this example to run properly.\n");
panic:
   ecore_evas_free(d.ee);
   ecore_evas_shutdown();
   edje_shutdown();

   return -1;
}

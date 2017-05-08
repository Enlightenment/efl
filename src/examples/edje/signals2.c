//Compile with:
// edje_cc signalsBubble.edc && gcc -o signals2 signals2.c `pkg-config --libs --cflags ecore ecore-evas edje`

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define EINA_UNUSED
#endif

#ifndef PACKAGE_DATA_DIR
#define PACKAGE_DATA_DIR "."
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include <stdio.h>

#define WIDTH     (700)
#define HEIGHT    (700)

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

/* mouse over signals */
static void
_on_mouse_over(void *data, Evas_Object *edje_obj,
               const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   Evas *evas;
   int x, y, mouseX, mouseY;

   evas = (Evas *) data;
   evas_object_geometry_get(edje_obj, &x, &y, NULL, NULL);

   evas_pointer_output_xy_get(evas, &mouseX, &mouseY);

   if ((rand() % 2) == 0)
     x += ((mouseX - x) + (x / 4 + mouseY / 2));
   else
     x -= ((mouseX - x) + (x / 4 + mouseY / 2));

   if ((rand() % 2) == 0)
     y += ((mouseY - y) + (y / 4 + mouseX / 2));
   else
     y -= ((mouseY - y) + (y / 4 + mouseX / 2));

   if (x > WIDTH)
     x = WIDTH;
   else if (x < 0) x = 0;

   if (y > HEIGHT)
     y = HEIGHT;
   else if (y < 0) y = 0;

   printf("Moving object to - (%d,%d)\n", x, y);

   evas_object_move(edje_obj, x, y);
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   const char *edje_file = PACKAGE_DATA_DIR"/signalsBubble.edj";
   Ecore_Evas *ee;
   Evas *evas;
   Evas_Object *bg;
   Evas_Object *edje_obj;

   if (!ecore_evas_init()) return EXIT_FAILURE;

   if (!edje_init()) goto shutdown_ecore_evas;

   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);

   if (!ee) goto shutdown_edje;

   ecore_evas_callback_delete_request_set(ee, _on_delete);
   ecore_evas_title_set(ee, "Edje animations and signals");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255); //White
   evas_object_move(bg, 0, 0); //orign
   evas_object_resize(bg, WIDTH, HEIGHT); //cover the window
   evas_object_show(bg);

   ecore_evas_object_associate(ee, bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);
   evas_object_focus_set(bg, EINA_TRUE);

   edje_obj = edje_object_add(evas);

   if (!edje_object_file_set(edje_obj, edje_file, "image_group"))
     {
        int err = edje_object_load_error_get(edje_obj);
        const char *errmsg = edje_load_error_str(err);
        fprintf(stderr, "Could not load the edje file - reason:%s\n", errmsg);
        goto shutdown_edje;
     }

   edje_object_signal_callback_add(edje_obj, "mouse,move", "part_image",
                                   _on_mouse_over, evas);
   evas_object_resize(edje_obj, 63, 63);
   evas_object_move(edje_obj, 50, 50);
   evas_object_show(edje_obj);

   ecore_evas_show(ee);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   edje_shutdown();
   ecore_evas_shutdown();

   return EXIT_SUCCESS;

   shutdown_edje: edje_shutdown();

   shutdown_ecore_evas: ecore_evas_shutdown();

   return EXIT_FAILURE;
}

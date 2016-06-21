/**
 * Simple Edje example illustrating svg part usuage in edc.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc svg.edc && gcc -o svg-test svg-test.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EINA_UNUSED
#endif

#ifndef PACKAGE_DATA_DIR
#define PACKAGE_DATA_DIR "."
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  800
#define HEIGHT 800

static void
_on_destroy(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

/* here just to keep our example's window size and background image's
 * size in synchrony */
static void
_on_canvas_resize(Ecore_Evas *ee)
{
   Evas_Object *bg;
   Evas_Object *edje_obj;
   int          w;
   int          h;

   bg = ecore_evas_data_get(ee, "background");
   edje_obj = ecore_evas_data_get(ee, "edje_obj");
   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(bg, w, h);
   evas_object_resize(edje_obj, w, h);
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   const char  *edje_file = "./svg.edj";
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
   Evas_Object *edje_obj;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   if (!edje_init())
     goto shutdown_ecore_evas;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee) goto shutdown_edje;

   ecore_evas_callback_destroy_set(ee, _on_destroy);
   ecore_evas_callback_resize_set(ee, _on_canvas_resize);
   ecore_evas_title_set(ee, "SVG Example");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 0, 0, 0, 255);
   evas_object_resize(bg, WIDTH, HEIGHT);
   evas_object_focus_set(bg, EINA_TRUE);
   evas_object_show(bg);
   ecore_evas_data_set(ee, "background", bg);


   edje_obj = edje_object_add(evas);

   edje_object_file_set(edje_obj, edje_file, "svg-test");
   evas_object_move(edje_obj, 0, 0);
   evas_object_resize(edje_obj, WIDTH, HEIGHT);
   evas_object_show(edje_obj);
   ecore_evas_data_set(ee, "edje_obj", edje_obj);


   ecore_evas_show(ee);

   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();
   edje_shutdown();

   return EXIT_SUCCESS;

 shutdown_edje:
   edje_shutdown();
 shutdown_ecore_evas:
   ecore_evas_shutdown();

   return EXIT_FAILURE;
}

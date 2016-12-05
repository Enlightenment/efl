/**
 * Edje example demonstrating how to use multiseat features following
 * a dynamic approach.
 *
 * It's an example of how themes can be implemented even if the number
 * of seats using the UI is unknown.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one) that supports multiseat. It may be wayland or
 * X11 with VNC support. Using other engines will lead you to a
 * situation where all seats are reported as the same one ("default").
 *
 * @verbatim
 * edje_cc dynamic-multiseat.edc && gcc -o edje-dynamic-multiseat edje-dynamic-multiseat.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
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

#define WIDTH 400
#define HEIGHT 400

static const char *GROUPNAME = "example/main";
static const char *EDJE_FILE = PACKAGE_DATA_DIR"/dynamic_multiseat.edj";

static void
_on_destroy(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_on_canvas_resize(Ecore_Evas *ee)
{
   Evas_Object *bg;
   Evas_Object *edje_obj;
   int w, h;

   bg = ecore_evas_data_get(ee, "background");
   edje_obj = ecore_evas_data_get(ee, "edje_obj");

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(bg, w, h);
   evas_object_resize(edje_obj, w, h);
}

static void
_edje_seat_cb(void *data EINA_UNUSED, Evas_Object *o EINA_UNUSED, const char *emission, const char *source EINA_UNUSED)
{
   const char *sig;

   sig = emission + strlen("seat,");
   printf("Device %s\n", sig);
}

static void
_print_msg_cb(void *data EINA_UNUSED, Evas_Object *o EINA_UNUSED, const char *emission, const char *source EINA_UNUSED)
{
   printf("Info: %s\n", emission);
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   Evas_Object *edje_obj, *bg;
   Ecore_Evas *ee;
   Evas *evas;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   if (!edje_init())
     goto shutdown_ecore_evas;

   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee) goto shutdown_edje;

   ecore_evas_callback_destroy_set(ee, _on_destroy);
   ecore_evas_callback_resize_set(ee, _on_canvas_resize);
   ecore_evas_title_set(ee, "Edje Dynamic Multiseat Example");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255);
   evas_object_resize(bg, WIDTH, HEIGHT);
   evas_object_show(bg);
   ecore_evas_data_set(ee, "background", bg);

   edje_obj = edje_object_add(evas);

   if (!edje_object_file_set(edje_obj, EDJE_FILE, GROUPNAME))
     printf("failed to set file %s.\n", EDJE_FILE);

   evas_object_move(edje_obj, 0, 0);
   evas_object_resize(edje_obj, WIDTH, HEIGHT);
   evas_object_show(edje_obj);
   ecore_evas_data_set(ee, "edje_obj", edje_obj);

   edje_object_signal_callback_add(edje_obj, "seat,*", "",
                                   _edje_seat_cb, NULL);
   edje_object_signal_callback_add(edje_obj, "*", "theme",
                                   _print_msg_cb, NULL);

   printf("Running example on evas engine %s\n",
          ecore_evas_engine_name_get(ee));

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

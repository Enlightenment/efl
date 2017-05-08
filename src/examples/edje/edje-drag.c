/**
 * Simple Edje example illustrating drag functions.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc drag.edc && gcc -o edje-drag edje-drag.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
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

#define WIDTH  300
#define HEIGHT 300

static const char commands[] = \
  "commands are:\n"
  "\tDown - set drag step to 1\n"
  "\tUp - set drag step to -1\n"
  "\tm - set drag value to 0.5\n"
  "\tPrior - set drag page to -1\n"
  "\tNext - set drag page to -1\n"
  "\tEsc - exit\n"
  "\th - print help\n";

static const char *PARTNAME = "example/knob";

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

static void
_on_bg_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info)
{
   Ecore_Evas          *ee;
   Evas_Event_Key_Down *ev;
   Evas_Object         *edje_obj;

   ee = (Ecore_Evas *)data;
   ev = (Evas_Event_Key_Down *)event_info;
   edje_obj = ecore_evas_data_get(ee, "edje_obj");

   if (!strcmp(ev->key, "h"))
     {
        printf(commands);
        return;
     }
   else if (!strcmp(ev->key, "Down"))
     {
	edje_object_part_drag_step(edje_obj, PARTNAME, 0, 1.0);
     }
   else if (!strcmp(ev->key, "Up"))
     {
	edje_object_part_drag_step(edje_obj, PARTNAME, 0, -1.0);
     }
   else if (!strcmp(ev->key, "m"))
     {
	edje_object_part_drag_value_set(edje_obj, PARTNAME, 0.0, 0.5);
     }
   else if (!strcmp(ev->key, "Prior"))
     {
	edje_object_part_drag_page(edje_obj, PARTNAME, 0.0, -1.0);
     }
   else if (!strcmp(ev->key, "Next"))
     {
	edje_object_part_drag_page(edje_obj, PARTNAME, 0.0, 1.0);
     }
   else if (!strcmp(ev->key, "Escape"))
     ecore_main_loop_quit();
   else
     {
        printf("unhandled key: %s\n", ev->key);
        printf(commands);
     }
}

static void
_on_knob_moved(void *data EINA_UNUSED, Evas_Object *o, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   double val;

   edje_object_part_drag_value_get(o, PARTNAME, NULL, &val);
   printf("value changed to: %0.3f\n", val);
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   const char  *edje_file = PACKAGE_DATA_DIR"/drag.edj";
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
   ecore_evas_title_set(ee, "Edje Drag Example");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255);
   evas_object_resize(bg, WIDTH, HEIGHT);
   evas_object_focus_set(bg, EINA_TRUE);
   evas_object_show(bg);
   ecore_evas_data_set(ee, "background", bg);

   evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_DOWN, _on_bg_key_down, ee);

   edje_obj = edje_object_add(evas);

   if (!edje_object_file_set(edje_obj, edje_file, "example/group"))
     printf("failed to set file %s.\n", edje_file);

   evas_object_move(edje_obj, 0, 0);
   evas_object_resize(edje_obj, WIDTH, HEIGHT);
   evas_object_show(edje_obj);
   ecore_evas_data_set(ee, "edje_obj", edje_obj);

   edje_object_part_drag_size_set(edje_obj, PARTNAME, 1.0, 0.4);

   if (!edje_object_part_drag_step_set(edje_obj, PARTNAME, 0.0, 0.1))
     printf("error when setting drag step size.\n");

   if (!edje_object_part_drag_page_set(edje_obj, PARTNAME, 0.0, 0.3))
     printf("error when setting drag page step size.\n");

   edje_object_signal_callback_add(edje_obj, "drag", PARTNAME, _on_knob_moved, NULL);

   printf(commands);

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

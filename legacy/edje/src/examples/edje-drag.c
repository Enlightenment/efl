/**
 * Simple Edje example illustrating drag functions.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc drag.edc && gcc -o drag-box drag-box.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define __UNUSED__
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  300
#define HEIGHT 300

#define RECTW 30
#define RECTH 30

#define NRECTS 20

static const char commands[] = \
  "commands are:\n"
  "\tDdown - set drag step to 1\n"
  "\tUp - set drag step to -1\n"
  "\tm - set drag value to 0.5\n"
  "\tPrior - set drag page to -1\n"
  "\tNext - set drag page to -1\n"
  "\tEsc - exit\n"
  "\th - print help\n";

static const char *PARTNAME = "example/knob";

static void
_on_destroy(Ecore_Evas *ee __UNUSED__)
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
_on_bg_key_down(void *data, Evas *e __UNUSED__, Evas_Object *o __UNUSED__, void *event_info)
{
   Ecore_Evas          *ee;
   Evas_Event_Key_Down *ev;
   Evas_Object         *edje_obj;

   ee = (Ecore_Evas *)data;
   ev = (Evas_Event_Key_Down *)event_info;
   edje_obj = ecore_evas_data_get(ee, "edje_obj");

   if (!strcmp(ev->keyname, "h"))
     {
        fprintf(stdout, commands);
        return;
     }
   else if (!strcmp(ev->keyname, "Down"))
     {
	edje_object_part_drag_step(edje_obj, PARTNAME, 0, 1.0);
     }
   else if (!strcmp(ev->keyname, "Up"))
     {
	edje_object_part_drag_step(edje_obj, PARTNAME, 0, -1.0);
     }
   else if (!strcmp(ev->keyname, "m"))
     {
	edje_object_part_drag_value_set(edje_obj, PARTNAME, 0.0, 0.5);
     }
   else if (!strcmp(ev->keyname, "Prior"))
     {
	edje_object_part_drag_page(edje_obj, PARTNAME, 0.0, -1.0);
     }
   else if (!strcmp(ev->keyname, "Next"))
     {
	edje_object_part_drag_page(edje_obj, PARTNAME, 0.0, 1.0);
     }
   else if (!strcmp(ev->keyname, "Escape"))
     ecore_main_loop_quit();
   else
     {
        printf("unhandled key: %s\n", ev->keyname);
        fprintf(stdout, commands);
     }
}

static void
_on_knob_moved(void *data __UNUSED__, Evas_Object *o, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   double val;

   edje_object_part_drag_value_get(o, PARTNAME, NULL, &val);
   printf("value changed to: %0.3f\n", val);
}

int
main(int argc __UNUSED__, char *argv[])
{
   char         edje_file_path[PATH_MAX];
   const char  *edje_file = "drag.edj";
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
   Evas_Object *edje_obj;
   Eina_Prefix *pfx;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   if (!edje_init())
     goto shutdown_ecore_evas;

   pfx = eina_prefix_new(argv[0], main,
                         "EDJE_EXAMPLES",
                         "edje/examples",
                         edje_file,
                         PACKAGE_BIN_DIR,
                         PACKAGE_LIB_DIR,
                         PACKAGE_DATA_DIR,
                         PACKAGE_DATA_DIR);
   if (!pfx)
     goto shutdown_edje;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee)
     goto free_prefix;

   ecore_evas_callback_destroy_set(ee, _on_destroy);
   ecore_evas_callback_resize_set(ee, _on_canvas_resize);
   ecore_evas_title_set(ee, "Edje Box Example");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255);
   evas_object_resize(bg, WIDTH, HEIGHT);
   evas_object_focus_set(bg, EINA_TRUE);
   evas_object_show(bg);
   ecore_evas_data_set(ee, "background", bg);

   evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_DOWN, _on_bg_key_down, ee);

   edje_obj = edje_object_add(evas);

   snprintf(edje_file_path, sizeof(edje_file_path),
            "%s/examples/%s", eina_prefix_data_get(pfx), edje_file);
   edje_object_file_set(edje_obj, edje_file_path, "example/group");
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

   fprintf(stdout, commands);

   ecore_evas_show(ee);

   ecore_main_loop_begin();

   eina_prefix_free(pfx);
   ecore_evas_free(ee);
   ecore_evas_shutdown();
   edje_shutdown();

   return EXIT_SUCCESS;

 free_prefix:
   eina_prefix_free(pfx);
 shutdown_edje:
   edje_shutdown();
 shutdown_ecore_evas:
   ecore_evas_shutdown();

   return EXIT_FAILURE;
}

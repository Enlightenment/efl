/**
 * Edje example demonstrating how to deal with part and objects focus.
 *
 * @verbatim
 * edje_cc focus.edc && gcc -o edje-focus edje-focus.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
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
static const char *EDJE_FILE = PACKAGE_DATA_DIR"/focus.edj";

static void
_on_destroy(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_on_canvas_resize(Ecore_Evas *ee)
{
   Evas_Object *edje_obj;
   int w, h;

   edje_obj = ecore_evas_data_get(ee, "edje_obj");

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(edje_obj, w, h);
}

static void
_focus_part_in_cb(void *data EINA_UNUSED, Evas_Object *o EINA_UNUSED,
                  const char *emission EINA_UNUSED, const char *source)
{
   printf("Focus set to part %s\n", source);
}

static void
_focus_part_out_cb(void *data EINA_UNUSED, Evas_Object *o EINA_UNUSED,
                  const char *emission EINA_UNUSED, const char *source)
{
   printf("Focus unset to part %s\n", source);
}

static void
_focus_obj_in_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Focus *ev = event->info;
   Evas_Object *rect = event->object;
   /* it always will print the default seat name, since FOCUS_OBJECT
      isn't setting a specific seat */
   printf("Focus set to object %s (seat %s)\n", evas_object_name_get(rect),
          efl_name_get(efl_input_device_get(ev)));
}

static void
_focus_obj_out_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Focus *ev = event->info;
   Evas_Object *rect = event->object;
   printf("Focus unset to object %s (seat %s)\n", evas_object_name_get(rect),
          efl_name_get(efl_input_device_get(ev)));
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   Evas_Object *edje_obj, *rect_left, *rect_right;
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
   ecore_evas_title_set(ee, "Edje Focus Example");

   evas = ecore_evas_get(ee);

   edje_obj = edje_object_add(evas);

   if (!edje_object_file_set(edje_obj, EDJE_FILE, GROUPNAME))
     printf("failed to set file %s.\n", EDJE_FILE);

   evas_object_move(edje_obj, 0, 0);
   evas_object_resize(edje_obj, WIDTH, HEIGHT);
   evas_object_show(edje_obj);
   ecore_evas_data_set(ee, "edje_obj", edje_obj);

   edje_object_signal_callback_add(edje_obj, "focus,part,in", "*",
                                   _focus_part_in_cb, NULL);
   edje_object_signal_callback_add(edje_obj, "focus,part,out", "*",
                                   _focus_part_out_cb, NULL);

   rect_left = evas_object_rectangle_add(evas);
   evas_object_name_set(rect_left, "rect left");
   evas_object_color_set(rect_left, 200, 200, 100, 255);
   edje_object_part_swallow(edje_obj, "button,3", rect_left);

   efl_event_callback_add(rect_left, EFL_EVENT_FOCUS_IN,
                          _focus_obj_in_cb, NULL);
   efl_event_callback_add(rect_left, EFL_EVENT_FOCUS_OUT,
                          _focus_obj_out_cb, NULL);

   rect_right = evas_object_rectangle_add(evas);
   evas_object_name_set(rect_right, "rect right");
   evas_object_color_set(rect_right, 100, 200, 200, 255);
   edje_object_part_swallow(edje_obj, "button,4", rect_right);

   efl_event_callback_add(rect_right, EFL_EVENT_FOCUS_IN,
                          _focus_obj_in_cb, NULL);
   efl_event_callback_add(rect_right, EFL_EVENT_FOCUS_OUT,
                          _focus_obj_out_cb, NULL);

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

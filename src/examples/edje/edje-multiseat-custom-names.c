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
static const char *EDJE_FILE = PACKAGE_DATA_DIR"/multiseat_custom_names.edj";

static Efl_Input_Device *default_seat = NULL;
static Efl_Input_Device *secondary_seat = NULL;

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
_device_rename(Efl_Input_Device *dev)
{
   if (!default_seat) {
       default_seat = dev;
       efl_name_set(dev, "default");
       return;
   }

   if (!secondary_seat) {
       secondary_seat = dev;
       efl_name_set(dev, "secondary");
   }
}

static void
_device_added(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Device *dev = event->info;

   if (efl_input_device_type_get(dev) != EFL_INPUT_DEVICE_TYPE_SEAT)
     return;
   _device_rename(dev);
}

static void
_device_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Device *dev = event->info;

   if (dev == default_seat)
     efl_name_set(dev, "default");
   else if (dev == secondary_seat)
     efl_name_set(dev, "secondary");
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   const Eina_List *devices, *l;
   Efl_Input_Device *dev;
   Evas_Object *edje_obj;
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

   devices = evas_device_list(evas, NULL);
   EINA_LIST_FOREACH(devices, l, dev)
     {
        if (efl_input_device_type_get(dev) == EFL_INPUT_DEVICE_TYPE_SEAT)
          _device_rename(dev);
     }
   efl_event_callback_add(evas, EFL_CANVAS_SCENE_EVENT_DEVICE_ADDED,
                          _device_added, NULL);
   efl_event_callback_add(evas, EFL_CANVAS_SCENE_EVENT_DEVICE_CHANGED,
                          _device_changed, NULL);

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

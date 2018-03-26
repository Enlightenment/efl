/**
 * Edje example demonstrating how to use multiseat features.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one) that supports multiseat. It may be wayland or
 * X11 with VNC support. Using other engines will lead you to a
 * situation where all seats are reported as the same one ("default").
 *
 * @verbatim
 * edje_cc multiseat.edc && gcc -o edje-multiseat edje-multiseat.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
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

#define EDJE_EDIT_IS_UNSTABLE_AND_I_KNOW_ABOUT_IT
#include <Edje_Edit.h>

#define WIDTH  400
#define HEIGHT 400

static const char *GROUPNAME = "example/main";
static const char *PARTNAME_KNOB1 = "example/knob1";
static const char *PARTNAME_KNOB2 = "example/knob2";
static const char *EDJE_FILE = PACKAGE_DATA_DIR"/multiseat.edj";

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
_on_rect_focus_in(void *data, const Efl_Event *event)
{
   Evas_Object *rect, *edje_obj;
   Efl_Input_Device *seat;
   Eina_Stringshare *name;
   Efl_Input_Focus *ev;

   edje_obj = data;
   rect = event->object;
   ev = event->info;
   seat = efl_input_device_get(ev);
   name = efl_canvas_layout_seat_name_get(edje_obj, seat);

   printf("Seat %s (%s) focused the rect object\n",
          efl_name_get(seat), name);

   if (!strcmp(name, "seat1"))
     evas_object_color_set(rect, 200, 0, 0, 255);
   else if (!strcmp(name, "seat2"))
     evas_object_color_set(rect, 0, 200, 0, 255);
   else
     printf("Unexpected seat %s - no color change\n", name);
}

static void
_on_rect_focus_out(void *data, const Efl_Event *event)
{
   Evas_Object *rect, *edje_obj;
   Efl_Input_Device *seat;
   Eina_Stringshare *name;
   Efl_Input_Focus *ev;

   edje_obj = data;
   rect = event->object;
   ev = event->info;
   seat = efl_input_device_get(ev);
   name = efl_canvas_layout_seat_name_get(edje_obj, seat);

   printf("Seat %s (%s) unfocused the rect object\n",
          efl_name_get(seat), name);
   evas_object_color_set(rect, 200, 200, 200, 255);

   efl_canvas_object_seat_focus_add(edje_obj, seat);
}

static void
_on_key_down(void *data, Evas *e EINA_UNUSED, Evas_Object *o, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   const Evas_Modifier *mods;
   Efl_Input_Device *seat;
   Evas *evas = data;

   seat = efl_input_device_seat_get(ev->dev);
   printf("Seat %s (%s) pressed key %s\n", efl_name_get(seat),
          efl_canvas_layout_seat_name_get(o, seat), ev->key);

   mods = evas_key_modifier_get(evas);
   if (!strcmp(ev->key, "p") &&
       evas_seat_key_modifier_is_set(mods, "Control", seat))
     {
        Evas_Object *edje_edit_obj;

        edje_edit_obj = edje_edit_object_add(evas);
        if (!edje_object_file_set(edje_edit_obj, EDJE_FILE, GROUPNAME))
          printf("failed to set file %s.\n", EDJE_FILE);

        printf("EDC source code:\n%s\n",
               edje_edit_source_generate(edje_edit_obj));
        evas_object_del(edje_edit_obj);
     }
}

static void
_on_drag_started(void *data EINA_UNUSED, Evas_Object *o, const char *emission, const char *source)
{
   Eina_Stringshare *seat_name;
   const char *seat_name_str;
   Efl_Input_Device *seat;

   seat_name_str = emission + strlen("drag,start,");
   seat_name = eina_stringshare_add(seat_name_str);
   seat = efl_canvas_layout_seat_get(o, seat_name);
   printf("Seat %s (%s) started drag %s\n", efl_name_get(seat),
           seat_name, source);
   eina_stringshare_del(seat_name);
}

static void
_setup_drag(Evas_Object *edje_obj, const char *partname)
{
   if (!edje_object_part_drag_size_set(edje_obj, partname, 1.0, 0.4))
     printf("error when setting drag size.\n");

   if (!edje_object_part_drag_step_set(edje_obj, partname, 0.0, 0.1))
     printf("error when setting drag step size.\n");

   if (!edje_object_part_drag_page_set(edje_obj, partname, 0.0, 0.3))
     printf("error when setting drag page step size.\n");

   edje_object_signal_callback_add(edje_obj, "drag,start,*", partname,
                                   _on_drag_started, NULL);
}

static void
_edje_load_cb(void *data EINA_UNUSED, Evas_Object *o EINA_UNUSED, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   printf("Edje loaded\n");
}

static void
_edje_seat_cb(void *data EINA_UNUSED, Evas_Object *o EINA_UNUSED, const char *emission, const char *source EINA_UNUSED)
{
   const char *sig;

   sig = emission + strlen("seat,");
   printf("Device %s\n", sig);
}

static void
_device_added(void *data, const Efl_Event *event)
{
   Efl_Input_Device *dev = event->info;
   Evas_Object *edje_obj = data;

   if (efl_input_device_type_get(dev) != EFL_INPUT_DEVICE_TYPE_SEAT)
     return;

   efl_canvas_object_seat_focus_add(edje_obj, dev);
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   const Eina_List *devices, *l;
   Evas_Object *edje_obj, *bg, *rect;
   Efl_Input_Device *dev;
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
   ecore_evas_title_set(ee, "Edje Multiseat Example");

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

   _setup_drag(edje_obj, PARTNAME_KNOB1);
   _setup_drag(edje_obj, PARTNAME_KNOB2);

   edje_object_part_text_set(edje_obj, "example/text1", "Type here : ");
   edje_object_part_text_cursor_end_set(edje_obj, "example/text1",
                                        EDJE_CURSOR_MAIN);

   edje_object_part_text_set(edje_obj, "example/text2", "Or maybe here : ");
   edje_object_part_text_cursor_end_set(edje_obj, "example/text2",
                                        EDJE_CURSOR_MAIN);

   rect = evas_object_rectangle_add(evas);
   evas_object_color_set(rect, 200, 200, 200, 255);
   edje_object_part_swallow(edje_obj, "example/swallow", rect);
   efl_event_callback_add(rect, EFL_EVENT_FOCUS_IN,
                          _on_rect_focus_in, edje_obj);
   efl_event_callback_add(rect, EFL_EVENT_FOCUS_OUT,
                          _on_rect_focus_out, edje_obj);

   devices = evas_device_list(evas, NULL);
   EINA_LIST_FOREACH(devices, l, dev)
     {
        if (efl_input_device_type_get(dev) == EFL_INPUT_DEVICE_TYPE_SEAT)
          efl_canvas_object_seat_focus_add(edje_obj, dev);

     }
   efl_event_callback_add(evas, EFL_CANVAS_SCENE_EVENT_DEVICE_ADDED,
                          _device_added, edje_obj);
   evas_object_event_callback_add(edje_obj, EVAS_CALLBACK_KEY_DOWN,
                                  _on_key_down, evas);
   edje_object_signal_callback_add(edje_obj, "seat,*", "",
                                   _edje_seat_cb, NULL);
   edje_object_signal_callback_add(edje_obj, "load", "",
                                   _edje_load_cb, NULL);

   printf("Running example on evas engine %s\n",
          ecore_evas_engine_name_get(ee));
   printf("Press 'Ctrl + p' to print EDC source code\n");

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

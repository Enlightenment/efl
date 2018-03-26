/**
 * Example of handling events with multiseat information in Evas.
 *
 * Illustrates handling of:
 *    * mouse events
 *    * keyboard events
 *    * focus events
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader also built. See stdout/stderr for
 * output.
 *
 * @verbatim
 * gcc -o evas_multiseat_events evas-multiseat-events.c `pkg-config --libs --cflags evas ecore ecore-evas`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Efl.h>
#include <stdio.h>

#define WIDTH  (400)
#define HEIGHT (200)

static void
_canvas_focus_in_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Device *seat;
   Efl_Input_Focus *ev;
   Evas_Object *focused;

   ev = event->info;
   seat = efl_input_device_get(ev);
   focused = efl_input_focus_object_get(ev);

   printf("Object %s was focused by seat %s\n",
          evas_object_name_get(focused),
          efl_name_get(seat));
}

static void
_hold_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Device *seat;
   Efl_Input_Hold *ev;

   ev = event->info;
   seat = efl_input_device_seat_get(efl_input_device_get(ev));

   printf("Hold %s at object %s from seat %s\n",
          efl_input_hold_get(ev) ? "on" : "off",
          evas_object_name_get(event->object),
          efl_name_get(seat));
}

static void
_focus_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Device *seat;
   Efl_Input_Focus *ev;

   ev = event->info;
   seat = efl_input_device_get(ev);

   printf("Focus %s at object %s from seat %s\n",
          event->desc == EFL_EVENT_FOCUS_IN ? "in" : "out",
          evas_object_name_get(event->object),
          efl_name_get(seat));
}

static void
_pointer_in_out_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Pointer *ev;
   Efl_Input_Device *seat;

   ev = event->info;
   seat = efl_input_device_seat_get(efl_input_device_get(ev));

   printf("Pointer %s at object %s from seat %s\n",
          event->desc == EFL_EVENT_POINTER_IN ? "in" : "out",
          evas_object_name_get(event->object),
          efl_name_get(seat));
}

static void
_pointer_down_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Pointer *ev;
   Efl_Input_Device *seat;
   Efl_Pointer_Flags pointer_flags;
   char buf[256];

   ev = event->info;
   seat = efl_input_device_seat_get(efl_input_device_get(ev));
   pointer_flags = efl_input_pointer_button_flags_get(ev);

   if (pointer_flags & EFL_POINTER_FLAGS_TRIPLE_CLICK)
     snprintf(buf, sizeof(buf), "Triple click with button %i",
              efl_input_pointer_button_get(ev));
   else if (pointer_flags & EFL_POINTER_FLAGS_DOUBLE_CLICK)
     snprintf(buf, sizeof(buf), "Double click with button %i",
              efl_input_pointer_button_get(ev));
   else
     {
        snprintf(buf, sizeof(buf), "Pointer button %i down",
                 efl_input_pointer_button_get(ev));
        efl_canvas_object_seat_focus_add(event->object, seat);
     }

   printf("%s at object %s from seat %s\n", buf,
          evas_object_name_get(event->object),
          efl_name_get(seat));
}

static void
_pointer_up_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Pointer *ev;
   Efl_Input_Device *seat;

   ev = event->info;
   seat = efl_input_device_seat_get(efl_input_device_get(ev));

   printf("Pointer button %i up at object %s from seat %s\n",
          efl_input_pointer_button_get(ev),
          evas_object_name_get(event->object),
          efl_name_get(seat));
}

static void
_pointer_move_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Pointer *ev;
   Efl_Input_Device *seat;

   ev = event->info;
   seat = efl_input_device_seat_get(efl_input_device_get(ev));

   printf("Pointer moved to %1.f,%1.f at object %s from seat %s\n",
          efl_input_pointer_value_get(ev, EFL_INPUT_VALUE_X),
          efl_input_pointer_value_get(ev, EFL_INPUT_VALUE_Y),
          evas_object_name_get(event->object),
          efl_name_get(seat));
}

static void
_pointer_wheel_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Pointer *ev;
   Efl_Input_Device *seat;

   ev = event->info;
   seat = efl_input_device_seat_get(efl_input_device_get(ev));

   printf("Wheel: '%i,%i' on object %s from seat %s\n",
          efl_input_pointer_wheel_horizontal_get(ev),
          efl_input_pointer_wheel_delta_get(ev),
          evas_object_name_get(event->object),
          efl_name_get(seat));
}

static void
_key_down_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   const Evas_Modifier *mods;
   Efl_Input_Device *seat;
   Efl_Input_Key *ev;

   ev = event->info;
   mods = evas_key_modifier_get(evas_object_evas_get(event->object));
   seat = efl_input_device_seat_get(efl_input_device_get(ev));

   printf("Key down: '%s' on object %s from seat %s\n",
          efl_input_key_name_get(ev),
          evas_object_name_get(event->object),
          efl_name_get(seat));

   if (evas_seat_key_modifier_is_set(mods, "Control", seat))
     printf("Ctrl is pressed by seat %s\n",
            efl_name_get(seat));
}

static void
_key_up_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Key *ev;
   Efl_Input_Device *seat;

   ev = event->info;
   seat = efl_input_device_seat_get(efl_input_device_get(ev));

   printf("Key up: '%s' on object %s from seat %s\n",
          efl_input_key_name_get(ev),
          evas_object_name_get(event->object),
          efl_name_get(seat));
}

static void
_dev_added_or_removed(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Device *dev = event->info;

   if (efl_input_device_type_get(dev) != EFL_INPUT_DEVICE_TYPE_SEAT)
     return;

   printf("The seat '%s' - description: '%s' was '%s'\n",
          efl_name_get(dev),
          efl_comment_get(dev),
          event->desc == EFL_CANVAS_SCENE_EVENT_DEVICE_ADDED ? "added" : "removed");
}

EFL_CALLBACKS_ARRAY_DEFINE(canvas_callbacks,
                           { EFL_CANVAS_SCENE_EVENT_OBJECT_FOCUS_IN,
                           _canvas_focus_in_cb },
                           { EFL_CANVAS_SCENE_EVENT_DEVICE_ADDED,
                           _dev_added_or_removed },
                           { EFL_CANVAS_SCENE_EVENT_DEVICE_REMOVED,
                           _dev_added_or_removed });

EFL_CALLBACKS_ARRAY_DEFINE(callbacks,
                           { EFL_EVENT_FOCUS_IN, _focus_cb },
                           { EFL_EVENT_FOCUS_OUT, _focus_cb },
                           { EFL_EVENT_KEY_DOWN, _key_down_cb },
                           { EFL_EVENT_KEY_UP, _key_up_cb },
                           { EFL_EVENT_HOLD, _hold_cb },
                           { EFL_EVENT_POINTER_IN, _pointer_in_out_cb },
                           { EFL_EVENT_POINTER_OUT, _pointer_in_out_cb },
                           { EFL_EVENT_POINTER_DOWN, _pointer_down_cb },
                           { EFL_EVENT_POINTER_UP, _pointer_up_cb },
                           { EFL_EVENT_POINTER_MOVE, _pointer_move_cb },
                           { EFL_EVENT_POINTER_WHEEL, _pointer_wheel_cb });

int
main(void)
{
   Evas_Object *bg, *red_rect, *blue_rect;
   const Eina_List *devices, *l;
   Efl_Input_Device *dev;
   Ecore_Evas *ee;
   Evas *canvas;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee)
     goto error;

   canvas = ecore_evas_get(ee);
   efl_event_callback_array_add(canvas, canvas_callbacks(), NULL);

   bg = evas_object_rectangle_add(canvas);
   evas_object_name_set(bg, "background");
   evas_object_color_set(bg, 255, 255, 255, 255);
   evas_object_move(bg, 0, 0);
   evas_object_resize(bg, WIDTH, HEIGHT);
   evas_object_show(bg);

   red_rect = evas_object_rectangle_add(canvas);
   evas_object_name_set(red_rect, "red rect");
   evas_object_color_set(red_rect, 255, 0, 0, 255);
   evas_object_move(red_rect, WIDTH/4, HEIGHT/4);
   evas_object_resize(red_rect, WIDTH/4 - 10, HEIGHT/2);
   evas_object_show(red_rect);
   efl_event_callback_array_add(red_rect, callbacks(), NULL);

   blue_rect = evas_object_rectangle_add(canvas);
   evas_object_name_set(blue_rect, "blue rect");
   evas_object_color_set(blue_rect, 0, 0, 255, 255);
   evas_object_move(blue_rect, WIDTH/2 + 10, HEIGHT/4);
   evas_object_resize(blue_rect, WIDTH/4 - 10, HEIGHT/2);
   evas_object_show(blue_rect);
   efl_event_callback_array_add(blue_rect, callbacks(), NULL);

   devices = evas_device_list(canvas, NULL);
   EINA_LIST_FOREACH(devices, l, dev)
     {
        if (efl_input_device_type_get(dev) == EFL_INPUT_DEVICE_TYPE_SEAT)
          printf("The seat '%s' - description: '%s' was 'added'\n",
                 efl_name_get(dev),
                 efl_comment_get(dev));
     }

   ecore_evas_show(ee);
   ecore_main_loop_begin();

   ecore_evas_free(ee);
   ecore_evas_shutdown();

   return 0;

error:
   fprintf(stderr, "At least one evas engine built and linked up to ecore evas "
                   "is required for this example to run properly.\n");
   ecore_evas_shutdown();
   return EXIT_FAILURE;
}

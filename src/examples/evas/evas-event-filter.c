/**
 * Example of filtering events in Evas
 *
 * Demonstrates how to filter events by seat using a pair of Evas
 * objects and the efl_input_seat_event_filter_set() routine.  The blue
 * rectangle can accept events from any seat, while the red one is
 * filtered to only take events from a single seat.
 *
 * @see efl_input_seat_event_filter_set
 *
 * @verbatim
 * gcc -o evas-event-filter evas-event-filter.c `pkg-config --libs --cflags evas ecore ecore-evas`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eina.h>
#include <Ecore.h>
#include <Evas.h>
#include <Ecore_Evas.h>
#include <stdio.h>

#define W (300)
#define H (300)
#define RECT_W (50)
#define RECT_H (50)

typedef struct _Context {
   Evas_Object *filtered_obj;
   Efl_Input_Device *allowed_seat;
} Context;

static void
_ee_del_request_cb(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_ee_resize_cb(Ecore_Evas *ee)
{
   Evas_Object *bg;
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   bg = ecore_evas_data_get(ee, "bg");
   evas_object_resize(bg, w, h);
}

static Eina_Bool
_allowed_seat_get(Evas_Object *filtered_obj,
                  Efl_Input_Device **allowed_seat)
{
   const Eina_List *devs, *l;
   Efl_Input_Device *seat;
   Eina_Bool allowed_seat_changed = EINA_FALSE;

   if (!filtered_obj) return EINA_TRUE;

   devs = evas_device_list(evas_object_evas_get(filtered_obj), NULL);
   EINA_LIST_FOREACH(devs, l, seat)
     {
        if ((efl_input_device_type_get(seat) != EFL_INPUT_DEVICE_TYPE_SEAT) ||
            (*allowed_seat == seat))
          continue;
        if (!allowed_seat_changed)
          {
             printf("The '%s' shall only receive events from seat '%s'\n",
                    evas_object_name_get(filtered_obj),
                    efl_name_get(seat));
             allowed_seat_changed = EINA_TRUE;
             *allowed_seat = seat;
             efl_input_seat_event_filter_set(filtered_obj, seat, EINA_TRUE);
             if (!efl_canvas_object_seat_focus_add(filtered_obj, seat))
               {
                  fprintf(stderr, "ERROR: The '%s' could not be focused by the seat '%s'\n",
                          evas_object_name_get(filtered_obj),
                          efl_name_get(seat));
                  return EINA_FALSE;
               }
          }
        else
          {
             if (efl_canvas_object_seat_focus_add(filtered_obj, seat))
               {
                  fprintf(stderr, "ERROR: The '%s' should not be focused by the seat '%s'\n",
                          evas_object_name_get(filtered_obj),
                          efl_name_get(seat));
                  return EINA_FALSE;
               }
          }
     }
   if (!allowed_seat_changed)
     *allowed_seat = NULL;
   return EINA_TRUE;
}

static const char *
_event_as_string(const void *desc)
{
   if (desc == EFL_EVENT_FOCUS_IN)
     return "FOCUS_IN";
   else if (desc == EFL_EVENT_FOCUS_OUT)
     return "FOCUS_OUT";
   else if (desc == EFL_EVENT_KEY_DOWN)
     return "KEY_DOWN";
   else if (desc == EFL_EVENT_KEY_UP)
     return "KEY_UP";
   else if (desc == EFL_EVENT_HOLD)
     return "HOLD";
   else if (desc == EFL_EVENT_POINTER_IN)
     return "POINTER_IN";
   else if (desc == EFL_EVENT_POINTER_OUT)
     return "POINTER_OUT";
   else if (desc == EFL_EVENT_POINTER_DOWN)
     return "POINTER_DOWN";
   else if (desc == EFL_EVENT_POINTER_UP)
     return "POINTER_UP";
   else if (desc == EFL_EVENT_POINTER_MOVE)
     return "POINTER_MOVE";
   else
     return "MOUSE_WHEEL";
}

static void
_obj_events_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Device *seat = efl_input_device_seat_get(efl_input_device_get(event->info));
   Context *ctx = data;
   const char  *event_name;

   event_name = _event_as_string(event->desc);
   if (seat != ctx->allowed_seat && event->object == ctx->filtered_obj)
     {
        fprintf(stderr, "ERROR: The object '%s' should not receive the event"
                "'%s' from the seat '%s'\n",
                evas_object_name_get(event->object), event_name,
                efl_name_get(seat));
        ecore_main_loop_quit();
     }
   else
     printf("The object '%s' recevied a '%s' event from seat '%s'\n",
            evas_object_name_get(event->object), event_name,
            efl_name_get(seat));
}

static void
_obj_del_event_cb(void *data, const Efl_Event *event)
{
   Context *ctx = data;

   if (event->object == ctx->filtered_obj)
     ctx->filtered_obj = NULL;
}

static void
_device_added_removed_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Device *dev = event->info;
   Context *ctx = data;

   if (efl_input_device_type_get(dev) != EFL_INPUT_DEVICE_TYPE_SEAT)
     return;

   if (event->desc == EFL_CANVAS_SCENE_EVENT_DEVICE_ADDED)
     {
        if (ctx->allowed_seat)
          efl_input_seat_event_filter_set(ctx->filtered_obj,
                                          ctx->allowed_seat, EINA_FALSE);
        ctx->allowed_seat = dev;
     }
   else
     {
        if (!_allowed_seat_get(ctx->filtered_obj, &ctx->allowed_seat))
          ecore_main_loop_quit();
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(_device_callbacks,
                           { EFL_CANVAS_SCENE_EVENT_DEVICE_ADDED, _device_added_removed_cb },
                           { EFL_CANVAS_SCENE_EVENT_DEVICE_REMOVED, _device_added_removed_cb });

EFL_CALLBACKS_ARRAY_DEFINE(_obj_callbacks,
                           { EFL_EVENT_FOCUS_IN, _obj_events_cb },
                           { EFL_EVENT_FOCUS_OUT, _obj_events_cb },
                           { EFL_EVENT_KEY_DOWN, _obj_events_cb },
                           { EFL_EVENT_KEY_UP, _obj_events_cb },
                           { EFL_EVENT_HOLD, _obj_events_cb },
                           { EFL_EVENT_POINTER_IN, _obj_events_cb },
                           { EFL_EVENT_POINTER_OUT,_obj_events_cb },
                           { EFL_EVENT_POINTER_DOWN, _obj_events_cb },
                           { EFL_EVENT_POINTER_UP, _obj_events_cb },
                           { EFL_EVENT_POINTER_MOVE, _obj_events_cb },
                           { EFL_EVENT_POINTER_WHEEL, _obj_events_cb },
                           { EFL_EVENT_DEL, _obj_del_event_cb });

static Evas_Object *
_rect_add(Evas *e, Context *ctx, const char *name,
          int w, int h, int x, int y, int r, int g, int b,
          Eina_Bool add_callbacks)
{
   Evas_Object *obj;

   obj = evas_object_rectangle_add(e);
   if (!obj)
     {
        fprintf(stderr, "Could not create the BG\n");
        return NULL;
     }

   evas_object_color_set(obj, r, g, b, 255);
   evas_object_resize(obj, w, h);
   evas_object_move(obj, x, y);
   evas_object_show(obj);
   evas_object_name_set(obj, name);
   if (add_callbacks)
     efl_event_callback_array_add(obj, _obj_callbacks(), ctx);
   return obj;
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   Context ctx = { 0 };
   Ecore_Evas *ee;
   Evas_Object *obj;
   Evas *e;

   if (!ecore_evas_init())
     {
        fprintf(stderr, "Could not init Ecore_Evas\n");
        return EXIT_FAILURE;
     }

   ee = ecore_evas_new(NULL, 0, 0, W, H, NULL);
   if (!ee)
     {
        fprintf(stderr, "Could not create the Ecore_Evas\n");
        goto err_ee;
     }

   e = ecore_evas_get(ee);

   obj = _rect_add(e, &ctx, "bg", W, H, 0, 0, 255, 255, 255, EINA_FALSE);
   if (!obj)
     {
        fprintf(stderr, "Could not create the BG\n");
        goto err_obj;
     }
   ecore_evas_data_set(ee, "bg", obj);

   obj = _rect_add(e, &ctx, "Red Rectangle", RECT_W, RECT_H, W/2 - RECT_W/2,
                   H/2 - RECT_H/2, 255, 0, 0, EINA_TRUE);
   if (!obj)
     {
        fprintf(stderr, "Could not create the red rectangle\n");
        goto err_obj;
     }
   ctx.filtered_obj = obj;

   obj = _rect_add(e, &ctx, "Blue Rectangle", RECT_W, RECT_H,
                   100, 100, 0, 0, 255, EINA_TRUE);
   if (!obj)
     {
        fprintf(stderr, "Could not create the blue rectangle\n");
        goto err_obj;
     }

   printf("The '%s' shall receive events from any seat\n",
          evas_object_name_get(obj));

   if (!_allowed_seat_get(ctx.filtered_obj, &ctx.allowed_seat))
     goto err_obj;
   efl_event_callback_array_add(e, _device_callbacks(), &ctx);
   ecore_evas_callback_resize_set(ee, _ee_resize_cb);
   ecore_evas_callback_delete_request_set(ee, _ee_del_request_cb);
   ecore_evas_show(ee);
   ecore_main_loop_begin();
   ecore_evas_free(ee);
   ecore_evas_shutdown();
   return EXIT_SUCCESS;

 err_obj:
   ecore_evas_free(ee);
 err_ee:
   ecore_evas_shutdown();
   return EXIT_FAILURE;
}

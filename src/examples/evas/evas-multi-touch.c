#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <stdio.h>
#include <errno.h>

#define WIDTH  (800)
#define HEIGHT (600)

#define RED 0
#define GREEN 85
#define BLUE 170

#define ALPHA_BEGIN 64
#define ALPHA_END 192
#define ALPHA_MOVE 255

#define RECT_SIZE 50

struct test_data
{
   Ecore_Evas  *ee;
   Evas        *canvas;
   Evas_Object *bg;
   Eina_Hash   *hash;
};

typedef struct touch_point
{
   Evas_Object *begin;
   Evas_Object *end;
   Evas_Object *move;
   int r, g, b;
   int dev;
} Touch_Point;

static struct test_data d = {0};

static void
_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(d.bg, w, h);
}

static Evas_Object *
_rect_add(int x, int y, int r, int g, int b, int alpha)
{
   int ar, ag, ab;

   Evas_Object *rect = evas_object_rectangle_add(d.canvas);
   evas_object_resize(rect, RECT_SIZE, RECT_SIZE);
   evas_object_move(rect, x - (RECT_SIZE / 2), y - (RECT_SIZE / 2));

   ar = r * alpha / 255;
   ag = g * alpha / 255;
   ab = b * alpha / 255;

   evas_object_color_set(rect, ar, ag, ab, alpha);

   return rect;
}

/* Create 3 rectangles, one to mark the beginning of the touch movement,
 * another one for the move itself, and the last one to mark the end of the
 * movement.
 */
static Touch_Point *
_touch_add(int dev, int x, int y)
{
   Touch_Point *tp = calloc(1, sizeof(*tp));

   tp->r = (((dev * 50) + RED) % 256);
   tp->g = (((dev * 50) + GREEN) % 256);
   tp->b = (((dev * 50) + BLUE) % 256);

   tp->begin = _rect_add(x, y, tp->r, tp->g, tp->b, ALPHA_BEGIN);
   tp->end = _rect_add(x, y, tp->r, tp->g, tp->b, ALPHA_END);
   tp->move = _rect_add(x, y, tp->r, tp->g, tp->b, ALPHA_MOVE);

   evas_object_show(tp->begin);
   evas_object_show(tp->move);

   return tp;
}

static void
_touch_move(Touch_Point *tp, int x, int y)
{
   evas_object_move(tp->move, x - (RECT_SIZE / 2), y - (RECT_SIZE / 2));
}

static void
_touch_end(Touch_Point *tp, int x, int y)
{
   evas_object_hide(tp->move);

   evas_object_move(tp->end, x - (RECT_SIZE / 2), y - (RECT_SIZE / 2));
   evas_object_show(tp->end);
}

static void
_touch_del(Touch_Point *tp)
{
   evas_object_del(tp->begin);
   evas_object_del(tp->end);
   evas_object_del(tp->move);

   free(tp);
}

static void
_mouse_down_handle(int device, int x, int y)
{
   Touch_Point *tp = NULL, *old_tp;

   tp = _touch_add(device, x, y);
   old_tp = eina_hash_set(d.hash, &device, tp);
   if (old_tp)
     _touch_del(old_tp);
}

static void
_mouse_move_handle(int device, int x, int y)
{
   Touch_Point *tp;

   tp = eina_hash_find(d.hash, &device);
   if (!tp)
     return;

   _touch_move(tp, x, y);
}

static void
_mouse_up_handle(int device, int x, int y)
{
   Touch_Point *tp;

   tp = eina_hash_find(d.hash, &device);
   if (!tp)
     {
        printf("ERR: Could not find Touch Point for dev: %d\n", device);
        return;
     }

   _touch_end(tp, x, y);
}

static void
_mouse_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;

   _mouse_down_handle(0, ev->canvas.x, ev->canvas.y);
}

static void
_multi_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Multi_Down *ev = event_info;

   _mouse_down_handle(ev->device, ev->canvas.x, ev->canvas.y);
}

static void
_mouse_move_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;

   _mouse_move_handle(0, ev->cur.canvas.x, ev->cur.canvas.y);
}

static void
_multi_move_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Multi_Move *ev = event_info;

   _mouse_move_handle(ev->device, ev->cur.canvas.x, ev->cur.canvas.y);
}

static void
_mouse_up_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;

   _mouse_up_handle(0, ev->cur.canvas.x, ev->cur.canvas.y);
}

static void
_multi_up_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Multi_Move *ev = event_info;

   _mouse_up_handle(ev->device, ev->cur.canvas.x, ev->cur.canvas.y);
}

int
main(void)
{
   if (!eina_init())
     return EXIT_FAILURE;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* initialize a hash to store the touch points/rectangles */
   d.hash = eina_hash_int32_new(EINA_FREE_CB(_touch_del));

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   d.ee = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);
   if (!d.ee)
     goto error;

   ecore_evas_callback_resize_set(d.ee, _canvas_resize_cb);
   ecore_evas_show(d.ee);

   /* the canvas pointer, de facto */
   d.canvas = ecore_evas_get(d.ee);

   d.bg = evas_object_rectangle_add(d.canvas);
   evas_object_name_set(d.bg, "our dear rectangle");
   evas_object_color_set(d.bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(d.bg, 0, 0); /* at canvas' origin */
   evas_object_resize(d.bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(d.bg);

   evas_object_event_callback_add(d.bg, EVAS_CALLBACK_MULTI_DOWN, _multi_down_cb, &d);
   evas_object_event_callback_add(d.bg, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, &d);

   evas_object_event_callback_add(d.bg, EVAS_CALLBACK_MULTI_UP, _multi_up_cb, &d);
   evas_object_event_callback_add(d.bg, EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, &d);

   evas_object_event_callback_add(d.bg, EVAS_CALLBACK_MULTI_MOVE, _multi_move_cb, &d);
   evas_object_event_callback_add(d.bg, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, &d);

   evas_object_focus_set(d.bg, EINA_TRUE); /* so we get input events */

   ecore_main_loop_begin();

   evas_object_event_callback_del(d.bg, EVAS_CALLBACK_MULTI_DOWN, _multi_down_cb);
   evas_object_event_callback_del(d.bg, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb);

   ecore_evas_free(d.ee);
   ecore_evas_shutdown();
   eina_shutdown();
   return 0;

error:
   fprintf(stderr, "you got to have at least one evas engine built and linked"
                   " up to ecore-evas for this example to run properly.\n");
   ecore_evas_shutdown();
   return -1;
}


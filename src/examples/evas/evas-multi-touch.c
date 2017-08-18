/**
 * Example of multi-touch in Evas.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader also built.
 *
 * @verbatim
 * gcc -o evas-multi-touch evas-multi-touch.c `pkg-config --libs --cflags evas ecore ecore-evas eina` -lm
 * @endverbatim
 */

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
_touch_update(Touch_Point *tp, float azimuth, float tilt, float twist, float pressure)
{
   Evas_Map* m = evas_map_new(4);
   evas_map_util_points_populate_from_object(m, tp->move);

   int r = tp->r * (1+pressure);
   int g = tp->g * (1+pressure);
   int b = tp->b * (1+pressure);

   int x, y, w, h;
   evas_object_geometry_get(tp->move, &x, &y, &w, &h);

   float vheight = cos(tilt);
   float vbase = sin(tilt);
   float vx = cos(azimuth) * vbase;
   float vy = sin(azimuth) * vbase;
   float rx = (-atan2(vheight,vy) + M_PI/2) * 180.0/M_PI;
   float ry = (-atan2(vheight,vx) + M_PI/2) * 180.0/M_PI;
   float rz = (twist) * 180.0/M_PI;

   /* apply */
   evas_object_color_set(tp->move, r, g, b, ALPHA_MOVE);
   evas_map_util_3d_rotate(m, rx, ry, rz, x+w/2, y+w/2, 0);
   evas_map_util_3d_perspective(m, x+w/2, y+w/2, 10, 100);
   evas_map_util_3d_lighting(m, WIDTH/2,HEIGHT/2, -500, 255, 255, 255, 192, 192, 192 );

   evas_object_map_set(tp->move, m);
   evas_object_map_enable_set(tp->move, EINA_TRUE);
   evas_map_free(m);
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
_mouse_update_handle(int device, float azimuth, float tilt, float twist, float pressure)
{
   Touch_Point *tp;

   tp = eina_hash_find(d.hash, &device);
   if (!tp)
     return;

   _touch_update(tp, azimuth, tilt, twist, pressure);
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

static void
_axis_update_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Axis_Update *ev = event_info;
   float twist = 0, azimuth = 0, tilt = 0;
   float pressure = 1, distance = 0;
   int i;

   for (i = 0; i < ev->naxis; i++)
     {
        float val = ev->axis[i].value;
        switch (ev->axis[i].label)
          {
          case EVAS_AXIS_LABEL_PRESSURE: pressure = val; break;
          case EVAS_AXIS_LABEL_TWIST:    twist    = val; break;
          case EVAS_AXIS_LABEL_AZIMUTH:  azimuth  = val; break;
          case EVAS_AXIS_LABEL_TILT:     tilt     = val; break;
          case EVAS_AXIS_LABEL_DISTANCE: distance = val; break;
          default: break;
          }
     }

    printf("distance %.2f, pressure %.2f, twist %.2f, azimuth %.2f, tilt %.2f\n",
           distance, pressure, twist, azimuth, tilt);
    _mouse_update_handle(ev->toolid, azimuth, tilt, twist, pressure);
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

   evas_object_event_callback_add(d.bg, EVAS_CALLBACK_AXIS_UPDATE, _axis_update_cb, &d);

   evas_object_focus_set(d.bg, EINA_TRUE); /* so we get input events */

   ecore_main_loop_begin();

   evas_object_event_callback_del(d.bg, EVAS_CALLBACK_MULTI_DOWN, _multi_down_cb);
   evas_object_event_callback_del(d.bg, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb);

   ecore_evas_free(d.ee);
   ecore_evas_shutdown();
   eina_shutdown();
   return 0;

error:
   fprintf(stderr, "error: Requires at least one Evas engine built and linked"
                   " to ecore-evas for this example to run properly.\n");
   ecore_evas_shutdown();
   return -1;
}

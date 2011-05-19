#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

typedef struct _State State;
typedef struct _Slice Slice;

struct _State
{
   Evas_Object *orig, *win, *base;
   Evas_Coord down_x, down_y;
   Eina_Bool  down : 1;
   Evas_Coord x, y;
   Eina_List *slices;
};

struct _Slice
{
   Evas_Object *obj;
};

static State state =
{
   NULL, NULL, NULL,
   0, 0,
   0,
   0, 0,
   NULL,
};

#define RES 8

static void
_slice_update(State *st)
{
   Evas_Coord x1, y1, x2, y2, mx, my, px, rx, ry, prx, pry, dst, dx, dy, pdst;
   Evas_Coord x, y, w, h;
   int i;
   Evas_Map *m;
   Slice *sl;
   int rad;
   
   evas_object_geometry_get(st->orig, &x, &y, &w, &h);
   x1 = st->down_x;
   y1 = st->down_y;
   x2 = st->x;
   y2 = st->y;
   mx = (x1 + x2) / 2;
   my = (y1 + y2) / 2;
   
   if (mx < 0) mx = 0;
   else if (mx >= w) mx = w - 1;
   if (my < 0) my = 0;
   else if (my >= h) my = h - 1;
   
   if (!st->base)
     {
        st->base = evas_object_image_add(evas_object_evas_get(st->win));
        evas_object_image_fill_set(st->base, 0, 0, w, h);
        evas_object_pass_events_set(st->base, 1);
        evas_object_image_source_set(st->base, st->orig);
        evas_object_show(st->base);
     }
   
   m = evas_map_new(4);
   evas_map_smooth_set(m, 0);

   evas_map_point_coord_set(m,    0, x     , y    , 0);
   evas_map_point_coord_set(m,    1, x + mx, y    , 0);
   evas_map_point_coord_set(m,    2, x + mx, y + h, 0);
   evas_map_point_coord_set(m,    3, x     , y + h, 0);
   
   evas_map_point_image_uv_set(m, 0, 0 , 0);
   evas_map_point_image_uv_set(m, 1, mx, 0);
   evas_map_point_image_uv_set(m, 2, mx, h);
   evas_map_point_image_uv_set(m, 3, 0 , h);

   evas_map_point_color_set(m, 0, 255, 255, 255, 255);
   evas_map_point_color_set(m, 1, 255, 255, 255, 255);
   evas_map_point_color_set(m, 2, 255, 255, 255, 255);
   evas_map_point_color_set(m, 3, 255, 255, 255, 255);
   
/*
   // FIXME: lighting should be manual with pt 0 and 3 being white and
   // 2 and 3 matching the 
   evas_map_util_3d_lighting(m, 
                             0  , 0  , -1000,
                             255, 255, 255,
                             20 , 20 , 20);
 */
   evas_map_util_3d_perspective(m, x + (w / 2), y + (h / 2), 0, 512);
   
   evas_object_map_enable_set(st->base, 1);
   evas_object_map_set(st->base, m);
   evas_map_free(m);
   
   EINA_LIST_FREE(st->slices, sl)
     {
        evas_object_del(sl->obj);
        free(sl);
     }
   
   // cylinder radius is width / 8
   rad = (w - mx) / 4;
   if (rad < (w / 16)) rad = (w / 16);
   if (rad > (w / 8)) rad = w / 8;
   
   rad = w / 12;
   
   px = mx;
   prx = 0;
   pry = rad;
   for (i = 1; i < RES; i++)
     {
        sl = calloc(1, sizeof(Slice));
        st->slices = eina_list_append(st->slices, sl);
        sl->obj = evas_object_image_add(evas_object_evas_get(st->win));
        evas_object_image_fill_set(sl->obj, 0, 0, w, h);
        evas_object_pass_events_set(sl->obj, 1);
        evas_object_image_source_set(sl->obj, st->orig);
        evas_object_show(sl->obj);
        
        rx = (double)rad * sin((i * M_PI) / RES);
        ry = (double)rad * cos((i * M_PI) / RES);
        dx = rx - prx;
        dy = ry - pry;
        dst = sqrt((dx * dx) + (dy * dy));
        if ((px + dst) > w)
          {
             pdst = dst;
             dst = w - px;
             rx = prx + (((rx - prx) * dst) / pdst);
             ry = pry + (((ry - pry) * dst) / pdst);
          }
        if (dst <= 0) break;
        
        m = evas_map_new(4);
        evas_map_smooth_set(m, 0);

        evas_map_point_coord_set(m,    0, x + mx + prx, y    , 0 - (rad - pry));
        evas_map_point_coord_set(m,    1, x + mx + rx , y    , 0 - (rad - ry ));
        evas_map_point_coord_set(m,    2, x + mx + rx , y + h, 0 - (rad - ry ));
        evas_map_point_coord_set(m,    3, x + mx + prx, y + h, 0 - (rad - pry));
        
        evas_map_point_image_uv_set(m, 0, px , 0);
        evas_map_point_image_uv_set(m, 1, px + dst, 0);
        evas_map_point_image_uv_set(m, 2, px + dst, h);
        evas_map_point_image_uv_set(m, 3, px , h);
        
        evas_map_point_color_set(m, 0, 255, 255, 255, 255);
        evas_map_point_color_set(m, 1, 255, 255, 255, 255);
        evas_map_point_color_set(m, 2, 255, 255, 255, 255);
        evas_map_point_color_set(m, 3, 255, 255, 255, 255);
        
        evas_map_util_3d_perspective(m, x + (w / 2), y + (h / 2), 0, 512);
        
        evas_object_map_enable_set(sl->obj, 1);
        evas_object_map_set(sl->obj, m);
        evas_map_free(m);
        
        prx = rx;
        pry = ry;
        px += dst;
     }
   if (px < w)
     {
        sl = calloc(1, sizeof(Slice));
        st->slices = eina_list_append(st->slices, sl);
        sl->obj = evas_object_image_add(evas_object_evas_get(st->win));
        evas_object_image_fill_set(sl->obj, 0, 0, w, h);
        evas_object_pass_events_set(sl->obj, 1);
        evas_object_image_source_set(sl->obj, st->orig);
        evas_object_show(sl->obj);
        
        m = evas_map_new(4);
        evas_map_smooth_set(m, 0);

        evas_map_point_coord_set(m,    0, x + mx + prx, y    , 0 - (rad - pry));
        evas_map_point_coord_set(m,    1, x + mx + (px - w) , y    , 0 - (rad * 2  ));
        evas_map_point_coord_set(m,    2, x + mx + (px - w) , y + h, 0 - (rad * 2  ));
        evas_map_point_coord_set(m,    3, x + mx + prx, y + h, 0 - (rad - pry));
        
        evas_map_point_image_uv_set(m, 0, px , 0);
        evas_map_point_image_uv_set(m, 1, w, 0);
        evas_map_point_image_uv_set(m, 2, w, h);
        evas_map_point_image_uv_set(m, 3, px , h);
        
        evas_map_point_color_set(m, 0, 255, 255, 255, 255);
        evas_map_point_color_set(m, 1, 255, 255, 255, 255);
        evas_map_point_color_set(m, 2, 255, 255, 255, 255);
        evas_map_point_color_set(m, 3, 255, 255, 255, 255);
        
        evas_map_util_3d_perspective(m, x + (w / 2), y + (h / 2), 0, 512);
        
        evas_object_map_enable_set(sl->obj, 1);
        evas_object_map_set(sl->obj, m);
        evas_map_free(m);
     }
}

static void
_slice_end(State *st)
{
   Slice *sl;
   EINA_LIST_FREE(st->slices, sl)
     {
        evas_object_del(sl->obj);
        free(sl);
     }
   if (st->base)
     {
        evas_object_del(st->base);
        st->base = NULL;
     }
}

static void
im_down_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Object *win = data;
   Evas_Coord x, y;

   if (ev->button != 1) return;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   state.orig = obj;
   state.win = win;
   state.down = 1;
   state.x = ev->canvas.x - x;
   state.y = ev->canvas.y - y;
   state.down_x = state.x;
   state.down_y = state.y;
   evas_object_lower(obj);
   printf("v %i %i\n", state.x, state.y);
   _slice_update(&state);
}

static void
im_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Object *win = data;
   Evas_Coord x, y;
   
   if (ev->button != 1) return;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   state.down = 0;
   state.x = ev->canvas.x - x;
   state.y = ev->canvas.y - y;
   evas_object_raise(obj);
   printf("^ %i %i\n", state.x, state.y);
   _slice_end(&state);
}

static void
im_move_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Object *win = data;
   Evas_Coord x, y;
   
   if (!state.down) return;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   state.x = ev->cur.canvas.x - x;
   state.y = ev->cur.canvas.y - y;
   printf("@ %i %i\n", state.x, state.y);
   _slice_update(&state);
}

void
test_flip_page(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *im;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "flip_page", ELM_WIN_BASIC);
   elm_win_title_set(win, "Flip Page");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

#if 1
   im = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", PACKAGE_DATA_DIR);
   elm_layout_file_set(im, buf, "layout");
#else   
   im = evas_object_image_filled_add(evas_object_evas_get(win));
   snprintf(buf, sizeof(buf), "%s/images/%s",
            PACKAGE_DATA_DIR, "twofish.jpg");
   evas_object_image_file_set(im, buf, NULL);
#endif   
   evas_object_move(im, 40, 40);
   evas_object_resize(im, 400, 400);
   evas_object_show(im);

   evas_object_event_callback_add(im, EVAS_CALLBACK_MOUSE_DOWN, im_down_cb, win);
   evas_object_event_callback_add(im, EVAS_CALLBACK_MOUSE_UP,   im_up_cb,   win);
   evas_object_event_callback_add(im, EVAS_CALLBACK_MOUSE_MOVE, im_move_cb, win);
   
   evas_object_resize(win, 480, 480);
   evas_object_show(win);
}
#endif

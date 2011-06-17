#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

typedef struct _Point
{
   Evas_Coord x, y, z, u, v;
} Point;

typedef struct _Side
{
   Evas_Object *o;
   Point pt[4];
} Side;

typedef struct _Cube
{
   Side side[6];
} Cube;

static Cube *cube;
static double rotx = 0.0, roty = 0.0, rotz = 0.0;
static double cxo = 0.0, cyo = 0.0, focv = 256.0, z0v = 0.0;
#define POINT(n, p, xx, yy, zz, uu, vv) \
   c->side[n].pt[p].x = xx; \
   c->side[n].pt[p].y = yy; \
   c->side[n].pt[p].z = zz; \
   c->side[n].pt[p].u = uu; \
   c->side[n].pt[p].v = vv

static Cube *
_cube_new(Evas *evas, Evas_Coord w, Evas_Coord h, Evas_Coord d)
{
   Cube *c;
   int i;
   
   w -= (w / 2);
   h -= (h / 2);
   d -= (d / 2);
   c = calloc(1, sizeof(Cube));
   for (i = 0; i < 6; i++)
     {
        Evas_Object *o;
        char buf[PATH_MAX];
        o = evas_object_image_add(evas);
        c->side[i].o = o;
        snprintf(buf, sizeof(buf), "%s/images/%s",
                 PACKAGE_DATA_DIR, "twofish.jpg");
        evas_object_image_file_set(o, buf, NULL);
        evas_object_image_fill_set(o, 0, 0, 256, 256);
        evas_object_resize(o, 256, 256);
        evas_object_image_smooth_scale_set(o, 0);
        evas_object_show(o);
     }
   POINT(0, 0, -w, -h, -d,   0,   0);
   POINT(0, 1,  w, -h, -d, 256,   0);
   POINT(0, 2,  w,  h, -d, 256, 256);
   POINT(0, 3, -w,  h, -d,   0, 256);
   
   POINT(1, 0,  w, -h, -d,   0,   0);
   POINT(1, 1,  w, -h,  d, 256,   0);
   POINT(1, 2,  w,  h,  d, 256, 256);
   POINT(1, 3,  w,  h, -d,   0, 256);
   
   POINT(2, 0,  w, -h,  d,   0,   0);
   POINT(2, 1, -w, -h,  d, 256,   0);
   POINT(2, 2, -w,  h,  d, 256, 256);
   POINT(2, 3,  w,  h,  d,   0, 256);
   
   POINT(3, 0, -w, -h,  d,   0,   0);
   POINT(3, 1, -w, -h, -d, 256,   0);
   POINT(3, 2, -w,  h, -d, 256, 256);
   POINT(3, 3, -w,  h,  d,   0, 256);
   
   POINT(4, 0, -w, -h,  d,   0,   0);
   POINT(4, 1,  w, -h,  d, 256,   0);
   POINT(4, 2,  w, -h, -d, 256, 256);
   POINT(4, 3, -w, -h, -d,   0, 256);
   
   POINT(5, 0, -w,  h, -d,   0,   0);
   POINT(5, 1,  w,  h, -d, 256,   0);
   POINT(5, 2,  w,  h,  d, 256, 256);
   POINT(5, 3, -w,  h,  d,   0, 256);
   
   return c;
}

static void
_cube_pos(Cube *c,
          Evas_Coord x, Evas_Coord y, Evas_Coord z,
          double dx, double dy, double dz,
          Evas_Coord cx, Evas_Coord cy, Evas_Coord foc, Evas_Coord z0)
{
   Evas_Map *m;
   int i, j, order[6], sorted;
   Evas_Coord mz[6];
   
   m = evas_map_new(4);
   evas_map_smooth_set(m, 0);
   
   for (i = 0; i < 6; i++)
     {
        Evas_Coord tz[4];
        
        for (j = 0; j < 4; j++)
          {
             evas_map_point_coord_set(m, j,
                                      c->side[i].pt[j].x + x,
                                      c->side[i].pt[j].y + y,
                                      c->side[i].pt[j].z + z);
             evas_map_point_image_uv_set(m, j,
                                         c->side[i].pt[j].u,
                                         c->side[i].pt[j].v);
             evas_map_point_color_set(m, j, 255, 255, 255, 255);
          }
        evas_map_util_3d_rotate(m, dx, dy, dz, x, y, z);
        evas_map_util_3d_lighting(m, -1000, -1000, -1000,
                                  255, 255, 255,
                                  20, 20, 20);
        evas_map_util_3d_perspective(m, cx, cy, foc, z0);
        if (evas_map_util_clockwise_get(m))
          {
             evas_object_map_enable_set(c->side[i].o, 1);
             evas_object_map_set(c->side[i].o, m);
             evas_object_show(c->side[i].o);
          }
        else
           evas_object_hide(c->side[i].o);
        
        order[i] = i;
        for (j = 0; j < 4; j++)
           evas_map_point_coord_get(m, j, NULL, NULL, &(tz[j]));
        mz[i] = (tz[0] + tz[1] + tz[2] + tz[3]) / 4;
     }
   sorted = 0;
   do
     {
        sorted = 1;
        for (i = 0; i < 5; i++)
          {
             if (mz[order[i]] > mz[order[i + 1]])
               {
                  j = order[i];
                  order[i] = order[i + 1];
                  order[i + 1] = j;
                  sorted = 0;
               }
          }
     }
   while (!sorted);
   
   evas_object_raise(c->side[order[0]].o);
   for (i = 1; i < 6; i++)
      evas_object_stack_below(c->side[order[i]].o, c->side[order[i - 1]].o);
   evas_map_free(m);
}

/*
static void
_cube_free(Cube *c)
{
   int i;
   
   for (i = 0; i < 6; i++) evas_object_del(c->side[i].o);
   free(c);
}
*/

static void
_cube_update(Evas_Object *win, Cube *c)
{
   Evas_Coord w, h;
   
   evas_object_geometry_get(win, NULL, NULL, &w, &h);
   _cube_pos(c,
             (w / 2), (h / 2), 512,
             rotx, roty, rotz,
             (w / 2) + cxo, (h / 2) + cyo, z0v, focv);
}

void
_ch_rot_x(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   rotx = elm_slider_value_get(obj);
   _cube_update(win, cube);
}

void
_ch_rot_y(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   roty = elm_slider_value_get(obj);
   _cube_update(win, cube);
}

void
_ch_rot_z(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   rotz = elm_slider_value_get(obj);
   _cube_update(win, cube);
}

void
_ch_cx(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   cxo = elm_slider_value_get(obj);
   _cube_update(win, cube);
}

void
_ch_cy(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   cyo = elm_slider_value_get(obj);
   _cube_update(win, cube);
}

void
_ch_foc(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   focv = elm_slider_value_get(obj);
   _cube_update(win, cube);
}

void
_ch_z0(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   z0v = elm_slider_value_get(obj);
   _cube_update(win, cube);
}

void
test_3d(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *sl;

   win = elm_win_add(NULL, "3d", ELM_WIN_BASIC);
   elm_win_title_set(win, "3D");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   cube = _cube_new(evas_object_evas_get(win), 240, 240, 240);
   
   bx = elm_box_add(win);
   evas_object_layer_set(bx, 10);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "Rot X");
   elm_slider_unit_format_set(sl, "%1.0f units");
   elm_slider_span_size_set(sl, 360);
   elm_slider_min_max_set(sl, 0, 360);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sl);
   evas_object_smart_callback_add(sl, "changed", _ch_rot_x, win);
   evas_object_show(sl);

   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "Rot Y");
   elm_slider_unit_format_set(sl, "%1.0f units");
   elm_slider_span_size_set(sl, 360);
   elm_slider_min_max_set(sl, 0, 360);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sl);
   evas_object_smart_callback_add(sl, "changed", _ch_rot_y, win);
   evas_object_show(sl);

   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "Rot Z");
   elm_slider_unit_format_set(sl, "%1.0f units");
   elm_slider_span_size_set(sl, 360);
   elm_slider_min_max_set(sl, 0, 360);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sl);
   evas_object_smart_callback_add(sl, "changed", _ch_rot_z, win);
   evas_object_show(sl);

   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "CX Off");
   elm_slider_unit_format_set(sl, "%1.0f units");
   elm_slider_span_size_set(sl, 360);
   elm_slider_min_max_set(sl, -320, 320);
   elm_slider_value_set(sl, cxo);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sl);
   evas_object_smart_callback_add(sl, "changed", _ch_cx, win);
   evas_object_show(sl);

   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "CY Off");
   elm_slider_unit_format_set(sl, "%1.0f units");
   elm_slider_span_size_set(sl, 360);
   elm_slider_min_max_set(sl, -320, 320);
   elm_slider_value_set(sl, cyo);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sl);
   evas_object_smart_callback_add(sl, "changed", _ch_cy, win);
   evas_object_show(sl);

   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "Foc");
   elm_slider_unit_format_set(sl, "%1.0f units");
   elm_slider_span_size_set(sl, 360);
   elm_slider_min_max_set(sl, 1, 2000);
   elm_slider_value_set(sl, focv);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sl);
   evas_object_smart_callback_add(sl, "changed", _ch_foc, win);
   evas_object_show(sl);

   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "Z0");
   elm_slider_unit_format_set(sl, "%1.0f units");
   elm_slider_span_size_set(sl, 360);
   elm_slider_min_max_set(sl, -2000, 2000);
   elm_slider_value_set(sl, z0v);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sl);
   evas_object_smart_callback_add(sl, "changed", _ch_z0, win);
   evas_object_show(sl);

   evas_object_resize(win, 480, 480);
   _cube_update(win, cube);
   evas_object_show(win);
}
#endif

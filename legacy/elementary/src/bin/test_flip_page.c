#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

//#define PAGEMESH 1

typedef struct _State State;
typedef struct _Slice Slice;

typedef struct _Vertex2 Vertex2;
typedef struct _Vertex3 Vertex3;

struct _State
{
   Evas_Object *orig, *win;
   Evas_Coord down_x, down_y;
   Eina_Bool  down : 1;
   Evas_Coord x, y;
   Slice *base;
   Eina_List *slices;
};

struct _Slice
{
   Evas_Object *obj;
   // (0)---(1)
   //  |     |
   //  |     |
   // (3)---(2)
   double u[4], v[4];
   double x[4], y[4], z[4];
};

struct _Vertex2
{
   double x, y;
};

struct _Vertex3
{
   double x, y, z;
};

static State state =
{
   NULL, NULL,
   0, 0,
   0,
   0, 0,
   NULL,
   NULL
};

#define RES 32

static Slice *
_slice_new(State *st)
{
   Slice *sl;
   
   sl = calloc(1, sizeof(Slice));
   if (!sl) return NULL;
   sl->obj = evas_object_image_add(evas_object_evas_get(st->win));
   evas_object_image_smooth_scale_set(sl->obj, 0);
   evas_object_pass_events_set(sl->obj, 1);
   evas_object_image_source_set(sl->obj, st->orig);
   evas_object_show(sl->obj);
   return sl;
}

static void
_slice_free(Slice *sl)
{
   evas_object_del(sl->obj);
   free(sl);
}

static void
_slice_apply(Slice *sl, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Evas_Map *m;
   int i;
   
   m = evas_map_new(4);
   evas_map_smooth_set(m, 0);

   for (i = 0; i < 4; i++)
     {
        evas_map_point_coord_set(m, i, x + sl->x[i], y + sl->y[i], sl->z[i]);
        evas_map_point_image_uv_set(m, i, sl->u[i] , sl->v[i]);
        evas_map_point_color_set(m, i, 255, 255, 255, 255);
     }
   
   evas_map_util_3d_perspective(m, x + (w / 2), y + (h / 2), 0, 512);

/*
   // FIXME: lighting should be manual with pt 0 and 3 being white and
   // 2 and 3 matching the
   evas_map_util_3d_lighting(m,
                             0  , 0  , -1000,
                             255, 255, 255,
                             20 , 20 , 20);
 */
   
   evas_object_map_enable_set(sl->obj, EINA_TRUE);
   evas_object_image_fill_set(sl->obj, 0, 0, w, h);
   evas_object_map_set(sl->obj, m);
   evas_map_free(m);
}

static void
_slice_xyz(Slice *sl, 
           double x1, double y1, double z1,
           double x2, double y2, double z2,
           double x3, double y3, double z3,
           double x4, double y4, double z4)
{
   sl->x[0] = x1; sl->y[0] = y1; sl->z[0] = z1;
   sl->x[1] = x2; sl->y[1] = y2; sl->z[1] = z2;
   sl->x[2] = x3; sl->y[2] = y3; sl->z[2] = z3;
   sl->x[3] = x4; sl->y[3] = y4; sl->z[3] = z4;
}

static void
_slice_uv(Slice *sl, 
           double u1, double v1,
           double u2, double v2,
           double u3, double v3,
           double u4, double v4)
{
   sl->u[0] = u1; sl->v[0] = v1;
   sl->u[1] = u2; sl->v[1] = v2;
   sl->u[2] = u3; sl->v[2] = v3;
   sl->u[3] = u4; sl->v[3] = v4;
}

static void
_slice_update(State *st)
{
   Evas_Coord x1, y1, x2, y2, mx, my, px, rx, ry, prx, pry, dst, dx, dy, pdst;
   Evas_Coord x, y, w, h;
   int i;
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

   if (!st->base) st->base = _slice_new(st);
   sl = st->base;
   
   _slice_xyz(sl,
              0,  0,  0,
              mx, 0,  0,
              mx, h,  0,
              0,  h,  0);
   _slice_uv(sl,
             0,  0,
             mx, 0,
             mx, h,
             0,  h);
   _slice_apply(sl, x, y, w, h);
   
   EINA_LIST_FREE(st->slices, sl) _slice_free(sl);

   // cylinder radius is width / 8
   rad = (w - mx) / 4;
   if (rad < (w / 16)) rad = (w / 16);
   if (rad > (w / 8)) rad = w / 8;

   rad = w / 10;

   px = mx;
   prx = 0;
   pry = rad;
   for (i = 1; i < RES; i++)
     {
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

        sl = _slice_new(st);

        _slice_xyz(sl, 
                   mx + prx, 0, -(rad - pry),
                   mx + rx,  0, -(rad - ry),
                   mx + rx,  h, -(rad - ry),
                   mx + prx, h, -(rad - pry));
        _slice_uv(sl,
                  px,       0,
                  px + dst, 0,
                  px + dst, h,
                  px,       h);
        _slice_apply(sl, x, y, w, h);
        st->slices = eina_list_append(st->slices, sl);
        
        prx = rx;
        pry = ry;
        px += dst;
     }
   if (px < w)
     {
        sl = _slice_new(st);
        
        _slice_xyz(sl, 
                   mx + prx,      0, -(rad - pry),
                   mx + (px - w), 0, -(rad * 2),
                   mx + (px - w), h, -(rad * 2),
                   mx + prx,      h, -(rad - pry));
        _slice_uv(sl,
                  px, 0,
                  w,  0,
                  w,  h,
                  px, h);
        _slice_apply(sl, x, y, w, h);
        st->slices = eina_list_append(st->slices, sl);
     }
}

static void
_slice_end(State *st)
{
   Slice *sl;
   
   if (st->base) _slice_free(st->base);
   st->base = NULL;
   EINA_LIST_FREE(st->slices, sl) _slice_free(sl);
}


#ifdef PAGEMESH
static Evas_Object *sl_rho, *sl_theta, *sl_A;

static void 
_deform_point(Vertex2 *vi, Vertex3 *vo, double rho, double theta, double A)
{
   // ^Y
   // |
   // |    X
   // +---->
   // theta == cone angle (0 -> PI/2)
   // A     == distance of cone apex from origin
   // rho   == angle of cone from vertical axis (...-PI/2 to PI/2...)
   Vertex3  v1;   // First stage of the deformation
   double R, r, beta;
   
   // Radius of the circle circumscribed by vertex (vi->x, vi->y) around A
   // on the x-y plane
   R = sqrt(vi->x * vi->x + pow(vi->y - A, 2)); 
   // Now get the radius of the cone cross section intersected by our vertex
   // in 3D space.
   r = R * sin(theta);                       
   // Angle subtended by arc |ST| on the cone cross section.
   beta = asin(vi->x / R) / sin(theta);       
   
   // *** MAGIC!!! ***
   v1.x = r * sin(beta);
   v1.y = R + A - r * (1 - cos(beta)) * sin(theta); 
   v1.z = r * (1 - cos(beta)) * cos(theta);
   
   // Apply a basic rotation transform around the y axis to rotate the curled
   // page. These two steps could be combined through simple substitution,
   // but are left separate to keep the math simple for debugging and
   // illustrative purposes.
   vo->x = (v1.x * cos(rho)) - (v1.z * sin(rho));
   vo->y = v1.y;
   vo->z = (v1.x * sin(rho)) + (v1.z * cos(rho));
}

static void
_test(void)
{
   static Eina_List *pts = NULL;
   Evas_Object *o;
   Evas_Map *m;
   int i, j, k;
   Evas_Coord x, y, w, h;
   State *st = &state;
   
   EINA_LIST_FREE(pts, o) evas_object_del(o);
   
   evas_object_geometry_get(st->orig, &x, &y, &w, &h); 
  for (j = 0; j < h; j += 20)
     {
        for (i = 0; i < w; i += 20)
          {
             Vertex2 vi;
             Vertex3 vo;
             double rho, theta, A, n;
             
             vi.x = i;
             vi.y = j;
             rho = elm_slider_value_get(sl_rho);
             A = elm_slider_value_get(sl_A);
             theta = elm_slider_value_get(sl_theta);
             
             _deform_point(&vi, &vo, rho, theta, A);
             
             o = evas_object_image_add(evas_object_evas_get(st->win));
             evas_object_image_smooth_scale_set(o, 0);
             evas_object_pass_events_set(o, 1);
             evas_object_image_source_set(o, st->orig);
             evas_object_show(o);
             
             m = evas_map_new(4);
             evas_map_smooth_set(m, 0);
             
             k = 0;
             evas_map_point_coord_set(m, k, x + vo.x, y + vo.y, -vo.z);
             evas_map_point_image_uv_set(m, k, 0 , 0);
             evas_map_point_color_set(m, k, 255, 255, 255, 255);
             k++;
             evas_map_point_coord_set(m, k, x + vo.x + 10, y + vo.y, -vo.z);
             evas_map_point_image_uv_set(m, k, w , 0);
             evas_map_point_color_set(m, k, 255, 255, 255, 255);
             k++;
             evas_map_point_coord_set(m, k, x + vo.x + 10, y + vo.y + 10, -vo.z);
             evas_map_point_image_uv_set(m, k, w , h);
             evas_map_point_color_set(m, k, 255, 255, 255, 255);
             k++;
             evas_map_point_coord_set(m, k, x + vo.x, y + vo.y + 10, -vo.z);
             evas_map_point_image_uv_set(m, k, 0 , h);
             evas_map_point_color_set(m, k, 255, 255, 255, 255);
             k++;
             
             evas_map_util_3d_perspective(m, x + (w / 2), y + (h / 2), 0, 512);
             
             evas_object_map_enable_set(o, EINA_TRUE);
             evas_object_image_fill_set(o, 0, 0, w, h);
             evas_object_map_set(o, m);
             evas_map_free(m);
             
             pts = eina_list_append(pts, o);
          }
     }
}

static void
_sl_ch(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _test();
}
#endif   

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
im_up_cb(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
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
im_move_cb(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
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

#if 0
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
   
#ifdef PAGEMESH
   Evas_Object *sl;

   sl = elm_slider_add(win);
   elm_slider_min_max_set(sl, -5, 5);
   elm_slider_value_set(sl, 0);
   sl_rho = sl;
   evas_object_smart_callback_add(sl, "changed", _sl_ch, NULL);
   elm_slider_label_set(sl, "rho");
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_span_size_set(sl, 320);
   evas_object_move(sl, 10, 20);
   evas_object_resize(sl, 460, 40);
   evas_object_layer_set(sl, 100);
   evas_object_show(sl);
   
   sl = elm_slider_add(win);
   elm_slider_min_max_set(sl, 0, 10);
   elm_slider_value_set(sl, 7.86);
   sl_theta = sl;
   evas_object_smart_callback_add(sl, "changed", _sl_ch, NULL);
   elm_slider_label_set(sl, "theta");
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_span_size_set(sl, 320);
   evas_object_move(sl, 10, 60);
   evas_object_resize(sl, 460, 40);
   evas_object_layer_set(sl, 100);
   evas_object_show(sl);
   
   sl = elm_slider_add(win);
   elm_slider_min_max_set(sl, -800, 800);
   elm_slider_value_set(sl, -400);
   sl_A = sl;
   evas_object_smart_callback_add(sl, "changed", _sl_ch, NULL);
   elm_slider_label_set(sl, "A");
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_span_size_set(sl, 320);
   evas_object_move(sl, 10, 100);
   evas_object_resize(sl, 460, 40);
   evas_object_layer_set(sl, 100);
   evas_object_show(sl);
#endif   
}
#endif

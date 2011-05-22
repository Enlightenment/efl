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
   int slices_w, slices_h;
   Slice **slices;
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
   0, 0,
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

   evas_map_util_3d_perspective(m, x + (w / 2), y + (h / 2), 0, 1024);

/*   
   evas_map_util_3d_lighting(m,
                             x + (w / 2)  , y + (h / 2)  , -h * 10,
                             255, 255, 255,
                             80 , 80 , 80);
   for (i = 0; i < 4; i++)
     {
        int r, g, b, a;
        
        evas_map_point_color_get(m, i, &r, &g, &b, &a);
        r = (double)r * 1.2; if (r > 255) r = 255;
        g = (double)g * 1.2; if (g > 255) g = 255;
        b = (double)b * 1.2; if (b > 255) b = 255;
        evas_map_point_color_set(m, i, r, g, b, a);
     }
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
_deform_point(Vertex2 *vi, Vertex3 *vo, double rho, double theta, double A)
{
   // ^Y
   // |
   // |    X
   // +---->
   // theta == cone angle (0 -> PI/2)
   // A     == distance of cone apex from origin
   // rho   == angle of cone from vertical axis (...-PI/2 to PI/2...)
   Vertex3  v1;
   double d, r, b;
   
   d = sqrt((vi->x * vi->x) + pow(vi->y - A, 2)); 
   r = d * sin(theta);                       
   b = asin(vi->x / d) / sin(theta);       
   
   v1.x = r * sin(b);
   v1.y = d + A - (r * (1 - cos(b)) * sin(theta)); 
   v1.z = r * (1 - cos(b)) * cos(theta);
   
   vo->x = (v1.x * cos(rho)) - (v1.z * sin(rho));
   vo->y = v1.y;
   vo->z = (v1.x * sin(rho)) + (v1.z * cos(rho));
}

static void
_interp_point(Vertex3 *vi1, Vertex3 *vi2, Vertex3 *vo, double v)
{
   vo->x = (v * vi2->x) + ((1.0 - v) * vi1->x);
   vo->y = (v * vi2->y) + ((1.0 - v) * vi1->y);
   vo->z = (v * vi2->z) + ((1.0 - v) * vi1->z);
}

static void
_state_slices_clear(State *st)
{
   int i, j, num;
   
   if (st->slices)
     {
        num = 0;
        for (j = 0; j < st->slices_h; j++)
          {
             for (i = 0; i < st->slices_w; i++)
               {
                  if (st->slices[num])
                    {
                       _slice_free(st->slices[num]);
                       st->slices[num] = NULL;
                    }
                  num++;
               }
          }
        free(st->slices);
        st->slices = NULL;
        st->slices_w = 0;
        st->slices_h = 0;
     }
}

/*
static void
_slice_obj_vert_color_merge(Slice *s1, int p1, Slice *s2, int p2, 
                            Slice *s3, int p3, Slice *s4, int p4)
{
   int r1 = 0, g1 = 0, b1 = 0, a1 = 0, r2 = 0, g2 = 0, b2 = 0, a2 = 0, n = 0;
   Evas_Map *m;
   
   if (s1)
     {
        m = (Evas_Map *)evas_object_map_get(s1->obj);
        evas_map_point_color_get(m, p1, &r1, &g1, &b1, &a1);
        r2 += r1; g2 += g1; b2 += b1; a2 += a1;
        n++;
     }
   if (s2)
     {
        m = (Evas_Map *)evas_object_map_get(s2->obj);
        evas_map_point_color_get(m, p2, &r1, &g1, &b1, &a1);
        r2 += r1; g2 += g1; b2 += b1; a2 += a1;
        n++;
     }
   if (s3)
     {
        m = (Evas_Map *)evas_object_map_get(s3->obj);
        evas_map_point_color_get(m, p3, &r1, &g1, &b1, &a1);
        r2 += r1; g2 += g1; b2 += b1; a2 += a1;
        n++;
     }
   if (s4)
     {
        m = (Evas_Map *)evas_object_map_get(s4->obj);
        evas_map_point_color_get(m, p4, &r1, &g1, &b1, &a1);
        r2 += r1; g2 += g1; b2 += b1; a2 += a1;
        n++;
     }
   
   r2 /= n; g2 /= n; b2 /= n; a2 /= n;
   
   if (s1)
     {
        m = (Evas_Map *)evas_object_map_get(s1->obj);
        evas_map_point_color_set(m, p1, r2, g2, b2, a2);
        evas_object_map_set(s1->obj, m);
     }
   if (s2)
     {
        m = (Evas_Map *)evas_object_map_get(s2->obj);
        evas_map_point_color_set(m, p2, r2, g2, b2, a2);
        evas_object_map_set(s2->obj, m);
     }
   if (s3)
     {
        m = (Evas_Map *)evas_object_map_get(s3->obj);
        evas_map_point_color_set(m, p3, r2, g2, b2, a2);
        evas_object_map_set(s3->obj, m);
     }
   if (s4)
     {
        m = (Evas_Map *)evas_object_map_get(s4->obj);
        evas_map_point_color_set(m, p4, r2, g2, b2, a2);
        evas_object_map_set(s4->obj, m);
     }
}
*/
   
static int
_state_update(State *st)
{
   Evas_Coord x1, y1, x2, y2, mx, my, dst, dx, dy;
   Evas_Coord x, y, w, h;
   int i, j, num;
   Slice *sl;
   double b, minv = 0.0, minva, mgrad;
   int gx, gy, gsz, gw, gh;
   double rho, A, theta, perc, percm, n, rhol, Al, thetal;

   evas_object_geometry_get(st->orig, &x, &y, &w, &h);
   x1 = st->down_x;
   y1 = st->down_y;
   x2 = st->x;
   y2 = st->y;
   if (x2 >= x1) x2 = x1 - 1;
   mx = (x1 + x2) / 2;
   my = (y1 + y2) / 2;

   if (mx < 0) mx = 0;
   else if (mx >= w) mx = w - 1;
   if (my < 0) my = 0;
   else if (my >= h) my = h - 1;

   dx = x2 - x1;
   dy = y2 - y1;
   dst = sqrt((dx * dx) + (dy * dy));
   if (dst < 10)
      {
         // FIXME: clean up old objects
         return 0;
      }
   
   b = (h / 2);
   mgrad = (double)(y1 - y2) / (double)(x1 - x2);

   gsz = 16;
   
   _state_slices_clear(st);
   
   st->slices_w = (w + gsz - 1) / gsz;
   st->slices_h = (h + gsz - 1) / gsz;
   st->slices = calloc(st->slices_w * st->slices_h, sizeof(Slice *));
   if (!st->slices) return 0;
   
   if (mx < 1) mx = 1; // quick hack to keep curl line visible
   
   if (mgrad == 0.0) // special horizontal case
      mgrad = 0.001; // quick dirty hack for now
   // else
     {
        minv = 1.0 / mgrad;
        // y = (m * x) + b             
        b = my + (minv * mx);
     }
   if ((b >= -5) && (b <= (h + 5)))
     {
        if (minv > 0.0) // clamp to h
          {
             minv = (double)(h + 5 - my) / (double)(mx);
             b = my + (minv * mx);
          }
        else // clamp to 0
          {
             minv = (double)(-5 - my) / (double)(mx);
             b = my + (minv * mx);
          }
     }
   
   // DEBUG
   static Evas_Object *ol = NULL;
   Evas_Coord lx1, ly1, lx2, ly2;
   if (!ol) ol = evas_object_line_add(evas_object_evas_get(st->win));
   evas_object_color_set(ol, 128, 0, 0, 128);
   lx1 = x;
   ly1 = y + b;
   lx2 = x + w;
   ly2 = y + b + (-minv * w);
   evas_object_line_xy_set(ol, lx1, ly1, lx2, ly2);
   evas_object_show(ol);
   // END DEBUG
   // 
   perc = (double)x2 / (double)x1;
   percm = (double)mx / (double)x1;
   if (perc < 0.0) perc = 0.0;
   else if (perc > 1.0) perc = 1.0;
   if (percm < 0.0) percm = 0.0;
   else if (percm > 1.0) percm = 1.0;
   
   minva = atan(minv) / (M_PI / 2);
   if (minva < 0.0) minva = -minva;
   
   // A = apex of cone
   if (b <= 0) A = b;
   else A = h - b;
   if (A < -(h * 20)) A = -h * 20;
   //--//
   Al = -5;
   
   // rho = is how much the page is turned
   n = 1.0 - perc;
   n = 1.0 - cos(n * M_PI / 2.0);
   n = n * n;
   rho = -(n * M_PI);
   //--//
   rhol = -(n * M_PI);
   
   // theta == curliness (how much page culrs in on itself
   n = sin((1.0 - perc) * M_PI);
   n = n * 1.2;
   theta = 7.86 + n;
   //--//
   n = sin((1.0 - perc) * M_PI);
   n = 1.0 - n;
   n = n * n;
   n = 1.0 - n;
   thetal = 7.86 + n;
   
   num = 0;
   for (gx = 0; gx < w; gx += gsz)
     {
        for (gy = 0; gy < h; gy += gsz)
          {
             Vertex2 vi[4], vil[2];
             Vertex3 vo[4], vol[2];
             gw = gsz;
             gh = gsz;
             if ((gx + gw) > w) gw = w - gx;
             if ((gy + gh) > h) gh = h - gy;
             
             vi[0].x = gx;      vi[0].y = gy;
             vi[1].x = gx + gw; vi[1].y = gy;
             vi[2].x = gx + gw; vi[2].y = gy + gh;
             vi[3].x = gx;      vi[3].y = gy + gh;
             
             vil[0].x = gx;      vil[0].y = h - gx;
             vil[1].x = gx + gw; vil[1].y = h - (gx + gw);
             
             for (i = 0; i < 2; i++)
               {
                  _deform_point(&(vil[i]), &(vol[i]), rhol, thetal, Al);
               }
             for (i = 0; i < 4; i++)
               {
                  _deform_point(&(vi[i]), &(vo[i]), rho, theta, A);
               }
             n = minva * sin(perc * M_PI);
             n = n * n;
             vol[0].y = gy;
             vol[1].y = gy;
             _interp_point(&(vo[0]), &(vol[0]), &(vo[0]), n);
             _interp_point(&(vo[1]), &(vol[1]), &(vo[1]), n);
             vol[0].y = gy + gh;
             vol[1].y = gy + gh;
             _interp_point(&(vo[2]), &(vol[1]), &(vo[2]), n);
             _interp_point(&(vo[3]), &(vol[0]), &(vo[3]), n);
             if (b > 0)
               {
                  Vertex3 vt;
                  
#define SWPV3(a, b) do {vt = (a); (a) = (b); (b) = vt;} while (0)
                  SWPV3(vo[0], vo[3]);
                  SWPV3(vo[1], vo[2]);
                  vo[0].y = h - vo[0].y;
                  vo[1].y = h - vo[1].y;
                  vo[2].y = h - vo[2].y;
                  vo[3].y = h - vo[3].y;
               }
             
             sl = _slice_new(st);
             st->slices[num] = sl;
             num++;
             _slice_xyz(sl,
                        vo[0].x, vo[0].y, vo[0].z,
                        vo[1].x, vo[1].y, vo[1].z,
                        vo[2].x, vo[2].y, vo[2].z,
                        vo[3].x, vo[3].y, vo[3].z);
             if (b <= 0)
                _slice_uv(sl,
                          gx,       gy,       gx + gw,  gy,
                          gx + gw,  gy + gh,  gx,       gy + gh);
             else
                _slice_uv(sl,
                          gx,       h - (gy + gh), gx + gw,  h - (gy + gh),
                          gx + gw,  h - gy,        gx,       h - gy);
             _slice_apply(sl, x, y, w, h);
          }
     }
   
   // FIX shading in the verticies to blend at the points
/*
   num = 0;
   for (j = 0; j < st->slices_h; j++)
     {
        for (i = 0; i < st->slices_w; i++)
          {
             Slice *s[4];
             
             s[0] = s[1] = s[2] = s[3] = NULL;
             
             if ((i > 0) && (j > 0)) s[0] = st->slices[num - 1 - st->slices_w];
             if (j > 0) s[1] = st->slices[num - st->slices_w];
             
             if (i > 0) s[2] = st->slices[num - 1];
             s[3] = st->slices[num];
             
             _slice_obj_vert_color_merge(s[0], 2, s[1], 3,
                                         s[2], 1, s[3], 0);
             num++;
          }
     }
 */
   return 1;
}

static void
_state_end(State *st)
{
   _state_slices_clear(st);
}











#ifdef PAGEMESH
static Evas_Object *sl_rho, *sl_theta, *sl_A;

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

             evas_map_util_3d_perspective(m, x + (w / 2), y + (h / 2), 0, 1024);

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
   if (_state_update(&state))
      evas_object_lower(obj);
   printf("v %i %i\n", state.x, state.y);
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
   _state_end(&state);
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
   if (_state_update(&state))
      evas_object_lower(obj);
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
   evas_object_move(im, 140, 140);
   evas_object_resize(im, 200, 200);
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

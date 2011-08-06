#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Slice Slice;
typedef struct _Vertex2 Vertex2;
typedef struct _Vertex3 Vertex3;

struct _Slice
{
   Evas_Object *obj;
   double u[4], v[4], x[4], y[4], z[4];
};

struct _Vertex2
{
   double x, y;
};

struct _Vertex3
{
   double x, y, z;
};

struct _Widget_Data
{
   Evas_Object *obj;
   Ecore_Animator *animator;
   double start, len;
   Elm_Flip_Mode mode;
   Evas_Object *clip;
   Evas_Object *event[4];
   struct {
      Evas_Object *content, *clip;
   } front, back;
   Ecore_Job *job;
   Evas_Coord down_x, down_y, x, y, ox, oy, w, h;
   Elm_Flip_Interaction intmode;
   int dir;
   double    dir_hitsize[4];
   Eina_Bool dir_enabled[4];
   int slices_w, slices_h;
   Slice **slices, **slices2;

   Eina_Bool state : 1;
   Eina_Bool down : 1;
   Eina_Bool finish : 1;
   Eina_Bool started : 1;
   Eina_Bool backflip : 1;
   Eina_Bool pageflip : 1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void _state_slices_clear(Widget_Data *st);
static void _configure(Evas_Object *obj);

static const char SIG_ANIMATE_BEGIN[] = "animate,begin";
static const char SIG_ANIMATE_DONE[] = "animate,done";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_ANIMATE_BEGIN, ""},
   {SIG_ANIMATE_DONE, ""},
   {NULL, NULL}
};

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->animator) ecore_animator_del(wd->animator);
   _state_slices_clear(wd);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _sizing_eval(obj);
}

static Eina_Bool
_elm_flip_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd)
     return EINA_FALSE;

   /* Try Focus cycle in subitem */
   if (wd->state)
     return elm_widget_focus_next_get(wd->front.content, dir, next);
   else
     return elm_widget_focus_next_get(wd->back.content, dir, next);

}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, minw2 = -1, minh2 = -1;
   Evas_Coord maxw = -1, maxh = -1, maxw2 = -1, maxh2 = -1;
   int fingx = 0, fingy = 0;
   if (!wd) return;
   if (wd->front.content)
     evas_object_size_hint_min_get(wd->front.content, &minw, &minh);
   if (wd->back.content)
     evas_object_size_hint_min_get(wd->back.content, &minw2, &minh2);
   if (wd->front.content)
     evas_object_size_hint_max_get(wd->front.content, &maxw, &maxh);
   if (wd->back.content)
     evas_object_size_hint_max_get(wd->back.content, &maxw2, &maxh2);

   if (minw2 > minw) minw = minw2;
   if (minh2 > minh) minh = minh2;
   if ((maxw2 >= 0) && (maxw2 < maxw)) maxw = maxw2;
   if ((maxh2 >= 0) && (maxh2 < maxh)) maxh = maxh2;

   if (wd->dir_enabled[0]) fingy++;
   if (wd->dir_enabled[1]) fingy++;
   if (wd->dir_enabled[2]) fingx++;
   if (wd->dir_enabled[3]) fingx++;

   elm_coords_finger_size_adjust(fingx, &minw, fingy, &minh);

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (!wd) return;
   if (sub == wd->front.content)
     {
        evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
        wd->front.content = NULL;
        evas_object_hide(wd->front.clip);
        _sizing_eval(obj);
     }
   else if (sub == wd->back.content)
     {
        evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
        wd->back.content = NULL;
        evas_object_hide(wd->back.clip);
        _sizing_eval(obj);
     }
}

static Slice *
_slice_new(Widget_Data *st __UNUSED__, Evas_Object *obj)
{
   Slice *sl;

   sl = calloc(1, sizeof(Slice));
   if (!sl) return NULL;
   sl->obj = evas_object_image_add(evas_object_evas_get(obj));
   elm_widget_sub_object_add(st->obj, sl->obj);
   evas_object_clip_set(sl->obj, evas_object_clip_get(st->obj));
   evas_object_smart_member_add(sl->obj, st->obj);
   evas_object_image_smooth_scale_set(sl->obj, 0);
   evas_object_pass_events_set(sl->obj, 1);
   evas_object_image_source_set(sl->obj, obj);
   return sl;
}

static void
_slice_free(Slice *sl)
{
   evas_object_del(sl->obj);
   free(sl);
}

static void
_slice_apply(Widget_Data *st, Slice *sl,
             Evas_Coord x __UNUSED__, Evas_Coord y __UNUSED__, Evas_Coord w, Evas_Coord h __UNUSED__,
             Evas_Coord ox, Evas_Coord oy, Evas_Coord ow, Evas_Coord oh)
{
   Evas_Map *m;
   int i;

   m = evas_map_new(4);
   if (!m) return;
   evas_map_smooth_set(m, 0);
   for (i = 0; i < 4; i++)
     {
        evas_map_point_color_set(m, i, 255, 255, 255, 255);
        if (st->dir == 0)
          {
             int p[4] = { 0, 1, 2, 3 };
             evas_map_point_coord_set(m, i, ox + sl->x[p[i]], oy + sl->y[p[i]], sl->z[p[i]]);
             evas_map_point_image_uv_set(m, i, sl->u[p[i]] , sl->v[p[i]]);
          }
        else if (st->dir == 1)
          {
             int p[4] = { 1, 0, 3, 2 };
             evas_map_point_coord_set(m, i, ox + (w - sl->x[p[i]]), oy + sl->y[p[i]], sl->z[p[i]]);
             evas_map_point_image_uv_set(m, i, ow - sl->u[p[i]] , sl->v[p[i]]);
          }
        else if (st->dir == 2)
          {
             int p[4] = { 1, 0, 3, 2 };
             evas_map_point_coord_set(m, i, ox + sl->y[p[i]], oy + sl->x[p[i]], sl->z[p[i]]);
             evas_map_point_image_uv_set(m, i, sl->v[p[i]] , sl->u[p[i]]);
          }
        else if (st->dir == 3)
          {
             int p[4] = { 0, 1, 2, 3 };
             evas_map_point_coord_set(m, i, ox + sl->y[p[i]], oy + (w - sl->x[p[i]]), sl->z[p[i]]);
             evas_map_point_image_uv_set(m, i, sl->v[p[i]] , oh - sl->u[p[i]]);
          }
     }
   evas_object_map_enable_set(sl->obj, EINA_TRUE);
   evas_object_image_fill_set(sl->obj, 0, 0, ow, oh);
   evas_object_map_set(sl->obj, m);
   evas_map_free(m);
}

static void
_slice_3d(Widget_Data *st __UNUSED__, Slice *sl, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Evas_Map *m = (Evas_Map *)evas_object_map_get(sl->obj);
   int i;

   if (!m) return;
   // vanishing point is center of page, and focal dist is 1024
   evas_map_util_3d_perspective(m, x + (w / 2), y + (h / 2), 0, 1024);
   for (i = 0; i < 4; i++)
     {
        Evas_Coord x, y, z;
        evas_map_point_coord_get(m, i, &x, &y, &z);
        evas_map_point_coord_set(m, i, x, y, 0);
     }
   if (evas_map_util_clockwise_get(m)) evas_object_show(sl->obj);
   else evas_object_hide(sl->obj);
   evas_object_map_set(sl->obj, m);
}

static void
_slice_light(Widget_Data *st __UNUSED__, Slice *sl, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Evas_Map *m = (Evas_Map *)evas_object_map_get(sl->obj);
   int i;

   if (!m) return;
   evas_map_util_3d_lighting(m,
                             // light position
                             // (centered over page 10 * h toward camera)
                             x + (w / 2)  , y + (h / 2)  , -10000,
                             255, 255, 255, // light color
                             0 , 0 , 0); // ambient minimum
   // multiply brightness by 1.2 to make lightish bits all white so we dont
   // add shading where we could otherwise be pure white
   for (i = 0; i < 4; i++)
     {
        int r, g, b, a;

        evas_map_point_color_get(m, i, &r, &g, &b, &a);
        r = (double)r * 1.2; if (r > 255) r = 255;
        g = (double)g * 1.2; if (g > 255) g = 255;
        b = (double)b * 1.2; if (b > 255) b = 255;
        evas_map_point_color_set(m, i, r, g, b, a);
     }
   evas_object_map_set(sl->obj, m);
}

static void
_slice_xyz(Widget_Data *st __UNUSED__, Slice *sl,
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
_slice_uv(Widget_Data *st __UNUSED__, Slice *sl,
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
_state_slices_clear(Widget_Data *st)
{
   int i, j, num;

   if (st->slices)
     {
        num = 0;
        for (j = 0; j < st->slices_h; j++)
          {
             for (i = 0; i < st->slices_w; i++)
               {
                  if (st->slices[num]) _slice_free(st->slices[num]);
                  if (st->slices2[num]) _slice_free(st->slices2[num]);
                  num++;
               }
          }
        free(st->slices);
        free(st->slices2);
        st->slices = NULL;
        st->slices2 = NULL;
     }
   st->slices_w = 0;
   st->slices_h = 0;
}

static int
_slice_obj_color_sum(Slice *s, int p, int *r, int *g, int *b, int *a)
{
   Evas_Map *m;
   int rr = 0, gg = 0, bb = 0, aa = 0;

   if (!s) return 0;
   m = (Evas_Map *)evas_object_map_get(s->obj);
   if (!m) return 0;
   evas_map_point_color_get(m, p, &rr, &gg, &bb, &aa);
   *r += rr; *g += gg; *b += bb; *a += aa;
   return 1;
}

static void
_slice_obj_color_set(Slice *s, int p, int r, int g, int b, int a)
{
   Evas_Map *m;

   if (!s) return;
   m = (Evas_Map *)evas_object_map_get(s->obj);
   if (!m) return;
   evas_map_point_color_set(m, p, r, g, b, a);
   evas_object_map_set(s->obj, m);
}

static void
_slice_obj_vert_color_merge(Slice *s1, int p1, Slice *s2, int p2,
                            Slice *s3, int p3, Slice *s4, int p4)
{
   int r = 0, g = 0, b = 0, a = 0, n = 0;

   n += _slice_obj_color_sum(s1, p1, &r, &g, &b, &a);
   n += _slice_obj_color_sum(s2, p2, &r, &g, &b, &a);
   n += _slice_obj_color_sum(s3, p3, &r, &g, &b, &a);
   n += _slice_obj_color_sum(s4, p4, &r, &g, &b, &a);

   if (n < 1) return;
   r /= n; g /= n; b /= n; a /= n;

   _slice_obj_color_set(s1, p1, r, g, b, a);
   _slice_obj_color_set(s2, p2, r, g, b, a);
   _slice_obj_color_set(s3, p3, r, g, b, a);
   _slice_obj_color_set(s4, p4, r, g, b, a);
}

static int
_state_update(Widget_Data *st)
{
   Evas_Coord x1, y1, x2, y2, mx, my;
   Evas_Coord x, y, w, h, ox, oy, ow, oh;
   int i, j, num, nn, jump, num2;
   Slice *sl;
   double b, minv = 0.0, minva, mgrad;
   int gx, gy, gszw, gszh, gw, gh, col, row, nw, nh;
   double rho, A, theta, perc, percm, n, rhol, Al, thetal;
   Vertex3 *tvo, *tvol;
   Evas_Object *front, *back;

   st->backflip = 1;
   if (st->state)
     {
        front = st->front.content;
        back = st->front.content;
     }
   else
     {
        front = st->back.content;
        back = st->back.content;
     }

   evas_object_geometry_get(st->obj, &x, &y, &w, &h);
   ox = x; oy = y; ow = w; oh = h;
   x1 = st->down_x;
   y1 = st->down_y;
   x2 = st->x;
   y2 = st->y;

   if (st->dir == 0)
     {
        // no nothing. left drag is standard
     }
   else if (st->dir == 1)
     {
        x1 = (w - 1) - x1;
        x2 = (w - 1) - x2;
     }
   else if (st->dir == 2)
     {
        Evas_Coord tmp;

        tmp = x1; x1 = y1; y1 = tmp;
        tmp = x2; x2 = y2; y2 = tmp;
        tmp = w; w = h; h = tmp;
     }
   else if (st->dir == 3)
     {
        Evas_Coord tmp;

        tmp = x1; x1 = y1; y1 = tmp;
        tmp = x2; x2 = y2; y2 = tmp;
        tmp = w; w = h; h = tmp;
        x1 = (w - 1) - x1;
        x2 = (w - 1) - x2;
     }

   if (x2 >= x1) x2 = x1 - 1;
   mx = (x1 + x2) / 2;
   my = (y1 + y2) / 2;

   if (mx < 0) mx = 0;
   else if (mx >= w) mx = w - 1;
   if (my < 0) my = 0;
   else if (my >= h) my = h - 1;

   mgrad = (double)(y1 - y2) / (double)(x1 - x2);

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

   nw = 16;
   nh = 16;
   if (nw < 1) nw = 1;
   if (nh < 1) nh = 1;
   gszw = w / nw;
   gszh = h / nh;
   if (gszw < 4) gszw = 4;
   if (gszh < 4) gszh = 4;

   nw = (w + gszw - 1) / gszw;
   nh = (h + gszh - 1) / gszh;
   if ((st->slices_w != nw) || (st->slices_h != nh)) _state_slices_clear(st);
   st->slices_w = nw;
   st->slices_h = nh;
   if (!st->slices)
     {
        st->slices = calloc(st->slices_w * st->slices_h, sizeof(Slice *));
        if (!st->slices) return 0;
        st->slices2 = calloc(st->slices_w * st->slices_h, sizeof(Slice *));
        if (!st->slices2)
          {
             free(st->slices);
             st->slices = NULL;
             return 0;
          }
     }

   num = (st->slices_w + 1) * (st->slices_h + 1);

   tvo = alloca(sizeof(Vertex3) * num);
   tvol = alloca(sizeof(Vertex3) * (st->slices_w + 1));

   for (col = 0, gx = 0; gx <= (w + gszw - 1); gx += gszw, col++)
     {
        Vertex2 vil;

        vil.x = gx;
        vil.y = h - ((gx * h) / (w + gszw - 1));
        _deform_point(&vil, &(tvol[col]), rhol, thetal, Al);
     }

   n = minva * sin(perc * M_PI);
   n = n * n;

   num = 0;
   for (col = 0, gx = 0; gx <= (w + gszw - 1); gx += gszw, col++)
     {
        for (gy = 0; gy <= (h + gszh - 1); gy += gszh)
          {
             Vertex2 vi;
             Vertex3 vo, tvo1;

             if (gx > w) vi.x = w;
             else vi.x = gx;
             if (gy > h) vi.y = h;
             else vi.y = gy;
             _deform_point(&vi, &vo, rho, theta, A);
             tvo1 = tvol[col];
             if (gy > h) tvo1.y = h;
             else tvo1.y = gy;
             _interp_point(&vo, &tvo1, &(tvo[num]), n);
             num++;
          }
     }

   jump = st->slices_h + 1;
   for (col = 0, gx = 0; gx < w; gx += gszw, col++)
     {
        num = st->slices_h * col;
        num2 = jump * col;

        gw = gszw;
        if ((gx + gw) > w) gw = w - gx;

        for (row = 0, gy = 0; gy < h; gy += gszh, row++)
          {
             Vertex3 vo[4];

             if (b > 0) nn = num + st->slices_h - row - 1;
             else nn = num + row;

             gh = gszh;
             if ((gy + gh) > h) gh = h - gy;

             vo[0] = tvo[num2 + row];
             vo[1] = tvo[num2 + row + jump];
             vo[2] = tvo[num2 + row + jump + 1];
             vo[3] = tvo[num2 + row + 1];
#define SWP(a, b) do {typeof(a) vt; vt = (a); (a) = (b); (b) = vt;} while (0)
             if (b > 0)
               {
                  SWP(vo[0], vo[3]);
                  SWP(vo[1], vo[2]);
                  vo[0].y = h - vo[0].y;
                  vo[1].y = h - vo[1].y;
                  vo[2].y = h - vo[2].y;
                  vo[3].y = h - vo[3].y;
               }

             // FRONT
             sl = st->slices[nn];
             if (!sl)
               {
                  sl = _slice_new(st, front);
                  st->slices[nn] = sl;
               }
             _slice_xyz(st, sl,
                        vo[0].x, vo[0].y, vo[0].z,
                        vo[1].x, vo[1].y, vo[1].z,
                        vo[2].x, vo[2].y, vo[2].z,
                        vo[3].x, vo[3].y, vo[3].z);
             if (b <= 0)
                _slice_uv(st, sl,
                          gx,       gy,       gx + gw,  gy,
                          gx + gw,  gy + gh,  gx,       gy + gh);
             else
                _slice_uv(st, sl,
                          gx,       h - (gy + gh), gx + gw,  h - (gy + gh),
                          gx + gw,  h - gy,        gx,       h - gy);

                          // BACK
             sl = st->slices2[nn];
             if (!sl)
               {
                  sl = _slice_new(st, back);
                  st->slices2[nn] = sl;
               }

             _slice_xyz(st, sl,
                        vo[1].x, vo[1].y, vo[1].z,
                        vo[0].x, vo[0].y, vo[0].z,
                        vo[3].x, vo[3].y, vo[3].z,
                        vo[2].x, vo[2].y, vo[2].z);
             if (st->backflip)
               {
                  if (b <= 0)
                     _slice_uv(st, sl,
                               gx + gw, gy,       gx,       gy,
                               gx,      gy + gh,  gx + gw,  gy + gh);
                  else
                     _slice_uv(st, sl,
                               gx + gw, h - (gy + gh), gx,      h - (gy + gh),
                               gx,      h - gy,        gx + gw, h - gy);
               }
             else
               {
                  if (b <= 0)
                     _slice_uv(st, sl,
                               w - (gx + gw), gy,       w - (gx),      gy,
                               w - (gx),      gy + gh,  w - (gx + gw), gy + gh);
                  else
                     _slice_uv(st, sl,
                               w - (gx + gw), h - (gy + gh), w - (gx),      h - (gy + gh),
                               w - (gx),      h - gy,        w - (gx + gw), h - gy);
               }
          }
     }

   num = 0;
   for (j = 0; j < st->slices_h; j++)
     {
        for (i = 0; i < st->slices_w; i++)
          {
             _slice_apply(st, st->slices[num], x, y, w, h, ox, oy, ow, oh);
             _slice_apply(st, st->slices2[num], x, y, w, h, ox, oy, ow, oh);
             _slice_light(st, st->slices[num], ox, oy, ow, oh);
             _slice_light(st, st->slices2[num], ox, oy, ow, oh);
             num++;
          }
     }

   for (i = 0; i <= st->slices_w; i++)
     {
        num = i * st->slices_h;
        for (j = 0; j <= st->slices_h; j++)
          {
             Slice *s[4];

             s[0] = s[1] = s[2] = s[3] = NULL;
             if ((i > 0)            && (j > 0))
                s[0] = st->slices[num - 1 - st->slices_h];
             if ((i < st->slices_w) && (j > 0))
                s[1] = st->slices[num - 1];
             if ((i > 0)            && (j < st->slices_h))
                s[2] = st->slices[num - st->slices_h];
             if ((i < st->slices_w) && (j < st->slices_h))
                s[3] = st->slices[num];
             if (st->dir == 0)
                _slice_obj_vert_color_merge(s[0], 2, s[1], 3,
                                            s[2], 1, s[3], 0);
             else if (st->dir == 1)
                _slice_obj_vert_color_merge(s[0], 3, s[1], 2,
                                            s[2], 0, s[3], 1);
             else if (st->dir == 2)
                _slice_obj_vert_color_merge(s[0], 3, s[1], 2,
                                            s[2], 0, s[3], 1);
             else if (st->dir == 3)
                _slice_obj_vert_color_merge(s[0], 2, s[1], 3,
                                            s[2], 1, s[3], 0);
             s[0] = s[1] = s[2] = s[3] = NULL;
             if ((i > 0)            && (j > 0))
                s[0] = st->slices2[num - 1 - st->slices_h];
             if ((i < st->slices_w) && (j > 0))
                s[1] = st->slices2[num - 1];
             if ((i > 0)            && (j < st->slices_h))
                s[2] = st->slices2[num - st->slices_h];
             if ((i < st->slices_w) && (j < st->slices_h))
                s[3] = st->slices2[num];
             if (st->dir == 0)
                _slice_obj_vert_color_merge(s[0], 3, s[1], 2,
                                            s[2], 0, s[3], 1);
             else if (st->dir == 1)
                _slice_obj_vert_color_merge(s[0], 2, s[1], 3,
                                            s[2], 1, s[3], 0);
             else if (st->dir == 2)
                _slice_obj_vert_color_merge(s[0], 2, s[1], 3,
                                            s[2], 1, s[3], 0);
             else if (st->dir == 3)
                _slice_obj_vert_color_merge(s[0], 3, s[1], 2,
                                            s[2], 0, s[3], 1);
             num++;
          }
     }

   num = 0;
   for (i = 0; i < st->slices_w; i++)
     {
        for (j = 0; j < st->slices_h; j++)
          {
             _slice_3d(st, st->slices[num], ox, oy, ow, oh);
             _slice_3d(st, st->slices2[num], ox, oy, ow, oh);
             num++;
          }
     }

   return 1;
}

static void
_state_end(Widget_Data *st)
{
   _state_slices_clear(st);
}


static void
flip_show_hide(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (elm_flip_front_get(obj))
     {
        if (wd->pageflip)
          {
             if (wd->front.content)
               {
                  evas_object_move(wd->front.content, 4999, 4999);
                  evas_object_show(wd->front.clip);
               }
             else
                evas_object_hide(wd->front.clip);
             if (wd->back.content)
                evas_object_show(wd->back.clip);
             else
                evas_object_hide(wd->back.clip);
          }
        else
          {
             if (wd->front.content)
                evas_object_show(wd->front.clip);
             else
                evas_object_hide(wd->front.clip);
             if (wd->back.content)
                evas_object_hide(wd->back.clip);
             else
                evas_object_hide(wd->back.clip);
          }
     }
   else
     {
        if (wd->pageflip)
          {
             if (wd->front.content)
                evas_object_show(wd->front.clip);
             else
                evas_object_hide(wd->front.clip);
             if (wd->back.content)
               {
                  evas_object_move(wd->back.content, 4999, 4999);
                  evas_object_show(wd->back.clip);
               }
             else
                evas_object_hide(wd->back.clip);
          }
        else
          {
             if (wd->front.content)
                evas_object_hide(wd->front.clip);
             else
                evas_object_hide(wd->front.clip);
             if (wd->back.content)
                evas_object_show(wd->back.clip);
             else
                evas_object_hide(wd->back.clip);
          }
     }
}

static void
_flip_do(Evas_Object *obj, double t, Elm_Flip_Mode mode, int lin, int rev)
{
   Evas_Coord x, y, w, h;
   double p, deg, pp;
   Evas_Map *mf, *mb;
   Evas_Coord cx, cy, px, py, foc;
   int lx, ly, lz, lr, lg, lb, lar, lag, lab;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;

   mf = evas_map_new(4);
   evas_map_smooth_set(mf, 0);
   mb = evas_map_new(4);
   evas_map_smooth_set(mb, 0);

   if (wd->front.content)
     {
        const char *type = evas_object_type_get(wd->front.content);

        // FIXME: only handles filled obj
        if ((type) && (!strcmp(type, "image")))
          {
             int iw, ih;
             evas_object_image_size_get(wd->front.content, &iw, &ih);
             evas_object_geometry_get(wd->front.content, &x, &y, &w, &h);
             evas_map_util_points_populate_from_geometry(mf, x, y, w, h, 0);
             evas_map_point_image_uv_set(mf, 0, 0, 0);
             evas_map_point_image_uv_set(mf, 1, iw, 0);
             evas_map_point_image_uv_set(mf, 2, iw, ih);
             evas_map_point_image_uv_set(mf, 3, 0, ih);
          }
        else
          {
             evas_object_geometry_get(wd->front.content, &x, &y, &w, &h);
             evas_map_util_points_populate_from_geometry(mf, x, y, w, h, 0);
          }
     }
   if (wd->back.content)
     {
        const char *type = evas_object_type_get(wd->back.content);

        if ((type) && (!strcmp(type, "image")))
          {
             int iw, ih;
             evas_object_image_size_get(wd->back.content, &iw, &ih);
             evas_object_geometry_get(wd->back.content, &x, &y, &w, &h);
             evas_map_util_points_populate_from_geometry(mb, x, y, w, h, 0);
             evas_map_point_image_uv_set(mb, 0, 0, 0);
             evas_map_point_image_uv_set(mb, 1, iw, 0);
             evas_map_point_image_uv_set(mb, 2, iw, ih);
             evas_map_point_image_uv_set(mb, 3, 0, ih);
          }
        else
          {
             evas_object_geometry_get(wd->back.content, &x, &y, &w, &h);
             evas_map_util_points_populate_from_geometry(mb, x, y, w, h, 0);
          }
     }

   evas_object_geometry_get(obj, &x, &y, &w, &h);

   cx = x + (w / 2);
   cy = y + (h / 2);

   px = x + (w / 2);
   py = y + (h / 2);
   foc = 2048;

   lx = cx;
   ly = cy;
   lz = -10000;
   lr = 255;
   lg = 255;
   lb = 255;
   lar = 0;
   lag = 0;
   lab = 0;

   switch (mode)
     {
      case ELM_FLIP_ROTATE_Y_CENTER_AXIS:
         p = 1.0 - t;
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
         if (wd->state) deg = 180.0 * p;
         else deg = 180 + (180.0 * p);
         if (rev) deg = -deg;
         evas_map_util_3d_rotate(mf, 0.0, deg, 0.0, cx, cy, 0);
         evas_map_util_3d_rotate(mb, 0.0, 180 + deg, 0.0, cx, cy, 0);
         break;
      case ELM_FLIP_ROTATE_X_CENTER_AXIS:
         p = 1.0 - t;
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
         if (wd->state) deg = 180.0 * p;
         else deg = 180 + (180.0 * p);
         if (rev) deg = -deg;
         evas_map_util_3d_rotate(mf, deg, 0.0, 0.0, cx, cy, 0);
         evas_map_util_3d_rotate(mb, 180.0 + deg, 0.0, 0.0, cx, cy, 0);
         break;
      case ELM_FLIP_ROTATE_XZ_CENTER_AXIS:
         p = 1.0 - t;
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
         if (wd->state) deg = 180.0 * p;
         else deg = 180 + (180.0 * p);
         if (rev) deg = -deg;
         evas_map_util_3d_rotate(mf, deg, 0.0, deg, cx, cy, 0);
         evas_map_util_3d_rotate(mb, 180 + deg, 0.0, 180 + deg, cx, cy, 0);
         break;
      case ELM_FLIP_ROTATE_YZ_CENTER_AXIS:
         p = 1.0 - t;
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
         if (wd->state) deg = 180.0 * p;
         else deg = 180 + (180.0 * p);
         if (rev) deg = -deg;
         evas_map_util_3d_rotate(mf, 0.0, deg, deg, cx, cy, 0);
         evas_map_util_3d_rotate(mb, 0.0, 180.0 + deg, 180.0 + deg, cx, cy, 0);
         break;
      case ELM_FLIP_CUBE_LEFT:
         p = 1.0 - t;
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
         deg = -90.0 * p;
         if (wd->state)
           {
              evas_map_util_3d_rotate(mf, 0.0, deg, 0.0, cx, cy, w / 2);
              evas_map_util_3d_rotate(mb, 0.0, deg + 90, 0.0, cx, cy, w / 2);
           }
         else
           {
              evas_map_util_3d_rotate(mf, 0.0, deg + 90, 0.0, cx, cy, w / 2);
              evas_map_util_3d_rotate(mb, 0.0, deg, 0.0, cx, cy, w / 2);
           }
         break;
      case ELM_FLIP_CUBE_RIGHT:
         p = 1.0 - t;
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
         deg = 90.0 * p;
         if (wd->state)
           {
              evas_map_util_3d_rotate(mf, 0.0, deg, 0.0, cx, cy, w / 2);
              evas_map_util_3d_rotate(mb, 0.0, deg - 90, 0.0, cx, cy, w / 2);
           }
         else
           {
              evas_map_util_3d_rotate(mf, 0.0, deg - 90, 0.0, cx, cy, w / 2);
              evas_map_util_3d_rotate(mb, 0.0, deg, 0.0, cx, cy, w / 2);
           }
         break;
      case ELM_FLIP_CUBE_UP:
         p = 1.0 - t;
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
         deg = -90.0 * p;
         if (wd->state)
           {
              evas_map_util_3d_rotate(mf, deg, 0.0, 0.0, cx, cy, h / 2);
              evas_map_util_3d_rotate(mb, deg + 90, 0.0, 0.0, cx, cy, h / 2);
           }
         else
           {
              evas_map_util_3d_rotate(mf, deg + 90, 0.0, 0.0, cx, cy, h / 2);
              evas_map_util_3d_rotate(mb, deg, 0.0, 0.0, cx, cy, h / 2);
           }
         break;
      case ELM_FLIP_CUBE_DOWN:
         p = 1.0 - t;
         pp = p;
         if (!lin) pp = (p * p);
         p = 1.0 - pp;
         deg = 90.0 * p;
         if (wd->state)
           {
              evas_map_util_3d_rotate(mf, deg, 0.0, 0.0, cx, cy, h / 2);
              evas_map_util_3d_rotate(mb, deg - 90, 0.0, 0.0, cx, cy, h / 2);
           }
         else
           {
              evas_map_util_3d_rotate(mf, deg - 90, 0.0, 0.0, cx, cy, h / 2);
              evas_map_util_3d_rotate(mb, deg, 0.0, 0.0, cx, cy, h / 2);
           }
         break;
      case ELM_FLIP_PAGE_LEFT:
        break;
      case ELM_FLIP_PAGE_RIGHT:
        break;
      case ELM_FLIP_PAGE_UP:
        break;
      case ELM_FLIP_PAGE_DOWN:
        break;
      default:
         break;
     }


   if (wd->front.content)
     {
        evas_map_util_3d_lighting(mf, lx, ly, lz, lr, lg, lb, lar, lag, lab);
        evas_map_util_3d_perspective(mf, px, py, 0, foc);
        evas_object_map_set(wd->front.content, mf);
        evas_object_map_enable_set(wd->front.content, 1);
        if (evas_map_util_clockwise_get(mf)) evas_object_show(wd->front.clip);
        else evas_object_hide(wd->front.clip);
     }

   if (wd->back.content)
     {
        evas_map_util_3d_lighting(mb, lx, ly, lz, lr, lg, lb, lar, lag, lab);
        evas_map_util_3d_perspective(mb, px, py, 0, foc);
        evas_object_map_set(wd->back.content, mb);
        evas_object_map_enable_set(wd->back.content, 1);
        if (evas_map_util_clockwise_get(mb)) evas_object_show(wd->back.clip);
        else evas_object_hide(wd->back.clip);
     }

   evas_map_free(mf);
   evas_map_free(mb);
}

static void
_showhide(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord x, y, w, h;
   if (!wd) return;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   if (wd->front.content)
     {
        if ((wd->pageflip) && (wd->state))
          {
             evas_object_move(wd->front.content, 4999, 4999);
          }
        else
          {
             if (!wd->animator)
                evas_object_move(wd->front.content, x, y);
          }
        evas_object_resize(wd->front.content, w, h);
     }
   if (wd->back.content)
     {
        if ((wd->pageflip) && (!wd->state))
          {
             evas_object_move(wd->back.content, 4999, 4999);
          }
        else
          {
             if (!wd->animator)
                evas_object_move(wd->back.content, x, y);
          }
        evas_object_resize(wd->back.content, w, h);
     }

}

static Eina_Bool
_flip(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   double t = ecore_loop_time_get() - wd->start;
   Evas_Coord w, h;

   if (!wd) return ECORE_CALLBACK_CANCEL;
   if (!wd->animator) return ECORE_CALLBACK_CANCEL;

   t = t / wd->len;
   if (t > 1.0) t = 1.0;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (wd->mode == ELM_FLIP_PAGE_LEFT)
     {
        wd->dir = 0;
        wd->started = EINA_TRUE;
        wd->pageflip = EINA_TRUE;
        wd->down_x = w - 1;
        wd->down_y = h / 2;
        wd->x = (1.0 - t) * wd->down_x;
        wd->y = wd->down_y;
        flip_show_hide(obj);
        _state_update(wd);
     }
   else if (wd->mode == ELM_FLIP_PAGE_RIGHT)
     {
        wd->dir = 1;
        wd->started = EINA_TRUE;
        wd->pageflip = EINA_TRUE;
        wd->down_x = 0;
        wd->down_y = h / 2;
        wd->x = (t) * w;
        wd->y = wd->down_y;
        flip_show_hide(obj);
        _state_update(wd);
     }
   else if (wd->mode == ELM_FLIP_PAGE_UP)
     {
        wd->dir = 2;
        wd->started = EINA_TRUE;
        wd->pageflip = EINA_TRUE;
        wd->down_x = w / 2;
        wd->down_y = h - 1;
        wd->x = wd->down_x;
        wd->y = (1.0 - t) * wd->down_y;
        flip_show_hide(obj);
        _state_update(wd);
     }
   else if (wd->mode == ELM_FLIP_PAGE_DOWN)
     {
        wd->dir = 3;
        wd->started = EINA_TRUE;
        wd->pageflip = EINA_TRUE;
        wd->down_x = w / 2;
        wd->down_y = 0;
        wd->x = wd->down_x;
        wd->y = (t) * h;
        flip_show_hide(obj);
        _state_update(wd);
     }
   else
      _flip_do(obj, t, wd->mode, 0, 0);

   if (t >= 1.0)
     {
        wd->pageflip = EINA_FALSE;
        _state_end(wd);
        evas_object_map_enable_set(wd->front.content, 0);
        evas_object_map_enable_set(wd->back.content, 0);
        // FIXME: hack around evas rendering bug (only fix makes evas bitch-slow
        evas_object_resize(wd->front.content, 0, 0);
        evas_object_resize(wd->back.content, 0, 0);
        evas_smart_objects_calculate(evas_object_evas_get(obj));
        // FIXME: end hack
        wd->animator = NULL;
        wd->state = !wd->state;
        _configure(obj);
        flip_show_hide(obj);
        evas_object_smart_callback_call(obj, SIG_ANIMATE_DONE, NULL);
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

static void
_configure(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord x, y, w, h;
   Evas_Coord fsize;
   if (!wd) return;

   _showhide(obj);
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   // FIXME: manual flip wont get fixed
   if (wd->animator) _flip(obj);

   if (wd->event[0])
     {
        fsize = (double)w * wd->dir_hitsize[0];
        elm_coords_finger_size_adjust(0, NULL, 1, &fsize);
        evas_object_move(wd->event[0], x, y);
        evas_object_resize(wd->event[0], w, fsize);
     }
   if (wd->event[1])
     {
        fsize = (double)w * wd->dir_hitsize[1];
        elm_coords_finger_size_adjust(0, NULL, 1, &fsize);
        evas_object_move(wd->event[1], x, y + h - fsize);
        evas_object_resize(wd->event[1], w, fsize);
     }
   if (wd->event[2])
     {
        fsize = (double)h * wd->dir_hitsize[2];
        elm_coords_finger_size_adjust(1, &fsize, 0, NULL);
        evas_object_move(wd->event[2], x, y);
        evas_object_resize(wd->event[2], fsize, h);
     }
   if (wd->event[3])
     {
        fsize = (double)h * wd->dir_hitsize[3];
        elm_coords_finger_size_adjust(1, &fsize, 0, NULL);
        evas_object_move(wd->event[3], x + w - fsize, y);
        evas_object_resize(wd->event[3], fsize, h);
     }
}

static void
_move(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   _configure(obj);
}

static void
_resize(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   _configure(obj);
}

static Eina_Bool
_animate(void *data)
{
   return _flip(data);
}

static double
_pos_get(Widget_Data *wd, int *rev, Elm_Flip_Mode *m)
{
   Evas_Coord x, y, w, h;
   double t = 1.0;

   evas_object_geometry_get(wd->obj, &x, &y, &w, &h);
   switch (wd->intmode)
     {
      case ELM_FLIP_INTERACTION_ROTATE:
      case ELM_FLIP_INTERACTION_CUBE:
          {
             if (wd->dir == 0)
               {
                  if (wd->down_x > 0)
                     t = 1.0 - ((double)wd->x / (double)wd->down_x);
                  *rev = 1;
               }
             else if (wd->dir == 1)
               {
                  if (wd->down_x < w)
                     t = 1.0 - ((double)(w - wd->x) / (double)(w - wd->down_x));
               }
             else if (wd->dir == 2)
               {
                  if (wd->down_y > 0)
                     t = 1.0 - ((double)wd->y / (double)wd->down_y);
               }
             else if (wd->dir == 3)
               {
                  if (wd->down_y < h)
                     t = 1.0 - ((double)(h - wd->y) / (double)(h - wd->down_y));
                  *rev = 1;
               }

             if (t < 0.0) t = 0.0;
             else if (t > 1.0) t = 1.0;

             if ((wd->dir == 0) || (wd->dir == 1))
               {
                  if (wd->intmode == ELM_FLIP_INTERACTION_ROTATE)
                     *m = ELM_FLIP_ROTATE_Y_CENTER_AXIS;
                  else if (wd->intmode == ELM_FLIP_INTERACTION_CUBE)
                    {
                       if (*rev)
                          *m = ELM_FLIP_CUBE_LEFT;
                       else
                          *m = ELM_FLIP_CUBE_RIGHT;
                    }
               }
             else
               {
                  if (wd->intmode == ELM_FLIP_INTERACTION_ROTATE)
                     *m = ELM_FLIP_ROTATE_X_CENTER_AXIS;
                  else if (wd->intmode == ELM_FLIP_INTERACTION_CUBE)
                    {
                       if (*rev)
                          *m = ELM_FLIP_CUBE_UP;
                       else
                          *m = ELM_FLIP_CUBE_DOWN;
                    }
               }
          }
      default:
        break;
     }
   return t;
}

static Eina_Bool
_event_anim(void *data, double pos)
{
   Widget_Data *wd = data;
   double p;

   p = ecore_animator_pos_map(pos, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);
   if (wd->finish)
     {
        if (wd->dir == 0)
           wd->x = wd->ox * (1.0 - p);
        else if (wd->dir == 1)
           wd->x = wd->ox + ((wd->w - wd->ox) * p);
        else if (wd->dir == 2)
           wd->y = wd->oy * (1.0 - p);
        else if (wd->dir == 3)
           wd->y = wd->oy + ((wd->h - wd->oy) * p);
     }
   else
     {
        if (wd->dir == 0)
           wd->x = wd->ox + ((wd->w - wd->ox) * p);
        else if (wd->dir == 1)
           wd->x = wd->ox * (1.0 - p);
        else if (wd->dir == 2)
           wd->y = wd->oy + ((wd->h - wd->oy) * p);
        else if (wd->dir == 3)
           wd->y = wd->oy * (1.0 - p);
     }
   switch (wd->intmode)
     {
      case ELM_FLIP_INTERACTION_NONE:
        break;
      case ELM_FLIP_INTERACTION_ROTATE:
      case ELM_FLIP_INTERACTION_CUBE:
          {
             Elm_Flip_Mode m = ELM_FLIP_ROTATE_X_CENTER_AXIS;
             int rev = 0;
             p = _pos_get(wd, &rev, &m);
             _flip_do(wd->obj, p, m, 1, rev);
          }
        break;
      case ELM_FLIP_INTERACTION_PAGE:
        wd->pageflip = EINA_TRUE;
        _configure(data);
        _state_update(wd);
        break;
      default:
        break;
     }
   if (pos < 1.0) return ECORE_CALLBACK_RENEW;

   wd->pageflip = EINA_FALSE;
   _state_end(wd);
   evas_object_map_enable_set(wd->front.content, 0);
   evas_object_map_enable_set(wd->back.content, 0);
   // FIXME: hack around evas rendering bug (only fix makes evas bitch-slow
   evas_object_resize(wd->front.content, 0, 0);
   evas_object_resize(wd->back.content, 0, 0);
   evas_smart_objects_calculate(evas_object_evas_get(wd->obj));
   // FIXME: end hack
   wd->animator = NULL;
   if (wd->finish) wd->state = !wd->state;
   flip_show_hide(wd->obj);
   _configure(wd->obj);
   wd->animator = NULL;
   evas_object_smart_callback_call(wd->obj, SIG_ANIMATE_DONE, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static void
_update_job(void *data)
{
   Widget_Data *wd = data;
   double p;
   Elm_Flip_Mode m = ELM_FLIP_ROTATE_X_CENTER_AXIS;
   int rev = 0;

   wd->job = NULL;
   switch (wd->intmode)
     {
      case ELM_FLIP_INTERACTION_ROTATE:
      case ELM_FLIP_INTERACTION_CUBE:
        p = _pos_get(wd, &rev, &m);
        _flip_do(wd->obj, p, m, 1, rev);
        break;
      case ELM_FLIP_INTERACTION_PAGE:
        wd->pageflip = EINA_TRUE;
        _configure(data);
        _state_update(wd);
        break;
      default:
        break;
     }
}

static void
_down_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Object *fl = data;
   Widget_Data *wd = elm_widget_data_get(fl);
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y, w, h;

   if (!wd) return;
   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return ;
   if (wd->animator)
     {
        ecore_animator_del(wd->animator);
        wd->animator = NULL;
     }
   wd->down = EINA_TRUE;
   wd->started = EINA_FALSE;
   evas_object_geometry_get(data, &x, &y, &w, &h);
   wd->x = ev->canvas.x - x;
   wd->y = ev->canvas.y - y;
   wd->w = w;
   wd->h = h;
   wd->down_x = wd->x;
   wd->down_y = wd->y;
}

static void
_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Object *fl = data;
   Widget_Data *wd = elm_widget_data_get(fl);
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Coord x, y, w, h;
   double tm = 0.5;

   if (!wd) return;
   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return ;
   wd->down = 0;
   evas_object_geometry_get(data, &x, &y, &w, &h);
   wd->x = ev->canvas.x - x;
   wd->y = ev->canvas.y - y;
   wd->w = w;
   wd->h = h;
   wd->ox = wd->x;
   wd->oy = wd->y;
   if (wd->job)
     {
        ecore_job_del(wd->job);
        wd->job = NULL;
     }
   wd->finish = EINA_FALSE;
   if (wd->dir == 0)
     {
        tm = (double)wd->x / (double)wd->w;
        if (wd->x < (wd->w / 2)) wd->finish = EINA_TRUE;
     }
   else if (wd->dir == 1)
     {
        if (wd->x > (wd->w / 2)) wd->finish = EINA_TRUE;
        tm = 1.0 - ((double)wd->x / (double)wd->w);
     }
   else if (wd->dir == 2)
     {
        if (wd->y < (wd->h / 2)) wd->finish = EINA_TRUE;
        tm = (double)wd->y / (double)wd->h;
     }
   else if (wd->dir == 3)
     {
        if (wd->y > (wd->h / 2)) wd->finish = EINA_TRUE;
        tm = 1.0 - ((double)wd->y / (double)wd->h);
     }
   if (tm < 0.01) tm = 0.01;
   else if (tm > 0.99) tm = 0.99;
   if (!wd->finish) tm = 1.0 - tm;
   tm *= 1.0; // FIXME: config for anim time
   if (wd->animator) ecore_animator_del(wd->animator);
   wd->animator = ecore_animator_timeline_add(tm, _event_anim, wd);
   _event_anim(wd, 0.0);
}

static void
_move_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Object *fl = data;
   Widget_Data *wd = elm_widget_data_get(fl);
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord x, y, w, h;

   if (!wd) return;
   if (!wd->down) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return ;
   evas_object_geometry_get(data, &x, &y, &w, &h);
   wd->x = ev->cur.canvas.x - x;
   wd->y = ev->cur.canvas.y - y;
   wd->w = w;
   wd->h = h;
   if (!wd->started)
     {
        Evas_Coord dx, dy;

        dx = wd->x - wd->down_x;
        dy = wd->y - wd->down_y;
        if (((dx * dx) + (dy * dy)) > (_elm_config->finger_size * _elm_config->finger_size / 4))
          {
             wd->dir = 0;
             if      ((wd->x > (w / 2)) && (dx <  0) && (abs(dx) >  abs(dy))) wd->dir = 0; // left
             else if ((wd->x < (w / 2)) && (dx >= 0) && (abs(dx) >  abs(dy))) wd->dir = 1; // right
             else if ((wd->y > (h / 2)) && (dy <  0) && (abs(dy) >= abs(dx))) wd->dir = 2; // up
             else if ((wd->y < (h / 2)) && (dy >= 0) && (abs(dy) >= abs(dx))) wd->dir = 3; // down
             wd->started = EINA_TRUE;
             if (wd->intmode == ELM_FLIP_INTERACTION_PAGE)
                wd->pageflip = EINA_TRUE;
             flip_show_hide(data);
             evas_smart_objects_calculate(evas_object_evas_get(data));
             _flip(data);
             // FIXME: hack around evas rendering bug (only fix makes evas bitch-slow)
             evas_object_map_enable_set(wd->front.content, 0);
             evas_object_map_enable_set(wd->back.content, 0);
// FIXME: XXX why does this bork interactive flip??
//             evas_object_resize(wd->front.content, 0, 0);
//             evas_object_resize(wd->back.content, 0, 0);
             evas_smart_objects_calculate(evas_object_evas_get(data));
             _configure(obj);
             // FIXME: end hack
             evas_object_smart_callback_call(obj, SIG_ANIMATE_BEGIN, NULL);
          }
        else return;
     }
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   if (wd->job) ecore_job_del(wd->job);
   wd->job = ecore_job_add(_update_job, wd);
}

EAPI Evas_Object *
elm_flip_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "flip");
   elm_widget_type_set(obj, "flip");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_focus_next_hook_set(obj, _elm_flip_focus_next_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   wd->obj = obj;

   wd->clip = evas_object_rectangle_add(e);
   evas_object_static_clip_set(wd->clip, 1);
   evas_object_color_set(wd->clip, 255, 255, 255, 255);
   evas_object_move(wd->clip, -49999, -49999);
   evas_object_resize(wd->clip, 99999, 99999);
   elm_widget_sub_object_add(obj, wd->clip);
   evas_object_clip_set(wd->clip, evas_object_clip_get(obj));
   evas_object_smart_member_add(wd->clip, obj);

   wd->front.clip = evas_object_rectangle_add(e);
   evas_object_static_clip_set(wd->front.clip, 1);
   evas_object_data_set(wd->front.clip, "_elm_leaveme", obj);
   evas_object_color_set(wd->front.clip, 255, 255, 255, 255);
   evas_object_move(wd->front.clip, -49999, -49999);
   evas_object_resize(wd->front.clip, 99999, 99999);
   elm_widget_sub_object_add(obj, wd->front.clip);
   evas_object_smart_member_add(wd->front.clip, obj);
   evas_object_clip_set(wd->front.clip, wd->clip);

   wd->back.clip = evas_object_rectangle_add(e);
   evas_object_static_clip_set(wd->back.clip, 1);
   evas_object_data_set(wd->back.clip, "_elm_leaveme", obj);
   evas_object_color_set(wd->back.clip, 255, 255, 255, 255);
   evas_object_move(wd->back.clip, -49999, -49999);
   evas_object_resize(wd->back.clip, 99999, 99999);
   elm_widget_sub_object_add(wd->back.clip, obj);
   evas_object_smart_member_add(obj, wd->back.clip);
   evas_object_clip_set(wd->back.clip, wd->clip);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _move, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, NULL);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   wd->state = 1;
   wd->intmode = ELM_FLIP_INTERACTION_NONE;

   _sizing_eval(obj);

   return obj;
}

EAPI void
elm_flip_content_front_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   int i;
   if (!wd) return;
   if (wd->front.content == content) return;
   if (wd->front.content) evas_object_del(wd->back.content);
   wd->front.content = content;
   if (content)
     {
        elm_widget_sub_object_add(obj, content);
        evas_object_smart_member_add(content, obj);
        evas_object_clip_set(content, wd->front.clip);
        evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        _sizing_eval(obj);
     }
   // force calc to contents are the right size before transition
   evas_smart_objects_calculate(evas_object_evas_get(obj));
   flip_show_hide(obj);
   _configure(obj);
   if (wd->intmode != ELM_FLIP_INTERACTION_NONE)
     {
        for (i = 0; i < 4; i++) evas_object_raise(wd->event[i]);
     }
}

EAPI void
elm_flip_content_back_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   int i;
   if (!wd) return;
   if (wd->back.content == content) return;
   if (wd->back.content) evas_object_del(wd->back.content);
   wd->back.content = content;
   if (content)
     {
        elm_widget_sub_object_add(obj, content);
        evas_object_smart_member_add(content, obj);
        evas_object_clip_set(content, wd->back.clip);
        evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        _sizing_eval(obj);
     }
   // force calc to contents are the right size before transition
   evas_smart_objects_calculate(evas_object_evas_get(obj));
   flip_show_hide(obj);
   _configure(obj);
   if (wd->intmode != ELM_FLIP_INTERACTION_NONE)
     {
        for (i = 0; i < 4; i++) evas_object_raise(wd->event[i]);
     }
}

EAPI Evas_Object *
elm_flip_content_front_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->front.content;
}


EAPI Evas_Object *
elm_flip_content_back_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->back.content;
}

EAPI Evas_Object *
elm_flip_content_front_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->front.content) return NULL;
   Evas_Object *content = wd->front.content;
   evas_object_clip_unset(content);
   elm_widget_sub_object_del(obj, content);
   evas_object_smart_member_del(content);
   wd->front.content = NULL;
   return content;
}

EAPI Evas_Object *
elm_flip_content_back_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->back.content) return NULL;
   Evas_Object *content = wd->back.content;
   evas_object_clip_unset(content);
   elm_widget_sub_object_del(obj, content);
   evas_object_smart_member_del(content);
   wd->back.content = NULL;
   return content;
}

EAPI Eina_Bool
elm_flip_front_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->state;
}

EAPI void
elm_flip_perspective_set(Evas_Object *obj, Evas_Coord foc __UNUSED__, Evas_Coord x __UNUSED__, Evas_Coord y __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
}

// FIXME: add ambient and lighting control

EAPI void
elm_flip_go(Evas_Object *obj, Elm_Flip_Mode mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!wd->animator) wd->animator = ecore_animator_add(_animate, obj);
   flip_show_hide(obj);
   wd->mode = mode;
   wd->start = ecore_loop_time_get();
   wd->len = 0.5; // FIXME: make config val
   if ((wd->mode == ELM_FLIP_PAGE_LEFT) ||
       (wd->mode == ELM_FLIP_PAGE_RIGHT) ||
       (wd->mode == ELM_FLIP_PAGE_UP) ||
       (wd->mode == ELM_FLIP_PAGE_DOWN))
      wd->pageflip = EINA_TRUE;
   // force calc to contents are the right size before transition
   evas_smart_objects_calculate(evas_object_evas_get(obj));
   _flip(obj);
   // FIXME: hack around evas rendering bug (only fix makes evas bitch-slow)
   evas_object_map_enable_set(wd->front.content, 0);
   evas_object_map_enable_set(wd->back.content, 0);
   evas_object_resize(wd->front.content, 0, 0);
   evas_object_resize(wd->back.content, 0, 0);
   evas_smart_objects_calculate(evas_object_evas_get(obj));
   _configure(obj);
   // FIXME: end hack
   evas_object_smart_callback_call(obj, SIG_ANIMATE_BEGIN, NULL);
}

EAPI void
elm_flip_interaction_set(Evas_Object *obj, Elm_Flip_Interaction mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   int i;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->intmode == mode) return;
   wd->intmode = mode;
   for (i = 0; i < 4; i++)
     {
        if (wd->intmode == ELM_FLIP_INTERACTION_NONE)
          {
             if (wd->event[i])
               {
                  evas_object_del(wd->event[i]);
                  wd->event[i] = NULL;
               }
          }
        else
          {
             if ((wd->dir_enabled[i]) && (!wd->event[i]))
               {
                  wd->event[i] = evas_object_rectangle_add(evas_object_evas_get(obj));
                  elm_widget_sub_object_add(obj, wd->event[i]);
                  evas_object_clip_set(wd->event[i], evas_object_clip_get(obj));
                  evas_object_color_set(wd->event[i], 0, 0, 0, 0);
                  evas_object_show(wd->event[i]);
                  evas_object_smart_member_add(wd->event[i], obj);
                  evas_object_event_callback_add(wd->event[i], EVAS_CALLBACK_MOUSE_DOWN, _down_cb, obj);
                  evas_object_event_callback_add(wd->event[i], EVAS_CALLBACK_MOUSE_UP, _up_cb, obj);
                  evas_object_event_callback_add(wd->event[i], EVAS_CALLBACK_MOUSE_MOVE, _move_cb, obj);
               }
          }
     }
   _sizing_eval(obj);
   _configure(obj);
}

EAPI Elm_Flip_Interaction
elm_flip_interaction_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_FLIP_INTERACTION_NONE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_FLIP_INTERACTION_NONE;
   return wd->intmode;
}

EAPI void
elm_flip_interacton_direction_enabled_set(Evas_Object *obj, Elm_Flip_Direction dir, Eina_Bool enabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   int i = -1;
   if (!wd) return;
   enabled = !!enabled;
   if      (dir == ELM_FLIP_DIRECTION_UP)    i = 0;
   else if (dir == ELM_FLIP_DIRECTION_DOWN)  i = 1;
   else if (dir == ELM_FLIP_DIRECTION_LEFT)  i = 2;
   else if (dir == ELM_FLIP_DIRECTION_RIGHT) i = 3;
   if (i < 0) return;
   if (wd->dir_enabled[i] == enabled) return;
   wd->dir_enabled[i] = enabled;
   if (wd->intmode == ELM_FLIP_INTERACTION_NONE) return;
   if ((wd->dir_enabled[i]) && (!wd->event[i]))
     {
        wd->event[i] = evas_object_rectangle_add(evas_object_evas_get(obj));
        elm_widget_sub_object_add(obj, wd->event[i]);
        evas_object_clip_set(wd->event[i], evas_object_clip_get(obj));
        evas_object_color_set(wd->event[i], 0, 0, 0, 0);
        evas_object_show(wd->event[i]);
        evas_object_smart_member_add(wd->event[i], obj);
        evas_object_event_callback_add(wd->event[i], EVAS_CALLBACK_MOUSE_DOWN, _down_cb, obj);
        evas_object_event_callback_add(wd->event[i], EVAS_CALLBACK_MOUSE_UP, _up_cb, obj);
        evas_object_event_callback_add(wd->event[i], EVAS_CALLBACK_MOUSE_MOVE, _move_cb, obj);
     }
   else if (!(wd->dir_enabled[i]) && (wd->event[i]))
     {
        evas_object_del(wd->event[i]);
        wd->event[i] = NULL;
     }
   _sizing_eval(obj);
   _configure(obj);
}

EAPI Eina_Bool
elm_flip_interacton_direction_enabled_get(Evas_Object *obj, Elm_Flip_Direction dir)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   int i = -1;
   if (!wd) return EINA_FALSE;
   if      (dir == ELM_FLIP_DIRECTION_UP)    i = 0;
   else if (dir == ELM_FLIP_DIRECTION_DOWN)  i = 1;
   else if (dir == ELM_FLIP_DIRECTION_LEFT)  i = 2;
   else if (dir == ELM_FLIP_DIRECTION_RIGHT) i = 3;
   if (i < 0) return EINA_FALSE;
   return wd->dir_enabled[i];
}

EAPI void
elm_flip_interacton_direction_hitsize_set(Evas_Object *obj, Elm_Flip_Direction dir, double hitsize)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   int i = -1;
   if (!wd) return;
   if      (dir == ELM_FLIP_DIRECTION_UP)    i = 0;
   else if (dir == ELM_FLIP_DIRECTION_DOWN)  i = 1;
   else if (dir == ELM_FLIP_DIRECTION_LEFT)  i = 2;
   else if (dir == ELM_FLIP_DIRECTION_RIGHT) i = 3;
   if (i < 0) return;
   if (hitsize < 0.0) hitsize = 0.0;
   else if (hitsize > 1.0) hitsize = 1.0;
   if (wd->dir_hitsize[i] == hitsize) return;
   wd->dir_hitsize[i] = hitsize;
   _sizing_eval(obj);
   _configure(obj);
}

EAPI double
elm_flip_interacton_direction_hitsize_get(Evas_Object *obj, Elm_Flip_Direction dir)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   int i = -1;
   if (!wd) return 0.0;
   if      (dir == ELM_FLIP_DIRECTION_UP)    i = 0;
   else if (dir == ELM_FLIP_DIRECTION_DOWN)  i = 1;
   else if (dir == ELM_FLIP_DIRECTION_LEFT)  i = 2;
   else if (dir == ELM_FLIP_DIRECTION_RIGHT) i = 3;
   if (i < 0) return 0.0;
   return wd->dir_hitsize[i];
}

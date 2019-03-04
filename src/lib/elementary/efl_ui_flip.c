#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_flip.h"
#include "elm_widget_container.h"

#include "efl_ui_flip_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_FLIP_CLASS
#define MY_CLASS_NAME "Efl.Ui.Flip"

static const char SIG_ANIMATE_BEGIN[] = "animate,begin";
static const char SIG_ANIMATE_DONE[] = "animate,done";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_ANIMATE_BEGIN, ""},
   {SIG_ANIMATE_DONE, ""},
   {NULL, NULL}
};

static Eina_Bool _flip(Evas_Object *obj);

static void _update_front_back(Eo *obj, Efl_Ui_Flip_Data *sd);

static void
_slice_free(Slice *sl)
{
   evas_object_del(sl->obj);
   free(sl);
}

static void
_state_slices_clear(Efl_Ui_Flip_Data *sd)
{
   int i, j, num;

   if (sd->slices)
     {
        num = 0;
        for (j = 0; j < sd->slices_h; j++)
          {
             for (i = 0; i < sd->slices_w; i++)
               {
                  if (sd->slices[num]) _slice_free(sd->slices[num]);
                  if (sd->slices2[num]) _slice_free(sd->slices2[num]);
                  num++;
               }
          }

        free(sd->slices);
        free(sd->slices2);
        sd->slices = NULL;
        sd->slices2 = NULL;
     }

   sd->slices_w = 0;
   sd->slices_h = 0;
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, minw2 = -1, minh2 = -1;
   Evas_Coord maxw = -1, maxh = -1, maxw2 = -1, maxh2 = -1;
   int fingx = 0, fingy = 0;

   EFL_UI_FLIP_DATA_GET(obj, sd);

   if (sd->front.content)
     evas_object_size_hint_combined_min_get(sd->front.content, &minw, &minh);
   if (sd->back.content)
     evas_object_size_hint_combined_min_get(sd->back.content, &minw2, &minh2);
   if (sd->front.content)
     evas_object_size_hint_max_get(sd->front.content, &maxw, &maxh);
   if (sd->back.content)
     evas_object_size_hint_max_get(sd->back.content, &maxw2, &maxh2);

   if (minw2 > minw) minw = minw2;
   if (minh2 > minh) minh = minh2;
   if ((maxw2 >= 0) && (maxw2 < maxw)) maxw = maxw2;
   if ((maxh2 >= 0) && (maxh2 < maxh)) maxh = maxh2;

   if (sd->dir_enabled[ELM_FLIP_DIRECTION_UP]) fingy++;
   if (sd->dir_enabled[ELM_FLIP_DIRECTION_DOWN]) fingy++;
   if (sd->dir_enabled[ELM_FLIP_DIRECTION_LEFT]) fingx++;
   if (sd->dir_enabled[ELM_FLIP_DIRECTION_RIGHT]) fingx++;

   elm_coords_finger_size_adjust(fingx, &minw, fingy, &minh);

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

EOLIAN static Efl_Ui_Theme_Apply_Error
_efl_ui_flip_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Flip_Data *sd EINA_UNUSED)
{
   Efl_Ui_Theme_Apply_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   _sizing_eval(obj);

   return int_ret;
}

static void
_changed_size_hints_cb(void *data,
                       Evas *e EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   _sizing_eval(data);
}

EOLIAN static Eina_Bool
_efl_ui_flip_efl_ui_widget_widget_sub_object_add(Eo *obj, Efl_Ui_Flip_Data *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   if (evas_object_data_get(sobj, "elm-parent") == obj)
     return EINA_TRUE;

   int_ret = elm_widget_sub_object_add(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   evas_object_data_set(sobj, "_elm_leaveme", sobj);
   evas_object_smart_member_add(sobj, obj);
   evas_object_event_callback_add
     (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints_cb, obj);
   evas_object_lower(sobj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_flip_efl_ui_widget_widget_sub_object_del(Eo *obj, Efl_Ui_Flip_Data *sd, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;


   int_ret = elm_widget_sub_object_del(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   if (sobj == sd->front.content)
     {
        evas_object_data_del(sobj, "_elm_leaveme");
        sd->front.content = NULL;
        evas_object_hide(sd->front.clip);
     }
   else if (sobj == sd->back.content)
     {
        evas_object_data_del(sobj, "_elm_leaveme");
        sd->back.content = NULL;
        evas_object_hide(sd->back.clip);
     }

   evas_object_smart_member_del(sobj);
   evas_object_clip_unset(sobj);

   evas_object_event_callback_del_full
     (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints_cb, obj);
   _sizing_eval(obj);

   return EINA_TRUE;
}

static Slice *
_slice_new(Evas_Object *container_obj,
           Evas_Object *obj)
{
   Slice *sl;

   sl = calloc(1, sizeof(Slice));
   if (!sl) return NULL;

   sl->obj = evas_object_image_add(evas_object_evas_get(obj));

   evas_object_smart_member_add(sl->obj, container_obj);

   evas_object_image_smooth_scale_set(sl->obj, EINA_FALSE);
   evas_object_pass_events_set(sl->obj, EINA_TRUE);
   evas_object_image_source_set(sl->obj, obj);

   return sl;
}

static void
_slice_apply(Efl_Ui_Flip_Data *sd,
             Slice *sl,
             Evas_Coord x EINA_UNUSED,
             Evas_Coord y EINA_UNUSED,
             Evas_Coord w,
             Evas_Coord h EINA_UNUSED,
             Evas_Coord ox,
             Evas_Coord oy,
             Evas_Coord ow,
             Evas_Coord oh)
{
   static Evas_Map *m = NULL;
   int i;

   if (!m) m = evas_map_new(4);
   if (!m) return;

   evas_map_smooth_set(m, EINA_FALSE);
   for (i = 0; i < 4; i++)
     {
        evas_map_point_color_set(m, i, 255, 255, 255, 255);
        if (sd->dir == ELM_FLIP_DIRECTION_LEFT)
          {
             int p[4] = { 0, 1, 2, 3 };
             evas_map_point_coord_set(m, i, ox + sl->x[p[i]], oy + sl->y[p[i]],
                                      sl->z[p[i]]);
             evas_map_point_image_uv_set(m, i, sl->u[p[i]], sl->v[p[i]]);
          }
        else if (sd->dir == ELM_FLIP_DIRECTION_RIGHT)
          {
             int p[4] = { 1, 0, 3, 2 };
             evas_map_point_coord_set(m, i, ox + (w - sl->x[p[i]]),
                                      oy + sl->y[p[i]], sl->z[p[i]]);
             evas_map_point_image_uv_set(m, i, ow - sl->u[p[i]], sl->v[p[i]]);
          }
        else if (sd->dir == ELM_FLIP_DIRECTION_UP)
          {
             int p[4] = { 1, 0, 3, 2 };
             evas_map_point_coord_set(m, i, ox + sl->y[p[i]], oy + sl->x[p[i]],
                                      sl->z[p[i]]);
             evas_map_point_image_uv_set(m, i, sl->v[p[i]], sl->u[p[i]]);
          }
        else /* if (sd->dir == 3) will be this anyway */
          {
             int p[4] = { 0, 1, 2, 3 };
             evas_map_point_coord_set(m, i, ox + sl->y[p[i]],
                                      oy + (w - sl->x[p[i]]), sl->z[p[i]]);
             evas_map_point_image_uv_set(m, i, sl->v[p[i]], oh - sl->u[p[i]]);
          }
     }

   evas_object_map_enable_set(sl->obj, EINA_TRUE);
   evas_object_image_fill_set(sl->obj, 0, 0, ow, oh);
   evas_object_map_set(sl->obj, m);
}

static void
_slice_3d(Efl_Ui_Flip_Data *sd EINA_UNUSED,
          Slice *sl,
          Evas_Coord x,
          Evas_Coord y,
          Evas_Coord w,
          Evas_Coord h)
{
   Evas_Map *m = evas_map_dup(evas_object_map_get(sl->obj));
   int i;

   if (!m) return;

   // vanishing point is center of page, and focal dist is 1024
   evas_map_util_3d_perspective(m, x + (w / 2), y + (h / 2), 0, 1024);
   for (i = 0; i < 4; i++)
     {
        Evas_Coord xx, yy;
        evas_map_point_coord_get(m, i, &xx, &yy, NULL);
        evas_map_point_coord_set(m, i, xx, yy, 0);
     }

   if (evas_map_util_clockwise_get(m)) evas_object_show(sl->obj);
   else evas_object_hide(sl->obj);

   evas_object_map_set(sl->obj, m);
   evas_map_free(m);
}

static void
_slice_light(Efl_Ui_Flip_Data *sd EINA_UNUSED,
             Slice *sl,
             Evas_Coord x,
             Evas_Coord y,
             Evas_Coord w,
             Evas_Coord h)
{
   Evas_Map *m = (Evas_Map *)evas_object_map_get(sl->obj);
   int i;

   if (!m) return;

   evas_map_util_3d_lighting(m,
     /* light position
      * (centered over page 10 * h toward camera) */
                             x + (w / 2), y + (h / 2), -10000,
                             255, 255, 255, // light color
                             0, 0, 0);   // ambient minimum

   // multiply brightness by 1.2 to make lightish bits all white so we don't
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
_slice_xyz(Efl_Ui_Flip_Data *sd EINA_UNUSED,
           Slice *sl,
           double xx1,
           double yy1,
           double zz1,
           double xx2,
           double yy2,
           double zz2,
           double xx3,
           double yy3,
           double zz3,
           double xx4,
           double yy4,
           double zz4)
{
   sl->x[0] = xx1; sl->y[0] = yy1; sl->z[0] = zz1;
   sl->x[1] = xx2; sl->y[1] = yy2; sl->z[1] = zz2;
   sl->x[2] = xx3; sl->y[2] = yy3; sl->z[2] = zz3;
   sl->x[3] = xx4; sl->y[3] = yy4; sl->z[3] = zz4;
}

static void
_slice_uv(Efl_Ui_Flip_Data *sd EINA_UNUSED,
          Slice *sl,
          double u1,
          double v1,
          double u2,
          double v2,
          double u3,
          double v3,
          double u4,
          double v4)
{
   sl->u[0] = u1; sl->v[0] = v1;
   sl->u[1] = u2; sl->v[1] = v2;
   sl->u[2] = u3; sl->v[2] = v3;
   sl->u[3] = u4; sl->v[3] = v4;
}

static void
_deform_point(Vertex2 *vi,
              Vertex3 *vo,
              double rho,
              double theta,
              double A)
{
   // ^Y
   // |
   // |    X
   // +---->
   // theta == cone angle (0 -> PI/2)
   // A     == distance of cone apex from origin
   // rho   == angle of cone from vertical axis (...-PI/2 to PI/2...)
   Vertex3 v1;
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
_interp_point(Vertex3 *vi1,
              Vertex3 *vi2,
              Vertex3 *vo,
              double v)
{
   vo->x = (v * vi2->x) + ((1.0 - v) * vi1->x);
   vo->y = (v * vi2->y) + ((1.0 - v) * vi1->y);
   vo->z = (v * vi2->z) + ((1.0 - v) * vi1->z);
}

static int
_slice_obj_color_sum(Slice *s,
                     int p,
                     int *r,
                     int *g,
                     int *b,
                     int *a)
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
_slice_obj_color_set(Slice *s,
                     int p,
                     int r,
                     int g,
                     int b,
                     int a)
{
   Evas_Map *m;

   if (!s) return;

   m = (Evas_Map *)evas_object_map_get(s->obj);
   if (!m) return;

   evas_map_point_color_set(m, p, r, g, b, a);
   evas_object_map_set(s->obj, m);
}

static void
_slice_obj_vert_color_merge(Slice *s1,
                            int p1,
                            Slice *s2,
                            int p2,
                            Slice *s3,
                            int p3,
                            Slice *s4,
                            int p4)
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
_state_update(Evas_Object *obj)
{
   Efl_Ui_Flip_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   Slice *sl;
   Vertex3 *tvo, *tvol;
   Evas_Object *front, *back;
   int i, j, num, nn, jump, num2;
   double b, minv = 0.0, minva, mgrad;
   Evas_Coord xx1, yy1, xx2, yy2, mx, my;
   Evas_Coord x, y, w, h, ox, oy, ow, oh;
   int gx, gy, gszw, gszh, gw, gh, col, row, nw, nh;
   double rho, A, theta, perc, n, rhol, Al, thetal;

   sd->backflip = EINA_TRUE;
   if (sd->state)
     {
        front = sd->front.content;
        back = sd->front.content;
     }
   else
     {
        front = sd->back.content;
        back = sd->back.content;
     }

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   ox = x; oy = y; ow = w; oh = h;
   xx1 = sd->down_x;
   yy1 = sd->down_y;
   xx2 = sd->x;
   yy2 = sd->y;

   if (sd->dir == ELM_FLIP_DIRECTION_LEFT)
     {
        // no nothing. left drag is standard
     }
   else if (sd->dir == ELM_FLIP_DIRECTION_RIGHT)
     {
        xx1 = (w - 1) - xx1;
        xx2 = (w - 1) - xx2;
     }
   else if (sd->dir == ELM_FLIP_DIRECTION_UP)
     {
        Evas_Coord tmp;

        tmp = xx1; xx1 = yy1; yy1 = tmp;
        tmp = xx2; xx2 = yy2; yy2 = tmp;
        tmp = w; w = h; h = tmp;
     }
   else /* if (sd->dir == 3) will be this anyway */
     {
        Evas_Coord tmp;

        tmp = xx1; xx1 = yy1; yy1 = tmp;
        tmp = xx2; xx2 = yy2; yy2 = tmp;
        tmp = w; w = h; h = tmp;
        xx1 = (w - 1) - xx1;
        xx2 = (w - 1) - xx2;
     }

   if (xx2 >= xx1) xx2 = xx1 - 1;
   mx = (xx1 + xx2) / 2;
   my = (yy1 + yy2) / 2;

   if (mx < 0) mx = 0;
   else if (mx >= w)
     mx = w - 1;
   if (my < 0) my = 0;
   else if (my >= h)
     my = h - 1;

   mgrad = (double)(yy1 - yy2) / (double)(xx1 - xx2);

   if (mx < 1) mx = 1;  // quick hack to keep curl line visible

   if (mgrad == 0.0) // special horizontal case
     mgrad = 0.001;  // quick dirty hack for now
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

   perc = (double)xx2 / (double)xx1;
   if (perc < 0.0) perc = 0.0;
   else if (perc > 1.0)
     perc = 1.0;

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

   // theta == curliness (how much page curls in on itself
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
   gszw = w / nw;
   gszh = h / nh;
   if (gszw < 4) gszw = 4;
   if (gszh < 4) gszh = 4;

   nw = (w + gszw - 1) / gszw;
   nh = (h + gszh - 1) / gszh;
   if ((sd->slices_w != nw) || (sd->slices_h != nh)) _state_slices_clear(sd);
   sd->slices_w = nw;
   sd->slices_h = nh;
   if (!sd->slices)
     {
        sd->slices = calloc(sd->slices_w * sd->slices_h, sizeof(Slice *));
        if (!sd->slices) return 0;
        sd->slices2 = calloc(sd->slices_w * sd->slices_h, sizeof(Slice *));
        if (!sd->slices2)
          {
             ELM_SAFE_FREE(sd->slices, free);
             return 0;
          }
     }

   num = (sd->slices_w + 1) * (sd->slices_h + 1);

   tvo = alloca(sizeof(Vertex3) * num);
   tvol = alloca(sizeof(Vertex3) * (sd->slices_w + 1));

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

   jump = sd->slices_h + 1;
   for (col = 0, gx = 0; gx < w; gx += gszw, col++)
     {
        num = sd->slices_h * col;
        num2 = jump * col;

        gw = gszw;
        if ((gx + gw) > w) gw = w - gx;

        for (row = 0, gy = 0; gy < h; gy += gszh, row++)
          {
             Vertex3 vo[4];

             memset(vo, 0, sizeof (vo));

             if (b > 0) nn = num + sd->slices_h - row - 1;
             else nn = num + row;

             gh = gszh;
             if ((gy + gh) > h) gh = h - gy;

             vo[0] = tvo[num2 + row];
             vo[1] = tvo[num2 + row + jump];
             vo[2] = tvo[num2 + row + jump + 1];
             vo[3] = tvo[num2 + row + 1];

#define SWP(a, b)   \
  do {              \
       typeof(a)vt; \
       vt = (a);    \
       (a) = (b);   \
       (b) = vt;    \
    } while (0)

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
             sl = sd->slices[nn];
             if (!sl)
               {
                  sl = _slice_new(obj, front);
                  if (!sl) return 0;
                  sd->slices[nn] = sl;
               }
             _slice_xyz(sd, sl,
                        vo[0].x, vo[0].y, vo[0].z,
                        vo[1].x, vo[1].y, vo[1].z,
                        vo[2].x, vo[2].y, vo[2].z,
                        vo[3].x, vo[3].y, vo[3].z);
             if (b <= 0)
               _slice_uv(sd, sl,
                         gx, gy, gx + gw, gy, gx + gw, gy + gh, gx, gy + gh);
             else
               _slice_uv(sd, sl,
                         gx, h - (gy + gh), gx + gw, h - (gy + gh), gx + gw,
                         h - gy, gx, h - gy);

             // BACK
             sl = sd->slices2[nn];
             if (!sl)
               {
                  sl = _slice_new(obj, back);
                  if (!sl) return 0;
                  sd->slices2[nn] = sl;
               }

             _slice_xyz(sd, sl,
                        vo[1].x, vo[1].y, vo[1].z,
                        vo[0].x, vo[0].y, vo[0].z,
                        vo[3].x, vo[3].y, vo[3].z,
                        vo[2].x, vo[2].y, vo[2].z);
             if (sd->backflip)
               {
                  if (b <= 0)
                    _slice_uv(sd, sl, gx + gw, gy, gx, gy, gx, gy + gh, gx + gw,
                              gy + gh);
                  else
                    _slice_uv(sd, sl, gx + gw, h - (gy + gh), gx, h - (gy + gh),
                              gx, h - gy, gx + gw, h - gy);
               }
             else
               {
                  if (b <= 0)
                    _slice_uv(sd, sl, w - (gx + gw), gy, w - (gx), gy, w - (gx),
                              gy + gh, w - (gx + gw), gy + gh);
                  else
                    _slice_uv(sd, sl, w - (gx + gw), h - (gy + gh), w - (gx),
                              h - (gy + gh), w - (gx), h - gy, w - (gx + gw),
                              h - gy);
               }
          }
     }

   for (num = 0; num < sd->slices_h * sd->slices_w; num++)
     {
        _slice_apply(sd, sd->slices[num], x, y, w, h, ox, oy, ow, oh);
        _slice_apply(sd, sd->slices2[num], x, y, w, h, ox, oy, ow, oh);
        _slice_light(sd, sd->slices[num], ox, oy, ow, oh);
        _slice_light(sd, sd->slices2[num], ox, oy, ow, oh);
     }

   for (i = 0; i <= sd->slices_w; i++)
     {
        num = i * sd->slices_h;
        for (j = 0; j <= sd->slices_h; j++)
          {
             Slice *s[4] = { NULL }, *s2[4] = { NULL };

             if ((i > 0) && (j > 0))
               s[0] = sd->slices[num - 1 - sd->slices_h],
               s2[0] = sd->slices2[num - 1 - sd->slices_h];
             if ((i < sd->slices_w) && (j > 0))
               s[1] = sd->slices[num - 1],
               s2[1] = sd->slices2[num - 1];
             if ((i > 0) && (j < sd->slices_h))
               s[2] = sd->slices[num - sd->slices_h],
               s2[2] = sd->slices2[num - sd->slices_h];
             if ((i < sd->slices_w) && (j < sd->slices_h))
               s[3] = sd->slices[num],
               s2[3] = sd->slices2[num];
             switch (sd->dir)
               {
                case ELM_FLIP_DIRECTION_LEFT:
                  _slice_obj_vert_color_merge
                    (s[0], 2, s[1], 3, s[2], 1, s[3], 0);
                  _slice_obj_vert_color_merge
                    (s2[0], 3, s2[1], 2, s2[2], 0, s2[3], 1);
                  break;

                case ELM_FLIP_DIRECTION_RIGHT:
                  _slice_obj_vert_color_merge
                    (s[0], 3, s[1], 2, s[2], 0, s[3], 1);
                  _slice_obj_vert_color_merge
                    (s2[0], 2, s2[1], 3, s2[2], 1, s2[3], 0);
                  break;

                case ELM_FLIP_DIRECTION_UP:
                  _slice_obj_vert_color_merge
                    (s[0], 3, s[1], 2, s[2], 0, s[3], 1);
                  _slice_obj_vert_color_merge
                    (s2[0], 2, s2[1], 3, s2[2], 1, s2[3], 0);
                  break;

                default:
                  _slice_obj_vert_color_merge
                    (s[0], 2, s[1], 3, s[2], 1, s[3], 0);
                  _slice_obj_vert_color_merge
                    (s2[0], 3, s2[1], 2, s2[2], 0, s2[3], 1);
               }
             num++;
          }
     }

   for (num = 0; num < sd->slices_h * sd->slices_w; num++)
     {
        _slice_3d(sd, sd->slices[num], ox, oy, ow, oh);
        _slice_3d(sd, sd->slices2[num], ox, oy, ow, oh);
     }

   return 1;
}

static void
_cross_fade_update(Evas_Object *obj, double t)
{
   int ca, cb;
   Evas_Object *aclip, *bclip;
   Eina_Bool front;
   double s;
   EFL_UI_FLIP_DATA_GET(obj, sd);
   front = sd->next_state;

   s = sin(t * M_PI_2);  // fade in sinusoidally
   t = s * s;
   ca = 255 * t;
   if (ca < 0) ca = 0;
   if (ca > 255) ca = 255;

   cb = sqrt(255 * 255 - ca * ca);
   if (cb < 0) cb = 0;
   if (cb > 255) cb = 255;

   if (front)
     {
         aclip = sd->front.clip;
         bclip = sd->back.clip;
     }
   else
     {
         aclip = sd->back.clip;
         bclip = sd->front.clip;
     }

   evas_object_color_set(aclip, ca, ca, ca, ca);
   evas_object_color_set(bclip, cb, cb, cb, cb);
}

static void
_state_end(Efl_Ui_Flip_Data *sd)
{
   _state_slices_clear(sd);
}

static void
_flip_show_hide(Evas_Object *obj)
{
   EFL_UI_FLIP_DATA_GET(obj, sd);
   if (elm_flip_front_visible_get(obj))
     {
        if (sd->pageflip)
          {
             if (sd->front.content)
               {
                  evas_object_move(sd->front.content, 4999, 4999);
                  evas_object_show(sd->front.clip);
               }
             else
               evas_object_hide(sd->front.clip);
             if (sd->back.content)
               evas_object_show(sd->back.clip);
             else
               evas_object_hide(sd->back.clip);
          }
        else
          {
             if (sd->front.content)
               evas_object_show(sd->front.clip);
             else
               evas_object_hide(sd->front.clip);
             evas_object_hide(sd->back.clip);
          }
     }
   else
     {
        if (sd->pageflip)
          {
             if (sd->front.content)
               evas_object_show(sd->front.clip);
             else
               evas_object_hide(sd->front.clip);
             if (sd->back.content)
               {
                  evas_object_move(sd->back.content, 4999, 4999);
                  evas_object_show(sd->back.clip);
               }
             else
               evas_object_hide(sd->back.clip);
          }
        else
          {
             evas_object_hide(sd->front.clip);
             if (sd->back.content)
               evas_object_show(sd->back.clip);
             else
               evas_object_hide(sd->back.clip);
          }
     }
}

static void
_map_uv_set(Evas_Object *obj, Evas_Map *map)
{
   Evas_Coord x, y, w, h;

   // FIXME: only handles filled obj
   if (efl_isa(obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS) &&
       !evas_object_image_source_get(obj))
     {
        int iw, ih;
        evas_object_image_size_get(obj, &iw, &ih);
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        evas_map_util_points_populate_from_geometry(map, x, y, w, h, 0);
        evas_map_point_image_uv_set(map, 0, 0, 0);
        evas_map_point_image_uv_set(map, 1, iw, 0);
        evas_map_point_image_uv_set(map, 2, iw, ih);
        evas_map_point_image_uv_set(map, 3, 0, ih);
     }
   else
     {
        evas_object_geometry_get(obj, &x, &y, &w, &h);
        evas_map_util_points_populate_from_geometry(map, x, y, w, h, 0);
     }
}

static void
_flip_do(Evas_Object *obj,
         double t,
         Elm_Flip_Mode mode,
         int lin,
         int rev)
{
   double p, deg, pp;
   Evas_Map *mf, *mb;
   Evas_Coord x, y, w, h;
   Evas_Coord cx, cy, px, py, foc;
   int lx, ly, lz, lr, lg, lb, lar, lag, lab;

   EFL_UI_FLIP_DATA_GET(obj, sd);

   mf = evas_map_new(4);
   evas_map_smooth_set(mf, EINA_FALSE);
   mb = evas_map_new(4);
   evas_map_smooth_set(mb, EINA_FALSE);

   if (sd->front.content)
     _map_uv_set(sd->front.content, mf);
   if (sd->back.content)
     _map_uv_set(sd->back.content, mb);

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
        if (sd->state) deg = 180.0 * p;
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
        if (sd->state) deg = 180.0 * p;
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
        if (sd->state) deg = 180.0 * p;
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
        if (sd->state) deg = 180.0 * p;
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
        if (sd->state)
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
        if (sd->state)
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
        if (sd->state)
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
        if (sd->state)
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

   if (sd->front.content)
     {
        evas_map_util_3d_lighting(mf, lx, ly, lz, lr, lg, lb, lar, lag, lab);
        evas_map_util_3d_perspective(mf, px, py, 0, foc);
        evas_object_map_set(sd->front.content, mf);
        evas_object_map_enable_set(sd->front.content, EINA_TRUE);
        if (evas_map_util_clockwise_get(mf)) evas_object_show(sd->front.clip);
        else evas_object_hide(sd->front.clip);
     }

   if (sd->back.content)
     {
        evas_map_util_3d_lighting(mb, lx, ly, lz, lr, lg, lb, lar, lag, lab);
        evas_map_util_3d_perspective(mb, px, py, 0, foc);
        evas_object_map_set(sd->back.content, mb);
        evas_object_map_enable_set(sd->back.content, EINA_TRUE);
        if (evas_map_util_clockwise_get(mb)) evas_object_show(sd->back.clip);
        else evas_object_hide(sd->back.clip);
     }

   evas_map_free(mf);
   evas_map_free(mb);
}

static void
_show_hide(Evas_Object *obj)
{
   EFL_UI_FLIP_DATA_GET(obj, sd);
   Evas_Coord x, y, w, h;
   if (!sd) return;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   if (sd->front.content)
     {
        if ((sd->pageflip) && (sd->state))
          {
             evas_object_move(sd->front.content, 4999, 4999);
          }
        else
          {
             if (!sd->animator)
               evas_object_move(sd->front.content, x, y);
          }
        evas_object_resize(sd->front.content, w, h);
     }
   if (sd->back.content)
     {
        if ((sd->pageflip) && (!sd->state))
          {
             evas_object_move(sd->back.content, 4999, 4999);
          }
        else
          {
             if (!sd->animator)
               evas_object_move(sd->back.content, x, y);
          }
        evas_object_resize(sd->back.content, w, h);
     }
}

static void
_configure(Evas_Object *obj)
{
   Evas_Coord x, y, w, h;
   Evas_Coord fsize;

   EFL_UI_FLIP_DATA_GET(obj, sd);
   _show_hide(obj);
   evas_object_geometry_get(obj, &x, &y, &w, &h);
   // FIXME: manual flip wont get fixed
   if (sd->animator) _flip(obj);

   if (sd->event[0])
     {
        fsize = (double)w * sd->dir_hitsize[0];
        elm_coords_finger_size_adjust(0, NULL, 1, &fsize);
        evas_object_geometry_set(sd->event[0], x, y, w, fsize);
     }
   if (sd->event[1])
     {
        fsize = (double)w * sd->dir_hitsize[1];
        elm_coords_finger_size_adjust(0, NULL, 1, &fsize);
        evas_object_geometry_set(sd->event[1], x, y + h - fsize, w, fsize);
     }
   if (sd->event[2])
     {
        fsize = (double)h * sd->dir_hitsize[2];
        elm_coords_finger_size_adjust(1, &fsize, 0, NULL);
        evas_object_geometry_set(sd->event[2], x, y, fsize, h);
     }
   if (sd->event[3])
     {
        fsize = (double)h * sd->dir_hitsize[3];
        elm_coords_finger_size_adjust(1, &fsize, 0, NULL);
        evas_object_geometry_set(sd->event[3], x + w - fsize, y, fsize, h);
     }
}

static Eina_Bool
_flip(Evas_Object *obj)
{
   double t;
   Evas_Coord w, h;

   EFL_UI_FLIP_DATA_GET(obj, sd);

   t = ecore_loop_time_get() - sd->start;

   if (!sd->animator) return ECORE_CALLBACK_CANCEL;

   t = t / sd->len;
   if (t > 1.0) t = 1.0;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (!sd->manual)
     {
        if (sd->mode == ELM_FLIP_PAGE_LEFT)
          {
             sd->dir = ELM_FLIP_DIRECTION_LEFT;
             sd->started = EINA_TRUE;
             sd->pageflip = EINA_TRUE;
             sd->down_x = w - 1;
             sd->down_y = h / 2;
             sd->x = (1.0 - t) * sd->down_x;
             sd->y = sd->down_y;
             _flip_show_hide(obj);
             _state_update(obj);
          }
        else if (sd->mode == ELM_FLIP_PAGE_RIGHT)
          {
             sd->dir = ELM_FLIP_DIRECTION_RIGHT;
             sd->started = EINA_TRUE;
             sd->pageflip = EINA_TRUE;
             sd->down_x = 0;
             sd->down_y = h / 2;
             sd->x = (t) * w;
             sd->y = sd->down_y;
             _flip_show_hide(obj);
             _state_update(obj);
          }
        else if (sd->mode == ELM_FLIP_PAGE_UP)
          {
             sd->dir = ELM_FLIP_DIRECTION_UP;
             sd->started = EINA_TRUE;
             sd->pageflip = EINA_TRUE;
             sd->down_x = w / 2;
             sd->down_y = h - 1;
             sd->x = sd->down_x;
             sd->y = (1.0 - t) * sd->down_y;
             _flip_show_hide(obj);
             _state_update(obj);
          }
        else if (sd->mode == ELM_FLIP_PAGE_DOWN)
          {
             sd->dir = ELM_FLIP_DIRECTION_DOWN;
             sd->started = EINA_TRUE;
             sd->pageflip = EINA_TRUE;
             sd->down_x = w / 2;
             sd->down_y = 0;
             sd->x = sd->down_x;
             sd->y = (t) * h;
             _flip_show_hide(obj);
             _state_update(obj);
          }
        else if (sd->mode == ELM_FLIP_CROSS_FADE)
          {
             sd->dir = 0;
             sd->started = EINA_TRUE;
             sd->pageflip = EINA_FALSE;
             _cross_fade_update(obj, t);
          }
        else
          _flip_do(obj, t, sd->mode, 0, 0);
     }

   if (t >= 1.0)
     {
#if 0 // this breaks manual flipping. :/
        if (sd->state == sd->next_state)
          {
             /* it was flipped while flipping, do it again */
             sd->start = ecore_loop_time_get();
             sd->state = !sd->next_state;
             return ECORE_CALLBACK_RENEW;
          }
#endif
        sd->pageflip = EINA_FALSE;
        _state_end(sd);
        evas_object_map_enable_set(sd->front.content, EINA_FALSE);
        evas_object_map_enable_set(sd->back.content, EINA_FALSE);
        // FIXME: hack around evas rendering bug (only fix makes evas bitch-slow
        evas_object_resize(sd->front.content, 0, 0);
        evas_object_resize(sd->back.content, 0, 0);
        evas_smart_objects_calculate(evas_object_evas_get(obj));
        // FIXME: end hack
        sd->animator = NULL;
        if (((sd->manual) && (sd->finish)) || (!sd->manual))
          sd->state = sd->next_state;
        _configure(obj);
        _flip_show_hide(obj);

        if (sd->mode == ELM_FLIP_CROSS_FADE)
          {
             // Make the content fully opaque again
             evas_object_color_set(sd->front.clip, 255, 255, 255, 255);
             evas_object_color_set(sd->back.clip, 255, 255, 255, 255);
          }

        efl_event_callback_legacy_call(obj, EFL_UI_FLIP_EVENT_ANIMATE_DONE, NULL);

        // update the new front and back object.
        _update_front_back(obj, sd);

        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

/* we have to have move/resize info up-to-date on those events. it
 * happens that smarts callbacks on them happen before we have the new
 * values, so using event callbacks instead */
static void
_on_move(void *data EINA_UNUSED,
         Evas *e EINA_UNUSED,
         Evas_Object *obj,
         void *event_info EINA_UNUSED)
{
   _configure(obj);
}

static void
_on_resize(void *data EINA_UNUSED,
           Evas *e EINA_UNUSED,
           Evas_Object *obj,
           void *event_info EINA_UNUSED)
{
   _configure(obj);
}

static Eina_Bool
_animate(void *data)
{
   return _flip(data);
}

static double
_pos_get(Evas_Object *obj,
         Efl_Ui_Flip_Data *sd,
         int *rev,
         Elm_Flip_Mode *m)
{
   Evas_Coord x, y, w, h;
   double t = 1.0;

   evas_object_geometry_get(obj, &x, &y, &w, &h);
   switch (sd->intmode)
     {
      case EFL_UI_FLIP_INTERACTION_ROTATE:
      case EFL_UI_FLIP_INTERACTION_CUBE:
      {
         if (sd->dir == ELM_FLIP_DIRECTION_LEFT)
           {
              if (sd->down_x > 0)
                t = 1.0 - ((double)sd->x / (double)sd->down_x);
              *rev = 1;
           }
         else if (sd->dir == ELM_FLIP_DIRECTION_RIGHT)
           {
              if (sd->down_x < w)
                t = 1.0 - ((double)(w - sd->x) / (double)(w - sd->down_x));
           }
         else if (sd->dir == ELM_FLIP_DIRECTION_UP)
           {
              if (sd->down_y > 0)
                t = 1.0 - ((double)sd->y / (double)sd->down_y);
           }
         else if (sd->dir == ELM_FLIP_DIRECTION_DOWN)
           {
              if (sd->down_y < h)
                t = 1.0 - ((double)(h - sd->y) / (double)(h - sd->down_y));
              *rev = 1;
           }

         if (t < 0.0) t = 0.0;
         else if (t > 1.0)
           t = 1.0;

         if ((sd->dir == ELM_FLIP_DIRECTION_LEFT) ||
             (sd->dir == ELM_FLIP_DIRECTION_RIGHT))
           {
              if (sd->intmode == EFL_UI_FLIP_INTERACTION_ROTATE)
                *m = ELM_FLIP_ROTATE_Y_CENTER_AXIS;
              else if (sd->intmode == EFL_UI_FLIP_INTERACTION_CUBE)
                {
                   if (*rev)
                     *m = ELM_FLIP_CUBE_LEFT;
                   else
                     *m = ELM_FLIP_CUBE_RIGHT;
                }
           }
         else
           {
              if (sd->intmode == EFL_UI_FLIP_INTERACTION_ROTATE)
                *m = ELM_FLIP_ROTATE_X_CENTER_AXIS;
              else if (sd->intmode == EFL_UI_FLIP_INTERACTION_CUBE)
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
_event_anim(void *data,
            double pos)
{
   Efl_Ui_Flip_Data *sd = data;
   double p;

   p = ecore_animator_pos_map(pos, ECORE_POS_MAP_ACCELERATE, 0.0, 0.0);
   if (sd->finish)
     {
        if (sd->dir == ELM_FLIP_DIRECTION_LEFT)
          sd->x = sd->ox * (1.0 - p);
        else if (sd->dir == ELM_FLIP_DIRECTION_RIGHT)
          sd->x = sd->ox + ((sd->w - sd->ox) * p);
        else if (sd->dir == ELM_FLIP_DIRECTION_UP)
          sd->y = sd->oy * (1.0 - p);
        else if (sd->dir == ELM_FLIP_DIRECTION_DOWN)
          sd->y = sd->oy + ((sd->h - sd->oy) * p);
     }
   else
     {
        if (sd->dir == ELM_FLIP_DIRECTION_LEFT)
          sd->x = sd->ox + ((sd->w - sd->ox) * p);
        else if (sd->dir == ELM_FLIP_DIRECTION_RIGHT)
          sd->x = sd->ox * (1.0 - p);
        else if (sd->dir == ELM_FLIP_DIRECTION_UP)
          sd->y = sd->oy + ((sd->h - sd->oy) * p);
        else if (sd->dir == ELM_FLIP_DIRECTION_DOWN)
          sd->y = sd->oy * (1.0 - p);
     }
   switch (sd->intmode)
     {
      case EFL_UI_FLIP_INTERACTION_NONE:
        break;

      case EFL_UI_FLIP_INTERACTION_ROTATE:
      case EFL_UI_FLIP_INTERACTION_CUBE:
          {
             Elm_Flip_Mode m = ELM_FLIP_ROTATE_X_CENTER_AXIS;
             int rev = 0;
             p = _pos_get(sd->obj, sd, &rev, &m);
             _flip_do(sd->obj, p, m, 1, rev);
          }
        break;

      case EFL_UI_FLIP_INTERACTION_PAGE:
        sd->pageflip = EINA_TRUE;
        _configure(sd->obj);
        _state_update(sd->obj);
        break;

      default:
        break;
     }
   if (pos < 1.0) return ECORE_CALLBACK_RENEW;

   sd->pageflip = EINA_FALSE;
   _state_end(sd);
   evas_object_map_enable_set(sd->front.content, EINA_FALSE);
   evas_object_map_enable_set(sd->back.content, EINA_FALSE);
   // FIXME: hack around evas rendering bug (only fix makes evas bitch-slow
   evas_object_resize(sd->front.content, 0, 0);
   evas_object_resize(sd->back.content, 0, 0);
   evas_smart_objects_calculate
     (evas_object_evas_get(sd->obj));
   // FIXME: end hack
   sd->animator = NULL;
   if (sd->finish) sd->state = sd->next_state;
   _flip_show_hide(sd->obj);
   _configure(sd->obj);
   sd->animator = NULL;
   efl_event_callback_legacy_call
     (sd->obj, EFL_UI_FLIP_EVENT_ANIMATE_DONE, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static void
_update_job(void *data)
{
   Elm_Flip_Mode m = ELM_FLIP_ROTATE_X_CENTER_AXIS;
   Evas_Object *obj = data;
   Efl_Ui_Flip_Data *sd = efl_data_scope_get(obj, MY_CLASS);
   int rev = 0;
   double p;

   sd->job = NULL;
   switch (sd->intmode)
     {
      case EFL_UI_FLIP_INTERACTION_ROTATE:
      case EFL_UI_FLIP_INTERACTION_CUBE:
        p = _pos_get(obj, sd, &rev, &m);
        _flip_do(obj, p, m, 1, rev);
        break;

      case EFL_UI_FLIP_INTERACTION_PAGE:
        sd->pageflip = EINA_TRUE;
        _configure(obj);
        _state_update(obj);
        break;

      default:
        break;
     }
}

static void
_down_cb(void *data,
         Evas *e EINA_UNUSED,
         Evas_Object *obj EINA_UNUSED,
         void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Object *fl = data;
   Evas_Coord x, y, w, h;

   EFL_UI_FLIP_DATA_GET(fl, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   ELM_SAFE_FREE(sd->animator, ecore_animator_del);
   sd->mouse_down = EINA_TRUE;
   sd->started = EINA_FALSE;
   evas_object_geometry_get(data, &x, &y, &w, &h);
   sd->x = ev->canvas.x - x;
   sd->y = ev->canvas.y - y;
   sd->w = w;
   sd->h = h;
   sd->down_x = sd->x;
   sd->down_y = sd->y;
}

static void
_up_cb(void *data,
       Evas *e EINA_UNUSED,
       Evas_Object *obj EINA_UNUSED,
       void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Object *fl = data;
   Evas_Coord x, y, w, h;
   double tm = 0.5;

   EFL_UI_FLIP_DATA_GET(fl, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   sd->mouse_down = EINA_FALSE;
   if (!sd->started) return;
   evas_object_geometry_get(data, &x, &y, &w, &h);
   sd->x = ev->canvas.x - x;
   sd->y = ev->canvas.y - y;
   sd->w = w;
   sd->h = h;
   sd->ox = sd->x;
   sd->oy = sd->y;
   ELM_SAFE_FREE(sd->job, ecore_job_del);
   sd->finish = EINA_FALSE;
   if (sd->dir == ELM_FLIP_DIRECTION_LEFT)
     {
        tm = (double)sd->x / (double)sd->w;
        if (sd->x < (sd->w / 2)) sd->finish = EINA_TRUE;
     }
   else if (sd->dir == ELM_FLIP_DIRECTION_RIGHT)
     {
        if (sd->x > (sd->w / 2)) sd->finish = EINA_TRUE;
        tm = 1.0 - ((double)sd->x / (double)sd->w);
     }
   else if (sd->dir == ELM_FLIP_DIRECTION_UP)
     {
        if (sd->y < (sd->h / 2)) sd->finish = EINA_TRUE;
        tm = (double)sd->y / (double)sd->h;
     }
   else if (sd->dir == ELM_FLIP_DIRECTION_DOWN)
     {
        if (sd->y > (sd->h / 2)) sd->finish = EINA_TRUE;
        tm = 1.0 - ((double)sd->y / (double)sd->h);
     }
   if (tm < 0.01) tm = 0.01;
   else if (tm > 0.99) tm = 0.99;
   if (!sd->finish) tm = 1.0 - tm;
   else sd->next_state = !sd->state;
   tm *= 1.0; // FIXME: config for anim time
   ecore_animator_del(sd->animator);
   sd->animator = ecore_evas_animator_timeline_add(fl, tm, _event_anim, sd);
   sd->len = tm;
   sd->start = ecore_loop_time_get();
   sd->manual = EINA_TRUE;
   _event_anim(sd, 0.0);
}

static void
_move_cb(void *data,
         Evas *e EINA_UNUSED,
         Evas_Object *obj EINA_UNUSED,
         void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Object *fl = data;
   Evas_Coord x, y, w, h;

   EFL_UI_FLIP_DATA_GET(fl, sd);
   if (!sd->mouse_down) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   evas_object_geometry_get(data, &x, &y, &w, &h);
   sd->x = ev->cur.canvas.x - x;
   sd->y = ev->cur.canvas.y - y;
   sd->w = w;
   sd->h = h;
   if (!sd->started)
     {
        Evas_Coord dx, dy;

        dx = sd->x - sd->down_x;
        dy = sd->y - sd->down_y;
        if (((dx * dx) + (dy * dy)) >
            (_elm_config->finger_size * _elm_config->finger_size / 4))
          {
             if ((sd->x > (w / 2)) &&
                 (dx < 0) && (abs(dx) > abs(dy)))
               {
                  sd->dir = ELM_FLIP_DIRECTION_LEFT;
                  if (!sd->dir_enabled[ELM_FLIP_DIRECTION_LEFT]) return;
               }
             else if ((sd->x < (w / 2)) && (dx >= 0) &&
                      (abs(dx) > abs(dy)))
               {
                  sd->dir = ELM_FLIP_DIRECTION_RIGHT;
                  if (!sd->dir_enabled[ELM_FLIP_DIRECTION_RIGHT]) return;
               }
             else if ((sd->y > (h / 2)) && (dy < 0) && (abs(dy) >= abs(dx)))
               {
                  sd->dir = ELM_FLIP_DIRECTION_UP;
                  if (!sd->dir_enabled[ELM_FLIP_DIRECTION_UP]) return;
               }
             else if ((sd->y < (h / 2)) && (dy >= 0) && (abs(dy) >= abs(dx)))
               {
                  sd->dir = ELM_FLIP_DIRECTION_DOWN;
                  if (!sd->dir_enabled[ELM_FLIP_DIRECTION_DOWN]) return;
               }
             else return;

             sd->started = EINA_TRUE;
             if (sd->intmode == EFL_UI_FLIP_INTERACTION_PAGE)
               sd->pageflip = EINA_TRUE;
             _flip_show_hide(data);
             evas_smart_objects_calculate(evas_object_evas_get(data));
             _flip(data);
             // FIXME: hack around evas rendering bug (only fix makes
             // evas bitch-slow)
             evas_object_map_enable_set(sd->front.content, EINA_FALSE);
             evas_object_map_enable_set(sd->back.content, EINA_FALSE);
// FIXME: XXX why does this bork interactive flip??
//             evas_object_resize(sd->front.content, 0, 0);
//             evas_object_resize(sd->back.content, 0, 0);
             evas_smart_objects_calculate(evas_object_evas_get(data));
             _configure(fl);
             // FIXME: end hack
             efl_event_callback_legacy_call(fl, EFL_UI_FLIP_EVENT_ANIMATE_BEGIN, NULL);
          }
        else return;
     }

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   ecore_job_del(sd->job);
   sd->job = ecore_job_add(_update_job, fl);
}

static Eina_Bool
_flip_content_set(Evas_Object *obj,
                  Evas_Object *content,
                  Eina_Bool front)
{
   int i;
   Evas_Object **cont;

   EFL_UI_FLIP_DATA_GET(obj, sd);

   cont = front ? &(sd->front.content) : &(sd->back.content);

   if (*cont == content) return EINA_TRUE;

   evas_object_del(*cont);
   *cont = content;

   if (content)
     {
        elm_widget_sub_object_add(obj, content);
        //evas_object_smart_member_add(content, obj);
        evas_object_clip_set
          (content, front ? sd->front.clip : sd->back.clip);
        if (efl_isa(content, EFL_UI_WIDGET_CLASS) && sd->state != front)
          elm_widget_tree_unfocusable_set(content, EINA_TRUE);
     }

   // force calc to contents are the right size before transition
   evas_smart_objects_calculate(evas_object_evas_get(obj));
   //evas_object_smart_calculate(obj);
   _flip_show_hide(obj);
   _configure(obj);
   _sizing_eval(obj);

   if (sd->intmode != EFL_UI_FLIP_INTERACTION_NONE)
     {
        for (i = 0; i < 4; i++)
          evas_object_raise(sd->event[i]);
     }

   return EINA_TRUE;
}

static Evas_Object *
_flip_content_unset(Evas_Object *obj,
                    Eina_Bool front)
{
   Evas_Object *content;
   Evas_Object **cont;

   EFL_UI_FLIP_DATA_GET(obj, sd);

   cont = front ? &(sd->front.content) : &(sd->back.content);

   if (!*cont) return NULL;

   content = *cont;
   _elm_widget_sub_object_redirect_to_top(obj, content);

   return content;
}

static Eina_Bool
_efl_ui_flip_content_set(Eo *obj, Efl_Ui_Flip_Data *_pd EINA_UNUSED, const char *part, Evas_Object *content)
{
   if (!part || !strcmp(part, "front"))
      return _flip_content_set(obj, content, EINA_TRUE);
   else if (!strcmp(part, "back"))
      return _flip_content_set(obj, content, EINA_FALSE);
   return EINA_FALSE;
}

static Evas_Object*
_efl_ui_flip_content_get(Eo *obj EINA_UNUSED, Efl_Ui_Flip_Data *sd, const char *part)
{
   if (!part || !strcmp(part, "front"))
      return sd->front.content;
   else if (!strcmp(part, "back"))
      return sd->back.content;
   return NULL;
}

static Evas_Object*
_efl_ui_flip_content_unset(Eo *obj EINA_UNUSED, Efl_Ui_Flip_Data *_pd EINA_UNUSED, const char *part)
{
   if (!part || !strcmp(part, "front"))
      return _flip_content_unset(obj, EINA_TRUE);
   else if (!strcmp(part, "back"))
      return _flip_content_unset(obj, EINA_FALSE);
   return NULL;
}

EOLIAN static void
_efl_ui_flip_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Flip_Data *priv)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   priv->clip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(priv->clip, EINA_TRUE);
   evas_object_geometry_set(priv->clip, -49999, -49999, 99999, 99999);
   evas_object_smart_member_add(priv->clip, obj);

   priv->front.clip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(priv->front.clip, EINA_TRUE);
   evas_object_data_set(priv->front.clip, "_elm_leaveme", obj);
   evas_object_geometry_set(priv->front.clip, -49999, -49999, 99999, 99999);
   evas_object_smart_member_add(priv->front.clip, obj);
   evas_object_clip_set(priv->front.clip, priv->clip);

   priv->back.clip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_static_clip_set(priv->back.clip, EINA_TRUE);
   evas_object_data_set(priv->back.clip, "_elm_leaveme", obj);
   evas_object_geometry_set(priv->back.clip, -49999, -49999, 99999, 99999);
   evas_object_smart_member_add(priv->back.clip, obj);
   evas_object_clip_set(priv->back.clip, priv->clip);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints_cb, obj);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _on_resize, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _on_move, NULL);

   priv->state = EINA_TRUE;
   priv->next_state = EINA_TRUE;
   priv->intmode = EFL_UI_FLIP_INTERACTION_NONE;

   elm_widget_can_focus_set(obj, EINA_FALSE);
}

EOLIAN static void
_efl_ui_flip_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Flip_Data *sd)
{
   ecore_animator_del(sd->animator);
   _state_slices_clear(sd);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_efl_ui_flip_efl_object_constructor(Eo *obj, Efl_Ui_Flip_Data *sd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   sd->obj = obj;

   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_PAGE_TAB_LIST);

   return obj;
}

EOLIAN static Eina_Bool
_efl_ui_flip_front_visible_get(const Eo *obj EINA_UNUSED, Efl_Ui_Flip_Data *sd)
{
   return sd->state;
}

EAPI void
elm_flip_perspective_set(Evas_Object *obj,
                         Evas_Coord foc EINA_UNUSED,
                         Evas_Coord x EINA_UNUSED,
                         Evas_Coord y EINA_UNUSED)
{
   EFL_UI_FLIP_CHECK(obj);
}

// FIXME: add ambient and lighting control

static void
_internal_elm_flip_go_to(Evas_Object *obj,
                Efl_Ui_Flip_Data *sd,
                Eina_Bool front,
                Elm_Flip_Mode mode)
{
   if (!sd->animator) sd->animator = ecore_evas_animator_add(obj, _animate, obj);

   sd->mode = mode;
   sd->start = ecore_loop_time_get();
   sd->next_state = front;
   sd->len = 0.5; // FIXME: make config val
   sd->manual = EINA_FALSE;
   if ((sd->mode == ELM_FLIP_PAGE_LEFT) ||
       (sd->mode == ELM_FLIP_PAGE_RIGHT) ||
       (sd->mode == ELM_FLIP_PAGE_UP) ||
       (sd->mode == ELM_FLIP_PAGE_DOWN))
     sd->pageflip = EINA_TRUE;
   // force calc to contents are the right size before transition
   evas_smart_objects_calculate(evas_object_evas_get(obj));

   if (sd->mode == ELM_FLIP_CROSS_FADE)
     {
        // Convention: a is fading in, b is fading out
        Evas_Object *a, *b;
        if (front)
          {
             a = sd->front.content;
             b = sd->back.content;
          }
        else
          {
             a = sd->back.content;
             b = sd->front.content;
          }

        // Stack fade-in content on top of fade-out content
        if (a && b) evas_object_stack_above(a, b);

        evas_object_show(sd->front.clip);
        evas_object_show(sd->back.clip);
     }
   else
     {
        _flip_show_hide(obj);
     }

   _flip(obj);
   // FIXME: hack around evas rendering bug (only fix makes evas bitch-slow)
   evas_object_map_enable_set(sd->front.content, EINA_FALSE);
   evas_object_map_enable_set(sd->back.content, EINA_FALSE);
   evas_object_resize(sd->front.content, 0, 0);
   evas_object_resize(sd->back.content, 0, 0);
   evas_smart_objects_calculate(evas_object_evas_get(obj));
   _configure(obj);
   // FIXME: end hack

   efl_event_callback_legacy_call(obj, EFL_UI_FLIP_EVENT_ANIMATE_BEGIN, NULL);

   // set focus to the content object when flip go to is called
   if (elm_object_focus_get(obj))
     {
        if (front) elm_object_focus_set(sd->front.content, EINA_TRUE);
        else elm_object_focus_set(sd->back.content, EINA_TRUE);
     }

   if (sd->front.content && efl_isa(sd->front.content, EFL_UI_WIDGET_CLASS))
     elm_widget_tree_unfocusable_set(sd->front.content, !front);
   if (sd->back.content && efl_isa(sd->back.content, EFL_UI_WIDGET_CLASS))
     elm_widget_tree_unfocusable_set(sd->back.content, front);


}

EOLIAN static void
_efl_ui_flip_go_to(Eo *obj, Efl_Ui_Flip_Data *sd, Eina_Bool front, Elm_Flip_Mode mode)
{
   if (sd->next_state == front) return;

   _internal_elm_flip_go_to(obj, sd, front, mode);
}

EOLIAN static void
_efl_ui_flip_go(Eo *obj, Efl_Ui_Flip_Data *sd, Elm_Flip_Mode mode)
{
   _internal_elm_flip_go_to(obj, sd, !sd->state, mode);
}

static void
_event_rect_create(Eo *obj, Efl_Ui_Flip_Data *sd, int i)
{
   Evas_Object *clip;
   Evas *e;

   if (sd->event[i]) return;

   e = evas_object_evas_get(obj);
   sd->event[i] = evas_object_rectangle_add(e);

   clip = evas_object_clip_get(obj);
   evas_object_data_set(sd->event[i], "_elm_leaveme", obj);
   evas_object_clip_set(sd->event[i], clip);
   evas_object_color_set(sd->event[i], 0, 0, 0, 0);
   evas_object_show(sd->event[i]);
   evas_object_smart_member_add(sd->event[i], obj);
   evas_object_event_callback_add
     (sd->event[i], EVAS_CALLBACK_MOUSE_DOWN, _down_cb, obj);
   evas_object_event_callback_add
     (sd->event[i], EVAS_CALLBACK_MOUSE_UP, _up_cb, obj);
   evas_object_event_callback_add
     (sd->event[i], EVAS_CALLBACK_MOUSE_MOVE, _move_cb, obj);
}

EOLIAN static void
_efl_ui_flip_interaction_set(Eo *obj, Efl_Ui_Flip_Data *sd, Efl_Ui_Flip_Interaction mode)
{
   int i;


   if (sd->intmode == mode) return;
   sd->intmode = mode;

   for (i = 0; i < 4; i++)
     {
        if (sd->intmode == EFL_UI_FLIP_INTERACTION_NONE)
          ELM_SAFE_FREE(sd->event[i], evas_object_del);
        else if (sd->dir_enabled[i])
          {
             int area = (i & 0x2) | (i ^ 0x1);
             if (sd->dir_hitsize[area] >= 0.0)
               _event_rect_create(obj, sd, area);
          }
     }

   _sizing_eval(obj);
   _configure(obj);
}

EOLIAN static Efl_Ui_Flip_Interaction
_efl_ui_flip_interaction_get(const Eo *obj EINA_UNUSED, Efl_Ui_Flip_Data *sd)
{
   return sd->intmode;
}

static Efl_Ui_Dir
_flip_dir_to_efl_ui_dir(Elm_Flip_Direction dir)
{
   switch (dir)
     {
      case ELM_FLIP_DIRECTION_RIGHT: return EFL_UI_DIR_RIGHT;
      case ELM_FLIP_DIRECTION_DOWN: return EFL_UI_DIR_DOWN;
      case ELM_FLIP_DIRECTION_LEFT: return EFL_UI_DIR_LEFT;
      case ELM_FLIP_DIRECTION_UP: return EFL_UI_DIR_UP;
     }
   ERR("Invalid value for Elm_Flip_Direction: %d", (int) dir);
   return EFL_UI_DIR_DEFAULT;
}

static Elm_Flip_Direction
_efl_ui_dir_to_flip_dir(Efl_Ui_Dir dir)
{
   switch (dir)
     {
      case EFL_UI_DIR_RIGHT:
      case EFL_UI_DIR_HORIZONTAL:
        return ELM_FLIP_DIRECTION_RIGHT;
      case EFL_UI_DIR_DOWN:
        return ELM_FLIP_DIRECTION_DOWN;
      case EFL_UI_DIR_LEFT:
        return ELM_FLIP_DIRECTION_LEFT;
      case EFL_UI_DIR_UP:
      case EFL_UI_DIR_VERTICAL:
      case EFL_UI_DIR_DEFAULT:
        return ELM_FLIP_DIRECTION_UP;
     }
   ERR("Invalid value for Efl_Ui_Dir: %d", (int) dir);
   return ELM_FLIP_DIRECTION_UP;
}

EOLIAN static void
_efl_ui_flip_interaction_direction_enabled_set(Eo *obj, Efl_Ui_Flip_Data *sd, Efl_Ui_Dir dir, Eina_Bool enabled)
{
   int i = _efl_ui_dir_to_flip_dir(dir);
   int area;

   enabled = !!enabled;
   if (sd->dir_enabled[i] == enabled) return;
   sd->dir_enabled[i] = enabled;
   if (sd->intmode == EFL_UI_FLIP_INTERACTION_NONE) return;

   area = (i & 0x2) | (i ^ 0x1); // up <-> down, left <-> right
   if (enabled && (sd->dir_hitsize[area] >= 0.0))
     _event_rect_create(obj, sd, area);
   else if (!enabled && (sd->dir_hitsize[area] <= 0.0))
     // Delete this hit area as it has the default hitsize (0)
     ELM_SAFE_FREE(sd->event[area], evas_object_del);

   _sizing_eval(obj);
   _configure(obj);
}

EOLIAN static Eina_Bool
_efl_ui_flip_interaction_direction_enabled_get(Eo *obj EINA_UNUSED, Efl_Ui_Flip_Data *sd, Efl_Ui_Dir dir)
{
   return sd->dir_enabled[_efl_ui_dir_to_flip_dir(dir)];
}

EOLIAN static void
_efl_ui_flip_interaction_direction_hitsize_set(Eo *obj, Efl_Ui_Flip_Data *sd, Efl_Ui_Dir dir, double hitsize)
{
   int i = _efl_ui_dir_to_flip_dir(dir);


   if (hitsize < 0.0)
     hitsize = -1.0;
   else if (hitsize > 1.0)
     hitsize = 1.0;

   if (sd->dir_hitsize[i] == hitsize) return;
   sd->dir_hitsize[i] = hitsize;

   if (hitsize >= 0.0)
     _event_rect_create(obj, sd, i);
   else
     ELM_SAFE_FREE(sd->event[i], evas_object_del);

   _sizing_eval(obj);
   _configure(obj);
}

static void
_update_front_back(Eo *obj, Efl_Ui_Flip_Data *pd)
{
   int count, index;
   Efl_Gfx_Entity *content;

   count = eina_list_count(pd->content_list);
   if (count <= 2) return;

   // update the next state object
   content = _flip_content_unset(obj, !pd->state);
   evas_object_hide(content);
   content = pd->state ? pd->front.content : pd->back.content;
   index = eina_list_data_idx(pd->content_list, content);
   index = (index == count-1) ? 0 : index + 1;
   content = eina_list_nth(pd->content_list, index);
   evas_object_show(content);
   _flip_content_set(obj, content, !pd->state);
}

static void
_content_added(Eo *obj, Efl_Ui_Flip_Data *pd, Efl_Gfx_Entity *content)
{
   elm_widget_sub_object_add(obj, content);

   if (!pd->front.content)
     {
        _flip_content_set(obj, content, EINA_TRUE);
        return;
     }
   if (!pd->back.content)
     {
        _flip_content_set(obj, content, EINA_FALSE);
        return;
     }
}

static void
_content_removed(Eo *obj, Efl_Ui_Flip_Data *pd, Efl_Gfx_Entity *content)
{
   Efl_Gfx_Entity *back_face, *cur_face, *face;
   int index, count;
   Eina_Bool state;

   elm_widget_sub_object_del(obj, content);
   // if its not the front or back object just return.
   if ((pd->front.content != content) ||
       (pd->back.content != content))
     return;

   cur_face = pd->state ? pd->front.content : pd->back.content;
   back_face = pd->state ? pd->back.content : pd->front.content;

   if (cur_face == content)
     {
        face = back_face;
        state = EINA_TRUE;
     }
   else
     {
        face = cur_face;
        state = EINA_FALSE;
     }

   _flip_content_unset(obj, state);

   count = eina_list_count(pd->content_list);
   if (count == 1) return;

   _flip_content_unset(obj, state);
   index = eina_list_data_idx(pd->content_list, face);
   index = (index == count-1) ? 0 : index + 1;
   _flip_content_set(obj, eina_list_nth(pd->content_list, index), state);
}

EOLIAN static double
_efl_ui_flip_interaction_direction_hitsize_get(Eo *obj EINA_UNUSED, Efl_Ui_Flip_Data *sd, Efl_Ui_Dir dir)
{
   int i = _efl_ui_dir_to_flip_dir(dir);

   return sd->dir_hitsize[i];
}

EOLIAN static Eina_Iterator *
_efl_ui_flip_efl_container_content_iterate(Eo *obj EINA_UNUSED, Efl_Ui_Flip_Data *pd)
{
   return eina_list_iterator_new(pd->content_list);
}

EOLIAN static int
_efl_ui_flip_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Flip_Data *pd)
{
   return eina_list_count(pd->content_list);
}

EOLIAN static Eina_Bool
_efl_ui_flip_efl_pack_unpack(Eo *obj, Efl_Ui_Flip_Data *pd, Efl_Gfx_Entity *subobj)
{
   pd->content_list = eina_list_remove(pd->content_list, subobj);
   pd->content_list = eina_list_remove(pd->content_list, subobj);
   _content_removed(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_flip_efl_pack_pack(Eo *obj, Efl_Ui_Flip_Data *pd, Efl_Gfx_Entity *subobj)
{
   pd->content_list = eina_list_remove(pd->content_list, subobj);
   pd->content_list = eina_list_append(pd->content_list, subobj);
   _content_added(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_flip_efl_pack_linear_pack_begin(Eo *obj, Efl_Ui_Flip_Data *pd, Efl_Gfx_Entity *subobj)
{
   pd->content_list = eina_list_remove(pd->content_list, subobj);
   pd->content_list = eina_list_prepend(pd->content_list, subobj);
   _content_added(obj, pd, subobj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_flip_efl_pack_linear_pack_end(Eo *obj, Efl_Ui_Flip_Data *pd, Efl_Gfx_Entity *subobj)
{
   pd->content_list = eina_list_remove(pd->content_list, subobj);
   pd->content_list = eina_list_append(pd->content_list, subobj);
   _content_added(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_flip_efl_pack_linear_pack_before(Eo *obj, Efl_Ui_Flip_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   pd->content_list = eina_list_remove(pd->content_list, subobj);
   pd->content_list = eina_list_prepend_relative(pd->content_list, subobj, existing);
   _content_added(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_flip_efl_pack_linear_pack_after(Eo *obj, Efl_Ui_Flip_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   pd->content_list = eina_list_remove(pd->content_list, subobj);
   pd->content_list = eina_list_append_relative(pd->content_list, subobj, existing);
   _content_added(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_flip_efl_pack_linear_pack_at(Eo *obj, Efl_Ui_Flip_Data *pd, Efl_Gfx_Entity *subobj, int index)
{
   Efl_Gfx_Entity *existing = NULL;
   existing = eina_list_nth(pd->content_list, index);
   pd->content_list = eina_list_remove(pd->content_list, subobj);
   pd->content_list = eina_list_prepend_relative(pd->content_list, subobj, existing);
   _content_added(obj, pd, subobj);
   return EINA_TRUE;
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_flip_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED, Efl_Ui_Flip_Data *pd, int index)
{
   return eina_list_nth(pd->content_list, index);
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_flip_efl_pack_linear_pack_unpack_at(Eo *obj, Efl_Ui_Flip_Data *pd, int index)
{
   Efl_Gfx_Entity *content = eina_list_nth(pd->content_list ,index);

   pd->content_list = eina_list_remove(pd->content_list, content);
   _content_removed(obj, pd, content);
   return content;
}

EOLIAN static int
_efl_ui_flip_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED, Efl_Ui_Flip_Data *pd, const Efl_Gfx_Entity *subobj)
{
   return eina_list_data_idx(pd->content_list, (void *)subobj);
}

EAPI void
elm_flip_interaction_direction_hitsize_set(Efl_Ui_Flip *obj, Elm_Flip_Direction dir, double hitsize)
{
   Efl_Ui_Dir uidir = _flip_dir_to_efl_ui_dir(dir);
   efl_ui_flip_interaction_direction_hitsize_set(obj, uidir, hitsize);
}

EAPI double
elm_flip_interaction_direction_hitsize_get(Efl_Ui_Flip *obj, Elm_Flip_Direction dir)
{
   Efl_Ui_Dir uidir = _flip_dir_to_efl_ui_dir(dir);
   return efl_ui_flip_interaction_direction_hitsize_get(obj, uidir);
}

EOAPI void
elm_flip_interaction_direction_enabled_set(Efl_Ui_Flip *obj, Elm_Flip_Direction dir, Eina_Bool enabled)
{
   Efl_Ui_Dir uidir = _flip_dir_to_efl_ui_dir(dir);
   efl_ui_flip_interaction_direction_enabled_set(obj, uidir, enabled);
}

EOAPI Eina_Bool
elm_flip_interaction_direction_enabled_get(Efl_Ui_Flip *obj, Elm_Flip_Direction dir)
{
   Efl_Ui_Dir uidir = _flip_dir_to_efl_ui_dir(dir);
   return efl_ui_flip_interaction_direction_enabled_get(obj, uidir);
}

/* Efl.Part begin */

static Eina_Bool
_part_is_efl_ui_flip_entry_part(const Eo *obj EINA_UNUSED, const char *part)
{
   return ((eina_streq(part, "front")) || (eina_streq(part, "back")));
}

ELM_PART_OVERRIDE_PARTIAL(efl_ui_flip, EFL_UI_FLIP, Efl_Ui_Flip_Data, _part_is_efl_ui_flip_entry_part)
ELM_PART_OVERRIDE_CONTENT_SET(efl_ui_flip, EFL_UI_FLIP, Efl_Ui_Flip_Data)
ELM_PART_OVERRIDE_CONTENT_GET(efl_ui_flip, EFL_UI_FLIP, Efl_Ui_Flip_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(efl_ui_flip, EFL_UI_FLIP, Efl_Ui_Flip_Data)
ELM_PART_CONTENT_DEFAULT_GET(efl_ui_flip, "front")
#include "efl_ui_flip_part.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define EFL_UI_FLIP_EXTRA_OPS \
   ELM_PART_CONTENT_DEFAULT_OPS(efl_ui_flip), \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_flip)

#include "efl_ui_flip.eo.c"

#include "efl_ui_flip_legacy.eo.h"

#define MY_CLASS_NAME_LEGACY "elm_flip"

static void
_efl_ui_flip_legacy_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Eo *
_efl_ui_flip_legacy_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, EFL_UI_FLIP_LEGACY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   return obj;
}

EAPI Evas_Object *
elm_flip_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(EFL_UI_FLIP_LEGACY_CLASS, parent);
}

#include "efl_ui_flip_legacy.eo.c"

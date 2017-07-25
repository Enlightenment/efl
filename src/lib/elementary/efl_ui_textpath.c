#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

#include "elm_widget_layout.h"
#include "efl_ui_textpath_internal_part.eo.h"
#include "elm_part_helper.h"


#define MY_CLASS EFL_UI_TEXTPATH_CLASS

#define MY_CLASS_NAME "Efl.Ui.Textpath"
#define MY_CLASS_NAME_LEGACY "elm_textpath"

#define PI 3.14159265
#define SLICE_MAX 200
#define SLICE_DEFAULT_NO 99

typedef struct _Efl_Ui_Textpath_Point Efl_Ui_Textpath_Point;
typedef struct _Efl_Ui_Textpath_Line Efl_Ui_Textpath_Line;
typedef struct _Efl_Ui_Textpath_Segment Efl_Ui_Textpath_Segment;
typedef struct _Efl_Ui_Textpath_Data Efl_Ui_Textpath_Data;

struct _Efl_Ui_Textpath_Point
{
   double x;
   double y;
};

struct _Efl_Ui_Textpath_Line
{
   Efl_Ui_Textpath_Point start;
   Efl_Ui_Textpath_Point end;
};

struct _Efl_Ui_Textpath_Segment
{
   EINA_INLIST;
   int length;
   Efl_Gfx_Path_Command_Type type;
   union
     {
        Eina_Bezier bezier;
        Efl_Ui_Textpath_Line line;
     };
};


struct _Efl_Ui_Textpath_Data {
   Evas_Object *content;
   Evas_Object *text_obj;
   char *text;
   Evas_Object *proxy;
   Efl_Gfx_Path *path;
   struct {
        double x, y;
        double radius;
        double start_angle;
   } circle;
   Efl_Ui_Textpath_Direction direction;
   int slice_no;
   Eina_Bool autofit;
   Eina_Bool ellipsis;

   Eina_Inlist *segments;
   int total_length;

   Evas_Object *clip;
};

#define EFL_UI_TEXTPATH_DATA_GET(o, sd) \
   Efl_Ui_Textpath_Data *sd = efl_data_scope_get(o, EFL_UI_TEXTPATH_CLASS)

static inline double
_deg_to_rad(double angle)
{
   return angle / 180 * PI;
}

static inline void
_transform_coord(double x, double y, double rad, double cx, double cy,
                 int *tx, int *ty)
{
   if (tx) *tx = (int)(x * cos(rad) - y * sin(rad) + cx);
   if (ty) *ty = (int)(x * sin(rad) + y * cos(rad) + cy);
}

static void
_textpath_ellipsis_set(Efl_Ui_Textpath_Data *pd, Eina_Bool enabled)
{
   //ELM_WIDGET_DATA_GET_OR_RETURN(pd->text_obj, wd);

   Eina_Strbuf *buf = eina_strbuf_new();
   if (enabled)
     {
        eina_strbuf_append_printf(buf, "DEFAULT='ellipsis=1.0'");
     }
   else
     {
        eina_strbuf_append_printf(buf, "DEFAULT='ellipsis=0.0'");
     }
   const char *format = eina_stringshare_add(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   //edje_object_part_text_style_user_pop(wd->resize_obj, "elm.text");
   //edje_object_part_text_style_user_push(wd->resize_obj, "elm.text", format);
   edje_object_part_text_style_user_pop(pd->text_obj, "elm.text");
   edje_object_part_text_style_user_push(pd->text_obj, "elm.text", format);
   ERR("set format: %s", format);
}


static void
_segment_draw(Efl_Ui_Textpath_Data *pd, int slice_no, double slice_len, int w1, int w2, int cmp, Evas_Map *map, Eina_Bool final, Eina_Bezier bezier)
{
   ERR("In: slice_no: %d, slice_len: %.2f, from: %d, to: %d, cur mp: %d", slice_no, slice_len, w1, w2, cmp);

   int x = 0, y = 0, w = 0, h = 0;
   int i, len, seg_len;
   double u0, u1, v0, v1;
   double dist, t, dt;
   double px, py, px2, py2;
   double rad;
   Eina_Vector2 vec, nvec, vec0, vec1, vec2, vec3;
   Eina_Matrix2 mat;

   len = w2 - w1;
   efl_gfx_size_get(pd->text_obj, &w, &h);
   seg_len = eina_bezier_length_get(&bezier);
   //dt = 1.0 / (double) pd->slice;
   if (pd->autofit)
     {
        dt = len / (seg_len * (double) slice_no);
     }
   else
     {
        dt = 1.0 / (double) slice_no;
     }
   //dist = ((double) w / (double) pd->slice);
   //dist = ((double) w / (double) pd->slice);
   //dist = slice_len;
   dist = len / (double)slice_no;
   ERR("slice_no: %d, distance: %.1f", slice_no, dist);
    //Compute Beziers.

   //length = eina_bezier_length_get(&bezier);
   rad = _deg_to_rad(90);
   eina_matrix2_values_set(&mat, cos(rad), -sin(rad), sin(rad), cos(rad));

   //index 0: v0, v3
   eina_bezier_values_get(&bezier, NULL, NULL, NULL, NULL, NULL, NULL, &px2, &py2);
   t = 0;
   eina_bezier_point_at(&bezier, t, &px, &py);
   eina_bezier_point_at(&bezier, t + dt, &px2, &py2);

   vec.x = (px2 - px);
   vec.y = (py2 - py);
   eina_vector2_normalize(&nvec, &vec);

   eina_vector2_transform(&vec, &mat, &nvec);
   eina_vector2_normalize(&nvec, &vec);
   eina_vector2_scale(&vec, &nvec, ((double) h) * 0.5);

   vec1.x = (vec.x + px);
   vec1.y = (vec.y + py);
   vec2.x = (-vec.x + px);
   vec2.y = (-vec.y + py);

   //add points to map
   for (i = 0; i < slice_no; i++)
     {
        //v0, v3
        vec0.x = vec1.x;
        vec0.y = vec1.y;
        vec3.x = vec2.x;
        vec3.y = vec2.y;

        //v1, v2
        t = ((double) (i + 1) * dt);
        eina_bezier_point_at(&bezier, t, &px, &py);
        eina_bezier_point_at(&bezier, t + dt, &px2, &py2);

        vec.x = (px2 - px);
        vec.y = (py2 - py);
        eina_vector2_normalize(&nvec, &vec);

        eina_vector2_transform(&vec, &mat, &nvec);
        eina_vector2_normalize(&nvec, &vec);
        eina_vector2_scale(&vec, &nvec, ((double) h) * 0.5);

        vec1.x = (vec.x + px);
        vec1.y = (vec.y + py);
        vec2.x = (-vec.x + px);
        vec2.y = (-vec.y + py);


        evas_map_point_coord_set(map, cmp + i * 4, (int) vec0.x + x, (int) vec0.y + y, 0);
        evas_map_point_coord_set(map, cmp + i * 4 + 1, (int) vec1.x + x, (int) vec1.y + y, 0);
        evas_map_point_coord_set(map, cmp + i * 4 + 2, (int) vec2.x + x, (int) vec2.y + y, 0);
        evas_map_point_coord_set(map, cmp + i * 4 + 3, (int) vec3.x + x, (int) vec3.y + y, 0);

        //UV
        u0 = w1 + i * dist;
        u1 = u0 + dist;
        v0 = (double) 0;
        v1 = (double) h;

        evas_map_point_image_uv_set(map, cmp + i * 4, u0, v0);
        evas_map_point_image_uv_set(map, cmp + i * 4 + 1, u1, v0);
        evas_map_point_image_uv_set(map, cmp + i * 4 + 2, u1, v1);
        evas_map_point_image_uv_set(map, cmp + i * 4 + 3, u0, v1);

        if (i < 3)
          {
             ERR("map(%d): %d %d :: %d %d :: %d %d :: %d %d", cmp + i*4, (int)vec0.x + x, (int)vec0.y + y, (int)vec1.x + x, (int)vec1.y + y, (int)vec2.x + x, (int)vec2.y + y, (int)vec3.x + x, (int)vec3.y + y);
             ERR("map uv: %.1f %.1f :: %.1f %.1f", u0, v0, u1, v1);
          }
     }
}

static void
_text_on_line_draw(Efl_Ui_Textpath_Data *pd, int w1, int w2, int cmp, Evas_Map *map, Efl_Ui_Textpath_Line line)
{
   double x1, x2, y1, y2;
   x1 = line.start.x;
   y1 = line.start.y;
   x2 = line.end.x;
   y2 = line.end.y;
   ERR("line: %.1f %.1f - %.1f %.1f", x1, y1, x2, y2);

   //test: line
   /*Eo *ln = evas_object_line_add(evas_object_evas_get(pd->text_obj));
   efl_gfx_size_set(ln, 500, 500);
   efl_gfx_color_set(ln, 255, 255, 0, 255);
   efl_gfx_visible_set(ln, EINA_TRUE);
   evas_object_line_xy_set(ln, x1, y1, x2, y2);*/
   //

   double line_len = sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
   double len = w2 - w1;
   if (line_len > len)
     {
        x2 = x1 + len * (x2 - x1) / line_len;
        y2 = y1 + len * (y2 - y1) / line_len;
        ERR("new: %.1f %.1f - %.1f %.1f", x1, y1, x2, y2);
     }

   Evas_Coord x, y, w, h;
   efl_gfx_geometry_get(pd->text_obj, &x, &y, &w, &h);
   ERR("content geo: %d %d %d %d", x, y, w, h);
   ERR("line geo: %.1f %.1f :: %.1f %.1f, area: %d %d, cmp: %d", x1, y1, x2, y2, w1, w2, cmp);

   double sina, cosa;
   len = sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
   sina = (y2 - y1) / len;
   cosa = (x2 - x1) / len;

   h = h / 2;
   evas_map_point_coord_set(map, cmp + 3, x1 - h * sina, y1 + h * cosa, 0);
   evas_map_point_coord_set(map, cmp + 2, x2 - h * sina, y2 + h * cosa, 0);
   evas_map_point_coord_set(map, cmp + 1, x2 + h * sina, y2 - h * cosa, 0);
   evas_map_point_coord_set(map, cmp + 0, x1 + h * sina, y1 - h * cosa, 0);

   h *= 2;
   //w = len < w ? len : w;
   evas_map_point_image_uv_set(map, cmp + 0, w1, 0);
   evas_map_point_image_uv_set(map, cmp + 1, w2, 0);
   evas_map_point_image_uv_set(map, cmp + 2, w2, h);
   evas_map_point_image_uv_set(map, cmp + 3, w1, h);

}

static int
_map_point_calc(Efl_Ui_Textpath_Data *pd)
{
   int map_no = 0;
   Efl_Ui_Textpath_Segment *seg;

   EINA_INLIST_FOREACH(pd->segments, seg)
     {
        if (seg->type == EFL_GFX_PATH_COMMAND_TYPE_LINE_TO)
          {
             map_no++;
          }
        else if (seg->type == EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO)
          {
             int no = pd->slice_no * seg->length / (double)pd->total_length;
             map_no += no;
          }
     }
   map_no *= 4;

   return map_no;
}

static void
_text_draw(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
   Efl_Ui_Textpath_Segment *seg;
   Evas_Map *map;
   double slice_unit, slice_len;
   int w, h, w1, w2;
   int remained_w;
   int total_slice;
   int cur_map_point = 0;
   int drawn_slice;

   efl_gfx_size_get(pd->text_obj, &w, &h);
   if (pd->autofit)
     {
        remained_w = w;
     }
   else
     {
        remained_w = pd->total_length;
     }
   slice_unit = (double)pd->slice_no / pd->total_length;

   slice_len = 1.0 / slice_unit;
   total_slice = w / slice_len + 1;

   int map_no = _map_point_calc(pd);
   ERR("map poins no: %d", map_no);
   if (map_no == 0) return;
   map = evas_map_new(map_no);

   w1 = w2 = 0;
   EINA_INLIST_FOREACH(pd->segments, seg)
     {
        int len = seg->length;
        if (!pd->autofit)
          {
             len = (double)seg->length * w / (double)pd->total_length;
             ERR("len: %d", len);
          }
        ERR("w: %d, total_length: %d, seg length: %d, len: %d", w, pd->total_length, seg->length, len);
        //w1 = w - remained_w;
        if (remained_w <= 0)
          break;
        w2 = w1 + len;
        if (w2 > w)
          w2 = w;
        //remained_w -= len;
        if (seg->type == EFL_GFX_PATH_COMMAND_TYPE_LINE_TO)
          {
             drawn_slice += 1;
             _text_on_line_draw(pd, w1, w2, cur_map_point, map, seg->line);
             cur_map_point += 4;
          }
        else
          {
             int slice_no = len * slice_unit + 1;
             slice_no = pd->slice_no * seg->length / (double)pd->total_length;
             slice_len = seg->length / slice_no;
             if (remained_w == 0)
               slice_no = total_slice - drawn_slice;
             drawn_slice += slice_no;
             _segment_draw(pd, slice_no, slice_len, w1, w2, cur_map_point, map, EINA_FALSE, seg->bezier);
             cur_map_point += slice_no * 4;
          }
        w1 = w2;
        remained_w -= len;
     }
   evas_object_map_enable_set(pd->text_obj, EINA_TRUE);
   evas_object_map_set(pd->text_obj, map);
   evas_map_free(map);
}


static void
_path_data_get(Eo *obj, Efl_Ui_Textpath_Data *pd, Eina_Bool set_min)
{
   ERR("In");

   const Efl_Gfx_Path_Command_Type *cmd;
   const double *points;
   Efl_Ui_Textpath_Segment *seg;

   //delete previous segment list
   EINA_INLIST_FREE(pd->segments, seg)
     {
        pd->segments = eina_inlist_remove(pd->segments, EINA_INLIST_GET(seg));
        free(seg);
     }

   Evas_Coord x, y;
   efl_gfx_position_get(obj, &x, &y);

   pd->total_length = 0;
   efl_gfx_path_get(obj, &cmd, &points);
   if (cmd)
     {
	 ERR("cmd exist");
        int pos = -1;
        Eina_Rectangle *rect = eina_rectangle_new(0, 0, 0, 0);
        while (*cmd != EFL_GFX_PATH_COMMAND_TYPE_END)
          {
             double px0, py0, ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1, px1, py1;
             ERR("cmd: %d", *cmd);
             if (*cmd == EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO)
               {
                  pos++;
                  px0 = points[pos] + x;
                  pos++;
                  py0 = points[pos] + y;
               }
             else if (*cmd == EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO)
               {
                  pos++;
                  ctrl_x0 = points[pos] + x;
                  pos++;
                  ctrl_y0 = points[pos] + y;
                  pos++;
                  ctrl_x1 = points[pos] + x;
                  pos++;
                  ctrl_y1 = points[pos] + y;
                  pos++;
                  px1 = points[pos] + x;
                  pos++;
                  py1 = points[pos] + y;

                  Eina_Bezier bz;
                  eina_bezier_values_set(&bz, px0, py0, ctrl_x0, ctrl_y0, ctrl_x1, ctrl_y1, px1, py1);
                  Efl_Ui_Textpath_Segment *seg = malloc(sizeof(Efl_Ui_Textpath_Segment));
                  if (!seg)
                    {
                       ERR("Failed to allocate segment");
                       px0 = px1;
                       py0 = py1;
                       continue;
                    }
                  seg->length = eina_bezier_length_get(&bz);
                  seg->bezier = bz;
                  seg->type = EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO;
                  pd->segments = eina_inlist_append(pd->segments, EINA_INLIST_GET(seg));
                  pd->total_length += seg->length;

                  //move points
                  px0 = px1;
                  py0 = py1;

                  double bx, by, bw, bh;
                  eina_bezier_bounds_get(&bz, &bx, &by, &bw, &bh);
                  Eina_Rectangle *brect = eina_rectangle_new(bx, by, bw, bh);
                  eina_rectangle_union(rect, brect);
                  eina_rectangle_free(brect);
               }
             else if (*cmd == EFL_GFX_PATH_COMMAND_TYPE_LINE_TO)
               {
                  pos++;
                  px1 = points[pos] + x;
                  pos++;
                  py1 = points[pos] + y;
                  ERR("line: %.1f %.1f :: %.1f %.1f", px0, py0, px1, py1);

                  Efl_Ui_Textpath_Segment *seg = malloc(sizeof(Efl_Ui_Textpath_Segment));
                  if (!seg)
                    {
                       ERR("Failed to allocate segment");
                       px0 = px1;
                       py0 = py1;
                    }
                  seg->type = EFL_GFX_PATH_COMMAND_TYPE_LINE_TO;
                  seg->line.start.x = px0;
                  seg->line.start.y = py0;
                  seg->line.end.x = px1;
                  seg->line.end.y = py1;
                  seg->length = sqrt((px1 - px0)*(px1 - px0) + (py1 - py0)*(py1 - py0));
                  pd->segments = eina_inlist_append(pd->segments, EINA_INLIST_GET(seg));
                  pd->total_length += seg->length;

                  Eina_Rectangle *lrect = eina_rectangle_new(px0, py0, px1 - px0, py1 - py0);
                  eina_rectangle_union(rect, lrect);
                  eina_rectangle_free(lrect);
               }
             cmd++;
          }
        if (set_min)
          {
             evas_object_size_hint_min_set(obj, rect->w, rect->h);
             ERR("set min size: %d %d %d %d", rect->x, rect->y, rect->w, rect->h);
          }
        eina_rectangle_free(rect);
     }
}

static void
_circle_draw(Evas_Object *eo, Efl_Ui_Textpath_Data *pd)
{
   Evas_Coord x, y, w, h;
   Evas_Object *proxy;
   Evas *e;
   Evas_Map *map;
   int slice_no, tx, ty;
   double total_angle, start_angle, angle, dist;
   double u0, u1, v0, v1;
   int i;

   pd->content = pd->text_obj; ////test
   if (!pd->content) return;

   efl_gfx_geometry_get(pd->content, &x, &y, &w, &h);
   //edje_object_size_min_calc(pd->content, &w, &h); //FIXME: get real size (tb size???)
   //evas_object_size_hint_min_get(pd->content, &w, &h);
   ERR("content geo: %d %d %d %d", x, y, w, h);
   if (w <= 0 && h <=0)
     {
        ERR("content has 0 size");
        return;
     }
   e = evas_object_evas_get(pd->content);

   if (pd->autofit)
     {
        total_angle = (360 * w) / (2 * PI * (pd->circle.radius - h / 2));
        if (total_angle < 0) total_angle = -total_angle;
        if (total_angle > 360) total_angle = 360;

        //ellipsis feature
        int circum = 2 * PI * (pd->circle.radius - h / 2);
        ERR("cir: %d, text w: %d", circum, w);
        //FIXME: get length after map 
        //if (w >= circum)
        /*
        if (total_angle >= 360)
          {
             w = circum;
             ERR("turn on ellipsis");
             evas_object_resize(pd->text_obj, circum, h);
             _textpath_ellipsis_set(pd, EINA_TRUE);
             ERR("Ellipsis: '%s'", elm_object_text_get(pd->text_obj));
          }*/
        ////


        slice_no = ((pd->slice_no * total_angle) / 360) + 1;
        if (slice_no > pd->slice_no) slice_no = pd->slice_no;
     }
   else
     {
        slice_no = pd->slice_no;
        total_angle = 360;
     }
   //if (slice_no % 2 == 0) slice_no++;

   dist = (double)w / slice_no;
   angle = _deg_to_rad(total_angle / slice_no);
   start_angle = _deg_to_rad(pd->circle.start_angle);
   ERR("total angle: %.1f, angle: %.1f, slice no: %d, pd->slice_no: %d", total_angle, angle, slice_no, pd->slice_no);

   proxy = NULL;
   if (pd->proxy)
     efl_del(pd->proxy);
   ERR("Test: create proxy image");
   proxy = evas_object_image_filled_add(evas_object_evas_get(pd->text_obj));
   if (!proxy) ERR("Failed to create proxy image");
   //pd->proxy = proxy;
   //efl_gfx_size_set(proxy, w, h);
   //evas_object_image_source_set(proxy, pd->content);
   /*Eina_Bool val = evas_object_image_source_set(proxy, pd->text_obj);
   if (!val) ERR("Failed to set source for proxy image");
   evas_object_resize(proxy, 200, 100);
   evas_object_move(proxy, 20, 20);
   evas_object_show(proxy);*/

   evas_object_move(pd->text_obj, 100, 100);

   //to revert direction
   //radius = -radius;
   //change map points (3,2,1,0 instead of 0,1,2,3)
   ///

   map = evas_map_new(slice_no * 4);

   for (i = 0; i < slice_no; i++)
     {
        //_transform_coord(0, -pd->circle.radius,
        _transform_coord(pd->circle.radius + h/2, 0,
                         i * angle + start_angle,
                         pd->circle.x, pd->circle.y,
                         &tx, &ty);
        if (i < 3)
        ERR("map: %d:: %d %d, circle: %.0f %.0f", i*4, tx, ty, pd->circle.x, pd->circle.y);
        evas_map_point_coord_set(map, i * 4, tx, ty, 0);
        //_transform_coord(0, -pd->circle.radius,
        _transform_coord(pd->circle.radius + h/2, 0,
                         (i + 1) * angle + start_angle,
                         pd->circle.x, pd->circle.y,
                         &tx, &ty);
        if (i < 3)
        ERR("map: %d:: %d %d, circle: %.0f %.0f", i*4 + 1, tx, ty, pd->circle.x, pd->circle.y);
        evas_map_point_coord_set(map, i * 4 + 1, tx, ty, 0);
        //_transform_coord(0, -(pd->circle.radius - h),
        _transform_coord(pd->circle.radius - h/2, 0,
                         (i + 1) * angle + start_angle,
                         pd->circle.x, pd->circle.y,
                         &tx, &ty);
        if (i < 3)
        ERR("map: %d:: %d %d, circle: %.0f %.0f, angle: %.1f", i*4 + 2, tx, ty, pd->circle.x, pd->circle.y, (i+1)*angle + start_angle);
        evas_map_point_coord_set(map, i * 4 + 2, tx, ty, 0);
        //_transform_coord(0, -(pd->circle.radius - h),
        _transform_coord(pd->circle.radius - h/2, 0,
                         i * angle + start_angle,
                         pd->circle.x, pd->circle.y,
                         &tx, &ty);
        if (i < 3)
        ERR("map: %d:: -- %d %d, circle: %.0f %.0f, angle: %.1f --", i*4 + 3, tx, ty, pd->circle.x, pd->circle.y, i*angle + start_angle);
        evas_map_point_coord_set(map, i * 4 + 3, tx, ty, 0);

        //uv
        u0 = i * dist;
        u1 = u0 + dist;
        v0 = 0.0;
        v1 = h;

        evas_map_point_image_uv_set(map, i * 4, u0, v0);
        evas_map_point_image_uv_set(map, i * 4 + 1, u1, v0);
        evas_map_point_image_uv_set(map, i * 4 + 2, u1, v1);
        evas_map_point_image_uv_set(map, i * 4 + 3, u0, v1);
        
#if 0
        if (u1 > w)
          {
             ERR("out of content's width");
             break;
          }
#endif
     }
   //evas_object_map_enable_set(proxy, EINA_TRUE);
   //evas_object_map_set(proxy, map);
   evas_object_map_enable_set(pd->text_obj, EINA_TRUE);
   evas_object_map_set(pd->text_obj, map);
   evas_map_free(map);

   //debug
   /*x = pd->circle.x;
   y = pd->circle.y;
   double r = pd->circle.radius;
   Eo *line = evas_object_line_add(e);
   efl_gfx_color_set(line, 100, 0, 0, 100);
   efl_gfx_visible_set(line, EINA_TRUE);
   evas_object_line_xy_set(line, x, y - r, x, y + r);

   line = evas_object_line_add(e);
   efl_gfx_color_set(line, 100, 100, 0, 100);
   efl_gfx_visible_set(line, EINA_TRUE);
   evas_object_line_xy_set(line, x - r, y, x + r, y);*/
   //
}

static void
_sizing_eval(Evas_Object *obj, Efl_Ui_Textpath_Data *pd)
{
   ////_circle_draw(obj, pd);
   //_path_data_get(obj, pd);
   _text_draw(obj, pd);
}

static void
_content_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Efl_Ui_Textpath_Data *pd = data;
   _sizing_eval(obj, pd);
}

static void
_ellipsis_set(Efl_Ui_Textpath_Data *pd)
{
   if (!pd->text_obj) return;

   Evas_Coord w = 0, h = 0;
   Eina_Bool is_ellipsis = EINA_FALSE;

   efl_gfx_size_get(pd->text_obj, &w, &h);
   if (pd->ellipsis)
     {
        if (w > pd->total_length)
          {
             is_ellipsis = EINA_TRUE;
             w = pd->total_length;
             ERR("enable ellipsis");
          }
     }
   evas_object_resize(pd->text_obj, w, h);
   _textpath_ellipsis_set(pd, is_ellipsis);
}

static void
_path_changed_cb(void *data, const Efl_Event *event)
{
    ERR("Path changed");
    EFL_UI_TEXTPATH_DATA_GET(data, sd);

    _path_data_get(data, sd, EINA_TRUE);
}

//exposed API
EOLIAN static void
_efl_ui_textpath_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
   ERR("In");
   _sizing_eval(obj, pd);
}

static Eina_Bool
_textpath_text_set_internal(Eo *obj, Efl_Ui_Textpath_Data *pd, const char *part, const char *text)
{
   ERR("in: part: %s, text: %s", part, text);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   Eina_Bool int_ret = EINA_TRUE;

   if (!text) text = "";
   //set format
   //_textpath_format_set(wd->resize_obj, sd->format);

   efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), text);

   edje_object_part_text_set(pd->text_obj, "elm.text", text);
   //elm_layout_text_set(pd->text_obj, NULL, text);
   if (int_ret)
     {
        //elm_obj_layout_sizing_eval(obj);
     }
   //
   const Evas_Object *tb;
   Evas_Coord tb_w = 0, tb_h = 0;
   //tb = edje_object_part_object_get(wd->resize_obj, "elm.text");
   //evas_object_textblock_size_native_get(tb, &tb_w, &tb_h);
   //evas_object_size_hint_min_set(obj, tb_w, tb_h);
   //ELM_WIDGET_DATA_GET_OR_RETURN(pd->text_obj, twd, EINA_FALSE);
   //tb = edje_object_part_object_get(twd->resize_obj, "elm.text");
   tb = edje_object_part_object_get(pd->text_obj, "elm.text");
   evas_object_textblock_size_native_get(tb, &tb_w, &tb_h);
   //??? layout sizing_eval also does min set based on the edje size calc
   evas_object_size_hint_min_set(pd->text_obj, tb_w, tb_h);
   //FIXME: if we dont resize layout obj, size of 0
   //and map does not work.
   evas_object_resize(pd->text_obj, tb_w, tb_h);
   ERR("tb size: %d %d", tb_w, tb_h);
   //

   _ellipsis_set(pd);
   elm_obj_layout_sizing_eval(obj);



   //Test
   /*Evas_Object *t = elm_layout_add(obj);
   elm_layout_theme_set(t, "textpath", "base", elm_widget_style_get(obj));
   elm_object_text_set(t, text);
   evas_object_move(t, 50, 50);
   evas_object_resize(t, tb_w, tb_h);
   evas_object_show(t);*/
   /////
   return int_ret;
}


EOLIAN static Eina_Bool
_efl_ui_textpath_text_set(Eo *obj, Efl_Ui_Textpath_Data *pd, const char *part, const char *text)
{
   ERR("in: part: %s, text: %s", part, text);
   return _textpath_text_set_internal(obj, pd, part, text);
}

EOLIAN static const char *
_efl_ui_textpath_text_get(Eo *obj, Efl_Ui_Textpath_Data *pd, const char *part)
{
   return elm_layout_text_get(pd->text_obj, part);
}

EOLIAN static void
_efl_ui_textpath_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Textpath_Data *priv)
{
   ERR("In");
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   //if (!elm_layout_theme_set(obj, "textpath", "base", elm_widget_style_get(obj)))
   //  CRI("Failed to set layout");

   //elm_layout_text_set(obj, NULL, "<br>");
   //elm_layout_text_set(obj, NULL, "");
   //elm_layout_sizing_eval(obj);

   /*priv->text_obj = elm_layout_add(obj);
   if (!elm_layout_theme_set(priv->text_obj, "textpath", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout");
   elm_layout_text_set(priv->text_obj, NULL, "Test");
   evas_object_move(priv->text_obj, 100, 100);
   evas_object_show(priv->text_obj);*/


   priv->text_obj = edje_object_add(evas_object_evas_get(obj));
   //priv->text_obj = wd->resize_obj;
   elm_widget_theme_object_set(obj, priv->text_obj, "textpath", "base",
                               elm_widget_style_get(obj));
   edje_object_part_text_set(priv->text_obj, "elm.text", "test text");
   evas_object_size_hint_weight_set(priv->text_obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(priv->text_obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   //test
   /*const Evas_Object *tb;
   Evas_Coord tb_w, tb_h;
   tb = edje_object_part_object_get(priv->text_obj, "elm.text");
   evas_object_textblock_size_native_get(tb, &tb_w, &tb_h);
   evas_object_size_hint_min_set(priv->text_obj, tb_w, tb_h);
   evas_object_size_hint_max_set(priv->text_obj, tb_w, tb_h);
   evas_object_resize(priv->text_obj, tb_w, tb_h);
   evas_object_size_hint_min_set(priv->text_obj, tb_w, tb_h);
   ERR("tb size: %d %d", tb_w, tb_h);
   //evas_object_resize(priv->text_obj, 100, 100);
   */
   //
   evas_object_show(priv->text_obj);

   evas_object_smart_member_add(priv->text_obj, obj);
   elm_widget_sub_object_add(obj, priv->text_obj);

   //priv->clip = evas_object_rectangle_add(obj);
   //evas_object_color_set(priv->clip, 255, 255, 0, 100);
   //evas_object_show(priv->clip);
   //evas_object_clip_set(priv->text_obj, priv->clip);

   efl_event_callback_add(obj, EFL_GFX_PATH_EVENT_CHANGED, _path_changed_cb, obj);
}

//why? to support legacy...???
EOLIAN static void
_efl_ui_textpath_class_constructor(Efl_Class *klass)
{
   ERR("In");
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Efl_Object *
_efl_ui_textpath_efl_object_constructor(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
   ERR("in");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->autofit = EINA_TRUE;
   pd->slice_no = SLICE_DEFAULT_NO;
   pd->direction = EFL_UI_TEXTPATH_DIRECTION_CW;

   return obj;
}

EOLIAN static void
_efl_ui_textpath_efl_object_destructor(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
   Efl_Ui_Textpath_Segment *seg;

   if (pd->content) evas_object_del(pd->content);
   if (pd->text) free(pd->text);
   if (pd->text_obj) evas_object_del(pd->text_obj);
   EINA_INLIST_FREE(pd->segments, seg)
     {
        pd->segments = eina_inlist_remove(pd->segments, EINA_INLIST_GET(seg));
        free(seg);
     }

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_textpath_efl_text_text_set(Eo *obj, Efl_Ui_Textpath_Data *pd, const char *text)
{
   _textpath_text_set_internal(obj, pd, "elm.text", text);
}

EOLIAN static const char *
_efl_ui_textpath_efl_text_text_get(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
   return elm_layout_text_get(pd->text_obj, "elm.text");
}

EOLIAN static Elm_Theme_Apply
_efl_ui_textpath_elm_widget_theme_apply(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
   ERR("theme set");
   Elm_Theme_Apply int_ret = ELM_THEME_APPLY_FAILED;

   int_ret = elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return ELM_THEME_APPLY_FAILED;

   elm_widget_theme_object_set(obj, pd->text_obj, "textpath", "base",
                               elm_widget_style_get(obj));
   //need to set ellipsis again

   return int_ret;
}

EOLIAN static void
_efl_ui_textpath_efl_gfx_position_set(Eo *obj, Efl_Ui_Textpath_Data *pd, Evas_Coord x, Evas_Coord y)
{
   ERR("obj: %p,text_obj: %p, set position: textpath: %d %d", obj, pd->text_obj, x, y);
   efl_gfx_position_set(efl_super(obj, MY_CLASS), x, y);
   if (pd->text_obj)
     {
        //ERR("move text_obj to: %d %d", x, y);
        //efl_gfx_position_set(pd->text_obj, x, y);
     }
   //efl_gfx_position_set(pd->clip, x, y);

   _path_data_get(obj, pd, EINA_FALSE);
   _text_draw(obj, pd);
}

EOLIAN static void
_efl_ui_textpath_efl_gfx_size_set(Eo *obj, Efl_Ui_Textpath_Data *pd, Evas_Coord w, Evas_Coord h)
{
   ERR("size set: %d %d", w, h);
   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);
   ///
   //efl_gfx_size_set(pd->clip, w, h);
}

#if 0
EOLIAN static void
_efl_ui_textpath_efl_text_text_set(Eo *obj, Efl_Ui_Textpath_Data *pd, const char *text)
{
   ERR("in: %s", text);
   Evas_Object *content;
   if (pd->text) free(pd->text);
   if (pd->content)
     {
        //efl_del(pd->content);
        evas_object_del(pd->content);
        pd->content = NULL;
     }
   if (!text) return;
   int len = strlen(text);
   pd->text = malloc(sizeof(char) * (len + 1));
   if (!pd->text) return;
   strncpy(pd->text, text, len);
   pd->text[len] = '\0';
   //content = evas_object_text_add(evas_object_evas_get(obj)); //efl_add(EVAS_TEXT_CLASS, obj);
   //evas_object_text_font_set(content, "century schoolbook l", 20); //FIXME: which font and size???
   //efl_text_set(content, pd->text);
   content = elm_label_add(obj);
   elm_object_text_set(content, pd->text);
   efl_gfx_size_hint_min_set(content, 10, 50); //FIXME: remove it
   efl_gfx_size_hint_weight_set(content, EFL_GFX_SIZE_HINT_FILL,
                                        EFL_GFX_SIZE_HINT_FILL);
   efl_gfx_position_set(content, 0, 0);
   evas_object_event_callback_add(content, EVAS_CALLBACK_RESIZE, _content_resize_cb, pd);
   //efl_gfx_size_set(content, 2 * pd->circle.radius, 2 * pd->circle.radius);
   efl_gfx_size_set(content, 2 * PI * pd->circle.radius, pd->circle.radius);
   evas_object_show(content);

   pd->content = content;
   ERR("show content: w h: %.0f", 2 * pd->circle.radius);
   _sizing_eval(pd);
}

EOLIAN static const char *
_efl_ui_textpath_efl_text_text_get(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
   return pd->text;
}
#endif

EOLIAN static void
_efl_ui_textpath_circle_set(Eo *obj, Efl_Ui_Textpath_Data *pd, double x, double y, double radius, double start_angle, Efl_Ui_Textpath_Direction direction)
{
   if (pd->circle.x == x && pd->circle.y == y &&
       pd->circle.radius == radius &&
       pd->circle.start_angle == start_angle &&
       pd->direction == direction)
     {
        ERR("Same circle");
        return;
     }
   pd->circle.x = x;
   pd->circle.y = y;
   pd->circle.radius = radius;
   pd->circle.start_angle = start_angle;
   pd->direction = direction;

   if (direction == EFL_UI_TEXTPATH_DIRECTION_CW)
     {
        efl_gfx_path_append_arc(obj, x - radius, y - radius, radius * 2,
                                radius * 2,  start_angle, -360);
     }
   else
     {
        efl_gfx_path_append_arc(obj, x - radius, y - radius, radius * 2,
                                radius * 2,  start_angle, 360);
     }


   if (pd->content)
     {
        efl_gfx_size_set(pd->content, 2 * radius, 2 * radius);
     }

   _sizing_eval(obj, pd);
}

EOLIAN static Eina_Bool
_efl_ui_textpath_autofit_get(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
    return pd->autofit;
}

EOLIAN static void
_efl_ui_textpath_autofit_set(Eo *obj, Efl_Ui_Textpath_Data *pd, Eina_Bool autofit)
{
    if (pd->autofit == autofit) return;
    pd->autofit = autofit;
   _sizing_eval(obj, pd);
}

EOLIAN static int
_efl_ui_textpath_slice_number_get(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
    return pd->slice_no;
}

EOLIAN static void
_efl_ui_textpath_slice_number_set(Eo *obj, Efl_Ui_Textpath_Data *pd, int slice_no)
{
   if (pd->slice_no == slice_no) return;
   pd->slice_no = slice_no;
   _sizing_eval(obj, pd);
}

EOLIAN static void
_efl_ui_textpath_ellipsis_set(Eo *obj, Efl_Ui_Textpath_Data *pd, Eina_Bool ellipsis)
{
   if (pd->ellipsis == ellipsis) return;
   pd->ellipsis = ellipsis;

   _ellipsis_set(pd);

   _sizing_eval(obj, pd);
}

EOLIAN static Eina_Bool
_efl_ui_textpath_ellipsis_get(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
   return pd->ellipsis;
}



/* Efl.Part begin */
//EFL_TEXT_PART_DEFAULT_IMPLEMENT(efl_ui_textpath, Efl_Ui_Textpath_Data)

ELM_PART_OVERRIDE(efl_ui_textpath, EFL_UI_TEXTPATH, ELM_LAYOUT, Efl_Ui_Textpath_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_TEXT_SET(efl_ui_textpath, EFL_UI_TEXTPATH, ELM_LAYOUT, Efl_Ui_Textpath_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_TEXT_GET(efl_ui_textpath, EFL_UI_TEXTPATH, ELM_LAYOUT, Efl_Ui_Textpath_Data, Elm_Part_Data)
#include "efl_ui_textpath_internal_part.eo.c"
/* Efl.Part end */

#define EFL_UI_TEXTPATH_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(efl_ui_textpath)

#include "efl_ui_textpath.eo.c"

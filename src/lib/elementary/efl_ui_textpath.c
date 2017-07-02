#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#include "elm_widget_layout.h"


#define MY_CLASS EFL_UI_TEXTPATH_CLASS

#define PI 3.14159265
#define SLICE_MAX 200
#define SLICE_DEFAULT_NO 69

typedef struct _Efl_Ui_Textpath_Data Efl_Ui_Textpath_Data;

struct _Efl_Ui_Textpath_Data {
   Evas_Object *content;
   char *text;
   Evas_Object *proxy;
   Efl_Gfx_Path *path;
   struct {
        double x, y;
        double radius;
        double start_angle;
   } circle;
   Efl_Ui_Textpath_Orientation ori;
   int slice_no;
   Eina_Bool autofit;
};

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
_circle_draw(Efl_Ui_Textpath_Data *pd)
{
   Evas_Coord x, y, w, h;
   Evas_Object *proxy;
   Evas *e;
   Evas_Map *map;
   int slice_no, tx, ty;
   double total_angle, start_angle, angle, dist;
   double u0, u1, v0, v1;
   int i;

   if (!pd->content) return;

   efl_gfx_geometry_get(pd->content, &x, &y, &w, &h);
   ERR("content geo: %d %d %d %d", x, y, w, h);
   e = evas_object_evas_get(pd->content);

   if (pd->autofit)
     {
        total_angle = (360 * w) / (2 * PI * (pd->circle.radius - h));
        if (total_angle > 360) total_angle = 360;

        slice_no = ((pd->slice_no * total_angle) / 360) + 1;
        if (slice_no > pd->slice_no) slice_no = pd->slice_no;
     }
   else
     {
        slice_no = pd->slice_no;
        total_angle = 360;
     }
   ERR("total angle: %.1f, slice no: %d, pd->slice_no: %d", total_angle, slice_no, pd->slice_no);

   dist = (double)w / slice_no;
   angle = _deg_to_rad(total_angle / slice_no);
   start_angle = _deg_to_rad(pd->circle.start_angle);

   if (pd->proxy)
     efl_del(pd->proxy);
   proxy = evas_object_image_filled_add(e);
   pd->proxy = proxy;
   efl_gfx_size_set(proxy, w, h);
   evas_object_image_source_set(proxy, pd->content);
   efl_gfx_visible_set(proxy, EINA_TRUE);

   map = evas_map_new(slice_no * 4);

   for (i = 0; i < slice_no; i++)
     {
        _transform_coord(0, -pd->circle.radius,
                         i * angle + start_angle,
                         pd->circle.x, pd->circle.y,
                         &tx, &ty);
        evas_map_point_coord_set(map, i * 4, tx, ty, 0);
        _transform_coord(0, -pd->circle.radius,
                         (i + 1) * angle + start_angle,
                         pd->circle.x, pd->circle.y,
                         &tx, &ty);
        evas_map_point_coord_set(map, i * 4 + 1, tx, ty, 0);
        _transform_coord(0, -(pd->circle.radius - h),
                         (i + 1) * angle + start_angle,
                         pd->circle.x, pd->circle.y,
                         &tx, &ty);
        evas_map_point_coord_set(map, i * 4 + 2, tx, ty, 0);
        _transform_coord(0, -(pd->circle.radius - h),
                         i * angle + start_angle,
                         pd->circle.x, pd->circle.y,
                         &tx, &ty);
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
   evas_object_map_enable_set(proxy, EINA_TRUE);
   evas_object_map_set(proxy, map);
   evas_map_free(map);

   //debug
   x = pd->circle.x;
   y = pd->circle.y;
   double r = pd->circle.radius;
   Eo *line = evas_object_line_add(e);
   efl_gfx_color_set(line, 100, 0, 0, 100);
   efl_gfx_visible_set(line, EINA_TRUE);
   evas_object_line_xy_set(line, x, y - r, x, y + r);

   line = evas_object_line_add(e);
   efl_gfx_color_set(line, 100, 100, 0, 100);
   efl_gfx_visible_set(line, EINA_TRUE);
   evas_object_line_xy_set(line, x - r, y, x + r, y);
}

static void
_sizing_eval(Efl_Ui_Textpath_Data *pd)
{
   _circle_draw(pd);
}

static void
_content_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Efl_Ui_Textpath_Data *pd = data;
   _sizing_eval(pd);
}

//exposed API
EOLIAN static void
_efl_ui_textpath_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
   ERR("In");
   _sizing_eval(pd);
}

EOLIAN static void
_efl_ui_textpath_class_constructor(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
}

EOLIAN static Efl_Object *
_efl_ui_textpath_efl_object_constructor(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
   ERR("in");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->autofit = EINA_TRUE;
   pd->slice_no = 99;
   pd->ori = EFL_UI_TEXTPATH_ORIENTATION_LEFT_TO_RIGHT;

   return obj;
}

EOLIAN static void
_efl_ui_textpath_efl_object_destructor(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
   if (pd->content) evas_object_del(pd->content);
   if (pd->text) free(pd->text);
   efl_destructor(efl_super(obj, MY_CLASS));
}

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
   content = evas_object_text_add(evas_object_evas_get(obj)); //efl_add(EVAS_TEXT_CLASS, obj);
   evas_object_text_font_set(content, "century schoolbook l", 20); //FIXME: which font and size???
   efl_text_set(content, pd->text);
   efl_gfx_size_hint_min_set(content, 10, 50); //FIXME: remove it
   efl_gfx_size_hint_weight_set(content, EFL_GFX_SIZE_HINT_FILL,
                                        EFL_GFX_SIZE_HINT_FILL);
   efl_gfx_position_set(content, 0, 0);
   evas_object_event_callback_add(content, EVAS_CALLBACK_RESIZE, _content_resize_cb, pd);
   efl_gfx_size_set(content, 2 * pd->circle.radius, 2 * pd->circle.radius);
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

//we cannot know the internal of efl_gfx_path
//therefore, we cannot draw text on it.
//cannot support this api
EOLIAN static Efl_Gfx_Path *
_efl_ui_textpath_path_get(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
    return pd->path;
}

EOLIAN static void
_efl_ui_textpath_path_set(Eo *obj, Efl_Ui_Textpath_Data *pd, Efl_Gfx_Path *path)
{
    pd->path = path;
}
//////

EOLIAN static void
_efl_ui_textpath_circle_set(Eo *obj, Efl_Ui_Textpath_Data *pd, double x, double y, double radius, double start_angle)
{
   if (pd->circle.x == x && pd->circle.y == y &&
       pd->circle.radius == radius &&
       pd->circle.start_angle == start_angle)
     {
        ERR("Same circle");
        return;
     }
   pd->circle.x = x;
   pd->circle.y = y;
   pd->circle.radius = radius;
   pd->circle.start_angle = start_angle;

   if (pd->content)
     {
        efl_gfx_size_set(pd->content, 2 * radius, 2 * radius);
     }

   _sizing_eval(pd);
}

EOLIAN static Efl_Ui_Textpath_Orientation
_efl_ui_textpath_orientation_get(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
    return pd->ori;
}

EOLIAN static void
_efl_ui_textpath_orientation_set(Eo *obj, Efl_Ui_Textpath_Data *pd, Efl_Ui_Textpath_Orientation orientation)
{
    if (pd->ori == orientation) return;
    pd->ori = orientation;
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
    _sizing_eval(pd);
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
   _sizing_eval(pd);
}


#include "efl_ui_textpath.eo.c"

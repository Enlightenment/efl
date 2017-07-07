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
#define SLICE_DEFAULT_NO 69

typedef struct _Efl_Ui_Textpath_Data Efl_Ui_Textpath_Data;

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
   Efl_Ui_Textpath_Orientation ori;
   int slice_no;
   Eina_Bool autofit;
   Eina_Bool ellipsis;
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
_textpath_ellipsis_enable_set(Efl_Ui_Textpath_Data *pd, int length, Eina_Bool enabled)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(pd->text_obj, wd);

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
   edje_object_part_text_style_user_pop(wd->resize_obj, "elm.text");
   edje_object_part_text_style_user_push(wd->resize_obj, "elm.text", format);
   ERR("set format: %s", format);
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
             _textpath_ellipsis_enable_set(pd, circum, EINA_TRUE);
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
   //efl_gfx_visible_set(proxy, EINA_TRUE);
   //Test
   //return;
   //


   map = evas_map_new(slice_no * 4);

   for (i = 0; i < slice_no; i++)
     {
        _transform_coord(0, -pd->circle.radius,
                         i * angle + start_angle,
                         pd->circle.x, pd->circle.y,
                         &tx, &ty);
        if (i < 3)
        ERR("map: %d:: %d %d, circle: %.0f %.0f", i*4, tx, ty, pd->circle.x, pd->circle.y);
        evas_map_point_coord_set(map, i * 4, tx, ty, 0);
        _transform_coord(0, -pd->circle.radius,
                         (i + 1) * angle + start_angle,
                         pd->circle.x, pd->circle.y,
                         &tx, &ty);
        if (i < 3)
        ERR("map: %d:: %d %d, circle: %.0f %.0f", i*4 + 1, tx, ty, pd->circle.x, pd->circle.y);
        evas_map_point_coord_set(map, i * 4 + 1, tx, ty, 0);
        _transform_coord(0, -(pd->circle.radius - h),
        //_transform_coord(0, (pd->circle.radius - h),
                         (i + 1) * angle + start_angle,
                         pd->circle.x, pd->circle.y,
                         &tx, &ty);
        if (i < 3)
        ERR("map: %d:: %d %d, circle: %.0f %.0f, angle: %.1f", i*4 + 2, tx, ty, pd->circle.x, pd->circle.y, (i+1)*angle + start_angle);
        evas_map_point_coord_set(map, i * 4 + 2, tx, ty, 0);
        _transform_coord(0, -(pd->circle.radius - h),
        //_transform_coord(0, (pd->circle.radius - h),
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
   /*
   evas_object_size_hint_min_get(pd->content, &w, &h);
   Evas_Map *m = evas_map_new(4);
   ERR("Test: map with only 4 points: 10-100; 10-80, w,h: %d %d", w, h);
   evas_object_geometry_get(pd->text_obj, &x, &y, &w, &h);
   ERR("layout geo: %d %d %d %d", x, y, w, h);
   evas_map_point_coord_set(m, 0, 10, 10, 0);
   evas_map_point_coord_set(m, 1, 100, 10, 0);
   evas_map_point_coord_set(m, 2, 100, 90, 0);
   evas_map_point_coord_set(m, 3, 10, 90, 0);
   evas_map_point_image_uv_set(m, 0, 0, 0);
   evas_map_point_image_uv_set(m, 1, w, 0);
   evas_map_point_image_uv_set(m, 2, w, h);
   evas_map_point_image_uv_set(m, 3, 0, h);
   //evas_object_map_enable_set(pd->text_obj, EINA_TRUE);
   //evas_object_map_set(pd->text_obj, m);*/
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
   //
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



   /*ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   const Evas_Object *tb;
   Evas_Coord tb_w, tb_h;
   tb = edje_object_part_object_get(wd->resize_obj, "elm.text");
   evas_object_textblock_size_native_get(tb, &tb_w, &tb_h);
   //evas_object_size_hint_min_set(wd->resize_obj, tb_w, tb_h);
   //evas_object_size_hint_max_set(wd->resize_obj, tb_w, tb_h);
   //evas_object_resize(obj, tb_w, tb_h);
   //evas_object_size_hint_min_set(obj, tb_w, tb_h);
   ERR("tb size: %d %d", tb_w, tb_h);*/

}

static Eina_Bool
_textpath_text_set_internal(Eo *obj, Efl_Ui_Textpath_Data *sd, const char *part, const char *text)
{
   ERR("in: part: %s, text: %s", part, text);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   Eina_Bool int_ret = EINA_TRUE;

   if (!text) text = "";
   //set format
   //_textpath_format_set(wd->resize_obj, sd->format);

   efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), text);

   elm_layout_text_set(sd->text_obj, NULL, text);
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
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->text_obj, twd, EINA_FALSE);
   tb = edje_object_part_object_get(twd->resize_obj, "elm.text");
   evas_object_textblock_size_native_get(tb, &tb_w, &tb_h);
   //??? layout sizing_eval also does min set based on the edje size calc
   evas_object_size_hint_min_set(sd->text_obj, tb_w, tb_h);
   //FIXME: if we dont resize layout obj, size of 0
   //and map does not work.
   evas_object_resize(sd->text_obj, tb_w, tb_h);
   ERR("tb size: %d %d", tb_w, tb_h);
   //
   elm_obj_layout_sizing_eval(obj);



   //Test
   Evas_Object *t = elm_layout_add(obj);
   elm_layout_theme_set(t, "textpath", "base", elm_widget_style_get(obj));
   elm_object_text_set(t, text);
   evas_object_move(t, 50, 50);
   evas_object_resize(t, tb_w, tb_h);
   evas_object_show(t);
   /////
   return int_ret;
}


EOLIAN static Eina_Bool
_efl_ui_textpath_text_set(Eo *obj, Efl_Ui_Textpath_Data *sd, const char *part, const char *text)
{
   ERR("in: part: %s, text: %s", part, text);
   return _textpath_text_set_internal(obj, sd, part, text);

   /*ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   Eina_Bool int_ret = EINA_TRUE;

   if (!text) text = "";
   //set format
   //_textpath_format_set(wd->resize_obj, sd->format);

   efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), text);

   elm_layout_text_set(sd->text_obj, NULL, text);
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
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->text_obj, twd, EINA_FALSE);
   tb = edje_object_part_object_get(twd->resize_obj, "elm.text");
   evas_object_textblock_size_native_get(tb, &tb_w, &tb_h);
   //??? layout sizing_eval also does min set based on the edje size calc
   evas_object_size_hint_min_set(sd->text_obj, tb_w, tb_h);
   //FIXME: if we dont resize layout obj, size of 0
   //and map does not work.
   evas_object_resize(sd->text_obj, tb_w, tb_h);
   ERR("tb size: %d %d", tb_w, tb_h);
   //
   elm_obj_layout_sizing_eval(obj);
   return int_ret;*/
}

EOLIAN static const char *
_efl_ui_textpath_text_get(Eo *obj, Efl_Ui_Textpath_Data *sd, const char *part)
{
   return elm_layout_text_get(sd->text_obj, part);
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

   priv->text_obj = elm_layout_add(obj);
   if (!elm_layout_theme_set(priv->text_obj, "textpath", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout");
   elm_layout_text_set(priv->text_obj, NULL, "Test");
   evas_object_move(priv->text_obj, 100, 100);
   evas_object_show(priv->text_obj);
   //priv->text_obj = edje_object_add(evas_object_evas_get(obj));
   //elm_widget_theme_object_set(obj, priv->text_obj, "textpath", "text", elm_widget_style_get(obj));
   //edje_object_part_text_set(priv->text_obj, "elm.text", "test text");
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
   _textpath_text_set_internal(obj, pd, "elm.text", text);
}

EOLIAN static const char *
_efl_ui_textpath_efl_text_text_get(Eo *obj, Efl_Ui_Textpath_Data *pd)
{
   return elm_layout_text_get(pd->text_obj, "elm.text");
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

EOLIAN static void
_efl_ui_textpath_ellipsis_set(Eo *obj, Efl_Ui_Textpath_Data *pd, Eina_Bool ellipsis)
{
   if (pd->ellipsis == ellipsis) return;
   pd->ellipsis = ellipsis;

   ///
   if (!pd->text_obj) return; //FIXME: add process in text_set
   Evas_Coord w = 0, h = 0;
   double circum;
   Eina_Bool is_ellipsis = EINA_FALSE;

   evas_object_size_hint_min_get(pd->text_obj, &w, &h);
   efl_gfx_size_get(pd->text_obj, &w, &h);
   circum = 2 * PI * (pd->circle.radius - h / 2);
   if (pd->ellipsis)
     {
        is_ellipsis = EINA_TRUE;
        if ((w > circum) && (circum > 0))
          w = circum;
        ERR("enable ellipsis");
     }
   /*if (pd->ellipsis)
     {
        if (w > circum)
          {
             evas_object_resize(pd->text_obj, circum, h);
             _textpath_ellipsis_enable_set(pd, circum, EINA_TRUE);
             elm_layout_sizing_eval(pd->text_obj);
          }
     }*/
   evas_object_resize(pd->text_obj, w, h);
   _textpath_ellipsis_enable_set(pd, circum, is_ellipsis);
   elm_layout_sizing_eval(pd->text_obj);
   //
   _sizing_eval(pd);
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
